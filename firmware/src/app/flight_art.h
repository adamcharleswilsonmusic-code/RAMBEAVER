/* SPDX-License-Identifier: GPL-3.0-only
 * Original native-code rooftop flight and arrival cinematics. The foreground
 * roof edge is always y208, matching the flight controller's contact plane.
 */
#pragma once
static void flight_backdrop(int cam){
 rect(0,23,240,217,RGB(24,36,57));
 int pulse=sky_pulse()/3;
 for(int y=23;y<191;y+=24)rect(0,y,240,24,RGB(25+y/18,37+y/15,58+y/11));
 for(int layer=0;layer<2;layer++){
  int scroll=cam/(layer?5:11),first=scroll/166-1;
  for(int i=first;i<first+4;i++){
   int x=i*166-scroll,y=30+layer*42+(hash((unsigned)i+layer*47)%13);
   mist(x,y,207,27,layer?RGB(65+pulse,55+pulse,89+pulse):RGB(47+pulse,79+pulse,101+pulse));
   mist(x+25,y+8,150,13,layer?RGB(43+pulse/2,47+pulse/2,75+pulse):RGB(36+pulse/2,60+pulse/2,83+pulse));
  }
 }
 /* These muted towers are deliberately distant, never collision surfaces. */
 skyline_layer(cam,7,67,206,RGB(28,44,65),0);
 skyline_layer(cam,3,82,218,RGB(35,53,73),1);
}
static void flight_roofs(int cam){
 rect(0,208,240,32,RGB(23,37,49));rect(0,208,240,3,RGB(122,151,164));
 int first=cam/192;
 for(int i=first;i<first+3;i++){
  int x=i*192-cam;rect(x,211,3,29,RGB(62,83,96));rect(x+188,211,4,29,RGB(14,27,38));
  rect(x+9,215,173,1,RGB(50,70,83));
  label(x+16,219,(i%3)==0?"AI SKYWAY":(i%3)==1?"MACHINE AIRSPACE":"AUTONOMOUS DISTRICT",RGB(73,112,127));
  int signal=(game.steps/28+i)%3;rect(x+172,221,3,3,signal?RGB(62,110,104):RGB(105,177,145));
 }
}
static void flight_pack(int px,int py,int direction,int firing){
 int x=direction>0?px+1:px+11,y=py+9;
 /* Two tanks, a shoulder strap and a downward nozzle remain distinct at
  * native size; the beaver underneath is the same approved native sprite. */
 rect(x-2,y+1,14,15,INK);rect(x,y,5,15,RGB(127,163,172));rect(x+6,y+1,5,14,RGB(74,115,135));
 rect(x+1,y+1,2,11,RGB(199,215,203));rect(x+7,y+2,2,10,RGB(121,163,173));
 rect(x-1,y+6,13,2,RGB(42,66,78));rect(x+2,y+15,6,3,RGB(34,46,57));
 rect(direction>0?x+10:x-3,y+2,3,13,RGB(57,67,60));rect(x+7,y+5,2,2,GOLD);
 if(firing){int h=5+(game.flight_tick&3)+(game.flight_thrust?4:0);ellipse(x+2,y+18,6,h,BLUE);ellipse(x+3,y+18,4,imax(3,h-3),CREAM);}
}
static void flight_parked_pack(int wx,int surface,int cam){
 int x=wx-cam;rect(x-11,surface-2,22,2,RGB(74,91,95));
 rect(x-8,surface-22,16,20,INK);rect(x-6,surface-21,5,17,RGB(143,180,182));
 rect(x+1,surface-20,5,16,RGB(72,119,139));rect(x-5,surface-19,2,12,CREAM);
 rect(x-7,surface-12,14,3,RGB(45,65,76));rect(x-3,surface-5,7,3,RGB(43,55,65));
 rect(x+2,surface-16,2,3,((game.steps/24)&1)?MINT:GOLD);
}
static void flight_scene_props(int cam){
 if(game.scene==1){
  if(!game.flight_done&&!game.flight_phase){
   flight_parked_pack(JETPACK_X,JETPACK_Y,cam);
   int x=JETPACK_X-cam;rect(x-27,25,54,16,RGB(17,33,43));rect(x-27,25,54,1,MINT);
   label(x-text_w(&FONT_XS,"JETPACK")/2,27,"JETPACK",MINT);
  }
  return;
 }
 if(game.scene==6)flight_roofs(cam);
}
static void flight_player_overlay(int px,int py){
 if(game.flight_phase==FLIGHT_TAKEOFF||game.flight_phase==FLIGHT_CRUISE)flight_pack(px,py,game.dir,1);
}
static void flight_landing_building(int cam,int hatch){
 const int wx=3472,w=368,roof=176,sign_x=3504;int x=wx-cam;
 rect(x,roof,w,240-roof,RGB(29,46,60));rect(x,roof,w,4,STEEL);rect(x+4,roof+4,w-8,3,RGB(17,29,40));
 for(int n=12;n<w-10;n+=52){rect(x+n,roof+10,40,48,RGB(21,36,49));rect(x+n+3,roof+13,34,2,RGB(50,71,86));}
 /* The complete large sign is bolted to this building, not the viewport. */
 int sx=sign_x-cam;
 rect(sx+12,130,5,46,RGB(77,97,110));rect(sx+164,130,5,46,RGB(77,97,110));
 rect(sx-3,68,188,66,RGB(55,88,100));rect(sx,71,182,60,RGB(12,25,35));
 rect(sx+2,72,178,1,MINT);rect(sx+2,129,178,1,MINT);
 cv_text(sx+(182-text_w(&FONT_M,"MOTHER"))/2,75,&FONT_M,"MOTHER",RGB(164,214,196));
 cv_text(sx+(182-text_w(&FONT_M,"DATA CENTER"))/2,102,&FONT_M,"DATA CENTER",RGB(136,197,203));
 /* A real opening in the landing roof leads to the enclosed access shaft. */
 int hx=3660-cam;rect(hx,172,40,8,RGB(12,22,30));rect(hx-3,170,46,3,RGB(101,127,131));
 if(!hatch){rect(hx+2,172,36,3,RGB(71,97,109));rect(hx+18,172,2,3,RGB(29,47,60));}
 else {rect(hx-3,160,3,13,RGB(90,121,131));rect(hx+40,160,3,13,RGB(90,121,131));}
}
static void flight_sequence_overlay(void){
 int phase=game.flight_phase,t=game.flight_tick;
 if(phase<FLIGHT_EXIT)return;
 if(phase==FLIGHT_EXIT){
  int cam=game.cam;flight_backdrop(cam);flight_roofs(cam);
  int x=game.x/Q8-cam,y=game.y/Q8;hero_beaver(x,y,1,beaver_pose());if(game.gun)weapon(x,y,1);flight_pack(x,y,1,1);return;
 }
 if(phase==FLIGHT_LAND){
  const int cam=3480;flight_backdrop(cam);flight_landing_building(cam,t>=72);
  int x,y,pose;
  if(t<48){x=-40+t*3;y=118+t*26/48;pose=BA_FALL;}
  else {x=104+(t-48)*88/48;y=144;pose=t<56?BA_LAND:BA_WALK_0+((t-56)/4)%8;}
  hero_beaver(x,y,1,pose);if(t<54)flight_pack(x,y,1,t<48);else flight_parked_pack(cam+105,176,cam);
  return;
 }
 /* The drop is an enclosed vertical shaft: no city/sky leaks into it. */
 rect(0,23,240,217,RGB(15,25,34));rect(70,23,108,217,RGB(25,40,50));
 rect(70,23,7,217,RGB(65,85,96));rect(171,23,7,217,RGB(65,85,96));
 rect(78,23,4,217,RGB(15,29,40));rect(166,23,4,217,RGB(15,29,40));
 for(int i=-1;i<7;i++){
  int y=i*48-(t*2)%48;rect(80,y,88,4,RGB(42,60,71));rect(83,y+4,4,14,RGB(83,125,123));
  rect(160,y+4,4,14,RGB(83,125,123));
 }
 rect(79,28,89,7,RGB(91,117,124));rect(81,35,85,4,RGB(15,28,39));
 hero_beaver(108,43+t*3,1,BA_FALL);
}
