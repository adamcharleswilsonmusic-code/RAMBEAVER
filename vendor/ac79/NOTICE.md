# Minimal AC79 package inputs

These three files are copied without modification from JieLi SDK commit `d179b4484759423312073f5fbb232501aa491047`:

- `cpu/wl82/tools/uboot.boot`
- `cpu/wl82/tools/cfg_tool.bin`
- `cpu/wl82/tools/cfg/eq_cfg_hw.bin`

Upstream: https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK/tree/d179b4484759423312073f5fbb232501aa491047

The SDK root LICENSE is retained here and in LICENSES/AC79-SDK-LICENSE.txt. Pinned Felucca credits and FoMni licensing identify these package files as Apache-2.0. They are opaque vendor binaries; the retained evidence does not independently audit their internals. No full SDK checkout is bundled.

Their SHA256 values are checked by tools/build.py and listed in release/source-inventory.json. Set AC79_SDK to this vendor/ac79 directory when rebuilding with tools/fm1pkg_make.py directly; env.sh does so for the normal build.

A release-preparation check found no additional LICENSE, COPYING or NOTICE files at cpu/, cpu/wl82/, cpu/wl82/tools/ or cpu/wl82/tools/cfg/. Other SDK subtrees carry notices for their own networking/OS/example components; those subtrees are not included or linked by this game. The root-license and pinned upstream-credit limitation above remains.
