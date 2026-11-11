#!/usr/bin/env python3
# Release portability/documentation changes: 2026-10-17; firmware behavior and assets unchanged.
# SPDX-License-Identifier: GPL-3.0-only
"""Local Record/Stop/Export controls. Passive MIDI input only, never MIDI output."""
import hashlib,json,os,secrets,shutil,struct,subprocess,sys,threading,time
from datetime import datetime
from pathlib import Path
from http.server import BaseHTTPRequestHandler,ThreadingHTTPServer
from zoneinfo import ZoneInfo
import mido
ROOT=Path(__file__).resolve().parent.parent
OUTPUT=ROOT/'recordings'
DESKTOP=Path(os.environ.get('RAMBEAVER_EXPORT_DIR', str(Path.home()/'Desktop'))).expanduser()
CONFIG=json.loads((ROOT/'config/recorder.json').read_text())
REVISION=CONFIG['revision']
SIZE=CONFIG['state_bytes']
BUILD=ROOT/f'build/recorder-{REVISION}'
WASM=ROOT/CONFIG['renderer']
LAYOUT=json.loads((ROOT/'config/capture-layout.json').read_text())
TOKEN=secrets.token_urlsafe(32)
NODE=shutil.which('node')
if not NODE:raise SystemExit('Node.js is required for recording export. Install Node.js, then retry.')

def write_json(path,value):
 temp=path.with_suffix(path.suffix+'.tmp');temp.write_text(json.dumps(value,indent=2)+'\n');temp.replace(path)

def digest(data):
 h=2166136261
 for b in data:h=((h^b)*16777619)&0xffffffff
 return h&0xfffffff

class Decoder:
 def __init__(self):
  self.current=None;self.previous=None;self.previous_id=-1;self.partial=0;self.corrupt=0;self.skipped=0
 def feed(self,m):
  if m.type!='sysex':return None
  p=bytes([240,*m.data,247])
  if len(p)<19 or p[1:4]!=bytes([125,82,66]) or p[4] not in (3,4):return None
  delta=p[4]==4;ver=p[5]|p[6]<<7;fid=p[7]|p[8]<<7;off=p[9]|p[10]<<7;size=p[11]|p[12]<<7;h=sum(p[13+k]<<(7*k) for k in range(4))
  if ver!=REVISION:raise RuntimeError(f'Connected stream is build {ver}; this recorder needs{REVISION}.')
  if not 0<size<=SIZE+(SIZE+127)//128+4:self.corrupt+=1;self.current=None;return None
  if off==0:
   if self.current is not None:self.partial+=1
   self.current=fid;self.packed=bytearray();self.checksum=h;self.size=size;self.delta=delta
  if self.current!=fid or off!=len(self.packed) or self.checksum!=h or self.size!=size or self.delta!=delta:
   self.partial+=1;self.current=None;return None
  j=17
  while j<len(p)-1:
   bits=p[j];j+=1
   for k in range(min(7,len(p)-1-j)):
    self.packed.append(p[j]|(((bits>>k)&1)<<7));j+=1
  if len(self.packed)<size:return None
  self.current=None
  if len(self.packed)!=size:self.corrupt+=1;return None
  raw=bytearray();j=0
  try:
   while j<size:
    tag=self.packed[j];j+=1;n=(tag&127)+1
    if tag&128:raw.extend([self.packed[j]]*n);j+=1
    else:
     if j+n>size:raise ValueError('Truncated literal')
     raw.extend(self.packed[j:j+n]);j+=n
    if len(raw)>SIZE:raise ValueError('Oversize state')
   if len(raw)!=SIZE:raise ValueError('Wrong state size')
  except (IndexError,ValueError):self.corrupt+=1;self.previous=None;return None
  if delta:
   if self.previous is None or fid!=((self.previous_id+1)&16383):self.skipped+=1;self.previous=None;return None
   raw=bytearray(a^b for a,b in zip(raw,self.previous))
  if digest(raw)!=h:self.corrupt+=1;self.previous=None;return None
  self.previous=bytes(raw);self.previous_id=fid
  return bytes(raw),fid,delta

