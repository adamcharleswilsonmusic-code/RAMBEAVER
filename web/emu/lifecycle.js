// A panel can retain a document after its view closes. Disconnect first, then
// suspend/close explicitly. Resume is always an intentional button action.
export function bindAudioLifetime({win=window,doc=document,getAudio,getNode,onSuspend=()=>{},onClose=()=>{}}){
 let suspended=false,closed=false;
 function suspend(){const audio=getAudio(),node=getNode();if(!audio||closed||suspended)return;suspended=true;try{node?.disconnect()}catch{};audio.suspend().catch(()=>{});onSuspend();}
 function close(){const audio=getAudio(),node=getNode();if(closed)return;closed=true;suspended=true;try{node?.disconnect()}catch{};if(audio&&audio.state!=='closed')audio.close().catch(()=>{});onClose();}
 function visibility(){if(doc.hidden)suspend();}
 win.addEventListener('blur',suspend);doc.addEventListener('visibilitychange',visibility);win.addEventListener('pagehide',close);win.addEventListener('beforeunload',close);
 return {suspend,close,async resume(){const audio=getAudio(),node=getNode();if(closed||!audio||audio.state==='closed')return false;if(doc.hidden)return false;if(suspended)node?.connect(audio.destination);suspended=false;await audio.resume();return true;},get suspended(){return suspended;},get closed(){return closed;}};
}
