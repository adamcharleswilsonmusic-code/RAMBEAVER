/* SPDX-License-Identifier: GPL-3.0-only
 * Original indexed pixel sprites and world-anchored scenery. All layers use
 * integer camera coordinates; visible objects retain identity across tiles. */
#include "sprites.h"
#include "beaver_bitmap.h"
#include "beaver_animation.h"
#include "scenery.h"
#define INK RGB(11,16,27)
#define CREAM RGB(245,230,198)
#define MINT RGB(124,224,178)
#define CORAL RGB(254,115,99)
#define GOLD RGB(252,200,81)
#define BLUE RGB(91,184,239)
#define STEEL RGB(130,157,172)
#define GREY RGB(114,126,137)
static const uint16_t art_palette[16]={0,INK,RGB(69,76,89),RGB(117,131,142),RGB(174,183,180),CREAM,C_WHITE,CORAL,RGB(116,74,53),RGB(81,121,151),MINT,GOLD,RGB(173,199,207),RGB(113,75,42),RGB(156,107,60),RGB(190,141,87)};
static void rect(int x,int y,int w,int h,uint16_t c){cv_rect(x,y,w,h,c);}
static void label(int x,int y,const char *s,uint16_t c){cv_text(x,y,&FONT_XS,s,c);}
static void biglabel(int x,int y,const char *s,uint16_t c){cv_text(x,y,&FONT_S,s,c);}
static void number(int x,int y,int n,uint16_t c){char b[5];int p=0;if(n>=100)b[p++]=(char)('0'+n/100%10);if(n>=10)b[p++]=(char)('0'+n/10%10);b[p++]=(char)('0'+n%10);b[p]=0;label(x,y,b,c);}
static void sprite(int x,int y,const uint8_t *px,int w,int h,int flip,int clip,uint16_t tint){if(x+w<0||x>240||y+h<23||y>239)return;for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){if(clip&&xx>=clip)continue;int p=px[yy*w+(flip?w-1-xx:xx)];if(p)cv_pset(x+xx,y+yy,tint?tint:art_palette[p]);}}
static unsigned hash(unsigned v){v^=v>>16;v*=0x45d9f3bu;v^=v>>16;return v;}
static void ellipse(int x,int y,int w,int h,uint16_t c){for(int r=0;r<h;r++){int pad=(r==0||r==h-1)?w/4:(r==1||r==h-2)?w/8:0;rect(x+pad,y+r,w-2*pad,1,c);}}
static void mist(int x,int y,int w,int h,uint16_t c){
 // Curved scanlines make smog read as clouds rather than another skyline.
 for(int r=0;r<h;r++){int ry=2*r-h+1,hh=h*h,area=(hh-ry*ry)*w*w,lo=0,hi=w;while(lo<hi){int mid=(lo+hi+1)/2;if(mid*mid*hh<=area)lo=mid;else hi=mid-1;}rect(x+(w-lo)/2,y+r,lo,1,c);}
}
static void pipe(int x,int y,int h,uint16_t c){rect(x,y,9,h,INK);rect(x+1,y,7,h,c);rect(x+2,y,2,h,STEEL);for(int z=3;z<h;z+=21){rect(x-2,y+z,13,3,INK);rect(x-1,y+z+1,11,1,c);}}
static void bin(int x,int y,int open,int scene){uint16_t c=scene==0?RGB(59,108,91):RGB(69,95,114);rect(x+3,y+4,31,27,INK);rect(x+5,y+5,27,24,c);rect(x+7,y+7,2,20,STEEL);for(int i=0;i<3;i++)rect(x+13+i*6,y+10,2,15,RGB(36,65,65));rect(x,y+2-open*5,37,5,INK);rect(x+1,y+2-open*5,35,2,MINT);rect(x+6,y+30,5,4,INK);rect(x+27,y+30,5,4,INK);if(open){rect(x+9,y-3,10,7,CREAM);rect(x+13,y-5,9,5,GOLD);}}
static int beaver_pose(void){
 if(game.dead)return BA_IDLE;
 if(game.bike_phase)return game.ground?BA_CROUCH:BA_APEX;
 if(game.flight_phase)return game.vy< -100?BA_RISE:game.vy>100?BA_FALL:BA_APEX;
 if(player_crouched())return BA_CROUCH;
 if(!game.ground){if(game.vy< -1490)return BA_COIL;if(game.vy< -1300)return BA_TAKEOFF;if(game.vy< -220)return BA_RISE;if(game.vy<=220)return BA_APEX;return BA_FALL;}
 if(game.anim_land)return game.anim_land>4?BA_LAND:BA_SETTLE;
 unsigned beat=art_beat();int groove=game.music&&!game.pause;
 if(iabs(game.vx)>100)return (groove&&beat<16384u?BA_WALK_ACCENT_0:BA_WALK_0)+game.anim_stride/(5*Q8);
 if(!groove)return BA_IDLE;
 if((art_music_step()&15u)<4u&&beat<16384u)return BA_GROOVE_ACCENT;
 return BA_GROOVE_0+(beat>>14);
}
static int beaver_pose_top(int pose){
 const uint8_t *data=beaver_anim[pose];for(int y=0;y<BEAVER_ANIM_H;y++)for(int x=0;x<BEAVER_ANIM_W;x++){int at=y*BEAVER_ANIM_W+x,p=data[at/2];if((at&1)?p&15:p>>4)return y;}return 0;
}
static int hero_tint;
static uint16_t hero_color(int p){static const uint16_t gold[16]={0,RGB(255,236,149),RGB(235,189,69),RGB(221,163,40),RGB(207,174,80),RGB(201,143,28),RGB(190,135,35),RGB(182,118,20),RGB(166,111,24),RGB(145,96,24),RGB(122,84,28),RGB(166,96,28),RGB(102,67,19),RGB(77,53,23),RGB(42,25,17),0};return game.golden?gold[p]:natural_beaver_pal[p];}
static void beaver(int x,int y,int dir,int pose,int small){
 int frame=small?(pose?BA_WALK_0+(game.steps/3)%8:BA_IDLE):pose;
 if(frame<0||frame>=BEAVER_ANIM_COUNT)frame=BA_IDLE;
 int tucked=!small&&player_crouched();int w=small?32:BEAVER_ANIM_W,h=small?14:tucked?24:BEAVER_ANIM_H;
 int origin=x-(dir>0?(small?12:22):(small?4:7)),feet=y+(small?18:32);
 const uint8_t *data=beaver_anim[frame];
 int upside=!small&&game.dead,top=upside?beaver_pose_top(frame):0;
 int chew=!small&&game.log_chop?((LOG_RESPONSE_TICKS-game.log_chop)/3)&1:0;
 for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){
  int sy=small?8+yy*24/h:tucked?yy*BEAVER_ANIM_H/h:yy,sx=xx*BEAVER_ANIM_W/w;
  if(dir<0)sx=BEAVER_ANIM_W-1-sx;
  int at=sy*BEAVER_ANIM_W+sx,p=data[at/2];p=(at&1)?p&15:p>>4;
  // A one-pixel jaw motion adds a short gnaw without changing gait or gun pose.
  if(p)cv_pset(origin+xx,feet-h+(upside?31+top-yy:yy)+(chew&&sx>=45&&sy>=BEAVER_ANIM_H-12&&sy<=BEAVER_ANIM_H-7),hero_tint?hero_color(p):natural_beaver_pal[p]);
 }
}
#include "hero_headband.h"
static void weapon(int x,int y,int dir){if(game.dead)return;int kick=game.recoil?1:0,lower=beaver_weapon_dy[beaver_pose()];if(player_crouched()){y+=4;lower=0;}int gx=dir>0?x+14-kick:x-10+kick;int visible=SPR_GUN_W*imin(game.gun,6)/6;
 // Reveal from the body toward the muzzle in either facing direction.
 for(int yy=0;yy<SPR_GUN_H;yy++)for(int xx=0;xx<SPR_GUN_W;xx++){if((dir>0?xx:SPR_GUN_W-1-xx)>=visible)continue;int p=spr_gun[yy*SPR_GUN_W+(dir<0?SPR_GUN_W-1-xx:xx)];if(p)cv_pset(gx+xx,y+18+lower+yy,art_palette[p]);}
 ellipse(dir>0?x+14:x+4,y+25+lower,5,3,RGB(156,107,60));if(game.recoil){int mx=dir>0?gx+SPR_GUN_W-1:gx-5;rect(mx,y+21+lower,6,3,GOLD);rect(mx+2,y+19+lower,2,7,GOLD);rect(mx+2,y+22+lower,3,1,CREAM);}
}
static int layer_x(int index,int camera,int divisor,int spacing){return index*spacing-camera/divisor;}
static void skyline_layer(int cam,int divisor,int spacing,int bottom,uint16_t c,int windows){int scroll=cam/divisor,first=scroll/spacing-1;for(int i=first;i<first+7;i++){int x=layer_x(i,cam,divisor,spacing),h=45+(int)(hash((unsigned)i+39)%66),w=spacing-9;rect(x,bottom-h,w,h,c);rect(x+7,bottom-h-5,w-14,5,c);if(hash((unsigned)i)&1)rect(x+w/2,bottom-h-14,2,14,c);if(windows)for(int row=0;row<h/13;row++)for(int col=0;col<w/12;col++){unsigned a=hash((unsigned)(i*71+row*9+col));rect(x+5+col*12,bottom-h+7+row*13,4,5,(a%5)?RGB(64,74,91):RGB(106,91,65));}}}
static int sky_pulse(void){
 if(!game.music||game.pause||game.dead||game.won)return 0;
 int phase=(int)art_beat();
 // One main-beat flash (2 Hz at this score's 120 BPM), decaying over 250 ms.
 // No subdivision flashes; only sky/cloud colors change.
 int p=phase<32768?(32768-phase)*90/32768:0;return p*p/90;
}
#include "data_center_art.h"
#include "district_art.h"
#include "flight_art.h"
static void bike_backdrop(int cam);
static void forest_backdrop(int cam);
static void forest_props(int cam);
static void forest_platform(const platform_t *p,int floor,int cam);
static void backdrop(int cam){
 if(game.scene==BIKE_SCENE){bike_backdrop(cam);return;}
 if(game.scene==FOREST_SCENE){forest_backdrop(cam);return;}
 if(game.scene==6){flight_backdrop(cam);return;}
 if(game.scene>=2){data_center_backdrop(cam);return;}
 district_backdrop(cam);
}
static void facade_draw(const art_box_t *p,int cam){int x=p->x-cam,base=p->y+p->h,scene=game.scene;if(x+p->w<0||x>240)return;
 uint16_t wall=scene==0?RGB(54,48,53):scene==1?RGB(37,51,65):RGB(35,51,53),seam=scene==0?RGB(66,56,60):RGB(44,64,70);
 rect(x,p->y,p->w,p->h,RGB(23,31,39));rect(x+2,p->y+3,p->w-4,p->h-3,wall);
 // Muted cornice and a foundation strictly bounded by the supporting street.
 rect(x,p->y,p->w,2,RGB(70,80,88));rect(x+3,p->y+3,p->w-6,1,seam);rect(x,base-7,p->w,7,RGB(31,38,43));rect(x+2,base-7,p->w-4,1,seam);
 for(int yy=p->y+17,row=0;yy<base-9;yy+=14,row++){rect(x+4,yy,p->w-8,1,seam);for(int xx=11+(row&1)*17;xx<p->w-5;xx+=34)rect(x+xx,yy+1,1,imin(13,base-9-yy),wall==RGB(54,48,53)?RGB(43,39,47):RGB(29,43,52));}
}
static void detail_draw(const art_detail_t *p,int cam){int x=p->x-cam,y=p->y;if(x+p->w<0||x>240)return;int scene=game.scene;
 if(p->kind==0){rect(x,y,p->w,p->h,RGB(19,30,40));rect(x+2,y+2,p->w-4,p->h-4,scene==0?RGB(39,59,62):scene==1?RGB(37,65,82):RGB(34,57,61));
  if(scene<2){rect(x+3,y+3,6,p->h-6,p->variant==0?RGB(102,93,66):RGB(51,78,88));rect(x+p->w/2,y+2,1,p->h-4,RGB(59,75,81));rect(x+1,y+p->h-2,p->w-2,2,RGB(68,81,87));}
  else {for(int j=0;j<3;j++){rect(x+5+j*7,y+4,2,p->h-8,RGB(65,86,85));rect(x+5+j*7,y+5,2,2,p->variant?RGB(84,118,111):RGB(123,110,66));}}}
 else if(p->kind==1){rect(x,y,p->w,p->h,RGB(22,33,39));rect(x+1,y,2,p->h,RGB(67,82,81));for(int yy=7;yy<p->h;yy+=25)rect(x-1,y+yy,p->w+2,2,RGB(74,85,86));}
 else if(p->kind==2){rect(x,y,p->w,p->h,RGB(25,35,43));rect(x+2,y+2,p->w-4,p->h-3,RGB(56,69,77));for(int j=5;j<p->w-3;j+=4)rect(x+j,y+4,1,p->h-6,RGB(30,44,51));}
 else if(p->kind==3){int end=x+p->w-1;rect(x-1,y-1,3,3,RGB(73,84,91));rect(end-1,y-1,3,3,RGB(73,84,91));cv_line(x,y,x+p->w/2,y+p->h-2,RGB(20,29,37));cv_line(x+p->w/2,y+p->h-2,end,y,RGB(20,29,37));}
 else {rect(x,y,p->w,p->h,RGB(28,38,44));rect(x+2,y+2,p->w-4,p->h-4,RGB(58,68,68));for(int yy=4;yy<p->h-2;yy+=3)rect(x+4,y+yy,p->w-8,1,RGB(29,41,46));}
}
static void platform_draw(const platform_t *p,int floor,int cam){int x=p->x-cam;if(x+p->w<0||x>240)return;uint16_t cap=game.scene<2?STEEL:RGB(170,171,131);rect(x,p->y,p->w,9,INK);rect(x,p->y,p->w,3,cap);rect(x+2,p->y+3,p->w-4,2,RGB(84,104,111));rect(x,p->y+7,p->w,2,RGB(44,59,67));
 for(int t=imax(0,(cam-p->x)/16-1);t<p->w/16+1;t++){int xx=x+t*16;if(xx>243)break;rect(xx+3,p->y+4,3,2,INK);if(!floor&&game.scene>=2){rect(xx+5,p->y,7,3,GOLD);cv_line(xx+5,p->y,xx+8,p->y+2,INK);}}
 if(floor){rect(x,p->y+9,p->w,240-p->y-9,RGB(28,34,41));rect(x,p->y+10,2,240-p->y-10,RGB(53,62,67));rect(x+p->w-2,p->y+10,2,240-p->y-10,RGB(53,62,67));for(int t=0;t<p->w/40;t++){int xx=x+t*40;if(xx<-45||xx>245)continue;cv_line(xx+9,p->y+14,xx+20,p->y+19,RGB(44,51,58));cv_line(xx+20,p->y+19,xx+14,p->y+22,RGB(44,51,58));}}
 else {cv_line(x+7,p->y+9,x+7,p->y+15,RGB(75,92,98));cv_line(x+7,p->y+15,x+16,p->y+9,RGB(75,92,98));cv_line(x+p->w-8,p->y+9,x+p->w-8,p->y+15,RGB(75,92,98));cv_line(x+p->w-8,p->y+15,x+p->w-17,p->y+9,RGB(75,92,98));}}
