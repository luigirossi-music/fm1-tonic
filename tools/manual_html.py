#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""docs/MANUAL.md -> docs/manual.html. The Markdown is the source; run this after editing it.

  tools/manual_html.py [IN.md [OUT.html]] [--fragment]

--multi OUT.html: ONE page with every language that has a source (MANUAL.md = English, MANUAL.<code>.md
for the others, see LANGS) and a language bar at the top; images inside (use with --embed).
--fragment writes the page without its <!doctype>/<html>/<head>/<body> wrapper (for hosts that
add their own); the default is a complete document that opens in any browser.

Handles what the manual uses: headings, paragraphs, **bold**, `code`, lists, tables, fenced
code, rules, and the contents list (linked to the sections by number). An image is
![what it shows](src "caption"): the alt text says what the picture shows (a screen: its text), the
caption what it is for; without a caption, the alt text is both."""
import base64
import datetime
import html
import re
import sys
from pathlib import Path

SRC = Path(__file__).resolve().parents[1]
PART_COLOURS = {"909": "#e8860f", "808": "#e0483e", "303A": "#4fae3a", "303B": "#2e9bd6", "BREAK": "#a066e0"}

CSS = """
:root {
  --ground: #f4f5f6; --sheet: #ffffff; --ink: #1c1f23; --muted: #5b626a; --rule: #d9dde1;
  --accent: #3d6f9e; --code-bg: #eef0f2; --swatch-ring: rgba(0,0,0,.18);
  color-scheme: light;
}
@media (prefers-color-scheme: dark) {
  :root:not([data-theme="light"]) {
    --ground: #131518; --sheet: #1a1d21; --ink: #e6e8eb; --muted: #9aa1a9; --rule: #2d3137;
    --accent: #8fb8e0; --code-bg: #24282d; --swatch-ring: rgba(255,255,255,.22);
    color-scheme: dark;
  }
}
:root[data-theme="dark"] {
  --ground: #131518; --sheet: #1a1d21; --ink: #e6e8eb; --muted: #9aa1a9; --rule: #2d3137;
  --accent: #8fb8e0; --code-bg: #24282d; --swatch-ring: rgba(255,255,255,.22);
  color-scheme: dark;
}
body {
  background: var(--ground); color: var(--ink); margin: 0;
  font: 400 17px/1.6 "Barlow", "Helvetica Neue", Arial, sans-serif;
  padding-inline: 16px; padding-block: 32px 64px;
}
main { max-width: 46rem; margin: 0 auto; }
h1, h2, h3 { font-family: "Barlow Semi Condensed", "Barlow", "Arial Narrow", sans-serif; text-wrap: balance; line-height: 1.15; }
h1 { font-size: 2.3rem; font-weight: 700; margin: 0 0 .3rem; letter-spacing: -.01em; }
h2 { font-size: 1.55rem; font-weight: 600; margin: 3rem 0 .8rem; padding-top: 1rem; border-top: 1px solid var(--rule); }
h2 .num { color: var(--accent); margin-right: .35em; font-variant-numeric: tabular-nums; }
h3 { font-size: 1.2rem; font-weight: 600; margin: 2rem 0 .5rem; }
h4 { font-family: "Barlow Semi Condensed", "Barlow", "Arial Narrow", sans-serif; font-size: 1.05rem; font-weight: 600; margin: 1.6rem 0 .4rem; color: var(--accent); }
p, li { max-width: 65ch; }
p { margin: 0 0 1rem; }
ul, ol { padding-left: 1.3rem; margin: 0 0 1rem; }
li { margin: .2rem 0; }
strong { font-weight: 600; }
.r { color: #c0281c; font-weight: 600; } .o { color: #a85a00; font-weight: 600; }
@media (prefers-color-scheme: dark) { :root:not([data-theme="light"]) .r { color: #ff7a70; } :root:not([data-theme="light"]) .o { color: #f0a043; } }
:root[data-theme="dark"] .r { color: #ff7a70; } :root[data-theme="dark"] .o { color: #f0a043; }
a { color: var(--accent); text-underline-offset: 2px; }
a:focus-visible { outline: 2px solid var(--accent); outline-offset: 2px; border-radius: 2px; }
code, pre { font-family: "IBM Plex Mono", ui-monospace, Menlo, Consolas, monospace; font-size: .86em; }
code { background: var(--code-bg); padding: .08em .35em; border-radius: 4px; }
pre { background: var(--code-bg); padding: 14px 16px; border-radius: 8px; overflow-x: auto; line-height: 1.45; margin: 0 0 1.2rem; }
pre code { background: none; padding: 0; font-size: inherit; }
hr { display: none; }
.table { overflow-x: auto; margin: 0 0 1.3rem; }
table { border-collapse: collapse; width: 100%; font-size: .95rem; font-variant-numeric: tabular-nums; }
th, td { text-align: left; vertical-align: top; padding: .45rem .7rem .45rem 0; border-bottom: 1px solid var(--rule); }
th { font-family: "Barlow Semi Condensed", "Barlow", sans-serif; font-weight: 600; text-transform: uppercase; letter-spacing: .04em; font-size: .82rem; color: var(--muted); }
td:first-child { white-space: nowrap; }
.lede { color: var(--muted); margin-bottom: 1.6rem; }
.status { background: var(--sheet); border: 1px solid var(--rule); border-radius: 8px; padding: 12px 16px; }
.status p { margin: 0; }
a.home { color: inherit; text-decoration: none; }
main > a.home { display: inline-block; font: 700 1rem "Barlow Semi Condensed", "Barlow", sans-serif; letter-spacing: .03em; color: var(--muted); margin: 0 0 .4rem; }
a.home:hover { color: var(--accent); }
pre:focus-visible, .table:focus-visible { outline: 2px solid var(--accent); outline-offset: 2px; }
summary:focus-visible { outline: 2px solid var(--accent); outline-offset: 2px; border-radius: 2px; }
.skip { position: absolute; left: 16px; top: -100px; background: var(--sheet); color: var(--accent); padding: 8px 14px;
        border: 2px solid var(--accent); border-radius: 8px; font-weight: 600; z-index: 10; }
.skip:focus { top: 12px; }
.callout { border-left: 3px solid var(--accent); background: var(--sheet); border-radius: 0 8px 8px 0; padding: 10px 16px; margin: 0 0 1.2rem; max-width: 65ch; }
.callout p { margin: 0; }
details { border: 1px solid var(--rule); border-radius: 8px; padding: 0 16px; margin: 0 0 1.2rem; }
details > summary { cursor: pointer; padding: 10px 0; font-weight: 600; color: var(--muted); }
details[open] > summary { margin-bottom: .4rem; }
details > :last-child { margin-bottom: 1rem; }
.swatch { display: inline-block; width: .7em; height: .7em; border-radius: 3px; margin-right: .4em; vertical-align: .02em; box-shadow: 0 0 0 1px var(--swatch-ring); }
nav.contents ol { columns: 2 14rem; column-gap: 2rem; padding-left: 1.3rem; }
.figs { display: flex; flex-wrap: wrap; gap: 20px; margin: .4rem 0 1.6rem; }
figure { margin: 0; flex: 0 1 auto; }
figure img { display: block; max-width: 100%; height: auto; border-radius: 10px; }
figure.shot img { width: 300px; image-rendering: pixelated; box-shadow: 0 0 0 1px var(--rule); }
figure.wide { flex-basis: 100%; }
figure.wide img { width: 100%; }
figcaption { color: var(--muted); font-size: .88rem; margin-top: .4rem; max-width: 300px; }
figure.wide figcaption { max-width: none; }
nav.contents li { break-inside: avoid; }
@media (prefers-reduced-motion: no-preference) { html { scroll-behavior: smooth; } }
/* the top bar (multi-language page): contents button at the left, search, languages; stays in view */
html { scroll-padding-top: calc(var(--barh, 72px) + 10px); }
.topbar { position: sticky; top: 0; z-index: 30; display: flex; flex-wrap: wrap; gap: .5rem .8rem; align-items: center;
          background: var(--sheet); border-bottom: 2px solid var(--accent); padding: .5rem 16px; margin: -32px -16px 28px; }
.topbar button, .langbar a { font: 700 .95rem "Barlow Semi Condensed", "Barlow", sans-serif; letter-spacing: .02em; color: var(--ink); background: var(--ground);
             border: 2px solid var(--rule); border-radius: 999px; padding: .35rem 1rem; min-height: 44px; box-sizing: border-box; display: inline-flex; align-items: center; gap: .4rem; cursor: pointer; text-decoration: none; }
.topbar button:hover, .langbar a:hover { border-color: var(--accent); }
.topbar button[aria-expanded="true"] { background: var(--accent); border-color: var(--accent); color: var(--sheet); }
.topbar button:disabled { opacity: .45; cursor: default; }
.search { display: flex; align-items: center; gap: .35rem; flex: 1 1 14rem; min-width: 0; max-width: 26rem; }
.search input { flex: 1 1 6rem; min-width: 0; min-height: 44px; box-sizing: border-box; font: 400 1rem "Barlow", Arial, sans-serif; color: var(--ink); background: var(--ground);
                border: 2px solid var(--rule); border-radius: 999px; padding: .3rem 1rem; }
.search input:focus-visible { outline: 2px solid var(--accent); outline-offset: 2px; border-color: var(--accent); }
.search .count { font: 600 .85rem "Barlow Semi Condensed", "Barlow", sans-serif; color: var(--muted); white-space: nowrap; min-width: 3.2rem; }
.search button { padding: .35rem .7rem; }
.langbar { display: flex; flex-wrap: wrap; gap: .4rem; align-items: center; }
.topbar .ver { margin-left: auto; font: 700 .85rem "Barlow Semi Condensed", "Barlow", sans-serif; letter-spacing: .03em; color: var(--accent); border: 2px solid var(--accent); border-radius: 999px; padding: .2rem .7rem; white-space: nowrap; }
.docver { margin: .2rem 0 1rem; color: var(--muted); font-size: .92rem; }
.langbar a[aria-current="true"] { background: var(--accent); border-color: var(--accent); color: var(--sheet); }
.topbar button:focus-visible, .langbar a:focus-visible { outline: 2px solid var(--accent); outline-offset: 2px; }
.tocpanel { position: fixed; z-index: 25; left: 0; top: var(--barh, 72px); bottom: 0; width: min(24rem, 92vw); overflow-y: auto; background: var(--sheet);
            border-right: 2px solid var(--accent); box-shadow: 4px 0 16px rgba(0,0,0,.18); padding: 1rem 1.1rem 3rem; }
.tocpanel[hidden] { display: none; }
.tocpanel ol { columns: 1; padding-left: 1.2rem; margin: 0; }
.tocpanel li { margin: .35rem 0; }
.tocpanel h3 { margin: 0 0 .6rem; font-size: 1.1rem; }
.langbar .short { display: none; }
@media (max-width: 640px) {
  .topbar { gap: .4rem; padding: .4rem 12px; margin-inline: -16px; }
  .topbar button, .langbar a { min-height: 40px; padding: .25rem .8rem; }
  #tocbtn .lbl { display: none; }
  .langbar { gap: .25rem; }
  .langbar a { padding: .2rem .45rem; font-size: .85rem; border-width: 1px; }
  #tocbtn { padding: .2rem .7rem; }
  .langbar .full { display: none; } .langbar .short { display: inline; }
  .search { flex-basis: 100%; max-width: none; }
  .search input { min-height: 40px; }
}
/* wip424: on a wide screen the contents is a permanent column at the left (never closes); the drawer stays on narrow screens */
.tocpanel a { text-decoration: none; display: block; padding: .12rem .45rem; border-radius: 6px; }
.tocpanel a:hover { text-decoration: underline; }
.tocpanel a.cur { color: var(--accent); font-weight: 600; background: var(--ground); box-shadow: inset 3px 0 0 var(--accent); }
@media (min-width: 1000px) {
  :root { --side: 20rem; }
  body { padding-left: calc(var(--side) + 16px); }
  .topbar { margin-left: calc(-1 * (var(--side) + 16px)); padding-left: 16px; }
  #tocbtn { display: none; }
  .tocpanel { width: var(--side); box-shadow: none; font-size: .95rem; line-height: 1.35; }
  #tocbody { padding: 1rem .8rem .8rem; }
  .tocpanel[hidden] { display: flex; }
  .tocpanel ol { padding-left: 1.1rem; }
}
mark.hit { background: #cfe3f7; color: #1c1f23; border-radius: 3px; padding: 0 .08em; }
mark.hit.cur { background: #8fb8e0; outline: 2px solid #3d6f9e; }
/* wip428: no second contents list in the middle of the page on screen (the left column has it); print / PDF keep it. The search sits under the contents in the left column */
@media screen { .lang nav.contents, .lang nav.contents + hr { display: none; } }
.tocpanel { display: flex; flex-direction: column; padding: 0; }
.tocpanel[hidden] { display: none; }
#tocbody { flex: 1 1 auto; overflow-y: auto; padding: 1rem 1.1rem .8rem; min-height: 0; }
#tocsearch { flex: 0 0 auto; border-top: 2px solid var(--accent); padding: .6rem .8rem; background: var(--sheet); }
#tocsearch:empty { display: none; }
#tocsearch .search { max-width: none; flex-wrap: nowrap; gap: .25rem; }
#tocsearch .search input { flex: 1 1 auto; min-width: 0; padding: .3rem .8rem; }
#tocsearch .search .count { min-width: 2.4rem; font-size: .8rem; text-align: center; }
#tocsearch .search button { padding: .2rem .5rem; min-width: 36px; min-height: 40px; }
@media print { .topbar, .tocpanel { display: none; } mark.hit { background: none; } }
"""


# a table wider than the page scrolls: then the keyboard can reach it (and it has a name)
SCRIPT = """<script>
(() => {
  const TABLE = { en: "Table", es: "Tabla", it: "Tabella", fr: "Tableau", pt: "Tabela", de: "Tabelle" };
  const tables = [...document.querySelectorAll(".table")];
  tables.forEach((t) => {
    const th = t.querySelector("th"), l = t.closest(".lang");
    t.setAttribute("aria-label", (TABLE[l ? l.lang : "en"] || "Table") + (th ? ": " + th.textContent : ""));
  });
  const fit = () => tables.forEach((t) => {
    if (t.scrollWidth > t.clientWidth + 1) { t.tabIndex = 0; t.setAttribute("role", "region"); }
    else { t.removeAttribute("tabindex"); t.removeAttribute("role"); }
  });
  fit();
  addEventListener("resize", fit);
  // several languages in one page: show one, remember it, follow links into another one
  const langs = [...document.querySelectorAll(".lang")], links = [...document.querySelectorAll(".langbar a")];
  if (langs.length > 1) {
    const hooks = [];
    const show = (code) => {
      langs.forEach((l) => { l.hidden = l.lang !== code; });
      links.forEach((a) => a.setAttribute("aria-current", String(a.dataset.lang === code)));
      document.documentElement.lang = code;
      try { localStorage.setItem("fm1-manual-lang", code); } catch (e) { /* private window */ }
      fit();
      hooks.forEach((h) => h(code));
    };
    const have = (c) => langs.some((l) => l.lang === c);
    const inLang = (id) => { const e = id && document.getElementById(id); const l = e && e.closest(".lang"); return l ? l.lang : null; };
    const fromHash = () => decodeURIComponent(location.hash.slice(1));
    links.forEach((a) => a.addEventListener("click", (e) => {
      e.preventDefault(); show(a.dataset.lang); history.replaceState(null, "", "#lang-" + a.dataset.lang); scrollTo(0, 0);
    }));
    addEventListener("hashchange", () => {
      const id = fromHash(), c = inLang(id);
      if (c) { show(c); document.getElementById(id).scrollIntoView(); }
    });
    // top bar: contents drawer (the contents of the language shown) and search (inside the language shown)
    const bar = document.querySelector(".topbar");
    if (bar) {
      const L = { en: ["Contents", "Search"], es: ["Contenido", "Buscar"], it: ["Indice", "Cerca"], fr: ["Sommaire", "Rechercher"], pt: ["Índice", "Pesquisar"], de: ["Inhalt", "Suchen"] };
      const panel = document.getElementById("tocpanel"), tbody = document.getElementById("tocbody"), tsearch = document.getElementById("tocsearch"), tocBtn = document.getElementById("tocbtn");
      const q = document.getElementById("q"), cnt = document.getElementById("qcount"), prev = document.getElementById("qprev"), next = document.getElementById("qnext");
      const size = () => document.documentElement.style.setProperty("--barh", bar.offsetHeight + "px");
      size(); addEventListener("resize", size);
      const wide = matchMedia("(min-width: 1000px)");   // wide: the contents is a permanent column, it never closes
      const searchBox = document.querySelector(".search");
      const place = () => { if (wide.matches) tsearch.append(searchBox); else bar.insertBefore(searchBox, null); };   // wide: the search is under the contents; narrow: in the top bar
      place();
      const closeToc = () => { if (wide.matches) return; panel.hidden = true; tocBtn.setAttribute("aria-expanded", "false"); };
      panel.hidden = !wide.matches;
      wide.addEventListener("change", () => { place(); panel.hidden = !wide.matches; tocBtn.setAttribute("aria-expanded", "false"); size(); });
      // the chapter being read is highlighted in the contents
      let tmap = [], curA = null;
      const cur = () => {
        if (!tmap.length) return;
        const lim = bar.offsetHeight + 30; let best = tmap[0][0];
        for (const [a, el] of tmap) { if (el.getBoundingClientRect().top <= lim) best = a; }
        if (best === curA) return;
        if (curA) { curA.classList.remove("cur"); curA.removeAttribute("aria-current"); }
        best.classList.add("cur"); best.setAttribute("aria-current", "location"); curA = best;
        if (!panel.hidden) {
          const pr = tbody.getBoundingClientRect(), ar = best.getBoundingClientRect();
          if (ar.top < pr.top + 8 || ar.bottom > pr.bottom - 8) tbody.scrollTop += ar.top - pr.top - pr.height / 3;
        }
      };
      const track = () => {
        curA = null;
        tmap = [...panel.querySelectorAll("a[href^='#']")].map((a) => [a, document.getElementById(decodeURIComponent(a.getAttribute("href").slice(1)))]).filter((x) => x[1]);
        cur();
      };
      let tick = 0;
      addEventListener("scroll", () => { if (!tick) tick = requestAnimationFrame(() => { tick = 0; cur(); }); }, { passive: true });
      tocBtn.addEventListener("click", () => { const open = panel.hidden; panel.hidden = !open; tocBtn.setAttribute("aria-expanded", String(open)); size(); cur(); });
      panel.addEventListener("click", (e) => {
        const a = e.target.closest("a"); if (!a) return;
        const el = document.getElementById(decodeURIComponent(a.getAttribute("href").slice(1)));
        if (el) { e.preventDefault(); history.replaceState(null, "", a.getAttribute("href")); el.scrollIntoView({ behavior: "instant" }); cur(); }   // a jump, not a long smooth scroll that stops short
        closeToc();
      });
      document.addEventListener("click", (e) => { if (!panel.hidden && !panel.contains(e.target) && !tocBtn.contains(e.target)) closeToc(); });
      const norm = (str) => {                       // lower case, no accents; map = where each character came from
        let out = "", map = [];
        for (let i = 0; i < str.length; i++) {
          for (const ch of str[i].normalize("NFD").replace(/[\u0300-\u036f]/g, "").toLowerCase()) { out += ch; map.push(i); }
        }
        map.push(str.length);
        return [out, map];
      };
      let hits = [], at = -1;
      const clear = () => {
        document.querySelectorAll("mark.hit").forEach((m) => { const p = m.parentNode; p.replaceChild(document.createTextNode(m.textContent), m); p.normalize(); });
        hits = []; at = -1;
      };
      const go = (k) => {
        if (!hits.length) return;
        if (at >= 0) hits[at].classList.remove("cur");
        at = (k + hits.length) % hits.length;
        const m = hits[at];
        m.classList.add("cur");
        for (let d = m.closest("details"); d; d = d.parentElement && d.parentElement.closest("details")) d.open = true;
        m.scrollIntoView({ block: "center" });
        cnt.textContent = (at + 1) + "/" + hits.length;
      };
      const run = (jump) => {
        clear();
        cnt.textContent = ""; prev.disabled = next.disabled = true;
        const term = norm(q.value)[0].trim();
        if (term.length < 2) return;
        const root = langs.find((l) => !l.hidden);
        if (!root) return;
        const w = document.createTreeWalker(root, NodeFilter.SHOW_TEXT, { acceptNode: (n) => (n.parentElement.closest("script,style,nav.contents") ? NodeFilter.FILTER_REJECT : NodeFilter.FILTER_ACCEPT) });
        const nodes = [];
        while (w.nextNode()) nodes.push(w.currentNode);
        let n = 0;
        for (const node of nodes) {
          if (n >= 3000) break;
          const [t, map] = norm(node.nodeValue);
          let i = t.indexOf(term);
          if (i < 0) continue;
          const spans = [];
          while (i >= 0 && n < 3000) { spans.push([map[i], map[i + term.length]]); n++; i = t.indexOf(term, i + term.length); }
          for (let k = spans.length - 1; k >= 0; k--) {     // from the end, so the offsets stay true
            const r = document.createRange(), m = document.createElement("mark");
            r.setStart(node, spans[k][0]); r.setEnd(node, spans[k][1]);
            m.className = "hit"; r.surroundContents(m);
          }
        }
        hits = [...root.querySelectorAll("mark.hit")];
        cnt.textContent = hits.length ? "" : "0";
        prev.disabled = next.disabled = !hits.length;
        if (jump) go(0); else if (hits.length) cnt.textContent = hits.length + "";
      };
      let timer = 0;
      q.addEventListener("input", () => { clearTimeout(timer); timer = setTimeout(() => run(true), 180); });
      q.addEventListener("keydown", (e) => {
        if (e.key === "Enter") { e.preventDefault(); if (!hits.length) run(true); else go(at + (e.shiftKey ? -1 : 1)); }
        else if (e.key === "Escape") { q.value = ""; run(false); }
      });
      next.addEventListener("click", () => go(at + 1));
      prev.addEventListener("click", () => go(at - 1));
      addEventListener("keydown", (e) => {
        if (e.key === "/" && !/^(INPUT|TEXTAREA|SELECT)$/.test(document.activeElement.tagName)) { e.preventDefault(); q.focus(); q.select(); }
        else if (e.key === "Escape" && !panel.hidden && !wide.matches) { closeToc(); tocBtn.focus(); }
      });
      hooks.push((code) => {
        const t = L[code] || L.en, l = langs.find((x) => x.lang === code), nav = l && l.querySelector("nav.contents ol");
        tocBtn.querySelector(".lbl").textContent = t[0]; tocBtn.setAttribute("aria-label", t[0]); panel.setAttribute("aria-label", t[0]);
        q.placeholder = t[1]; q.setAttribute("aria-label", t[1]);
        tbody.textContent = "";
        const h = document.createElement("h3"); h.textContent = t[0]; tbody.append(h);
        if (nav) { const c = nav.cloneNode(true); c.querySelectorAll("[id]").forEach((e) => e.removeAttribute("id")); tbody.append(c); }
        closeToc(); size(); track();
        if (q.value) run(false);
      });
    }
    let start = inLang(fromHash());
    if (!start) { try { start = localStorage.getItem("fm1-manual-lang"); } catch (e) { start = null; } }
    if (!start || !have(start)) start = (navigator.languages || [navigator.language || "en"]).map((x) => x.slice(0, 2).toLowerCase()).find(have) || "en";
    show(start);
    if (location.hash && document.getElementById(fromHash())) document.getElementById(fromHash()).scrollIntoView();
  }
})();
</script>
"""


def inline(t):
    t = html.escape(t, quote=False)
    t = re.sub(r"`([^`]+)`", r"<code>\1</code>", t)
    t = re.sub(r"\*\*([^*]+)\*\*", r"<strong>\1</strong>", t)
    t = re.sub(r"\{r:([^}]+)\}", r'<span class="r">\1</span>', t)   # red = with FUNC held
    t = re.sub(r"\{o:([^}]+)\}", r'<span class="o">\1</span>', t)   # orange = with FILL held
    t = re.sub(r"\[([^\]]+)\]\(([^)\s]+)\)", lambda m: f'<a href="{m.group(2)}">{m.group(1)}</a>', t)
    for name, col in PART_COLOURS.items():          # a part named at the start of a table cell gets its swatch
        t = re.sub(rf"^<strong>{name}</strong>", f'<span class="swatch" style="background:{col}"></span><strong>{name}</strong>', t)
    return t


def slug(title):
    m = re.match(r"(\d+)\. ", title)                 # "8. Break", not "1.0.9 · 15 October" (an id each)
    return f"s{m.group(1)}" if m else re.sub(r"[^a-z0-9]+", "-", title.lower()).strip("-")


IMG_BASE = SRC / "docs"
EMBED = False                                         # --fragment: images as data URIs


def figure(alt, src, caption=None):
    path = IMG_BASE / src
    if EMBED:
        mime = "image/svg+xml" if src.endswith(".svg") else "image/png"
        url = f"data:{mime};base64," + base64.b64encode(path.read_bytes()).decode()
    else:
        url = src
    cls = "wide" if src.endswith(".svg") else "shot"
    return (f'<figure class="{cls}"><img src="{url}" alt="{html.escape(alt)}" loading="lazy">'
            + ("" if caption == "" else f"<figcaption>{inline(caption or alt)}</figcaption>") + "</figure>")


def convert(md):
    lines = md.splitlines()
    out, i, contents, sections = [], 0, None, {}
    for ln in lines:                                  # section numbers -> ids, for the contents
        m = re.match(r"## (\d+)\. (.+)", ln)
        if m:
            sections[m.group(2).strip()] = f"s{m.group(1)}"
    while i < len(lines):
        ln = lines[i]
        if ln.startswith("```"):
            j = i + 1
            while j < len(lines) and not lines[j].startswith("```"):
                j += 1
            label = "Command" if j - i == 2 else "Commands"     # a pre scrolls: the keyboard can reach it
            out.append(f'<pre tabindex="0" aria-label="{label}"><code>' + html.escape("\n".join(lines[i + 1:j]), quote=False) +
                       "</code></pre>")
            i = j + 1
            continue
        if ln.startswith("!["):                      # consecutive image lines: one row of figures
            figs = []
            while i < len(lines) and (lines[i].startswith("![") or (not lines[i].strip() and i + 1 < len(lines)
                                                                    and lines[i + 1].startswith("!["))):
                m = re.match(r'!\[(.*)\]\((\S+)(?: "(.*)")?\)', lines[i])
                if m:
                    figs.append(figure(m.group(1), m.group(2), m.group(3)))
                i += 1
            out.append('<div class="figs">' + "".join(figs) + "</div>")
            continue
        if ln.strip().startswith("<details>"):          # a toggle: <details><summary>…</summary> … </details>
            m = re.match(r"\s*<details>\s*<summary>(.*)</summary>\s*$", ln)
            out.append(f"<details><summary>{inline(m.group(1)) if m else ''}</summary>")
            i += 1
            continue
        if ln.strip() == "</details>":
            out.append("</details>")
            i += 1
            continue
        if ln.startswith("> "):                           # a callout: the important bit
            para = []
            while i < len(lines) and lines[i].startswith(">"):
                para.append(lines[i][1:].strip())
                i += 1
            out.append(f'<div class="callout"><p>{inline(" ".join(para))}</p></div>')
            continue
        if ln.startswith("# "):                         # the title; X0X's own a way home (the site's root)
            t = ln[2:].strip()
            if t == "X0X":
                out.append('<h1><a class="home" href="../">X0X</a></h1>')
            else:
                out.append(f'<a class="home" href="../">FM1 Tonic</a>\n<h1>{inline(t)}</h1>')
        elif ln.startswith("## "):
            t = ln[3:].strip()
            m = re.match(r"(\d+)\. (.+)", t)
            if t == "Contents":
                contents = True
                out.append('<nav class="contents" aria-label="Contents"><h2 id="contents">Contents</h2>')
            elif m:
                out.append(f'<h2 id="s{m.group(1)}"><span class="num">{m.group(1)}</span>{inline(m.group(2))}</h2>')
            else:
                out.append(f'<h2 id="{slug(t)}">{inline(t)}</h2>')
        elif ln.startswith("#### "):
            out.append(f'<h4 id="{slug(ln[5:])}">{inline(ln[5:])}</h4>')
        elif ln.strip() in ("### Contents", "### Contenido", "### Indice", "### Sommaire", "### Índice", "### Inhalt"):              # a contents list inside a section (not its own h2)
            contents = True
            out.append(f'<nav class="contents" aria-label="{ln.strip()[4:]}"><h3 id="contents">{ln.strip()[4:]}</h3>')
        elif ln.startswith("### "):
            out.append(f'<h3 id="{slug(ln[4:])}">{inline(ln[4:])}</h3>')
        elif ln.strip() == "---":
            if contents:
                out.append("</nav>")
                contents = None
            out.append("<hr>")
        elif ln.startswith("|"):
            rows = []
            while i < len(lines) and lines[i].startswith("|"):
                rows.append([c.strip() for c in lines[i].strip().strip("|").split("|")])
                i += 1
            head, body = rows[0], [r for r in rows[1:] if not set("".join(r)) <= set("-: ")]
            out.append('<div class="table"><table><thead><tr>' + "".join(f"<th>{inline(c)}</th>" for c in head) +
                       "</tr></thead><tbody>" + "".join("<tr>" + "".join(f"<td>{inline(c)}</td>" for c in r) + "</tr>"
                                                         for r in body) + "</tbody></table></div>")
            continue
        elif re.match(r"(\d+\.|-) ", ln):
            ordered = ln[0].isdigit()
            items, subs = [], {}
            while i < len(lines) and (re.match(r"(\d+\.|-) ", lines[i]) or (lines[i].startswith("  ") and items)):
                if contents and lines[i].startswith("  - "):          # contents: a sub-entry under the last item
                    subs.setdefault(len(items) - 1, []).append(lines[i].strip()[2:])
                elif lines[i].startswith("  "):
                    items[-1] += " " + lines[i].strip()
                else:
                    items.append(re.sub(r"^(\d+\.|-) ", "", lines[i]))
                i += 1
            if contents:
                def link(t):                                  # "Title {#id}": an explicit target (a part not translated yet)
                    m = re.search(r"\s*\{#([\w-]+)\}$", t)
                    return (f'<a href="#{m.group(1)}">{inline(t[:m.start()])}</a>' if m else
                            f'<a href="#{sections.get(t.strip(), slug(t))}">{inline(t)}</a>')
                items = [link(t) + ("<ul>" + "".join(f"<li>{link(s)}</li>" for s in subs[n]) + "</ul>" if n in subs else "")
                         for n, t in enumerate(items)]
            else:
                items = [inline(t) for t in items]
            tag = "ol" if ordered else "ul"
            out.append(f"<{tag}>" + "".join(f"<li>{t}</li>" for t in items) + f"</{tag}>")
            continue
        elif ln.strip():
            para = [ln]
            while i + 1 < len(lines) and lines[i + 1].strip() and not re.match(r"(#|\||```|---|\d+\. |- |!\[|> |\s*</?details)", lines[i + 1]):
                i += 1
                para.append(lines[i])
            text = inline(" ".join(p.strip() for p in para))
            if text.startswith("<strong>Status.</strong>"):
                out.append(f'<div class="status"><p>{text}</p></div>')
            elif out and out[-1].startswith("<h1>"):
                out.append(f'<p class="lede">{text}</p>')
            else:
                out.append(f"<p>{text}</p>")
        i += 1
    return "\n".join(out)


LANGS = [("en", "English", "MANUAL.md"), ("es", "Español", "MANUAL.es.md"), ("it", "Italiano", "MANUAL.it.md"),
         ("fr", "Français", "MANUAL.fr.md"), ("pt", "Português", "MANUAL.pt.md"), ("de", "Deutsch", "MANUAL.de.md")]


VERSION_LABEL = {"en": "Manual version", "es": "Versión del manual", "it": "Versione del manuale", "fr": "Version du manuel",
                 "pt": "Versão do manual", "de": "Handbuch-Version"}
FIRMWARE_LABEL = {"en": "firmware", "es": "firmware", "it": "firmware", "fr": "firmware", "pt": "firmware", "de": "Firmware"}


def manual_version():
    """wip438: the manual's own number (docs/MANUAL_VERSION, raised at every manual change: 1.44, 1.45 ...), the firmware version
    (VERSION) and the build date, so a person who downloads it again can see whether it is newer"""
    mv = (SRC / "docs" / "MANUAL_VERSION").read_text().strip()
    fw = (SRC / "VERSION").read_text().strip()
    return mv, fw, datetime.date.today().strftime("%-d %b %Y")


def build_multi(dst):
    """one page, every language that has a source file; ids of the other languages get a code- prefix (a link to a part
    not translated yet keeps pointing at the English one)"""
    have, parts = [], []
    mver, fwver, mdate = manual_version()
    for code, name, fn in LANGS:
        f = SRC / "docs" / fn
        if not f.exists():
            continue
        body = convert(f.read_text())
        if code == "en":
            en_body = body
        else:
            # wip401: a chapter not translated yet is shown in ENGLISH inside this language (no jump to the English part)
            cuts = [(int(mm.group(1)), mm.start()) for mm in re.finditer(r'<h2 id="s(\d+)">', en_body)] + [(99, len(en_body))]
            for (k, a), (_, z) in zip(cuts, cuts[1:]):
                if k >= 2 and f'id="s{k}"' not in body:
                    body += "\n" + en_body[a:z]
            ids = set(re.findall(r'id="([^"]+)"', body))
            body = re.sub(r'id="([^"]+)"', lambda m: f'id="{code}-{m.group(1)}"', body)
            body = re.sub(r'href="#([^"]+)"', lambda m: f'href="#{code}-{m.group(1)}"' if m.group(1) in ids else m.group(0), body)
        verline = (f'<p class="docver"><b>{VERSION_LABEL.get(code, "Manual version")} {mver}</b> · FM1 Tonic {FIRMWARE_LABEL.get(code, "firmware")} {fwver} · {mdate}</p>')
        body = body.replace("</h1>", "</h1>\n" + verline, 1)
        have.append((code, name))
        parts.append(f'<div class="lang" id="lang-{code}" lang="{code}">\n{body}\n</div>')
    bar = ('<header class="topbar"><button type="button" id="tocbtn" aria-expanded="false" aria-controls="tocpanel"><span aria-hidden="true">☰</span><span class="lbl">Contents</span></button>'
           '<nav class="langbar" aria-label="Language">' +
           "".join(f'<a href="#lang-{c}" data-lang="{c}" lang="{c}" hreflang="{c}"><span class="full">{n}</span><span class="short" aria-hidden="true">{c.upper()}</span></a>' for c, n in have) + "</nav>"
           '<div class="search" role="search"><input type="search" id="q" placeholder="Search" aria-label="Search" autocomplete="off" enterkeyhint="search">'
           '<span class="count" id="qcount" aria-live="polite"></span>'
           '<button type="button" id="qprev" aria-label="Previous result" disabled>↑</button><button type="button" id="qnext" aria-label="Next result" disabled>↓</button></div>'
           f'<span class="ver" title="Manual version {mver}">v{mver}</span>'
           '</header>\n<aside class="tocpanel" id="tocpanel" hidden aria-label="Contents"><div id="tocbody"></div><div id="tocsearch"></div></aside>')
    page = ('<!doctype html>\n<html lang="en">\n<head>\n<meta charset="utf-8">\n'
            '<meta name="viewport" content="width=device-width, initial-scale=1">\n'
            f'<title>FM1 Tonic manual {mver}</title>\n'
            '<meta name="description" content="The manual for FM1 Tonic, drum machine firmware for the M-VAVE FM-1. English, Español.">\n'
            '<link rel="preconnect" href="https://fonts.googleapis.com">\n<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>\n'
            '<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Barlow:wght@400;600&'
            'family=Barlow+Semi+Condensed:wght@600;700&family=IBM+Plex+Mono:wght@400&display=swap">\n'
            f"<style>{CSS}</style>\n</head>\n<body>\n{bar}\n"
            '<a class="skip" href="#content">Skip to content</a>\n<main id="content">\n' + "\n".join(parts) +
            f"\n</main>\n{SCRIPT}</body>\n</html>\n")
    Path(dst).write_text(page)
    print(f"{dst}: {len(page)} bytes, languages: {', '.join(c for c, _ in have)}")


def main():
    args = [a for a in sys.argv[1:] if a not in ("--fragment", "--embed", "--multi")]
    global EMBED
    if "--multi" in sys.argv:
        EMBED = True
        return build_multi(args[0] if args else SRC / "docs" / "manual-multi.html")
    fragment = "--fragment" in sys.argv
    EMBED = fragment or "--embed" in sys.argv       # --embed: a complete page with the images inside
    src = Path(args[0]) if args else SRC / "docs" / "MANUAL.md"
    dst = Path(args[1]) if len(args) > 1 else SRC / "docs" / "manual.html"
    md = src.read_text()
    h1 = re.search(r"^# (.+)$", md, re.M)
    title, what = ("Manual · FM1 Tonic", "The manual") if not h1 or h1.group(1) == "X0X" else ((f"{h1.group(1)} manual" if h1.group(1).startswith("FM1") else f"{h1.group(1)} · FM1 Tonic"), h1.group(1))
    body = convert(md)
    page = (f"<title>{html.escape(title)}</title>\n"
            f'<meta name="description" content="{html.escape(what)} for FM1 Tonic, drum machine firmware for the M-VAVE FM-1.">\n'
            '<link rel="preconnect" href="https://fonts.googleapis.com">\n'
            '<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>\n'
            '<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Barlow:wght@400;600&'
            'family=Barlow+Semi+Condensed:wght@600;700&family=IBM+Plex+Mono:wght@400&display=swap">\n'
            f"<style>{CSS}</style>\n<main>\n{body}\n</main>\n{SCRIPT}")
    skip = re.search(r'</nav>\n<hr>\n<h2 id="([^"]+)"', body)     # past the contents, or to the page
    page = page.replace("<main>", f'<a class="skip" href="#{skip.group(1) if skip else "content"}">Skip to content</a>\n'
                        '<main id="content">', 1)
    if not fragment:
        head, _, rest = page.partition('<a class="skip"')
        rest = '<a class="skip"' + rest
        page = ('<!doctype html>\n<html lang="en">\n<head>\n<meta charset="utf-8">\n'
                '<meta name="viewport" content="width=device-width, initial-scale=1">\n' + head +
                "</head>\n<body>\n" + rest + "</body>\n</html>\n")
    dst.write_text(page)
    print(f"{dst}: {len(page)} bytes")


if __name__ == "__main__":
    main()
