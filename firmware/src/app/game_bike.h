/* SPDX-License-Identifier: GPL-3.0-only
 * Original finite road run between the two city districts. Auto forward,
 * ordinary Jump edge, ten fleeing ground groups, two ramps and harmless smash-through heads, no new input assignment. */
#pragma once
#define BIKE_OBSTACLES 10
static const int bike_obstacle_x[BIKE_OBSTACLES]={250,330,410,490,990,1070,1150,1630,1710,1770};
static const int bike_obstacle_h[3]={36,44,40};
static const int bike_obstacle_w[3]={30,52,30};
#define BIKE_END_X 1850
static const int bike_ramp_x[2]={600,1220},bike_ramp_w[2]={90,110},bike_ramp_h[2]={36,50};
static int bike_floor(int x){
 for(int n=0;n<2;n++)if(x>=bike_ramp_x[n]&&x<bike_ramp_x[n]+bike_ramp_w[n])return 212-(x-bike_ramp_x[n])*bike_ramp_h[n]/bike_ramp_w[n];
 return 212;
}
/* Explosions live in the particle pool, so a dead bike actor can be reused
 * while its smoke fades; posse reservations still retain ownership. */
static int bike_free_slot(void){for(int i=WORKER_FIRST;i<WORKER_LAST;i++)if(!game.robots[i].hp&&!posse_reserved(i))return i;return -1;}
static void bike_heads_step(void){
 /* Ground groups have priority when a pooled slot opens. All begin beyond
  * the visible right edge; tagged slots use the ordinary gun/posse/kill path. */
 for(int n=0;n<BIKE_OBSTACLES;n++)if(!(game.bike_hit_mask&(1<<n))&&game.x/Q8>=bike_obstacle_x[n]-192){
  int active=0;for(int i=WORKER_FIRST;i<WORKER_LAST;i++)active|=game.robots[i].hp&&game.robots[i].shots==((n+1)<<1);
  if(!active){int i=bike_free_slot();if(i>=0){reward_spawn(i);game.robots[i]=(robot_t){imax(bike_obstacle_x[n],game.cam+246),212-bike_obstacle_h[n%3],0,0,1,1,0,(n+1)<<1,0};}}
 }
 static const int trigger[6]={0,460,660,1090,1300,1510},height[6]={174,150,122,165,106,174};
 if(game.bike_head_wave<6&&game.x/Q8>=trigger[game.bike_head_wave]){
  int i=bike_free_slot();if(i>=0){
   int n=game.bike_head_wave++;reward_spawn(i);game.robots[i]=(robot_t){game.cam+275,height[n],0,0,1,-1,0,1,height[n]};
  }
 }
 for(int i=WORKER_FIRST;i<WORKER_LAST;i++){
  robot_t *r=game.robots+i;if(r->flash)r->flash--;if(!r->hp)continue;
  int kind=r->shots&1;if(kind){r->x--;r->y=r->aim_y+(tri(game.bike_tick+i*13,48)-12)/4;}
  else {r->x++;r->y=bike_floor(r->x+10)-bike_obstacle_h[((r->shots>>1)-1)%3];} /* One pixel/tick fleeing; bike closes at two. */
  if(!kind&&robot_on_screen(i)){int beat=game.x/Q8<800?0:1,window=beat?game.x/Q8>=1400:game.x/Q8>=140;if(window&&!(game.bike_hit_mask&(1<<(16+beat)))){game.bike_hit_mask|=1<<(16+beat);game_voice(1);}}
  /* No contact damage or hostile shots in this mode. Rider and both wheels
   * form the collision silhouette, so a clean airborne hit also explodes. */
  if(r->x<game.x/Q8+64&&r->x+(kind?27:bike_obstacle_w[((r->shots>>1)-1)%3])>game.x/Q8-15&&r->y<game.y/Q8+44&&r->y+(kind?28:bike_obstacle_h[((r->shots>>1)-1)%3])>game.y/Q8-8)kill_robot(i);
  else if(r->x+28<game.cam){reward_discard(i);if(!kind)game.bike_hit_mask|=1<<((r->shots>>1)-1);r->hp=0;}
 }
}
static void bike_clear_heads(void){for(int i=WORKER_FIRST;i<WORKER_LAST;i++){reward_discard(i);game.robots[i]=(robot_t){0};}}
static void bike_start(int mount){
 if(mount)stage_clear(0);clear_effects();bike_clear_heads();game.bike_done=0;game.bike_phase=mount?BIKE_MOUNT:BIKE_RIDE;game.bike_tick=game.bike_hit_mask=game.bike_head_wave=game.bike_jump_tick=game.bike_land_tick=0;game.bike_start_x=game.x;
 game.checkpoint=BIKE_CP;game.cp_scene=BIKE_SCENE;game.cp_x=58;game.cp_y=168;
 int first=!(game.visited&(1u<<BIKE_SCENE));if(!mount){set_scene(BIKE_SCENE,58,168,0);if(first)arrival_start(2,-80,58,168);}
 game.hurt_flash=0;game.dir=1;game.vx=game.vy=game.duck=game.gun=game.recoil=game.fireclock=0;game.ground=!mount;game.transition=0;game.support=0;game.drop_timer=0;game.drop_platform=-1;
 game_music_variant(0);game_music(game.music);game.save_pending=1;game.save_requested=0;
}
static void bike_finish(void){
 stage_clear(BIKE_SCENE);clear_effects();game.bike_done=1;game.bike_phase=BIKE_DISMOUNT;game.bike_tick=game.bike_jump_tick=game.bike_land_tick=0;
 set_scene(1,32,168,0);if(game.arrival)arrival_start(2,-80,32,168);game.transition=0;game.checkpoint=INNER_START_CP;game.cp_scene=1;game.cp_x=32;game.cp_y=180;
 game.ground=0;game.vy=game.vx=0;game.save_pending=1;game.save_requested=0;
}
static void bike_step(uint32_t k,uint32_t e){
 game.steps++;game.bike_tick++;if(game.bike_land_tick)game.bike_land_tick--;game.dir=1;game.duck=0;
 if(game.pop_clock)game.pop_clock--;if(game.invuln)game.invuln--;if(game.hurt_flash)game.hurt_flash--;if(game.shake)game.shake--;if(game.recoil)game.recoil--;
 if(game.charge<CREW_TICKS)game.charge++;if(game.notice)game.notice--;
 for(int i=0;i<NPARTICLE;i++){particle_t *p=game.particles+i;if(p->life){p->x+=p->vx;p->y+=p->vy;if(p->type<5||p->type==6)p->vy+=24;p->life--;}}
 if(game.bike_phase==BIKE_MOUNT){int t=imin(64,game.bike_tick);game.x=game.bike_start_x+(3560*Q8-game.bike_start_x)*t/64;game.y=(180-48*t*(64-t)/1024-12*t/64)*Q8;game.ground=0;if(t==64)bike_start(0);return;}
 if(game.bike_phase==BIKE_DISMOUNT){int t=imin(32,game.bike_tick);game.x=(32+50*t/32)*Q8;game.y=(168-28*t*(32-t)/256+12*t/32)*Q8;game.ground=0;if(t==32){game.bike_phase=0;game.ground=1;game.support=0;game.input_lock=1;}return;}
 if(game.bike_phase==BIKE_EXIT){game.x+=4*Q8;game.y=168*Q8;game.ground=1;if(game.x>(game.cam+268)*Q8)bike_finish();return;}
 int oldfloor=bike_floor(game.x/Q8);game.vx=3*Q8;game.x+=game.vx;int floor=bike_floor(game.x/Q8);
 if(game.ground){
  if(oldfloor<212&&floor==212){game.vy=oldfloor<170?-1750:-1500;game.ground=0;game.bike_jump_tick=4;game.bike_land_tick=0;game_sound(S_JUMP);game_voice(3);}
  else game.y=(floor-44)*Q8;
 }
 if((e&JUMP)&&game.ground&&!game.bike_jump_tick){game.bike_jump_tick=-3;game.bike_land_tick=0;}
 if(game.ground&&game.bike_jump_tick<0&&++game.bike_jump_tick==0){game.vy=-2100;game.ground=0;game.bike_jump_tick=4;game_sound(S_JUMP);game_voice(3);}
 if(!game.ground){game.bike_jump_tick++;game.vy+=72;game.y+=game.vy;if(game.y>=(floor-44)*Q8&&game.vy>=0){game.y=(floor-44)*Q8;game.vy=0;game.ground=1;game.bike_jump_tick=0;game.bike_land_tick=8;}}

 game.cam=imax(0,imin(scene_width[BIKE_SCENE]-240,game.x/Q8-58));game.cam_q8=game.cam*Q8;
 if(game.x>=BIKE_END_X*Q8){game.bike_phase=BIKE_EXIT;game.bike_tick=0;return;}
 if(k&FIRE){game.gun=6;if(!game.fireclock){bullet(game.x/Q8+29,player_shot_y(),9,0,0);game.fireclock=6;game.recoil=4;game.shots++;game_sound(S_GUN);}}else if(game.gun)game.gun--;if(game.fireclock)game.fireclock--;
 if(e&CREW)posse_begin();
 if((e&BOMB)&&game.bottles>0&&!game.bomb.stage&&!game.bomb.cooldown){game.bottles--;game.throws++;game.bomb=(bomb_t){game.x+12*Q8,game.y+10*Q8,680,-980,1,80,0};game_sound(S_SWARM);game_voice(2);}
 bike_heads_step();posse_step();update_bullets();update_bomb();
}
