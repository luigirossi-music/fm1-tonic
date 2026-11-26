#!/bin/bash
# whatsnew_site.sh: change ONLY the "What's new" pop-up, its button text and the manual of the live web page.
# The firmware, the emulator and everything else stay as they are (nothing is rebuilt).
#     bash ~/whatsnew_site.sh
# It takes the published site (branch gh-pages of luigirossi-music/fm1-tonic), puts the What's new of ~/fm1-x0x-main
# (the newest zip, unpacked by tonic.sh) into emu/index.html, copies the manual, and sends it back.
# The page updates in 1-3 minutes: https://luigirossi-music.github.io/fm1-tonic/emu/
# Needs git, gh and python3 (the same as github_publish.sh).
set -u
OWNER="luigirossi-music"; REPO="fm1-tonic"
REMOTE="${FM1_REMOTE:-https://github.com/$OWNER/$REPO.git}"
SKIPGH="${FM1_SKIP_GH:-}"
SRC="${FM1_SRC:-$HOME/fm1-x0x-main}"
die() { echo; echo "STOP: $*" >&2; exit 1; }
[ -f "$SRC/web/emu/index.html" ] || die "no $SRC. Run: bash ~/tonic.sh  (it unpacks the newest zip there) and try again."
TOOL="$SRC/tools/whatsnew_page.py"
[ -f "$TOOL" ] || die "no $TOOL (the zip is older than wip656)."
MAN="$SRC/web/emu/downloads/FM1-Tonic-manual.html"
[ -f "$MAN" ] || die "no manual file ($MAN)."
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
echo "== 2/3 Writing What's new + the manual into the page"
python3 "$TOOL" "$SRC/web/emu/index.html" "$W/site/emu/index.html" || die "the page could not be changed (read the line above)"
mkdir -p "$W/site/emu/downloads" && cp "$MAN" "$W/site/emu/downloads/FM1-Tonic-manual.html" || die "could not copy the manual"
cd "$W/site" || die "cannot enter the site folder"
if git diff --quiet && [ -z "$(git status --porcelain)" ]; then echo "Nothing changed: the page already shows this."; exit 0; fi
echo "== 3/3 Sending it"
git add -A
git -c user.name="Luigi Rossi" -c user.email="$OWNER@users.noreply.github.com" commit -q -m "What's new in 1.8h + manual" || die "git commit failed"
git push -q origin HEAD:gh-pages || die "git push failed (if it says authentication, run: gh auth login)"
[ -z "$SKIPGH" ] && gh api -X POST "repos/$OWNER/$REPO/pages/builds" >/dev/null 2>&1
echo; echo "DONE. In 1-3 minutes: https://$OWNER.github.io/$REPO/emu/   (press What's new in 1.8h)"
