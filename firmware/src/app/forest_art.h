/* SPDX-License-Identifier: GPL-3.0-only
 * Original code-drawn woodland, builders and fictional river diversion. */
#pragma once
static void forest_tree(int wx,int cam,int top,int h,int w,int front){woodland_tree(wx-cam,top,h,w,front,hash((unsigned)wx*37u+(unsigned)top*11u+(unsigned)h));}
typedef struct {int x,base,h,w;} forest_stand_t;
static forest_stand_t forest_stand(int near,int i){
 static const int far_x[]={1460,1491,1573,1610,1628,1717,1772,1798,1870,1904,1976,2058,2091,2167,2190,2269};
 static const int near_x[]={1506,1571,1605,1702,1774,1810,1889,1931,2118,2184,2259};
 unsigned seed=hash((unsigned)i*317u+(near?921u:39u));
 forest_stand_t t={near?near_x[i]:far_x[i],near?135+(int)(seed%43):52+(int)(seed%35),near?112+(int)((seed>>6)%74):56+(int)((seed>>6)%52),near?38+(int)((seed>>13)%33):24+(int)((seed>>13)%19)};
 if(!near&&(t.x==1976||t.x==2058))t.base=60; /* River-bank ground horizon. */
 return t;
}
static void forest_sky(int cam,int w){
 rect(0,-103,w,361,RGB(144,184,193));
 int wide=forest_wide_view(),scroll=cam/7+(int)game.steps/64,first=scroll/210-1;
 for(int i=first;i<first+5;i++){
  unsigned h=hash((unsigned)i+731u);int x=i*210-scroll,y=(wide?-80:26)+(int)(h%14),cw=54+(int)((h>>7)%28);
  mist(x,y+5,cw,12,RGB(203,222,212));mist(x+cw/5,y,cw/2,16,RGB(218,231,217));
 }
 /* An occasional two/three-bird flock, behind the canopy, never HUD or
  * combat particles. Active time alone controls its path and wing beat. */
 int phase=game.steps%2000;if(game.scene==FOREST_SCENE&&phase>=100&&phase<270){
  int x=(phase-100)*(w+24)/170-12,y=wide?-94:27,wing=(game.steps/8&3)-1;
  for(int k=0;k<2;k++){cv_line(x-4,y+wing+k,x,y+2+k,RGB(49,76,80));cv_line(x,y+2+k,x+4,y+wing+k,RGB(49,76,80));}
 }
}
static void forest_backdrop(int cam){
 int w=(int)cv_w;forest_sky(cam,w);
 /* The raised river-bank horizon belongs only to the separate wide arena. */
 if(forest_wide_view()){
  rect(0,60,w,198,RGB(112,140,99));
  for(int near=0;near<2;near++)for(int n=0;n<(near?11:16);n++){
   forest_stand_t t=forest_stand(near,n);int half=t.base<60?0:5+(t.base-60)*45/152;
   if(iabs(t.x-2016)<t.w/2+half+9)continue;
   ellipse(t.x-cam-t.w/4,t.base-2,t.w/2,4,near?RGB(85,119,78):RGB(107,136,94));
   forest_tree(t.x,cam,t.base-t.h,t.h,t.w,near);
  }
  return;
 }
 /* Cell jitter depends only on world index: no random draw state or camera
  * reshuffling, and no green layer painted over the lower trunks. */
 rect(0,100,w,158,RGB(98,135,100));rect(0,192,w,66,RGB(59,96,61));
 int far=cam/5,first=far/96-1;
 for(int i=first;i<first+8;i++){unsigned h=hash((unsigned)i+39u);int base=183+(int)(h%24),height=122+(int)((h>>7)%57);forest_tree(i*96+(int)((h>>14)%61)-30,far,base-height,height,48+(int)((h>>20)%29),0);}
 int near=cam/2;first=near/145-1;
 for(int i=first;i<first+6;i++){unsigned h=hash((unsigned)i+379u);int base=201+(int)(h%15),height=151+(int)((h>>7)%59);forest_tree(i*145+(int)((h>>14)%85)-42,near,base-height,height,46+(int)((h>>20)%32),1);}
}
static void forest_platform(const platform_t *p,int floor,int cam){
 int x=p->x-cam;if(x+p->w<0||x>(int)cv_w)return;
 if(!floor){
  if(p==terrains[FOREST_SCENE]+terrain_count[FOREST_SCENE]-1){if(!game.forest_done)return;for(int n=0;n<p->w;n+=13){rect(x+n,p->y,12,8,RGB(126,82,44));rect(x+n,p->y,12,2,RGB(186,139,76));}return;}
  int base=floor_at(FOREST_SCENE,p->x+p->w/2);
  rect(x+8,p->y+5,7,base-p->y,RGB(82,78,48));rect(x+p->w-16,p->y+5,6,base-p->y,RGB(85,81,50));
  rect(x,p->y,p->w,8,RGB(90,61,38));rect(x,p->y,p->w,2,RGB(162,122,68));rect(x+3,p->y+3,p->w-6,1,RGB(119,83,45));
  ellipse(x-2,p->y,8,8,RGB(166,127,77));ellipse(x,p->y+2,4,4,RGB(101,68,41));return;
 }
 rect(x,p->y,p->w,258-p->y,RGB(74,61,42));rect(x,p->y,p->w,3,RGB(125,154,74));rect(x,p->y+3,p->w,3,RGB(68,104,54));
 /* Clip each grass mark and dirt stroke to this actual solid-ground span.
  * Rasterizing the eight-pixel diagonal preserves its original slope. */
 for(int n=0;n<p->w;n+=25){
  if(n+4<p->w)rect(x+n+4,p->y-2,1,4,RGB(159,172,92));
  for(int k=0;k<=8&&n+7+k<p->w;k++)cv_pset(x+n+7+k,p->y+9+(7*k+4)/8,RGB(101,81,48));
 }
}
static void forest_props(int cam){
 for(int i=0;i<8;i++){int wx=110+i*225+(int)(hash((unsigned)i+903u)%81)-40,fy=floor_at(FOREST_SCENE,wx),x=wx-cam;int supported=0;for(int p=0;p<floor_count[FOREST_SCENE];p++)supported|=wx>=terrains[FOREST_SCENE][p].x&&wx<terrains[FOREST_SCENE][p].x+terrains[FOREST_SCENE][p].w;if(!supported)continue;if(x< -30||x>(int)cv_w+30)continue;
  if(i&1){ellipse(x-12,fy-17,30,17,RGB(92,110,96));rect(x-4,fy-14,12,2,RGB(139,151,124));}
  else{forest_tree(wx,cam,53+(i%3)*12,fy-53-(i%3)*12,48,1);cv_line(x-3,fy-4,x-17,fy,RGB(77,74,45));}
 }
 /* Perspective river: a distant narrow channel opens toward the player.
  * Horizontal banks/ripples establish a ground plane. Only the near lip
  * pours over the front; the dam and crew occupy the middle distance. */
 int dry_front=game.forest_done?144+(game.dam_phase==2?imin(68,game.dam_tick):68):144;
 for(int y=60;y<212;y++){
  int depth=y-60,half=5+depth*45/152,cx=2016-cam;
  rect(cx-half-6,y,half*2+12,1,RGB(99,124,76));
  uint16_t water=game.forest_done&&y>=144&&y<dry_front?RGB(100,116,85):game.forest_done&&y<144?RGB(91,157,155):RGB(68,139,147);
  rect(cx-half,y,half*2,1,water);
  if(game.forest_done&&y>=144&&y<dry_front&&y%23<3)rect(cx-7,y,14,1,RGB(113,129,91));
 }
 for(int n=0;n<10;n++){
  int y=67+(n*17+(int)game.steps/4)%142,half=5+(y-60)*45/152;
  if(game.forest_done)continue;
  int x=2016-half/2+(n%3-1)*half/3-cam;rect(x,y,imax(4,half/2),1,RGB(162,202,183));
 }
 if(game.forest_done){
  /* Held upstream pool: small surface ripples, no lateral diversion. */
  for(int n=0;n<3;n++){int y=113+n*4,x=1991-cam+n*5;rect(x,y,32+(n&1)*5,1,RGB(167,206,186));}
 }
 /* A narrow foreground spill lip above the earth face, never a wall of water. */
 rect(1970-cam,212,100,46,RGB(57,66,48));
 rect(1970-cam,212,100,8,game.forest_done&&dry_front>=212?RGB(83,105,77):RGB(76,147,150));
 for(int n=0;n<6;n++){int x=1976+n*15-cam;rect(x,214+(game.steps/5+n)%4,8,1,game.forest_done&&dry_front>=212?RGB(106,129,91):RGB(178,211,188));}
 ellipse(1980-cam,227,24,11,RGB(86,93,69));ellipse(2027-cam,235,30,13,RGB(78,88,65));
 /* A large approach sign on the bank, clear of the raised log caps. */
 if(forest_wide_view())return;
 int sx=1572-cam;
 rect(sx+9,117,6,95,RGB(91,69,43));rect(sx+179,117,6,95,RGB(91,69,43));
 rect(sx,44,196,77,RGB(50,60,39));rect(sx+2,46,192,73,RGB(116,95,52));rect(sx+5,49,186,67,RGB(43,66,44));
 biglabel(sx+10,52,"AI WATER SUPPLY",CREAM);cv_text(sx+10,78,&FONT_M,"THIS WAY",GOLD);
 rect(sx+127,88,45,6,GOLD);for(int n=0;n<17;n++)rect(sx+166+n,77+n,2,29-2*n>0?29-2*n:1,GOLD);
}
static int forest_dam_logs(void){return game.forest_done?12:game.dam_progress<90?0:imin(11,1+(game.dam_progress-90)*11/(DAM_BUILD_TICKS-90));}
static void forest_mission_draw(int cam){
 if(game.scene!=FOREST_SCENE||(!forest_wide_view()&&!game.forest_done))return;
 int logs=forest_dam_logs(),working=game.dam_phase==1&&!game.dam_entry&&!game.dam_blocked;
 /* Distant construction, behind the defense lane: three logs per course. */
 for(int n=0;n<logs;n++){int x=1979+(n%3)*24-cam,y=144-(n/3)*6;rect(x,y,29,6,RGB(100,64,36));rect(x+1,y,26,2,RGB(190,145,76));ellipse(x+23,y,6,6,RGB(216,172,100));}
 for(int n=0;n<3;n++){
  int x=1923+n*17+(working?tri(game.steps+n*17,36)/4:0)-cam;
  beaver(x,129,1,working,1);
  if(working){rect(x+1,127,18,4,RGB(150,103,53));rect(x+2,127,16,1,RGB(217,174,93));}
 }
}
static int wind_clear(int wx,int y){
 if(wx>game.x/Q8-28&&wx<game.x/Q8+40&&y>game.y/Q8-8&&y<game.y/Q8+38)return 0;
 for(int i=0;i<NROB;i++)if(robot_here(i)&&game.robots[i].hp){const robot_t *r=game.robots+i;if(wx>r->x-9&&wx<r->x+39&&y>r->y-16&&y<r->y+38)return 0;}
 for(int i=0;i<terrain_count[game.scene];i++){const platform_t *p=terrains[game.scene]+i;if(wx>p->x-7&&wx<p->x+p->w+7&&y>p->y-12&&y<p->y+22)return 0;}
 for(int i=0;i<NSNACK;i++)if(snack_visible(i)&&iabs(wx-snacks[i].x)<17&&iabs(y-snacks[i].y)<20)return 0;
 for(int i=0;i<NLOG;i++)if(log_visible(i)&&iabs(wx-log_defs[i].x)<22&&iabs(y-log_defs[i].y)<22)return 0;
 if(game.scene<2){
  for(unsigned i=0;i<DISTRICT_BUILDINGS;i++)if(district_buildings[i].scene==game.scene){district_sign_t n;if(district_sign_slot(district_buildings+i,i,&n)&&district_overlap(wx-3,y-3,7,7,n.x-3,n.y-3,n.w+6,26))return 0;}
  for(unsigned i=0;i<DC_WALL_PANELS;i++){const dc_wall_panel_t *p=dc_wall_panels+i;if(p->scene==game.scene&&district_overlap(wx-3,y-3,7,7,p->x-3,p->y-3,dc_panel_width(p)+6,dc_panel_height(p)+6))return 0;}
  int door=game.scene?1580:520;if(iabs(wx-door)<48&&y>51)return 0;
 }
 if(game.scene==FOREST_SCENE&&((wx>1564&&wx<1776&&y<128)||(wx>1896&&wx<2073&&y>108&&y<158)))return 0; /* Clear builders/gauge/supply fixture. */
 return 1;
}
static void forest_leaf(int x,int y,int size,int spin,int tint){
 int half=spin==2||spin==6?1:spin==1||spin==5?size/3:size/2;
 int tall=spin&1?size/3:imax(2,size/4),tilt=spin<4?1:-1;
 uint16_t body=tint?RGB(191,156,67):RGB(158,180,78),edge=RGB(100,132,63);
 for(int row=-tall;row<=tall;row++){int width=imax(1,half*(tall+1-iabs(row))/(tall+1));rect(x-width+row*tilt/2,y+row,width*2+1,1,iabs(row)==tall?edge:body);}
 cv_line(x-half-2,y+tall+1,x+half,y-tall,edge);rect(x,y,1,2,RGB(218,194,105));
}
static void city_litter(int x,int y,int spin,int kind){
 if(kind){
  rect(x-3,y-2,7,5,RGB(187,145,93));rect(x-2,y-1,5,3,RGB(208,183,130));cv_line(x-5,y+(spin&1?1:-2),x-3,y,CREAM);cv_line(x+4,y,x+6,y+(spin&1?-2:2),CREAM);rect(x,y-1,2,3,RGB(115,148,126));
 }else{
  int fold=(spin&3)-1;rect(x-4,y-2,9,5,RGB(210,207,178));rect(x-3,y-3+fold,4,2,RGB(184,190,169));cv_line(x-3,y+2,x+1,y-2,RGB(115,139,132));rect(x+2,y+2-fold,4,2,RGB(160,173,159));
 }
}
static void wind_foreground(int cam){
 if(game.scene!=0&&game.scene!=1&&game.scene!=6&&game.scene!=BIKE_SCENE&&game.scene!=FOREST_SCENE)return;
 int span=scene_width[game.scene]+240,forest=game.scene==FOREST_SCENE;
 for(int i=0;i<(forest?20:28);i++){
  unsigned h=hash((unsigned)i+173u);int depth=(int)(h%3),wx,y;
  if(forest){wx=(i*span/20+(int)game.steps*(2+depth)/5)%span-120+tri(game.steps/(7+depth)+i*9,36)-9;y=37+(int)((h>>5)%117)+tri(game.steps/(7-depth)+i*11,42)/3-7;}
  else{wx=(i*span/28+(int)game.steps*(1+depth)/4)%span-120+tri(game.steps/(8+depth)+i*7,32)-8;y=43+(int)((h>>5)%108)+tri(game.steps/(9-depth)+i*13,36)/3-6;}
  int x=wx-cam;if(x< -12||x>(int)cv_w+12||!wind_clear(wx,y))continue;
  if(forest)forest_leaf(x,y,6+depth*3,(game.steps/(5+depth)+i*3)&7,(h>>14)&1);
  else city_litter(x,y,(game.steps/6+i)&7,(h>>14)&1);
 }
}
static void robot_burst_draw(const particle_t *p,int x,int y){
 int r=p->type==5?17:p->type==7?9:6;
 if(x+r>game.x/Q8-game.cam-24&&x-r<game.x/Q8-game.cam+36&&y+r>player_hit_top()-3&&y-r<game.y/Q8+35)return;
 for(int i=0;i<NROB;i++)if(robot_here(i)&&game.robots[i].hp){robot_t *b=game.robots+i;int bx=b->x-game.cam;if(x+r>bx-3&&x-r<bx+32&&y+r>b->y-12&&y-r<b->y+34)return;}
 if(game.scene==3&&game.robots[MOTHER].hp){int mx=game.robots[MOTHER].x-game.cam;if(x+r>mx-12&&x-r<mx+54&&y+r>82&&y-r<216)return;}
 if(p->type==5){int size=5+(12-p->life);ellipse(x-size,y-size,size*2,size*2,CORAL);rect(x-size-3,y-1,size*2+6,3,GOLD);rect(x-1,y-size-3,3,size*2+6,GOLD);ellipse(x-size/2,y-size/2,size,size,CREAM);}
 else if(p->type==6){cv_line(x-p->vx/110,y-p->vy/110,x,y,GOLD);rect(x,y,3,2,CREAM);}
 else {int size=5+(28-p->life)/3;ellipse(x-size/2,y-size/2,size,size,RGB(133,144,137));ellipse(x-size/2+1,y-size/2,size/2,size/2,RGB(181,186,158));}
}
