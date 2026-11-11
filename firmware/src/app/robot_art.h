/* SPDX-License-Identifier: GPL-3.0-only
 * Original native humanoid security-machine artwork, 2026-10-13.
 * Articulated biped guards/workers and rotor-borne humanoid heads.
 * Collision, movement and attack timing are owned by game.c. */
#pragma once
static void mech_joint(int x,int y,uint16_t metal,uint16_t edge){
 rect(x-2,y-1,5,3,INK);rect(x-1,y-2,3,5,INK);
 rect(x-1,y-1,3,3,metal);rect(x-1,y-1,2,1,edge);rect(x,y+1,2,1,RGB(29,41,50));
}
static void mech_link(int x0,int y0,int x1,int y1,uint16_t metal,uint16_t edge){
 for(int n=-2;n<=2;n++)cv_line(x0+n,y0,x1+n,y1,INK);
 for(int n=-1;n<=1;n++)cv_line(x0+n,y0,x1+n,y1,metal);
 cv_line(x0-1,y0,x1-1,y1,edge);
}
static void mech_walker(int x,int y,int dir,int phase,int worker,int flash){
 static const int stride[8]={3,2,0,-2,-3,-2,0,2};
 static const int lift[8]={0,0,0,0,0,1,2,1};
 int p=phase<0?0:phase&7,moving=phase>=0;
 int a=moving?stride[p]*dir:-2,b=moving?stride[(p+4)&7]*dir:2;
 int la=moving?lift[p]:0,lb=moving?lift[(p+4)&7]:0;
 uint16_t hi=flash?C_WHITE:RGB(178,193,190),mid=flash?C_WHITE:RGB(104,125,133);
 uint16_t low=flash?C_WHITE:RGB(61,78,91),shade=flash?C_WHITE:RGB(34,45,56);
 /* Long separate thighs/shins. One planted sole always ends at y+31. */
 int hips[2]={x+10,x+18},offs[2]={a,b},ups[2]={la,lb};
 for(int j=0;j<2;j++){
  int knee=hips[j]+offs[j]/2,ankle=hips[j]+offs[j],up=ups[j];
  mech_link(hips[j],y+20,knee,y+25-up,j?low:mid,j?mid:hi);
  mech_joint(knee,y+25-up,shade,mid);
  mech_link(knee,y+26-up,ankle,y+29-up,low,mid);
  rect(ankle-3,y+29-up,7,3,INK);rect(ankle-2,y+29-up,5,1,mid);
  rect(ankle+(dir>0?0:-2),y+30-up,4,1,low);
 }
 /* Far arm hangs in a deliberately uneven, almost human posture. */
 int back=x+14-dir*7,elbow=back-dir,hand=back+dir;
 mech_link(back,y+11,elbow,y+17,shade,low);
 mech_link(elbow,y+17,hand,y+22,low,mid);mech_joint(elbow,y+17,shade,mid);
 rect(hand-2,y+21,4,4,INK);rect(hand-1,y+21,2,3,mid);rect(hand+1,y+23,1,2,low);
 /* Sloped shoulders taper to a narrow abdomen and separated hip sockets. */
 rect(x+6,y+9,17,5,INK);rect(x+8,y+12,13,6,INK);
 rect(x+8,y+10,12,3,mid);rect(x+8,y+10,3,2,hi);rect(x+19,y+11,3,5,shade);
 rect(x+9,y+13,10,5,low);rect(x+10,y+13,3,4,mid);
 rect(x+13,y+12,2,5,shade);rect(x+10,y+17,8,2,INK);
 rect(x+12,y+18,5,3,shade);rect(x+12,y+18,2,2,hi);
 rect(x+8,y+20,13,3,INK);rect(x+9,y+20,10,2,low);rect(x+10,y+20,3,1,mid);
 if(worker){rect(x+8,y+10,3,1,GOLD);rect(x+18,y+10,2,1,RGB(143,113,55));}
 /* Small canted head on visible neck pistons, never a broad toy face. */
 int head=x+14+dir;
 rect(head-2,y+7,4,3,INK);rect(head-1,y+7,1,3,mid);
 rect(head-3,y,6,8,INK);rect(head-4,y+2,8,5,INK);
 rect(head-2,y+1,4,1,hi);rect(head-3,y+2,5,2,mid);
 rect(head+2,y+2,1,4,shade);rect(head-2,y+5,5,2,low);
 rect(head-3,y+4,6,2,INK);rect(head+(dir>0?0:-3),y+4,3,1,flash?C_WHITE:CORAL);
 rect(head+(dir>0?2:-3),y+4,1,1,flash?C_WHITE:RGB(255,177,127));
 /* Near shoulder, bent elbow, hand and compact attached muzzle are distinct. */
 int shoulder=x+14+dir*7,near_elbow=shoulder+dir,grip=x+14+dir*9;
 mech_joint(shoulder,y+11,low,hi);mech_link(shoulder,y+12,near_elbow,y+17,low,hi);
 mech_joint(near_elbow,y+17,shade,mid);mech_link(near_elbow,y+17,grip,y+18,low,mid);
 rect(grip-1,y+17,3,4,INK);rect(grip,y+17,1,2,hi);
 int muzzle=dir>0?x+24:x;
 rect(muzzle,y+16,4,3,INK);rect(muzzle,y+16,4,1,mid);
 rect(dir>0?x+27:x,y+17,1,1,RGB(13,19,25));
}
/* Projectile contract: mouth center is (x+12+2*dir,y+18).
 * Rotor stays at/under y, leaving existing tell markers above the head clear. */
