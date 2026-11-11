// SPDX-License-Identifier: GPL-3.0-only
// Streams every authoritative recorded state, never simulates gameplay.
import fs from 'node:fs';
import {once} from 'node:events';
const [dir,wasmPath]=process.argv.slice(2);let mem;
const wasi=new Proxy({clock_time_get:(_a,_b,p)=>{new DataView(mem.buffer).setBigUint64(p,0n,true);return 0;}},{get:(o,k)=>o[k]||(()=>0)});
const {instance}=await WebAssembly.instantiate(fs.readFileSync(wasmPath),{wasi_snapshot_preview1:wasi,env:new Proxy({},{get:()=>()=>0})});
const ex=instance.exports;mem=ex.memory;ex._initialize?.();ex.web_boot();
const capture=JSON.parse(fs.readFileSync(dir+'/capture.json'));if(ex.web_capture_version()!==capture.revision||ex.web_capture_size()!==capture.state_bytes)throw Error('Capture version/layout does not match take renderer');
const input=fs.openSync(dir+'/frames.bin','r'),header=Buffer.alloc(12),size=ex.web_capture_size();let count=0;
function readExact(buffer,allowEof=false){let n=0;while(n<buffer.length){const k=fs.readSync(input,buffer,n,buffer.length-n,null);if(!k){if(!n&&allowEof)return false;throw Error('Incomplete recorded state');}n+=k;}return true;}
while(readExact(header,true)){
 const timestamp=header.readDoubleLE(0),n=header.readUInt32LE(8);if(n!==size)throw Error('Recorded state layout does not match renderer');
 const state=Buffer.alloc(n);readExact(state);let hash=2166136261;for(const b of state)hash=Math.imul(hash^b,16777619)>>>0;
 new Uint8Array(mem.buffer,ex.web_capture_ptr(),n).set(state);if(!ex.web_capture_apply(hash&0xfffffff))throw Error('Recorded state rejected');
 const src=new Uint8Array(mem.buffer,ex.web_fb(),115200),frame=Buffer.alloc(8+230400);frame.writeDoubleLE(timestamp,0);
 for(let p=0;p<57600;p++){const c=src[p*2]<<8|src[p*2+1],o=8+p*4;frame[o]=(c&31)*255/31;frame[o+1]=(c>>5&63)*255/63;frame[o+2]=(c>>11)*255/31;frame[o+3]=255;}
 if(!process.stdout.write(frame))await once(process.stdout,'drain');count++;
}
fs.closeSync(input);if(!count)throw Error('No device frames were recorded');console.error(`Rendered all ${count} actual device frames`);
