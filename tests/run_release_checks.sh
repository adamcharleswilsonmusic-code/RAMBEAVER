#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/.."
OUT=build/test/revision-745
mkdir -p "$OUT" build/gen
python3 tools/gen_tables.py build/gen/felucca_tables.h
[ -f build/gen/felucca_font.h ] || python3 tools/gen_font.py build/gen/felucca_font.h >/dev/null
${CC:-cc} -O1 -g -fsanitize=address -ffp-contract=off -std=gnu99 -Wno-unused-function -Wno-unused-parameter \
 -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen tests/capture_745.c -lm -o "$OUT/capture"
"$OUT/capture" > "$OUT/capture.txt"
node tests/capture_745.mjs > "$OUT/capture-browser.txt"
node tools/verify_renderer.mjs
