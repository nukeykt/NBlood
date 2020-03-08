//-------------------------------------------------------------------------
/*
Copyright (C) 2016 EDuke32 developers and contributors

This file is part of EDuke32.

EDuke32 is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------

#include "duke3d.h"
#include "compat.h"
#include "sbar.h"
#include "menus.h"

int32_t g_textstat = RS_AUTO | RS_NOCLIP | RS_TOPLEFT;

void G_InitText()
{
    // check if the minifont will support lowercase letters (3136-3161)
    // there is room for them in tiles012.art between "[\]^_." and "{|}~"
    minitext_lowercase = 1;

    for (int i = MINIFONT + ('a'-'!'); minitext_lowercase && i < MINIFONT + ('z'-'!') + 1; ++i)
        minitext_lowercase &= (int)tileLoad(i);

    TileFontPtr_t tilefont_BIGALPHANUM = tilefontGetPtr(BIGALPHANUM);
    unsigned i, j;
    j = -10;
    for (i = 0; i <= '9'-'0'; ++i)
    {
        char c = '0' + i;
        uint32_t chr32 = tilefontGetChr32FromASCII(c);
        tilefontMaybeDefineMapping(tilefont_BIGALPHANUM, chr32, BIGALPHANUM + i + j);
    }
    j += i;
    for (i = 0; i <= 'Z'-'A'; ++i)
    {
        char c = 'A' + i;
        uint32_t chr32 = tilefontGetChr32FromASCII(c);
        tilefontMaybeDefineMapping(tilefont_BIGALPHANUM, chr32, BIGALPHANUM + i + j);
    }
    j += i;
    for (i = 0; i <= 'z'-'a'; ++i)
    {
        char c = 'a' + i;
        uint32_t chr32 = tilefontGetChr32FromASCII(c);
        tilefontMaybeDefineMapping(tilefont_BIGALPHANUM, chr32, BIGALPHANUM + i + j);
    }
    j += i;
    struct { char c; int32_t tilenum; } const bigalphanum_mappings[] =
    {
        { '-', BIGALPHANUM - 11, },
        { '_', BIGALPHANUM - 11, },
        { '.', BIGPERIOD, },
        { ',', BIGCOMMA, },
        { '!', BIGX_, },
        { '?', BIGQ, },
        { ';', BIGSEMI, },
        { ':', BIGCOLIN, },
        { '\\', BIGALPHANUM + 68, },
        { '/', BIGALPHANUM + 68, },
        { '%', BIGALPHANUM + 69, },
        { '`', BIGAPPOS, },
        { '\"', BIGAPPOS, },
        { '\'', BIGAPPOS, },
    };
    for (auto const & mapping : bigalphanum_mappings)
    {
        uint32_t chr32 = tilefontGetChr32FromASCII(mapping.c);
        tilefontMaybeDefineMapping(tilefont_BIGALPHANUM, chr32, mapping.tilenum);
    }

    TileFontPtr_t tilefont_STARTALPHANUM = tilefontGetPtr(STARTALPHANUM);
    TileFontPtr_t tilefont_MINIFONT = tilefontGetPtr(MINIFONT);
    for (i = 0; i <= '\x7F'-'!'; ++i)
    {
        char c = '!' + i;
        uint32_t chr32 = tilefontGetChr32FromASCII(c);
        tilefontMaybeDefineMapping(tilefont_STARTALPHANUM, chr32, STARTALPHANUM + i);
        tilefontMaybeDefineMapping(tilefont_MINIFONT, chr32, MINIFONT + i);
    }
}

// assign the character's tilenum
static int32_t G_GetStringTileLegacy(int32_t font, char c, int32_t f)
{
    if (f & TEXT_DIGITALNUMBER)
        return c - '0' + font;
    else if (f & (TEXT_BIGALPHANUM|TEXT_GRAYFONT))
    {
        int32_t offset = (f & TEXT_GRAYFONT) ? 26 : 0;

        if (c >= '0' && c <= '9')
            return c - '0' + font + ((f & TEXT_GRAYFONT) ? 26 : -10);
        else if (c >= 'a' && c <= 'z')
            return c - 'a' + font + ((f & TEXT_GRAYFONT) ? -26 : 26);
        else if (c >= 'A' && c <= 'Z')
            return c - 'A' + font;
        else switch (c)
        {
        case '_':
        case '-':
            return font - (11 + offset);
        case '.':
            return font + (BIGPERIOD - (BIGALPHANUM + offset));
        case ',':
            return font + (BIGCOMMA - (BIGALPHANUM + offset));
        case '!':
            return font + (BIGX_ - (BIGALPHANUM + offset));
        case '?':
            return font + (BIGQ - (BIGALPHANUM + offset));
        case ';':
            return font + (BIGSEMI - (BIGALPHANUM + offset));
        case ':':
            return font + (BIGCOLIN - (BIGALPHANUM + offset));
        case '\\':
        case '/':
            return font + (68 - offset); // 3008-2940
        case '%':
            return font + (69 - offset); // 3009-2940
        case '`':
        case '\"': // could be better hacked in
        case '\'':
            return font + (BIGAPPOS - (BIGALPHANUM + offset));

        case '\x7F':
            return font;
            break;

        case '\n':
            return SCREENTEXT_NEWLINE;
        case '\t':
            return SCREENTEXT_TAB;
        case ' ':
        default: // unknown character
            return SCREENTEXT_SPACE;
        }
    }
    else
        return c - '!' + font; // uses ASCII order
}

int32_t G_GetStringTileASCII(TileFontPtr_t tilefontPtr, int32_t font, char c, int32_t f)
{
    if (tilefontPtr.opaque != nullptr)
        return tilefontLookup(tilefontPtr, tilefontGetChr32FromASCII(c));

    return G_GetStringTileLegacy(font, c, f);
}

static inline int32_t G_GetStringTile(TileFontPtr_t tilefontPtr, uint32_t chr32)
{
    if (tilefontPtr.opaque != nullptr)
        return tilefontLookup(tilefontPtr, chr32);

    return 0;
}

uint32_t G_ScreenTextFromString(ScreenTextGlyph_t * const textbuf, char const * str, char const * const end, TileFontPtr_t tilefontPtr, int32_t font, int32_t flags)
{
    ScreenTextGlyph_t * text = textbuf;
    char c;

    while (str < end && (c = *str))
    {
        // handle escape sequences
        if (c == '^' && str + 1 < end && Bisdigit(*(str + 1)) && !(flags & TEXT_LITERALESCAPE))
        {
            char smallbuf[4];

            ++str;
            smallbuf[0] = *str;

            ++str;
            if (str < end && Bisdigit(*str))
            {
                smallbuf[1] = *str;
                smallbuf[2] = '\0';
                ++str;
            }
            else
                smallbuf[1] = '\0';

            if (!(flags & TEXT_IGNOREESCAPE))
            {
                uint8_t const pal = Batoi(smallbuf);
                *text++ = SCREENTEXT_PALCHANGE | pal;
            }

            continue;
        }

        if (!(c & 0x80))
        {
            // handle case bits
            if (flags & TEXT_UPPERCASE)
            {
                if (flags & TEXT_INVERTCASE) // optimization...?
                { // v^ important that these two ifs remain separate due to the else below
                    if (Bisupper(c))
                        c = Btolower(c);
                }
                else if (Bislower(c))
                    c = Btoupper(c);
            }
            else if (flags & TEXT_INVERTCASE)
            {
                if (Bisupper(c))
                    c = Btolower(c);
                else if (Bislower(c))
                    c = Btoupper(c);
            }

            if ((flags & TEXT_CONSTWIDTHNUMS) && c >= '0' && c <= '9')
                *text++ = SCREENTEXT_CONSTWIDTH;

            if (c == '\n')
                *text++ = SCREENTEXT_NEWLINE;
            else if (c == '\t')
                *text++ = SCREENTEXT_TAB;
            else if (c == ' ')
                *text++ = SCREENTEXT_SPACE;
            else
                *text++ = G_GetStringTileASCII(tilefontPtr, font, c, flags);

            ++str;
        }
        else
        {
            uint32_t chr32 = 0;
            size_t bytes = min(utf8charbytes(c), size_t(end - str));
            memcpy(&chr32, str, bytes);

            *text++ = G_GetStringTile(tilefontPtr, chr32);

            str += bytes;
        }
    }

    *text = 0;

    return text - textbuf;
}

void G_SetScreenTextEmpty(vec2_t & empty, int32_t font, int32_t f)
{
    TileFontPtr_t tilefontPtr = tilefontFind(font);

    if (f & (TEXT_INTERNALSPACE|TEXT_TILESPACE))
    {
        char space = '.'; // this is subject to change as an implementation detail
        if (f & TEXT_TILESPACE)
            space = '\x7F'; // tile after '~'
        uint32_t const tile = G_GetStringTileASCII(tilefontPtr, font, space, f);
        Bassert(tile < MAXTILES);

        empty.x += tilesiz[tile].x << 16;
    }

    if (f & (TEXT_INTERNALLINE|TEXT_TILELINE))
    {
        char line = 'A'; // this is subject to change as an implementation detail
        if (f & TEXT_TILELINE)
            line = '\x7F'; // tile after '~'
        uint32_t const tile = G_GetStringTileASCII(tilefontPtr, font, line, f);
        Bassert(tile < MAXTILES);

        empty.y += tilesiz[tile].y << 16;
    }
}

void G_PrintGameText(int32_t tile, int32_t x, int32_t y, const char *t,
                     int32_t s, int32_t p, int32_t o,
                     int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                     int32_t z, int32_t a)
{
    int32_t f = TEXT_CONSTWIDTHNUMS;

    if (t == NULL)
        return;

    if (!(o & ROTATESPRITE_FULL16))
    {
        x <<= 16;
        y <<= 16;
    }

    if (x == (160<<16))
        f |= TEXT_XCENTER;

    G_ScreenText(tile, x, y, z, 0, 0, t, s, p, 2|o, a, MF_Bluefont.emptychar.x, MF_Bluefont.emptychar.y, MF_Bluefont.between.x, MF_Bluefont.between.y, MF_Bluefont.textflags|f, x1, y1, x2, y2);
}

vec2_t gametext_(int32_t x, int32_t y, const char *t, int32_t s, int32_t p, int32_t o, int32_t a, int32_t f)
{
    return G_ScreenText(MF_Bluefont.tilenum, x, y, MF_Bluefont.zoom, 0, 0, t, s, p, o|g_textstat, a, MF_Bluefont.emptychar.x, MF_Bluefont.emptychar.y, MF_Bluefont.between.x, MF_Bluefont.between.y, MF_Bluefont.textflags|f, 0, 0, xdim-1, ydim-1);
}
void gametext_simple(int32_t x, int32_t y, const char *t)
{
    G_ScreenText(MF_Bluefont.tilenum, x, y, MF_Bluefont.zoom, 0, 0, t, 0, MF_Bluefont.pal, g_textstat, 0, MF_Bluefont.emptychar.x, MF_Bluefont.emptychar.y, MF_Bluefont.between.x, MF_Bluefont.between.y, MF_Bluefont.textflags, 0, 0, xdim-1, ydim-1);
}
vec2_t mpgametext(int32_t x, int32_t y, const char *t, int32_t s, int32_t o, int32_t a, int32_t f)
{
    return G_ScreenText(MF_Bluefont.tilenum, x, y, textsc(MF_Bluefont.zoom), 0, 0, t, s, MF_Bluefont.pal, o|g_textstat, a, MF_Bluefont.emptychar.x, MF_Bluefont.emptychar.y, MF_Bluefont.between.x, MF_Bluefont.between.y, MF_Bluefont.textflags|f, 0, 0, xdim-1, ydim-1);
}
vec2_t mpgametextsize(const char *t, int32_t f)
{
    return G_ScreenTextSize(MF_Bluefont.tilenum, 0, 0, textsc(MF_Bluefont.zoom), 0, t, g_textstat, MF_Bluefont.emptychar.x, MF_Bluefont.emptychar.y, MF_Bluefont.between.x, MF_Bluefont.between.y, MF_Bluefont.textflags|f, 0, 0, xdim-1, ydim-1);
}

// minitext_yofs: in hud_scale-independent, (<<16)-scaled, 0-200-normalized y coords,
// (sb&ROTATESPRITE_MAX) only.
int32_t minitext_yofs = 0;
int32_t minitext_lowercase = 0;
int32_t minitext_(int32_t x, int32_t y, const char *t, int32_t s, int32_t p, int32_t sb)
{
    vec2_t dim;
    int32_t z = MF_Minifont.zoom;

    if (t == NULL)
    {
        OSD_Printf("minitext: NULL text!\n");
        return 0;
    }

    if (!(sb & ROTATESPRITE_FULL16))
    {
        x<<=16;
        y<<=16;
    }

    if (sb & ROTATESPRITE_MAX)
    {
        x = sbarx16(x);
        y = minitext_yofs+sbary16(y);
        z = sbarsc(z);
    }

    sb &= (ROTATESPRITE_MAX-1)|RS_CENTERORIGIN;

    dim = G_ScreenText(MF_Minifont.tilenum, x, y, z, 0, 0, t, s, p, sb, 0, MF_Minifont.emptychar.x, MF_Minifont.emptychar.y, MF_Minifont.between.x, MF_Minifont.between.y, MF_Minifont.textflags, 0, 0, xdim-1, ydim-1);

    x += dim.x;

    if (!(sb & ROTATESPRITE_FULL16))
        x >>= 16;

    return x;
}

void menutext_(int32_t x, int32_t y, int32_t s, char const *t, int32_t o, int32_t f)
{
    G_ScreenText(MF_Redfont.tilenum, x, y - (12<<16), MF_Redfont.zoom, 0, 0, t, s, MF_Redfont.pal, o, 0, MF_Redfont.emptychar.x, MF_Redfont.emptychar.y, MF_Redfont.between.x, MF_Redfont.between.y, f|MF_Redfont.textflags|TEXT_LITERALESCAPE, 0, 0, xdim-1, ydim-1);
}

void captionmenutext(int32_t x, int32_t y, char const *t)
{
    G_ScreenText(MF_Redfont.tilenum, x, y - (12<<16), MF_Redfont.zoom, 0, 0, t, 0, ud.menutitle_pal, g_textstat, 0, MF_Redfont.emptychar.x, MF_Redfont.emptychar.y, MF_Redfont.between.x, MF_Redfont.between.y, MF_Redfont.textflags|TEXT_LITERALESCAPE|TEXT_XCENTER|TEXT_YCENTER, 0, 0, xdim-1, ydim-1);
}


int32_t user_quote_time[MAXUSERQUOTES];
static char user_quote[MAXUSERQUOTES][178];

void G_AddUserQuote(const char *daquote)
{
    int32_t i;

    for (i=MAXUSERQUOTES-1; i>0; i--)
    {
        Bstrcpy(user_quote[i], user_quote[i-1]);
        user_quote_time[i] = user_quote_time[i-1];
    }
    Bstrcpy(user_quote[0], daquote);
    OSD_Printf("%s\n", daquote);

    user_quote_time[0] = ud.msgdisptime;
    pub = NUMPAGES;
}

int32_t textsc(int32_t sc)
{
    return scale(sc, ud.textscale, 400);
}

int32_t hud_glowingquotes = 1;

#define FTAOPAQUETIME 30

// alpha increments of 8 --> 256 / 8 = 32 --> round up to power of 2 --> 32 --> divide by 2 --> 16 alphatabs required
static inline int32_t textsh(uint32_t t)
{
    return (hud_glowingquotes && ((videoGetRenderMode() == REND_CLASSIC && numalphatabs < 15) || t >= FTAOPAQUETIME))
        ? sintable[(t << 7) & 2047] >> 11
        : (sintable[(FTAOPAQUETIME << 7) & 2047] >> 11);
}

// orientation flags depending on time that a quote has still to be displayed
static inline int32_t texto(int32_t t)
{
    if (videoGetRenderMode() != REND_CLASSIC || numalphatabs >= 15 || t > 4)
        return 0;

    if (t > 2)
        return 1;

    return 1|32;
}

static inline int32_t texta(int32_t t)
{
    if (videoGetRenderMode() == REND_CLASSIC && numalphatabs < 15)
        return 0;

    return 255 - clamp(t<<3, 0, 255);
}

static FORCE_INLINE int32_t text_ypos(void)
{
    if (ud.hudontop == 1 && ud.screen_size == 4 && ud.althud == 1)
        return 32<<16;

#ifdef GEKKO
    return 16<<16;
#elif defined EDUKE32_TOUCH_DEVICES
    return 24<<16;
#else
    return 1<<16;
#endif
}

// this handles both multiplayer and item pickup message type text
// both are passed on to gametext
void G_PrintGameQuotes(int32_t snum)
{
    auto const ps = g_player[snum].ps;
    const int32_t reserved_quote = (ps->ftq >= QUOTE_RESERVED && ps->ftq <= QUOTE_RESERVED3);
    // NOTE: QUOTE_RESERVED4 is not included.

    int32_t const ybase = (fragbarheight()<<16) + text_ypos();
    int32_t height = 0;
    int32_t k = ps->fta;


    // primary quote

    do
    {
        if (k <= 1)
            break;

        if (EDUKE32_PREDICT_FALSE(apStrings[ps->ftq] == NULL))
        {
            OSD_Printf(OSD_ERROR "%s %d null quote %d\n", "text:", __LINE__, ps->ftq);
            break;
        }

        int32_t y = ybase;
        if (reserved_quote)
        {
#ifdef SPLITSCREEN_MOD_HACKS
            if (!g_fakeMultiMode)
                y = 140<<16;
            else
                y = 70<<16;
#else
            y = 140<<16;
#endif
        }

        int32_t pal = 0;
        int32_t x = 160<<16;

#ifdef SPLITSCREEN_MOD_HACKS
        if (g_fakeMultiMode)
        {
            pal = g_player[snum].pcolor;
            const int32_t sidebyside = ud.screen_size != 0;

            if (sidebyside)
                x = snum == 1 ? 240<<16 : 80<<16;
            else if (snum == 1)
                y += 100<<16;
        }
#endif

        height = gametext_(x, y, apStrings[ps->ftq], textsh(k), pal, texto(k), texta(k), TEXT_XCENTER).y + (1<<16);
    }
    while (0);


    // userquotes

    int32_t y = ybase;

    if (k > 1 && !reserved_quote)
        y += k <= 8 ? (height * (k-1))>>3 : height;

    for (int i = 0; i < MAXUSERQUOTES; i++)
    {
        k = user_quote_time[i];

        if (k <= 0)
            continue;

        // int32_t const sh = hud_glowingquotes ? sintable[((totalclock+(i<<2))<<5)&2047]>>11 : 0;

        // could use some kind of word wrap here
        height = mpgametext(mpgametext_x, y, user_quote[i], textsh(k), texto(k), texta(k), 0).y + textsc(1<<16);
        y += k <= 4 ? (height * (k-1))>>2 : height;
    }
}

void P_DoQuote(int32_t q, DukePlayer_t *p)
{
    int32_t cq = 0;

    if (ud.fta_on == 0 || q < 0 || !(p->gm & MODE_GAME))
        return;

    if (q & MAXQUOTES)
    {
        cq = 1;
        q &= ~MAXQUOTES;
    }

    if (EDUKE32_PREDICT_FALSE(apStrings[q] == NULL))
    {
        OSD_Printf(OSD_ERROR "%s %d null quote %d\n", "text:", __LINE__, q);
        return;
    }

    if (p->fta > 0 && q != QUOTE_RESERVED && q != QUOTE_RESERVED2)
        if (p->ftq == QUOTE_RESERVED || p->ftq == QUOTE_RESERVED2) return;

    p->fta = 100;

    if (p->ftq != q)
    {
        if (p == g_player[screenpeek].ps && apStrings[q][0] != '\0')
            OSD_Printf(cq ? OSDTEXT_DEFAULT "%s\n" : "%s\n", apStrings[q]);

        p->ftq = q;
    }

    pub = NUMPAGES;
    pus = NUMPAGES;
}
