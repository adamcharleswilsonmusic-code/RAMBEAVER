/* SPDX-License-Identifier: GPL-3.0-only
 * Original fixed-step jetpack stage. Three existing transient robot slots,
 * ordinary bullets/effects, and a single checkpoint keep every budget bounded.
 */
#pragma once
#define FLIGHT_WARNING_TICKS 112
#define FLIGHT_LAUNCH_X 222
#define FLIGHT_HEAD_CLEARANCE 40
#define FLIGHT_WAVE_TICKS 128
#define FLIGHT_TAKEOFF_TICKS 64
#define FLIGHT_EXIT_TICKS 64
#define FLIGHT_LAND_TICKS 96
#define FLIGHT_DROP_TICKS 64
static void flight_clear_heads(void){
 for(int i=WORKER_FIRST;i<WORKER_LAST;i++){reward_discard(i);game.robots[i]=(robot_t){0};}
}
static void flight_start(int launch){
 if(launch)stage_clear(1);clear_effects();flight_clear_heads();
 game.flight_done=0;game.flight_tick=game.flight_wave=0;
 game.flight_timer=110;game.flight_thrust=1;
 game.checkpoint=10;game.cp_scene=1;game.cp_x=checkpoints[10].x;game.cp_y=checkpoints[10].y-PLAYER_H;
 int first=!(game.visited&(1u<<6));if(!launch){set_scene(6,58,104,1);if(first)arrival_start(3,-52,58,104);}
 game.flight_phase=launch?FLIGHT_TAKEOFF:FLIGHT_CRUISE;
 game.transition=0;game.vx=game.vy=game.buffer=game.gun=game.recoil=game.fireclock=0;
 game.ground=game.duck=0;game.support=game.drop_platform=-1;game.drop_timer=0;game.dir=1;
 game.invuln=60;game.hurt_flash=0;game_music_variant(5);game_music(game.music);
 game.save_pending=1;game.save_requested=0;
 if(launch)game_sound(S_SNACK);
}
static void flight_finish(void){
 flight_clear_heads();game.flight_phase=game.flight_tick=game.flight_thrust=0;game.flight_done=1;
 set_scene(2,32,180,1);game.checkpoint=11;
 game.cp_scene=2;game.cp_x=32;game.cp_y=180;game.factory_timer=80;
 game.save_pending=1;game.save_requested=0;game.notice=150;
}
static void flight_spawn_wave(void){
 /* Adjacent open lanes alternate through the middle. A new pair never
  * demands a full-height reversal, and all attacks are visible before motion. */
 static const uint8_t gap[]={1,0,1,2,1,0,1,2};
 static const uint8_t lane_y[]={44,104,164};
 int open=gap[game.flight_wave%8],order=0;
 for(int lane=0;lane<3;lane++)if(lane!=open){
  int i=factory_free_slot();if(i<0)break;
  reward_spawn(i);game.robots[i]=(robot_t){game.cam+270,lane_y[lane],FLIGHT_HEAD_QUEUED,FLIGHT_WARNING_TICKS+64*order++,2,-1,0,0,lane_y[lane]+18};
 }
 game.flight_wave++;game_sound(S_TELL);
}
static int flight_launch_blocked(int i){
 const robot_t *r=game.robots+i;uint32_t id=game.spawn_id[i-WORKER_FIRST];
 for(int j=WORKER_FIRST;j<WORKER_LAST;j++)if(j!=i){
  const robot_t *other=game.robots+j;if(!other->hp||other->y!=r->y)continue;
  if(other->state==0){if(other->x>game.cam+FLIGHT_LAUNCH_X-FLIGHT_HEAD_CLEARANCE)return 1;}
  else if(game.spawn_id[j-WORKER_FIRST]<id)return 1;
 }
 return 0;
}
static void flight_heads_step(void){
 /* Move departures first, then admit warnings in stable spawn order. Only
  * one head owns a lane's launch point; its rotor must clear by40 pixels. */
 for(int i=WORKER_FIRST;i<WORKER_LAST;i++){
  robot_t *r=game.robots+i;if(r->flash)r->flash--;if(!r->hp||r->state!=0)continue;
  r->x--; /* Four pixels per frame relative to the forward-moving beaver. */
  if(r->x+28<game.cam){reward_discard(i);r->hp=r->flash=0;continue;}
  int px=game.x/Q8,py=game.y/Q8;
  if(r->x<px+PLAYER_W&&r->x+27>px&&r->y<py+PLAYER_H&&r->y+28>py){
   damage();reward_discard(i);r->hp=0;r->flash=24;
  }
 }
 unsigned handled=0;
 for(int n=WORKER_FIRST;n<WORKER_LAST;n++){
  int i=-1;uint32_t oldest=0xffffffffu;
  for(int j=WORKER_FIRST;j<WORKER_LAST;j++)if(!(handled&(1u<<(j-WORKER_FIRST)))&&game.robots[j].hp&&game.robots[j].state!=0&&game.spawn_id[j-WORKER_FIRST]<oldest){i=j;oldest=game.spawn_id[j-WORKER_FIRST];}
  if(i<0)break;handled|=1u<<(i-WORKER_FIRST);robot_t *r=game.robots+i;
  if(flight_launch_blocked(i)){
   r->state=FLIGHT_HEAD_QUEUED;r->x=game.cam+270;
   /* Stagger time may elapse off screen, but the entire112-tick warning
    * remains once admitted. Queue state uses existing saved robot fields. */
   r->timer=imax(FLIGHT_WARNING_TICKS,r->timer-1);continue;
  }
  r->state=1;r->x=game.cam+FLIGHT_LAUNCH_X;
  if(--r->timer<=0){r->state=0;r->timer=0;}
 }
}
static void flight_step(uint32_t k,uint32_t e){
 game.steps++;if(game.pop_clock)game.pop_clock--;game.flight_tick++;game.dir=1;
 if(game.charge<CREW_TICKS)game.charge++;
 if(game.notice)game.notice--;if(game.invuln)game.invuln--;if(game.hurt_flash)game.hurt_flash--;
 if(game.shake)game.shake--;if(game.recoil)game.recoil--;
 for(int i=0;i<NPARTICLE;i++){
  particle_t *p=game.particles+i;
  if(p->life){p->x+=p->vx;p->y+=p->vy;if(p->type<5||p->type==6)p->vy+=24;p->life--;}
 }
 if(game.flight_phase==FLIGHT_TAKEOFF){
  game.flight_thrust=1;game.x+=2*Q8;game.y-=Q8/2;
  if(game.flight_tick>=FLIGHT_TAKEOFF_TICKS){flight_start(0);game.input_lock=0;}
  return;
 }
 if(game.flight_phase==FLIGHT_EXIT){
  game.flight_thrust=1;game.x+=4*Q8;game.y+=(104*Q8-game.y)/8;
  if(game.flight_tick>=FLIGHT_EXIT_TICKS){game.flight_phase=FLIGHT_LAND;game.flight_tick=0;game_music_variant(2);}
  return;
 }
 if(game.flight_phase==FLIGHT_LAND){
  game.flight_thrust=game.flight_tick<66;
  if(game.flight_tick>=FLIGHT_LAND_TICKS){game.flight_phase=FLIGHT_DROP;game.flight_tick=0;game.flight_thrust=0;}
  return;
 }
 if(game.flight_phase==FLIGHT_DROP){
  game.flight_tick=imin(game.flight_tick,FLIGHT_DROP_TICKS);
  if(game.flight_tick>=FLIGHT_DROP_TICKS&&(!game.music||om_audio_stat(6)==2))flight_finish();
  return;
 }
 game.flight_thrust=!!(k&JUMP);
 game.vx=3*Q8;game.x+=game.vx;
 game.vy=imin(610,imax(-620,game.vy+(game.flight_thrust?-55:42)));
 game.y+=game.vy;
 if(game.y<FLIGHT_TOP*Q8){game.y=FLIGHT_TOP*Q8;game.vy=0;}
 if(game.y>FLIGHT_BOTTOM*Q8){game.y=FLIGHT_BOTTOM*Q8;damage();game.vy=-520;}
 game.cam=game.x/Q8-58;game.cam_q8=game.cam*Q8;
 if(game.x/Q8>=FLIGHT_END_X){
  stage_clear(6);clear_effects();flight_clear_heads();game.flight_phase=FLIGHT_EXIT;game.flight_tick=0;return;
 }
 if(--game.flight_timer<=0&&game.x/Q8<FLIGHT_END_X-720){flight_spawn_wave();game.flight_timer=FLIGHT_WAVE_TICKS;}
 if(k&FIRE){
  game.gun=6;
  if(!game.fireclock){bullet(game.x/Q8+29,game.y/Q8+23,9,0,0);game.fireclock=6;game.recoil=4;game.shots++;game_sound(S_GUN);}
 }else if(game.gun)game.gun--;
 if(game.fireclock)game.fireclock--;
 if(e&CREW)posse_begin();
 if((e&BOMB)&&game.bottles>0&&!game.bomb.stage&&!game.bomb.cooldown){
  game.bottles--;game.throws++;game.bomb=(bomb_t){game.x+12*Q8,game.y+10*Q8,680,-980,1,80,0};game_sound(S_SWARM);
 }
 posse_step();flight_heads_step();update_bullets();update_bomb();
}
