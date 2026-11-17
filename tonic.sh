#!/bin/bash
# tonic.sh: unpack the newest FM1 Tonic zip (found anywhere in your home folder), build the emulator, open it. ONE command:
#     bash ~/tonic.sh              (the highest fm1-*wip<number>.zip found anywhere in your home folder)
#     bash ~/tonic.sh file.zip     (a zip you name)
# Stop the emulator with Ctrl+C in this terminal. No need for Ctrl+Shift+R: the server sends "no cache".
set -u
TDIR="$HOME/fm1-x0x-main"
STAMP="$HOME/.tonic_last_zip"
PIDF="$HOME/.tonic_server_pid"
PORT=8000

die() { echo "ERROR: $*" >&2; exit 1; }

[ -f "$HOME/emsdk/emsdk_env.sh" ] || die "emscripten not found in ~/emsdk (START_LINUX.txt, block C)"

# 1. which zip: the one you named, else the highest wip number anywhere in your home folder
ZIP="${1:-}"
if [ -z "$ZIP" ]; then   # search the whole home folder (Desktop, Downloads, anywhere) and the current folder; highest wip number wins, then newest
    ZIP=$(find "$HOME" "$PWD" -maxdepth 5 \( -name '.*' -o -path "$TDIR" -o -path "$HOME/emsdk" \) -prune -o \
          -type f -name 'fm1-*wip[0-9]*.zip' -printf '%T@ %p\n' 2>/dev/null |
          sed -E 's/^([0-9.]+) (.*wip([0-9]+).*)$/\3 \1 \2/' | sort -k1,1n -k2,2n | tail -n 1 | cut -d' ' -f3-)
fi
[ -n "$ZIP" ] && echo "Zip: $ZIP"

# 2. stop the server of the last run (it would serve a folder we are about to replace)
if [ -f "$PIDF" ]; then kill "$(cat "$PIDF")" 2>/dev/null; rm -f "$PIDF"; fi

# 3. unpack only if it is a new or changed zip
if [ -n "$ZIP" ] && [ -f "$ZIP" ]; then
    SIG="$ZIP $(stat -c %Y "$ZIP") $(stat -c %s "$ZIP")"
    if [ ! -d "$TDIR" ] || [ "$(cat "$STAMP" 2>/dev/null)" != "$SIG" ]; then
        echo "Unpacking $(basename "$ZIP") ..."
        rm -rf "$TDIR"
        unzip -q "$ZIP" -d "$HOME" || die "unzip failed"
        echo "$SIG" > "$STAMP"
    else
        echo "Using $(basename "$ZIP") (already unpacked)"
    fi
elif [ -d "$TDIR" ]; then
    echo "No zip found: using the firmware already in ~/fm1-x0x-main"
else
    die "no fm1-*wip<number>.zip found in your home folder and no ~/fm1-x0x-main"
fi
[ -f "$TDIR/web/emu/build.sh" ] || die "the zip does not look right (no web/emu/build.sh)"

# 4. build; on a failure show the end of the log and stop (nothing is started)
export EMSDK_QUIET=1
# shellcheck disable=SC1091
set +u
source "$HOME/emsdk/emsdk_env.sh" >/dev/null 2>&1 || die "could not load emsdk"
set -u
cd "$TDIR" || die "cannot enter $TDIR"
LOG=$(mktemp)
echo "Building ..."
if ! sh web/emu/build.sh >"$LOG" 2>&1; then
    echo "BUILD FAILED. Last lines:"
    tail -n 25 "$LOG"
    echo "Full log: $LOG   (send me the lines above)"
    exit 1
fi
grep -E '^emu:' "$LOG"

# 5. serve it (first free port from 8000), no cache, and open the browser
while ss -ltn 2>/dev/null | grep -q ":$PORT "; do PORT=$((PORT + 1)); done
cd build/emu || die "no build/emu"
echo "Emulator: http://localhost:$PORT   (press \"Switch on\"; Ctrl+C here to stop)"
( sleep 1; xdg-open "http://localhost:$PORT" >/dev/null 2>&1 || true ) &
trap 'rm -f "$PIDF"' EXIT
python3 - "$PORT" "$PIDF" <<'PY'
import http.server, mimetypes, os, socketserver, sys
mimetypes.add_type("application/wasm", ".wasm")
mimetypes.add_type("text/javascript", ".js")
class H(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Cache-Control", "no-store")
        super().end_headers()
    def log_message(self, *a):
        pass
open(sys.argv[2], "w").write(str(os.getpid()))
socketserver.ThreadingTCPServer.allow_reuse_address = True
try:
    with socketserver.ThreadingTCPServer(("127.0.0.1", int(sys.argv[1])), H) as s:
        s.serve_forever()
except KeyboardInterrupt:
    pass
PY
