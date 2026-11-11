/* SPDX-License-Identifier: GPL-3.0-only
 * Original code-drawn data-center art, authored for Beaver: Zero Waste.
 * Decorative geometry only: floors, ledges, doors and encounters are unchanged.
 */
#pragma once

/* A few deliberately authored notices use the existing readable 12/16px
 * fonts. Their line breaks and positions remain fixed in the world. */
typedef struct {int16_t scene,x,y;uint8_t large,rows;const char *line[4];} dc_wall_panel_t;
static const dc_wall_panel_t dc_wall_panels[]={
 {0,20,110,0,3,{"NO BEAVERS.","NO EXCEPTIONS.","ESPECIALLY THAT ONE.",0}},
 {1,1352,58,1,1,{"REPORT ALL BEAVERS",0,0,0}},
 {2,448,50,1,3,{"THIS FACILITY MUST","BE KEPT CLEAN","OF BEAVERS",0}},
 {4,48,50,1,4,{"NOTICE: ANY ANIMAL","CLAIMING TO BE A","LARGE HAMSTER","IS PROBABLY A BEAVER."}}
};
#define DC_WALL_PANELS (sizeof dc_wall_panels/sizeof dc_wall_panels[0])
static int dc_panel_width(const dc_wall_panel_t *p){
 int w=0;for(int i=0;i<p->rows;i++)w=imax(w,text_w(p->large?&FONT_S:&FONT_XS,p->line[i]));return w+12;
}
static int dc_panel_height(const dc_wall_panel_t *p){return p->rows*(p->large?16:14)+12;}
static int dc_panel_reserved(int x,int y,int w,int h){
 for(unsigned i=0;i<DC_WALL_PANELS;i++){
  const dc_wall_panel_t *p=dc_wall_panels+i;if(p->scene!=game.scene)continue;
  if(x<p->x+dc_panel_width(p)+3&&x+w>p->x-3&&y<p->y+dc_panel_height(p)+3&&y+h>p->y-3)return 1;
 }
 return 0;
}
static void dc_wall_panels_draw(int cam){
 for(unsigned i=0;i<DC_WALL_PANELS;i++){
  const dc_wall_panel_t *p=dc_wall_panels+i;if(p->scene!=game.scene)continue;
  int x=p->x-cam,w=dc_panel_width(p),h=dc_panel_height(p);if(x+w<0||x>240)continue;
  int pulse=sky_pulse()/7,warm=game.scene==0||game.scene==4;
  uint16_t c=warm?RGB(193+pulse,157+pulse,99+pulse):RGB(123+pulse,189+pulse,190+pulse);
  rect(x-2,p->y-2,w+4,h+4,warm?RGB(71+pulse,57+pulse,41):RGB(38,65+pulse,74+pulse));
  rect(x,p->y,w,h,RGB(17,27,34));rect(x+1,p->y,w-2,1,c);rect(x+1,p->y+h-1,w-2,1,c);
  for(int row=0;row<p->rows;row++)cv_text(x+6,p->y+6+row*(p->large?16:14),p->large?&FONT_S:&FONT_XS,p->line[row],c);
 }
}

/* Keep indicator rows and machinery detail out of the walkable ledge bands.
 * Dark wall/cabinet silhouettes may continue behind a ledge; the ledge itself
 * is rendered later, with the game's normal bright collision-surface palette.
 */
static int dc_detail_clear(int wx,int y,int w,int h){
 if(dc_panel_reserved(wx,y,w,h))return 0;
 for(int i=floor_count[game.scene];i<terrain_count[game.scene];i++){
  const platform_t *p=terrains[game.scene]+i;
  if(wx<p->x+p->w+5&&wx+w>p->x-5&&y<p->y+22&&y+h>p->y-6)return 0;
 }
 if(game.scene==2&&wx<53&&wx+w>-5&&y+h>154)return 0;
 return 1;
}

