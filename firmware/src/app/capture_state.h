/* SPDX-License-Identifier: GPL-3.0-only
 * Versioned, read-only render snapshot. No save data is read or written. */
#pragma once
#define CAPTURE_VERSION 745
#define CAPTURE_CHUNK 84u
#define CAPTURE_HEADER 17u
static uint32_t capture_visual_override,capture_visual_beat,capture_visual_step;
static uint32_t art_beat(void){return capture_visual_override?capture_visual_beat:om_music_beat();}
static uint32_t art_music_step(void){return capture_visual_override?capture_visual_step:om_audio_stat(2);}
typedef struct {game_t world;uint32_t beat,music_step,xruns,cpu;} capture_state_t;
typedef char capture_i32_required[(sizeof(int)==4)?1:-1];
typedef char capture_size_bounded[(sizeof(capture_state_t)<16384)?1:-1];
static uint32_t capture_hash(const uint8_t *p,uint32_t n){uint32_t h=2166136261u;while(n--)h=(h^*p++)*16777619u;return h&0x0fffffffu;}
#define CAPTURE_PACKED_MAX (sizeof(capture_state_t)+(sizeof(capture_state_t)+127)/128+4)
/* PackBits-style bounded byte runs. Literal blocks cost one byte per128;
 * runs of3+ bytes save transfer bandwidth without changing render state. */
static uint32_t capture_pack(uint8_t *out,const uint8_t *in,uint32_t n){
 uint32_t i=0,w=0;
 while(i<n){
  uint32_t run=1;while(i+run<n&&run<128&&in[i+run]==in[i])run++;
  if(run>=3){out[w++]=128u|(run-1);out[w++]=in[i];i+=run;}
  else {uint32_t start=i;i+=run;
   while(i<n&&i-start<128){run=1;while(i+run<n&&run<3&&in[i+run]==in[i])run++;if(run>=3)break;i++;}
   out[w++]=i-start-1;for(uint32_t j=start;j<i;j++)out[w++]=in[j];
  }
 }
 return w;
}
static uint32_t capture_packet(uint8_t *p,const uint8_t *state,uint32_t size,uint32_t frame,uint32_t offset,uint32_t hash){
 uint32_t n=0,count=imin(CAPTURE_CHUNK,size-offset);p[n++]=0xf0;p[n++]=0x7d;p[n++]=0x52;p[n++]=0x42;p[n++]=3;
 p[n++]=CAPTURE_VERSION&127;p[n++]=CAPTURE_VERSION>>7;p[n++]=frame&127;p[n++]=(frame>>7)&127;
 p[n++]=offset&127;p[n++]=(offset>>7)&127;p[n++]=size&127;p[n++]=(size>>7)&127;
 for(int k=0;k<4;k++)p[n++]=(hash>>(7*k))&127;
 for(uint32_t j=0;j<count;){uint32_t bits=n++;p[bits]=0;for(int k=0;k<7&&j<count;k++,j++){uint8_t b=state[offset+j];p[bits]|=(b>>7)<<k;p[n++]=b&127;}}
 p[n++]=0xf7;return n;
}
