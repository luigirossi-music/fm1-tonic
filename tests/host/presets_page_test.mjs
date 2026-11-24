// SPDX-License-Identifier: GPL-3.0-only
// The emulator page's preset backup (web/emu/index.html, PB.presetList / presetGet / presets / presetName) against a
// simulated FM-1 in JS that answers as firmware/src/backup.c does (cmds 0x44 request, 0x45 list, 0x46 data, 0x47 end):
// the list, a get of 3 slots (120-byte frames), an empty slot, a busy FM-1, a lost frame, a wrong CRC, a cut cable,
// a FM-1 that never answers the list, and the names.   node tests/host/presets_page_test.mjs
import fs from "fs";
import path from "path";
import zlib from "zlib";
import { fileURLToPath } from "url";

const root = path.join(path.dirname(fileURLToPath(import.meta.url)), "..", "..");
const html = fs.readFileSync(path.join(root, "web/emu/index.html"), "utf8");
const m = /\/\/ --- backup protocol: begin[^\n]*\n([\s\S]*?)\/\/ --- backup protocol: end/.exec(html);
if (!m) { console.log("FAIL no backup protocol markers in web/emu/index.html"); process.exit(1); }
const PB = new Function(`"use strict";\n${m[1]}\nreturn PB;`)();
Object.assign(PB.DELAY, { poll: 5, idle: 100, ask: 100, first: 100, hs: 50 });

let fails = 0;
const check = (ok, msg) => { console.log(`${ok ? "ok  " : "FAIL"} ${msg}`); if (!ok) fails++; };
const le32 = (v) => [v & 255, (v >>> 8) & 255, (v >>> 16) & 255, (v >>> 24) & 255];

function blobOf(name, n, seed) {
  const b = new Uint8Array(n);
  for (let i = 0; i < n; i++) b[i] = (i * 7 + seed) & 255;
  b.set([0x54, 0x4E, 0x50, 0x31], 0);
  for (let i = 16; i < 40; i++) b[i] = 0;
  for (let i = 0; i < name.length; i++) b[16 + i] = name.charCodeAt(i);
  return b;
}
const slots = { 0: blobOf("Marchiav", 1960, 1), 5: blobOf("A/B: test?", 100, 2), 31: blobOf("", 777, 3) };

