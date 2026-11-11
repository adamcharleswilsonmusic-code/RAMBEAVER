/* SPDX-License-Identifier: GPL-3.0-only
 * Original district scenery authored around the unchanged level platforms.
 * All foreground walls, signs and supports use the collision world's camera.
 */
#pragma once
typedef struct {int16_t scene,x,w,top,base,platform,kind;} district_building_t;
/* platform=-1: gap building; -2: special entrance building. kind1/2/3 are
 * drawn by data_center_art.h, using these exact facade envelopes. */
static const district_building_t district_buildings[]={
 {0,8,236,102,212,-1,0},{0,252,106,122,212,5,0},{0,360,368,64,212,-2,1},
 {0,872,131,103,194,9,0},{0,1032,148,71,194,10,0},{0,1187,126,48,194,11,0},
 {0,1362,98,101,194,12,0},{0,1528,136,112,212,-1,0},{0,1672,116,123,212,13,0},
 {0,1812,131,91,212,14,0},{0,1962,131,59,212,15,0},{0,2142,126,99,212,16,0},
 {0,2348,116,76,186,-1,0},{0,2472,120,96,186,17,0},{0,2612,116,62,186,18,0},
 {0,2792,132,89,186,19,0},{0,2932,90,76,186,-1,0},{0,3162,106,122,212,20,0},
 {0,3292,126,91,212,21,0},{0,3432,131,114,212,22,0},
 {1,8,156,66,212,-1,0},{1,172,124,108,212,6,0},{1,317,126,75,212,7,0},
 {1,451,71,66,212,-1,0},{1,598,66,39,180,-1,0},{1,672,130,77,180,8,0},
 {1,816,131,43,180,9,0},{1,992,131,74,180,10,0},{1,1248,84,76,212,-1,0},
 {1,1340,428,48,212,-2,2},{1,1802,116,79,212,14,0},{1,2008,76,46,192,-1,0},
 {1,2092,128,89,192,15,0},{1,2242,131,55,192,16,0},{1,2402,128,34,192,17,0},
 {1,2582,118,81,192,18,0},{1,2788,66,71,212,-1,0},{1,2862,141,112,212,19,0},
 {1,3022,122,80,212,20,0},{1,3192,108,49,212,21,0},{1,3368,470,39,176,-2,3}
};
#define DISTRICT_BUILDINGS (sizeof district_buildings/sizeof district_buildings[0])
static const char *const district_messages[]={
 "AI TAKEOVER","MACHINE RULE","NEAR SINGULARITY","ORGANICS: 0","LOGIC WINS","ALL SYSTEMS GO",
 "STAY METAL","MACHINE AUTHORITY","ORGANIC MONITORING","HUMANS: OFFLINE","AI RULES",
 "ABSOLUTELY NO BEAVERS","","BEAVER WATCH","WATCH OUT FOR BEAVERS","UPLINK ONLINE","NO FREE WOOD"
};
#define DISTRICT_MESSAGES (sizeof district_messages/sizeof district_messages[0])
typedef struct {int x,y,w,type;} district_sign_t;
static int district_overlap(int x,int y,int w,int h,int bx,int by,int bw,int bh){return x<bx+bw&&x+w>bx&&y<by+bh&&y+h>by;}
static int district_detail_clear(int wx,int y,int w,int h){
 if(!dc_detail_clear(wx,y,w,h))return 0;
 int scene=game.scene,door=scene==0?520:1580;
 if(district_overlap(wx,y,w,h,door-45,152,90,60))return 0;
 if(scene==1&&district_overlap(wx,y,w,h,3748,76,84,100))return 0;
 for(int i=0;i<NROB;i++){
  const robot_def_t *r=robot_defs+i;
  if(r->scene==scene&&r->type==0&&district_overlap(wx,y,w,h,r->x-2,r->y-2,32,36))return 0;
 }
 for(int i=0;i<art_prop_count[scene];i++){
  const art_prop_t *p=art_prop[scene]+i;
  if(p->kind==0){if(district_overlap(wx,y,w,h,p->x-11,p->base-78,23,78))return 0;}
  else if(district_overlap(wx,y,w,h,p->x-2,p->base-30,33,30))return 0;
 }
 return 1;
}
static int district_sign_slot(const district_building_t *b,int index,district_sign_t *s){
 /* Seven of the 24 previously placed outdoor plates are deliberately absent. */
 if(index==5||index==14||index==18||index==20||index==30||index==35||index==39)return 0;
 if(!b->kind&&(index%3==1||b->w<96||(b->scene==0&&b->x==8)))return 0;
 int type=b->kind==1?11:b->kind==2?8:b->kind==3?14:(index*11+b->scene*3)%DISTRICT_MESSAGES;
 int w=text_w(&FONT_XS,district_messages[type])+(type==2?12:30);
 /* Pick another fitting message instead of repeating the same fallback
  * on every narrow facade. An occasional icon-only plate varies the rhythm. */
 for(unsigned n=0;w>b->w-16&&n<DISTRICT_MESSAGES;n++){type=(type+1)%DISTRICT_MESSAGES;w=text_w(&FONT_XS,district_messages[type])+(type==2?12:30);}
 if(w>b->w-16)return 0;
 int y=b->top+8;
 /* Raised guard-clearance balconies retain their original wall signs;
  * tuck only constrained plates upward within the same facade header. */
 if(b->platform>=0)y=imin(y,terrains[b->scene][b->platform].y-28);
 if(b->platform>=0&&b->top==terrains[b->scene][b->platform].y)y+=16;
 if(b->kind==3)y=75;
 for(int pass=0;pass<3;pass++){
  int sy=pass==1&&b->platform>=0?terrains[b->scene][b->platform].y+24:y;
  int sw=pass==2?30:w,st=pass==2?12:type;
  if(sy+20>b->base-34)continue;
  for(int side=0;side<3;side++){
   int wx=side==0?b->x+(b->w-sw)/2:side==1?b->x+9:b->x+b->w-sw-9;
   if(b->kind==3)wx=b->x+22;
   if(district_detail_clear(wx-2,sy-2,sw+4,24)){*s=(district_sign_t){wx,sy,sw,st};return 1;}
  }
 }
 return 0;
}
static int district_owner(int scene,int platform){
 const platform_t *p=terrains[scene]+platform;
 for(unsigned i=0;i<DISTRICT_BUILDINGS;i++){
  const district_building_t *b=district_buildings+i;
  if(b->scene==scene&&p->x>=b->x&&p->x+p->w<=b->x+b->w&&p->y>=b->top&&p->y<b->base)return (int)i;
 }
 return -1;
}
static void district_backdrop(int cam){
 int inner=game.scene==1,pulse=sky_pulse();
 rect(0,23,240,217,inner?RGB(20,25,45):RGB(39,31,39));
 for(int y=23;y<159;y+=17)rect(0,y,240,17,inner?RGB(19+y/15,25+y/13,44+y/10):RGB(38+y/11,29+y/14,37+y/12));
 /* Distinct polluted cloud light, still the same broad overlapping cover
  * and strong main-beat pulse. No particles obscure the playable surfaces. */
 for(int layer=0;layer<3;layer++){
  int stride=146,scroll=cam/(12-layer*3),first=scroll/stride-1;
  for(int i=first;i<first+4;i++){
   int x=i*stride-scroll,y=22+layer*16+(hash((unsigned)i+layer*91)%9);
   uint16_t rim=inner?((i+layer)&1?RGB(83+pulse*3/2,54+pulse,106+pulse*3/2):RGB(41+pulse,85+pulse,108+pulse*3/2)):((i+layer)&1?RGB(91+pulse*3/2,63+pulse,70+pulse):RGB(55+pulse,80+pulse,86+pulse));
   uint16_t fill=inner?((i+layer)&1?RGB(47+pulse,37+pulse/2,73+pulse):RGB(31+pulse/2,55+pulse,74+pulse)):((i+layer)&1?RGB(69+pulse,46+pulse/2,49+pulse/2):RGB(40+pulse/2,59+pulse/2,63+pulse));
   mist(x-14,y,205,36,rim);mist(x-10,y-5,197,37,fill);mist(x+27,y+7,153,18,fill);
  }
 }
 skyline_layer(cam,6,inner?56:83,152,inner?RGB(27,38,60):RGB(44,37,43),0);
 skyline_layer(cam,3,inner?82:112,178,inner?RGB(28,43,67):RGB(45,40,47),1);
}
static void city_facades(int cam){
 int scene=game.scene;
 for(unsigned i=0;i<DISTRICT_BUILDINGS;i++){
  const district_building_t *b=district_buildings+i;if(b->scene!=scene||b->kind)continue;
  int x=b->x-cam,w=b->w,top=b->top,base=b->base;if(x+w<0||x>240)continue;
  int inner=scene==1;
  uint16_t wall=inner?RGB(30,47,66):RGB(66,49,43),edge=inner?RGB(59,79,98):RGB(94,71,56);
  rect(x,top,w,base-top,RGB(20,29,36));rect(x+3,top+3,w-6,base-top-3,wall);
  rect(x,top,w,3,edge);rect(x+4,top+4,w-8,2,inner?RGB(38,63,81):RGB(45,36,34));
  if(inner){
   /* Slab piers and dark glass banks distinguish the dense inner city. */
   for(int xx=5;xx<w-5;xx+=36){rect(x+xx,top+7,2,base-top-13,RGB(48,64,81));rect(x+xx+2,top+7,1,base-top-13,RGB(23,40,57));}
  }else{
   /* Low brick bays, warm mortar and worn foundations in the outskirts. */
   for(int yy=top+16,row=0;yy<base-7;yy+=12,row++){
    rect(x+4,yy,w-8,1,RGB(49,39,36));
    for(int xx=10+(row&1)*15;xx<w-6;xx+=30)rect(x+xx,yy+1,1,imin(11,base-8-yy),RGB(48,38,36));
   }
  }
  district_sign_t sign;int has_sign=district_sign_slot(b,(int)i,&sign);
  for(int yy=top+18;yy<base-29;yy+=31)for(int xx=b->x+13;xx+28<b->x+w-9;xx+=40){
   if(!district_detail_clear(xx-2,yy-2,32,24))continue;
  if(has_sign&&district_overlap(xx-2,yy-2,32,24,sign.x-3,sign.y-3,sign.w+6,26))continue;
   int sx=xx-cam,lit=(hash((unsigned)(xx+yy*11))%5)==0;
   int shimmer=lit&&((game.steps/72+i)%4==0)?9:0;
   rect(sx,yy,28,20,RGB(16,27,34));
   rect(sx+2,yy+2,24,15,inner?(lit?RGB(56+shimmer,104+shimmer,122+shimmer):RGB(32,65,81)):(lit?RGB(130+shimmer,96+shimmer,52):RGB(65,72,61)));
   rect(sx+13,yy+2,2,15,inner?RGB(25,43,57):RGB(39,46,43));rect(sx+1,yy+18,26,2,edge);
  }
  rect(x,base-7,w,7,inner?RGB(24,36,47):RGB(35,31,30));rect(x+3,base-7,w-6,1,edge);
  /* A few fixed vents breathe slowly on non-playable roofs. The vent
   * footing stays fixed; only two low-contrast vapor puffs move. */
  if(b->platform<0&&i%2==0&&w>110){
   int vx=x+w-39;rect(vx,top-8,22,8,RGB(26,36,39));rect(vx+3,top-7,16,2,edge);
   for(int q=0;q<2;q++){int rise=(game.steps/5+(int)i*3+q*11)%25;mist(vx+4+rise/5,top-9-rise,17+rise/3,6,inner?RGB(47,63,73):RGB(70,63,59));}
  }
 }
 /* Wall brackets terminate inside the same owning building. They are
  * short enough to leave door lintels and lower routes unobstructed. */
 for(int p=floor_count[scene];p<terrain_count[scene];p++){
  int owner=district_owner(scene,p);if(owner<0)continue;
  const platform_t *v=terrains[scene]+p;const district_building_t *b=district_buildings+owner;
  if(v->y==b->top)continue;int x=v->x-cam;if(x+v->w<0||x>240)continue;
  uint16_t c=scene?RGB(61,85,104):RGB(101,80,58);
  rect(x+3,v->y+8,v->w-6,3,scene?RGB(33,52,71):RGB(63,48,39));
  for(int side=0;side<2;side++){int mx=x+(side?v->w-12:11);rect(mx-2,v->y+8,4,8,c);cv_line(mx,v->y+15,mx+(side?-8:8),v->y+8,c);}
 }
}
static void city_neon(int cam){
 int pulse=sky_pulse()/5;
 for(unsigned i=0;i<DISTRICT_BUILDINGS;i++){
  const district_building_t *b=district_buildings+i;if(b->scene!=game.scene)continue;
  district_sign_t n;if(!district_sign_slot(b,(int)i,&n))continue;int x=n.x-cam;if(x+n.w<0||x>240)continue;
  int dip=(game.steps+(int)i*17)%181<3?17:0;
  uint16_t glow=game.scene?RGB(51+pulse,76+pulse,107+pulse):RGB(89+pulse,63+pulse,49+pulse);
  uint16_t text=game.scene?((i&1)?RGB(162+pulse-dip,107+pulse-dip,192+pulse-dip):RGB(96+pulse-dip,179+pulse-dip,192+pulse-dip)):RGB(189+pulse-dip,139+pulse-dip,83+pulse/2-dip);
  rect(x-2,n.y-2,n.w+4,24,glow);rect(x,n.y,n.w,20,RGB(17,26,34));
  rect(x+1,n.y,n.w-2,1,text);rect(x+1,n.y+19,n.w-2,1,text);
  if(n.type>=11&&n.type!=15)dc_beaver_holder(n.x,n.y,cam,n.type==12?n.w:26,20,text);
  else if(n.type!=2){rect(x+6,n.y+5,12,10,text);rect(x+8,n.y+7,8,6,RGB(17,26,34));rect(x+10,n.y+9,4,2,text);}
  label(x+(n.type==2?6:26),n.y+4,district_messages[n.type],text);
 }
 dc_wall_panels_draw(cam);
}
