# Install and connect

Use `dist/RAMBEAVER-FM-1_745.fwsc` from this release. It installs a game firmware application on the M-VAVE FM-1.

1. Install the Python dependencies listed in the README.
2. Connect the powered FM-1 using a USB data cable.
3. Stop recording and click Stop following in any browser viewer.
4. Run `python tools/fm1_install.py dist/RAMBEAVER-FM-1_745.fwsc` and confirm the install.
5. Keep the USB connection intact until the installer restarts the device and reports `FM-1_745`.

To inspect identity without installing, run `python tools/fm1_install.py --info`. To choose among devices, add `--port PART_OF_PORT_NAME`. The updater filters for FM-1/Felucca-style ports and verifies the model before writing; no `--force` option is needed for this package.

If identity discovery times out, close the browser follower or other MIDI clients, then retry. If the device remains in its update loader after an interrupted transfer, rerun the same installer/package to finish. Do not use unrelated rescue blobs.

To play or follow, run Launch Preview.command on macOS or `python3 tools/launch.py preview`. The local viewer and firmware must have matching capture versions. Build 745 uses 5016-byte snapshots. Browser play has separate saves and begins sound only after Start game & sound. Follow FM-1 is silent and reads the physical device state.
