#!/bin/bash
# github_publish.sh: put FM1 Tonic on GitHub (public) and keep it up to date. ONE command, safe to run again and again:
#     bash ~/fm1-x0x-main/github_publish.sh              (message = "FM1 Tonic 1.0")
#     bash ~/fm1-x0x-main/github_publish.sh "what changed"
# What it does, in order (it says each step on the screen):
#   1. makes the repository luigirossi-music/fm1-tonic on GitHub if it does not exist yet (public)
#   2. copies the project to ~/fm1-tonic-repo (the git folder: it lives OUTSIDE ~/fm1-x0x-main because tonic.sh /
#      tonic_site.sh wipe ~/fm1-x0x-main at every new zip), commits and pushes it (branch main).
#      The HANDOFFs, the wip zips and the manual files are NOT sent (.gitignore): they stay on your PC.
#   3. if the website is built (bash ~/tonic_site.sh makes build/pages), puts it on the gh-pages branch and switches
#      GitHub Pages on: https://luigirossi-music.github.io/fm1-tonic/
#   4. if the firmware file is built (build/fm1-tonic-<version>.fwsc), makes the GitHub Release v<version> with it.
# First time only:  sudo apt install -y git gh   and then   gh auth login   (the answers are in the handoff).
set -u
OWNER="luigirossi-music"
REPO="fm1-tonic"
SRC="${FM1_SRC:-$HOME/fm1-x0x-main}"
WORK="${FM1_WORK:-$HOME/fm1-tonic-repo}"
REMOTE="${FM1_REMOTE:-https://github.com/$OWNER/$REPO.git}"
SKIPGH="${FM1_SKIP_GH:-}"            # only for tests: no gh calls
die() { echo; echo "STOP: $*" >&2; exit 1; }
say() { echo; echo "== $*"; }

[ -d "$SRC" ] && [ -f "$SRC/VERSION" ] || die "no $SRC. Run: bash ~/tonic.sh  (it unpacks the newest zip there) and try again."
command -v git >/dev/null 2>&1 || die "git is missing. Run: sudo apt install -y git gh"
if [ -z "$SKIPGH" ]; then
    command -v gh >/dev/null 2>&1 || die "gh (the GitHub tool) is missing. Run: sudo apt install -y git gh"
    gh auth status >/dev/null 2>&1 || die "you are not logged in to GitHub. Run: gh auth login   (answers: GitHub.com, HTTPS, Yes, Login with a web browser)"
    gh auth setup-git >/dev/null 2>&1
fi
VER="$(tr -d ' \r\n' < "$SRC/VERSION")"
MSG="${1:-FM1 Tonic $VER}"

# 1. the repository
if [ -z "$SKIPGH" ]; then
    if gh repo view "$OWNER/$REPO" >/dev/null 2>&1; then
        say "1/4 The repository $OWNER/$REPO exists"
    else
        say "1/4 Making the public repository $OWNER/$REPO"
        gh repo create "$OWNER/$REPO" --public --description "FM1 Tonic: drum machine firmware for the M-VAVE FM-1 (version $VER)" \
            || die "could not create the repository. Make it by hand on github.com (Public, no README, no licence) and run this again."
    fi
fi

# 2. the source
say "2/4 Copying the project to $WORK and sending it"
mkdir -p "$WORK" || die "cannot make $WORK"
find "$WORK" -mindepth 1 -maxdepth 1 ! -name .git -exec rm -rf {} + || die "cannot clean $WORK"
tar -C "$SRC" --exclude=./build --exclude=./.git --exclude=__pycache__ --exclude=.DS_Store -cf - . | tar -C "$WORK" -xf - || die "copy failed"
cd "$WORK" || die "cannot enter $WORK"
[ -d .git ] || git init -q -b main || die "git init failed"
if git remote get-url origin >/dev/null 2>&1; then git remote set-url origin "$REMOTE"; else git remote add origin "$REMOTE"; fi
git config user.name >/dev/null 2>&1 || git config user.name "Luigi Rossi"
git config user.email >/dev/null 2>&1 || git config user.email "$OWNER@users.noreply.github.com"
# wip440: files that are now in .gitignore (FEATURES.md, the handoffs) must also LEAVE the repository if an earlier push had them:
# empty the index, then add again what .gitignore allows (the working copy is not touched)
git rm -r -q --cached . >/dev/null 2>&1
git add -A
BIG="$(git diff --cached --name-only | while read -r f; do [ -f "$f" ] && [ "$(stat -c %s "$f")" -gt 50000000 ] && echo "$f"; done)"
[ -z "$BIG" ] || { git reset -q; die "these files are over 50 MB (GitHub refuses 100 MB): $BIG   (send me this line)"; }
if git diff --cached --quiet; then
    echo "Nothing new to commit."
else
    git commit -q -m "$MSG" || die "git commit failed"
    echo "Committed: $MSG"
fi
git push -u origin HEAD:main || die "git push failed (read the lines above; if it says authentication, run: gh auth login)"

# 3. the website
if [ -d "$SRC/build/pages/install" ]; then
    say "3/4 Sending the website (gh-pages)"
    P="$(mktemp -d)"
    cp -a "$SRC/build/pages/." "$P/"
    touch "$P/.nojekyll"
    ( cd "$P" && git init -q -b gh-pages && git add -A &&
      git -c user.name="Luigi Rossi" -c user.email="$OWNER@users.noreply.github.com" commit -q -m "Site for FM1 Tonic $VER" &&
      git push -q -f "$REMOTE" gh-pages ) || { rm -rf "$P"; die "could not send the website"; }
    rm -rf "$P"
    if [ -z "$SKIPGH" ]; then
        gh api -X POST "repos/$OWNER/$REPO/pages" -f "source[branch]=gh-pages" -f "source[path]=/" >/dev/null 2>&1 \
            || echo "(Pages was already on, or switch it on by hand: the repository > Settings > Pages > Deploy from a branch > gh-pages / root)"
        gh api -X POST "repos/$OWNER/$REPO/pages/builds" >/dev/null 2>&1
    fi
else
    say "3/4 No website built yet: run  bash ~/tonic_site.sh  (Ctrl+C when it opens the page) and then this command again"
fi

# 4. the release with the firmware file
PKG="$SRC/build/fm1-tonic-$VER.fwsc"
if [ -f "$PKG" ]; then
    if [ -z "$SKIPGH" ]; then
        if gh release view "v$VER" -R "$OWNER/$REPO" >/dev/null 2>&1; then
            say "4/4 The release v$VER exists (to replace the file: gh release delete v$VER -R $OWNER/$REPO --yes  and run this again)"
        else
            say "4/4 Making the release v$VER with the firmware file"
            gh release create "v$VER" "$PKG" -R "$OWNER/$REPO" --title "FM1 Tonic $VER" \
                --notes "FM1 Tonic $VER for the M-VAVE FM-1. Install it with the web installer: https://$OWNER.github.io/$REPO/install/ (Chrome or Edge, the FM-1 plugged in by USB). Use it at your own risk: custom firmware, not made by M-VAVE." \
                || echo "(the release was not made: read the line above)"
        fi
    fi
else
    say "4/4 No firmware file ($PKG): run  bash ~/tonic_site.sh  first, then this again"
fi

echo
echo "DONE."
echo "  Source:   https://github.com/$OWNER/$REPO"
echo "  Web page: https://$OWNER.github.io/$REPO/   (the first time it needs 1-3 minutes)"
