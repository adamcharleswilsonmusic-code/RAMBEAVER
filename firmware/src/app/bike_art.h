/* SPDX-License-Identifier: GPL-3.0-only
 * Original code-drawn motorbike, road and ten fleeing ground groups, two ramps and six harmless flying targets. */
#pragma once
static void bike_backdrop(int cam){
 rect(0,23,240,217,RGB(32,31,44));
 int scroll=cam/6;for(int i=scroll/180-1;i<scroll/180+3;i++)mist(i*180-scroll,43+(i&1)*15,95,18,RGB(64,48,58));
 skyline_layer(cam,8,67,154,RGB(40,43,58),0);skyline_layer(cam,3,94,197,RGB(27,38,49),1);
 int wall=cam/2,first=wall/116-1;
 for(int i=first;i<first+4;i++){
  unsigned seed=hash((unsigned)i+411u);int x=i*116-wall,top=75+(int)(seed%43);
  rect(x,top,108,123,RGB(26,37,46));rect(x+3,top+4,102,194-top,RGB(41,52,61));rect(x+4,top+5,2,188-top,RGB(77,75,68));
  for(int n=0;n<4;n++){int xx=x+9+n*25;rect(xx,top+18,15,52,RGB(18,29,37));rect(xx+2,top+20,3,46,n&1?RGB(77,83,75):RGB(91,63,66));rect(xx,top+72,3,19+(seed>>n&7),RGB(29,39,43));}
  rect(x+18,top+7,64,5,RGB(18,24,34));rect(x+21,top+8,57,2,i&1?RGB(141,63,71):RGB(56,127,120));
  for(int n=0;n<8;n++){unsigned z=hash(seed+(unsigned)n*39u);int yy=top+12+(int)(z%(unsigned)(183-top));rect(x+6+(int)(z>>9)%86,yy,3+(int)(z>>18&7),1,RGB(57,62,61));}
  cv_line(x+91,top+8,x+91,195,RGB(72,67,59));rect(x+89,170,5,3,RGB(95,70,52));
 }
 /* Quiet continuous footing, without lane marks, asphalt or curb clutter. */
 rect(0,198,240,14,RGB(44,49,45));rect(0,212,240,28,RGB(31,38,34));rect(0,212,240,2,RGB(83,91,77));

}
/* Nine authored rig states: two riding beats, preload, takeoff, rise,
 * apex, descent, suspension compression and recovery. Shared contacts keep
 * the paws, seat and pegs connected as the limbs and tail articulate. */