static void dc_no_beaver(int wx,int y,int cam,uint16_t c){
 int x=wx-cam;
 ellipse(x,y,22,18,c);ellipse(x+2,y+2,18,14,RGB(20,31,37));
 /* Original tiny side-profile pictogram: low back, muzzle and paddle. */
 ellipse(x+7,y+7,10,6,RGB(139,158,137));
 ellipse(x+14,y+7,5,4,RGB(139,158,137));
 ellipse(x+2,y+11,7,4,RGB(139,158,137));
 rect(x+14,y+6,2,2,RGB(139,158,137));
 rect(x+9,y+12,2,3,RGB(139,158,137));
 rect(x+16,y+11,2,3,RGB(139,158,137));
 cv_line(x+4,y+3,x+18,y+15,c);cv_line(x+5,y+3,x+19,y+15,c);
}
static void dc_beaver_holder(int wx,int y,int cam,int w,int h,uint16_t c){
 dc_no_beaver(wx+(w-22)/2,y+(h-18)/2,cam,c);
}
static void dc_notice(int wx,int y,int cam,const char *text){
 int w=text_w(&FONT_XS,text)+32,x=wx-cam;
 if(x+w<0||x>240)return;
 if(dc_panel_reserved(wx-2,y-2,w+4,24))return;
 /* Explicit upper-wall slots avoid the actual ledge/support bands. They
  * stay visible as fixtures instead of vanishing under broad jump masks. */
 for(int i=0;i<terrain_count[game.scene];i++){
  const platform_t *p=terrains[game.scene]+i;
  if(wx<p->x+p->w+5&&wx+w>p->x-5&&y<p->y+22&&y+20>p->y-6)return;
 }
 int pulse=sky_pulse()/6;
 uint16_t c=game.scene==4?RGB(177+pulse,149+pulse,91+pulse):RGB(106+pulse,179+pulse,172+pulse);
 rect(x-1,y-1,w+2,22,RGB(39+pulse,64+pulse,64+pulse));rect(x,y,w,20,RGB(17,28,35));
 rect(x+1,y,w-2,1,c);rect(x+1,y+19,w-2,1,c);
 dc_beaver_holder(wx,y,cam,26,20,c);label(x+27,y+4,text,c);
}

static void side_room_exterior(int cam){
 int scene=game.scene;if(scene!=0&&scene!=1)return;
 int cx=scene==0?520:1580,wx=scene==0?360:1340,w=scene==0?368:428,x=wx-cam,top=scene==0?64:48;
 if(x+w<0||x>240)return;
 /* The complete nearby platform group belongs to this one building.
  * Its entry bay retains the exact door center; the wings are asymmetric. */
 uint16_t wall=scene==0?RGB(64,49,42):RGB(31,48,66);
 rect(x,top,w,212-top,RGB(20,30,37));
 rect(x+3,top+4,w-6,208-top,wall);
 rect(x,top,w,4,scene==0?RGB(96,74,59):RGB(69,88,105));
 rect(x+5,top+7,w-10,3,RGB(27,37,43));
 for(int yy=top+33;yy<202;yy+=26)rect(x+6,yy,w-12,1,scene?RGB(44,61,77):RGB(46,38,34));
 /* Both jambs and the foundation share the interactive door's exact
  * center/floor. The door itself is drawn afterward by the caller. */
 int door=cx-cam;
 rect(door-37,147,74,65,RGB(23,36,41));
 rect(door-35,149,70,4,RGB(66,91,85));
 rect(door-35,153,4,59,RGB(46,65,64));
 rect(door+31,153,4,59,RGB(46,65,64));
 rect(x,205,w,7,RGB(26,35,40));
 rect(x+3,205,w-6,2,RGB(69,77,76));
 /* Just a few quiet utility panels; none cross doors or ledge bands. */
 for(int px=wx+17;px+38<wx+w-10;px+=71)for(int row=0;row<3;row++){
  int py=top+39+row*32;
  if(py+21>=203||!dc_detail_clear(px,py,38,21))continue;
  if(px<cx+45&&px+38>cx-45&&py+21>147)continue;
  rect(px-cam,py,38,21,RGB(20,32,39));
  rect(px+3-cam,py+3,32,15,scene==0?RGB(48,65,59):RGB(39,65,75));
  rect(px+18-cam,py+3,2,15,RGB(29,43,46));
 }
}

