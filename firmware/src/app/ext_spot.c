/* SPDX-License-Identifier: GPL-3.0-only */
/* wip657 (step E2a of "a project = 2 spots"): the SECOND SPOT of a project. It carries what does not fit in the TNP1 blob of
 * the first spot: the song, the motion lanes and the ARP notes (project_t.arr + project_t.note, 20484 bytes raw), packed with
 * lz.c into ONE flash object (PLAT_STORE_MAX = 3840 bytes, header included).
 *
 *   bytes 0-3   'T' 'E' 'X' '2'
 *   bytes 4-7   raw length (little endian)       bytes 8-11  CRC-32 of the raw bytes
 *   bytes 12-15 packed length                    bytes 16-19 PAIR MARK (wip659, E2c): the CRC-32 of the place's main spot (its TNP1 blob)
 *   bytes 20..  the lz stream
 *
 * The pair mark is how a half-finished save is recognised: the main spot is written first, the second one after it. If the power goes in between, the place
 * holds the NEW main spot and the OLD second spot, whose mark is the OLD main spot's CRC: it does not match, ext_unpack refuses it and the project loads
 * with no song, no lanes and no notes (never the old ones with the new patterns).
 *
 * Kept free of project.c's types so tests/host/ext_spot_test.c can run it alone (it needs lz.c and a crc32_of() before it).
 * Pack: out = 0 only measures. Returns the whole length (header + stream), 0 = it would not fit in max bytes: nothing is
 * lost then, the caller refuses the save (MEMORY FULL) before it writes anything. mark = the pair mark above (ignored when out = 0). Unpack: 0 = the raw bytes are in place,
 * -1 = not there / damaged / another pair's (the caller empties song, lanes and notes). */
#define EXT_HDR 20u

typedef struct {
    uint8_t *out;
    uint32_t cap, n;
    int over;
} ext_sink_t;

static int ext_put(void *ctx, const uint8_t *p, uint32_t k)
{
    ext_sink_t *e = ctx;
    if (k > e->cap - e->n) {
        e->over = 1;
        return -1;
    }
    if (e->out)
        for (uint32_t i = 0; i < k; i++)
            e->out[EXT_HDR + e->n + i] = p[i];
    e->n += k;
    return 0;
}

static uint32_t ext_pack(const uint8_t *raw, uint32_t rawlen, uint8_t *out, uint32_t max, uint32_t mark)
{
    ext_sink_t e = {out, max > EXT_HDR ? max - EXT_HDR : 0u, 0, 0};
    lz_sink_t sink = {ext_put, &e};
    uint32_t crc;
    if (max <= EXT_HDR || lz_compress(raw, rawlen, &sink) || e.over)
        return 0;
    if (out) {
        crc = crc32_of(raw, rawlen);
        out[0] = 'T'; out[1] = 'E'; out[2] = 'X'; out[3] = '2';
        for (int i = 0; i < 4; i++) {
            out[4 + i] = (uint8_t)(rawlen >> (8 * i));
            out[8 + i] = (uint8_t)(crc >> (8 * i));
            out[12 + i] = (uint8_t)(e.n >> (8 * i));
            out[16 + i] = (uint8_t)(mark >> (8 * i));
        }
    }
    return EXT_HDR + e.n;
}

typedef struct {
    const uint8_t *p;
    uint32_t left;
} ext_src_t;

static int ext_get(void *ctx)
{
    ext_src_t *s = ctx;
    return s->left ? (s->left--, *s->p++) : -1;
}

static uint32_t ext_rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static int ext_unpack(const uint8_t *b, uint32_t n, uint8_t *raw, uint32_t rawlen, uint32_t mark)
{
    uint32_t plen;
    ext_src_t s;
    lz_src_t src;
    if (n < EXT_HDR || b[0] != 'T' || b[1] != 'E' || b[2] != 'X' || b[3] != '2' || ext_rd32(b + 4) != rawlen || ext_rd32(b + 16) != mark)
        return -1;                                  /* (a wrong mark: nothing is decoded, the raw bytes are not touched) */
    plen = ext_rd32(b + 12);
    if (plen > n - EXT_HDR)
        return -1;
    s.p = b + EXT_HDR;
    s.left = plen;
    src.get = ext_get;
    src.ctx = &s;
    if (lz_decompress(&src, raw, rawlen) || crc32_of(raw, rawlen) != ext_rd32(b + 8))
        return -1;
    return 0;
}