static int bike_pose_frame(void){
 if(game.bike_phase!=BIKE_RIDE&&game.bike_phase!=BIKE_EXIT)return 0;
 if(game.ground&&game.bike_jump_tick<0)return 2;
 if(game.ground&&bike_floor(game.x/Q8)<212)return 3;
 if(!game.ground){if(game.bike_jump_tick<=3)return 2;if(game.bike_jump_tick<=6)return 3;return game.vy< -220?4:game.vy>220?6:5;}
 return game.bike_land_tick>5?7:game.bike_land_tick?8:(game.steps/5)&1;
}
static int bike_front_lift(int p){(void)p;return 0;} /* Pitch belongs to the complete rig. */
static int bike_rear_lift(int p){(void)p;return 0;}
static int bike_seat_bob(int p){static const int v[9]={0,1,2,0,-1,0,0,3,1};return v[p];}
static void motorbike_local(int x,int floor,int moving){
 if(x< -64||x>280)return;int p=moving?bike_pose_frame():0,fl=bike_front_lift(p),rl=bike_rear_lift(p),bob=bike_seat_bob(p),rear=x-7,front=x+32,y=floor;
 static const int spoke_x[4]={4,3,0,-3},spoke_y[4]={0,3,4,3};
 /* Thick black tread stays planted on the same contact line. Small hubs
  * leave substantial tire mass, framed by blunt armored mudguards. */
 for(int n=0;n<2;n++){int cx=n?front:rear,wy=y-(n?fl:rl),spin=moving?(game.steps/2)%4:0,dx=spoke_x[spin],dy=spoke_y[spin];
  ellipse(cx-13,wy-26,26,26,INK);ellipse(cx-11,wy-24,22,22,RGB(36,44,47));ellipse(cx-8,wy-21,16,16,INK);ellipse(cx-6,wy-19,12,12,STEEL);ellipse(cx-4,wy-17,8,8,RGB(37,50,56));
  for(int t=0;t<5;t++){int tx=cx-10+t*5,edge=(t==0||t==4)?4:0;rect(tx,wy-25+edge,3,3,RGB(90,101,100));rect(tx,wy-4-edge,3,3,RGB(61,73,77));}
  cv_line(cx-dx,wy-13-dy,cx+dx,wy-13+dy,CREAM);cv_line(cx-dy,wy-13+dx,cx+dy,wy-13-dx,GREY);rect(cx-1,wy-14,3,3,GREY);
  for(int row=0;row<5;row++)rect(cx-10-row/2,wy-29+row,21+row,1,row?RGB(42,56,65):STEEL);
 }
 y+=bob;
 /* Continuous low chassis: a broad tank, overlapping side plates and a
  * deep belly pan, rather than exposed frame tubing. */
 for(int row=0;row<15;row++){int left=x+1-(row>8?(row-8)/2:0),right=x+20+imin(14,row*2);rect(left,y-31+row,right-left,1,row<4?RGB(91,108,111):row<10?RGB(58,75,86):RGB(34,48,60));}
 cv_line(x+1,y-31,x+21,y-31,CREAM);cv_line(x+21,y-31,x+33,y-24,STEEL);
 rect(x-5,y-24,17,5,INK);rect(x-4,y-24,15,2,RGB(114,87,64));
 for(int row=0;row<16;row++){int left=x-2+row/6,right=x+37-row/5;rect(left,y-20+row,right-left,1,row<4?RGB(72,91,101):row<12?RGB(41,59,72):RGB(23,34,44));}
 cv_line(x-2,y-20,x+33,y-20,STEEL);cv_line(x+3,y-5,x+33,y-5,GREY);
 rect(x+5,y-16,17,9,INK);for(int n=0;n<3;n++)rect(x+6,y-15+n*3,14,1,RGB(143,156,151));
 for(int n=0;n<3;n++){rect(x+3+n*11,y-19,3,3,GOLD);rect(x+24,y-12+n*2,7,1,RGB(81,102,111));}
 /* Exposed shock and fork connect the wheel hub to the tank, visible
  * above the armored nose even when the front rises on a ramp. */
 for(int n=0;n<4;n++)cv_line(x+22+n,y-32-fl,x+31+n,floor-13-fl,RGB(159,133,83));
 for(int n=0;n<5;n++)cv_line(x+24+n,y-28-fl+n*3,x+28+n,y-29-fl+n*3,CREAM);
 rect(x+30,floor-16-fl,5,6,STEEL);
 /* Solid forward-heavy wedge. The near plate fills the former open
  * triangle while a thicker, raked cutting face stays inside x+64. */
 for(int row=0;row<29;row++){
  int right=48+imin(15,row*2/3),left=29+row/7;
  uint16_t c=row<7?RGB(111,129,132):row<21?RGB(64,84,99):RGB(35,51,65);
  rect(x+left,y-34-fl+row,right-left+1,1,c);
  rect(x+right-3,y-34-fl+row,4,1,row<22?RGB(185,195,184):RGB(130,149,150));
 }
 cv_line(x+29,y-34-fl,x+48,y-34-fl,CREAM);cv_line(x+32,y-30-fl,x+49,y-30-fl,RGB(146,163,164));
 cv_line(x+34,y-7-fl,x+63,y-7-fl,INK);rect(x+58,y-9-fl,6,3,STEEL);
 for(int n=0;n<3;n++){rect(x+34+n*7,y-25-fl+n*4,3,3,GOLD);rect(x+35+n*7,y-24-fl+n*4,1,1,INK);}
 /* Small dark vents and two diagonal paint marks read as improvised armor. */
 for(int n=0;n<3;n++)rect(x+35+n*4,y-15-fl,2,5,RGB(24,37,48));
 cv_line(x+43,y-28-fl,x+48,y-23-fl,GOLD);cv_line(x+47,y-28-fl,x+52,y-23-fl,GOLD);
 /* Heavy exhaust is attached to the engine casing, clear of the road. */
 rect(x-18,y-15,27,7,INK);rect(x-17,y-14,25,4,RGB(102,118,120));rect(x-16,y-14,21,1,STEEL);rect(x-19,y-15,4,7,RGB(51,65,73));rect(x-19,y-13,2,3,INK);rect(x-9,y-15,2,6,GOLD);
 cv_line(x+21,y-29-fl,x+15,y-31-fl,STEEL);rect(x+12,y-32-fl,10,2,INK);rect(x+27,y-32-fl,5,3,GOLD);rect(x-17,y-26,5,3,CORAL);
}
/* A dedicated saddle pose uses the retained fur/head pixels. Torso rotates
 * around the hip; limbs are jointed to fixed handlebar/peg coordinates. */
