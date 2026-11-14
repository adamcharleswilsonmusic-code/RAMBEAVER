# RAMBEAVER

A side-scrolling action game for the **M-VAVE FM-1**. Play a beaver with a red headband, defend a dam, ride through the robot city, fly to the data center, and shut down Mother. Original game art and gameplay run on the instrument's display, with synthesized music, effects, and recorded character voices.

**Current build: FM-1_745.** This release includes the exact firmware installed and verified on October 17, 2026, its corresponding source, a matching browser viewer, and a one-page guide. The public repository is [adamcharleswilsonmusic-code/RAMBEAVER](https://github.com/adamcharleswilsonmusic-code/RAMBEAVER).

## Play

Download or clone this repository, then open the [quick-start guide](docs/rambeaver-quick-start.pdf). The ready-to-install firmware is [RAMBEAVER-FM-1_745.fwsc](dist/RAMBEAVER-FM-1_745.fwsc).

For browser play, install Python 3.10 or newer and run:

```sh
python3 tools/launch.py preview
```

On macOS, double-click **Launch Preview.command**. Keep its window open. The included browser build is served locally at `http://127.0.0.1:8765/`. Click **Start game & sound** to play. **Follow FM-1** instead mirrors a connected device silently; it does not run a second game or overwrite progress.

## Install on the FM-1

Install the local Python dependencies once:

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install -r requirements.txt
```

Connect the FM-1 with a USB data cable. Stop any active recording and choose **Stop following** in the browser before updating. The package replaces the instrument's current firmware application.

```sh
python tools/fm1_install.py dist/RAMBEAVER-FM-1_745.fwsc
```

The installer asks before writing, checks the package, transfers it, restarts the FM-1, and verifies its identity. Keep the cable connected until it reports `FM-1_745`. Read [installation details](docs/INSTALL.md) for port selection and connection troubleshooting.

## Controls and progress

The first two white keys move left/right. The third white key from the right ducks or drops through a thin platform; the last two white keys jump and fire. The first black key enters doors, the second black key from the right summons the posse, and the last black key throws a firebomb. **PLAY/STOP** opens the menu; turn **SELECT** and press **SEL** to choose.

Health logs restore one missing heart. The posse recharges in12 seconds. The bike moves forward automatically and launches from ramps; jetpack flight also advances automatically, with Jump held to rise and released to descend.

**Pause before power-off and wait for PROGRESS SAVED.** Live saves queue until Pause or death. Continue resumes saved progress; Retry returns to the checkpoint and ends flawless eligibility. Browser saves are separate from device saves.

A fresh run with no actual damage, death, or retry and every enemy defeated, including both side rooms, unlocks **Golden Beaver**. Pause shows eligibility. A fresh Jump/Fire press advances the exclusive unlock page to the epilogue; release and press again to start a golden New Run. Save the unlock before switching off.

## Record a playthrough

The integrated **Record / Stop & Export** controls capture actual device frames with **direct FoMni FM-1 USB audio**. Every recorded scene is exported; there is no microphone or browser-synth fallback. Recording requires macOS, Apple Command Line Tools, Node.js 18+, and the Python dependencies above. The browser preview and silent viewer do not require these recording helpers.

```sh
python tools/prepare_recorder.py
python tools/launch.py record
```

Or double-click **Launch Recorder.command** after preparation. In Chrome, select **Follow FM-1**, approve MIDI/SysEx access, then click **Record** when ready. Keep the viewer visible. **Stop & Export** finishes the take and creates:

```text
~/Desktop/RAMBEAVER-YYYY-MM-DD_HH-MM-SS.mov
```

The exact path and status appear in the viewer. Raw frames, a frozen matching renderer, USB audio, and reports remain in `recordings/take-.../`. Set `RAMBEAVER_EXPORT_DIR` to choose another destination. See [recording details](docs/RECORDING.md).

## Build and modify

All game, platform, loader, DSP, graphics, fonts, packaging scripts, and generated score/voice headers needed for build 745 are included. The three small package inputs from the pinned AC79 SDK are in `vendor/ac79/`; the compiler and full SDK are not bundled.

See [BUILDING.md](BUILDING.md) for the tested JieLi toolchain, Docker configuration, native tests, and browser rebuild. Building never installs or reboots the device. The supplied firmware keeps identity745; do not use the inherited `--release` option for a matching 745 rebuild.

## Verification

Build 745 clips ground hatching to solid-ground edges without changing collision geometry. Native checks covered 8,820 camera/edge views and four browser/native capture fixtures. Its installation passed four identity checks over30 seconds, and a five-second device capture exported 141 frames with USB audio, zero corrupted frames, no added audio underruns, and zero dropped audio buffers. Full physical gameplay and speaker quality remain hands-on checks. Host timestamps align capture and audio; end-to-end LCD/USB latency is not calibrated.

The source/build checksums, clean rebuild results, and release contents are in [release/manifest.json](release/manifest.json) and [SHA256SUMS](SHA256SUMS).

## Credits and licenses

The source code is **GPL-3.0-only**, built on **Felucca → X0X → FoMni**, with author notices preserved. Thanks to **Leo Kuroshita (@kurogedelic), Hügelton Instruments**, and **Charles Vestal**. Terminus and Barlow fonts retain SIL OFL 1.1 notices; SDK and browser runtime components keep their own licenses.

Read [LICENSING.md](LICENSING.md), [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md), and the [publication checklist](release/PUBLICATION-CHECKLIST.md). Song and character voice recordings supplied by **Adam Wilson**. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for asset provenance and component licenses.

RAMBEAVER is independent software and is not endorsed by M-VAVE, Hügelton Instruments, or the other upstream projects.

You can also try RAMBEAVER in the browser, no flashing needed, in the free community [FM-1 Emulator](https://fm1-emulator.atst.tools/).
