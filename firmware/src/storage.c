/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
 * X0X: object map changed (see below). */
/* Persistent storage on the SPI NOR.
 *
 * Every object has an A/B sector pair. A save goes to the copy that is not
 * the current one: erase the sector, program the payload (from offset 256),
 * then the 32-byte header at offset 0 last. The header is the commit record;
 * on load the valid copy with the highest seq wins, so a torn write leaves
 * the previous copy in charge.
 *
 * Flash access goes through three hooks (also used by the host test):
 *   st_read(off, dst, n)   st_erase(off)   st_prog(off, src, n)
 */
#define ST_MAGIC 0x30583058u                   /* "X0X0": never mistaken for a Felucca object */
#define ST_SECTOR 4096u
#define ST_PAYLOAD_OFF 256u
#define ST_PAYLOAD_MAX (ST_SECTOR - ST_PAYLOAD_OFF)

/* flash map, X0X (plat.h OBJ_*): settings 0xFC000 (FL_GLOB); sound, patterns 1-8, patterns 9-16,
 * the song at 0x97000.. (2 sectors each, below the user sample slots at 0xA0000); the panel
 * calibration at 0xDC000 and motion at 0xDE000 (Felucca's user-preset area, unused by X0X). Nothing
 * above 0xDFFFF: 0xE0000.. is the update loader's staging area. */
typedef struct {
    uint32_t magic;
    uint16_t type, slot;
    uint32_t seq, len, crc, rsv[2];
    uint32_t hcrc;
} st_hdr_t;

static int st_read(uint32_t off, void *dst, uint32_t n);
static int st_erase(uint32_t off);
static int st_prog(uint32_t off, const void *src, uint32_t n);

static uint32_t st_crc32(const void *p, uint32_t n)   /* zlib CRC-32, 4 bits per step */
{
    static const uint32_t T[16] = {
        0x00000000u, 0x1DB71064u, 0x3B6E20C8u, 0x26D930ACu, 0x76DC4190u, 0x6B6B51F4u, 0x4DB26158u, 0x5005713Cu,
        0xEDB88320u, 0xF00F9344u, 0xD6D6A3E8u, 0xCB61B38Cu, 0x9B64C2B0u, 0x86D3D2D4u, 0xA00AE278u, 0xBDBDF21Cu};
    const uint8_t *b = p;
    uint32_t c = 0xFFFFFFFFu;
    while (n--) {
        c ^= *b++;
        c = (c >> 4) ^ T[c & 15u];
        c = (c >> 4) ^ T[c & 15u];
    }
    return ~c;
}

static uint32_t st_sector(uint32_t obj, uint32_t copy)  /* flash offset of copy A (0) / B (1) */
{
    if (obj == OBJ_SET)
        return 0xFC000u + copy * ST_SECTOR;
    if (obj == OBJ_PANEL)
        return 0xDC000u + copy * ST_SECTOR;
    if (obj == OBJ_MOTION)
        return 0xDE000u + copy * ST_SECTOR;
    if (obj == OBJ_SONG)
        return 0x9D000u + copy * ST_SECTOR;
    return 0x97000u + (obj - OBJ_SOUND) * 2u * ST_SECTOR + copy * ST_SECTOR;   /* SOUND, PAT0, PAT1 */
}

static uint8_t st_buf[ST_PAYLOAD_MAX] __attribute__((aligned(4)));

static int st_head(uint32_t obj, uint32_t copy, st_hdr_t *h)   /* commit record valid: 0 */
{
    if (st_read(st_sector(obj, copy), h, sizeof *h))
        return -1;
    if (h->magic != ST_MAGIC || h->type != obj || h->len > ST_PAYLOAD_MAX ||
        h->hcrc != st_crc32(h, sizeof *h - 4u))
        return -1;
    return 0;
}

static int st_body(uint32_t obj, uint32_t copy, const st_hdr_t *h)   /* payload -> st_buf, CRC ok: 0 */
{
    if (st_read(st_sector(obj, copy) + ST_PAYLOAD_OFF, st_buf, h->len) || st_crc32(st_buf, h->len) != h->crc)
        return -1;
    return 0;
}

/* the current copy: the valid one with the highest seq (A on a tie), -1 when
 * neither is valid. Headers first, so only the winner's payload is read (it
 * is left in st_buf); *h gets its header. */