static int bike_rider_active(void){return game.bike_phase==BIKE_RIDE||game.bike_phase==BIKE_EXIT||(game.bike_phase==BIKE_MOUNT&&game.bike_tick>=48)||(game.bike_phase==BIKE_DISMOUNT&&game.bike_tick<12);}
static void bike_paw(int x0,int y0,int x1,int y1,int x2,int y2){
 for(int n=0;n<3;n++){cv_line(x0+n,y0,x1+n,y1,n?hero_color(12):hero_color(7));cv_line(x1+n,y1,x2+n,y2,n?hero_color(12):hero_color(7));}
 rect(x2-1,y2,5,2,hero_color(12));rect(x2,y2,3,1,hero_color(10));
}
static void bike_rider(int x,int floor){
 int p=bike_pose_frame(),bob=bike_seat_bob(p),fl=bike_front_lift(p),hipx=x+2,hipy=floor-27+bob;
 static const int cosine[9]={236,238,239,226,222,237,242,243,238},sine[9]={99,94,91,120,127,97,83,80,94},tail[9]={0,2,3,-1,-3,-1,2,3,1};int cs=cosine[p],sn=sine[p];
 floor+=bob;
 /* Broad flattened tail angles back above the rear fender, behind the hip. */
 for(int sy=10;sy<19;sy++)for(int sx=0;sx<19;sx++){int c=natural_beaver[sy*50+sx];if(c)cv_pset(x-27+sx, floor-29+(sy-10)+(18-sx)/5+tail[p]*(18-sx)/18,hero_color(c));}
 bike_paw(x+2, floor-26,x+10,floor-18,x+10,floor-10);
 /* Inverse integer rotation preserves the small eye, muzzle and fur. */
 for(int yy=-28;yy<9;yy++)for(int xx=-17;xx<29;xx++){
  int sx=28+(xx*cs-yy*sn)/256,sy=13+(xx*sn+yy*cs)/256;
  if(sx>=16&&sx<50&&sy>=0&&sy<16){int c=natural_beaver[sy*50+sx];if(c)cv_pset(hipx+xx,hipy+yy,hero_color(c));}
 }
 for(int n=0;n<HEADBAND_POINTS;n++){int dx=headband_xy[BA_IDLE][n][0]-3-28,dy=headband_xy[BA_IDLE][n][1]-13-13;cv_pset(hipx+(dx*cs+dy*sn)/256,hipy+(-dx*sn+dy*cs)/256,RGB(228,41,45));}
 /* Near leg crosses the frame; bent foreleg reaches the real grip. */
 bike_paw(x+1,floor-25,x+(p>=4&&p<=6?8:12),floor-(p>=4&&p<=6?20:17),x+13,floor-10);
 bike_paw(x+13,floor-34,x+(p&1?13:15),floor-30-fl/2,x+17,floor-29-fl);
 rect(x+10,floor-9,7,2,STEEL);rect(x+16,floor-30-fl,6,2,INK);
 if(game.gun)weapon(x+6,floor-43,1);
}
/* One rigid rotation for every motorcycle part AND its seated rider.
 * Scratch pixels use idle space in the existing wide-forest canvas, never
 * extra RAM or mutable gameplay state. Inverse sampling avoids pixel holes. */