// the simulated FM-1
class Dev {
  constructor(o = {}) { this.slots = { ...slots, ...(o.extra || {}) }; this.o = o; this.put = null; this.requestsSent = 0; this.eraseCalls = 0; this.q = []; this.lost = false; this.requests = 0; }
  readReq(addr, len) {
    const u = [0, 0x59, 0x30, 8, 0, 0, 0, ...le32(addr), len & 255, len >> 8, 0];
    let sum = 0;
    for (let i = 6; i < 14; i++) sum += u[i];
    u.push(~sum & 255);
    this.q.push([0xF0, ...PB.pack7(u), 0xF7]);
    this.requestsSent++;
  }
  nextRead() {
    const p = this.put;
    if (p.off >= p.len) {                          // a pass is over
      if (p.pass === 0) { p.pass = 1; p.off = 0; p.c = []; }
      else {
        const ok = PB.crc32(Uint8Array.from(p.c)) === p.crc;
        p.done = true;
        if (this.o.failCode) this.readReq((0xCF000000 | this.o.failCode) >>> 0, 8);
        else if (!ok) this.readReq((0xCF000000 | 5) >>> 0, 8);
        else { this.slots[p.slot] = Uint8Array.from(p.c); this.readReq(0xC0000000, 8); }
        return;
      }
    }
    this.readReq(p.off, Math.min(512, p.len - p.off));
  }
  send(pkt) {
    const f = PB.parseFrame(pkt);
    if (!f) return true;
    if (f.cmd === 0x30) {                          // a response to one of the device's reads: fl, addr(4), len(3), data
      if (!this.put) return true;
      const p = this.put, n = f.body.length - 8, addr = (f.body[1] | f.body[2] << 8 | f.body[3] << 16 | f.body[4] << 24) >>> 0;
      if (p.done) { this.put = null; return true; }       // the answer to the result read
      if (addr === p.off) {
        if (p.pass === 1) p.c.push(...f.body.slice(8, 8 + n));
        p.off += n;
      }
      if (this.o.failCode && p.pass === 0) { p.pass = 1; p.off = p.len; }
      this.nextRead();
      return true;
    }
    if (f.cmd === 0x48) {
      if (this.o.silent) return true;
      const slot = f.body[0], len = f.body[1] | f.body[2] << 8 | f.body[3] << 16, crc = (f.body[5] | f.body[6] << 8 | f.body[7] << 16 | f.body[8] << 24) >>> 0;
      this.put = { slot, len, crc, off: 0, pass: 0, c: [], done: false };
      this.nextRead();
      return true;
    }
    if (f.cmd === 0x49) {
      this.eraseCalls++;
      if (this.o.silent || (this.o.silentOnce && this.eraseCalls === 1)) return true;
      const st = this.o.eraseStatus || 0;
      if (!st) delete this.slots[f.body[0]];
      this.q.push(PB.frame(0x47, [f.body[0], ...le32(0), ...le32(0), st]));
      return true;
    }
    if (f.cmd !== 0x44) return true;
    this.requests++;
    const [sub, slot] = f.body;
    if (this.o.silent) return true;
    if (this.o.busyFirst && this.requests === 1) return true;          // a busy list gets no answer
    if (sub === 1) {
      const body = [0, 0, 0, 0];
      let mask = 0;
      const lens = [];
      for (let i = 0; i < 32; i++) { const b = this.slots[i]; if (b) mask |= 1 << i; lens.push(b ? b.length : 0); }
      const mk = le32(mask >>> 0);
      for (let i = 0; i < 4; i++) body[i] = mk[i];
      for (const l of lens) body.push(l & 255, l >> 8);
      this.q.push(PB.frame(0x45, body));
    } else if (sub === 4 || sub === 6) {                                   // wip670: places 1-8 (slots 32..39), or their second spots (wire byte + 0x80)
      if (this.o.oldFirmware) return true;                                  // an older firmware does not answer
      const base = sub === 6 ? 0x80 : 0, body = [0];
      for (let i = 0; i < 8; i++) { const b = this.slots[base + 32 + i]; if (b) body[0] |= 1 << i; }
      for (let i = 0; i < 8; i++) { const l = (this.slots[base + 32 + i] || []).length; body.push(l & 255, l >> 8); }
      this.q.push(PB.frame(0x45, body));
    } else if (sub === 5) {                                                 // the second spots of slots 0..31
      if (this.o.oldFirmware) return true;
      const body = [0, 0, 0, 0];
      let mask = 0;
      const lens = [];
      for (let i = 0; i < 32; i++) { const b = this.slots[0x80 + i]; if (b) mask |= 1 << i; lens.push(b ? b.length : 0); }
      const mk = le32(mask >>> 0);
      for (let i = 0; i < 4; i++) body[i] = mk[i];
      for (const l of lens) body.push(l & 255, l >> 8);
      this.q.push(PB.frame(0x45, body));
    } else if (sub === 2) {
      if (this.o.cutAfterList) { this.lost = true; return true; }
      const b = this.slots[slot];
      if (!b) { this.q.push(PB.frame(0x47, [slot, ...le32(0), ...le32(0), 1])); return true; }
      let off = 0, n = 0;
      while (off < b.length) {
        const k = Math.min(120, b.length - off);
        if (!(this.o.dropAt === n)) this.q.push(PB.frame(0x46, [slot, ...le32(off), ...b.subarray(off, off + k)]));
        off += k; n++;
      }
      const crc = PB.crc32(b) ^ (this.o.badCrc ? 1 : 0);
      this.q.push(PB.frame(0x47, [slot, ...le32(b.length), ...le32(crc >>> 0), 0]));
    }
    return true;
  }
  drain() { this.q.length = 0; }
  async read(ms) {
    if (this.q.length) return this.q.shift();
    await new Promise((r) => setTimeout(r, Math.min(ms, 20)));
    return this.q.shift() || null;
  }
}
const same = (a, b) => a.length === b.length && a.every((v, i) => v === b[i]);
const fails_with = async (fn) => { try { await fn(); return null; } catch (e) { return e; } };

