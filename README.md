# RAMBEAVER

A side-scrolling action game for the **M-VAVE FM-1**: defend the dam, ride through robot city, fly to the data center, and defeat Mother.

**Current release: FM-1_745.**

- [Quick-start guide PDF](rambeaver-quick-start.pdf) · [PNG](rambeaver-quick-start.png)
- [Firmware745](RAMBEAVER-FM-1_745.fwsc)
- [Complete corresponding source, browser viewer and recorder](RAMBEAVER-745-source.zip)
- [Checksums](SHA256SUMS.txt) · [Release manifest](manifest.json)
- [Credits and component notices](THIRD_PARTY_NOTICES.md)

Extract the source archive to get the full source tree, build scripts, detailed installation instructions, ready-to-run browser viewer, and macOS Record / Stop & Export helpers. No original song audio, raw microphone recordings, private playthrough footage, or compiler/SDK trees are included.

To preview after extracting:

```sh
cd RAMBEAVER
python3 tools/launch.py preview
```

To install, follow the included README: create a Python virtual environment, install requirements.txt, stop any active viewer/recording, and run tools/fm1_install.py with the included firmware. The installer asks before writing and verifies the restarted745 identity.

The game is GPL-3.0-only source code, with component licenses retained in the complete archive. Its platform derives from **Felucca → X0X → FoMni**, by Leo Kuroshita (@kurogedelic), Hügelton Instruments, and Charles Vestal. Song and character voice recordings supplied by **Adam Wilson**. RAMBEAVER is independent software, without endorsement from the hardware or upstream projects.

Build745 clips forest ground hatching to the solid-ground edges. The package and browser WASM rebuild byte-for-byte from the supplied source. Record / Stop & Export captures actual FM-1 video and direct USB audio to a uniquely named Desktop movie on macOS.
