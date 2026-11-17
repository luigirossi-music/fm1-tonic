#!/bin/bash
# FM1 TONIC emulator: build it and start it in one go (Linux). Run:  bash ~/fm1-x0x-main/start_emulator.sh
# Then open  http://localhost:8000  in the browser, press "Switch on" (Ctrl+Shift+R after a new build). Stop: Ctrl+C.
set -e
export EMSDK_QUIET=1
# shellcheck disable=SC1090
source ~/emsdk/emsdk_env.sh
cd "$(dirname "$0")"
sh web/emu/build.sh
cd build/emu
( sleep 2; xdg-open http://localhost:8000 >/dev/null 2>&1 || true ) &
python3 -m http.server 8000
