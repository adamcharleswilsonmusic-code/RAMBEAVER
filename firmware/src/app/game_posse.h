/* SPDX-License-Identifier: GPL-3.0-only
 * Original bounded viewport posse sweep. Selection is fixed at activation. */
#pragma once
static int robot_on_screen(int i){
 if(i==MOTHER||i<0||i>=NROB||!robot_here(i)||!game.robots[i].hp)return 0;
 const robot_t *r=game.robots+i;
 int flying=robot_kind(i)==1||robot_kind(i)==3;
 int width=flying?27:28,height=flying?28:32;
 /* The actual body/rotor footprint, excluding warning marks and the HUD. */
 return r->x+width>game.cam&&r->x<game.cam+game_view_width()&&r->y+height>(forest_wide_view()?-103:23)&&r->y<(forest_wide_view()?257:240);
}
static int posse_reserved(int id){
 if(!game.crew)return 0;
 for(int j=0;j<game.crew_count;j++)if(game.crew_ids[j]==id)return 1;
 return 0;
}
static void posse_begin(void){
 if(game.charge<CREW_TICKS||game.crew){game.save_status=4;game.notice=70;return;}
 game.crew_count=0;
 for(int i=0;i<NROB;i++)if(robot_on_screen(i)){
  int j=game.crew_count++;game.crew_ids[j]=(uint8_t)i;
  game.crew_x[j]=(int16_t)game.robots[i].x;game.crew_y[j]=(int16_t)game.robots[i].y;
 }
 if(!game.crew_count){game.save_status=3;game.notice=70;return;}
 game.crewtarget=game.crew_ids[0];game.charge=0;game.crew=1;game.crews++;
 if(!game_voice_available(7))game_sound(S_SWARM);game_voice(7);
}
static void posse_step(void){
 if(!game.crew)return;
 if(game.crew<36)for(int j=0;j<game.crew_count;j++){
  const robot_t *r=game.robots+game.crew_ids[j];
  if(r->hp){game.crew_x[j]=(int16_t)r->x;game.crew_y[j]=(int16_t)r->y;}
 }
 if(++game.crew==36)for(int j=0;j<game.crew_count;j++){
  int i=game.crew_ids[j];if(i!=MOTHER&&robot_here(i))kill_robot(i);
 }
 if(game.crew>82){game.crew=0;game.crew_count=0;}
}
