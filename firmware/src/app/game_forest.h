/* SPDX-License-Identifier: GPL-3.0-only
 * Original forest and bounded river-work defense. Three shared transient
 * slots, nine timed groups, one build counter; no construction input. */
#pragma once
static void forest_clear_robots(void){for(int i=WORKER_FIRST;i<WORKER_LAST;i++){reward_discard(i);game.robots[i]=(robot_t){0};}}
static void forest_on_kill(int i){int id=(game.robots[i].shots>>1)-1;if(id>=0&&id<4)game.forest_mask|=1<<id;}
static void forest_begin_defense(void){
 forest_clear_robots();clear_effects();game.forest_mask=15;game.hurt_flash=0;
 game.dam_phase=1;game.dam_progress=game.dam_tick=game.dam_wave=game.dam_blocked=0;game.dam_wait=45;
 game.transition=0;game.dam_entry=(game.visited&(1u<<9))?0:DAM_WALK_IN;game.visited|=1u<<9;game.dam_entry_tick=0;game.cam=1740;game.cam_q8=1740*Q8;
 game.x=(game.dam_entry?1690:1810)*Q8;game.y=180*Q8;game.vx=game.vy=0;game.dir=1;game.ground=1;game.support=3;game.input_lock=0;game.gun=game.recoil=game.duck=0;
 game_music_variant(8);game.checkpoint=FOREST_DAM_CP;game.cp_scene=FOREST_SCENE;game.cp_x=1810;game.cp_y=180;
 game.save_pending=1;game.save_requested=0;
}
static void forest_depart(void){
 if(game.forest_kills!=15)reward_miss();forest_clear_robots();clear_effects();game.forest_mask=15;game.hurt_flash=0;
 game.transition=0;game.dam_phase=1;game.dam_entry=DAM_WALK_OFF;game.dam_entry_tick=0;game.dam_entry_cam=game.cam;
 game.dam_progress=game.dam_tick=game.dam_wave=game.dam_blocked=0;game.gun=game.recoil=game.duck=0;game.vx=game.vy=0;game.dir=1;
 game.checkpoint=FOREST_DAM_CP;game.cp_scene=FOREST_SCENE;game.cp_x=1810;game.cp_y=180;game.save_pending=1;game.save_requested=0;
}
static void forest_entry_step(void){
 game.steps++;game.anim_land=game.duck=0;game.dir=1;game.y=180*Q8;game.vx=game.vy=0;game.ground=1;
 if(game.dam_entry==DAM_WALK_OFF){
  if(game.x<(game.dam_entry_cam+268)*Q8){game.x+=3*Q8;game.anim_stride=(game.anim_stride+3*Q8)%(40*Q8);}
  else if(++game.dam_entry_tick>=6)forest_begin_defense();
 }else{
  int target=game.dam_entry==DAM_WALK_IN?1810:2192;
  game.x=imin(target*Q8,game.x+3*Q8);game.anim_stride=(game.anim_stride+3*Q8)%(40*Q8);
  if(game.x>=target*Q8){game.dam_entry=0;game.input_lock=0;edges=0;game.support=game.dam_phase==1?3:4;}
 }
}
static void forest_to_city(void){
 forest_clear_robots();game.dam_phase=0;game.forest_done=1;
 set_scene(0,32,180,1);game.checkpoint=0;game.cp_scene=0;game.cp_x=32;game.cp_y=180;
 game.save_pending=1;game.save_requested=0;
}
static void forest_spawn(int slot,int x,int y,int kind,int tag,int dir){
 reward_spawn(slot);game.robot_voice_seen[slot/32]&=~(1u<<(slot%32));game.robots[slot]=(robot_t){x,y,4,40,2,dir,0,(tag<<1)|kind,y+18};
}
static void forest_robot_step(int i){
 robot_t *r=game.robots+i;if(r->flash)r->flash--;if(!r->hp)return;
 int px=game.x/Q8,py=game.y/Q8,kind=r->shots&1;
 if(r->state==4){
  if(game.dam_phase==1){
   r->x=factory_move_toward(r->x,px,kind?2:1);
   if(kind)r->y=factory_move_toward(r->y,r->aim_y,2);
  }
  if(r->timer)r->timer--;
  if(!r->timer&&robot_on_screen(i)){r->state=0;r->timer=30;}
  return;
 }
 /* Walkers cross the worksite instead of camping the water. Heads descend
  * from their entry lane, then track above the player between warnings. */
 if(game.dam_phase==1&&(r->state==0||r->state==3)){
  if(iabs(r->x-px)>72)r->x+=r->x<px?1:-1;
  if(kind)r->y=factory_move_toward(r->y,imin(140,imax(55,py-38)),1);
 }
 if(r->state==0){
  if(r->timer)r->timer--;
  if(!r->timer&&robot_on_screen(i)&&iabs(r->x-px)<180){r->dir=px<r->x?-1:1;r->state=1;r->timer=75;r->aim_y=kind?imin(198,imax(65,py+18)):r->y+15;game_sound(S_TELL);robot_alert_voice(i);}
 }else{
  if(kind&&r->state==1)r->y=factory_move_toward(r->y,r->aim_y-18,1);
  if(--r->timer<=0){
   if(r->state==1){r->state=2;r->timer=10;bullet(r->x+12+2*r->dir,kind?r->y+18:r->y+15,r->dir*4,1,0);game.attacks++;game_sound(S_LASER);}
   else if(r->state==2){r->state=3;r->timer=110;}
   else {r->state=0;r->timer=35;}
  }
 }
 if((!kind||r->state==2)&&iabs(r->x-px)<20&&player_robot_vertical(r->y,kind?28:32))damage();
}
static void forest_update(void){
 if(game.dam_entry)return;
 if(game.dam_phase==2){
  game.dam_tick=imin(DAM_DIVERT_TICKS,game.dam_tick+1);
  if(game.dam_tick>=DAM_DIVERT_TICKS&&game.x>=2168*Q8){
   game.dam_phase=3;game.checkpoint=FOREST_EXIT_CP;game.cp_scene=FOREST_SCENE;game.cp_x=2110;game.cp_y=180;
   set_scene(FOREST_SCENE,2070,180,0);game.dam_entry=DAM_WALK_RETURN;game.dam_entry_tick=0;game.cam=2110;game.cam_q8=2110*Q8;game.save_pending=1;
  }
  return;
 }
 if(game.dam_phase==3)return;
 if(!game.dam_phase){
  static const int anchors[4]={430,930,1390,1615};
  for(int n=0;n<4;n++)if(!(game.forest_mask&(1<<n))&&game.x/Q8>anchors[n]-185&&game.x/Q8<anchors[n]+220){
   int active=0;for(int i=WORKER_FIRST;i<WORKER_LAST;i++)active|=game.robots[i].hp&&(game.robots[i].shots>>1)==n+1;
   if(!active){int i=factory_free_slot();if(i>=0)forest_spawn(i,anchors[n],floor_at(FOREST_SCENE,anchors[n])-32,0,n+1,-1);}
  }
 }else{
  game.dam_tick++;
  if(game.dam_wait)game.dam_wait--;
  if(!game.dam_wait&&game.dam_wave<DAM_WAVES&&game.dam_tick>=45+game.dam_wave*140){
   /* A bounded group uses whichever slots are free; neither the timer nor
    * construction waits for kills. Every spawn starts wholly out of view. */
   for(int n=0;n<3;n++){
    int i=factory_free_slot();if(i<0)break;
    int lane=(game.dam_wave*3+n)%6,kind=lane!=0&&lane!=3;
    int x=lane<3?game.cam-56-n*32:game.cam+446+n*32,y=kind?20+lane*19:180;
    if(lane==2||lane==5){x=game.cam+90+(game.dam_wave*67+n*83)%240;y=-145;}
    forest_spawn(i,x,y,kind,0,x<game.x/Q8?1:-1);
    if(kind)game.robots[i].aim_y=90+(game.dam_wave*17+n*23)%55;
   }
   game.dam_wave++;game.dam_wait=0;
  }
 }
 game.dam_blocked=0;
 for(int i=WORKER_FIRST;i<WORKER_LAST;i++){
  robot_t *r=game.robots+i;
  if(!game.dam_phase&&r->hp&&iabs(r->x-game.x/Q8)>340){reward_discard(i);r->hp=r->flash=0;continue;}
  forest_robot_step(i);
 }
 if(game.dam_phase==1){
  if(game.dam_progress<DAM_BUILD_TICKS)game.dam_progress++;
  if(game.dam_progress>=DAM_BUILD_TICKS){
   game.forest_done=1;game.dam_phase=2;game.dam_tick=0;game.dam_completions++;game_music_variant(7);
   stage_clear(FOREST_SCENE);forest_clear_robots();clear_effects();game.gun=game.recoil=game.duck=0;game.save_pending=1;game_sound(S_SNACK);
  }
 }
}
