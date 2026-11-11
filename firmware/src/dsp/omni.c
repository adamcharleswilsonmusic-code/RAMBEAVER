/* SPDX-License-Identifier: GPL-3.0-only
 * Felucca ANALOG + NOISE engines, Copyright (C) 2026 Leo Kuroshita,
 * Hügelton Instruments. Game adapter: eight bounded one-shot voices.
 * Render owns all engine state; UI communicates through an SPSC ring.
 */
#include "omni.h"
#include "felucca_tables.h"
#include "voice_bank.h"
#include "music_score.h"
#include "anthem_score.h"
enum {P_E0,P_E1,P_E2,P_E3,P_E4,P_E5,P_E6,P_E7,P_FTYPE};
typedef struct {int16_t p[9];} track_t;
typedef struct {int32_t s[8];uint32_t ph[3],note,age;int32_t pitch16,fine;} voice_t;
typedef struct {uint32_t inc;int32_t pitch16,fine,shape,cutoff,amp0,amp1;} vmod_t;
#include "felucca_dsp.h"
#include "felucca_analog.h"
#include "felucca_noise.h"
volatile uint32_t om_active_keys,om_triggers,om_dropped;
volatile uint8_t om_seq_playing,om_seq_recording,om_seq_step;
volatile uint16_t om_seq_bpm;
volatile uint32_t om_seq_ticks,om_seq_mask[FMO_STEPS],om_seq_hit_dropped;
static const om_param_t params[4]={{"",0,100,0},{"",0,100,0},{"",0,100,0},{"",0,100,0}};
const om_param_t *om_param_info(int p){return p>=0&&p<4?params+p:0;}
void om_set(int p,int v){(void)p;(void)v;}
void om_seq_toggle_play(void){} void om_seq_toggle_record(void){} void om_seq_clear(void){} void om_seq_set_tempo(int b){(void)b;} int om_take_seq_hit(int *k){(void)k;return 0;}
const char *om_sample_name(int k){(void)k;return "SYNTH";} int om_placeholder(void){return 0;}
#define Q 64u
#define BARRIER() __asm__ volatile("" ::: "memory")
typedef struct {int effect;uint32_t epoch;} cmd_t;
static cmd_t commands[Q];static volatile uint32_t wr,rd,epoch,music_enabled;
static uint32_t seen_epoch,music_clock,music_step,clip_pos,clip_frac;
static volatile uint32_t audio_frames,music_notes,audio_resets,last_music_frame;
static uint32_t music_accum,music_cursor,music_frames,music_started,anthem_cursor;
static volatile uint32_t music_restart_requested,music_mode_requested;
static uint32_t music_mode,music_sfx_gain,victory_fade;
static volatile uint32_t music_victory_requested,music_silence_requested,music_cancel_handover,alarm_stop_requested;
static uint32_t handover_silence;
static volatile uint32_t engine_requested;
uint32_t om_audio_stat(int i){switch(i){case 0:return audio_frames;case 1:return music_clock;case 2:return music_step;case 3:return music_notes;case 4:return audio_resets;case 5:return last_music_frame;case 6:return music_mode;default:return 0;}}
static int playing_clip=-1;
static uint32_t voice_pending_mask,voice_last[8],robot_voice_last;
static uint8_t voice_pending_priority[8];
static int speech_priority;
static uint32_t speech_gain;
#define VOICE_CMD_BASE 100
#define VOICE_CANCEL 99
typedef struct {track_t t;voice_t v;uint32_t remaining,total;int noise,pitch,sweep,amp,effect,instrument;} slot_t;
static slot_t slots[8];
static void clear_slot(slot_t *s){uint8_t *p=(uint8_t *)s;for(uint32_t i=0;i<sizeof *s;i++)p[i]=0;}
void game_sound(int e){uint32_t w=wr;if(w-rd>=Q){om_dropped++;return;}commands[w%Q]=(cmd_t){e,epoch};BARRIER();wr=w+1;}
void om_trigger(int k,int v){(void)v;game_sound(k);}
int game_voice_available(int c){return c>=0&&c<8&&clips[c].length;}
static void voice_request(int c,int priority,int forced){
 if(!game_voice_available(c))return;uint32_t now=audio_frames+1u;
 static const uint32_t cooldown[8]={132300,132300,0,15435,26460,35280,0,66150};
 if(!forced){if(voice_last[c]&&now-voice_last[c]<cooldown[c])return;if(c<2&&robot_voice_last&&now-robot_voice_last<132300u)return;}
 voice_last[c]=now;if(c<2)robot_voice_last=now;game_sound(VOICE_CMD_BASE+priority*8+c);
}
void game_voice(int c){static const uint8_t priority[8]={1,1,4,2,6,5,3,6};if(c>=0&&c<8)voice_request(c,priority[c],0);}
void game_voice_end_combat(void){game_sound(VOICE_CANCEL);}
void game_music(int on){music_enabled=on!=0;}
void game_engine(int rev){engine_requested=(uint32_t)clamp(rev,0,127);BARRIER();}
void game_jet(int thrust){engine_requested=thrust?0x100u:0;BARRIER();}
void om_panic(void){engine_requested=0;music_enabled=0;BARRIER();epoch++;}
void om_init(void){int i;speech_gain=256;engine_requested=0;music_sfx_gain=256;victory_fade=music_victory_requested=music_silence_requested=music_cancel_handover=alarm_stop_requested=handover_silence=0;wr=rd=epoch=seen_epoch=0;music_enabled=0;music_clock=music_step=0;music_accum=music_cursor=anthem_cursor=music_frames=music_started=music_restart_requested=music_mode_requested=music_mode=0;audio_frames=music_notes=audio_resets=last_music_frame=0;playing_clip=-1;speech_priority=0;voice_pending_mask=robot_voice_last=0;for(i=0;i<8;i++){voice_last[i]=0;voice_pending_priority[i]=0;}om_triggers=om_dropped=om_active_keys=0;for(i=0;i<8;i++)clear_slot(slots+i);}
static void music_note(const music_event_t *e){
 static const int lead_wave[9]={0,4,AW_SYNC,4,2,AW_SYNC,2,2,2},lead_cut[9]={94,91,88,98,80,100,102,84,91},lead_mix[9]={32,48,46,56,22,58,26,24,30};
 static const int bass_cut[9]={83,88,79,94,70,92,80,72,86},bass_sub[9]={62,54,68,48,70,56,66,68,65};
 slot_t *s=slots+e->channel;clear_slot(s);s->effect=S_MUSIC;s->instrument=e->instrument;s->pitch=e->note;s->amp=e->level*74;s->total=(e->length*5512u*9u)/10u;
 s->t=(track_t){{lead_wave[music_mode],music_mode==2?32:music_mode==5?17:11,lead_mix[music_mode],0,lead_cut[music_mode],music_mode==2?28:music_mode==5?22:18,music_mode==3?14:music_mode==5?8:6,0,0}};
 if(e->instrument==1){s->pitch=e->note+12;s->t=(track_t){{AW_SUB,0,bass_sub[music_mode],0,bass_cut[music_mode],music_mode==3?28:12,music_mode==3?17:5,0,0}};s->amp=e->level*94;}
 else if(e->instrument==3){s->t=(track_t){{3,0,0,0,112,0,0,0,0}};s->pitch=32;s->total=9000;s->amp=e->level*118;}
 else if((e->instrument>=4&&e->instrument<=6)||e->instrument==8){int clap=e->instrument==4||e->instrument==8;s->noise=1;s->total=e->instrument==8?2400:clap?5000:e->instrument==6?5500:1400;s->t=(track_t){{0,clap?8:0,clap?103:115,clap?55:72,0,0,0,0,0}};s->amp=e->level*(clap?94:67);}
 else if(e->instrument==7){s->total=2300;s->t.p[4]=104;s->amp=e->level*64;}
 else if(e->instrument==9){s->t=music_mode==7?(track_t){{3,2,18,0,78,6,0,0,0}}:(track_t){{2,3,22,0,86,8,0,0,0}};s->amp=e->level*(music_mode==7?76:80);}
 else if(e->instrument==10){s->t=(track_t){{3,0,0,0,70,4,0,0,0}};s->amp=e->level*105;}
 else if(e->instrument==11){s->t=(track_t){{2,7,36,0,76,6,0,0,0}};s->amp=e->level*64;}
 else if(e->instrument==12){s->t=(track_t){{2,2,18,0,84,6,1,0,0}};s->amp=e->level*71;s->total=e->length*5512u*98u/100u;}
 else if(e->instrument==13){s->t=(track_t){{2,4,26,0,96,6,0,0,0}};s->amp=e->level*70;}
 else if(e->instrument==14){s->t=(track_t){{AW_SUB,0,54,0,89,8,2,0,0}};s->amp=e->level*94;}
 s->remaining=s->total;s->v.note=s->pitch;if(s->noise)noise_note_on(&s->t,&s->v);else analog_note_on(&s->t,&s->v);
}
static void music_tick(void){
 // Ordinary zone changes retain phrase/beat; the anthem has its own full
 //28-bar phrase. Victory handoff starts it at0 after the exit as before.
 if((music_step&15u)==0&&music_mode!=music_mode_requested){
  if(music_mode==6||music_mode_requested==6){music_step=0;music_cursor=anthem_cursor=0;}
  music_mode=music_mode_requested;
 }
 if(music_mode==6){
  while(anthem_cursor<ANTHEM_EVENTS&&anthem_events[anthem_cursor].tick==music_step){const anthem_event_t *a=anthem_events+anthem_cursor++;music_event_t e={0,a->channel,a->note,a->instrument,a->length,a->level,64};music_note(&e);}
 }else while(music_cursor<MUSIC_EVENTS&&music_events[music_cursor].tick==music_step){const music_event_t *e=music_events+music_cursor++;if(e->mask&(1u<<music_mode))music_note(e);}
 music_notes++;last_music_frame=audio_frames;
}
void game_music_variant(int zone){music_mode_requested=(uint32_t)clamp(zone,0,8);music_cancel_handover=1;music_silence_requested=0;music_victory_requested=0;}
void game_music_victory(void){
 music_silence_requested=0;
 if(music_mode_requested!=6||handover_silence){music_cancel_handover=1;music_mode_requested=6;music_victory_requested=1;}
}
void game_music_silence(void){music_victory_requested=0;music_cancel_handover=1;music_silence_requested=1;}
void game_alarm_stop(void){alarm_stop_requested=1;}
static void silence_commit(void){for(int k=0;k<4;k++)clear_slot(slots+k);music_enabled=0;victory_fade=0;}
static void victory_commit(void){
 for(int k=0;k<4;k++)clear_slot(slots+k);
 music_mode=6;music_accum=music_cursor=anthem_cursor=music_frames=music_started=music_clock=music_step=0;
}
uint32_t om_music_beat(void){return music_enabled?((music_frames%22050u)*65536u)/22050u:0;}
void game_music_restart(void){music_restart_requested=1;}
static void start(int e){
 int i,b=4;uint32_t least=0xffffffffu;slot_t *s;
 for(i=4;i<8;i++){if(!slots[i].remaining||slots[i].effect==S_ENGINE){b=i;break;}if(slots[i].effect!=S_SPEECH&&slots[i].remaining<least){least=slots[i].remaining;b=i;}}
 s=slots+b;clear_slot(s);s->effect=e;s->amp=6200;s->pitch=60;s->total=6615;
 s->t=(track_t){{3,0,0,0,110,0,0,0,0}};
 switch(e){
 case S_GUN:s->noise=1;s->total=2646;s->t=(track_t){{0,28,97,22,0,0,0,20,0}};s->amp=9800;break;
 case S_JUMP:s->pitch=57;s->sweep=22;s->total=8820;break;
 case S_HIT:s->noise=1;s->total=7056;s->t=(track_t){{3,0,90,35,0,32,0,0,0}};s->pitch=42;break;
 case S_DEATH:s->pitch=55;s->sweep=-31;s->total=26460;s->amp=8500;s->t.p[0]=1;break;
 case S_TELL:s->pitch=77;s->sweep=-5;s->total=11025;s->t.p[0]=2;break;
 case S_LASER:s->pitch=84;s->sweep=-30;s->total=11025;s->t.p[0]=1;break;
 case S_SNACK:s->pitch=84;s->sweep=7;s->total=6615;break;
 case S_WIN:s->pitch=72;s->sweep=12;s->total=22050;break;
 case S_SWARM:s->pitch=88;s->sweep=9;s->total=13230;break;
 case S_POP:s->noise=1;s->pitch=40;s->total=17640;s->amp=8000;break;
 case S_ALARM:s->pitch=83;s->sweep=-28;s->total=11025;s->amp=6800;s->t.p[0]=2;break;
 default:{static const int notes[16]={48,60,63,67,48,60,70,67,46,58,62,65,46,58,69,65};s->pitch=notes[music_step++&15];music_notes++;last_music_frame=audio_frames;s->total=6615;s->amp=2300;s->t.p[0]=2;break;}}
 s->remaining=s->total;s->v.note=(uint32_t)s->pitch;s->v.age=om_triggers;
 if(s->noise)noise_note_on(&s->t,&s->v);else analog_note_on(&s->t,&s->v);
 if(e==S_POP){s->v.ph[0]=s->v.ph[1]=0;s->v.s[5]=(int32_t)(0x13579bdu+om_triggers*7919u);s->v.s[6]=s->v.s[3]=s->v.s[4]=0;}
 om_triggers++;
}
/* Both sounds are composite timbres INSIDE one existing SFX slot each. */
static void engine_prepare(void){
 int found=-1,free=-1;for(int k=4;k<8;k++){if(slots[k].remaining&&slots[k].effect==S_ENGINE)found=k;if(!slots[k].remaining&&free<0)free=k;}
 int jet=!!(engine_requested&0x100u);
 if(found>=0&&engine_requested&&slots[found].noise!=jet){clear_slot(slots+found);free=found;found=-1;}
 if(found<0&&engine_requested&&free>=0){slot_t *s=slots+free;clear_slot(s);s->effect=S_ENGINE;s->noise=jet;s->remaining=s->total=0x7fffffffu;s->v.pitch16=jet?37*16:24*16+(int)engine_requested*3;s->v.s[5]=0x2468ace;}
}
static void jet_render(slot_t *s,int32_t *out,uint32_t n){
 int on=!!(engine_requested&0x100u),combat=0;for(int k=4;k<8;k++)combat+=slots[k].remaining&&slots[k].effect!=S_ENGINE;
 int swell=(int)(s->v.age<18000u?s->v.age:18000u),target=on?2200+swell/18:0;
 if(combat)target=target*180/256;if(playing_clip>=0)target=target*128/256;
 s->v.pitch16=37*16+swell/1600;uint32_t inc=pitch_inc((uint32_t)s->v.pitch16);
 for(uint32_t j=0;j<n;j++){
  int d=target-s->amp;s->amp+=d/(on?256:128)+(d>0?1:d<0?-1:0);if((d>0&&s->amp>target)||(d<0&&s->amp<target))s->amp=target;
  int noise=(int32_t)(noise32(&s->v.s[5])>>16)-32768;
  s->v.s[6]+=(noise-s->v.s[6])/5;s->v.s[7]+=(noise-s->v.s[7])/40;s->v.ph[0]+=inc;
  int air=(noise-s->v.s[6])/3+s->v.s[6]*2+s->v.s[7];
  int wave=air/3+sine_i(s->v.ph[0])/4;out[j]+=wave*s->amp/32768;
 }
 if(on)s->v.age+=n;else if(!s->amp)s->remaining=0;
}
static void engine_render(slot_t *s,int32_t *out,uint32_t n){
 if(s->noise){jet_render(s,out,n);return;}
 int target=engine_requested?1700+(int)engine_requested*12:0;if(playing_clip>=0)target=target*144/256;s->v.pitch16+=clamp(24*16+(int)engine_requested*3-s->v.pitch16,-3,3);uint32_t inc=pitch_inc((uint32_t)s->v.pitch16);
 for(uint32_t j=0;j<n;j++){
  int d=target-s->amp;s->amp+=d/64+(d>0?1:d<0?-1:0);if((d>0&&s->amp>target)||(d<0&&s->amp<target))s->amp=target;
  s->v.ph[0]+=inc;int wave=(sine_i(s->v.ph[0])*4+sine_i(s->v.ph[0]*2u)*2+sine_i(s->v.ph[0]*4u)+osc_saw(s->v.ph[0],inc))/8;
  int noise=(int32_t)(noise32(&s->v.s[5])>>16)-32768;s->v.s[6]+=(noise-s->v.s[6])/12;
  wave=wave*(24576+(sine_i(s->v.ph[0]/2u)>>3))/32768+s->v.s[6]/12;out[j]+=wave*s->amp/32768;
 }
 if(!engine_requested&&!s->amp)s->remaining=0;
}
static void explosion_render(slot_t *s,int32_t *out,uint32_t n){
 uint32_t start=s->total-s->remaining;int p=46*16-(int)(start*22u*16u/s->total);uint32_t inc=pitch_inc((uint32_t)p);
 for(uint32_t j=0;j<n;j++){
  uint32_t age=start+j,remain=age<s->total?s->total-age:0;int noise=(int32_t)(noise32(&s->v.s[5])>>16)-32768;s->v.s[6]+=(noise-s->v.s[6])/9;s->v.ph[0]+=inc;s->v.ph[1]+=inc*7u;
  int boom=(int)(age<9000?9000-age:0),crack=(int)(age<1600?1600-age:0),tail=(int)remain;
  int v=sine_i(s->v.ph[0])*boom/9000*3600/32768+(noise-s->v.s[6])*crack/1600*3900/32768+s->v.s[6]*tail/(int)s->total*2200/32768;
  if(age>700&&age<6000)v+=sine_i(s->v.ph[1])*(6000-(int)age)/5300*450/32768;
  int filtered=v-s->v.s[3]+s->v.s[4]*32700/32768;s->v.s[3]=v;s->v.s[4]=filtered;v=filtered;
  int fade=(int)(age<24?age:24);v=v*fade/24;if(remain<128)v=v*(int)remain/128;out[j]+=v;
 }
 s->remaining=s->remaining>n?s->remaining-n:0;
}
static void speech_clear(void){
 playing_clip=-1;speech_priority=0;voice_pending_mask=0;for(int c=0;c<8;c++)voice_pending_priority[c]=0;for(int k=4;k<8;k++)if(slots[k].effect==S_SPEECH)clear_slot(slots+k);
}
static void speech_schedule(void){
 int chosen=-1,priority=0;for(int c=0;c<8;c++)if((voice_pending_mask&(1u<<c))&&voice_pending_priority[c]>priority){chosen=c;priority=voice_pending_priority[c];}
 if(chosen<0||(playing_clip>=0&&speech_priority>=priority))return;
 int slot=-1;uint32_t shortest=0xffffffffu;
 for(int k=4;k<8;k++)if(slots[k].remaining&&slots[k].effect==S_SPEECH){slot=k;break;}
 if(slot<0)for(int k=4;k<8;k++){if(!slots[k].remaining||slots[k].effect==S_ENGINE){slot=k;break;}if(slots[k].remaining<shortest){slot=k;shortest=slots[k].remaining;}}
 slot_t *s=slots+slot;clear_slot(s);s->effect=S_SPEECH;s->remaining=s->total=(clips[chosen].length*441u+79u)/80u;
 playing_clip=chosen;speech_priority=priority;clip_pos=clip_frac=0;voice_pending_mask&=~(1u<<chosen);voice_pending_priority[chosen]=0;
}
void om_render(int32_t *out,uint32_t n,uint32_t gain){
 uint32_t off,i,k,suppress_alarm;int32_t mono[32];
 if(music_restart_requested){music_restart_requested=0;victory_fade=0;music_accum=music_cursor=anthem_cursor=music_frames=music_started=music_clock=music_step=0;}
 if(music_cancel_handover){music_cancel_handover=0;victory_fade=handover_silence=0;}
 if(music_silence_requested){
  music_silence_requested=0;handover_silence=1;
  if(music_enabled&&music_started)victory_fade=256;else silence_commit();
 }
 if(music_victory_requested&&music_enabled){
  music_victory_requested=0;handover_silence=0;
  if(music_started)victory_fade=256;else victory_commit();
 }
 if(seen_epoch!=epoch){seen_epoch=epoch;for(k=0;k<8;k++)slots[k].remaining=0;speech_clear();speech_gain=256;audio_resets++;}
 suppress_alarm=alarm_stop_requested;
 if(suppress_alarm){alarm_stop_requested=0;for(k=4;k<8;k++)if(slots[k].effect==S_ALARM)clear_slot(slots+k);}
 while(rd!=wr){cmd_t c=commands[rd%Q];rd++;if(c.epoch!=seen_epoch)continue;
  if(c.effect==VOICE_CANCEL){speech_clear();continue;}
  if(c.effect>=VOICE_CMD_BASE){int id=(c.effect-VOICE_CMD_BASE)&7,priority=(c.effect-VOICE_CMD_BASE)/8;
   if(id==6&&playing_clip==6)continue; /* One reaction, never a queued echo. */
   if(priority>=6){voice_pending_mask&=~((1u<<0)|(1u<<1)|(1u<<3));if(voice_pending_priority[6]<7)voice_pending_mask&=~(1u<<6);}
   voice_pending_mask|=1u<<id;if(priority>voice_pending_priority[id])voice_pending_priority[id]=(uint8_t)priority;continue;}
  if(!(suppress_alarm&&c.effect==S_ALARM))start(c.effect);
 }
 engine_prepare();
 speech_schedule();
 for(off=0;off<n;off+=32){uint32_t len=n-off<32?n-off:32,fx_count=0;int handover=victory_fade>0;om_active_keys=0;speech_schedule();
  for(k=4;k<8;k++)fx_count+=slots[k].remaining&&slots[k].effect!=S_ENGINE;
  // Fast, interpolated music-only headroom during stacked combat effects.
  // Release recovers over a few dozen milliseconds; effect levels stay intact.
  uint32_t duck_target=fx_count>=4?96u:fx_count==3?128u:fx_count==2?224u:256u;
  uint32_t next_duck=duck_target<music_sfx_gain?duck_target:music_sfx_gain+4u;
  if(next_duck>duck_target)next_duck=duck_target;
  for(i=0;i<len;i++)mono[i]=0;
  if(music_enabled&&!music_started){music_started=1;music_tick();}
  for(k=0;k<8;k++){slot_t *s=slots+k;vmod_t m;if(!s->remaining)continue;om_active_keys|=1u<<k;
   if(s->effect==S_SPEECH)continue;
   if(s->effect==S_ENGINE){engine_render(s,mono,len);continue;}if(s->effect==S_POP){explosion_render(s,mono,len);continue;}
   m=(vmod_t){0};m.pitch16=(s->pitch*16)+s->sweep*16*(int32_t)(s->total-s->remaining)/(int32_t)s->total;if(s->effect==S_ALARM){int wobble=(int)((s->total-s->remaining)/220u)%16;m.pitch16+=((wobble<8?wobble:16-wobble)-4)*8;}
   m.inc=pitch_inc((uint32_t)m.pitch16);m.shape=64<<8;
   m.amp0=s->amp*(int32_t)s->remaining/(int32_t)s->total;m.amp1=s->amp*(int32_t)(s->remaining>len?s->remaining-len:0)/(int32_t)s->total;s->v.pitch16=m.pitch16;
   if(s->effect==S_MUSIC){
    uint32_t age=s->total-s->remaining,attack=s->instrument==12?882u:s->instrument==13?330u:s->instrument==14?160u:s->instrument==9?1102u:s->instrument==10?3528u:s->instrument==11?9702u:s->instrument==3?24u:s->noise?16u:128u;
    /* Anthem brass holds the printed long notes, with a short natural release. */
    if(s->instrument==12){int32_t env=age<2205u?256-(int32_t)(age*48u/2205u):208,release=2205;
     m.amp0=s->amp*env/256*(int32_t)(s->remaining<(uint32_t)release?s->remaining:(uint32_t)release)/release;
     m.amp1=s->amp*env/256*(int32_t)(s->remaining>len?(s->remaining-len<(uint32_t)release?s->remaining-len:(uint32_t)release):0)/release;}
    m.amp0=m.amp0*(int32_t)music_sfx_gain/256;m.amp1=m.amp1*(int32_t)next_duck/256;
    int32_t a0=(int32_t)(age<attack?age:attack),a1=(int32_t)(age+len<attack?age+len:attack);m.amp0=m.amp0*a0/(int32_t)attack;m.amp1=m.amp1*a1/(int32_t)attack;
    if(handover){
     m.amp0=m.amp0*(int32_t)victory_fade/256;
     m.amp1=m.amp1*(int32_t)(victory_fade>len?victory_fade-len:0)/256;
    }
    if(s->instrument==3){m.pitch16=32*16+(age<1323?(int32_t)((1323-age)*21*16/1323):0);m.inc=pitch_inc((uint32_t)m.pitch16);s->v.pitch16=m.pitch16;s->t.p[3]=age<384?5:0;}
    if(s->instrument>=9){m.cutoff=(int32_t)((age<9702u?age:9702u)*3u*256u/9702u);}
    else if(k<2){uint32_t beat=music_frames%22050u;int32_t pump=beat<4096u?96+(int32_t)(beat*160u/4096u):256;m.amp0=m.amp0*pump/256;m.amp1=m.amp1*pump/256;m.cutoff=(int32_t)((s->instrument==1?23u:17u)*256u*s->remaining/s->total);if(s->t.p[0]==4)m.shape=(64<<8)-(int32_t)(12u*256u*s->remaining/s->total);}
    if((s->instrument==4||s->instrument==8)&&age<1320u&&((age/220u)&1u)){m.amp0=m.amp0/4;m.amp1=m.amp1/4;}
   }
   if(s->noise)noise_render(&s->t,&s->v,mono,len,&m);else analog_render(&s->t,&s->v,mono,len,&m);
   s->remaining=s->remaining>len?s->remaining-len:0;
  }
  music_sfx_gain=next_duck;
  uint32_t speech_target=playing_clip>=0?112u:256u,speech_next=speech_gain;
  if(speech_next>speech_target)speech_next-=(speech_next-speech_target)/4u+1u;else if(speech_next<speech_target)speech_next+=2u;
  if(speech_next>256u)speech_next=256u;
  for(i=0;i<len;i++){int32_t v=mono[i]*(int32_t)(speech_gain+((int32_t)speech_next-(int32_t)speech_gain)*(int32_t)i/(int32_t)len)/256;if(playing_clip>=0){const clip_t *c=clips+playing_clip;v+=(int32_t)c->data[clip_pos]*90;clip_frac+=8000;if(clip_frac>=44100){clip_frac-=44100;if(++clip_pos>=c->length){playing_clip=-1;speech_priority=0;for(int j=4;j<8;j++)if(slots[j].effect==S_SPEECH)slots[j].remaining=0;}}}
   v=clamp(v,-30000,30000);v=(v*(int32_t)gain)>>12;out[2*(off+i)]=out[2*(off+i)+1]=v*256;}
  speech_gain=speech_next;audio_frames+=len;
  if(music_enabled&&handover){victory_fade= victory_fade>len?victory_fade-len:0;if(!victory_fade){if(handover_silence)silence_commit();else victory_commit();}}
  else if(music_enabled){music_frames+=len;music_clock=music_frames%22050u;music_accum+=len*MUSIC_BPM*4u;while(music_accum>=2646000u){music_accum-=2646000u;music_step=(music_step+1)%(music_mode==6?ANTHEM_TICKS:MUSIC_TICKS);if(!music_step)music_cursor=anthem_cursor=0;music_tick();}}
 }
}

