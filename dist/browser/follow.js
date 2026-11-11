// SPDX-License-Identifier: GPL-3.0-only
// Read-only render snapshots. No keyboard replay, game simulation, audio, or storage.
export function hashState(bytes){let h=2166136261;for(const b of bytes)h=Math.imul(h^b,16777619)>>>0;return h&0x0fffffff;}
export function unpackState(packed,size){
 const out=new Uint8Array(size);let i=0,w=0;
 while(i<packed.length){const tag=packed[i++],n=(tag&127)+1;if(w+n>size)return null;
  if(tag&128){if(i>=packed.length)return null;out.fill(packed[i++],w,w+n);}
  else {if(i+n>packed.length)return null;out.set(packed.subarray(i,i+n),w);i+=n;}
  w+=n;
 }
 return w===size?out:null;
}
export class FrameDecoder {
 constructor(version,size,onFrame){this.version=version;this.size=size;this.onFrame=onFrame;this.previous=null;this.previousId=-1;this.reset();}
 reset(){this.id=-1;this.offset=0;this.bytes=null;}
 feed(p){
  if(p.length<18||p[0]!==240||p[1]!==125||p[2]!==82||p[3]!==66||(p[4]!==3&&p[4]!==4)||p.at(-1)!==247)return false;
  if(p.slice(1,-1).some(v=>v>127))return false;
  const version=p[5]|p[6]<<7,id=p[7]|p[8]<<7,offset=p[9]|p[10]<<7,size=p[11]|p[12]<<7;
  const delta=p[4]===4;
  const hash=p[13]|p[14]<<7|p[15]<<14|p[16]<<21;
  if(version!==this.version)throw new Error(`Device and preview must both be build ${this.version}.`);
  if(size<1||size>this.size+Math.ceil(this.size/128)+4)return false;
  if(offset===0){this.id=id;this.offset=0;this.hash=hash;this.delta=delta;this.bytes=new Uint8Array(size);}
  if(id!==this.id||offset!==this.offset||hash!==this.hash||delta!==this.delta||!this.bytes){this.reset();return false;}
  let j=17,o=offset;
  while(j<p.length-1){const bits=p[j++];for(let k=0;k<7&&j<p.length-1;k++){if(o>=size){this.reset();return false;}this.bytes[o++]=p[j++]|((bits>>k&1)<<7);}}
  if(o-offset!==Math.min(84,size-offset)){this.reset();return false;}
  this.offset=o;
  if(o===size){const data=unpackState(this.bytes,this.size);this.reset();
   if(!data||(delta&&(!this.previous||id!==((this.previousId+1)&16383)))){this.previous=null;return false;}
   if(delta)for(let i=0;i<data.length;i++)data[i]^=this.previous[i];
   if(hashState(data)!==hash){this.previous=null;return false;}
   this.previous=data.slice();this.previousId=id;this.onFrame(data,hash,id);return true;}
  return false;
 }
}
export async function connectFollow({onFrame,onStatus,onStop}){
 if(!navigator.requestMIDIAccess)throw new Error('Use a browser with Web MIDI support, such as Chrome or Edge.');
 const access=await navigator.requestMIDIAccess({sysex:true});
 const candidates=[...access.inputs.values()].filter(p=>/FM-1|Felucca/i.test(p.name||''));
 if(candidates.length!==1)throw new Error(candidates.length?'Connect only the FM-1 you want to follow.':'No FM-1 found. Connect its USB cable and try again.');
 const input=candidates[0],outputs=[...access.outputs.values()].filter(p=>p.name===input.name);
 if(outputs.length!==1)throw new Error('The matching FM-1 output port is unavailable.');
 const output=outputs[0];let stopped=false,lastFrame=0,count=0,firstFrame=0,timer;
 const {instance}=await WebAssembly.instantiate(await(await fetch('omni.wasm')).arrayBuffer(),{wasi_snapshot_preview1:new Proxy({},{get:()=>()=>0}),env:new Proxy({},{get:()=>()=>0})});
 const ex=instance.exports;if(ex._initialize)ex._initialize();ex.web_boot();
 function stop(reason='Device follow stopped. Browser progress is unchanged.'){
  if(stopped)return;stopped=true;clearInterval(timer);try{output.send([0xbf,119,0]);}catch{}input.onmidimessage=null;access.onstatechange=null;input.close();output.close();onStop(reason);
 }
 const decoder=new FrameDecoder(ex.web_capture_version(),ex.web_capture_size(),(bytes,hash,id)=>{
  new Uint8Array(ex.memory.buffer,ex.web_capture_ptr(),bytes.length).set(bytes);
  if(!ex.web_capture_apply(hash))return;
  const now=performance.now();lastFrame=now;if(!firstFrame)firstFrame=now;count++;
  const dv=new DataView(bytes.buffer),xruns=dv.getUint32(bytes.length-8,true),cpu=dv.getUint32(bytes.length-4,true);
  onFrame(new Uint16Array(ex.memory.buffer,ex.web_fb(),57600).slice());
  onStatus(`Following FM-1 · silent · ${count>1?(1000*(count-1)/(now-firstFrame)).toFixed(1):'…'} fps · Record captures video + FM-1 USB audio.`,{id,xruns,cpu});
 });
 try{await input.open();await output.open();}catch(e){stop();throw e;}
 input.onmidimessage=e=>{try{decoder.feed(Array.from(e.data));}catch(e){stop(e.message);}};
 access.onstatechange=()=>{if(input.state==='disconnected'||output.state==='disconnected')stop('FM-1 disconnected. Reconnect and choose Follow FM-1 again.');};
 const began=performance.now();output.send([0xbf,119,127]);onStatus('Waiting for FM-1 build 742…');
 timer=setInterval(()=>{if(document.hidden){stop('Follow stopped while hidden. Choose Follow FM-1 to reconnect.');return;}
  const age=performance.now()-(lastFrame||began);if(age>5000){stop('No complete device frames. Check build 742 and reconnect.');return;}
  if(age>750){decoder.reset();onStatus('Waiting for a fresh device frame — resynchronizing…');}
  output.send([0xbf,119,127]);
 },1000);
 return {stop};
}
