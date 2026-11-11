/* SPDX-License-Identifier: GPL-3.0-only
 * Optional bounded USB-MIDI view stream. Main-loop snapshots, USB ISR packet producer/consumer.
 * CC119/channel16 renews a three-second lease; zero stops. Never blocks audio,
 * never receives game inputs, never writes flash. OTA immediately owns USB. */
#pragma once
static capture_state_t capture_copy;
static uint8_t capture_previous[sizeof(capture_state_t)],capture_previous_valid,capture_delta;
static uint8_t capture_packed[CAPTURE_PACKED_MAX];
static uint32_t capture_packed_size;
static uint32_t capture_lease,capture_next,capture_frame,capture_checksum;
static volatile uint32_t capture_offset;
static volatile uint8_t capture_enabled,capture_sending;
static void capture_stop(void){uint8_t was=capture_enabled;capture_enabled=capture_sending=0;capture_previous_valid=0;mo_r=mo_w;if(was&&usb.config){midi_out_q[mo_w%MQ]=0x0000f705u;RING_PUBLISH();mo_w++;}}
static void capture_poll(void){
 uint32_t pkt,now=fm1_ms;
 while(plat_midi_in(&pkt))if(((pkt>>8)&255)==0xbf&&((pkt>>16)&127)==119){
  if((pkt>>24)&127){if(!capture_enabled)capture_previous_valid=0;capture_enabled=1;capture_lease=now;}else capture_stop();
 }
 if(!usb.config||usb.ota_req||usb.uboot_req||sx_ready||sx_busy){if(capture_enabled)capture_stop();return;}
 if(!capture_enabled)return;
 if(now-capture_lease>3000u){capture_stop();return;}
 if(!capture_sending){
  if((int32_t)(now-capture_next)<0)return;
  capture_copy.world=game;capture_copy.beat=om_music_beat();capture_copy.music_step=om_audio_stat(2);capture_copy.xruns=audio_xruns;capture_copy.cpu=audio_cpu_pct;
  capture_checksum=capture_hash((const uint8_t*)&capture_copy,sizeof capture_copy);capture_frame=(capture_frame+1)&16383u;
  /* Full independent frame on connection and every16 frames. Otherwise XOR
   * against the preceding authoritative snapshot before bounded byte packing. */
  capture_delta=capture_previous_valid&&((capture_frame&15u)!=0);uint8_t *raw=(uint8_t*)&capture_copy;
  for(uint32_t i=0;i<sizeof capture_copy;i++){uint8_t current=raw[i];if(capture_delta)raw[i]^=capture_previous[i];capture_previous[i]=current;}
  capture_previous_valid=1;capture_packed_size=capture_pack(capture_packed,raw,sizeof capture_copy);capture_offset=0;capture_next=now+25u;RING_PUBLISH();capture_sending=1;
 }
}
/* One bounded chunk after USB polling, outside the audio render. It must not
 * wait for the main loop, whose LCD work can take tens of milliseconds. */
static void capture_tx_poll(void){
 if(!capture_enabled||!capture_sending)return;
 if(!usb.config||usb.ota_req||usb.uboot_req||sx_ready||sx_busy){capture_stop();return;}
 RING_PUBLISH();
 uint8_t p[128];uint32_t n=capture_packet(p,capture_packed,capture_packed_size,capture_frame,capture_offset,capture_checksum),packets=(n+2)/3;
 p[4]=capture_delta?4:3;
 if(mo_w-mo_r+packets>MQ)return; /* Backpressure: retry, never busy-wait. */
 for(uint32_t i=0;i<n;){uint32_t k=imin(3,n-i),cin=k==3&&i+3<n?4:k+4;
  midi_out_q[mo_w%MQ]=cin|(uint32_t)p[i]<<8|(k>1?(uint32_t)p[i+1]<<16:0)|(k>2?(uint32_t)p[i+2]<<24:0);RING_PUBLISH();mo_w++;i+=k;
 }
 capture_offset+=imin(CAPTURE_CHUNK,capture_packed_size-capture_offset);
 if(capture_offset>=capture_packed_size)capture_sending=0;
}