static int bike_pitch(void){
 if(game.bike_phase!=BIKE_RIDE&&game.bike_phase!=BIKE_EXIT)return 0;
 int x=game.x/Q8;
 if(game.ground){
  for(int n=0;n<2;n++)if(x>=bike_ramp_x[n]-32&&x<bike_ramp_x[n]+bike_ramp_w[n])return (n?24:22)*imin(39,x-bike_ramp_x[n]+32)/39;
  return 0;
 }
 int launch_x=x-3*imax(0,game.bike_jump_tick-5),ramp=0;
 for(int n=0;n<2;n++)if(launch_x>=bike_ramp_x[n]+bike_ramp_w[n]&&launch_x<bike_ramp_x[n]+bike_ramp_w[n]+8)ramp=n+1;
 int angle;
 if(game.vy<0){angle=imin(ramp==2?24:22,-game.vy*(ramp==2?24:22)/(ramp==2?1678:1428));if(!ramp)angle=angle*imin(8,imax(0,game.bike_jump_tick-3))/8;}
 else {int clearance=imax(0,bike_floor(x)-44-game.y/Q8);angle=-imin(10,game.vy/100)*imin(28,clearance)/28;}
 return angle;
}
static void bike_rotation(int angle,int *cs,int *sn){
 static const int cos_q8[25]={256,256,256,256,255,255,255,254,254,253,252,251,250,249,248,247,246,245,243,242,241,239,237,236,234};
 static const int sin_q8[25]={0,4,9,13,18,22,27,31,36,40,44,49,53,58,62,66,71,75,79,83,88,92,96,100,104};
 int a=imin(24,iabs(angle));*cs=cos_q8[a];*sn=angle<0?-sin_q8[a]:sin_q8[a];
}
static int bike_rig_floor(int floor,int moving){
 if(!moving)return floor;
 if(game.ground)return floor+bike_floor(game.x/Q8-7)-bike_floor(game.x/Q8);
 int clearance=imax(0,bike_floor(game.x/Q8)-44-game.y/Q8);
 return floor+3*imin(12,clearance)/12;
}
static void bike_rig_draw(int x,int floor,int moving,int rider){
 if(x< -100||x>300)return;
 int angle=moving?bike_pitch():0;
 if(!angle){int base=bike_rig_floor(floor,moving);if(rider)bike_rider(x,base);else motorbike_local(x,base,moving);return;}
 enum {BW=112,BH=80,BY=520,BASE=BW*BY};
 uint32_t ow=cv_w,oh=cv_h;int oo=cv_oy,oy0=cv_y0,oy1=cv_y1;
 for(int n=0;n<BW*BH;n++)cv_px[BASE+n]=0;
 cv_w=BW;cv_h=BH;cv_oy=cv_y0=BY;cv_y1=BY+BH;
 if(rider)bike_rider(32,70);else motorbike_local(32,70,moving);
 cv_w=ow;cv_h=oh;cv_oy=oo;cv_y0=oy0;cv_y1=oy1;
 int cs,sn;bike_rotation(angle,&cs,&sn);
 int ax=x-7,ay=bike_rig_floor(floor,moving)-13;
 /* Local pivot25,57 is the rear hub. Both wheels and the beaver share it. */
 for(int dy=-76;dy<=38;dy++)for(int dx=-44;dx<=110;dx++){
  int sx=25+(dx*cs-dy*sn+128)/256,sy=57+(dx*sn+dy*cs+128)/256;
  if((unsigned)sx<BW&&(unsigned)sy<BH){uint16_t c=cv_px[BASE+sy*BW+sx];if(c)cv_pset(ax+dx,ay+dy,swap16(c));}
 }
}
static void motorbike(int x,int floor,int moving){bike_rig_draw(x,floor,moving,0);}
static void bike_rider_draw(int cam){
 int x=game.x/Q8-cam;
 if(game.bike_phase==BIKE_DISMOUNT)x-=22*(12-game.bike_tick)/12;
 bike_rig_draw(x,game.y/Q8+44,game.bike_phase==BIKE_RIDE||game.bike_phase==BIKE_EXIT,1);
}
/* Fleeing civilians lean into their run, pump opposite arms/legs and glance
 * over a shoulder briefly. All poses derive from the captured game clock. */
