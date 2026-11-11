#!/usr/bin/env python3
# Release portability/documentation changes: 2026-10-17; firmware behavior and assets unchanged.
# SPDX-License-Identifier: GPL-3.0-only
"""Convert exactly the eight user-authorized recordings; never modify originals.
Run with music/.venv/bin/python (numpy/scipy), or --input a source copy folder.
"""
from pathlib import Path
import argparse,hashlib,json,wave,math
import numpy as np
from scipy.signal import resample_poly
R=Path(__file__).resolve().parents[1]
FILES=[('beaver detected.wav','robot-acquire',.8),('stop that beaver.wav','robot-windup',.9),('chew on this.wav','successful-molotov',1.),('beaver jump.wav','real-jump',.25),('beaver hurt.wav','actual-damage',.6),('beaver log.wav','successful-health-log',.95),('witness me.wav','sixth-robot-kill',1.),('beavers essemble.wav','successful-posse-replaces-whistle',1.15)]
def convert(folder):
 lines=['/* SPDX-License-Identifier: GPL-3.0-only */','/* User-recorded voices; separate recording provenance: samples/voices/recordings.json. */','#pragma once','#include <stdint.h>','typedef struct {const int8_t *data;uint32_t length;} clip_t;'];report=[];arrays=[];entries=[];total=0
 dest=R/'samples/voices/converted';dest.mkdir(parents=True,exist_ok=True)
 for i,(name,cue,limit) in enumerate(FILES):
  p=folder/name;raw=p.read_bytes()
  with wave.open(str(p),'rb') as f:
   assert (f.getnchannels(),f.getframerate(),f.getsampwidth(),f.getcomptype())==(1,48000,3,'NONE'),name
   frames=f.getnframes();data=f.readframes(frames)
  v=np.frombuffer(data,dtype=np.uint8).reshape(-1,3).astype(np.int32);v=v[:,0]|v[:,1]<<8|v[:,2]<<16;v=(v^(1<<23))-(1<<23);x=v.astype(float)/(1<<23)
  x=resample_poly(x,1,6,window=('kaiser',8.6));x-=np.mean(x);assert len(x)<=int(limit*8000),name
  # Gentle2:1 peak compression,2ms attack/60ms release; retain all frames.
  env=0.;out=np.empty_like(x)
  for j,s in enumerate(x):
   a=math.exp(-1/(8000*(.002 if abs(s)>env else .060)));env=a*env+(1-a)*abs(s);gain=1. if env<=.10 else (env/.10)**-.5;out[j]=s*gain
  out*=94/max(abs(out));fade=np.minimum(1.,np.minimum(np.arange(len(out))/32,(len(out)-1-np.arange(len(out)))/64));pcm=np.rint(out*fade).clip(-127,127).astype(np.int8);total+=len(pcm);arrays.append(pcm)
  lines.append(f'static const int8_t voice_{i}[{len(pcm)}]={{');lines.extend(','.join(map(str,pcm[j:j+32]))+',' for j in range(0,len(pcm),32));lines.append('};');entries.append(f'{{voice_{i},{len(pcm)}}}')
  with wave.open(str(dest/f'{i}-{cue}.wav'),'wb') as f:f.setparams((1,2,8000,0,'NONE','not compressed'));f.writeframes((pcm.astype(np.int16)*256).astype('<i2').tobytes())
  report.append({'slot':i,'source_file':name,'source_sha256':hashlib.sha256(raw).hexdigest(),'source_frames':frames,'source_format':'mono48000Hz24bitPCM','cue':cue,'limit_seconds':limit,'bytes':len(pcm),'seconds':len(pcm)/8000,'pcm_sha256':hashlib.sha256(pcm.tobytes()).hexdigest(),'converted_wav':str((dest/f'{i}-{cue}.wav').relative_to(R))})
 assert total<=48000,total
 lines.append('static const clip_t clips[8]={'+','.join(entries)+'};');(R/'firmware/src/dsp/voice_bank.h').write_text('\n'.join(lines)+'\n')
 meta={'runtime_mapping':{'0': 'First visible alert/acquisition per ordinary robot; shared3s robot cooldown.', '1': 'Later attack windup; shared3s robot cooldown.', '2': 'Every successful Molotov throw; queued behind higher-priority speech, never empty/held-repeat/gun.', '3': 'Actual ground or motorcycle takeoff; not key-hold or jetpack thrust.', '4': 'Actual damage including fatal damage, after panic cleanup.', '5': 'Only successful one-use health-log consumption, not full-health contact or bottle pickups.', '6': 'Every sixth ordinary robot kill; persistent counter, simultaneous/active reactions coalesced. No ending or party speech.', '7': 'Successful posse summon only, replacing its synth whistle.'},'recorded_by':'User-supplied Adam recordings; no synthesized or imported third-party speech','originals_unchanged':True,'processing':'Anti-aliased48k→8k scipy resample_poly, Kaiser8.6; DC removal, gentle2:1 peak compression2ms/60ms, normalized peak94/128,4ms in/8ms out fades; all source duration retained, no word truncation.','total_bytes':total,'limit_bytes':48000,'remaining_bytes':48000-total,'clips':report};(R/'samples/voices/recordings.json').write_text(json.dumps(meta,indent=2)+'\n');(R/'build/voice-budget.json').write_text(json.dumps(meta,indent=2)+'\n');print(f'{total}/48000 PCM bytes;{48000-total} spare. All8 original hashes retained.')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--input',type=Path,required=True);convert(p.parse_args().input)
