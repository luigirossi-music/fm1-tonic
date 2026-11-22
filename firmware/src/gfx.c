/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* Small-canvas renderer (no full framebuffer). Draw text/lines into
 * an off-screen strip, then blit it in one DMA transfer. Pixels are stored
 * byte-swapped (the panel takes RGB565 big-endian). */
typedef struct {               /* proportional, see tools/gen_font.py */
    uint8_t h;
    uint8_t pad;               /* bitmap starts this many pixels left of the pen */
    uint8_t first, last;
    const uint8_t *adv;        /* advance per glyph */
    const uint8_t *bw;         /* bitmap width per glyph (starts FONT_PAD left of the pen) */
    const uint16_t *off;       /* byte offset of each glyph */
    const uint8_t *data;
} felucca_font_t;
#include "felucca_font.h"

#define CV_MAX (240u * 144u)      /* X0X: the main area, both bands, is 240 x 144 */
static uint16_t cv_px[CV_MAX] __attribute__((section(".pool")));
static uint32_t cv_w, cv_h;
static int32_t cv_oy;            /* y offset for graph drawing */
static int32_t cv_y0, cv_y1;     /* X0X: the rows drawing may touch (cv_band) */

#define RGB(r, g, b) ((uint16_t)((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3)))
#define C_BLACK 0x0000u
/* wip544 GRIGIOTONIC: the background and the "touched" colour follow the theme (black / white in the dark themes) */
static uint16_t pal_bg, pal_wh = 0xFFFFu;
#define C_BG pal_bg                  /* the page background (black; GRIGIOTONIC: the logo grey) */
#define C_WHITE pal_wh               /* accent only: what is being touched / where we are (GRIGIOTONIC: near black) */
/* The screen's five steps, darkest to brightest, plus white; picked as THEME in GLOBAL. */
typedef struct {
    const char *name;
    uint16_t c[5];
} palette_t;
/* X0X: the first three steps are neutral greys in every theme (structure: rules, empty cells,
 * labels); only the two brightest carry the theme's hue (values, highlights). Colour on the
 * screen then means something: a part, a value, an alarm. */
#define NEUTRALS RGB(34, 34, 40), RGB(74, 74, 82), RGB(150, 150, 158)
static const palette_t PALETTES[] = {
    {"GREEN", {NEUTRALS, RGB(80, 220, 120), RGB(150, 255, 175)}},
    {"AMBER", {NEUTRALS, RGB(232, 136, 30), RGB(255, 184, 70)}},
    {"CYAN", {NEUTRALS, RGB(60, 180, 235), RGB(150, 225, 255)}},
    {"RED", {NEUTRALS, RGB(232, 76, 60), RGB(255, 132, 112)}},
    {"MONO", {NEUTRALS, RGB(196, 196, 202), RGB(236, 236, 240)}},
    /* TONIC: Microtonic's greys and blues (blue-tinted greys; the two brightest steps are the blues) */
    {"TONIC", {RGB(22, 30, 36), RGB(45, 53, 58), RGB(160, 170, 177), RGB(59, 114, 148), RGB(169, 219, 241)}},
    /* GRIGIOTONIC (wip544): the steps flip on the light grey: rules / empty cells a little darker than the grey, labels dark grey, values blue */
    {"GRIGIOTONIC", {RGB(172, 174, 176), RGB(150, 153, 158), RGB(70, 72, 78), RGB(40, 104, 170), RGB(24, 118, 205)}},
};
#define GRIGIO_PAL 6u
#define NPALETTES (sizeof(PALETTES) / sizeof(PALETTES[0]))
static uint16_t pal[5];
#define C_LINE pal[0]                /* 1 rules, separators */
#define C_DIM pal[1]                 /* 2 inactive, empty steps, units */
#define C_GRAY pal[2]                /* 3 labels */
#define C_AMB pal[3]                 /* 4 secondary text */
#define C_HI pal[4]                  /* 5 values, curves */

static void palette_set(uint32_t i)
{
    uint32_t k;
    for (k = 0; k < 5u; k++)
        pal[k] = PALETTES[i % NPALETTES].c[k];
    pal_bg = (i % NPALETTES) == GRIGIO_PAL ? RGB(197, 198, 197) : C_BLACK;   /* the grey of the welcome logo */
    pal_wh = (i % NPALETTES) == GRIGIO_PAL ? RGB(20, 26, 34) : 0xFFFFu;
}