{
  const l = await PB.presetList(new Dev());
  check(l.mask === (((1 << 0) | (1 << 5) | (1 << 31)) >>> 0), "list: the mask has slots 0, 5, 31");
  check(l.lens[0] === 1960 && l.lens[5] === 100 && l.lens[31] === 777 && l.lens[1] === 0, "... the lengths");
}
{
  const d = new Dev({ busyFirst: true });
  const l = await PB.presetList(d);
  check(d.requests === 2 && l.lens[0] === 1960, "list: no answer the first time (busy), asked again");
}
{
  const e = await fails_with(() => PB.presetList(new Dev({ silent: true })));
  check(e && e.code === "stopped", "list: an FM-1 that never answers: a clear message after 3 tries");
}
{
  const blob = await PB.presetGet(new Dev(), 0);
  check(same(blob, slots[0]), "get: slot 0, 1960 bytes in 120-byte frames, byte for byte");
  check(same(await PB.presetGet(new Dev(), 5), slots[5]), "get: slot 5, 100 bytes");
  check(same(await PB.presetGet(new Dev(), 31), slots[31]), "get: slot 31, 777 bytes");
}
{
  const e = await fails_with(() => PB.presetGet(new Dev(), 7));
  check(e && e.code === "slot1" && /Place 16/.test(e.message), "get of an empty slot: status 1 named with its number (slot 7 = preset 16)");
  const e2 = await fails_with(() => PB.presetGet(new Dev({ dropAt: 3 }), 0));
  check(e2 && e2.code === "lost", "get: a lost frame is seen (offset gap)");
  const e3 = await fails_with(() => PB.presetGet(new Dev({ badCrc: true }), 0));
  check(e3 && e3.code === "lost", "get: a wrong CRC-32 is refused");
  const e4 = await fails_with(() => PB.presetGet(new Dev({ silent: true }), 0));
  check(e4 && e4.code === "stopped", "get: an FM-1 that stops sending: a clear message");
}
{
  const calls = [];
  const out = await PB.presets(new Dev(), (n, t) => calls.push([n, t]));
  check(out.length === 3 && out.map((p) => p.number).join() === "9,14,40", "presets: the 3 used slots as numbers 9, 14, 40");
  check(out.every((p) => same(p.blob, slots[p.slot])), "... each blob as stored");
  check(calls.length && calls[calls.length - 1][0] === 3 && calls[calls.length - 1][1] === 3, "... progress ends at 3 of 3");
  check(out[0].name === "Marchiav" && out[2].name === "preset" && out[1].name === "A_B_ test" && !/[\/:?]/.test(out[1].name), "... file-safe names, an empty name becomes 'preset'");
}
{
  const e = await fails_with(() => PB.presets(new Dev({ cutAfterList: true }), () => {}));
  check(e && (e.code === "lost" || e.code === "stopped"), "a cable cut while receiving: a clear message");
}