class Recorder:
 def __init__(self):
  self.lock=threading.RLock();self.state={'status':'ready','folder':str(OUTPUT),'output':'','frames':0,'seconds':0,'fps':0,'message':'Ready. Keep the connected Chrome viewer visible.'};self.worker=None;self.stop_event=threading.Event()
 def status(self):
  with self.lock:return dict(self.state)
 def update(self,**values):
  with self.lock:self.state.update(values)
 def start(self,validation=False):
  with self.lock:
   if self.worker and self.worker.is_alive():raise ValueError('A recording or export is already running.')
   stamp=datetime.now(ZoneInfo('America/Vancouver')).strftime('%Y-%m-%d_%H-%M-%S')
   folder=(BUILD/'validation' if validation else OUTPUT)/('take-'+stamp)
   folder.mkdir(parents=True,exist_ok=False);self.folder=folder;self.validation=validation;self.auto_export=False;self.stop_event.clear()
   self.destination=DESKTOP/('RAMBEAVER-recorder-check-'+stamp+'.mov') if validation=='desktop' else folder/'playthrough.mov' if validation else DESKTOP/('RAMBEAVER-'+stamp+'.mov')
   self.state={'status':'starting','folder':str(folder),'output':str(self.destination),'frames':0,'seconds':0,'fps':0,'message':'Opening FM-1 USB audio and listening for device frames…'}
   self.worker=threading.Thread(target=self.record,daemon=True);self.worker.start()
 def stop(self,auto_export=False):
  with self.lock:
   if self.state['status'] not in ('starting','recording'):raise ValueError('No active recording.')
   self.auto_export=auto_export;self.stop_event.set();self.update(status='stopping',message='Finishing the audio and preserving received frames…')
 def record(self):
  folder=self.folder;audio=None;start=time.monotonic();first=last=None;frames=0;scenes=set();decoder=Decoder();error=None;stop=folder/'STOP';last_write=0;first_xrun=last_xrun=None
  try:
   ports=[p for p in mido.get_input_names() if 'Felucca' in p or 'FM-1' in p]
   if len(ports)!=1:raise RuntimeError(f'Expected one FM-1 MIDI input; found {len(ports)}. Connect the device and Chrome follower.')
   # Freeze the matching renderer with the take, so future builds cannot alter it.
   (folder/'renderer.wasm').write_bytes(WASM.read_bytes())
   with (folder/'audio.log').open('w') as audio_log:
    audio=subprocess.Popen([str(BUILD/'audio'),str(folder/'usb-audio.mov'),'86400',str(stop)],stdout=audio_log,stderr=subprocess.STDOUT,cwd=ROOT)
    with mido.open_input(ports[0]) as inp,(folder/'frames.bin').open('wb') as raw,(folder/'frames.jsonl').open('w') as index:
     while not self.stop_event.is_set():
      now=time.monotonic()
      if audio.poll() is not None:raise RuntimeError('FM-1 USB audio stopped. See audio.log in the take folder.')
      m=inp.poll()
      if m is None:time.sleep(.0005)
      else:
       result=decoder.feed(m)
       if result:
        data,fid,delta=result;last=now
        if first is None:first=now
        row={'received_host_seconds':now,'frame':fid,'delta':delta}
        row.update({k:struct.unpack_from('<i',data,off)[0] for k,off in LAYOUT.items()})
        raw.write(struct.pack('<dI',now,len(data)));raw.write(data);index.write(json.dumps(row)+'\n');frames+=1;scenes.add(row['scene'])
        last_xrun=row['xruns']
        if first_xrun is None:first_xrun=last_xrun
      if now-last_write>.5:
       status='recording' if frames and (folder/'usb-audio.mov.ready').exists() else 'starting'
       self.update(status=status,frames=frames,seconds=round(now-start,1),fps=round((frames-1)/(last-first),1) if first and last>first else 0,message='Recording device + USB audio. Keep Chrome’s follower visible.' if status=='recording' else 'Waiting for the connected Chrome follower and USB audio…')
       raw.flush();index.flush();last_write=now
      if now-(last or start)>6:raise RuntimeError(f'No device frames for6seconds. Keep the{REVISION} Chrome follower connected and visible. The received portion is saved.')
  except Exception as exc:error=str(exc)
  finally:
   capture_end=time.monotonic();stop.touch()
   if audio:
    try:audio.wait(timeout=15)
    except subprocess.TimeoutExpired:audio.terminate();audio.wait(timeout=5);error=error or 'USB audio did not finish normally.'
   end=capture_end
   report={'revision':REVISION,'state_bytes':SIZE,'frames':frames,'scenes':sorted(scenes),'start_host_seconds':start,'first_frame_host_seconds':first,'last_frame_host_seconds':last,'end_host_seconds':max(last or start,end),'duration_seconds':max(0,end-start),'corrupt_frames':decoder.corrupt,'discarded_partial_chunks':decoder.partial,'dependent_deltas_skipped':decoder.skipped,'audio_xruns_added':None if first_xrun is None else last_xrun-first_xrun,'midi_output_commands':0,'gameplay_commands_sent':0,'save_commands_sent':0,'error':error,'renderer_sha256':hashlib.sha256((folder/'renderer.wasm').read_bytes()).hexdigest() if (folder/'renderer.wasm').exists() else None}
   write_json(folder/'capture.json',report)
   audio_meta=folder/'usb-audio.mov.json'
   audio_ok=audio_meta.exists() and json.loads(audio_meta.read_text()).get('writer_status')==2 and json.loads(audio_meta.read_text()).get('samples',0)>0
   self.update(status='stopped' if frames and audio_ok else 'error',frames=frames,seconds=round(report['duration_seconds'],1),message=(error+' ' if error else '')+('Take saved. Click Export to create playthrough.mov.' if frames and audio_ok else 'Recording unavailable. Raw evidence and error details are in the take folder.'))
   if self.auto_export and frames and audio_ok:
    self.update(status='exporting',message='Exporting your complete take to Desktop…');self.export_worker()
 def export(self):
  with self.lock:
   if self.worker and self.worker.is_alive():raise ValueError('Wait for the recording to stop first.')
   if self.state['status'] not in ('stopped','complete'):raise ValueError('Record and stop a take first.')
   if (self.folder/'playthrough.mov.json').exists():self.update(status='complete',message='Already exported. Your movie is ready.');return
   self.update(status='exporting',message='Exporting every received frame with synchronized FM-1 USB audio…')
   self.worker=threading.Thread(target=self.export_worker,daemon=True);self.worker.start()
 def export_worker(self):
  try:
   with (self.folder/'export.log').open('w') as log:
    render=subprocess.Popen([NODE,str(ROOT/'tools/render_fullrun.mjs'),str(self.folder),str(self.folder/'renderer.wasm')],stdout=subprocess.PIPE,stderr=log,cwd=ROOT)
    encode=subprocess.Popen([str(BUILD/'export'),str(self.folder)],stdin=render.stdout,stdout=log,stderr=subprocess.STDOUT,cwd=ROOT);render.stdout.close()
    er=encode.wait();rr=render.wait()
    if er or rr:raise RuntimeError('Export failed; raw recording is preserved. See export.log.')
   meta=json.loads((self.folder/'playthrough.mov.json').read_text());capture=json.loads((self.folder/'capture.json').read_text())
   if meta['actual_device_frames']!=capture['frames']:raise RuntimeError('Export frame count differs from recorded count.')
   if self.destination!=self.folder/'playthrough.mov':
    if self.destination.exists():raise RuntimeError('Desktop filename already exists. Raw movie is preserved in the take folder.')
    temp=self.destination.with_suffix('.mov.partial')
    with temp.open('xb') as out,(self.folder/'playthrough.mov').open('rb') as src:shutil.copyfileobj(src,out,1024*1024)
    temp.rename(self.destination)
   self.update(status='complete',message=('Capture warning: '+capture['error']+' ' if capture.get('error') else '')+('Export complete. Game video + FM-1 USB audio saved to Desktop.' if not self.validation else 'Validation export complete.'),export_seconds=round(meta['duration_seconds'],2))
  except Exception as exc:self.update(status='error',message=str(exc))