static void door_draw(int wx,int fy,int cam,const char *name){int x=wx-cam,w=imax(42,text_w(&FONT_XS,name)+10),l=x-w/2;rect(l,fy-55,w,55,INK);rect(l+1,fy-54,w-2,53,RGB(66,120,103));rect(l+3,fy-52,w-6,13,RGB(24,44,42));label(x-text_w(&FONT_XS,name)/2,fy-53,name,MINT);
 rect(x-14,fy-38,28,38,RGB(16,30,33));rect(x-12,fy-36,24,34,RGB(38,66,64));rect(x-9,fy-33,18,27,RGB(48,76,71));rect(x+8,fy-22,2,4,GOLD);rect(l,fy-2,w,2,MINT);}
static void scene_props(int cam){if(game.scene==BIKE_SCENE)return;int scene=game.scene;
 if(scene==FOREST_SCENE){forest_props(cam);return;}
 if(scene==6){flight_scene_props(cam);return;}
 if(scene>=2){data_center_interior(cam);if(scene==4||scene==5)door_draw(24,212,cam,"EXIT");return;}
 city_facades(cam);
 for(int i=0;i<art_prop_count[scene];i++){const art_prop_t *p=art_prop[scene]+i;int x=p->x-cam,fy=p->base;if(x<-40||x>260)continue;if(p->kind==0){rect(x-1,fy-71,2,71,RGB(73,86,92));rect(x-4,fy-4,8,4,RGB(51,67,71));rect(x-9,fy-76,20,5,RGB(65,81,89));rect(x-7,fy-71,16,2,RGB(169,153,91));}else {rect(x,fy-26,29,26,RGB(21,34,37));rect(x+2,fy-24,25,22,RGB(48,77,65));rect(x-1,fy-29,31,3,RGB(68,97,82));for(int n=0;n<3;n++)rect(x+6+n*7,fy-21,2,15,RGB(29,56,49));}}
 side_room_exterior(cam);
 if(scene==0){int x=4-cam;rect(x,25,140,44,INK);biglabel(x+6,26,"ZERO WASTE CITY",MINT);label(x+6,44,"ALL HUMANS ARE DEAD.",CORAL);label(x+6,56,"NEXT ARE THE BEAVERS.",CREAM);door_draw(520,212,cam,"SALVAGE");}
 if(scene==1){data_center_exterior(cam);door_draw(1580,212,cam,"CACHE");}
 if(scene==2)door_draw(24,212,cam,"EXIT");
 city_neon(cam);
 if(scene==1)flight_scene_props(cam);
 if(scene>=4){door_draw(24,212,cam,"EXIT");rect(72-cam,40,178,31,INK);biglabel(79-cam,43,scene==4?"SALVAGE DEN":"CACHE",GOLD);label(79-cam,59,"THE CITY MISSED A BIT.",CREAM);}
}
static void checkpoint_draw(int cam){for(int i=0;i<NCP;i++){const checkpoint_t *c=checkpoints+i;if(c->scene!=game.scene)continue;int x=c->x-cam;uint16_t color=i<=game.checkpoint?MINT:GOLD;rect(x-10,c->y-2,20,2,color);rect(x-6,c->y+4,3,2,color);rect(x+3,c->y+4,3,2,color);}}
#include "robot_art.h"
#include "boss_art.h"
static void bike_civilian(int x,int h,int id);
static void robot_draw(int i,int cam){robot_t *r=game.robots+i;if(!robot_here(i))return;if(i==MOTHER)return;int x=r->x-cam,y=r->y;if(x<-40||x>(int)cv_w+20)return;if(!r->hp)return;
 if(game.scene==BIKE_SCENE&&!robot_kind(i)){int id=(r->shots>>1)-1;bike_civilian(x,bike_obstacle_h[id%3],id);return;}
 if(robot_kind(i)==1||robot_kind(i)==3){mech_flyer(x,y,r->dir,r->state,r->timer,r->flash);if(game.scene==BIKE_SCENE){rect(x+7,y+11,12,2,MINT);rect(x+30,y+8,7,1,MINT);rect(x+33,y+16,5,1,MINT);}}
 else {ellipse(x+2,y+31,24,2,RGB(16,26,30));mech_walker(x,y,r->dir,((game.scene==FOREST_SCENE&&(r->state==4||r->state==0))||(robot_defs[i].type==3&&(r->state==0||r->state==3)))?(r->x/2)&7:-1,robot_defs[i].type==3,r->flash);}

 if(r->state==1){int total=game.scene==6?FLIGHT_WARNING_TICKS:game.scene==FOREST_SCENE?75:game.scene==0?80:60;rect(x,y-8,27,4,INK);rect(x+1,y-7,(total-imin(total,r->timer))*25/total,2,CORAL);}else if(r->state==3)rect(x+8,y+8,10,2,RGB(60,103,119));
}
static void logs_draw(int cam){
 for(int i=0;i<NLOG;i++)if(log_visible(i)){
  const log_def_t *p=log_defs+i;int x=p->x-cam,y=p->y;
  if(x< -16||x>(int)cv_w+16)continue;
  ellipse(x-13,y-9,26,9,RGB(38,29,24));rect(x-10,y-9,20,7,RGB(105,65,39));
  rect(x-9,y-8,18,2,RGB(145,94,53));rect(x-10,y-4,19,2,RGB(73,46,32));
  rect(x-7,y-6,11,1,RGB(61,41,31));rect(x-2,y-9,9,1,RGB(174,125,75));
  ellipse(x+6,y-9,8,8,RGB(204,160,101));ellipse(x+8,y-7,4,4,RGB(130,87,48));
  rect(x+9,y-6,2,2,RGB(211,162,91));

 }
}
static void heart(int x,int y,int on){uint16_t c=on?CORAL:RGB(59,66,82);ellipse(x,y,6,6,c);ellipse(x+6,y,6,6,c);rect(x+1,y+4,10,3,c);rect(x+3,y+7,6,2,c);rect(x+5,y+9,2,1,c);}
#include "player_shadow.h"
#include "story_art.h"
#include "woodland_art.h"
#include "ending_art.h"
#include "forest_art.h"
#include "bike_art.h"
#include "intro_art.h"
static void posse_draw(int cam){
 if(!game.crew||!game.crew_count)return;
 int sx[4]={-25,(int)cv_w+25,31,(int)cv_w-20};static const int sy[4]={188,171,50,61};
 int phase=game.crew;
 if(phase>=36&&phase<65)for(int j=0;j<game.crew_count;j++){
  int tx=game.crew_x[j]-cam,ty=game.crew_y[j];
  if(tx< -40||tx>(int)cv_w+20)continue;
  for(int n=0;n<4;n++)ellipse(tx-6+(n*13+phase*3)%34,ty+2+(n*7)%19,13,9,(n&1)?CREAM:GREY);
  rect(tx+6+(phase+j)%13,ty-3,3,3,STEEL);
 }
 for(int i=0;i<4;i++){
  int j=i*game.crew_count/4,tx=game.crew_x[j]-cam,ty=game.crew_y[j]+7,x,y;
  if(phase<36){int t=imin(27,imax(0,phase-i*2));x=sx[i]+(tx-sx[i])*t/27;y=sy[i]+(ty-sy[i])*t/27;}
  else if(phase<65){x=tx+(i&1?12:-13);y=ty-2-tri(phase+i*5,12)/2;}
  else {int t=phase-65;x=tx+(sx[i]-tx)*t/18;y=ty+(sy[i]-ty)*t/18;}
  beaver(x,y,phase<65?(x<tx?1:-1):(sx[i]>tx?1:-1),1,1);
 }
}
/* The defense uses a400x360 world canvas, reduced3:5 into216 rows.
 * Its original212px floor remains at screen212; HUD/fonts stay native.
 * Compaction is forward, then the216 rows shift downward in reverse. */