// ---------------------------------------------------------------- put, erase, zip (step C2)
function tnpBlob(name, npat = 1, seed = 1) {
  const n = 392 + 196 * npat, b = new Uint8Array(n);
  for (let i = 0; i < n; i++) b[i] = (i * 5 + seed) & 255;
  b.set([0x54, 0x4E, 0x50, 0x31], 0);
  b[4] = n & 255; b[5] = n >> 8; b[9] = npat;
  for (let i = 16; i < 40; i++) b[i] = 0;
  for (let i = 0; i < name.length; i++) b[16 + i] = name.charCodeAt(i);
  return b;
}
{
  const tb = tnpBlob("My Beat", 8, 4);
  check(PB.tnpOk(tb) && !PB.tnpOk(tb.subarray(0, 500)) && !PB.tnpOk(tnpBlob("x", 17)) && !PB.tnpOk(new Uint8Array(588)), "tnpOk: a TNP1 blob of the right size only");
  {                                              // wip227: a preset with the 'TFX1' compressor / reverb / delay trailer (SAVE AS makes these)
    const base = 392 + 196 * 8, fx = new Uint8Array(base + 80);
    fx.set(tnpBlob("With FX", 8, 9));
    fx[4] = fx.length & 255; fx[5] = fx.length >> 8;
    fx.set([0x54, 0x46, 0x58, 0x31], base);
    check(PB.tnpOk(fx), "tnpOk: a preset with the TFX1 trailer (+ 80 bytes) is accepted");
    fx[base] = 0x58;
    check(!PB.tnpOk(fx), "tnpOk: the trailer must say TFX1");
    const f2 = new Uint8Array(base + 96);          // wip242: 'TFX2' (+ 96 bytes: the tracks' own sends)
    f2.set(tnpBlob("With FX2", 8, 9));
    f2[4] = f2.length & 255; f2[5] = f2.length >> 8;
    f2.set([0x54, 0x46, 0x58, 0x32], base);
    check(PB.tnpOk(f2), "tnpOk: a preset with the TFX2 trailer (+ 96 bytes) is accepted");
    f2[base + 3] = 0x31;
    check(!PB.tnpOk(f2), "tnpOk: a 96-byte trailer must say TFX2");
    const f3 = new Uint8Array(base + 112);         // wip316: 'TFX3' (+ 112 bytes: the shelving cuts; what the firmware's SAVE AS writes)
    f3.set(tnpBlob("With FX3", 8, 9));
    f3[4] = f3.length & 255; f3[5] = f3.length >> 8;
    f3.set([0x54, 0x46, 0x58, 0x33], base);
    check(PB.tnpOk(f3), "tnpOk: a preset with the TFX3 trailer (+ 112 bytes) is accepted");
    f3[base + 3] = 0x32;
    check(!PB.tnpOk(f3), "tnpOk: a 112-byte trailer must say TFX3");
    const odd = new Uint8Array(base + 40); odd.set(tnpBlob("x", 8)); odd[4] = odd.length & 255; odd[5] = odd.length >> 8;
    check(!PB.tnpOk(odd), "tnpOk: another extra length is refused");
  }
  const d = new Dev();
  const rc = await PB.presetPut(d, 1, tb);
  check(rc === 0 && same(d.slots[1], tb), "put: slot 10 (index 1) stored, byte for byte (the blob served twice)");
  check(d.requestsSent === 4 * 2 + 1, "... 4 reads of 512 in each of 2 passes, then the result read answered");
  const d2 = new Dev();
  const e = await fails_with(() => PB.presetPut(d2, 1, new Uint8Array(588)));
  check(e && e.code === "head" && d2.requests === 0, "put of a non-TNP blob: refused on the page, nothing sent");
  const d3 = new Dev({ failCode: 10 });
  check(await PB.presetPut(d3, 1, tb) === 10 && !d3.slots[1], "put while the FM-1 plays: its code 10 returned, nothing stored");
  const e2 = await fails_with(() => PB.presetPut(new Dev({ silent: true }), 1, tb));
  check(e2 && e2.code === "stopped", "put: an FM-1 that does not answer: a clear message");
  check(PB.PUT_ERRORS[10] && /STOP/.test(PB.PUT_ERRORS[10]) && PB.PUT_ERRORS[5] && PB.PUT_ERRORS[8], "the put's error codes have messages");
}
{
  const d = new Dev();
  check(d.slots[5] && await PB.presetErase(d, 5) === 0 && !d.slots[5] && d.slots[0], "erase: slot 14 cleared, the others kept");
  const d2 = new Dev({ silentOnce: true });
  check(await PB.presetErase(d2, 5) === 0 && d2.eraseCalls === 2, "erase: no answer the first time, asked again");
  const e = await fails_with(() => PB.presetErase(new Dev({ eraseStatus: 4 }), 5));
  check(e && /playing/i.test(e.message), "erase while playing: a clear message");
  const e2 = await fails_with(() => PB.presetErase(new Dev({ silent: true }), 5));
  check(e2 && e2.code === "stopped", "erase: an FM-1 that does not answer: a clear message");
}
function zipOf(files, deflate) {          // files: [{name, data}]
  const enc = new TextEncoder(), parts = [], central = [];
  let off = 0;
  for (const f of files) {
    const nm = enc.encode(f.name), raw = deflate ? zlib.deflateRawSync(Buffer.from(f.data)) : Buffer.from(f.data), method = deflate ? 8 : 0;
    const crc = zlib.crc32 ? zlib.crc32(Buffer.from(f.data)) >>> 0 : PB.crc32(f.data);
    const lh = Buffer.alloc(30);
    lh.writeUInt32LE(0x04034b50, 0); lh.writeUInt16LE(20, 4); lh.writeUInt16LE(method, 8); lh.writeUInt32LE(crc, 14);
    lh.writeUInt32LE(raw.length, 18); lh.writeUInt32LE(f.data.length, 22); lh.writeUInt16LE(nm.length, 26);
    parts.push(lh, Buffer.from(nm), raw);
    const ch = Buffer.alloc(46);
    ch.writeUInt32LE(0x02014b50, 0); ch.writeUInt16LE(20, 4); ch.writeUInt16LE(20, 6); ch.writeUInt16LE(method, 10);
    ch.writeUInt32LE(crc, 16); ch.writeUInt32LE(raw.length, 20); ch.writeUInt32LE(f.data.length, 24); ch.writeUInt16LE(nm.length, 28); ch.writeUInt32LE(off, 42);
    central.push(ch, Buffer.from(nm));
    off += 30 + nm.length + raw.length;
  }
  const cs = central.reduce((a, b) => a + b.length, 0), end = Buffer.alloc(22);
  end.writeUInt32LE(0x06054b50, 0); end.writeUInt16LE(files.length, 8); end.writeUInt16LE(files.length, 10); end.writeUInt32LE(cs, 12); end.writeUInt32LE(off, 16);
  return new Uint8Array(Buffer.concat([...parts, ...central, end]));
}
{
  const a = tnpBlob("Alpha", 8, 1), b = tnpBlob("Beta", 1, 2);
  const files = [{ name: "09_Alpha.tnp", data: a }, { name: "12_Beta.tnp", data: b }];
  for (const deflate of [false, true]) {
    const out = await PB.unzip(zipOf(files, deflate));
    check(out.length === 2 && out[0].name === "09_Alpha.tnp" && same(out[0].data, a) && same(out[1].data, b), `unzip: a ${deflate ? "deflate" : "stored"} zip, both files byte for byte`);
  }
  const e = await fails_with(() => PB.unzip(new Uint8Array(100)));
  check(e && e.code === "zip", "unzip: not a zip: a plain message");
  const bad = zipOf(files, false); bad[bad.length - 30] ^= 0xFF;
  const e2 = await fails_with(() => PB.unzip(bad.subarray(0, 60)));
  check(e2 && e2.code === "zip", "unzip: a cut zip: a plain message");
  const z = zipOf([...files, { name: "readme.txt", data: new Uint8Array(5) }, { name: "31_Low.tnp", data: a }, { name: "junk.tnp", data: new Uint8Array(600) }, { name: "dir/10_InDir.tnp", data: b }], false);
  const r = await PB.presetFiles("my.zip", z);
  check(r.items.map((i) => i.number).join() === "9,12,10" && r.items[0].name === "Alpha", "presetFiles: the numbers from the names (9, 12, 10), names from the presets");
  check(r.skipped.length === 3 && r.skipped.some((x) => /readme/.test(x)) && r.skipped.some((x) => /31_Low.*1 to 30/.test(x)) && r.skipped.some((x) => /junk/.test(x)), "... skipped with a reason: not .tnp, number out of 9-40, not a preset");
  const one = await PB.presetFiles("Alpha.tnp", a), one2 = await PB.presetFiles("22_Alpha.tnp", a);
  check(one.items.length === 1 && one.items[0].number === 0 && one2.items[0].number === 22, "presetFiles: a single .tnp: no number in the name = 0 (the page asks for a slot), 22 from '22_'");
  const bad1 = await PB.presetFiles("x.tnp", new Uint8Array(50));
  check(!bad1.items.length && bad1.skipped.length === 1, "presetFiles: a single file that is not a preset is skipped");
}
// wip243: .mtpreset -> TNP1 in the page (PB.mtpresetToTnp), byte for byte the same as tools/tnp_pack.c (the fixture .tnp was made by the C tool from the
// fixture .mtpreset; tests/host/mtpreset_fuzz.mjs compares 300 random presets against a built tnp_pack)
{
  const txt = fs.readFileSync(path.join(root, "tests/fixtures/Rostoma_Hieroglycepha__121_.mtpreset"), "utf8");
  const ref = new Uint8Array(fs.readFileSync(path.join(root, "tests/fixtures/Rostoma_Hieroglycepha_121.tnp")));
  const r = PB.mtpresetToTnp(txt, "x");
  check(same(r.blob, ref), "mtpreset: the Rostoma preset converts to the same 1960 bytes as tools/tnp_pack");
  check(r.name === "Rostoma" && r.notes.length === 1 && /12 patterns.*first 8/.test(r.notes[0]), "mtpreset: the name is cut to 8 letters (as the made-up names; wip320); the 12 patterns -> 8 note");
  check(PB.tnpOk(r.blob), "mtpreset: the result passes tnpOk");
  const fac = new Uint8Array(fs.readFileSync(path.join(root, "presets/01_Marchiav.tnp")));
  check(same(r.blob.subarray(40), fac.subarray(40)) && same(r.blob.subarray(0, 16), fac.subarray(0, 16)), "mtpreset: the sounds and patterns are factory preset 1 (only the name differs)");
  // wip322: the way back: TNP1 -> .mtpreset text -> TNP1 gives the same preset (the sounds give or take one 16-bit step, the patterns exactly)
  {
    const dir = path.join(root, "presets");
    let worst = 0, files = 0, patOk = true, hdrOk = true, shape = true;
    for (const fn of fs.readdirSync(dir).filter((f) => /\.tnp$/.test(f)).concat([null])) {
      const b = new Uint8Array(fn ? fs.readFileSync(path.join(dir, fn)) : ref);
      const total = b[4] | (b[5] << 8), a = b.subarray(0, total);
      const txt2 = PB.tnpToMtpreset(a);
      const r2 = PB.mtpresetToTnp(txt2, "x");
      files++;
      shape = shape && txt2.startsWith("MicroTonicPresetV3: {\r\n") && txt2.endsWith("}\r\n");
      for (const i of [4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15]) if (r2.blob[i] !== a[i]) hdrOk = false;
      for (let v = 0; v < 8; v++) {
        for (let i = 0; i < 21; i++) {
          const o = 40 + v * 44 + 2 * i, d = Math.abs((a[o] | (a[o + 1] << 8)) - (r2.blob[o] | (r2.blob[o + 1] << 8)));
          if (d > worst) worst = d;
        }
        if (a[40 + v * 44 + 42] !== r2.blob[40 + v * 44 + 42]) worst = 99999;
      }
      const pb = 40 + 8 * 44;
      for (let i = pb; i < a.length; i++) if (a[i] !== r2.blob[i]) patOk = false;
      if (r2.blob.length !== a.length) patOk = false;
    }
    check(shape && files >= 9, `tnpToMtpreset: ${files} presets (the 8 factory ones + the Rostoma fixture) give a MicroTonicPresetV3 text, CRLF`);
    check(hdrOk, "tnpToMtpreset -> mtpresetToTnp: tempo, swing, step rate, fill rate, master, pattern count the same");
    check(worst <= 2, `tnpToMtpreset -> mtpresetToTnp: every sound parameter and pan back within 2 of 65535 (worst ${worst})`);
    check(patOk, "tnpToMtpreset -> mtpresetToTnp: the patterns (lengths, chained, trigs, accents, fills) are identical");
    const eTnp = await fails_with(async () => PB.tnpToMtpreset(new Uint8Array(100)));
    check(eTnp && eTnp.code === "tnp", "tnpToMtpreset: not a TNP1 blob: a plain message");
  }
  const e1 = await fails_with(async () => PB.mtpresetToTnp("hello"));
  const e2 = await fails_with(async () => PB.mtpresetToTnp("MicroTonicPresetV3: {\n}\n"));
  const e3 = await fails_with(async () => PB.mtpresetToTnp(txt.replace(/OscFreq = [0-9.]+/, "OscFreq = abc")));
  check(e1 && e1.code === "mtp" && e2 && e2.code === "mtp" && e3 && e3.code === "mtp" && /drum 1/.test(e3.message), "mtpreset: not a preset / no patterns / a value that cannot be read: a plain message");
  const t2 = PB.mtpresetToTnp(txt.replace(/\tName = "[^"]*"\r?\n/, "").replace("StepRate = 1/16", "StepRate = 1/4"), "My Fallback");
  check(t2.name === "My Fallb" && t2.blob[10] === 0 && t2.notes.some((n) => /step rate 1\/4/.test(n)), "mtpreset: no Name -> the file's name; an unsupported step rate -> 1/16 with a note");
  const files = [{ name: "10_Rostoma.mtpreset", data: new TextEncoder().encode(txt) }, { name: "Plain name (127).mtpreset", data: new TextEncoder().encode(txt) },
                 { name: "09_Old.tnp", data: tnpBlob("Old", 8, 4) }, { name: "bad.mtpreset", data: new TextEncoder().encode("nope") }, { name: "__MACOSX/._x.mtpreset", data: new Uint8Array(3) }];
  const z = await PB.presetFiles("mix.zip", zipOf(files, true));
  check(z.items.length === 3 && z.items.map((i) => i.number).join() === "10,0,9", "presetFiles: a zip with .mtpreset and .tnp: numbers 10, none, 9");
  check(z.skipped.length === 1 && /bad\.mtpreset: it is not a \.mtpreset file/.test(z.skipped[0]), "presetFiles: a bad .mtpreset is skipped with the reason; the mac junk is ignored silently");
  const one = await PB.presetFiles("Plain name (127).mtpreset", new TextEncoder().encode(txt));
  check(one.items.length === 1 && one.items[0].number === 0 && PB.tnpOk(one.items[0].blob) && one.items[0].notes.length === 1, "presetFiles: a single .mtpreset (no number in its name = 0, the page asks for a slot)");
  const one2 = await PB.presetFiles("22_Foo.mtpreset", new TextEncoder().encode(txt));
  check(one2.items.length === 1 && one2.items[0].number === 22, "presetFiles: '22_' in an .mtpreset name = slot 22");
}
// wip323: the export section of the page (buttons, slot menu) and its scripts
{
  const ids = ["xmt-slot", "xmt-all", "xmt-one", "xmt-bar", "xmt-status"];
  check(ids.every((id) => html.includes(`id=\"${id}\"`)), "export page: the slot menu, both buttons, the bar and the status line are in the page");
  const scripts = [...html.matchAll(/<script(?![^>]*\bsrc=)[^>]*>([\s\S]*?)<\/script>/g)].map((x) => x[1]);
  let okSyntax = scripts.length > 0;
  for (const code of scripts) { try { new Function(code); } catch (e) { okSyntax = false; console.log("  syntax:", e.message); } }
  check(okSyntax, "export page: every script of the page still parses");
  check(/PB\.tnpToMtpreset/.test(html) && /fm1-tonic-mtpresets\.zip/.test(html), "export page: the buttons call tnpToMtpreset and name the zip");
  const names = ["Marchiav", "A/B: test?", "", "../x\\y"].map((n) => `${String(9).padStart(2, "0")}_${PB.presetName(blobOf(n, 500, 1))}.mtpreset`);
  check(names.every((n) => /^\d\d_[A-Za-z0-9 _-]+\.mtpreset$/.test(n)), `export page: file names are safe (${names.join(" | ")})`);
}
// wip670: places 1-8 (slots 32..39) and the second spots (wire byte + 0x80)
{
  const ext = (n, seed) => { const b = new Uint8Array(n); for (let i = 0; i < n; i++) b[i] = (i * 3 + seed) & 255; b.set([0x54, 0x45, 0x58, 0x32], 0); return b; };
  const extra = { 34: blobOf("Third", 700, 9), [0x80]: ext(500, 4), [0x80 | 34]: ext(300, 5) };
  const pl = await PB.presetListPlaces(new Dev({ extra }));
  check(pl && pl.mask === (1 << 2) && pl.lens[2] === 700 && pl.lens[0] === 0, "places 1-8: the list (sub 4) has place 3 only, length 700");
  const xl = await PB.presetListPlaces(new Dev({ extra }), true);
  check(xl && xl.mask === (1 << 2) && xl.lens[2] === 300, "places 1-8: the second spots list (sub 6): place 3, length 300");
  const xm = await PB.presetList(new Dev({ extra }), true);
  check(xm.mask === 1 && xm.lens[0] === 500, "slots 0..31: the second spots list (sub 5): slot 0, length 500");
  const all = await PB.presets(new Dev({ extra }));
  check(all.map((p) => p.number).join() === "3,9,14,40", `presets: places 1-8 first, numbers 3, 9, 14, 40 (got ${all.map((p) => p.number).join()})`);
  check(same(all[0].ext, extra[0x80 | 34]) && same(all[1].ext, extra[0x80 | 0]) && all[2].ext === null, "presets: each place carries its second spot (or null)");
  check(same(all[0].blob, extra[34]), "presets: place 3's preset byte for byte");
  const old = await PB.presets(new Dev({ extra, oldFirmware: true }));
  check(old.map((p) => p.number).join() === "9,14,40" && old.every((p) => p.ext === null), "an older firmware (no answer to sub 4): only the 32 slots, no second spots");
  const d = new Dev({ extra });
  const rc = await PB.presetPut(d, 5, ext(450, 8), () => {}, true);
  check(rc === 0 && d.slots[0x80 | 5] && d.slots[0x80 | 5].length === 450 && same(d.slots[5], slots[5]), "put of a second spot into slot 5: stored under 0x85, the preset itself untouched");
  const e1 = await fails_with(() => PB.presetPut(new Dev(), 5, blobOf("x", 500, 1), () => {}, true));
  check(e1 && e1.code === "head", "a preset (TNP1) is refused as a second spot");
  const e2 = await fails_with(() => PB.presetPut(new Dev(), 5, ext(500, 1)));
  check(e2 && e2.code === "head", "a second spot is refused as a preset");
  const d2 = new Dev({ extra });
  check(await PB.presetErase(d2, 34, true) === 0 && !d2.slots[0x80 | 34] && d2.slots[34], "erase of place 3's second spot: only the second spot is gone");
  const g = await PB.presetGet(new Dev({ extra }), 34, true);
  check(same(g, extra[0x80 | 34]), "get of place 3's second spot byte for byte");
}
{
  const third = blobOf("Third", 588, 9); third[9] = 1; third[4] = 588 & 255; third[5] = 588 >> 8;   // a valid one-pattern TNP1 (total 588 at byte 4)
  const all = await PB.presets(new Dev({ extra: { 34: third, [0x80 | 34]: Uint8Array.from([0x54, 0x45, 0x58, 0x32, ...new Array(30).fill(7)]) } }));
  const files = PB.presetBackupFiles(all);
  check(files.map((f) => f.name).join() === "03_Third.tnp,09_Marchiav.tnp,14_A_B_ test.tnp,40_preset.tnp",
        `backup files: ONE file per place (got ${files.map((f) => f.name).join()})`);
  const sp = PB.splitTnp(files[0].data);
  check(same(sp.main, all[0].blob) && same(sp.ext, all[0].ext), "backup files: place 3's file = its preset + its song part, split back byte for byte");
  const sp2 = PB.splitTnp(files[1].data);
  check(same(sp2.main, all[1].blob) && sp2.ext === null && files[1].data.length === all[1].blob.length, "a place without a second spot: the plain .tnp, unchanged");
  const one = await PB.presetFiles("11_Third.tnp", files[0].data);
  check(one.items.length === 1 && same(one.items[0].blob, all[0].blob) && same(one.items[0].ext, all[0].ext), "restore reading: a .tnp with a song part gives the preset and its second spot");
}
// wip673: restore of whole places (E3b-2b)
{
  check([0, 1, 8, 9, 30, 31, 40].map((n) => PB.slotOf(n)).join() === "-1,32,39,0,21,-1,-1", "slotOf: places 1-8 -> slots 32..39, 9-30 -> 0..21, 0 / 31 / 40 -> -1");
  check(PB.slotOf(1) === 32 && PB.placeNo(PB.slotOf(7)) === 7 && PB.placeNo(PB.slotOf(30)) === 30, "slotOf and placeNo are the same map");
  const third = blobOf("Third", 588, 9); third[9] = 1; third[4] = 588 & 255; third[5] = 588 >> 8;
  const tex = Uint8Array.from([0x54, 0x45, 0x58, 0x32, ...new Array(30).fill(7)]);
  const old2 = Uint8Array.from([0x54, 0x45, 0x58, 0x32, ...new Array(40).fill(9)]);
  // 1 a preset WITH a second spot into place 3: both stored
  const d = new Dev({ extra: { [0x80 | 34]: old2 } });
  check(await PB.restorePlace(d, PB.slotOf(3), { name: "Third", blob: third, ext: tex }) === 0 && same(d.slots[34], third) && same(d.slots[0x80 | 34], tex), "restore: place 3 gets the preset and the new second spot");
  // 2 a plain preset: the old second spot of that place is erased
  const d2 = new Dev({ extra: { [0x80 | 34]: old2 } });
  check(await PB.restorePlace(d2, 34, { name: "Third", blob: third, ext: null }) === 0 && same(d2.slots[34], third) && !d2.slots[0x80 | 34], "restore: a plain .tnp erases the place's old second spot");
  // 3 older firmware: no erase asked, a second spot is refused
  const d3 = new Dev({ oldFirmware: true });
  check(await PB.restorePlace(d3, 5, { name: "x", blob: third, ext: null }, () => {}, false) === 0 && d3.eraseCalls === 0, "restore on an older firmware: no erase of a second spot");
  const e = await fails_with(() => PB.restorePlace(new Dev({ oldFirmware: true }), 5, { name: "x", blob: third, ext: tex }, () => {}, false));
  check(e && e.code === "head", "restore: a song part on an older firmware is refused with a plain message");
  // 4 a failed main put stops before the second spot
  const d4 = new Dev({ failCode: 10 });
  check(await PB.restorePlace(d4, 34, { name: "Third", blob: third, ext: tex }) === 10 && !d4.slots[0x80 | 34], "restore: the preset failed (code 10): its code is returned, no second spot written");
  // 5 what is filled
  const f = await PB.presetFilled(new Dev({ extra: { 34: third, [0x80 | 34]: tex, [0x80]: tex } }));
  check(f.places && f.main.has(0) && f.main.has(34) && f.main.has(5) && f.ext.has(34) && f.ext.has(0) && !f.ext.has(5), "presetFilled: presets and second spots of slots 0..31 and places 1-8");
  const fo = await PB.presetFilled(new Dev({ oldFirmware: true }));
  check(!fo.places && fo.main.has(0) && fo.ext.size === 0, "presetFilled: an older firmware: no places 1-8, no second spots");
  // 6 file numbers
  const a = blobOf("Alpha", 588, 1); a[9] = 1; a[4] = 588 & 255; a[5] = 588 >> 8;
  const r1 = await PB.presetFiles("05_Alpha.tnp", a), r2 = await PB.presetFiles("31_Alpha.tnp", a), r3 = await PB.presetFiles("00_Alpha.tnp", a);
  check(r1.items.length === 1 && r1.items[0].number === 5, "presetFiles: '05_' is place 5 now");
  check(!r2.items.length && /31_Alpha.*1 to 30.*31-40/.test(r2.skipped[0]) && !r3.items.length, "presetFiles: 31..40 (old backups) and 00 are skipped with a message");
}
console.log(fails ? "PRESETS PAGE TEST FAILED" : "presets page test passed");
process.exit(fails ? 1 : 0);
