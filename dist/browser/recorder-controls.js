/* SPDX-License-Identifier: GPL-3.0-only
 * Local passive recording service; never writes to the FM-1 or starts playback. */
(()=>{
 const base='http://127.0.0.1:8766',panel=document.createElement('section');panel.id='device-recording';panel.style.cssText='margin-top:16px;border:1px solid #56686e;border-radius:10px;padding:14px;background:#172631';
 panel.innerHTML='<strong>Record game video + FM-1 USB audio</strong><div class="controls"><button id="take-record" class="primary" disabled>● Record</button><button id="take-stop" disabled>■ Stop &amp; Export</button></div><p class="status" id="take-status" aria-live="polite">Connecting to recorder…</p><p class="status" id="take-path" style="overflow-wrap:anywhere;user-select:all">Desktop/RAMBEAVER-YYYY-MM-DD_HH-MM-SS.mov</p><a id="take-movie" hidden target="_blank" rel="noopener">Open finished movie</a><p class="status">Play on the FM-1. Keep this viewer visible. Stop automatically exports all recorded scenes with USB audio to your Desktop.</p>';
 const anchor=document.getElementById('follow-status');anchor.insertAdjacentElement('afterend',panel);
 const record=panel.querySelector('#take-record'),stop=panel.querySelector('#take-stop'),status=panel.querySelector('#take-status'),path=panel.querySelector('#take-path'),movie=panel.querySelector('#take-movie');let token='',busy=false;
 async function refresh(){try{const s=await(await fetch(base+'/status')).json();token=s.token;const following=!document.getElementById('stop-follow').hidden;
 record.disabled=busy||!following||!['ready','complete','stopped','error'].includes(s.status);stop.disabled=busy||!['starting','recording'].includes(s.status);
 status.textContent=s.status.toUpperCase()+' · '+s.message+(s.frames?` · ${s.frames} frames · ${s.seconds}s`:'');if(!following&&s.status==='ready')status.textContent='Connect Follow FM-1 above, then press Record.';
 path.textContent=s.output||'Desktop/RAMBEAVER-YYYY-MM-DD_HH-MM-SS.mov';movie.hidden=s.status!=='complete';movie.href=base+'/movie';
 }catch(e){record.disabled=stop.disabled=true;status.textContent='Recorder is offline. Open Launch Recorder.command in the RAMBEAVER folder.';}}
 async function action(name){busy=true;record.disabled=stop.disabled=true;try{const r=await fetch(base+'/'+name,{method:'POST',headers:{'X-Recorder-Token':token}});const s=await r.json();if(!r.ok)throw Error(s.error);}catch(e){status.textContent=e.message;}finally{busy=false;await refresh();}}
 record.onclick=()=>action('record');stop.onclick=()=>action('stop');setInterval(refresh,500);refresh();
})();
