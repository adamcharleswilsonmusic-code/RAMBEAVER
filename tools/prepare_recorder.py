#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Compile local macOS recording helpers. Does not access the FM-1."""
import json,platform,shutil,subprocess,sys
from pathlib import Path
root=Path(__file__).resolve().parent.parent
if platform.system()!='Darwin':raise SystemExit('Recording requires macOS AVFoundation. Browser play and the silent device viewer remain available on other platforms.')
if not shutil.which('node'):raise SystemExit('Install Node.js before preparing the recorder.')
if not shutil.which('swiftc'):raise SystemExit('Install Apple Command Line Tools (xcode-select --install), then retry.')
config=json.loads((root/'config/recorder.json').read_text());out=root/f"build/recorder-{config['revision']}";out.mkdir(parents=True,exist_ok=True)
cache=out/'module-cache';cache.mkdir(exist_ok=True)
for source,target in [('record_fullrun_audio.swift','audio'),('export_fullrun.swift','export')]:
 subprocess.run(['swiftc','-module-cache-path',str(cache),str(root/'tools'/source),'-o',str(out/target)],check=True)
subprocess.run(['node',str(root/'tools/verify_renderer.mjs')],check=True,cwd=root)
print('Recorder prepared. Start Launch Recorder.command; recording begins only when you click Record.')