static void data_center_exterior(int cam){
 const int wx=3368,w=470,base=176;int x=wx-cam;
 if(x+w<0||x>240)return;
 /* One building stretches well beyond a screen, with a tall utility block
  * rising from its roof. Its foundation is entirely on the final street. */
 rect(x,39,w,base-39,RGB(21,33,46));
 rect(x+5,44,w-10,base-44,RGB(36,49,62));
 rect(x+188,24,94,20,RGB(26,39,52));
 rect(x+192,27,86,17,RGB(48,62,73));
 for(int xx=198;xx<273;xx+=9)rect(x+xx,29,3,11,RGB(23,35,46));
 rect(x-2,39,w+2,5,RGB(61,76,86));
 rect(x+2,44,w-4,3,RGB(18,30,40));
 rect(x+7,62,w-14,6,RGB(25,38,50));
 rect(x+7,67,w-14,1,RGB(56,70,81));
 /* Giant service panels and structural buttresses replace the small city
  * windows. Restrained values keep player/robot silhouettes readable. */
 for(int xx=14;xx<w-25;xx+=78){
  rect(x+xx,76,62,85,RGB(27,41,54));
  rect(x+xx+3,79,56,79,RGB(32,46,59));
  for(int yy=86;yy<150;yy+=13)rect(x+xx+7,yy,47,2,RGB(42,58,69));
  rect(x+xx-7,48,5,base-48,RGB(49,64,76));
  rect(x+xx-5,49,2,base-49,RGB(32,47,59));
 }
 rect(x,base-8,w,8,RGB(21,32,42));
 rect(x+3,base-8,w-6,2,RGB(63,76,82));
 /* The former street entrance is now an unlit, sealed service bay below
  * the launch roof. The jetpack route enters the distant mother facility. */
 int entrance=3790-cam;
 rect(entrance-42,76,84,100,RGB(15,29,35));
 rect(entrance-39,79,78,94,RGB(62,81,94));
 rect(entrance-36,96,72,77,RGB(20,37,42));
 rect(entrance-36,90,72,16,RGB(23,36,47));
 label(entrance-text_w(&FONT_XS,"SERVICE BAY")/2,92,"SERVICE BAY",RGB(134,155,165));
 rect(entrance-34,108,68,63,RGB(34,53,69));
 for(int yy=112;yy<169;yy+=9)rect(entrance-31,yy,62,2,RGB(55,76,91));
 rect(entrance-1,109,2,61,RGB(20,37,51));
 rect(entrance-25,145,50,11,RGB(22,35,46));label(entrance-18,145,"SEALED",RGB(149,123,86));
 /* One threshold joins the building foundation to the existing street. */
 rect(entrance-42,172,84,4,RGB(71,101,95));
 rect(entrance-38,172,76,1,RGB(123,151,130));
 dc_no_beaver(3816,47,cam,RGB(179,127,143));
}

static void data_center_backdrop(int cam){
 int pulse=sky_pulse()/3;
 rect(0,23,240,217,RGB(16,24,31));
 /* A continuous ceiling, rear-wall/floor junction and structural ribs
  * make this an enclosed room even when the foreground has a floor gap. */
 rect(0,45,240,159,RGB(25,35,43));
 int scroll=cam/4,first=scroll/112-1;
 for(int i=first;i<first+5;i++){
  int x=i*112-scroll;
  rect(x+5,49,102,144,RGB(22,31,39));
  rect(x+9,53,94,135,RGB(26,36,44));
  rect(x+3,45,3,160,RGB(39,49,56));
  rect(x+105,45,3,160,RGB(16,25,32));
  rect(x+11,84,88,1,RGB(31,42,50));
  rect(x+11,137,88,1,RGB(31,42,50));
  /* Long overhead fixtures and their suspension rods, never sky glow. */
  rect(x+26,36,2,10,RGB(44,58,63));
  rect(x+77,36,2,10,RGB(44,58,63));
  rect(x+19,45,68,5,RGB(16,26,32));
  rect(x+23,47,60,2,game.scene==3?RGB(124+pulse,75+pulse/2,83+pulse/2):RGB(70+pulse,121+pulse,115+pulse));
 }
 rect(0,193,240,47,RGB(18,28,34));
 rect(0,193,240,2,RGB(35,48,54));
 rect(0,23,240,12,RGB(12,20,27));
 rect(0,34,240,9,RGB(39,51,60));
 rect(0,42,240,3,RGB(13,22,29));
 rect(0,29,240,2,RGB(52,64,69));
 /* All ceiling ribs/cable brackets are world-stable at one shared depth. */
 first=scroll/56-1;
 for(int i=first;i<first+7;i++){
  int x=i*56-scroll;
  rect(x,23,4,20,RGB(24,36,45));
  rect(x+1,28,2,5,RGB(65,76,79));
 }
 if(game.scene>=4){
  /* Small rooms have a continuous close wall, not the outdoor facade
   * generator. Their ceiling and rear floor junction are fully enclosed. */
  uint16_t wall=game.scene==4?RGB(41,39,37):RGB(28,39,45);
  rect(0,45,240,164,wall);rect(0,207,240,33,RGB(20,29,33));
  for(int wx=0;wx<320;wx+=80){int x=wx-cam;rect(x,45,3,164,RGB(24,33,37));rect(x+3,45,1,164,RGB(48,57,59));}
  rect(0,91,240,1,RGB(34,43,46));rect(0,146,240,1,RGB(34,43,46));
  int x=147-cam;rect(x,43,82,6,RGB(17,27,32));rect(x+4,46,74,2,RGB(88+pulse,101+pulse,91+pulse));
 }
}

