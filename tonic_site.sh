#!/bin/bash
# tonic_site.sh: unpack the newest FM1 Tonic zip (found anywhere in your home folder), build the firmware (release style), make the
# local website (installer, back up, break loops, emulator, manual) and open its Install page. ONE command:
#     bash ~/tonic_site.sh             (the highest fm1-*wip<number>.zip found anywhere in your home folder)
#     bash ~/tonic_site.sh file.zip    (a zip you name)
# Then press Install on the page (Chrome or Edge, the FM-1 plugged in). Stop the site with Ctrl+C in this terminal.
# It shares the unpack folder and stamp with tonic.sh, so the two never wipe each other's work. The site runs on
# port 8001 (tonic.sh uses 8000), so both can be on at once.
set -u
TDIR="$HOME/fm1-x0x-main"
STAMP="$HOME/.tonic_last_zip"           # the same file as tonic.sh
PIDF="$HOME/.tonic_site_pid"
PORT=8001

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

# 2. stop the site of the last run
if [ -f "$PIDF" ]; then kill "$(cat "$PIDF")" 2>/dev/null; rm -f "$PIDF"; fi

# 3. unpack only if it is a new or changed zip
SIG=""
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
    SIG="folder $(stat -c %Y "$TDIR/VERSION" 2>/dev/null)"
else
    die "no fm1-*wip<number>.zip found in your home folder and no ~/fm1-x0x-main"
fi
cd "$TDIR" || die "cannot enter $TDIR"
[ -f tools/make_pages.py ] && [ -f web/emu/build.sh ] || die "the zip does not look right (no tools/make_pages.py)"

# 4. build (skipped when this zip was already built for the site)
REL="$(tr -d ' \r\n' < VERSION)"      # wip433: version 1.0 = no "-beta" any more
PKG="build/fm1-tonic-$REL.fwsc"
MARK="build/.site_built"
if [ -f "$PKG" ] && [ -d build/pages/install ] && [ "$(cat "$MARK" 2>/dev/null)" = "$SIG" ]; then
    echo "Site already built for this zip (delete build/.site_built to force a rebuild)"
else
    export EMSDK_QUIET=1
    set +u
    # shellcheck disable=SC1091
    source "$HOME/emsdk/emsdk_env.sh" >/dev/null 2>&1 || die "could not load emsdk"
    set -u
    LOG=$(mktemp)
    run() {  # run "what" command...   shows the end of the log on a failure
        local what="$1"; shift
        echo "$what ..."
        if ! "$@" >"$LOG" 2>&1; then
            echo "FAILED: $what. Last lines:"
            tail -n 25 "$LOG"
            echo "Full log: $LOG   (send me the lines above)"
            exit 1
        fi
    }
    run "Building the emulator" sh web/emu/build.sh
    run "Building the firmware $REL" sh ./build.sh --release "$REL"
    [ -f "$PKG" ] || die "the build did not make $PKG"
    run "Making the website" python3 tools/make_pages.py "$PKG" "$REL" build/pages
    echo "$SIG" > "$MARK"
fi
[ -d build/pages/install ] || die "no build/pages/install"

# 5. serve it (first free port from 8001), no cache, and open the Install page
while ss -ltn 2>/dev/null | grep -q ":$PORT "; do PORT=$((PORT + 1)); done
cd build/pages || die "no build/pages"
echo "Install page:  http://localhost:$PORT/install/    Back up:  http://localhost:$PORT/backup/"
echo "(Chrome or Edge, the FM-1 plugged in; Ctrl+C here to stop)"
( sleep 1; xdg-open "http://localhost:$PORT/install/" >/dev/null 2>&1 || true ) &
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
