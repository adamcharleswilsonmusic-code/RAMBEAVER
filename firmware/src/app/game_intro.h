/* SPDX-License-Identifier: GPL-3.0-only
 * Original bounded opening: saved games and checkpoint retries skip it. */
#pragma once
#define INTRO_LINE_TICKS 144
#define INTRO_STORY_TICKS (4*INTRO_LINE_TICKS)
#define INTRO_DEPART_TICKS 120
#define INTRO_REVEAL_TICKS 192
static void intro_begin(void){
 game.arrival=0;game.intro_phase=1;game.intro_tick=0;game.input_lock=1;game.transition=0;
 clear_effects();game_music_variant(4);game_music(game.music);
}
static void intro_finish(void){
 game.intro_phase=game.intro_tick=0;game.input_lock=1;game.transition=0;
 game.vx=game.vy=game.buffer=game.gun=game.recoil=game.fireclock=0;
 arrival_start(1,-52,32,180);game_music_variant(7);game_music(game.music);
}
static void intro_step(uint32_t keys,uint32_t pressed){
 if(game.input_lock){if(!(keys&ACTIONS))game.input_lock=0;pressed=0;}
 if(game.intro_phase==1){
  if(++game.intro_tick>=INTRO_STORY_TICKS||(pressed&(JUMP|FIRE))){
   game.intro_phase=3;game.intro_tick=0;game.input_lock=1;
  }
 }else if(game.intro_phase==3){
  if(++game.intro_tick>=INTRO_REVEAL_TICKS||(pressed&(JUMP|FIRE))){game.intro_phase=2;game.intro_tick=0;game.input_lock=1;}
 }else{
  if(game.intro_tick<INTRO_DEPART_TICKS)game.intro_tick++;
  if(pressed&(JUMP|FIRE)){game.intro_tick=INTRO_DEPART_TICKS;game.input_lock=1;}
  if(game.intro_tick>=INTRO_DEPART_TICKS){
   /* Arrive on the first main-song bar, with controls held until release. */
   game_music_variant(7);
   if(!game.music||om_audio_stat(6)==7)intro_finish();
  }
 }
}
