/* SPDX-License-Identifier: GPL-3.0-only
 * Original rounded mother-machine artwork. No conveyor or decorative cogs.
 * Arena ledges stay left of the machine silhouette beginning at x=430.
 * Rotor heads and the belly hatch share the rounded body center. */
static void mother_metal(int x,int y,int w,int h,int alive,int flash){
 /* Curved plate, lit from upper left. Short integer bands avoid a flat fill. */
 for(int yy=0;yy<h;yy++){
  int ry=2*yy-h+1,hh=h*h,area=(hh-ry*ry)*w*w,lo=0,hi=w;
  while(lo<hi){int mid=(lo+hi+1)/2;if(mid*mid*hh<=area)lo=mid;else hi=mid-1;}
  int left=(w-lo)/2,right=left+lo;
  for(int xx=left;xx<right;xx+=6){
   int shade=30-xx*26/w-yy*20/h;
   int edge=imin(xx-left,right-xx);
   if(edge<4)shade-=13;
   if(yy>h/9&&yy<h/3&&xx>w/7&&xx<w/3)shade+=14;
   if(flash)shade+=24;
   if(!alive)shade/=2;
   uint16_t c=alive?RGB(56+shade,75+shade,86+shade):RGB(56+shade,65+shade,70+shade);
   rect(x+xx,y+yy,imin(6,right-xx),1,c);
  }
 }
}
static void mother_draw(int cam){
 robot_t *r=game.robots+MOTHER;
 int x=r->x-cam-64,alive=r->hp>0,dying=!alive&&game.boss_end>0;
 int elapsed=alive?0:dying?180-game.boss_end:180;
 int settle=alive?0:imin(12,imax(0,elapsed-40)/8);
 int motion=alive?tri((int)game.steps/3,16)-4:0;
 int aperture=alive?factory_hatch_aperture():0,opening=aperture>0;
 uint16_t shell=alive?(r->flash?RGB(104,128,139):RGB(65,85,98)):RGB(57,66,72);
 uint16_t trim=alive?RGB(151,171,182):RGB(87,101,104);

 /* A large bulging, rounded mechanical belly sits on two broad flat feet. */
 for(int i=0;i<2;i++){
  int foot=x+19+i*107;
  rect(foot,192,25,20,INK);rect(foot+3,195,19,13,RGB(67,87,92));
  rect(foot,209,25,3,trim);
 }
 mist(x-5,77+settle,173,130-settle,INK);
 mother_metal(x+1,83+settle,161,117-settle,alive,alive&&r->flash);
 cv_line(x+23,110+settle,x+13,143+settle,trim);
 cv_line(x+25,111+settle,x+15,144+settle,RGB(46,65,78));
 cv_line(x+143,116+settle,x+153,146+settle,RGB(36,53,66));
 rect(x+108,180,20,4,RGB(33,45,54));
 for(int i=0;i<4;i++)rect(x+109+i*5,181,2,2,alive?RGB(124,143,151):GREY);
 cv_line(x+112,153+settle,x+131,157+settle,RGB(45,63,74));
 cv_line(x+114,152+settle,x+126,154+settle,RGB(115,135,145));

 /* Armored control head with recessed narrow optics; no decorative face. */
 int head_y=55+motion+settle/2;
 ellipse(x+36,head_y,88,55,INK);ellipse(x+40,head_y+4,80,46,shell);
 rect(x+46,head_y+5,68,2,trim);rect(x+41,head_y+10,2,27,RGB(114,139,150));
 rect(x+114,head_y+10,5,35,RGB(39,54,66));
 rect(x+46,head_y+18,65,11,INK);
 for(int i=0;i<2;i++){
  int xx=x+50+i*35;
  rect(xx,head_y+22,20,3,alive?RGB(230,79,54):RGB(45,53,58));
  if(alive)rect(xx+2,head_y+22,12,1,RGB(255,160,91));
  cv_line(xx,head_y+18,xx+20,head_y+21,RGB(35,49,61));
 }
 rect(x+51,head_y+37,42,3,RGB(36,52,65));
 for(int i=0;i<4;i++)rect(x+98+i*4,head_y+35,2,7,RGB(30,44,56));

 /* Short side actuators stay clear of both the core and the belly hatch. */
 int arm=alive&&r->state==1?tri((int)game.steps/2,12):0;
 rect(x-10,128,12,46-arm,INK);rect(x-7,130,6,40-arm,trim);
 ellipse(x-16,170-arm,23,14,INK);rect(x-12,172-arm,14,3,shell);
 rect(x-6,141,4,3,RGB(56,77,89));rect(x-6,158,4,3,RGB(56,77,89));
 rect(x+158,128,12,46-arm,INK);rect(x+161,130,6,40-arm,trim);
 ellipse(x+152,170-arm,23,14,INK);rect(x+156,172-arm,14,3,shell);
 rect(x+162,141,4,3,RGB(56,77,89));rect(x+162,158,4,3,RGB(56,77,89));

 /* Bright, unobscured power core; shots from either ledge cross its face. */
 rect(x+61,130,40,47,INK);
 rect(x+65,134,32,39,alive?(r->state==1?RGB(67,69,69):RGB(163,108,55)):RGB(40,47,48));
 if(alive&&r->state!=1){
  rect(x+71,139,20,29,RGB(229,132,67));rect(x+76,142,10,23,RGB(255,215,133));
  for(int i=0;i<3;i++)rect(x+68,141+i*10,4,3,RGB(38,45,45));
 }else if(alive){
  for(int i=0;i<6;i++)rect(x+66,136+i*6,30,3,STEEL);
 }else{
  cv_line(x+68,136,x+80,153,INK);cv_line(x+80,153,x+70,169,INK);
  cv_line(x+94,136,x+82,152,INK);cv_line(x+82,152,x+94,170,INK);
 }

 /* Two iris-like shutters frame a visible rotor head before launch.
  * The opening stays clear while a live head glides out of the belly. */
 rect(x+59,175,44,37,INK);
 rect(x+61,177,40,35,opening?RGB(20,31,36):RGB(63,82,93));
 if(opening){
  int travel=imin(17,aperture),emerging=0;
  for(int i=WORKER_FIRST;i<WORKER_LAST;i++)if(game.robots[i].hp&&game.robots[i].state==FACTORY_EMERGE)emerging=1;
  if(aperture>=14&&!emerging)mech_flyer(factory_spawn_x()-cam,180,-1,0,0,0);
  rect(x+61,177,40,17-travel,trim);
  rect(x+61,194+travel,40,18-travel,trim);
  rect(x+60,177,2,35,RGB(123,146,149));rect(x+100,177,2,35,RGB(35,49,58));
  rect(x+64,177+16-travel,33,1,RGB(196,176,122));
  rect(x+64,194+travel,33,1,RGB(82,91,87));
 }else{
  rect(x+62,193,38,2,INK);
  rect(x+78,186,6,14,alive?RGB(137,132,99):RGB(76,85,81));
  rect(x+64,179,31,1,trim);rect(x+97,180,2,29,RGB(35,50,62));
 }

 if(!alive&&elapsed>40){
  /* Detached shell plates fall once and remain beside the disabled body. */
  int fall=imin(34,(elapsed-40)/2);
  rect(x+22-fall/3,166+fall,16,5,trim);
  rect(x+123+fall/4,169+fall,18,5,trim);
  cv_line(x+34,122+settle,x+54,139+settle,INK);
  cv_line(x+126,125+settle,x+107,142+settle,INK);
 }
 if(alive&&r->state==1){
  rect(x+37,35,88,4,INK);rect(x+38,36,(65-r->timer)*86/65,2,CORAL);
 }

 if(dying){
  /* Three local expanding bursts, 512 ms apart; no full-screen strobe. */
  if(elapsed<96){
   int burst_id=elapsed/32,t=elapsed%32;
   if(t<18){
    int cx=x+84+(burst_id==1?-32:burst_id==2?30:0),cy=154-burst_id*16;
    int size=8+t*2;
    ellipse(cx-size/2,cy-size/2,size,size,CORAL);
    if(t<11)ellipse(cx-size/3,cy-size/3,size*2/3,size*2/3,GOLD);
    if(t<5)ellipse(cx-4,cy-4,8,8,CREAM);
   }
  }
  /* Slow smoke follows the rupture, then ends when the machine is disabled. */
  if(elapsed>18)for(int i=0;i<3;i++){
   int drift=imin(48,(elapsed-18)/3+i*5);
   ellipse(x+51+i*17,132-drift,15+i*2,11+i,RGB(73,82,83));
  }
 }
 if(alive)label(x+14,24,"MOTHER / ASSEMBLY 01",CORAL);
}
