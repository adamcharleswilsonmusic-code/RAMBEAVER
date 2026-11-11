/* SPDX-License-Identifier: GPL-3.0-only */
#define OM_WEB 1
#include "../web/emu/omni_web.c"
#include "../firmware/src/dsp/omni.c"
static int checks;
#define CHECK(c) do{checks++;if(!(c)){fprintf(stderr,"capture745:%d %s\n",__LINE__,#c);exit(1);}}while(0)
static void snapshot(int id){
 capture_state_t state={game,om_music_beat(),om_audio_stat(2),0,12};uint32_t sum=capture_hash((const uint8_t*)&state,sizeof state),writes=store_writes;
 FILE*f;char p[240];snprintf(p,sizeof p,"build/test/revision-745/capture-%d.bin",id);f=fopen(p,"wb");CHECK(f);fwrite(&state,1,sizeof state,f);fclose(f);
 snprintf(p,sizeof p,"build/test/revision-745/capture-%d.wire",id);f=fopen(p,"wb");CHECK(f);
 uint8_t packed[CAPTURE_PACKED_MAX];uint32_t size=capture_pack(packed,(const uint8_t*)&state,sizeof state);CHECK(size<=sizeof packed);
 for(uint32_t off=0;off<size;off+=CAPTURE_CHUNK){uint8_t pkt[128];uint32_t len=capture_packet(pkt,packed,size,id,off,sum);CHECK(len<=128);for(unsigned i=1;i<len-1;i++)CHECK(pkt[i]<128);uint16_t n=len;fwrite(&n,2,1,f);fwrite(pkt,1,len,f);}fclose(f);
 web_capture=state;CHECK(web_capture_apply(sum));CHECK(!memcmp(&game,&state.world,sizeof game)&&writes==store_writes);CHECK(!web_capture_apply(sum^1));
 snprintf(p,sizeof p,"build/test/revision-745/capture-%d.rgb565",id);f=fopen(p,"wb");fwrite(fb,2,57600,f);fclose(f);
 snprintf(p,sizeof p,"build/test/revision-745/capture-%d.png",id);png_write(p);capture_visual_override=0;
}
int main(void){store_len[0]=store_len[1]=-1;boot();world_reset(1);game.visited=511;set_scene(FOREST_SCENE,1108,168,0);game.arrival=game.input_lock=game.intro_phase=0;game.ground=1;game.hp=1;
 int cams[]={1024,1030,1140,1188};for(int id=0;id<4;id++){game.cam=cams[id];game.cam_q8=cams[id]*Q8;snapshot(id);}
 FILE*f=fopen("build/test/revision-745/pack-fixtures.bin","wb");CHECK(f);
 for(int pattern=0;pattern<32;pattern++){
  uint8_t raw[sizeof(capture_state_t)],packed[CAPTURE_PACKED_MAX];uint32_t rng=17u+(unsigned)pattern;
  for(unsigned i=0;i<sizeof raw;i++){rng=rng*1664525u+1013904223u;raw[i]=pattern%4==0?0:pattern%4==1?(uint8_t)i:pattern%4==2?(i%5?0:255):(uint8_t)(rng>>24);}
  uint32_t n=capture_pack(packed,raw,sizeof raw),size=sizeof raw;CHECK(n<=sizeof packed);fwrite(&size,4,1,f);fwrite(&n,4,1,f);fwrite(raw,1,sizeof raw,f);fwrite(packed,1,n,f);
 }fclose(f);
 printf("PASS %d snapshot/protocol checks; state bytes%zu; four ground-edge render fixtures; no state mutation, storage write or simulated input.\n",checks,sizeof(capture_state_t));return 0;}
