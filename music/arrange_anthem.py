# SPDX-License-Identifier: GPL-3.0-only
"""Original four-voice instrumental setting of the official O Canada melody.
Melody: Calixa Lavallee; transcribed from Canadian Heritage's one-page score.
No lyrics, recording, published accompaniment or performance is imported.
"""
from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1]
# F major; sixteenth-note ticks at the game's120BPM. Each row is one4/4bar.
# Zero is a rest. Ornamented pickups use the exact printed dotted durations.
bars=[
 [(69,8),(72,6),(72,2)],[(65,12),(67,4)],[(69,4),(70,4),(72,4),(74,4)],[(67,12),(0,4)],
 [(69,8),(71,6),(71,2)],[(72,12),(74,4)],[(76,4),(76,4),(74,4),(74,4)],[(72,12),(67,3),(69,1)],
 [(70,6),(69,2),(67,4),(69,3),(70,1)],[(72,6),(70,2),(69,4),(70,2),(72,2)],[(74,4),(72,4),(70,4),(69,4)],[(67,12),(67,3),(69,1)],
 [(70,6),(69,2),(67,4),(69,3),(70,1)],[(72,6),(70,2),(69,4),(69,4)],[(67,4),(72,4),(72,2),(71,2),(69,2),(71,2)],[(72,8),(0,8)],
 [(69,8),(72,6),(72,2)],[(65,8),(0,8)],[(70,8),(74,6),(74,2)],[(67,8),(0,8)],
 [(72,8),(73,6),(73,2)],[(74,4),(70,4),(69,4),(67,4)],[(65,8),(67,8)],[(69,12),(0,4)],
 [(72,8),(77,6),(77,2)],[(74,4),(70,4),(69,4),(67,4)],[(72,8),(64,8)],[(65,12),(0,4)]
]
# Original bright brass lead, buoyant bass, chord arpeggios and parade drums.
roots=[41,36,41,36,41,36,41,36,46,41,46,36,46,41,36,41,41,41,46,36,45,38,36,41,46,43,36,41]
events=[];melody=[]
for bar,notes in enumerate(bars):
 assert sum(n[1] for n in notes)==16
 t=bar*16
 for note,length in notes:
  if note:events.append([t,0,note,12,length,54+(3 if bar>=24 else 0)]);melody.append([t,note,length])
  t+=length
 root=roots[bar];third=root+(3 if bar in (21,25) else 4)
 for beat in range(4 if bar<27 else 3):
  dt=beat*4;bass=root+(0,7,12,7)[beat]
  events.append([bar*16+dt,1,bass,14,3,40+(4 if beat==0 else 0)])
  harmony=(root+12,third+12,root+19,third+12)[beat]
  events.append([bar*16+dt,2,harmony,13,3,29+(3 if bar>=24 else 0)])
 for dt in range(0,16 if bar<27 else 12,2):
  drum=3 if dt%8==0 else 8 if dt%4==0 else 5
  events.append([bar*16+dt,3,32 if drum==3 else 67,drum,1,44 if drum==3 else 34 if drum==8 else 25])

events.sort(key=lambda x:(x[0],x[1]));assert len({(e[0],e[1]) for e in events})==len(events)
lines=['/* SPDX-License-Identifier: GPL-3.0-only */','/* Original synth setting; melody reference: provenance/references/o-canada-official-score.pdf. */','#pragma once','#define ANTHEM_TICKS 448u','#define ANTHEM_LOOP_FRAMES 2469600u',f'#define ANTHEM_EVENTS {len(events)}','typedef struct {uint16_t tick;uint8_t channel,note,instrument,length,level;} anthem_event_t;','static const anthem_event_t anthem_events[ANTHEM_EVENTS]={']
lines += ['{'+','.join(map(str,e))+'},' for e in events];lines += ['};']
(R/'firmware/src/dsp/anthem_score.h').write_text('\n'.join(lines)+'\n')
meta={'title':'O Canada','melody_composer':'Calixa Lavallee','reference_url':'https://www.canada.ca/content/dam/pch/documents/services/anthems-canada/oCanada_SheetMusic_BIL_1492621145226_eng.pdf','official_context_url':'https://www.canada.ca/en/canadian-heritage/services/anthem-canada.html','reference_pdf':'provenance/references/o-canada-official-score.pdf','reference_sha256':hashlib.sha256((R/'provenance/references/o-canada-official-score.pdf').read_bytes()).hexdigest(),'derivation':'Manual note/duration transcription of the published melody, F major; original triumphant rounded singing lead, buoyant bass, major/minor chord arpeggios and parade percussion. No recording, lyrics or published accompaniment imported.','rights_basis':'Canadian Heritage states the anthem melody/words are public domain and arrangements may have their own copyright; this records that source statement, not a blanket rights claim for recordings or other arrangements.','bpm':120,'bars':28,'ticks':448,'frames':2469600,'seconds':56,'events':events,'melody_note_duration_trace':melody,'revision':735,'lead_timbre':'735: warm rounded triangle, gentle oscillator blend and filtering,20ms attack/50ms release; corrected melody and buoyant backing unchanged.','score_comparison':'music/o-canada-score-comparison.json','voice_limit':'4music +4effects','mode':6,'start_policy':'Only after building exit, after shutdown silence and indoor alarm.'}
(R/'music/o-canada-arrangement.json').write_text(json.dumps(meta,indent=2)+'\n')
print(f'O Canada: {len(melody)} melody notes, {len(events)} events,28bars/56seconds, F major; original four-voice synth setting.')