static inline uint16_t swap16(uint32_t c) { return (uint16_t)(((c >> 8) & 0xFFu) | ((c & 0xFFu) << 8)); }

static void cv_begin(uint32_t w, uint32_t h, uint16_t bg)
{
    uint32_t i, n;
    if (w * h > CV_MAX)
        h = CV_MAX / w;
    lcd_sync();                     /* the last blit may still read cv_px */
    cv_w = w;
    cv_h = h;
    cv_oy = cv_y0 = 0;
    cv_y1 = (int32_t)h;
    n = w * h;
    for (i = 0; i < n; i++)
        cv_px[i] = swap16(bg);
}

/* X0X: draw the next things as if the canvas were rows y0 .. y0+h-1 alone (one band of a
 * canvas that holds several, sent in one transfer so the bands change on screen together) */
static void cv_band(int32_t y0, int32_t h)
{
    cv_oy = cv_y0 = y0;
    cv_y1 = y0 + h;
}

static void cv_blit(uint32_t x, uint32_t y) { lcd_blit(x, y, cv_w, cv_h, cv_px); }

/* canvas rows r0 .. cv_h-1 only, to screen row y + r0 */
static void cv_blit_from(uint32_t x, uint32_t y, uint32_t r0)
{
    if (r0 < cv_h)
        lcd_blit(x, y + r0, cv_w, cv_h - r0, cv_px + r0 * cv_w);
}

static inline void cv_pset(int32_t x, int32_t y, uint16_t c)
{
    y += cv_oy;
    if ((uint32_t)x < cv_w && y >= cv_y0 && y < cv_y1)
        cv_px[(uint32_t)y * cv_w + (uint32_t)x] = swap16(c);
}

static void cv_rect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t c)
{
    int32_t x1 = x + w, y1 = y + h + cv_oy, i;
    uint16_t sc = swap16(c);
    y += cv_oy;                             /* clipped once, then filled row by row */
    if (x < 0)
        x = 0;
    if (y < cv_y0)
        y = cv_y0;
    if (x1 > (int32_t)cv_w)
        x1 = (int32_t)cv_w;
    if (y1 > cv_y1)
        y1 = cv_y1;
    for (; y < y1; y++)
        for (i = x; i < x1; i++)
            cv_px[(uint32_t)y * cv_w + (uint32_t)i] = sc;
}