static int st_current(uint32_t obj, st_hdr_t *h)
{
    st_hdr_t a, b;
    int va = st_head(obj, 0, &a) == 0, vb = st_head(obj, 1, &b) == 0;
    if (vb && (!va || b.seq > a.seq)) {
        if (st_body(obj, 1, &b) == 0) {
            *h = b;
            return 1;
        }
        vb = 0;
    }
    if (va && st_body(obj, 0, &a) == 0) {
        *h = a;
        return 0;
    }
    if (vb && st_body(obj, 1, &b) == 0) {
        *h = b;
        return 1;
    }
    return -1;
}

/* load object into dst (up to max bytes); returns the length, or -1 (none; an empty copy, as a restore of an
 * absent object writes it (backup.c), is none too) */
static int st_load(uint32_t obj, void *dst, uint32_t max)
{
    uint32_t i;
    st_hdr_t h;
    if (st_current(obj, &h) < 0 || !h.len)
        return -1;
    if (h.len > max)
        h.len = max;
    for (i = 0; i < h.len; i++)
        ((uint8_t *)dst)[i] = st_buf[i];
    return (int)h.len;
}

/* the copy before the current one, if it is valid too: its length, or -1 (a stream torn across
 * objects falls back to the copies of the save before: project.c load_stream) */
static int st_load_prev(uint32_t obj, void *dst, uint32_t max)
{
    uint32_t i;
    st_hdr_t h;
    int cur = st_current(obj, &h);
    if (cur < 0 || st_head(obj, (uint32_t)!cur, &h) || st_body(obj, (uint32_t)!cur, &h))
        return -1;
    if (h.len > max)
        h.len = max;
    for (i = 0; i < h.len; i++)
        ((uint8_t *)dst)[i] = st_buf[i];
    return (int)h.len;
}

static int st_save(uint32_t obj, const void *src, uint32_t len)
{
    uint32_t seq, base, off;
    int cur, rc;
    st_hdr_t h;
    if (len > ST_PAYLOAD_MAX)
        return -1;
    cur = st_current(obj, &h);
    seq = cur < 0 ? 0u : h.seq;
    base = st_sector(obj, cur == 0 ? 1u : 0u);       /* write the other copy */
    for (off = 0; off < len; off++)
        st_buf[off] = ((const uint8_t *)src)[off];    /* the driver wants RAM sources */
    if ((rc = st_erase(base)) != 0)
        return rc;
    for (off = 0; off < len; off += 256u) {
        uint32_t n = len - off > 256u ? 256u : len - off;
        if ((rc = st_prog(base + ST_PAYLOAD_OFF + off, st_buf + off, n)) != 0)
            return rc;
    }
    h.magic = ST_MAGIC;
    h.type = (uint16_t)obj;
    h.slot = (uint16_t)(cur == 0 ? 1 : 0);
    h.seq = seq + 1u;
    h.len = len;
    h.crc = st_crc32(st_buf, len);
    h.rsv[0] = h.rsv[1] = 0xFFFFFFFFu;
    h.hcrc = st_crc32(&h, sizeof h - 4u);
    if ((rc = st_prog(base, &h, sizeof h)) != 0)       /* the commit record, last */
        return rc;
    {   /* read back: a write-protected or failing part must not report SAVED */
        st_hdr_t chk;
        uint32_t c = cur == 0 ? 1u : 0u;
        if (st_head(obj, c, &chk) || chk.seq != h.seq || st_body(obj, c, &chk))
            return -7;
    }
    return 0;
}

/* USER PRESET SLOTS (SAVE AS): 32 single sectors from ST_PRESET_BASE, header at 0, payload at 256, the header last.
 * No A/B copy (32 slots would need 64 sectors): a power cut in the middle of a slot's save loses that slot only.
 * ST_PRESET_OK is 1 since wip182: the user sample slots (the BREAK loops) that lived at 0xA0000.. were removed from the
 * device (slots.c SMP_USER_SLOTS 0). A device that had loops uploaded still holds their data here: a slot's header
 * (magic, type 0x5000 | slot, CRCs) is what makes it a preset, so old sample data is never read as one, and the first
 * save into a sector erases it. Sectors 0xA0000..0xBFFFF; the panel calibration at 0xDC000 is untouched. */
