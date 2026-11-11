# Record game video and FM-1 USB audio

Recording is a macOS feature implemented with AVFoundation, Node.js, and Python/Mido. Install Apple Command Line Tools, Node.js 18+, and the README's Python dependencies, then run `python tools/prepare_recorder.py` once.

Start `python tools/launch.py record` or double-click Launch Recorder.command. Keep the launch window open. In Chrome at `http://127.0.0.1:8765/`, choose Follow FM-1 and approve MIDI/SysEx access. Allow audio capture for the launching app when macOS asks. The recorder selects only the device named **FoMni FM-1**; a missing or unauthorized device is an error, with no microphone fallback.

Click **Record** in the game viewer. Play using the physical FM-1 controls. The status shows recorded frames and elapsed time. Keep the viewer visible; hiding or closing it ends the follower's stream. Click **Stop & Export** when finished. Export keeps every received scene, renders it with a take-frozen matching 745 renderer, and embeds direct USB game audio in `~/Desktop/RAMBEAVER-YYYY-MM-DD_HH-MM-SS.mov`. The exact filename and an Open finished movie link appear in the viewer.

Set the `RAMBEAVER_EXPORT_DIR` environment variable before launching to choose another final folder. Raw states, timestamps, audio, a frozen renderer, and capture/export reports remain under `recordings/take-.../`. These private take folders are ignored by Git. Each take has a unique timestamp; existing Desktop movies are not overwritten.

The recorder is passive: it opens MIDI input only and sends no gameplay, save, or firmware commands. The browser follower owns the capture request/lease. Missing or corrupt state packets are rejected; dependent deltas wait for the next complete base frame. Export holds the previous received picture across gaps and does not invent motion. A stream interruption is reported and preserves the received portion. Audio failure blocks normal export instead of silently delivering a video-only movie.

The MOV uses 240×240 H.264 video at the received frame timestamps and stereo 44.1kHz PCM USB audio. Host timestamps align the tracks. This is a reconstructed device-state recording rather than LCD filming; physical display/USB audio latency has not been calibrated. For editing with a separate camera recording, capture a visible/audible synchronization cue yourself.

Browser play and the silent device viewer work without the recording service. If the service is offline, the panel explains how to start it. Recording helpers do not update or reboot the instrument.
