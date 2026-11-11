/* SPDX-License-Identifier: GPL-3.0-only
 * Original one-use health logs. Uses reserved saved collection bits, never
 * changes locomotion, projectile fire, collision or the input assignments.
 */
#pragma once
#define NLOG 17
#define LOG_BIT_BASE 64
#define LOG_RESPONSE_TICKS 18 /* Brief visual response; never an input lock. */
typedef struct {int scene,x,y;} log_def_t; /* center x, supporting floor y */
static const log_def_t log_defs[NLOG]={
 {0,195,212},{0,1395,194},{0,2185,212},
 {1,95,212},{1,1845,212},{1,2690,192},
 {2,145,212},{2,1205,184},{2,1885,212},{2,3240,212},
 {3,165,212},{4,155,212},{7,170,212},{7,1120,200},{7,1680,212},{7,1820,212},{7,1930,212}
};
static int log_visible(int i){return log_defs[i].scene==game.scene&&!collected(LOG_BIT_BASE+i);}
/* Test the moving player box against a log, including contact between steps.
 * Intersect both axes at the same time so a diagonal near-miss stays a miss. */
static int log_sweep_axis(int a,int b,int lo,int hi,int *enter,int *leave){
 if(a==b)return a>lo&&a<hi;
 int t0=(lo-a)*1024/(b-a),t1=(hi-a)*1024/(b-a);
 if(t0>t1){int t=t0;t0=t1;t1=t;}
 *enter=imax(*enter,t0);*leave=imin(*leave,t1);
 return *enter<*leave;
}
static int log_crossed(int i,int before_x,int before_y){
 const log_def_t *p=log_defs+i;int enter=0,leave=1024;
 return log_sweep_axis(before_x,game.x,(p->x-13-PLAYER_W)*Q8,(p->x+13)*Q8,&enter,&leave)
     &&log_sweep_axis(before_y,game.y,(p->y-9-PLAYER_H)*Q8,p->y*Q8,&enter,&leave);
}
static void update_logs(int before_x,int before_y){
 if(game.dead||game.won||game.pause||game.input_lock)return;
 if(game.log_chop&&!--game.log_chop)game.log_target=-1;
 for(int i=0;i<NLOG;i++)if(log_visible(i)&&log_crossed(i,before_x,before_y)){
  if(game.hp>=3){
   if(!game.notice||game.save_status==7){game.save_status=7;game.notice=55;}
   continue;
  }
  /* Commit the one-use bit and heart together before the cosmetic response.
   * Continued contact, saving mid-response and later damage cannot reuse it. */
  collect_bit(LOG_BIT_BASE+i);game.hp=imin(3,game.hp+1);
  game.log_target=i;game.log_chop=LOG_RESPONSE_TICKS;
  const log_def_t *p=log_defs+i;
  burst(p->x,p->y-5,10,0,5);game_sound(S_SNACK);game_voice(5);
  game.save_pending=1;game.save_requested=0;game.save_status=8;game.notice=100;
  break;
 }
}
