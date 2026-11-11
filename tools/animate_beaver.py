#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Original native pixel articulation of the retained ImageGen animal.
All colors/anatomy come from beaver_bitmap.h. Each56x32 frame is4-bit packed;
ground contact is row31. Eight transparent top rows preserve the accepted
walking pixels while providing room for authored airborne articulation.
"""
from pathlib import Path
import re,json,math
from PIL import Image,ImageDraw
R=Path(__file__).resolve().parents[1]
s=(R/'firmware/src/app/beaver_bitmap.h').read_text()
data=list(map(int,re.search(r'natural_beaver\[\d+\]=\{([^}]+)',s).group(1).split(',')))
pal=[tuple(map(int,m)) for m in re.findall(r'RGB\((\d+),(\d+),(\d+)\)',re.search(r'natural_beaver_pal\[16\]=\{([^}]+)',s).group(1))]
W,H=56,32
frames=[];names=[]
dark={i:min(range(1,16),key=lambda j:sum((pal[j][k]-pal[i][k]*.72)**2 for k in range(3))) for i in range(1,16)}
def pixel(im,x,y,c):
 if c and 0<=x<W and 0<=y<H:im[y*W+x]=c

def build(name,phase=None,pose='idle',groove=None,walk_accent=False):
 im=[0]*(W*H);base=13;bodydy=0;pitch=0;tail=0
 hd=fd=hl=fl=0
 if phase is not None:
  sweep=[-3,-1,2,3,3,1,-2,-3];lift=[0,0,1,3,3,1,0,0]
  hd,fd=sweep[phase],sweep[(phase+4)%8];hl,fl=lift[phase],lift[(phase+4)%8]
  bodydy=[0,-1,-1,0,0,-1,-1,0][phase];tail=[0,-1,-2,-2,-1,0,0,-1][phase]
 if pose=='breath':bodydy=-1
 if pose=='takeoff':bodydy=-1;pitch=-3;tail=2;hd=-3;hl=0;fd=-1;fl=4
 if pose=='rise':bodydy=-1;pitch=-2;tail=2;hd=2;hl=4;fd=-2;fl=4
 if pose=='apex':bodydy=-1;tail=1;hd=1;hl=3;fd=-1;fl=3
 if pose=='fall':bodydy=-1;pitch=1;tail=-2;hd=-1;hl=2;fd=2;fl=0
 if pose=='land':bodydy=2;hd=-2;fd=2;tail=0
 if pose=='settle':bodydy=1;hd=-1;fd=1
 if pose=='crouch':bodydy=2;hd=-2;fd=2;hl=fl=0
 if groove is not None:
  bodydy,pitch,tail=[(1,1,1),(-3,-3,-2),(-1,-1,-3),(0,0,0),(-4,-4,2)][groove]
 if walk_accent:tail-=1
 def body_y(sx,sy):return base+sy+bodydy+int((sx-25)*pitch/24)-(1 if walk_accent and sx>=33 else 0)
 def stroke(a,b,width,c):
  # Authored joints/limbs, not stretched pixels cut from the source bitmap.
  n=max(abs(b[0]-a[0]),abs(b[1]-a[1]),1)
  for z in range(n+1):
   xx=round(a[0]+(b[0]-a[0])*z/n);yy=round(a[1]+(b[1]-a[1])*z/n)
   for ox in range(-(width//2),(width+1)//2):pixel(im,xx+ox,yy,c)
 def leg(fore,dx,lift,far=False):
  hipx=41 if fore else 28
  hip=(hipx+(2 if far else 0),26+bodydy)
  paw=(hipx+dx+(3 if far else 0),31-lift)
  joint=(hip[0]+( -1 if fore else 2)+dx//3, min(29+bodydy,paw[1]-1))
  colors=(13,12,10) if far else (12,7,3)
  stroke(hip,joint,5 if not fore else 4,colors[0])
  stroke((hip[0]-1,hip[1]),(joint[0]-1,joint[1]),3,colors[1])
  stroke(joint,paw,3,colors[0]);stroke((joint[0]-1,joint[1]),(paw[0]-1,paw[1]),1,colors[2])
  # Short flattened paws project forward, with one muted claw highlight.
  for xx in range(paw[0]-2,paw[0]+4):pixel(im,xx,paw[1],13 if far else 12)
  for xx in range(paw[0]-1,paw[0]+3):pixel(im,xx,paw[1]-1,colors[1])
  pixel(im,paw[0]+3,paw[1],10)
 # Far legs show behind the body and run in the opposite gait phase.
 if phase is not None:
  leg(False,-hd,fl,True);leg(True,-fd,hl,True)
 # Tail follows the stride one beat behind the body; tip stays near the floor.
 for sy in range(10,19):
  for sx in range(19):
   c=data[sy*50+sx];yy=base+sy+int(tail*(18-sx)/18)
   pixel(im,sx+3,min(31,yy),c)
 # Preserve head, muzzle, fur and body proportions; only a restrained pitch/bob.
 for sy in range(16):
  for sx in range(16,50):pixel(im,sx+3,body_y(sx,sy),data[sy*50+sx])
 leg(False,hd,hl);leg(True,fd,fl)
 names.append(name);frames.append(im)

def jump(name,angle,bodydy,hind,fore,tail_tip):
 """Original rigid torso articulation and explicitly drawn short-leg joints."""
 im=[0]*(W*H);a=math.radians(angle);cs,sn=math.cos(a),math.sin(a)
 def joint(sx,sy):
  dx,dy=sx-28,sy-13
  return (round(31+dx*cs-dy*sn),round(26+bodydy+dx*sn+dy*cs))
 def stroke(a,b,width,color):
  n=max(abs(b[0]-a[0]),abs(b[1]-a[1]),1)
  for z in range(n+1):
   xx=round(a[0]+(b[0]-a[0])*z/n);yy=round(a[1]+(b[1]-a[1])*z/n)
   for ox in range(-(width//2),(width+1)//2):pixel(im,xx+ox,yy,color)
 def leg(front,points,far=False):
  hip=joint(41 if front else 28,13);elbow,paw=points
  if far:hip=(hip[0]+2,hip[1]);elbow=(elbow[0]+2,elbow[1]);paw=(paw[0]+2,paw[1]-1)
  colors=(13,12,10) if far else (12,7,3)
  stroke(hip,elbow,4 if front else 5,colors[0]);stroke((hip[0]-1,hip[1]),(elbow[0]-1,elbow[1]),2,colors[1])
  stroke(elbow,paw,3,colors[0]);stroke((elbow[0]-1,elbow[1]),(paw[0]-1,paw[1]),1,colors[2])
  for xx in range(paw[0]-2,paw[0]+4):pixel(im,xx,paw[1],13 if far else 12)
  for xx in range(paw[0]-1,paw[0]+3):pixel(im,xx,paw[1]-1,colors[1])
  pixel(im,paw[0]+3,paw[1],10)
 leg(False,hind,True);leg(True,fore,True)
 root_shift=joint(18,15)[1]-28
 for sy in range(10,19):
  for sx in range(19):
   yy=13+sy+round((root_shift*sx+tail_tip*(18-sx))/18)
   pixel(im,sx+3,min(31,yy),data[sy*50+sx])
 # Inverse mapping keeps the fur, small eye and muzzle intact without holes.
 for yy in range(H):
  for xx in range(W):
   dx,dy=xx-31,yy-(26+bodydy)
   sx=round(28+dx*cs+dy*sn);sy=round(13-dx*sn+dy*cs)
   if 16<=sx<50 and 0<=sy<16:pixel(im,xx,yy,data[sy*50+sx])
 leg(False,hind);leg(True,fore)
 names.append(name);frames.append(im)

build('IDLE')
for i in range(8):build('WALK_'+str(i),phase=i)
jump('COIL',-4,2,((30,30),(27,31)),((40,29),(44,31)),0)
jump('TAKEOFF',-23,-3,((27,27),(24,31)),((42,21),(43,24)),2)
jump('RISE',-27,-4,((28,26),(33,28)),((40,20),(42,22)),1)
jump('APEX',-6,-4,((28,25),(32,28)),((40,23),(41,26)),-2)
jump('FALL',12,-4,((27,26),(30,30)),((44,28),(47,31)),-5)
jump('LAND',2,3,((29,30),(26,31)),((43,30),(48,31)),0)
for pose in ['settle','crouch','breath']:build(pose.upper(),pose=pose)
for i in range(4):build('GROOVE_'+str(i),groove=i)
build('GROOVE_ACCENT',groove=4)
for i in range(8):build('WALK_ACCENT_'+str(i),phase=i,walk_accent=True)
lines=['/* SPDX-License-Identifier: GPL-3.0-only */','/* Original pose data generated by tools/animate_beaver.py from the retained animal. */','#pragma once',f'#define BEAVER_ANIM_W {W}',f'#define BEAVER_ANIM_H {H}',f'#define BEAVER_ANIM_BYTES {W*H//2}',f'#define BEAVER_ANIM_COUNT {len(frames)}','enum {'+', '.join('BA_'+name for name in names)+'};',f'static const uint8_t beaver_anim[{len(frames)}][BEAVER_ANIM_BYTES]={{']
for f in frames:lines.append('{'+','.join(str(f[i]<<4|f[i+1]) for i in range(0,len(f),2))+'},')
weapon={'IDLE':0,'COIL':1,'TAKEOFF':-7,'RISE':-9,'APEX':-5,'FALL':0,'LAND':2,'SETTLE':1,'CROUCH':2,'BREATH':-1,'GROOVE_0':1,'GROOVE_1':-3,'GROOVE_2':-1,'GROOVE_3':0,'GROOVE_ACCENT':-4}
for i in range(8):weapon['WALK_'+str(i)]=[0,-1,-1,0,0,-1,-1,0][i];weapon['WALK_ACCENT_'+str(i)]=weapon['WALK_'+str(i)]-1
lines+=['};','static const int8_t beaver_weapon_dy[BEAVER_ANIM_COUNT]={'+','.join(str(weapon[n]) for n in names)+'};']
(R/'firmware/src/app/beaver_animation.h').write_text('\n'.join(lines)+'\n')
out=R/'build/test/animation-724';out.mkdir(parents=True,exist_ok=True)
canvas=Image.new('RGB',(688,((len(frames)+3)//4)*114+8),(27,35,44));d=ImageDraw.Draw(canvas)
for i,(name,f) in enumerate(zip(names,frames)):
 im=Image.new('RGBA',(W,H));im.putdata([(*pal[p],255 if p else 0) for p in f]);x=8+i%4*172;y=8+i//4*114
 d.text((x,y),name,fill=(225,229,220));im=im.resize((W*3,H*3),Image.Resampling.NEAREST);canvas.paste(im,(x,y+14),im)
canvas.save(out/'pose-sheet.png')
meta={'generator':'tools/animate_beaver.py','source':'firmware/src/app/beaver_bitmap.h','origin':'Original hip/elbow/hock/paw articulation and rigid torso rotation around the retained generated naturalistic animal; no new external image inputs. Torso proportions retained without squash scaling.','frames':names,'size':[W,H],'format':'4-bit palette indices, two pixels per byte; color index zero transparent','bytes':len(frames)*W*H//2,'sole_row':31,'accepted_walk_preservation':'Idle and8 original distance-gait frames are unchanged pixel-for-pixel apart from8 transparent rows added above; the common ground-foot anchor remains fixed.','gait':'8 phases per40 world pixels travelled; beat accents retain identical leg phases and paw pixels.','jump':'First airborne coil,23-degree upward hind-leg push,27-degree rising torso and lifted forepaws, tucked apex,12-degree descending extension, splayed compressed landing and settle. No physics changes or input delay.','groove':'Four torso/chest/head weight-shift frames plus bar accent, planted soles, original paddle movement; eight restrained upper-body walk accents.','shadow_audit':'Retained bitmap contains textured paddle, fur, articulated paw/sole pixels and transparent background; no separate baked ground-shadow shape was removed.'}
(R/'assets/generated/animation-provenance.json').write_text(json.dumps(meta,indent=2)+'\n')
print(f'{len(frames)} original poses; {meta["bytes"]} packed bytes; grounded sole at y31.')
