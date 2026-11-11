/* SPDX-License-Identifier: GPL-3.0-only
 * Original fixed-step platform game. World state is bounded and independent
 * of drawing/audio; all attacks use held/edge inputs and no aiming control. */
#include "levels.h"
#include <stddef.h>
#define Q8 256
#define WORLD 3840
#define PLAYER_W 24
#define PLAYER_H 32
#define NBULLET 24
#define NPARTICLE 80
#define LEFT (1u<<0)
#define RIGHT (1u<<2)
#define DOOR (1u<<1)
#define DOWN (1u<<23)
#define JUMP (1u<<24)
#define FIRE (1u<<26)
#define CREW (1u<<22)
#define BOMB (1u<<25)
#define BOTTLE_CAP 3
#define PARTY_TICKS 1536
#define DAM_BUILD_TICKS 1375 /* 22 seconds, never blocked by enemies. */
#define DAM_WAVES 9
#define DAM_DIVERT_TICKS 144
enum {DAM_WALK_OFF=1,DAM_WALK_IN,DAM_WALK_RETURN};
#define CREW_TICKS 750 /* 12 seconds of active 16 ms game steps. */
#define ACTIONS (LEFT|RIGHT|DOOR|DOWN|JUMP|FIRE|CREW|BOMB)
enum {BIKE_MOUNT=1,BIKE_RIDE,BIKE_EXIT,BIKE_DISMOUNT};
enum {FLIGHT_HEAD_QUEUED=4};
enum {FLIGHT_TAKEOFF=1,FLIGHT_CRUISE,FLIGHT_EXIT,FLIGHT_LAND,FLIGHT_DROP};
#define FLIGHT_TOP 40
#define FLIGHT_BOTTOM 176
#define FLIGHT_END_X 3500
static const uint8_t WHITE_K[16]={0,2,4,6,7,9,11,12,14,16,18,19,21,23,24,26};
static const uint8_t BLACK_K[11]={1,3,5,8,10,13,15,17,20,22,25};
static int imin(int a,int b){return a<b?a:b;}static int imax(int a,int b){return a>b?a:b;}static int iabs(int a){return a<0?-a:a;}
static int tri(int v,int period){v%=period;if(v<0)v+=period;return v<period/2?v:period-v;}
typedef struct {int x,y,state,timer,hp,dir,flash,shots,aim_y;} robot_t;
typedef struct {int x,y,vx,owner,life,type;} bullet_t;
typedef struct {int x,y,vx,vy,stage,timer,cooldown;} bomb_t;
typedef struct {int x,y,vx,vy,life,type,color;} particle_t;
typedef struct {
 int x,y,vx,vy,dir,hp,ground,coyote,buffer,invuln,hurt_flash,gun,recoil,fireclock;
 int support,drop_platform,drop_timer,duck;
 int log_target,log_chop; /* Consumed log and cosmetic gnaw countdown. */
 int anim_stride,anim_land; /* Visual state only: distance phase and landing settle. */
 int pause,menu,dead,won,scene,district,room,checkpoint,cp_scene,cp_x,cp_y;
 int charge,bottles,crew,crewtarget,cam,cam_q8,shake,music;
 int crew_count;uint8_t crew_ids[NROB];int16_t crew_x[NROB],crew_y[NROB];
 int return_scene,return_x,return_y,input_lock,transition,boss_end,factory_timer;
 int ending_phase,ending_tick,intro_phase,intro_tick,party_tick;
 int flight_phase,flight_tick,flight_thrust,flight_done,flight_timer,flight_wave;
 int bike_phase,bike_tick,bike_done,bike_hit_mask,bike_start_x,bike_jump_tick,bike_land_tick,bike_head_wave;
 int kill_voice_count;uint32_t robot_voice_seen[2];
 int notice,save_pending,save_status,save_requested;
 /*743 reward ledger: stable authored bits, unique serial per pooled spawn. */
 /* gold_reveal:0 ordinary finish,1 exclusive earned unlock,2 confirmed party. */
 int golden,run_valid,gold_reveal;uint32_t authored_kills[2],forest_kills,spawn_serial,spawn_kills,spawn_id[3];
 uint32_t stage_spawns[9],stage_kills[9],stage_damage,stage_missed;int flawless_tick;
 uint32_t visited;int arrival,arrival_target,arrival_y,arrival_tick;
 int forest_done,forest_mask,dam_phase,dam_tick,dam_progress,dam_wave,dam_wait,dam_blocked,dam_completions,pop_clock,dam_entry,dam_entry_tick,dam_entry_cam;
 uint32_t collected[PICK_WORDS],steps,shots,hits,attacks,crews,throws,factory_spawns;
 robot_t robots[NROB];bullet_t bullets[NBULLET];bomb_t bomb;particle_t particles[NPARTICLE];
} game_t;
static game_t game;
static uint32_t old_keys,old_buttons,edges,bedges,clock_last,accumulator,draw_last;
static int menu_turn;
static void render(void);
static void flight_start(int launch);
static void bike_start(int mount);
static void bike_step(uint32_t k,uint32_t e);
static void forest_begin_defense(void);
static void forest_update(void);
static void forest_depart(void);
static void forest_entry_step(void);
static void forest_on_kill(int i);
static void robot_alert_voice(int i);
static int player_crouched(void){return game.duck&&!game.dead&&game.ground&&!game.drop_timer&&!game.flight_phase&&!game.bike_phase;}
static int player_hit_top(void){return game.y/Q8+(player_crouched()?20:0);}
static int player_shot_y(void){return game.y/Q8+(player_crouched()?27:23);}
static int player_robot_vertical(int y,int h){return player_crouched()?player_hit_top()<y+h&&game.y/Q8+PLAYER_H>y:iabs(y-game.y/Q8)<28;}
static int collected(int i){return !!(game.collected[(unsigned)i/32]&(1u<<(i%32)));}
static void collect_bit(int i){game.collected[(unsigned)i/32]|=1u<<(i%32);}
static int robot_here(int i){if(game.scene==6&&game.robots[i].state==FLIGHT_HEAD_QUEUED)return 0;return (game.scene==6||game.scene==FOREST_SCENE||game.scene==BIKE_SCENE)?(i>=WORKER_FIRST&&i<WORKER_LAST):robot_defs[i].scene==game.scene;}
static int robot_kind(int i){return game.scene==BIKE_SCENE||game.scene==FOREST_SCENE?(game.robots[i].shots&1):robot_defs[i].type;}
static int snack_visible(int i){return snacks[i].scene==game.scene&&!collected(i)&&(snacks[i].drop<0||!game.robots[snacks[i].drop].hp);}
static int floor_at(int scene,int x){for(int i=0;i<floor_count[scene];i++){const platform_t *p=terrains[scene]+i;if(x>=p->x&&x<p->x+p->w)return p->y;}return 212;}
static void particle(int x,int y,int vx,int vy,int type,int color,int life){for(int i=0;i<NPARTICLE;i++)if(!game.particles[i].life){game.particles[i]=(particle_t){x*Q8,y*Q8,vx,vy,life,type,color};return;}}
static void burst(int x,int y,int count,int type,int color){for(int i=0;i<count;i++)particle(x,y,((i*137+game.steps*17)%701)-350,-180-(i*71)%650,type,color,22+(i*7)%24);}
static void clear_effects(void){for(int i=0;i<NBULLET;i++)game.bullets[i].life=0;for(int i=0;i<NPARTICLE;i++)game.particles[i].life=0;game.bomb=(bomb_t){0};game.crew=game.crew_count=0;}
static int forest_wide_view(void){return game.scene==FOREST_SCENE&&((game.dam_phase==1&&game.dam_entry!=DAM_WALK_OFF)||game.dam_phase==2);}
static int game_view_width(void){return forest_wide_view()?400:240;}
static int camera_target(int scene,int x){
 if(scene==FOREST_SCENE){
  if(game.dam_entry==DAM_WALK_OFF)return game.dam_entry_cam;
  if(game.dam_entry==DAM_WALK_RETURN)return 2110;
  if(game.dam_phase==1||game.dam_phase==2)return 1740;
  if(!game.forest_done)return imin(1700,imax(0,x-82));
 }
 int offset=82;
 if(scene==3)offset-=imin(58,imax(0,(x-260)*2/3));
 return imax(0,imin(scene_width[scene]-240,x-offset));
}
static void reward_miss(void){game.run_valid=0;}
static void reward_discard(int i){if(i>=WORKER_FIRST&&i<WORKER_LAST){int n=i-WORKER_FIRST;if(game.spawn_id[n]){reward_miss();game.stage_missed|=1u<<game.scene;}game.spawn_id[n]=0;}}
static void reward_spawn(int i){reward_discard(i);if(game.spawn_serial>=1000000u){reward_miss();return;}game.spawn_id[i-WORKER_FIRST]=++game.spawn_serial;game.stage_spawns[game.scene]++;}
static void reward_kill(int i){
 if(i>=WORKER_FIRST&&i<WORKER_LAST){int n=i-WORKER_FIRST;if(game.spawn_id[n]){game.spawn_kills++;game.stage_kills[game.scene]++;game.spawn_id[n]=0;}
 if(game.scene==FOREST_SCENE&&!game.dam_phase){int id=(game.robots[i].shots>>1)-1;if(id>=0&&id<4)game.forest_kills|=1u<<id;}}
 else game.authored_kills[i/32]|=1u<<(i%32);
}
static int stage_clean(int scene){
 uint32_t mask=1u<<scene;if(scene==0)mask|=1u<<4;if(scene==1)mask|=1u<<5;
 if((game.stage_damage|game.stage_missed)&mask)return 0;
 for(int i=0;i<NROB;i++)if(robot_defs[i].type!=3&&(mask&(1u<<robot_defs[i].scene))&&!(game.authored_kills[i/32]&(1u<<(i%32))))return 0;
 if(scene==FOREST_SCENE&&game.forest_kills!=15)return 0;
 return game.stage_spawns[scene]==game.stage_kills[scene];
}
static void stage_clear(int scene){if(stage_clean(scene))game.flawless_tick=70;else reward_miss();}
static void reward_finish(void){
 if(!game.run_valid||game.hits||!game.forest_done||!game.bike_done||!game.flight_done||game.forest_kills!=15||game.spawn_serial!=game.spawn_kills)return;
 for(int i=0;i<NROB;i++)if(robot_defs[i].type!=3&&!(game.authored_kills[i/32]&(1u<<(i%32))))return;
 if(!game.golden){game.golden=game.gold_reveal=1;game.save_pending=1;}
}
static void arrival_start(int mode,int start,int target,int y){
 game.arrival=mode;game.arrival_target=target*Q8;game.arrival_y=y*Q8;game.arrival_tick=0;
 game.x=start*Q8;game.y=y*Q8;game.hurt_flash=0;game.invuln=60;game.input_lock=0;
 game.vx=game.vy=0;game.ground=mode!=3;game.dir=start>target?-1:1;
}
static void arrival_step(void){
 game.steps++;game.arrival_tick++;int speed=(game.room||(game.scene==2&&game.arrival_target==32*Q8))?Q8:3*Q8;int d=game.arrival_target-game.x,dx=imin(speed,imax(-speed,d));
 game.x+=dx;game.vx=dx;game.anim_stride=(game.anim_stride+iabs(dx))%(40*Q8);game.y=game.arrival_y;
 game.hurt_flash=0;game.invuln=60;
 if(game.arrival==3){game.flight_thrust=1;game_jet(1);}
 if(game.x==game.arrival_target){game.arrival=0;game.vx=0;game.input_lock=1;edges=0;game.save_pending=1;}
}
static void set_scene(int scene,int x,int y,int lock){int previous_scene=game.scene,first=!(game.visited&(1u<<scene));game.visited|=1u<<scene;game.arrival=0;game_engine(0);game.scene=scene;game.district=scene==1;game.room=scene==4||scene==5;game.x=x*Q8;game.y=y*Q8;game.vx=game.vy=game.buffer=game.gun=0;game.ground=0;game.support=game.drop_platform=-1;game.drop_timer=game.duck=game.hurt_flash=0;game.anim_stride=game.anim_land=0;game.log_target=-1;game.log_chop=0;game.input_lock=lock;game.invuln=imax(60,game.invuln);game.transition=70;game_music_variant(scene==BIKE_SCENE?0:scene==FOREST_SCENE?7:scene==6?5:scene==3&&!game.robots[MOTHER].hp?2:scene<4?scene:scene==4?0:1);game.cam=camera_target(scene,x);game.cam_q8=game.cam*Q8;clear_effects();for(int i=0;i<NROB;i++){game.robots[i].state=0;game.robots[i].timer=40;}/* Beat continues across traversal; only visual/world transients reset. */
 if(first&&scene!=6&&scene!=BIKE_SCENE){int start=scene==4||scene==5||(scene==2&&previous_scene==6)?12:x>scene_width[scene]/2?scene_width[scene]+40:game.cam-52;arrival_start(1,start,x,y);}
}
#include "game_ending.h"
#include "game_intro.h"
static void world_reset(int fresh){
 int music=fresh?1:game.music,charge=game.charge,bottles=game.bottles,cp=game.checkpoint,rs=game.return_scene,rx=game.return_x,ry=game.return_y;
 int flight_retry=game.flight_phase&&!game.flight_done,flight_done=game.flight_done;
 int kills=game.kill_voice_count,gold=game.golden;uint32_t visited=game.visited,missed=game.stage_missed|(1u<<game.scene),damage_mask=game.stage_damage;
 int fd=game.forest_done,fm=game.forest_mask,bd=game.bike_done,br=game.bike_phase&&!game.bike_done;
 uint32_t picks[PICK_WORDS];uint8_t hp[NROB];for(int i=0;i<PICK_WORDS;i++)picks[i]=game.collected[i];for(int i=0;i<NROB;i++)hp[i]=(uint8_t)game.robots[i].hp;
 game=(game_t){0};game.golden=gold;game.run_valid=fresh;game.stage_missed=fresh?0:missed;game.stage_damage=fresh?0:damage_mask;game.visited=fresh?0:visited;if(fresh)game.checkpoint=FOREST_START_CP;game.dir=1;game.hp=3;game.music=music;game.factory_timer=80;
 for(int i=0;i<NROB;i++){const robot_def_t *d=robot_defs+i;game.robots[i]=(robot_t){d->x,d->y,0,40,fresh?d->hp:hp[i],-1,0,0,d->y+12};if(d->type==3)game.robots[i].hp=0;if(i==MOTHER)game.robots[i].hp=d->hp;}
 if(!fresh){game.kill_voice_count=kills;game.charge=charge;game.bottles=bottles;game.checkpoint=cp;game.return_scene=rs;game.return_x=rx;game.return_y=ry;game.flight_done=flight_done;game.forest_done=fd;game.forest_mask=fm;game.bike_done=bd;for(int i=0;i<PICK_WORDS;i++)game.collected[i]=picks[i];}
 if(!fresh&&game.checkpoint>=FOREST_START_CP&&game.checkpoint<=FOREST_EXIT_CP&&game.forest_done)game.checkpoint=FOREST_EXIT_CP;
 const checkpoint_t *c=checkpoints+game.checkpoint;game.cp_scene=c->scene;game.cp_x=c->x;game.cp_y=c->y-PLAYER_H;set_scene(c->scene,c->x,c->y-PLAYER_H,0);game.notice=140;om_panic();if(fresh)game_music_restart();game_music(game.music);
 if(!fresh&&flight_retry)flight_start(0);
 if(!fresh&&(br||game.checkpoint==BIKE_CP)&&!game.bike_done)bike_start(0);
 if(game.scene==FOREST_SCENE){if(game.checkpoint==FOREST_DAM_CP)forest_begin_defense();else if(game.forest_done)game.dam_phase=3;}
}
/* v8 appends the flight checkpoint to the unchanged 144-byte v7 record.
 * v7 appends cinematic progress to the unchanged 116-byte v4-v6 prefix.
 * v6 stores actual HP in reserved. Consumed logs remain bits64..75. */