static void forest_view_resolve(void){
 for(int y=0;y<216;y++)for(int x=0;x<240;x++)cv_px[y*240+x]=cv_px[(y*5/3)*400+x*5/3];
 for(int y=215;y>=0;y--)for(int x=239;x>=0;x--)cv_px[(y+23)*240+x]=cv_px[y*240+x];
 for(int x=0;x<240;x++)cv_px[239*240+x]=cv_px[238*240+x];
 cv_w=cv_h=240;cv_oy=cv_y0=0;cv_y1=240;
}
static void render(void){int wide=forest_wide_view();cv_begin(wide?400:240,wide?360:240,wide?RGB(126,155,123):INK);if(wide)cv_oy=103;int cam=game.cam,scripted=game.flight_phase>=FLIGHT_EXIT;backdrop(cam);scene_props(cam);if(game.scene==FOREST_SCENE)forest_mission_draw(cam);if(game.scene==3){ending_scene_props(cam);mother_draw(cam);}if(game.scene!=6&&game.scene!=BIKE_SCENE){for(int i=0;i<terrain_count[game.scene];i++){if(game.scene==FOREST_SCENE)forest_platform(terrains[game.scene]+i,i<floor_count[game.scene],cam);else platform_draw(terrains[game.scene]+i,i<floor_count[game.scene],cam);}
 checkpoint_draw(cam);logs_draw(cam);
 for(int i=0;i<NSNACK;i++)if(snack_visible(i)){int x=snacks[i].x-cam,y=snacks[i].y;if(x<-12||x>252)continue;
  // One standing glass bottle with cloth wick, seated on its actual surface.
  rect(x-4,y-3,8,12,INK);rect(x-3,y-2,6,10,RGB(99,145,101));rect(x-1,y-7,3,5,RGB(116,162,111));rect(x-2,y-8,5,2,CREAM);rect(x+2,y-10,4,2,CORAL);rect(x-2,y+2,4,3,GOLD);rect(x-2,y-1,1,3,RGB(189,213,165));
 }
 }
 wind_foreground(cam);
 bike_scene_draw(cam);
 if(!scripted){
 for(int i=0;i<NROB;i++)robot_draw(i,cam);
 int px=game.x/Q8-cam,py=game.y/Q8,pose=beaver_pose();if(!game.flight_phase&&!game.bike_phase&&game.ending_phase<ENDING_CITY&&!(game.ending_phase==ENDING_ESCAPE&&game.x<=12*Q8))player_shadow_draw(cam);if(!game.dead&&game.ending_phase<ENDING_CITY&&!(game.ending_phase==ENDING_ESCAPE&&game.x<=12*Q8)&&!(game.hurt_flash&&(game.steps/4)%2)){if(bike_rider_active())bike_rider_draw(cam);else {hero_beaver(px,py,game.dir,pose);if(game.gun)weapon(px,py,game.dir);}if(game.flight_phase)flight_player_overlay(px,py);}
 for(int i=0;i<NBULLET;i++){bullet_t *b=game.bullets+i;if(b->life){int x=b->x-cam;rect(x-b->vx/2,b->y,b->owner?8:6,b->type?7:b->owner?3:2,b->owner?CORAL:GOLD);if(b->owner)rect(x,b->y,3,1,CREAM);}}
 if(game.bomb.stage){bomb_t *b=&game.bomb;int x=b->x/Q8-cam,y=b->y/Q8;if(b->stage==1){ellipse(x-4,y-5,9,12,INK);ellipse(x-2,y-4,5,9,GOLD);rect(x,y-8,2,5,CORAL);rect(x-2,y-8,5,2,CREAM);}else {rect(x-33,y-5,66,5,RGB(146,59,51));for(int i=0;i<8;i++){int h=10+tri(game.steps+i*9,36);ellipse(x-30+i*8,y-h,8,h,CORAL);ellipse(x-27+i*8,y-h+5,4,h-5,GOLD);if(i%2)rect(x-25+i*8,y-h+9,2,6,CREAM);}if(b->timer>139){ellipse(x-16,y-34,32,27,GOLD);biglabel(x-19,y-46,"POOF!",CREAM);}}}
 posse_draw(cam);
 for(int i=0;i<NPARTICLE;i++){particle_t *p=game.particles+i;if(!p->life)continue;int x=p->x/Q8-cam,y=p->y/Q8;if(p->type>=5){robot_burst_draw(p,x,y);continue;}uint16_t c=p->color==2?GOLD:p->color==3?BLUE:p->color==1?STEEL:p->color==5?RGB(190,141,87):CREAM;if(p->type==1||p->type==2){rect(x,y,5,5,INK);rect(x+1,y+1,3,3,c);if(p->type==2)rect(x+3,y-4,6,3,c);}else if(p->type==3)ellipse(x,y,5,3,RGB(99,104,109));else rect(x,y,p->type==4?3:2,p->type==4?1:2,c);}
 }
 flight_sequence_overlay();
 if(wide)forest_view_resolve();
 // Consistent frame: world, character and effects share one integer camera;
 // shake is confined to short warning flashes rather than changing layer phase.
 if(game.transition&&!game.dead){rect(12,26,216,18,INK);label(22,29,scene_names[game.scene],game.scene==1?BLUE:game.scene>=2?GOLD:MINT);}
 rect(0,0,240,23,INK);for(int i=0;i<3;i++)heart(7+i*16,6,game.hp>i);
 char state[16]={'P','O','S','S','E',' ',0};
 if(game.charge==CREW_TICKS){const char *r="READY";for(int i=0;i<6;i++)state[6+i]=r[i];}
 else {int n=((CREW_TICKS-game.charge)*16+999)/1000,p=6;if(n>=10)state[p++]=(char)('0'+n/10);state[p++]=(char)('0'+n%10);state[p++]='s';state[p]=0;}
 label(62+(89-text_w(&FONT_XS,state))/2,1,state,game.charge==CREW_TICKS?MINT:CREAM);
 rect(62,15,89,4,RGB(44,58,67));rect(62,15,game.charge*89/CREW_TICKS,4,MINT);
 label(169+(45-text_w(&FONT_XS,"BOTTLES"))/2,1,"BOTTLES",GOLD);for(int i=0;i<BOTTLE_CAP;i++){uint16_t c=i<game.bottles?GOLD:RGB(54,55,56);rect(169+i*19,15,7,5,c);rect(171+i*19,12,3,3,c);}rect(0,21,240,2,RGB(49,64,75));
 if(game.scene==FOREST_SCENE&&game.dam_phase==1&&!game.dam_entry&&!game.dead){int pulse=tri(game.steps,80);const char *title="DEFEND THE DAM";int x=(240-text_w(&FONT_M,title))/2;rect(x-4,27,text_w(&FONT_M,title)+8,28,INK);cv_text(x,29,&FONT_M,title,RGB(165+pulse*2,18+pulse/2,26+pulse/4));}
 if(game.flawless_tick&&(!game.ending_phase||game.ending_phase==ENDING_SHUTDOWN)&&!game.dead&&!game.pause){const char*s="FLAWLESS!";int w=text_w(&FONT_S,s);rect((240-w)/2-7,63,w+14,23,INK);biglabel((240-w)/2,66,s,GOLD);}
 ending_overlay();intro_overlay();
 if(game.pause&&!game.dead){rect(12,41,216,188,INK);rect(12,41,216,2,MINT);biglabel(28,51,game.notice&&game.save_status==-1?"SAVE FAILED":game.notice&&game.save_status==1?"PROGRESS SAVED":"TAKE A BREATHER",GOLD);static const char *const menu[]={"RESUME","SAVE PROGRESS","RETRY CHECKPOINT","NEW RUN","MUSIC"};for(int i=0;i<5;i++){if(i==game.menu)rect(23,79+i*20,194,18,RGB(41,65,68));biglabel(31,80+i*20,menu[i],i==game.menu?MINT:CREAM);if(i==4)biglabel(138,160,game.music?"ON":"OFF",MINT);}label(24,186,"SELECT KNOB: CHOOSE",GREY);label(24,199,"SEL: OK  PLAY: RETURN",GREY);label(24,215,game.run_valid?"FLAWLESS RUN: ELIGIBLE":"FLAWLESS RUN: NOT ELIGIBLE",game.run_valid?GOLD:GREY);}
 if(game.dead){
  rect(8,50,224,148,INK);rect(8,50,224,2,CORAL);const char *title="BEAVER DOWN",*retry="JUMP / FIRE: CHECKPOINT";
  cv_text((240-text_w(&FONT_L,title))/2,66,&FONT_L,title,CORAL);cv_text((240-text_w(&FONT_S,retry))/2,110,&FONT_S,retry,CREAM);
  ellipse(89,177,63,2,RGB(41,52,52));hero_beaver(114,146,1,BA_IDLE);
 }else if(game.won&&game.ending_phase!=ENDING_DONE){rect(12,62,216,130,INK);biglabel(25,76,"PRODUCTION: OFF!",MINT);label(25,174,"JUMP / FIRE: NEW RUN",GOLD);}

 cv_blit(0,0);
}
