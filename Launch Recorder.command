#!/bin/zsh
cd "$(dirname "$0")" || exit 1
if [[ -x .venv/bin/python ]]; then
 exec .venv/bin/python tools/launch.py record
else
 exec python3 tools/launch.py record
fi
