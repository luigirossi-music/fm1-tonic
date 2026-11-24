#!/bin/bash
# future_site.sh: change ONLY the "Future updates" list of the live web page. The firmware, the manual and everything else stay as they are.
#     bash ~/future_site.sh                     (the list = FUTURE_UPDATES.txt next to this file, else ~/fm1-x0x-main/docs/FUTURE_UPDATES.txt)
#     bash ~/future_site.sh /path/FUTURE_UPDATES.txt
# It takes the published site (branch gh-pages of luigirossi-music/fm1-tonic), puts your list into emu/index.html, and sends it back.
# The page updates in 1-3 minutes: https://luigirossi-music.github.io/fm1-tonic/emu/
# Needs git, gh and python3 (the same as github_publish.sh; the first time: sudo apt install -y git gh python3  and  gh auth login).
set -u
OWNER="luigirossi-music"; REPO="fm1-tonic"
REMOTE="${FM1_REMOTE:-https://github.com/$OWNER/$REPO.git}"
SKIPGH="${FM1_SKIP_GH:-}"
die() { echo; echo "STOP: $*" >&2; exit 1; }
HERE="$(cd "$(dirname "$0")" && pwd)"
LIST="${1:-}"
[ -z "$LIST" ] && [ -f "$HERE/FUTURE_UPDATES.txt" ] && LIST="$HERE/FUTURE_UPDATES.txt"
[ -z "$LIST" ] && LIST="$HOME/fm1-x0x-main/docs/FUTURE_UPDATES.txt"
[ -f "$LIST" ] || die "no list file ($LIST). Put FUTURE_UPDATES.txt next to this script or name it: bash ~/future_site.sh /path/FUTURE_UPDATES.txt"
TOOL="$HERE/future_updates.py"
[ -f "$TOOL" ] || TOOL="$HERE/tools/future_updates.py"
[ -f "$TOOL" ] || TOOL="$HOME/fm1-x0x-main/tools/future_updates.py"
[ -f "$TOOL" ] || die "no future_updates.py (put it next to this script, or run: bash ~/tonic.sh once)"
command -v git >/dev/null 2>&1 || die "git is missing. Run: sudo apt install -y git gh"
command -v python3 >/dev/null 2>&1 || die "python3 is missing. Run: sudo apt install -y python3"
if [ -z "$SKIPGH" ]; then
    command -v gh >/dev/null 2>&1 || die "gh (the GitHub tool) is missing. Run: sudo apt install -y git gh"
    gh auth status >/dev/null 2>&1 || die "you are not logged in to GitHub. Run: gh auth login   (answers: GitHub.com, HTTPS, Yes, Login with a web browser)"
    gh auth setup-git >/dev/null 2>&1
fi
W="$(mktemp -d)"; trap 'rm -rf "$W"' EXIT
echo "== 1/3 Taking the published site (gh-pages)"
git clone -q --depth 1 -b gh-pages "$REMOTE" "$W/site" || die "could not get the gh-pages branch (is the site published yet? run github_publish.sh first)"
[ -f "$W/site/emu/index.html" ] || die "the published site has no emu/index.html"
echo "== 2/3 Writing the list into emu/index.html"
python3 "$TOOL" "$LIST" "$W/site/emu/index.html" || die "the list could not be written (read the line above)"
cd "$W/site" || die "cannot enter the site folder"
if git diff --quiet; then echo "Nothing changed: the page already shows this list."; exit 0; fi
echo "== 3/3 Sending it"
git -c user.name="Luigi Rossi" -c user.email="$OWNER@users.noreply.github.com" commit -q -am "Future updates list" || die "git commit failed"
git push -q origin HEAD:gh-pages || die "git push failed (if it says authentication, run: gh auth login)"
[ -z "$SKIPGH" ] && gh api -X POST "repos/$OWNER/$REPO/pages/builds" >/dev/null 2>&1
echo; echo "DONE. In 1-3 minutes: https://$OWNER.github.io/$REPO/emu/   (press Future updates)"