static void mech_flyer(int x,int y,int dir,int state,int timer,int flash){
 uint16_t hi=flash?C_WHITE:RGB(176,192,195),mid=flash?C_WHITE:RGB(102,126,142);
 uint16_t low=flash?C_WHITE:RGB(50,67,84),shade=flash?C_WHITE:RGB(28,39,52);
 int spin=(game.steps/2)&3,opening=1;
 if(state==1)opening=1+(60-imin(60,imax(0,timer)))*5/60;
 else if(state==2)opening=6;
 else if(state==3)opening=1+imin(70,imax(0,timer))*5/70;
 /* Obvious rotating two-blade lift assembly and mast; no body or legs. */
 rect(x+12,y+1,3,5,INK);rect(x+13,y+2,1,3,mid);
 int blade=spin&1?8:13;
 cv_line(x+13-blade,y+1,x+13+blade,y+1,low);
 cv_line(x+13-blade,y,x+13+blade,y+(spin==3?2:0),hi);
 if(spin==2)cv_line(x+8,y+2,x+18,y,mid);
 rect(x+11,y,5,3,INK);rect(x+12,y,3,1,hi);
 /* Long faceted human-like cranium, brow, nose bridge and hinged lower jaw. */
 rect(x+6,y+5,16,19,INK);rect(x+4,y+8,20,13,INK);
 rect(x+8,y+4,12,3,INK);rect(x+8,y+5,10,2,hi);
 rect(x+6,y+7,14,5,mid);rect(x+6,y+8,3,8,hi);
 rect(x+20,y+7,3,14,shade);rect(x+9,y+11,11,10,low);
 rect(x+3,y+12,3,7,INK);rect(x+3,y+13,1,4,mid);
 rect(x+23,y+12,3,7,INK);rect(x+23,y+13,1,4,low);
 /* Recessed, asymmetrical narrow optics under heavy angled brow plates. */
 rect(x+7,y+10,6,3,INK);rect(x+16,y+10,5,3,INK);
 rect(x+8,y+11,4,1,flash?C_WHITE:CORAL);rect(x+17,y+11,3,1,flash?C_WHITE:CORAL);
 rect(x+8,y+10,3,1,low);rect(x+18,y+10,3,1,shade);
 rect(x+13+dir,y+11,2,5,mid);rect(x+14+dir,y+13,1,3,hi);
 rect(x+10,y+14,2,2,shade);rect(x+19,y+14,2,2,shade);
 int mouth=x+12+dir*2,my=y+18-opening/2;
 rect(mouth-5,my-1,11,opening+2,shade);rect(mouth-4,my,9,opening,INK);
 rect(mouth-4,my-1,7,1,mid);
 if(state==1&&timer<25)rect(mouth-1,y+18,3,1,RGB(183,77,65));
 if(state==2){rect(mouth-2,y+17,5,3,CORAL);rect(mouth-1,y+18,3,1,CREAM);}
 rect(x+8,y+23+opening/3,12,3,INK);rect(x+9,y+23+opening/3,9,1,mid);
 rect(x+10,y+24+opening/3,7,1,low);
}