#define ST_PRESET_BASE 0xA0000u
#define ST_PRESET_OK 1
#define ST_PRESET_N 22                         /* wip654: the user slots (places 9-30; were 32 = 9-40): the USB backup (backup.c) knows these only */
#define ST_PRESET_USERSPAN 32                  /* wip654: the override slots keep their flash index (32-39) and sector (0xC0000..), so the old ones still read: indexes 22-31 are not slots any more */
/* wip255 (option B, step B1): 8 more slots, flash index 32-39, = presets 1-8 written over (they replace the factory preset of that number).
 * Sectors 0xC0000..0xC7FFF: free (nothing in the source uses 0xC0000..0xDBFFF; the panel calibration is at 0xDC000). */
#define ST_PRESET2_BASE 0xC0000u
#define ST_PRESET_NALL 40
static uint32_t st_preset_addr(int slot)
{
    return slot < ST_PRESET_USERSPAN ? ST_PRESET_BASE + (uint32_t)slot * ST_SECTOR : ST_PRESET2_BASE + (uint32_t)(slot - ST_PRESET_USERSPAN) * ST_SECTOR;
}
#define ST_SLOT_BAD(slot) ((slot) < 0 || (slot) >= ST_PRESET_NALL || ((slot) >= ST_PRESET_N && (slot) < ST_PRESET_USERSPAN))

static int st_preset_load(int slot, void *dst, uint32_t max)
{
    st_hdr_t h;
    uint32_t i, a = st_preset_addr(slot);
    if (!ST_PRESET_OK || ST_SLOT_BAD(slot) || st_read(a, &h, sizeof h))
        return -1;
    if (h.magic != ST_MAGIC || h.type != (0x5000u | (uint32_t)slot) || !h.len || h.len > ST_PAYLOAD_MAX ||
        h.hcrc != st_crc32(&h, sizeof h - 4u))
        return -1;
    if (st_read(a + ST_PAYLOAD_OFF, st_buf, h.len) || st_crc32(st_buf, h.len) != h.crc)
        return -1;
    if (h.len > max)
        h.len = max;
    for (i = 0; i < h.len; i++)
        ((uint8_t *)dst)[i] = st_buf[i];
    return (int)h.len;
}

/* A slot's blob as the flash holds it, for backup.c (USB): 0 when the slot holds a whole preset (header and payload
 * CRC-32 right), else -1. *len, *crc: the payload's; *addr: where the payload starts (st_read). */
static int st_preset_info(int slot, uint32_t *len, uint32_t *crc, uint32_t *addr)
{
    st_hdr_t h;
    uint32_t a = st_preset_addr(slot);
    if (!ST_PRESET_OK || ST_SLOT_BAD(slot) || st_read(a, &h, sizeof h))
        return -1;
    if (h.magic != ST_MAGIC || h.type != (0x5000u | (uint32_t)slot) || !h.len || h.len > ST_PAYLOAD_MAX ||
        h.hcrc != st_crc32(&h, sizeof h - 4u))
        return -1;
    if (st_read(a + ST_PAYLOAD_OFF, st_buf, h.len) || st_crc32(st_buf, h.len) != h.crc)
        return -1;
    *len = h.len;
    *crc = h.crc;
    *addr = a + ST_PAYLOAD_OFF;
    return 0;
}

/* A slot written in pieces (backup.c, the USB put: the blob is not all in RAM): erase the slot's sector, program the
 * payload pieces (st_preset_write: off + n <= len, pieces of up to 256 bytes), then the header last (st_preset_commit,
 * which reads the payload back and checks its CRC-32). A power cut in between loses that slot only, as st_preset_save. */
static int st_preset_erase(int slot)
{
    if (!ST_PRESET_OK)
        return -3;
    if (ST_SLOT_BAD(slot))
        return -1;
    return st_erase(st_preset_addr(slot));
}

static int st_preset_write(int slot, uint32_t off, const void *src, uint32_t n)
{
    if (!ST_PRESET_OK)
        return -3;
    if (ST_SLOT_BAD(slot) || off + n > ST_PAYLOAD_MAX)
        return -1;
    return st_prog(st_preset_addr(slot) + ST_PAYLOAD_OFF + off, src, n);
}

