#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Place decorative art around unchanged collision geometry, offline and deterministically."""
from pathlib import Path
import json,math
R=Path(__file__).resolve().parents[1];plan=json.loads((R/'assets/level-plan.json').read_text())
# Authored door centres and foundation heights; labels live within lintels.
doors=[[(520,212,'SALVAGE')],[(1580,212,'CACHE'),(3790,176,'DATA CENTER')],[(24,212,'EXIT')],[],[(24,212,'EXIT')],[(24,212,'EXIT')]]
billboards=[[(4,25,140,44,0)],[],[],[],[(72,40,178,31,4)],[(72,40,178,31,5)]]
sign_text=['SINGULARITY: 99%','REPORT ALL BEAVERS','HUMANS: OFFLINE','EFFICIENCY IS LAW','ORGANICS DETECTED','COMPLIANCE IS CALM']
neons=[]
def overlaps(a,b):return a[0]<b[0]+b[2] and a[0]+a[2]>b[0] and a[1]<b[1]+b[3] and a[1]+a[3]>b[1]
for sc in range(3):
 last=-600
 for x in range(40,plan['widths'][sc]-120,24):
  if x-last<480 or (sc==0 and x<400):continue
  ix=(len(neons)+sc)%len(sign_text);w=len(sign_text[ix])*6+8
  if not any(a+4<=x and x+w<=a+length-4 for a,y,length in plan['floors'][sc]):continue
  for y in [40,24,56]:
   box=(x-3,y-3,w+6,20)
   # Physics gives <70 px rise and <114 px airborne horizontal reach.
   # Reserve the entire body and jump approach around every raised surface.
   envelopes=[(px-114,py-106,pw+228,110) for px,py,pw in plan['ledges'][sc]]
   envelopes += [(a,fy-106,length,110) for a,fy,length in plan['floors'][sc]]
   if not any(overlaps(box,q) for q in envelopes+[(dx-45,dy-58,90,58) for dx,dy,_ in doors[sc]]):
    neons.append((sc,x,y,w,ix));billboards[sc].append((x,y,w,14,10+ix));last=x;break
facades=[];details=[];props=[];tests=[]
def overlaps(a,b):return a[0]<b[0]+b[2] and a[0]+a[2]>b[0] and a[1]<b[1]+b[3] and a[1]+a[3]>b[1]
for scene in range(6):
 fs=plan['floors'][scene];ls=plan['ledges'][scene]
 forbidden=[(x-5,y-5,w+10,23) for x,y,w in ls]+[(x-45,y-58,90,58) for x,y,n in doors[scene]]+[(x-4,y-4,w+8,h+8) for x,y,w,h,_ in billboards[scene]]
 occupied=[]
 def clear(box):return not any(overlaps(box,b) for b in forbidden+occupied)
 def supported(x,w):return any(a+4<=x and x+w<=a+length-4 for a,y,length in fs)
 # Lamps and bins have real footings and avoid doors, platforms and initial actors/loot.
 actors=[(x-6,y-7,38,39) for sc,x,y,t,hp in plan['robots'] if sc==scene and t!=2]
 pickups=[(x-8,y-9,16,18) for sc,x,y,*_ in plan['pickups'] if sc==scene]
 if scene!=3:
  for a,fy,length in fs:
   if scene<2:
    for desired in range(a+128,a+length-30,320):
     for delta in [0,38,-38,76,-76]:
      x=desired+delta;box=(x-10,fy-76,21,76)
      if supported(box[0],box[2]) and clear(box) and not any(overlaps(box,q) for q in actors+pickups):
       props.append((scene,x,fy,0));occupied.append(box);break
   for desired in range(a+116,a+length-30,220):
    for delta in [0,34,-34,68,-68]:
     x=desired+delta;box=(x,fy-29,30,29)
     if supported(x,30) and clear(box) and not any(overlaps(box,q) for q in actors+pickups):
      props.append((scene,x,fy,1));occupied.append(box);break
 if scene==3:continue
 for a,fy,length in fs:
  n=max(1,math.ceil((length-16)/176));width=(length-16)//n
  for j in range(n):
   x=a+8+j*width;w=width-7;h=(104 if scene==0 else 128 if scene==1 else 140)+((j+scene)%3)*7;top=max(48,fy-h)
   if scene>=4:top=82
   facades.append((scene,x,top,w,fy-top))
   # Only complete windows fit; their trim never crosses a platform/support band.
   for yy in range(top+23,fy-27,27):
    for xx in range(x+12,x+w-29,38):
     box=(xx,yy,27,19)
     if yy+19<=fy-10 and clear((xx-2,yy-2,31,23)):
      details.append((scene,xx,yy,27,19,0,(j+xx//38+yy//27)%4));tests.append(('window',scene,box))
   # Wall drains and shutter panels also stop above the foundation.
   bx=x+w-11;box=(bx,top+16,5,fy-top-23)
   if clear((bx-2,top+14,9,fy-top-20)):details.append((scene,*box,1,0))
   vent=(x+15,top-13,32,13)
   if clear((vent[0]-3,vent[1]-3,38,19)):details.append((scene,*vent,2,0))
   # A short anchored service cable, rather than free-floating power lines.
   cable=(x+7,top+7,w-14,9)
   if clear((cable[0]-2,cable[1]-2,cable[2]+4,13)):details.append((scene,*cable,3,0))
   # Low wall vents add depth where no clear window bay is available.
   panel=(x+12,fy-29,35,18)
   if clear((panel[0]-2,panel[1]-2,39,22)):details.append((scene,*panel,4,scene))
# Compile per-scene bounded tables. Each feature is culled by its own world rectangle.
rows=['/* Generated original scenery; collision/physics tables are unchanged. */','#pragma once','typedef struct {int16_t x,y,w,h;} art_box_t;','typedef struct {int16_t x,y,w,h;uint8_t kind,variant;} art_detail_t;','typedef struct {int16_t x,base;uint8_t kind;} art_prop_t;']
for name,data,typ,nfields in [('facade',facades,'art_box_t',4),('detail',details,'art_detail_t',6),('prop',props,'art_prop_t',3)]:
 counts=[]
 for scene in range(6):
  entries=[r[1:] for r in data if r[0]==scene];counts.append(len(entries));rows.append(f'static const {typ} {name}_{scene}[]={{'+(','.join('{'+','.join(map(str,r))+'}' for r in entries) if entries else '{0}')+'};')
 rows.append(f'static const {typ} *const art_{name}[6]={{'+','.join(f'{name}_{i}' for i in range(6))+'};')
 rows.append(f'static const uint16_t art_{name}_count[6]={{'+','.join(map(str,counts))+'};')
rows+=['typedef struct {int16_t x,y,w;uint8_t scene,text;} neon_t;','static const char *const neon_text[]={'+','.join(json.dumps(t) for t in sign_text)+'};',f'#define NNEON {len(neons)}','static const neon_t neon_signs[]={'+','.join('{%d,%d,%d,%d,%d}'%(x,y,w,sc,t) for sc,x,y,w,t in neons)+'};']
(R/'firmware/src/app/scenery.h').write_text('\n'.join(rows)+'\n')
(R/'assets/scenery-plan.json').write_text(json.dumps({'facades':facades,'details':details,'props':props,'doors':doors,'billboards':billboards,'neons':neons},indent=2)+'\n')
print(f'Scenery: {len(facades)} grounded facades, {len(details)} cleared details, {len(props)} supported street/room props, {len(neons)} neon signs outside jump envelopes.')
