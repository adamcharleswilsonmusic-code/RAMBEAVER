#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Serve the bundled preview and optionally the passive recording controls."""
import atexit,json,subprocess,sys,urllib.request,webbrowser
from pathlib import Path
root=Path(__file__).resolve().parent.parent
mode=sys.argv[1] if len(sys.argv)>1 else 'preview'
config=json.loads((root/'config/recorder.json').read_text())
children=[]
def stop_children():
 for child in children:
  if child.poll() is None:child.terminate()
atexit.register(stop_children)
try:
 with urllib.request.urlopen('http://127.0.0.1:8765/version.json',timeout=1) as r:version=json.load(r)
 if version.get('revision')!=config['revision']:raise SystemExit('Another preview occupies port8765. Close it before starting this release.')
except urllib.error.HTTPError:raise SystemExit('Another server occupies port8765. Close that server before starting this release.')
except (urllib.error.URLError,TimeoutError):
 child=subprocess.Popen([sys.executable,'-m','http.server','8765','--bind','127.0.0.1','--directory',str(root/'dist/browser')]);children.append(child)
 try:child.wait(timeout=.15);raise SystemExit('Preview server could not start.')
 except subprocess.TimeoutExpired:pass
url='http://127.0.0.1:8765/'
print('RAMBEAVER viewer: '+url,flush=True)
if mode=='record':
 if not (root/f"build/recorder-{config['revision']}/audio").exists():raise SystemExit('Prepare the recorder first: python tools/prepare_recorder.py')
 webbrowser.open(url)
 subprocess.run([sys.executable,str(root/'tools/fullrun_recorder.py')],cwd=root,check=True)
else:
 webbrowser.open(url)
 print('Leave this window open. Start game & sound plays in the browser; Follow FM-1 mirrors the USB device silently.')
 try:
  if children:children[0].wait()
 except KeyboardInterrupt:pass