static int st_preset_commit(int slot, uint32_t len, uint32_t crc)
{
    st_hdr_t h, chk;
    uint32_t a = st_preset_addr(slot);
    int rc;
    if (!ST_PRESET_OK)
        return -3;
    if (ST_SLOT_BAD(slot) || !len || len > ST_PAYLOAD_MAX)
        return -1;
    h.magic = ST_MAGIC;
    h.type = (uint16_t)(0x5000u | (uint32_t)slot);
    h.slot = (uint16_t)slot;
    h.seq = 1u;
    h.len = len;
    h.crc = crc;
    h.rsv[0] = h.rsv[1] = 0xFFFFFFFFu;
    h.hcrc = st_crc32(&h, sizeof h - 4u);
    if ((rc = st_prog(a, &h, sizeof h)) != 0)          /* the commit record, last */
        return rc;
    if (st_read(a, &chk, sizeof chk) || chk.hcrc != h.hcrc || st_read(a + ST_PAYLOAD_OFF, st_buf, len) ||
        st_crc32(st_buf, len) != crc)
        return -7;
    return 0;
}

static int st_preset_save(int slot, const void *src, uint32_t len)
{
    st_hdr_t h, chk;
    uint32_t off, a = st_preset_addr(slot);
    int rc;
    if (!ST_PRESET_OK)
        return -3;
    if (ST_SLOT_BAD(slot) || !len || len > ST_PAYLOAD_MAX)
        return -1;
    for (off = 0; off < len; off++)
        st_buf[off] = ((const uint8_t *)src)[off];
    if ((rc = st_erase(a)) != 0)
        return rc;
    for (off = 0; off < len; off += 256u) {
        uint32_t n = len - off > 256u ? 256u : len - off;
        if ((rc = st_prog(a + ST_PAYLOAD_OFF + off, st_buf + off, n)) != 0)
            return rc;
    }
    h.magic = ST_MAGIC;
    h.type = (uint16_t)(0x5000u | (uint32_t)slot);
    h.slot = (uint16_t)slot;
    h.seq = 1u;
    h.len = len;
    h.crc = st_crc32(st_buf, len);
    h.rsv[0] = h.rsv[1] = 0xFFFFFFFFu;
    h.hcrc = st_crc32(&h, sizeof h - 4u);
    if ((rc = st_prog(a, &h, sizeof h)) != 0)          /* the commit record, last */
        return rc;
    if (st_read(a, &chk, sizeof chk) || chk.hcrc != h.hcrc || st_read(a + ST_PAYLOAD_OFF, st_buf, len) ||
        st_crc32(st_buf, len) != h.crc)
        return -7;
    return 0;
}

/* wip654: THE SECOND SPOT of a project (the per-step ARP notes, the song, the motion lanes; project.c packs them): one more sector per place,
 * 30 places = 22 user + 8 override. Spot e (0-29: user slot 0-21, override slot 32-39 = e 22-29): e 0-9 at 0xB6000..0xBFFFF (the sectors of the old
 * user places 31-40, which are gone), e 10-29 at 0xC8000..0xDBFFF (free until now). Header type 0x6000 | e (the presets are 0x5000 | slot), so an old
 * preset left in one of these sectors is never read as a second spot. */
#define ST_EXT_BASE_A 0xB6000u
#define ST_EXT_BASE_B 0xC8000u
static int st_ext_index(int slot)
{
    if (slot >= 0 && slot < ST_PRESET_N)
        return slot;
    if (slot >= ST_PRESET_USERSPAN && slot < ST_PRESET_NALL)
        return ST_PRESET_N + (slot - ST_PRESET_USERSPAN);
    return -1;
}
static uint32_t st_ext_addr(int e) { return e < 10 ? ST_EXT_BASE_A + (uint32_t)e * ST_SECTOR : ST_EXT_BASE_B + (uint32_t)(e - 10) * ST_SECTOR; }

static int st_ext_load(int slot, void *dst, uint32_t max)
{
    st_hdr_t h;
    uint32_t i, a;
    int e = st_ext_index(slot);
    if (!ST_PRESET_OK || e < 0)
        return -1;
    a = st_ext_addr(e);
    if (st_read(a, &h, sizeof h))
        return -1;
    if (h.magic != ST_MAGIC || h.type != (0x6000u | (uint32_t)e) || !h.len || h.len > ST_PAYLOAD_MAX ||
        h.hcrc != st_crc32(&h, sizeof h - 4u))
        return -1;
    if (st_read(a + ST_PAYLOAD_OFF, st_buf, h.len) || st_crc32(st_buf, h.len) != h.crc)
        return -1;
    if (h.len > max)
        h.len = max;
    for (i = 0; i < h.len; i++)
        ((uint8_t *)dst)[i] = st_buf[i];
    return (int)h.len;
}

