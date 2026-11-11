# Building RAMBEAVER745

These commands run from the repository root. Install Python 3.10+ and a C compiler for native checks. Browser rebuilding uses Emscripten 4.0.23. Device compilation uses JieLi's Linux x86-64 toolchain `jieli-linux-toolchains-20260730.1`; on macOS or other non-Linux-x86-64 hosts it runs through Docker's `linux/amd64` support.

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install -r requirements.txt
```

## Device firmware

Obtain the JieLi toolchain from the vendor's [Linux toolchain download](https://pkgman.jieliapp.com/s/linux-toolchain). The optional `tools/get_toolchain.sh` downloads the vendor's current archive into `~/.jieli` and links `~/.jieli/toolchain`; it does not pin the moving download. For an exact745 rebuild, use the tested 20260730.1 directory and verify the executable hashes in `provenance/toolchain.json`. No compiler binary is included in this repository.

Set the compiler path, then build:

```sh
export JIELI_TOOLCHAIN="$HOME/.jieli/toolchain"
. ./env.sh
sh build.sh
```

`env.sh` selects the included minimal SDK inputs in `vendor/ac79`, Terminus fonts, and the tested Docker image digest. Override `AC79_SDK` only when intentionally using another SDK checkout. The three SDK inputs are checked by SHA256 before packaging. Their source commit and notices are in `vendor/ac79/NOTICE.md`.

Output: `build/RAMBEAVER-FM-1_745.fwsc`. The application is 256,756 bytes; data/BSS 44,136 bytes; display pool 288,000 bytes. The installed package SHA256 is recorded in `release/manifest.json`. The clean release-folder rebuild must match it before publication. Do not pass `--release`: that inherited option changes firmware identity/version and produces a different package.

The build validates application/RAM bounds, RAM-only routines, instruction constraints, and HAL register-access boundaries. Neither `build.sh` nor the browser build accesses USB or installs firmware.

## Native and browser builds

```sh
sh host/build_host.sh
```

The native executable reads scripts as described in `host/omni_host.c`; no hardware access is involved.

With Emscripten 4.0.23 activated in your shell:

```sh
OM_VERSION=745 OM_WEB_OUT=build/browser sh web/emu/build.sh
```

The ready-to-run build in `dist/browser/` uses the same firmware sources and capture version 745. It expects 5016-byte states. `tools/verify_renderer.mjs` checks the bundled renderer against `config/recorder.json`.

## Recording helpers and guide

On macOS with Node.js 18+ and Apple Command Line Tools:

```sh
python tools/prepare_recorder.py
```

This compiles the AVFoundation audio/export helpers under `build/recorder-745/`. It neither opens MIDI nor starts audio capture. `python tools/launch.py record` starts the local viewer and recorder service. The browser's Record button starts the take.

To regenerate the one-page PDF:

```sh
python -m pip install -r requirements-docs.txt
python tools/generate_quick_start.py
```

The guide uses standard PDF fonts and the included native screenshot in `docs/assets/`. No private fonts, cache paths, or local recordings are required.

## Generated assets

The exact score, sprite, animation, and 8-bit voice headers are included in `firmware/src/`. A normal firmware build does not require the original song or microphone recordings. `tools/reduce_beaver.py` and `tools/animate_beaver.py` reproduce sprite derivations from the included generated image. Optional authoring tools require `requirements-assets.txt` and Pillow.

The song arrangement scripts use the included note-analysis JSON; they do not need the source MP3 to compile the release. `tools/import_recorded_voices.py --input PATH` is an optional replacement-voice importer and requires your own eight source recordings. Its inputs are never fetched automatically. Do not regenerate scores, levels, art, or voices merely to rebuild 745: the checked-in headers are the authoritative release inputs.

## Release checks

After installing the Python dependencies and a C compiler, run:

```sh
sh tests/run_release_checks.sh
python tools/verify_release.py
```

The first command compares four native/browser ground-edge capture fixtures and transport corruption handling. The second verifies the public source tree, package hashes, unchanged745 firmware source, and private-recording/path exclusions. Both are offline and send no device commands.
