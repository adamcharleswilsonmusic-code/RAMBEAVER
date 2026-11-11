/* SPDX-License-Identifier: GPL-3.0-only
 * Original bounded factory-head encounter, 2026-10-13.
 * Keeps the three type-3 save slots and two hit points. State 4 is a
 * transient, harmless hatch exit; no new persistent state is required. */
#pragma once
enum {FACTORY_EMERGE=4,FACTORY_EXIT_TICKS=56,FACTORY_CADENCE=88};
static int factory_spawn_x(void){return game.robots[MOTHER].x+4;}
static int factory_hatch_x(void){return game.robots[MOTHER].x-5;}
static int factory_free_slot(void){
 for(int i=WORKER_FIRST;i<WORKER_LAST;i++)
  if(!game.robots[i].hp&&!game.robots[i].flash&&!posse_reserved(i))return i;
 return -1;
}
static int factory_hatch_aperture(void){
 if(!game.robots[MOTHER].hp||game.ending_phase)return 0;
 for(int i=WORKER_FIRST;i<WORKER_LAST;i++)
  if(game.robots[i].hp&&game.robots[i].state==FACTORY_EMERGE)return 18;
 if(game.x/Q8<=230||factory_free_slot()<0||game.factory_timer>24)return 0;
 return imin(18,(24-imax(0,game.factory_timer))*18/24);
}
static void factory_update(void){
 if(game.scene!=3||!game.robots[MOTHER].hp||game.ending_phase||game.dead||game.won||game.x/Q8<=230)return;
 if(game.factory_timer>0)game.factory_timer--;
 if(game.factory_timer)return;
 int slot=factory_free_slot();
 if(slot<0){game.factory_timer=24;return;}
 /* Do not put a fresh head on top of another one still leaving the hatch. */
 for(int i=WORKER_FIRST;i<WORKER_LAST;i++){
  robot_t *r=game.robots+i;
  if(r->hp&&iabs(r->x-factory_spawn_x())<34&&iabs(r->y-180)<30){game.factory_timer=24;return;}
 }
 reward_spawn(slot);game.robot_voice_seen[slot/32]&=~(1u<<(slot%32));game.robots[slot]=(robot_t){factory_spawn_x(),180,FACTORY_EMERGE,FACTORY_EXIT_TICKS,2,-1,0,0,198};
 game.factory_spawns++;game.factory_timer=FACTORY_CADENCE;game_sound(S_TELL);
}
static int factory_move_toward(int value,int target,int speed){
 return value+imin(speed,imax(-speed,target-value));
}
static void factory_flyer_step(int i){
 robot_t *r=game.robots+i;
 if(!r->hp)return;
 if(!game.robots[MOTHER].hp||game.ending_phase){r->hp=0;return;}
 int px=game.x/Q8,py=game.y/Q8,dist=iabs(r->x-px);
 int target_y=imin(184,imax(110,py+5+tri((int)game.steps+i*9,24)/3));
 if(r->state==FACTORY_EMERGE){
  /* First leave the belly visibly. Then keep the harmless exit state until
   * there is a clear gap from the player, even if they stand at the outlet. */
  if(r->timer>0){r->timer--;r->x--;}
  else if(dist<56)r->x+=r->x>=px?2:-2;
  r->x=imin(580,imax(48,r->x));r->y=factory_move_toward(r->y,target_y,1);
  r->dir=px<r->x?-1:1;
  if(!r->timer&&iabs(r->x-px)>=56){r->state=0;r->timer=18;}
  return; /* Emergence never shoots or deals contact damage. */
 }
 if(r->state==0||r->state==3){
  int side=r->x>=px?1:-1;
  int target_x=imin(580,imax(48,px+side*(92+(i-WORKER_FIRST)*8)));
  r->x=factory_move_toward(r->x,target_x,2);
  r->y=factory_move_toward(r->y,target_y,2);
  r->dir=px<r->x?-1:1;
 }else if(r->state==1){
  /* Aim is committed at the tell, not tracked while charging. */
  r->y=factory_move_toward(r->y,r->aim_y-18,2);
 }
 if(r->state==0){
  if(r->timer>0)r->timer--;
  dist=iabs(r->x-px);
  if(!r->timer&&dist>=54&&dist<=210&&iabs(r->y-target_y)<8){
   r->state=1;r->timer=60;r->aim_y=imin(202,imax(128,py+18));game_sound(S_TELL);robot_alert_voice(i);
  }
 }else if(--r->timer<=0){
  if(r->state==1){
   r->state=2;r->timer=10;
   bullet(r->x+12+2*r->dir,r->y+18,r->dir*5,1,0);
   r->shots++;game.attacks++;game_sound(S_LASER);
  }else if(r->state==2){r->state=3;r->timer=54;}
  else {r->state=0;r->timer=18;}
 }
 if(iabs(r->x-px)<21&&iabs(r->y-py)<28)damage();
}