static int st_ext_erase(int slot)
{
    int e = st_ext_index(slot);
    if (!ST_PRESET_OK)
        return -3;
    return e < 0 ? -1 : st_erase(st_ext_addr(e));
}

/* wip669: a second spot for backup.c (USB), as st_preset_info / write / commit: 0 = the spot holds a whole blob (header + CRC-32 right) */
static int st_ext_info(int slot, uint32_t *len, uint32_t *crc, uint32_t *addr)
{
    st_hdr_t h;
    uint32_t a;
    int e = st_ext_index(slot);
    if (!ST_PRESET_OK || e < 0)
        return -1;
    a = st_ext_addr(e);
    if (st_read(a, &h, sizeof h))
        return -1;
    if (h.magic != ST_MAGIC || h.type != (0x6000u | (uint32_t)e) || !h.len || h.len > ST_PAYLOAD_MAX ||
        h.hcrc != st_crc32(&h, sizeof h - 4u))
        return -1;
    if (st_read(a + ST_PAYLOAD_OFF, st_buf, h.len) || st_crc32(st_buf, h.len) != h.crc)
        return -1;
    *len = h.len;
    *crc = h.crc;
    *addr = a + ST_PAYLOAD_OFF;
    return 0;
}

static int st_ext_write(int slot, uint32_t off, const void *src, uint32_t n)
{
    int e = st_ext_index(slot);
    if (!ST_PRESET_OK)
        return -3;
    if (e < 0 || off + n > ST_PAYLOAD_MAX)
        return -1;
    return st_prog(st_ext_addr(e) + ST_PAYLOAD_OFF + off, src, n);
}

static int st_ext_commit(int slot, uint32_t len, uint32_t crc)
{
    st_hdr_t h, chk;
    uint32_t a;
    int rc, e = st_ext_index(slot);
    if (!ST_PRESET_OK)
        return -3;
    if (e < 0 || !len || len > ST_PAYLOAD_MAX)
        return -1;
    a = st_ext_addr(e);
    h.magic = ST_MAGIC;
    h.type = (uint16_t)(0x6000u | (uint32_t)e);
    h.slot = (uint16_t)e;
    h.seq = 1u;
    h.len = len;
    h.crc = crc;
    h.rsv[0] = h.rsv[1] = 0xFFFFFFFFu;
    h.hcrc = st_crc32(&h, sizeof h - 4u);
    if ((rc = st_prog(a, &h, sizeof h)) != 0)
        return rc;
    if (st_read(a, &chk, sizeof chk) || chk.hcrc != h.hcrc || st_read(a + ST_PAYLOAD_OFF, st_buf, len) ||
        st_crc32(st_buf, len) != crc)
        return -7;
    return 0;
}

static int st_ext_save(int slot, const void *src, uint32_t len)
{
    st_hdr_t h, chk;
    uint32_t off, a;
    int rc, e = st_ext_index(slot);
    if (!ST_PRESET_OK)
        return -3;
    if (e < 0 || !len || len > ST_PAYLOAD_MAX)
        return -1;
    a = st_ext_addr(e);
    for (off = 0; off < len; off++)
        st_buf[off] = ((const uint8_t *)src)[off];
    if ((rc = st_erase(a)) != 0)
        return rc;
    for (off = 0; off < len; off += 256u) {
        uint32_t n = len - off > 256u ? 256u : len - off;
        if ((rc = st_prog(a + ST_PAYLOAD_OFF + off, st_buf + off, n)) != 0)
            return rc;
    }
    h.magic = ST_MAGIC;
    h.type = (uint16_t)(0x6000u | (uint32_t)e);
    h.slot = (uint16_t)e;
    h.seq = 1u;
    h.len = len;
    h.crc = st_crc32(st_buf, len);
    h.rsv[0] = h.rsv[1] = 0xFFFFFFFFu;
    h.hcrc = st_crc32(&h, sizeof h - 4u);
    if ((rc = st_prog(a, &h, sizeof h)) != 0)
        return rc;
    if (st_read(a, &chk, sizeof chk) || chk.hcrc != h.hcrc || st_read(a + ST_PAYLOAD_OFF, st_buf, len) ||
        st_crc32(st_buf, len) != h.crc)
        return -7;
    return 0;
}