REC=Recorder()
HTML='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>RAMBEAVER · Recording controls</title>
<style>body{background:#0a1720;color:#f3e8ce;font:18px system-ui;margin:0;padding:36px;max-width:850px}h1{font-size:32px;margin:0 0 12px}p{line-height:1.5}button{font:700 19px system-ui;border:0;border-radius:10px;padding:17px 26px;cursor:pointer;margin:8px 10px 8px 0}#record{background:#e5b84c}#stop{background:#e67760}#export{background:#98cbb4}button:disabled{opacity:.35;cursor:default}.box{background:#132733;padding:20px;border-radius:14px;margin:22px 0}#path{font:14px ui-monospace;overflow-wrap:anywhere;user-select:all}#metrics{color:#b2c9d2}#state{font-weight:bold;color:#e5b84c}.small{font-size:14px;color:#b2c9d2}a{color:#98cbb4}</style>
<h1>Record your Beaver playthrough</h1><p>Keep your connected <strong>matching Chrome viewer visible</strong>. Play on the FM-1. These controls record the actual device picture and its USB audio.</p>
<div><button id="record" onclick="act('record')">● Record</button><button id="stop" onclick="act('stop')" disabled>■ Stop</button><button id="export" onclick="act('export')" disabled>Export movie</button></div>
<div class="box"><div id="state">Ready</div><p id="message">Your take starts only when you click Record.</p><div id="metrics">0 frames · 0 seconds</div></div>
<div class="box"><strong>Movie destination</strong><p id="path">Desktop/RAMBEAVER-YYYY-MM-DD_HH-MM-SS.mov</p><button onclick="navigator.clipboard.writeText(document.querySelector('#path').textContent)">Copy path</button></div>
<p class="small">Record → Stop → Export movie. Each take gets its own folder. All scenes are included. Raw device frames and USB audio stay beside the movie. Export uses original frame timing; screen/USB latency has not been calibrated. No microphone or computer audio is recorded.</p>
<script>const token='TOKEN';let pending=false;async function refresh(){try{const s=await(await fetch('/status')).json();document.querySelector('#state').textContent=s.status.toUpperCase();document.querySelector('#message').textContent=s.message;document.querySelector('#metrics').textContent=`${s.frames} frames · ${s.seconds}s · ${s.fps} fps`;if(s.output)document.querySelector('#path').textContent=s.output;document.querySelector('#record').disabled=pending||!['ready','stopped','complete','error'].includes(s.status);document.querySelector('#stop').disabled=pending||!['starting','recording'].includes(s.status);document.querySelector('#export').disabled=pending||!['stopped','complete'].includes(s.status);}catch(e){document.querySelector('#message').textContent='Recorder connection lost. Keep this window open and restart Launch Recorder.command if needed.';}}async function act(action){pending=true;await refresh();try{const r=await fetch('/'+action,{method:'POST',headers:{'X-Recorder-Token':token}});const d=await r.json();if(!r.ok)alert(d.error);}finally{pending=false;await refresh();}}setInterval(refresh,500);refresh();</script></html>'''

class Handler(BaseHTTPRequestHandler):
 def log_message(self,*a):pass
 def send(self,status,data,kind='application/json'):
  content=data.encode() if isinstance(data,str) else json.dumps(data).encode();self.send_response(status);self.send_header('Content-Type',kind);origin=self.headers.get('Origin');self.send_header('Access-Control-Allow-Origin',origin if origin in ('http://127.0.0.1:8765','http://localhost:8765','http://127.0.0.1:8766') else 'http://127.0.0.1:8766');self.send_header('Cache-Control','no-store');self.send_header('Content-Length',str(len(content)));self.end_headers();self.wfile.write(content)
 def do_GET(self):
  if self.path=='/':self.send(200,HTML.replace('TOKEN',TOKEN),'text/html; charset=utf-8')
  elif self.path=='/status':self.send(200,dict(REC.status(),token=TOKEN,revision=REVISION,state_bytes=SIZE))
  elif self.path=='/movie' and REC.status()['status']=='complete':
   movie=Path(REC.status()['output']);self.send_response(200);self.send_header('Content-Type','video/quicktime');self.send_header('Content-Length',str(movie.stat().st_size));self.end_headers()
   with movie.open('rb') as f:shutil.copyfileobj(f,self.wfile)
  elif self.path=='/viewer-controls.js':self.send(200,(ROOT/'web/emu/recorder-controls.js').read_text(),'text/javascript; charset=utf-8')
  else:self.send(404,{'error':'Not found'})
 def do_OPTIONS(self):
  self.send_response(204);self.send_header('Access-Control-Allow-Origin','http://127.0.0.1:8765');self.send_header('Access-Control-Allow-Methods','POST, GET, OPTIONS');self.send_header('Access-Control-Allow-Headers','X-Recorder-Token');self.end_headers()
 def do_POST(self):
  if self.headers.get('X-Recorder-Token')!=TOKEN or self.headers.get('Host') not in ('127.0.0.1:8766','localhost:8766'):self.send(403,{'error':'Use the local recording control window.'});return
  try:
   if self.path=='/record':REC.start()
   elif self.path=='/stop':REC.stop(auto_export=True)
   elif self.path=='/export':REC.export()
   else:raise ValueError('Unknown action')
   self.send(200,REC.status())
  except Exception as exc:self.send(409,{'error':str(exc)})

if __name__=='__main__':
 os.chdir(ROOT);OUTPUT.mkdir(exist_ok=True);DESKTOP.mkdir(parents=True,exist_ok=True)
 if '--validate' in sys.argv or '--validate-desktop' in sys.argv:
  REC.start(validation='desktop' if '--validate-desktop' in sys.argv else True);time.sleep(5)
  if REC.status()['status'] in ('starting','recording'):REC.stop(auto_export='--validate-desktop' in sys.argv)
  REC.worker.join(25)
  if '--validate-desktop' not in sys.argv:
   if REC.status()['status']!='stopped':raise RuntimeError(REC.status())
   REC.export();REC.worker.join(120)
  print(json.dumps(REC.status(),indent=2));sys.exit(0 if REC.status()['status']=='complete' else 1)
 print('Recording controls: http://127.0.0.1:8766/\nMovies: '+str(DESKTOP),flush=True)
 ThreadingHTTPServer(('127.0.0.1',8766),Handler).serve_forever()