static void dc_rack(int wx,int floor,int height,int cam,int variant){
 int x=wx-cam,top=floor-height,w=44;
 if(x+w<0||x>240)return;
 rect(x,top,w,height,RGB(14,23,30));
 rect(x+2,top+2,w-4,height-3,RGB(32,45,54));
 rect(x+4,top+4,w-8,height-8,RGB(19,30,38));
 rect(x+2,top+2,2,height-4,RGB(46,61,68));
 /* Cabinet feet and cable riser attach to this exact floor run. */
 rect(x+3,floor-4,7,4,RGB(49,62,65));
 rect(x+w-10,floor-4,7,4,RGB(49,62,65));
 /* Power enters through the floor bus; upper wall space stays clear for
  * notices and catwalks rather than crossing them with hanging cables. */
 rect(x+w-7,floor-6,2,6,RGB(57,76,73));
 int pulse=sky_pulse()/6;
 for(int yy=top+8,row=0;yy<floor-10;yy+=12,row++){
  if(!dc_detail_clear(wx+5,yy,w-10,9))continue;
  rect(x+6,yy,w-12,9,RGB(37,49,57));
  rect(x+10,yy+3,18,1,RGB(18,29,37));
  rect(x+10,yy+5,18,1,RGB(22,34,40));
  int active=(row+variant+(int)game.steps/11)%7<2;
  rect(x+32,yy+3,2,2,active?RGB(91+pulse,172+pulse,141+pulse):RGB(54,90,85));
  if(active)rect(x+10,yy+7,6+(game.steps/7)%11,1,RGB(54,105+pulse,97+pulse));
 }
}

static void dc_workbench(int wx,int floor,int cam,int variant){
 int x=wx-cam,w=112,top=floor-31;
 if(x+w<0||x>240)return;
 /* A still maintenance bench: subdued casing, attached short legs and
  * a small diagnostic console. No moving production decorations. */
 rect(x,top,w,20,RGB(15,26,32));
 rect(x+3,top+3,w-6,13,RGB(42,58,62));
 rect(x+4,top+5,w-8,9,RGB(25,40,47));
 rect(x+5,top+19,5,floor-top-19,RGB(46,61,65));
 rect(x+w-10,top+19,5,floor-top-19,RGB(46,61,65));
 if(dc_detail_clear(wx+4,top+4,w-8,10)){
  rect(x+12,top+6,35,6,RGB(34,49,56));
  rect(x+62,top+6,35,6,RGB(34,49,56));
  rect(x+39,top+8,5,1,RGB(67,82,83));
  rect(x+65,top+8,5,1,RGB(67,82,83));
 }
 if(dc_detail_clear(wx+39,top-25,34,25)){
  rect(x+39,top-25,34,20,RGB(15,28,35));
  rect(x+43,top-21,26,12,RGB(32,58,61));
  rect(x+46,top-18,11,1,variant?RGB(82,103,88):RGB(76,107,110));
  rect(x+53,top-5,6,5,RGB(50,69,73));
 }
}