static void cv_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint16_t c)
{
    int32_t dx = x1 > x0 ? x1 - x0 : x0 - x1, sx = x0 < x1 ? 1 : -1;
    int32_t dy = y1 > y0 ? y0 - y1 : y1 - y0, sy = y0 < y1 ? 1 : -1;
    int32_t err = dx + dy, guard = 2000;
    while (guard--) {                       /* bounded: a line is never longer than 480 px */
        int32_t e2 = 2 * err;               /* both tests use the same error value */
        cv_pset(x0, y0, c);
        if (x0 == x1 && y0 == y1)
            break;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

/* glyph index of a character: lower case folds to upper case when the font
 * has none, anything missing (and C1 controls) draws as '?' */
static uint32_t glyph(const felucca_font_t *f, uint32_t ch)
{
    if (ch >= 'a' && ch <= 'z' && f->last < 'a')
        ch -= 32u;
    if (ch < f->first || ch > f->last || (ch >= 127u && ch < 160u))
        ch = '?';
    return ch - f->first;
}

/* text in colour c, each pixel's coverage blended over what is already on the canvas (an
 * anti-aliased face on a coloured box gets no dark fringe); returns the end x */
static int32_t cv_text(int32_t x, int32_t y, const felucca_font_t *f, const char *s, uint16_t c)
{
    uint32_t r = c >> 11, g = (c >> 5) & 63u, b = c & 31u;
    for (; *s; s++) {
        uint32_t gi = glyph(f, (uint8_t)*s), gx, gy, w, bpr;
        const uint8_t *gd;
        w = f->bw[gi];
        bpr = (w + 1u) / 2u;
        gd = f->data + f->off[gi];
        for (gy = 0; gy < f->h; gy++)
            for (gx = 0; gx < w; gx++) {
                uint32_t a = gd[gy * bpr + gx / 2u];
                int32_t px = x - f->pad + (int32_t)gx, py = y + (int32_t)gy + cv_oy;
                a = (gx & 1u) ? (a & 15u) : (a >> 4);
                if (!a || (uint32_t)px >= cv_w || py < cv_y0 || py >= cv_y1)
                    continue;
                if (a == 15u) {
                    cv_px[(uint32_t)py * cv_w + (uint32_t)px] = swap16(c);
                } else {
                    uint32_t bg = swap16(cv_px[(uint32_t)py * cv_w + (uint32_t)px]);
                    uint32_t br = bg >> 11, bgg = (bg >> 5) & 63u, bb = bg & 31u;
                    uint32_t nr = (r * a + br * (15u - a)) / 15u, ng = (g * a + bgg * (15u - a)) / 15u,
                             nb = (b * a + bb * (15u - a)) / 15u;
                    cv_px[(uint32_t)py * cv_w + (uint32_t)px] = swap16((nr << 11) | (ng << 5) | nb);
                }
            }
        x += f->adv[gi];
    }
    return x;
}

static int32_t text_w(const felucca_font_t *f, const char *s)
{
    int32_t w = 0;
    for (; *s; s++)
        w += f->adv[glyph(f, (uint8_t)*s)];
    return w;
}

/* one-shot: text in a box, cleared to black, blitted */
static void draw_text_box(uint32_t x, uint32_t y, uint32_t w, const felucca_font_t *f, const char *s,
                          uint16_t c, int align)
{
    int32_t tw = text_w(f, s), tx = 0;
    cv_begin(w, f->h, C_BLACK);
    if (align == 1)
        tx = ((int32_t)w - tw) / 2;
    else if (align == 2)
        tx = (int32_t)w - tw;
    cv_text(tx, 0, f, s, c);
    cv_blit(x, y);
    lcd_sync();                     /* one-shots (boot, crash, UBOOT, update) finish here */
}

/* The FM1 TONIC welcome screen (boot): assets/fm1-tonic-welcome.svg drawn with fills only (rows of lcd_fill, no
 * bitmap, no RAM): a small grey box with the name on top, four knobs, eight steps, the version below. */
static uint32_t wl_isqrt(uint32_t n)
{
    uint32_t r = 0;
    while ((r + 1u) * (r + 1u) <= n)
        r++;
    return r;
}
static void wl_disc(int32_t cx, int32_t cy, int32_t r, uint16_t c)
{
    int32_t dy;
    for (dy = -r; dy <= r; dy++) {
        int32_t hw = (int32_t)wl_isqrt((uint32_t)(r * r + r - dy * dy));
        lcd_fill((uint32_t)(cx - hw), (uint32_t)(cy + dy), (uint32_t)(2 * hw + 1), 1, c);
    }
}
static int32_t wl_gy0 = 36, wl_gh = 155;            /* wip260: the grey's first row and height (the logo is scaled) */
static uint16_t wl_grey(int32_t y)                  /* the box's light-to-darker grey, by screen row */
{
    int32_t g = 205 - 36 * (y - wl_gy0) / wl_gh;
    return RGB(g, g, g);
}
static void wl_rrect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, int flat, uint16_t fc)
{
    int32_t j;
    for (j = 0; j < h; j++) {
        int32_t e = j < h - 1 - j ? j : h - 1 - j, in = 0;
        if (e < r) {
            int32_t d2 = 2 * (r - e) - 1;
            in = r - (int32_t)wl_isqrt((uint32_t)(r * r - d2 * d2 / 4));
        }
        lcd_fill((uint32_t)(x + in), (uint32_t)(y + j), (uint32_t)(w - 2 * in), 1, flat ? fc : wl_grey(y + j));
    }
}
static int32_t wl_big(int32_t x, const felucca_font_t *f, const char *s, uint16_t c)   /* f at 2 x (the welcome name), on the canvas */
{
    uint32_t cr = c >> 11, cg = (c >> 5) & 63u, cb = c & 31u;
    for (; *s; s++) {
        uint32_t gi = glyph(f, (uint8_t)*s), gx, gy, gw = f->bw[gi], bpr = (gw + 1u) / 2u;
        const uint8_t *gd = f->data + f->off[gi];
        for (gy = 0; gy < f->h; gy++)
            for (gx = 0; gx < gw; gx++) {
                uint32_t a = gd[gy * bpr + gx / 2u], i, j;
                a = (gx & 1u) ? (a & 15u) : (a >> 4);
                if (!a)
                    continue;
                for (j = 0; j < 2u; j++)
                    for (i = 0; i < 2u; i++) {
                        int32_t px = x + ((int32_t)gx - f->pad) * 2 + (int32_t)i, py = (int32_t)gy * 2 + (int32_t)j;
                        uint32_t bg;
                        if ((uint32_t)px >= cv_w || (uint32_t)py >= cv_h)
                            continue;
                        bg = swap16(cv_px[(uint32_t)py * cv_w + (uint32_t)px]);
                        cv_px[(uint32_t)py * cv_w + (uint32_t)px] = swap16((((cr * a + (bg >> 11) * (15u - a)) / 15u) << 11) |
                            ((((cg * a + ((bg >> 5) & 63u) * (15u - a)) / 15u)) << 5) | ((cb * a + (bg & 31u) * (15u - a)) / 15u));
                    }
            }
        x += f->adv[glyph(f, (uint8_t)*s)] * 2;
    }
    return x;
}
static uint32_t wl_alpha(const felucca_font_t *f, uint32_t gi, int32_t gx, int32_t gy)   /* a glyph pixel's 0..15, 0 outside */
{
    uint32_t gw = f->bw[gi], bpr = (gw + 1u) / 2u, a;
    if (gx < 0 || gy < 0 || gx >= (int32_t)gw || gy >= (int32_t)f->h)
        return 0;
    a = f->data[f->off[gi] + (uint32_t)gy * bpr + (uint32_t)gx / 2u];
    return (gx & 1) ? (a & 15u) : (a >> 4);
}
/* wip260: the welcome name at k/1000 x the font (smooth: each pixel is a bilinear sample of the glyph), the same colours */
static int32_t wl_scaled(int32_t x, const felucca_font_t *f, const char *s, uint16_t c, int32_t k)
{
    uint32_t cr = c >> 11, cg = (c >> 5) & 63u, cb = c & 31u;
    int32_t xm = x * 1000;
    for (; *s; s++) {
        uint32_t gi = glyph(f, (uint8_t)*s);
        int32_t sw = (int32_t)f->bw[gi] * k / 1000 + 2, sh = (int32_t)f->h * k / 1000 + 2, ox, oy;
        int32_t pad = (int32_t)f->pad * k / 1000;
        for (oy = 0; oy < sh; oy++)
            for (ox = 0; ox < sw; ox++) {
                int32_t sx = ((ox * 1000 + 500) * 1000) / k - 500, sy = ((oy * 1000 + 500) * 1000) / k - 500;
                int32_t ix = sx >= 0 ? sx / 1000 : -1 - (-sx - 1) / 1000, iy = sy >= 0 ? sy / 1000 : -1 - (-sy - 1) / 1000;
                int32_t fx = sx - ix * 1000, fy = sy - iy * 1000, px = xm / 1000 + ox - pad, py = oy;
                uint32_t a00 = wl_alpha(f, gi, ix, iy), a10 = wl_alpha(f, gi, ix + 1, iy),
                         a01 = wl_alpha(f, gi, ix, iy + 1), a11 = wl_alpha(f, gi, ix + 1, iy + 1), a, bg;
                a = (a00 * (uint32_t)((1000 - fx) * (1000 - fy)) + a10 * (uint32_t)(fx * (1000 - fy)) +
                     a01 * (uint32_t)((1000 - fx) * fy) + a11 * (uint32_t)(fx * fy)) / 58824u;   /* 0..255 (15 x 1e6 / 255) */
                if (!a || (uint32_t)px >= cv_w || (uint32_t)py >= cv_h)
                    continue;
                if (a > 255u)
                    a = 255u;
                bg = swap16(cv_px[(uint32_t)py * cv_w + (uint32_t)px]);
                cv_px[(uint32_t)py * cv_w + (uint32_t)px] = swap16((((cr * a + (bg >> 11) * (255u - a)) / 255u) << 11) |
                    ((((cg * a + ((bg >> 5) & 63u) * (255u - a)) / 255u)) << 5) | ((cb * a + (bg & 31u) * (255u - a)) / 255u));
            }
        xm += (int32_t)f->adv[gi] * k;
    }
    return xm / 1000;
}
static void welcome_draw(void)
{
    static const int16_t KS[4] = {-500, 423, -940, 707}, KC[4] = {866, 906, 342, 707};   /* pointer angles x 1000 */
    const felucca_font_t *f = &FONT_M;
    const int32_t K = 1600;                                      /* wip260: the whole logo at 80 % (the name: 2 x FONT_M x 0.8) */
    const int32_t X0 = 30, Y0 = 36;                              /* the scaled box's corner */
    int32_t i, t, a, b, tx, ty = Y0 + 7, r, x, h = (int32_t)f->h * K / 1000 + 2;
    lcd_fill(0, 0, 240, 240, C_BLACK);
    wl_gy0 = Y0 + 2;
    wl_gh = 124;
    wl_rrect(X0, Y0, 180, 128, 11, 1, RGB(106, 106, 106));      /* the box: border, then the grey */
    wl_rrect(X0 + 2, Y0 + 2, 176, 124, 10, 0, 0);
    a = text_w(f, "FM1") * K / 1000;
    b = text_w(f, "TONIC") * K / 1000;
    t = a + 6 + b;
    tx = X0 + (180 - t) / 2;
    cv_begin((uint32_t)t, (uint32_t)h, C_BLACK);                 /* the name on the grey */
    for (r = 0; r < h; r++)
        for (x = 0; x < t; x++)
            cv_px[(uint32_t)(r * t + x)] = swap16(wl_grey(ty + r));
    wl_scaled(0, f, "FM1", RGB(79, 132, 176), K);
    wl_scaled(a + 6, f, "TONIC", RGB(47, 47, 47), K);
    cv_blit((uint32_t)tx, (uint32_t)ty);
    for (i = 0; i < 4; i++) {                                    /* four knobs, a blue pointer each */
        int32_t cx = X0 + (40 + 48 * i) * 4 / 5, cy = Y0 + 68;
        wl_disc(cx, cy, 18, RGB(122, 122, 122));
        wl_disc(cx, cy, 16, RGB(226, 226, 226));
        wl_disc(cx - 4, cy - 5, 7, RGB(246, 246, 246));
        for (r = 0; r <= 13; r += 2)
            wl_disc(cx + KS[i] * r / 1000, cy - KC[i] * r / 1000, 2, RGB(33, 70, 200));
    }
    for (i = 0; i < 8; i++) {                                    /* eight steps, three lit */
        int lit = i == 0 || i == 3 || i == 5;
        wl_disc(X0 + 34 + 16 * i, Y0 + 109, 6, lit ? RGB(59, 120, 208) : RGB(128, 128, 128));
        wl_disc(X0 + 34 + 16 * i, Y0 + 109, 4, lit ? RGB(143, 196, 248) : RGB(222, 222, 222));
    }
    {                                                            /* wip260: BY LUIGI ROSSI, the same font (FONT_B) at 1.45 x (wip261) */
        const char *by = "BY LUIGI ROSSI";
        int32_t bw = text_w(&FONT_B, by) * 1450 / 1000 + 2, bh = (int32_t)FONT_B.h * 1450 / 1000 + 2;
        cv_begin((uint32_t)bw, (uint32_t)bh, C_BLACK);
        wl_scaled(0, &FONT_B, by, RGB(188, 194, 199), 1450);
        cv_blit((uint32_t)((240 - bw) / 2), 186u);
    }
    lcd_sync();
}
/* wip215: the version, big, on its own, for half a second after the welcome screen (FONT_M at 2 x, centred) */
static void welcome_version(const char *ver0)
{
    const felucca_font_t *f = &FONT_M;
    char ver[24];
    int n = 0;
    const char *p = ver0 + ((ver0[0] == 'V' || ver0[0] == 'v') ? 1 : 0);   /* wip238: 'V0.9' is shown as 'VER. 0.9' */
    ver[n++] = 'V'; ver[n++] = 'E'; ver[n++] = 'R'; ver[n++] = '.'; ver[n++] = ' ';
    while (*p && n < 23)
        ver[n++] = *p++;
    ver[n] = 0;
    {
    int32_t w = text_w(f, ver) * 2, h = (int32_t)f->h * 2;
    lcd_fill(0, 0, 240, 240, C_BLACK);
    cv_begin((uint32_t)w, (uint32_t)h, C_BLACK);
    wl_big(0, f, ver, RGB(143, 196, 248));
    cv_blit((uint32_t)((240 - w) / 2), (uint32_t)((240 - h) / 2));
    lcd_sync();
    }
}
