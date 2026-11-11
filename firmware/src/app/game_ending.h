/* SPDX-License-Identifier: GPL-3.0-only
 * Original bounded, resumable ending. Ordinary movement remains in step(). */
#pragma once
enum {ENDING_NONE,ENDING_SHUTDOWN,ENDING_ESCAPE,ENDING_CITY,ENDING_FOREST,ENDING_DONE};
#define ENDING_SHUTDOWN_TICKS 180
#define ENDING_PANEL_TICKS 170
static void ending_music(void){
 if(game.ending_phase<ENDING_CITY)game_music_silence();
 else {game_music_victory();game_music(game.music);}
}
static void ending_phase(int phase){
 game.ending_phase=phase;game.ending_tick=0;if(phase==ENDING_FOREST){game.party_tick=0;}game.save_pending=1;game.save_requested=0;
 if(phase!=ENDING_ESCAPE)game_alarm_stop();ending_music();
}
static void ending_begin(void){game.hurt_flash=0;
 if(game.ending_phase||game.won)return;game_voice_end_combat();
 stage_clear(3);reward_finish();game.arrival=0;game.dead=0;game.hp=imax(1,game.hp);game.factory_timer=0;
 for(int i=WORKER_FIRST;i<WORKER_LAST;i++)game.robots[i].hp=0;
 clear_effects();game.gun=game.recoil=game.fireclock=game.duck=game.drop_timer=0;
 game.drop_platform=-1;game.log_target=-1;game.log_chop=0;
 game.input_lock=game.transition=0;game.boss_end=ENDING_SHUTDOWN_TICKS;
 ending_phase(ENDING_SHUTDOWN);
 burst(robot_defs[MOTHER].x+12,145,18,2,1);
}
static void ending_camera(int target){
 game.cam_q8+=(target*Q8-game.cam_q8)/6;
 if(iabs(target*Q8-game.cam_q8)<Q8)game.cam_q8=target*Q8;
 game.cam=game.cam_q8/Q8;
}
static void ending_move(int velocity){
 int was_ground=game.ground,old_x=game.x,old_y=game.y;
 game.vx=velocity;game.x=imax(12*Q8,game.x+velocity);
 game.vy=imin(1700,game.vy+72);int fall_speed=game.vy;game.y+=game.vy;
 game.ground=0;game.support=-1;
 for(int i=0;i<terrain_count[3];i++){
  const platform_t *p=terrains[3]+i;
  if(game.vy>=0&&old_y+PLAYER_H*Q8<=p->y*Q8&&game.y+PLAYER_H*Q8>=p->y*Q8&&game.x+PLAYER_W*Q8>p->x*Q8&&game.x<(p->x+p->w)*Q8){
   game.y=(p->y-PLAYER_H)*Q8;game.vy=0;game.ground=1;game.support=i;
  }
 }
 if(game.y>180*Q8){game.y=180*Q8;game.vy=0;game.ground=1;game.support=0;}
 if(game.ground){
  if(!was_ground&&fall_speed>220)game.anim_land=8;
  game.anim_stride=(game.anim_stride+iabs(game.x-old_x))%(40*Q8);
 }else game.anim_land=0;
 if(game.x==12*Q8)game.vx=0;
}
static void ending_step(void){
 if(game.ending_phase==ENDING_FOREST)game.party_tick=(game.party_tick+1)%PARTY_TICKS;
 game.steps++;game.ending_tick++;
 if(game.notice)game.notice--;if(game.anim_land)game.anim_land--;
 if(game.shake)game.shake--;
 for(int i=0;i<NPARTICLE;i++){
  particle_t *p=game.particles+i;
  if(p->life){p->x+=p->vx;p->y+=p->vy;p->vy+=24;p->life--;}
 }
 if(game.ending_phase==ENDING_SHUTDOWN){
  ending_move(0);ending_camera(scene_width[3]-240);
  game.boss_end=imax(0,ENDING_SHUTDOWN_TICKS-game.ending_tick);
  if(game.ending_tick<150&&game.ending_tick%18==0){
   burst(robot_defs[MOTHER].x+12,108+(game.ending_tick%70),8,0,2);game_sound(S_HIT);
  }
  if(game.ending_tick>=ENDING_SHUTDOWN_TICKS){game.dir=-1;ending_phase(ENDING_ESCAPE);}
 }else if(game.ending_phase==ENDING_ESCAPE){
  if(game.ending_tick%18==1)game_sound(S_ALARM);
  if(game.ending_tick%45==0){burst(robot_defs[MOTHER].x+12,120,6,0,2);game_sound(S_HIT);}
  game.dir=-1;ending_move(-650);
  ending_camera(imax(0,imin(scene_width[3]-240,game.x/Q8-82)));
  if(game.x<=12*Q8&&game.ending_tick>=75&&game.cam<2){clear_effects();ending_phase(ENDING_CITY);}
 }else if(game.ending_phase==ENDING_CITY){
  if(game.ending_tick>=ENDING_PANEL_TICKS)ending_phase(ENDING_FOREST);
 }else if(game.ending_phase==ENDING_FOREST&&game.ending_tick>=ENDING_PANEL_TICKS){
  ending_phase(ENDING_DONE);game.won=1;game.input_lock=1;
  game_sound(S_WIN);
 }
}
static void ending_restore(int phase,int tick,int x,int y,int vy,int camera,int stride){
 set_scene(3,x/Q8,y/Q8,0);clear_effects();
 game.ending_phase=phase;game.ending_tick=tick;game.x=x;game.y=y;game.vy=vy;
 game.cam_q8=camera;game.cam=camera/Q8;game.anim_stride=stride;game.anim_land=0;
 game.dir=phase==ENDING_ESCAPE?-1:1;game.vx=phase==ENDING_ESCAPE?-650:0;
 game.ground=y==180*Q8;game.support=game.ground?0:-1;
 game.boss_end=phase==ENDING_SHUTDOWN?ENDING_SHUTDOWN_TICKS-tick:0;
 game.dead=game.transition=game.invuln=game.factory_timer=0;game.hp=imax(1,game.hp);
 game.won=phase==ENDING_DONE;game.input_lock=game.won;
 for(int i=WORKER_FIRST;i<WORKER_LAST;i++)game.robots[i].hp=0;
 ending_music();if(phase!=ENDING_ESCAPE)game_alarm_stop();
}