static void dc_catwalk_mounts(int cam){
 for(int i=floor_count[game.scene];i<terrain_count[game.scene];i++){
  const platform_t *p=terrains[game.scene]+i;int x=p->x-cam;if(x+p->w<0||x>240)continue;
  for(int side=0;side<2;side++){
   int mx=x+(side?p->w-13:8);rect(mx,p->y-5,5,23,RGB(38,57,62));
   rect(mx+1,p->y-4,2,2,RGB(87,108,104));rect(mx+1,p->y+14,2,2,RGB(75,98,96));
  }
 }
}
static void data_center_interior(int cam){
 int scene=game.scene;
 if(scene>=4){
  /* A salvage store and a maintenance cache share a closed service-room
   * shell. Storage rests on y212; actual playable shelves render later. */
  if(scene==4)dc_no_beaver(15,58,cam,RGB(185,149,95));
  label((scene==4?88:121)-cam,scene==4?150:59,scene==4?"SALVAGE STORE":"MAINTENANCE",RGB(119,136,129));
  dc_workbench(77,212,cam,scene==5);
  int x=230-cam;
  rect(x,83,66,129,RGB(17,28,34));rect(x+3,86,60,123,RGB(39,50,53));
  rect(x+31,86,2,123,RGB(23,36,39));
  for(int y=98;y<202;y+=31)if(dc_detail_clear(234,y,54,20)){
   rect(x+7,y,22,20,scene==4?RGB(66,64,48):RGB(39,63,64));
   rect(x+37,y,22,20,scene==4?RGB(55,60,47):RGB(42,59,67));
   rect(x+15,y+4,8,2,RGB(88,92,72));rect(x+43,y+4,8,2,RGB(69,89,86));
  }
  rect(x+3,208,60,4,RGB(49,61,61));
  rect(304-cam,44,4,168,RGB(50,65,66));rect(306-cam,44,1,168,RGB(71,82,79));
  dc_wall_panels_draw(cam);dc_catwalk_mounts(cam);return;
 }
 if(scene==2){
  /* A closed roof-entry chute replaces the old street-facing exit door. */
  int x=18-cam;rect(x,43,52,169,RGB(14,25,34));rect(x+4,44,44,168,RGB(24,41,52));
  rect(x,43,4,169,RGB(63,82,93));rect(x+48,43,4,169,RGB(63,82,93));
  for(int yy=94;yy<180;yy+=38)rect(x+5,yy,42,3,RGB(41,61,71));
  rect(x+3,34,46,11,RGB(78,102,111));rect(x+7,37,38,5,RGB(30,50,63));rect(x+25,37,2,5,RGB(14,26,37));
  label(x+8,66,"ROOF",RGB(98,126,137));rect(x+4,209,44,3,RGB(78,100,105));
 }
 /* Local fixtures use the same constant-height floor runs as collision;
  * no machine crosses a pit or floats above a street-height transition. */
 for(int f=0;f<floor_count[scene];f++){
  const platform_t *p=terrains[scene]+f;
  if(p->x+p->w<cam||p->x>cam+240)continue;
  for(int wx=p->x+72,index=0;wx+44<p->x+p->w-12;wx+=116,index++){
   if(scene==3&&wx>290)continue; /* Let the mother silhouette dominate. */
   dc_rack(wx,p->y,imin(p->y-56,100+(index%3)*10),cam,index+f);
  }
  int machine=p->x+p->w-138;
  if(scene==2&&p->w>=400)dc_workbench(machine,p->y,cam,f&1);
  /* Conduits connect the ceiling raceway to each supported machine bay. */
  int x=p->x+18-cam;
  if(x>-8&&x<248){
   rect(x,46,5,p->y-46,RGB(27,43,50));
   rect(x+1,46,1,p->y-46,RGB(50,68,73));
   rect(x-2,p->y-4,9,4,RGB(39,55,59));
   for(int yy=59;yy<p->y-8;yy+=26)rect(x-1,yy,7,2,RGB(49,63,68));
  }
 }
 /* The end of the long traversal is a sealed production bulkhead. */
 if(scene==2){
  dc_notice(76,50,cam,"MACHINE AUTHORITY");
  dc_notice(1380,52,cam,"WOOD IS BANNED");
  dc_notice(2834,52,cam,"TEETH OFF");
  int x=3314-cam;
  if(x<240&&x+46>0){
   rect(x,45,46,167,RGB(17,29,35));
   rect(x+3,48,5,164,RGB(49,68,70));
   rect(x+9,52,37,160,RGB(24,40,45));
   rect(x+9,74,37,4,RGB(56,79,77));
   rect(x+40,78,2,132,RGB(42,65,64));
  }
 }
 dc_wall_panels_draw(cam);dc_catwalk_mounts(cam);
}
