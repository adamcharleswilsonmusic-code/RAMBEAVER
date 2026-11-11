/* Modified for the local Fart Machine One / Beaver game derivative, 2026-10-13. Upstream origin and notices: LICENSING.md and provenance/source-inventory.json. */
/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#define FMO_KEYS 27
#define FMO_VOICES 8
#define FMO_STEPS 16
enum { P_REVERB,P_DELAY,P_TIME,P_FEEDBACK,P_NPARAMS };
enum { S_GUN,S_JUMP,S_HIT,S_TELL,S_LASER,S_SNACK,S_WIN,S_SWARM,S_MUSIC,S_ALARM,S_POP,S_ENGINE,S_SPEECH,S_DEATH };
typedef struct {const char *name;int16_t lo,hi,def;} om_param_t;
const om_param_t *om_param_info(int p);
void om_init(void);void om_set(int p,int v);void om_trigger(int key,int velocity);void om_panic(void);
void om_render(int32_t *out,uint32_t n,uint32_t gain);
void game_sound(int effect);void game_voice(int clip);void game_music(int enabled);
extern volatile uint32_t om_active_keys,om_triggers,om_dropped;
extern volatile uint8_t om_seq_playing,om_seq_recording,om_seq_step;
extern volatile uint16_t om_seq_bpm;
extern volatile uint32_t om_seq_ticks,om_seq_mask[FMO_STEPS],om_seq_hit_dropped;
void om_seq_toggle_play(void);void om_seq_toggle_record(void);void om_seq_clear(void);void om_seq_set_tempo(int bpm);int om_take_seq_hit(int *key);
const char *om_sample_name(int key);int om_placeholder(void);

/* Read-only diagnostics: frames, beat phase, tune position, notes, resets, last note frame. */
uint32_t om_audio_stat(int index);

uint32_t om_music_beat(void);

void game_music_restart(void);
void game_music_variant(int zone);
void game_music_victory(void);
void game_music_silence(void);
void game_alarm_stop(void);

/* Low-priority engine control; rev0 stops,1..127 revs. Never queues a voice. */
void game_engine(int rev);
/* Mutually exclusive jet timbre in the same preemptible engine slot. */
void game_jet(int thrust);

int game_voice_available(int clip);
void game_voice_end_combat(void);