static void bike_civilian_body(int x,int floor,int h,int id,int phase,int look){
 static const int stride[6]={-7,-4,3,7,4,-3},lift[6]={0,4,7,0,3,0};
 int bob=(phase==1||phase==4)?2:0,hy=floor-h+bob,k=stride[phase],hipy=floor-15;
 uint16_t shell=id%3==0?RGB(188,157,113):id%3==1?RGB(143,174,173):RGB(180,139,135);
 rect(x+9,hy,12,10,INK);rect(x+10,hy+1,10,7,shell);
 rect(x+(look?10:17),hy+3,4,2,look?CORAL:MINT);rect(x+(look?9:20),hy+6,3,2,STEEL);
 cv_line(x+11,hy-1,x+14,hy-4,STEEL);rect(x+13,hy-5,3,2,CORAL);
 for(int row=0;row<h-25;row++)rect(x+6+(h-25-row)/5,hy+11+row,13,1,RGB(63,87,96));
 rect(x+11,hy+12,6,3,shell);rect(x+12,hy+17,5,2,GOLD);
 for(int leg=0;leg<2;leg++){int hip=x+8+leg*7,dx=leg?-k:k,up=lift[(phase+leg*3)%6];
  for(int n=0;n<2;n++){cv_line(hip+n,hipy,hip+dx/2+n,floor-8-up,STEEL);cv_line(hip+dx/2+n,floor-8-up,hip+dx+n,floor-3-up,CREAM);}
  rect(hip+dx-2,floor-4-up,7,4,INK);
  int shoulder=x+10+leg*7,arm=leg?k:-k;cv_line(shoulder,hy+13,shoulder+arm,hy+18,STEEL);cv_line(shoulder+arm,hy+18,shoulder+arm+4,hy+12,STEEL);rect(shoulder+arm+3,hy+10,4,4,shell);
 }
}
static void bike_civilian(int x,int h,int id){
 int phase=(game.steps/3+id*2)%6,look=(game.steps+id*17)%92<16;
 /* A paired group supplies a short cluster; never a solid wall of actors. */
 int floor=bike_floor(x+game.cam+10);
 if(id%3==1)bike_civilian_body(x+25,floor,h-5,id+1,(phase+3)%6,0);
 bike_civilian_body(x,floor,h,id,phase,look);
}
static void bike_scene_draw(int cam){
 if(game.scene==BIKE_SCENE){
  for(int n=0;n<2;n++){
   int left=bike_ramp_x[n]-cam,w=bike_ramp_w[n],h=bike_ramp_h[n];
   if(left>240||left+w<0)continue;
   for(int dx=0;dx<w;dx++){int rise=dx*h/w;rect(left+dx,212-rise,1,rise,RGB(67,77,76));rect(left+dx,212-rise,1,2,RGB(176,184,151));}
   for(int dx=16;dx<w;dx+=24){int rise=dx*h/w;rect(left+dx,214-rise,3,imax(0,rise-2),RGB(30,43,43));}
  }

 }
 if(game.bike_phase==BIKE_MOUNT)motorbike(3560-cam,212,0);
 else if(game.bike_phase==BIKE_RIDE||game.bike_phase==BIKE_EXIT){int rise=168-game.y/Q8,sw=imax(18,47-rise/3);ellipse(game.x/Q8+12-cam-sw/2,212,sw,2,RGB(18,27,34));motorbike(game.x/Q8-cam,game.y/Q8+44,1);}
 else if(game.bike_phase==BIKE_DISMOUNT)motorbike(10-cam,212,0);
 else if(game.scene==0&&!game.bike_done)motorbike(3560-cam,212,0);
}
