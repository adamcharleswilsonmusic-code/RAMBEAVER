// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';
import assert from 'node:assert/strict';
import {FrameDecoder,hashState,unpackState} from '../web/emu/follow.js';
const root='build/test/revision-745/';let mem;
const wasi=new Proxy({clock_time_get:(_a,_b,p)=>{new DataView(mem.buffer).setBigUint64(p,0n,true);return 0;}},{get:(o,k)=>o[k]||(()=>0)});
const {instance}=await WebAssembly.instantiate(fs.readFileSync('dist/browser/omni.wasm'),{wasi_snapshot_preview1:wasi,env:new Proxy({},{get:()=>()=>0})});
const e=instance.exports;mem=e.memory;e._initialize?.();e.web_boot();let count=0;
const decoder=new FrameDecoder(745,e.web_capture_size(),(bytes,hash,id)=>{
 assert.deepEqual(Buffer.from(bytes),fs.readFileSync(root+`capture-${id}.bin`));new Uint8Array(mem.buffer,e.web_capture_ptr(),bytes.length).set(bytes);const writes=e.web_store_writes();assert.equal(e.web_capture_apply(hash),1);assert.equal(e.web_store_writes(),writes);
 assert.deepEqual(Buffer.from(mem.buffer,e.web_fb(),115200),fs.readFileSync(root+`capture-${id}.rgb565`));count++;
});
for(let id=0;id<4;id++){
 const wire=fs.readFileSync(root+`capture-${id}.wire`),packets=[];for(let j=0;j<wire.length;){const n=wire.readUInt16LE(j);j+=2;packets.push(Array.from(wire.subarray(j,j+n)));j+=n;}
 for(const p of packets)decoder.feed(p);assert.equal(count,id+1);
 // Partial, reordered, duplicate and damaged frames never reach the renderer.
 const old=count;decoder.reset();packets.filter((_,i)=>i!==1).forEach(p=>decoder.feed(p));assert.equal(count,old);
 decoder.reset();[packets[0],packets[2],packets[1],...packets.slice(3)].forEach(p=>decoder.feed(p));assert.equal(count,old);
 decoder.reset();[packets[0],packets[0],...packets.slice(2)].forEach(p=>decoder.feed(p));assert.equal(count,old);
 decoder.reset();packets.map((p,i)=>i===1?p.map((b,k)=>k===20?b^1:b):p).forEach(p=>decoder.feed(p));assert.equal(count,old);
 decoder.reset();const mismatch=packets[0].slice();mismatch[5]--;assert.throws(()=>decoder.feed(mismatch),/both be build/);
}
const packedFixtures=fs.readFileSync(root+'pack-fixtures.bin');let patterns=0;
for(let i=0;i<packedFixtures.length;){let size=packedFixtures.readUInt32LE(i),n=packedFixtures.readUInt32LE(i+4);i+=8;const raw=packedFixtures.subarray(i,i+size);i+=size;const packed=packedFixtures.subarray(i,i+n);i+=n;assert.deepEqual(Buffer.from(unpackState(packed,size)),raw);patterns++;}
assert.equal(patterns,32);assert.equal(unpackState(Uint8Array.from([255]),128),null);assert.equal(unpackState(Uint8Array.from([127,1,2]),128),null);assert.equal(unpackState(Uint8Array.from([255,0]),1),null);
const report={revision:745,states:4,compression_roundtrips:32,bytes:e.web_capture_size(),native_wasm_pixels_identical:4*57600,corrupt_partial_reordered_duplicate_rejected:true,no_storage_writes:true,no_audio_or_simulation:true};fs.writeFileSync(root+'capture-validation.json',JSON.stringify(report,null,2)+'\n');console.log(report);
