// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';
const config=JSON.parse(fs.readFileSync('config/recorder.json'));let mem;
const wasi=new Proxy({clock_time_get:(_a,_b,p)=>{new DataView(mem.buffer).setBigUint64(p,0n,true);return 0;}},{get:(o,k)=>o[k]||(()=>0)});
const {instance}=await WebAssembly.instantiate(fs.readFileSync(config.renderer),{wasi_snapshot_preview1:wasi,env:new Proxy({},{get:()=>()=>0})});
mem=instance.exports.memory;instance.exports._initialize?.();
if(instance.exports.web_capture_version()!==config.revision||instance.exports.web_capture_size()!==config.state_bytes)throw Error('Recorder configuration and browser renderer do not match');
console.log(`Matching renderer: build ${config.revision}, ${config.state_bytes} bytes`);
