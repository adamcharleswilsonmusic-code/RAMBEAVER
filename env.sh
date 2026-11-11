# SPDX-License-Identifier: GPL-3.0-only
# Source from the repository root: . ./env.sh
RAMBEAVER_ROOT="${RAMBEAVER_ROOT:-$PWD}"
export JIELI_TOOLCHAIN="${JIELI_TOOLCHAIN:-$HOME/.jieli/toolchain}"
export AC79_SDK="${AC79_SDK:-$RAMBEAVER_ROOT/vendor/ac79}"
export JIELI_DOCKER_IMAGE="${JIELI_DOCKER_IMAGE:-debian:bookworm-slim@sha256:7c7b2c966bc9ee8cedfeef67e0e279108992c77681fa595db4a9d65c06ccc587}"
export X0X_FONTSET=terminus
if [ -x "$RAMBEAVER_ROOT/.venv/bin/python" ]; then
    export PATH="$RAMBEAVER_ROOT/.venv/bin:$PATH"
    export PYTHON="${PYTHON:-$RAMBEAVER_ROOT/.venv/bin/python}"
else
    export PYTHON="${PYTHON:-python3}"
fi