typedef struct {
 uint32_t magic,version,collected[PICK_WORDS];
 int32_t checkpoint,reserved,charge,bottles,music,won,room_scene,return_scene,return_x,return_y;
 uint8_t hp[NROB];
 int32_t ending_phase,ending_tick,ending_x,ending_y,ending_vy,ending_cam,ending_stride;
 int32_t flight_state; /* 0 city approach, 1 restart flight, 2 landed. */
 int32_t forest_done,forest_mask,dam_phase,dam_tick,forest_x,forest_y;
 int32_t bike_state,party_tick;
 int32_t kill_voice_count;
 /* v12 exact gameplay resume excludes cosmetic particles; under one sector. */
 uint32_t golden,resume_hash;uint8_t resume[offsetof(game_t,particles)];
} game_save_t;
#define GAME_SAVE_V11_BYTES offsetof(game_save_t,golden)
typedef char save_fits_sector[(sizeof(game_save_t)<=PLAT_STORE_MAX)?1:-1];
static uint32_t resume_hash(const uint8_t *p,unsigned n){uint32_t h=2166136261u;while(n--)h=(h^*p++)*16777619u;return h;}

#define GAME_SAVE_V6_BYTES offsetof(game_save_t,ending_phase)
#define GAME_SAVE_V7_BYTES offsetof(game_save_t,flight_state)
#define GAME_SAVE_V8_BYTES offsetof(game_save_t,forest_done)
#define GAME_SAVE_V10_BYTES offsetof(game_save_t,kill_voice_count)
#define GAME_SAVE_V9_BYTES offsetof(game_save_t,bike_state)
static int save_game(void){
 game_save_t s;memset(&s,0,sizeof s);s.magic=0x50414e44u;s.version=12;s.kill_voice_count=game.kill_voice_count;s.party_tick=game.party_tick;s.bike_state=game.bike_done?2:game.bike_phase?1:0;s.reserved=game.hp;
 s.flight_state=game.flight_done?2:game.flight_phase?1:0;
 s.forest_done=game.forest_done;s.forest_mask=game.forest_mask;s.dam_phase=game.scene==FOREST_SCENE?game.dam_phase:0;s.dam_tick=game.dam_phase==2?game.dam_tick:0;
 if(game.scene==FOREST_SCENE){s.forest_x=game.x;s.forest_y=game.y;}
 for(int i=0;i<PICK_WORDS;i++)s.collected[i]=game.collected[i];
 s.checkpoint=game.checkpoint;s.charge=game.charge;s.bottles=game.bottles;s.music=game.music;s.won=game.won;
 s.room_scene=game.room?game.scene:-1;s.return_scene=game.return_scene;s.return_x=game.return_x;s.return_y=game.return_y;
 for(int i=0;i<NROB;i++)s.hp[i]=(uint8_t)game.robots[i].hp;
 if(game.ending_phase){s.ending_phase=game.ending_phase;s.ending_tick=game.ending_tick;s.ending_x=game.x;s.ending_y=game.y;s.ending_vy=game.vy;s.ending_cam=game.cam_q8;s.ending_stride=game.anim_stride;}
 s.golden=game.golden;memcpy(s.resume,&game,sizeof s.resume);s.resume_hash=resume_hash(s.resume,sizeof s.resume);
 int rc=plat_store_save(OBJ_PROJ,&s,sizeof s);game.save_status=rc?-1:1;game.notice=150;game.save_pending=0;return rc;
}
static int valid_save(const game_save_t *s){
 int legacy=s->version==4;
 if(s->version==12){
  if(s->golden>1||s->resume_hash!=resume_hash(s->resume,sizeof s->resume))return 0;
  game_t r={0};memcpy(&r,s->resume,sizeof s->resume);
  if(r.scene<0||r.scene>8||r.checkpoint<0||r.checkpoint>=NCP||r.hp<0||r.hp>3||r.run_valid<0||r.run_valid>1||r.golden!=(int)s->golden||r.gold_reveal<0||r.gold_reveal>2||(r.gold_reveal&&!r.golden)||r.visited>1023||r.arrival<0||r.arrival>3||r.crew_count<0||r.crew_count>NROB||r.log_target< -1||r.log_target>=17||r.spawn_kills>r.spawn_serial||r.spawn_serial>1000000||r.drop_platform< -1||r.drop_platform>=terrain_count[r.scene]||r.flight_phase<0||r.flight_phase>FLIGHT_DROP||r.bike_phase<0||r.bike_phase>BIKE_DISMOUNT)return 0;
  for(int i=0;i<r.crew_count;i++)if(r.crew_ids[i]>=NROB)return 0;
  for(int i=0;i<3;i++)if(r.spawn_id[i]>r.spawn_serial)return 0;
 }

 if(s->version>=11&&(s->kill_voice_count<0||s->kill_voice_count>1000000))return 0;
 if(s->magic!=0x50414e44u||s->version<4||s->version>12||s->checkpoint<0||s->checkpoint>=NCP||s->reserved<0||s->reserved>(legacy?999:s->version>=6?3:0)||s->charge<0||s->charge>(legacy?5:CREW_TICKS)||s->bottles<0||s->bottles>BOTTLE_CAP||s->music<0||s->music>1||s->won<0||s->won>1||s->room_scene<-1||s->room_scene>5||(s->room_scene>=0&&s->room_scene<4)||s->return_scene<0||s->return_scene>3||s->return_x<0||s->return_x>=scene_width[s->return_scene]||s->return_y<0||s->return_y>230)return 0;
 if(s->version<9&&s->checkpoint>=FOREST_START_CP)return 0;
 if(s->version<10&&s->checkpoint>=BIKE_CP)return 0;
 if(s->version>=10&&(s->party_tick<0||s->party_tick>=PARTY_TICKS))return 0;
 if(s->version>=10&&(s->bike_state<0||s->bike_state>2||(s->bike_state==1&&(s->checkpoint!=BIKE_CP||s->room_scene!=-1||s->flight_state==1||s->ending_phase||s->won))))return 0;
 for(int i=0;i<NROB;i++)if(s->hp[i]>(robot_defs[i].type==3?2:robot_defs[i].hp))return 0;
 if(s->version>=7){
  int phase=s->ending_phase,t=s->ending_tick;
  if(phase<0||phase>ENDING_DONE)return 0;
  if(phase){
   if(s->hp[MOTHER]||s->reserved<1||s->won!=(phase==ENDING_DONE)||s->ending_x<0||s->ending_x>(scene_width[3]-PLAYER_W)*Q8||s->ending_y<0||s->ending_y>212*Q8||s->ending_vy< -1600||s->ending_vy>1700||s->ending_cam<0||s->ending_cam>(scene_width[3]-240)*Q8||s->ending_stride<0||s->ending_stride>=40*Q8)return 0;
   if(t<0||(phase==ENDING_SHUTDOWN&&t>=ENDING_SHUTDOWN_TICKS)||(phase==ENDING_ESCAPE&&t>400)||((phase==ENDING_CITY||phase==ENDING_FOREST)&&t>=ENDING_PANEL_TICKS)||(phase==ENDING_DONE&&t))return 0;
  }
 }
 if(s->version>=8&&(s->flight_state<0||s->flight_state>2||(s->flight_state==1&&(s->checkpoint>10||s->room_scene!=-1||s->ending_phase||s->won))))return 0;
 if(s->version>=9){
  if(s->forest_done<0||s->forest_done>1||s->forest_mask<0||s->forest_mask>15||s->dam_phase<0||s->dam_phase>3||s->dam_tick<0||s->dam_tick>144)return 0;
  if((s->dam_phase==1&&s->forest_done)||(s->dam_phase>=2&&!s->forest_done))return 0;
  if(s->checkpoint>=FOREST_START_CP&&(s->forest_x<(s->version>=12?-80*Q8:0)||s->forest_x>(scene_width[FOREST_SCENE]+(s->version>=12?40:-PLAYER_W))*Q8||s->forest_y<0||s->forest_y>270*Q8))return 0;
  if(s->dam_phase&&s->checkpoint<FOREST_START_CP)return 0;
 }
 return 1;
}
void ui_init(void){
 game=(game_t){0};world_reset(1);game_save_t s;memset(&s,0,sizeof s);
 int n=plat_store_load(OBJ_PROJ,&s,sizeof s);
 if((n==sizeof s||(n==GAME_SAVE_V11_BYTES&&s.version==11)||(n==GAME_SAVE_V10_BYTES&&s.version==10)||(n==GAME_SAVE_V9_BYTES&&s.version==9)||(n==GAME_SAVE_V8_BYTES&&s.version==8)||(n==GAME_SAVE_V7_BYTES&&s.version==7)||(n==GAME_SAVE_V6_BYTES&&s.version<=6))&&valid_save(&s)){
  int legacy=s.version==4;game.kill_voice_count=s.version>=11?s.kill_voice_count:0;game.checkpoint=s.checkpoint;game.charge=legacy?s.charge*CREW_TICKS/5:s.charge;
  game.bottles=legacy?s.bottles/3:s.bottles;game.music=s.music;game.return_scene=s.return_scene;game.return_x=s.return_x;game.return_y=s.return_y;
  for(int i=0;i<PICK_WORDS;i++)game.collected[i]=legacy?0:s.collected[i];
  for(int i=0;i<NROB;i++)game.robots[i].hp=s.hp[i];
  game.forest_done=s.version>=9?s.forest_done:1;game.forest_mask=s.version>=9?s.forest_mask:15;
  game.flight_done=s.version>=8?s.flight_state==2:(checkpoints[s.checkpoint].scene==2||checkpoints[s.checkpoint].scene==3||s.won||s.ending_phase);
  game.bike_done=s.version>=10?s.bike_state==2:(s.checkpoint>=INNER_START_CP&&s.checkpoint<FOREST_START_CP);
  if(s.version>=10&&s.bike_state==1)game.bike_phase=BIKE_RIDE;
  if(s.version>=8&&s.flight_state==1)game.flight_phase=FLIGHT_CRUISE;
  world_reset(0);game.robots[MOTHER].hp=s.hp[MOTHER];
  /* Older builds allowed walking back from the facility into city rooms.
   * Resume those saves at their earned facility checkpoint: the old street
   * connection no longer exists, so the closed bay cannot strand a save. */
  if(s.room_scene>=4&&(!game.flight_done||checkpoints[s.checkpoint].scene<2))set_scene(s.room_scene,42,180,0);
  if(s.version>=9&&s.checkpoint>=FOREST_START_CP&&s.dam_phase==2){
   set_scene(FOREST_SCENE,s.forest_x/Q8,s.forest_y/Q8,0);game.checkpoint=FOREST_DAM_CP;game.cp_scene=FOREST_SCENE;game.cp_x=1810;game.cp_y=180;game.dam_phase=2;game.dam_tick=s.dam_tick;game.dam_progress=DAM_BUILD_TICKS;game.x=s.forest_x;game.y=s.forest_y;game.cam=camera_target(FOREST_SCENE,s.forest_x/Q8);game.cam_q8=game.cam*Q8;
  }
  game.party_tick=s.version>=10?s.party_tick:0;game.hp=s.version>=6?s.reserved:3;game.won=s.won;
  if(s.version>=7&&s.ending_phase)ending_restore(s.ending_phase,s.ending_tick,s.ending_x,s.ending_y,s.ending_vy,s.ending_cam,s.ending_stride);
  else if(game.won||!game.robots[MOTHER].hp){
   // Old completed/defeated saves settle safely; never replay a past explosion.
   game.robots[MOTHER].hp=0;ending_restore(ENDING_DONE,0,12*Q8,180*Q8,0,0,0);
  }else if(!game.hp){game.dead=1;om_panic();}
  if(s.version==12){
   memset(&game,0,sizeof game);memcpy(&game,s.resume,sizeof s.resume);game.golden=s.golden;
   game.pause=game.menu=0;game.input_lock=1;game.save_pending=game.save_requested=0;
   om_panic();if(game.ending_phase)ending_music();else {game_music_variant(game.bike_phase?0:game.scene==FOREST_SCENE?(game.dam_phase==1?8:7):game.flight_phase?5:game.scene<4?game.scene:game.scene==4?0:1);game_music(game.music);}
  }else {game.run_valid=0;game.arrival=0;game.visited=1023;game.save_pending=1;}

 }else intro_begin();

 old_keys=plat_keys();old_buttons=plat_buttons();edges=bedges=0;menu_turn=0;plat_enc(EN_SELECT);clock_last=plat_ms();accumulator=0;draw_last=0;plat_glow(0);plat_play_red(0);
}
int ui_dirty(void){return 1;}void ui_say(const char *a,const char *b){(void)a;(void)b;game.notice=120;}
static int near_door(void){int px=game.x/Q8+12,feet=game.y/Q8+PLAYER_H;
 if(game.room)return px<62&&iabs(feet-212)<=10;
 if(game.scene==0)return iabs(px-520)<28&&iabs(feet-212)<=10;
 if(game.scene==1)return iabs(px-1580)<28&&iabs(feet-212)<=10;
 return 0;
}
static void enter_exit(void){if(game.crew||game.bomb.stage){game.notice=70;game.save_status=5;return;}if(game.room){if(stage_clean(game.scene))game.flawless_tick=70;set_scene(game.return_scene,game.return_x,game.return_y,1);}else {game.return_scene=game.scene;game.return_x=game.x/Q8;game.return_y=game.y/Q8;set_scene(game.scene==0?4:5,42,180,1);}game_sound(S_SNACK);}
static void robot_explosion(int i){
 robot_t *r=game.robots+i;particle(r->x+12,r->y+14,0,0,5,0,12);
 static const int vx[4]={-410,360,-240,470},vy[4]={-410,-580,-640,-250};
 for(int n=0;n<4;n++)particle(r->x+12,r->y+14,vx[n],vy[n],6,n,18+n*2);
 for(int n=0;n<2;n++)particle(r->x+9+n*7,r->y+12,(n?1:-1)*70,-110,7,n,24+n*4);
 if(!game.pop_clock){game_sound(S_POP);game.pop_clock=5;}
}
static void kill_robot(int i){robot_t *r=game.robots+i;if(!r->hp)return;reward_kill(i);if(game.scene==FOREST_SCENE)forest_on_kill(i);if(game.scene==BIKE_SCENE&&!(r->shots&1)){int id=(r->shots>>1)-1;if(id>=0&&id<10)game.bike_hit_mask|=1<<id;}r->hp=0;r->flash=40;game.shake=i==MOTHER?10:4;if(i==MOTHER){burst(r->x+12,r->y+12,30,2,1);game_sound(S_HIT);ending_begin();}else {robot_explosion(i);game.kill_voice_count=game.kill_voice_count>=1000000?1:game.kill_voice_count+1;if(game.kill_voice_count%6==0)game_voice(6);}}
static int robot_on_screen(int i);
static void hit_robot(int i,int damage){robot_t *r=game.robots+i;if(!r->hp)return;if(((game.scene==FOREST_SCENE&&game.dam_phase==1)||game.scene==BIKE_SCENE)&&!robot_on_screen(i))return;if(i==MOTHER&&r->state==1){burst(r->x+17,153,4,0,2);return;}r->hp-=damage;r->flash=7;burst(r->x+(i==MOTHER?17:12),i==MOTHER?153:r->y+14,5,0,2);if(r->hp<=0){r->hp=1;kill_robot(i);}else game_sound(S_HIT);}
static void damage(void){if(game.invuln||game.arrival||game.dead||game.won||game.ending_phase)return;reward_miss();game.stage_damage|=1u<<game.scene;game.save_pending=1;game.log_target=-1;game.log_chop=0;game.hp--;game.hits++;game.invuln=game.hurt_flash=100;game.vx=-game.dir*350;game.vy=-620;game.shake=7;burst(game.x/Q8+12,game.y/Q8+18,8,0,0);game_sound(S_HIT);if(!game.hp){game.dead=1;game.duck=game.drop_timer=0;game.drop_platform=-1;om_panic();game_sound(S_DEATH);}else game_voice(4);}
static void bullet(int x,int y,int vx,int owner,int type){for(int i=0;i<NBULLET;i++)if(!game.bullets[i].life){game.bullets[i]=(bullet_t){x,y,vx,owner,80,type};return;}}
#include "game_posse.h"
#include "game_factory.h"
static void robot_alert_voice(int i){
 if(i==MOTHER||!robot_on_screen(i))return;uint32_t bit=1u<<(i%32);int word=i/32;
 if(!(game.robot_voice_seen[word]&bit)){game.robot_voice_seen[word]|=bit;game_voice(0);}else game_voice(1);
}
static void toggle_pause(void){game.pause=!game.pause;game.menu=0;menu_turn=0;edges=0;game.buffer=0;game.duck=game.drop_timer=0;game.drop_platform=-1;if(game.pause)om_panic();else if(game.ending_phase)ending_music();else game_music(game.music);}
void ui_input_only(void){int delta=plat_enc(EN_SELECT);if(game.pause)menu_turn=(menu_turn+delta%5)%5;uint32_t k=plat_keys(),b=plat_buttons();edges|=k&~old_keys;bedges|=b&~old_buttons;old_keys=k;old_buttons=b;}
static void fire_damage(int amount){for(int i=0;i<NROB;i++){robot_t *r=game.robots+i;if(robot_here(i)&&r->hp&&iabs(r->x+12-game.bomb.x/Q8)<44&&r->y+28>=game.bomb.y/Q8-75&&r->y<=game.bomb.y/Q8+14)hit_robot(i,amount);}}
static void update_bomb(void){bomb_t *b=&game.bomb;if(b->cooldown)b->cooldown--;if(!b->stage)return;if(b->stage==1){int prev=b->y;b->x+=b->vx;b->y+=b->vy;b->vy+=65;b->timer--;for(int i=0;i<terrain_count[game.scene];i++){const platform_t *p=terrains[game.scene]+i;if(b->vy>=0&&prev<=p->y*Q8&&b->y>=p->y*Q8&&b->x>=p->x*Q8&&b->x<(p->x+p->w)*Q8){b->stage=2;b->timer=150;b->y=p->y*Q8;game.shake=7;burst(b->x/Q8,b->y/Q8-4,18,0,2);game_sound(S_HIT);fire_damage(2);break;}}if(b->stage==1&&(!b->timer||b->x<0||b->x>scene_width[game.scene]*Q8||b->y>265*Q8)){b->stage=0;b->cooldown=20;}}else {if(b->timer%15==0)fire_damage(1);if(--b->timer<=0){b->stage=0;b->cooldown=20;}}}
static void update_robots(void){
 if(game.scene==FOREST_SCENE){forest_update();return;}
 factory_update();
 for(int i=0;i<NROB;i++){robot_t *r=game.robots+i;const robot_def_t *d=robot_defs+i;if(!robot_here(i))continue;if(r->flash)r->flash--;if(!r->hp)continue;if(d->type==3){factory_flyer_step(i);continue;}int dist=iabs(r->x-game.x/Q8);if(dist>450&&i!=MOTHER)continue;
 if(d->type==1){if(r->state==0){r->x=d->x+tri((int)game.steps+i*17,96)/3-8;r->y=d->y+tri((int)game.steps+i*11,64)/4-4;}else if(r->state==1){int dy=r->aim_y-18-r->y;r->y+=dy>0?imax(1,dy/9):dy<0?imin(-1,dy/9):0;}else if(r->state==3){r->y+=(d->y-r->y)/5;if(iabs(d->y-r->y)<5)r->y=d->y;}}
 if(r->state==0){if(r->timer)r->timer--;if(!r->timer&&dist<(i==MOTHER?290:game.scene==0?140:180)){r->state=1;r->timer=i==MOTHER?65:game.scene==0?80:60;r->dir=game.x/Q8<r->x?-1:1;r->aim_y=(d->type==1||i==MOTHER)?game.y/Q8+18:r->y+14;game_sound(S_TELL);robot_alert_voice(i);}}
 else if(--r->timer<=0){if(r->state==1){r->state=2;r->timer=i==MOTHER?18:10;int type=i==MOTHER?(r->shots&1):0;int yy=type?204:r->aim_y;bullet(r->x+12+(d->type==1?2*r->dir:0),d->type==1?r->y+18:yy,r->dir*(i==MOTHER?6:5),1,type);r->shots++;game.attacks++;game_sound(S_LASER);}else if(r->state==2){r->state=3;r->timer=i==MOTHER?95:game.scene==0?100:70;}else {r->state=0;r->timer=25;}}
 if(dist<21&&player_robot_vertical(r->y,robot_kind(i)==1?28:32))damage();}
}
/* The visible inner core is the actual vulnerable bullet rectangle. */
static int mother_core_left(void){return game.robots[MOTHER].x-3;}
static int mother_shot_hits(int before,int after,int y){return imax(before,after)>=mother_core_left()+4&&imin(before,after)<=mother_core_left()+35&&y>=134&&y<=172;}
static void update_bullets(void){for(int i=0;i<NBULLET;i++){bullet_t *b=game.bullets+i;if(!b->life)continue;int before=b->x;b->x+=b->vx;b->life--;if(b->x<0||b->x>scene_width[game.scene]){b->life=0;continue;}if(b->owner){if(b->x>=game.x/Q8-3&&b->x<=game.x/Q8+PLAYER_W+3&&b->y+(b->type?7:3)>player_hit_top()&&b->y<game.y/Q8+PLAYER_H){b->life=0;damage();}}else for(int j=0;j<NROB;j++){robot_t *r=game.robots+j;if(robot_here(j)&&r->hp&&(j==MOTHER?mother_shot_hits(before,b->x,b->y):(imax(before,b->x)>=r->x-2&&imin(before,b->x)<=r->x+28&&b->y>=r->y&&b->y<=r->y+28))){b->life=0;hit_robot(j,1);break;}}}}
#include "game_flight.h"
#include "game_forest.h"
#include "game_bike.h"
#include "game_logs.h"
static void step(void){uint32_t k=old_keys,e=edges,be=bedges;edges=bedges=0;if(be&(1u<<B_PLAY)){if(!game.dead)toggle_pause();be&=~(1u<<B_SEL);}if(be&(1u<<B_SAVE))game.save_pending=game.save_requested=1;
 int engine=0;if(!game.pause&&!game.dead&&!game.won){
  if(game.bike_phase==BIKE_MOUNT)engine=30+game.bike_tick/4;
  else if(game.bike_phase==BIKE_RIDE)engine=game.ground?(game.bike_tick<30?40+game.bike_tick:74):(90+imin(22,imax(-15,-game.vy/120)));
  else if(game.bike_phase==BIKE_EXIT)engine=64;else if(game.bike_phase==BIKE_DISMOUNT)engine=30;
 }game_engine(engine);
 if(game.pause){game.menu=((game.menu+menu_turn)%5+5)%5;menu_turn=0;if(be&(1u<<B_SEL))switch(game.menu){case 0:toggle_pause();break;case 1:game.save_pending=game.save_requested=1;break;case 2:world_reset(0);break;case 3:world_reset(1);intro_begin();game.save_pending=1;break;case 4:game.music=!game.music;break;}if(game.notice)game.notice--;return;}
 if(game.flawless_tick)game.flawless_tick--;
 if(game.intro_phase){intro_step(k,e);return;}
 if(game.ending_phase&&!game.won){ending_step();return;}
 if(game.ending_phase==ENDING_DONE)game.party_tick=(game.party_tick+1)%PARTY_TICKS;
 if(game.won&&game.input_lock){if(!(k&ACTIONS))game.input_lock=0;return;}
 if(game.dead||game.won){if(e&(JUMP|FIRE)){
  if(game.won&&game.gold_reveal==1){game.gold_reveal=2;game.party_tick=0;game.input_lock=1;game.save_pending=1;game.save_requested=0;return;}
  int fresh=game.won;world_reset(fresh);if(fresh){intro_begin();game.save_pending=1;}
 }return;}
 if(game.arrival){arrival_step();return;}
 if(game.input_lock){if(!(k&ACTIONS))game.input_lock=0;return;}if((e&DOOR)&&near_door()){enter_exit();return;}
 if(game.flight_phase){flight_step(k,e);game_jet(!game.dead&&!game.pause&&game.flight_phase&&game.flight_thrust);return;}
 if(game.bike_phase){bike_step(k,e);return;}
 if(game.scene==FOREST_SCENE&&game.dam_entry){forest_entry_step();return;}
 if(game.scene==FOREST_SCENE&&game.dam_phase==2){k=RIGHT;e=0;}
 int anim_was_ground=game.ground,anim_before_x=game.x,log_before_y=game.y;if(game.anim_land)game.anim_land--;
 game.steps++;if(game.pop_clock)game.pop_clock--;if(!game.boss_end&&game.charge<CREW_TICKS)game.charge++;if(game.notice)game.notice--;if(game.transition)game.transition--;if(game.invuln)game.invuln--;if(game.hurt_flash)game.hurt_flash--;if(game.shake)game.shake--;if(game.recoil)game.recoil--;
 for(int i=0;i<NPARTICLE;i++){particle_t *p=game.particles+i;if(p->life){p->x+=p->vx;p->y+=p->vy;if(p->type<5||p->type==6)p->vy+=24;p->life--;}}
 if(game.drop_timer){if(--game.drop_timer==0||game.y/Q8>terrains[game.scene][game.drop_platform].y+2){game.drop_timer=0;game.drop_platform=-1;}}
 game.duck=!!(k&DOWN)&&(game.ground||game.drop_timer);
 if((e&DOWN)&&game.ground&&game.support>=floor_count[game.scene]&&game.support<terrain_count[game.scene]){game.drop_platform=game.support;game.drop_timer=24;game.duck=1;game.ground=game.coyote=game.buffer=0;game.support=-1;game.y+=2*Q8;game.vy=imax(game.vy,160);}
 if(e&JUMP)game.buffer=7;else if(game.buffer)game.buffer--;if(game.ground)game.coyote=6;else if(game.coyote)game.coyote--;
 if(game.buffer&&game.coyote){game.vy=-1600;game.ground=game.coyote=game.buffer=game.duck=0;burst(game.x/Q8+10,game.y/Q8+PLAYER_H,4,3,0);game_sound(S_JUMP);game_voice(3);}
 /* Crouch owns grounded movement; drop-through and jump leave the pose first. */
 if(player_crouched()){game.vx=0;game.gun=game.recoil=0;}
 else {int ax=!!(k&RIGHT)-!!(k&LEFT);if(ax)game.dir=ax;int target=ax*650;game.vx+=(target-game.vx)/3;if(!ax&&iabs(game.vx)<8)game.vx=0;}
 if(!(k&JUMP)&&game.vy<-650)game.vy=-650;game.vy+=72;if(game.vy>1700)game.vy=1700;
 int anim_fall_speed=game.vy;
 int previous=game.y;game.x+=game.vx;game.y+=game.vy;int width=scene_width[game.scene];game.ground=0;game.support=-1;
 for(int i=0;i<terrain_count[game.scene];i++){const platform_t *p=terrains[game.scene]+i;if(game.scene==FOREST_SCENE&&i==terrain_count[FOREST_SCENE]-1&&!game.forest_done)continue;if(game.drop_timer&&i==game.drop_platform)continue;if(game.vy>=0&&previous+PLAYER_H*Q8<=p->y*Q8&&game.y+PLAYER_H*Q8>=p->y*Q8&&game.x+PLAYER_W*Q8>p->x*Q8&&game.x<(p->x+p->w)*Q8){game.y=(p->y-PLAYER_H)*Q8;game.vy=0;game.ground=1;game.support=i;}}
 /* A held Down also takes effect on the landing frame, before the gun gate. */
 game.duck=!!(k&DOWN)&&(game.ground||game.drop_timer);
 if(player_crouched()){game.vx=0;game.gun=game.recoil=0;}
 if(game.scene==FOREST_SCENE){if(game.dam_phase==1)game.x=imin(1940*Q8,imax(1804*Q8,game.x));if(game.dam_phase==2)game.x=imin(2168*Q8,game.x);if(!game.dam_phase&&!game.forest_done)game.x=imin(1934*Q8,game.x);}
 if(game.x>(width-PLAYER_W)*Q8){if(game.scene==FOREST_SCENE&&game.forest_done){forest_to_city();return;}if(game.scene==0&&!game.bike_done){game.x=(width-PLAYER_W)*Q8;if(game.ground&&game.y==(212-PLAYER_H)*Q8)bike_start(1);return;}if(game.scene==0||game.scene==2){stage_clear(game.scene);set_scene(game.scene+1,32,180,0);if(game.scene==1)game.checkpoint=INNER_START_CP;return;}game.x=(width-PLAYER_W)*Q8;}
 if(game.x<0){if(game.scene==1||game.scene==3){int prev=game.scene-1;set_scene(prev,scene_width[prev]-64,floor_at(prev,scene_width[prev]-64)-PLAYER_H,0);return;}game.x=0;}
 if(game.y>270*Q8){damage();if(!game.dead){int scene=game.cp_scene;set_scene(scene,game.cp_x,game.cp_y,0);return;}}
 // Animation follows resolved motion; it never feeds velocity/collision back.
 if(game.ground){if(!anim_was_ground&&anim_fall_speed>220)game.anim_land=8;
  game.anim_stride=(game.anim_stride+iabs(game.x-anim_before_x))%(40*Q8);
 }else game.anim_land=0;
 if(game.scene==0&&!game.bike_done&&game.x/Q8>=3528&&game.ground&&game.y==(212-PLAYER_H)*Q8){bike_start(1);return;}
 if(game.scene==FOREST_SCENE&&!game.dam_phase&&!game.forest_done&&game.x/Q8>=1788&&game.ground){forest_depart();return;}
 if((k&FIRE)&&!player_crouched()){game.gun=6;if(!game.fireclock){bullet(game.x/Q8+12+game.dir*17,player_shot_y(),game.dir*9,0,0);game.fireclock=6;game.recoil=4;game.vx-=game.dir*160;game.shots++;particle(game.x/Q8+14,game.y/Q8+22,-game.dir*220,-280,4,2,24);game_sound(S_GUN);}}else if(game.gun)game.gun--;if(game.fireclock)game.fireclock--;
 if(e&CREW)posse_begin();
 if(e&BOMB){if(game.bottles>0&&!game.bomb.stage&&!game.bomb.cooldown){game.bottles--;game.throws++;game.bomb=(bomb_t){game.x+12*Q8,game.y+10*Q8,game.dir*680,-980,1,80,0};game_sound(S_SWARM);game_voice(2);}else {game.save_status=game.bottles?5:6;game.notice=70;}}
 posse_step();
 update_robots();update_bullets();update_bomb();if(game.ending_phase)return;update_logs(anim_before_x,log_before_y);
 for(int i=0;i<NSNACK;i++)if(game.bottles<BOTTLE_CAP&&snack_visible(i)&&iabs(snacks[i].x-(game.x/Q8+12))<18&&iabs(snacks[i].y-(game.y/Q8+16))<24){collect_bit(i);game.bottles++;burst(snacks[i].x,snacks[i].y,4,0,2);game_sound(S_SNACK);}
 for(int i=game.checkpoint+1;i<NCP;i++){const checkpoint_t *c=checkpoints+i;if(c->scene==game.scene&&iabs(game.x/Q8+12-c->x)<24&&game.ground){game.checkpoint=i;game.cp_scene=c->scene;game.cp_x=c->x;game.cp_y=c->y-PLAYER_H;game.save_pending=1;game.save_requested=0;game.notice=150;game_sound(S_SNACK);}}
 if(game.ending_phase)return;

 if(game.scene==1&&!game.flight_done&&iabs(game.x/Q8+12-JETPACK_X)<22&&game.ground&&iabs(game.y/Q8+PLAYER_H-JETPACK_Y)<3){flight_start(1);return;}

 int desired=camera_target(game.scene,game.x/Q8);game.cam_q8+=(desired*Q8-game.cam_q8)/4;game.cam=game.cam_q8/Q8;
}
#ifndef PLAT_STORE_PAUSED_ONLY
#ifdef OM_HOST
#define PLAT_STORE_PAUSED_ONLY 0
#else
#define PLAT_STORE_PAUSED_ONLY 1
#endif
#endif
void ui_frame(void){ui_input_only();uint32_t now=plat_ms(),elapsed=now-clock_last;clock_last=now;accumulator+=elapsed>100?100:elapsed;while(accumulator>=16){accumulator-=16;step();}if(game.save_pending){
#if PLAT_STORE_PAUSED_ONLY
 if(!game.pause&&!game.dead){if(!game.notice||(game.save_status>=0&&game.save_status<=2)){game.save_status=2;game.notice=150;}}else
#endif
 save_game();}
 plat_leds((1u<<B_PLAY)|(1u<<B_SAVE)|(game.pause?1u<<B_SEL:0),game.pause?0:LEFT|RIGHT|DOWN|JUMP|FIRE|(near_door()?DOOR:0)|(game.bottles>0?BOMB:0)|(game.charge==CREW_TICKS?CREW:0));if(now-draw_last>=32){draw_last=now;render();}}
