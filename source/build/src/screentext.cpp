/*
 * screentext.cpp
 *
 * Copyright © 2020, Evan Ramos. All rights reserved.
 */

#include "screentext.h"
#include "build.h"

static inline void SetIfGreater(int32_t *variable, int32_t potentialValue)
{
    if (potentialValue > *variable)
        *variable = potentialValue;
}

// get the string length until the next '\n'
static inline int32_t GetStringLineLength(char const * const start, char const * const end)
{
    char const * text = start;

    while (text < end && *text != '\n')
        ++text;

    return text - start;
}

static inline int32_t GetStringNumLines(char const * text, char const * const end)
{
    int32_t count = 1;

    while (text < end)
    {
        if (*text == '\n')
            ++count;
        ++text;
    }

    return count;
}
// Note: Neither of these care about TEXT_LINEWRAP. This is intended.

// This function requires you to Xfree() the returned char*.
static char * GetSubString(char const * text, char const * const end, int32_t const length)
{
    auto line = (char *)Xmalloc((length+1) * sizeof(char));
    int32_t counter = 0;

    while (counter < length && text < end)
    {
        line[counter] = *text;

        ++text;
        ++counter;
    }

    line[counter] = '\0';

    return line;
}

#define CONSTWIDTHNUMS(f, t) (((f) & TEXT_CONSTWIDTHNUMS) && (t) >= '0' && (t) <= '9')

#define LINEWRAP_MARGIN 14

static getstringtile_t GetStringTile;

void screentextSetStringTile(getstringtile_t func)
{
    GetStringTile = func;
}

// qstrdim
vec2_t screentextGetSize(ScreenTextSize_t const & data)
{
    if (data.str == NULL)
        return {};

    // optimization: justification in both directions
    if ((data.f & TEXT_XJUSTIFY) && (data.f & TEXT_YJUSTIFY))
        return data.between;

    char const * text = data.str;
    char const * const end = Bstrchr(data.str, '\0');

    vec2_t size{}; // eventually the return value
    vec2_t pos{}; // holds the coordinate position as we draw each character tile of the string
    vec2_t extent{}; // holds the x-width of each character and the greatest y-height of each line
    vec2_t offset{}; // temporary; holds the last movement made in both directions

    // handle zooming where applicable
    int32_t xspace = mulscale16(data.empty.x, data.zoom);
    int32_t yline = mulscale16(data.empty.y, data.zoom);
    int32_t xbetween = mulscale16(data.between.x, data.zoom);
    int32_t ybetween = mulscale16(data.between.y, data.zoom);
    // size/width/height/spacing/offset values should be multiplied or scaled by zoom (since 100% is 65536, the same as 1<<16)

    int32_t tile;
    char t;

    // loop through the string
    while (text < end && (t = *text))
    {
        // handle escape sequences
        if (t == '^' && Bisdigit(*(text + 1)) && !(data.f & TEXT_LITERALESCAPE))
        {
            text += 2;
            if (Bisdigit(*text))
                ++text;
            continue;
        }

        // handle case bits
        if (data.f & TEXT_UPPERCASE)
        {
            if (data.f & TEXT_INVERTCASE) // optimization...?
            { // v^ important that these two ifs remain separate due to the else below
                if (Bisupper(t))
                    t = Btolower(t);
            }
            else if (Bislower(t))
                t = Btoupper(t);
        }
        else if (data.f & TEXT_INVERTCASE)
        {
            if (Bisupper(t))
                t = Btolower(t);
            else if (Bislower(t))
                t = Btoupper(t);
        }

        // translate the character to a tilenum
        tile = GetStringTile(data.font, &t, data.f);

        // reset this here because we haven't printed anything yet this loop
        extent.x = 0;

        // reset this here because the act of printing something on this line means that we include the margin above in the total size
        offset.y = 0;

        // handle each character itself in the context of screen drawing
        switch (t)
        {
        case '\t':
        case ' ':
            // width
            extent.x = xspace;

            if (data.f & (TEXT_INTERNALSPACE|TEXT_TILESPACE))
            {
                char space = '.'; // this is subject to change as an implementation detail
                if (data.f & TEXT_TILESPACE)
                    space = '\x7F'; // tile after '~'
                tile = GetStringTile(data.font, &space, data.f);

                extent.x += (tilesiz[tile].x * data.zoom);
            }

            // prepare the height // near-CODEDUP the other two near-CODEDUPs for this section
            {
                int32_t tempyextent = yline;

                if (data.f & (TEXT_INTERNALLINE|TEXT_TILELINE))
                {
                    char line = 'A'; // this is subject to change as an implementation detail
                    if (data.f & TEXT_TILELINE)
                        line = '\x7F'; // tile after '~'
                    tile = GetStringTile(data.font, &line, data.f);

                    tempyextent += tilesiz[tile].y * data.zoom;
                }

                SetIfGreater(&extent.y, tempyextent);
            }

            if (t == '\t')
                extent.x <<= 2; // *= 4

            break;

        case '\n': // near-CODEDUP "if (wrap)"
            extent.x = 0;

            // save the position
            if (!(data.f & TEXT_XOFFSETZERO)) // we want the entire offset to count as the character width
                pos.x -= offset.x;
            SetIfGreater(&size.x, pos.x);

            // reset the position
            pos.x = 0;

            // prepare the height
            {
                int32_t tempyextent = yline;

                if (data.f & (TEXT_INTERNALLINE|TEXT_TILELINE))
                {
                    char line = 'A'; // this is subject to change as an implementation detail
                    if (data.f & TEXT_TILELINE)
                        line = '\x7F'; // tile after '~'
                    tile = GetStringTile(data.font, &line, data.f);

                    tempyextent += tilesiz[tile].y * data.zoom;
                }

                SetIfGreater(&extent.y, tempyextent);
            }

            // move down the line height
            if (!(data.f & TEXT_YOFFSETZERO))
                pos.y += extent.y;

            // reset the current height
            extent.y = 0;

            // line spacing
            offset.y = (data.f & TEXT_YJUSTIFY) ? 0 : ybetween; // ternary to prevent overflow
            pos.y += offset.y;

            break;

        default:
            // width
            extent.x = tilesiz[tile].x * data.zoom;

            if (CONSTWIDTHNUMS(data.f, t))
            {
                char numeral = '0'; // this is subject to change as an implementation detail
                extent.x = (tilesiz[GetStringTile(data.font, &numeral, data.f)].x-1) * data.zoom;
            }

            // height
            SetIfGreater(&extent.y, (tilesiz[tile].y * data.zoom));

            break;
        }

        // incrementing the coordinate counters
        offset.x = 0;

        // advance the x coordinate
        if (!(data.f & TEXT_XOFFSETZERO) || CONSTWIDTHNUMS(data.f, t))
            offset.x += extent.x;

        // account for text spacing
        if (!CONSTWIDTHNUMS(data.f, t)
            && t != '\n'
            && !(data.f & TEXT_XJUSTIFY)) // to prevent overflow
            offset.x += xbetween;

        // line wrapping
        if ((data.f & TEXT_LINEWRAP) && !(data.f & TEXT_XRIGHT) && !(data.f & TEXT_XCENTER) && data.blockangle % 512 == 0)
        {
            int32_t wrap = 0;
            const int32_t ang = data.blockangle % 2048;

            // this is the only place in qstrdim where angle actually affects direction, but only in the wrapping measurement
            switch (ang)
            {
            case 0:
                wrap = (data.pos.x + (pos.x + offset.x) > ((data.o & 2) ? (320<<16) : ((data.b2.x - LINEWRAP_MARGIN)<<16)));
                break;
            case 512:
                wrap = (data.pos.y + (pos.x + offset.x) > ((data.o & 2) ? (200<<16) : ((data.b2.y - LINEWRAP_MARGIN)<<16)));
                break;
            case 1024:
                wrap = (data.pos.x - (pos.x + offset.x) < ((data.o & 2) ? 0 : ((data.b1.x + LINEWRAP_MARGIN)<<16)));
                break;
            case 1536:
                wrap = (data.pos.y - (pos.x + offset.x) < ((data.o & 2) ? 0 : ((data.b1.y + LINEWRAP_MARGIN)<<16)));
                break;
            }
            if (wrap) // near-CODEDUP "case '\n':"
            {
                // save the position
                SetIfGreater(&size.x, pos.x);

                // reset the position
                pos.x = 0;

                // prepare the height
                {
                    int32_t tempyextent = yline;

                    if (data.f & (TEXT_INTERNALLINE|TEXT_TILELINE))
                    {
                        char line = 'A'; // this is subject to change as an implementation detail
                        if (data.f & TEXT_TILELINE)
                            line = '\x7F'; // tile after '~'
                        tile = GetStringTile(data.font, &line, data.f);

                        tempyextent += tilesiz[tile].y * data.zoom;
                    }

                    SetIfGreater(&extent.y, tempyextent);
                }

                // move down the line height
                if (!(data.f & TEXT_YOFFSETZERO))
                    pos.y += extent.y;

                // reset the current height
                extent.y = 0;

                // line spacing
                offset.y = (data.f & TEXT_YJUSTIFY) ? 0 : ybetween; // ternary to prevent overflow
                pos.y += offset.y;
            }
            else
                pos.x += offset.x;
        }
        else
            pos.x += offset.x;

        // save some trouble with calculation in case the line breaks
        if (!(data.f & TEXT_XOFFSETZERO) || CONSTWIDTHNUMS(data.f, t))
            offset.x -= extent.x;

        // iterate to the next character in the string
        ++text;
    }

    // calculate final size
    if (!(data.f & TEXT_XOFFSETZERO))
        pos.x -= offset.x;

    if (!(data.f & TEXT_YOFFSETZERO))
    {
        pos.y -= offset.y;
        pos.y += extent.y;
    }
    else
        pos.y += ybetween;

    SetIfGreater(&size.x, pos.x);
    SetIfGreater(&size.y, pos.y);

    // justification where only one of the two directions is set, so we have to iterate
    if (data.f & TEXT_XJUSTIFY)
        size.x = xbetween;
    if (data.f & TEXT_YJUSTIFY)
        size.y = ybetween;

    return size;
}

static inline void AddCoordsFromRotation(vec2_t *coords, const vec2_t *unitDirection, const int32_t magnitude)
{
    coords->x += mulscale14(magnitude, unitDirection->x);
    coords->y += mulscale14(magnitude, unitDirection->y);
}

// screentext
vec2_t screentextRender(ScreenText_t const & data)
{
    if (data.str == NULL)
        return {};

    char const * text = data.str;
    char const * const end = Bstrchr(data.str, '\0');

    int32_t o = data.o;
    // eliminate conflicts, necessary here to get the correct size value
    // especially given justification's special handling in screentextGetSize()
    if ((data.f & TEXT_XRIGHT) || (data.f & TEXT_XCENTER) || (data.f & TEXT_XJUSTIFY) || (data.f & TEXT_YJUSTIFY) || data.blockangle % 512 != 0)
        o &= ~TEXT_LINEWRAP;

    ScreenTextSize_t sizedata{data.size};
    sizedata.f &= ~(TEXT_XJUSTIFY|TEXT_YJUSTIFY);
    if (data.f & TEXT_XJUSTIFY)
        sizedata.between.x = 0;
    if (data.f & TEXT_YJUSTIFY)
        sizedata.between.y = 0;
    sizedata.o = o;

    vec2_t size = screentextGetSize(sizedata); // eventually the return value, and we need it for alignment
    vec2_t origin{}; // where to start, depending on the alignment
    vec2_t pos{}; // holds the coordinate position as we draw each character tile of the string
    vec2_t extent{}; // holds the x-width of each character and the greatest y-height of each line
    const vec2_t Xdirection = { sintable[(data.blockangle+512)&2047], sintable[data.blockangle&2047], };
    const vec2_t Ydirection = { sintable[(data.blockangle+1024)&2047], sintable[(data.blockangle+512)&2047], };

    // handle zooming where applicable
    int32_t xspace = mulscale16(data.empty.x, data.zoom);
    int32_t yline = mulscale16(data.empty.y, data.zoom);
    int32_t xbetween = mulscale16(data.between.x, data.zoom);
    int32_t ybetween = mulscale16(data.between.x, data.zoom);
    // size/width/height/spacing/offset values should be multiplied or scaled by zoom (since 100% is 65536, the same as 1<<16)

    int32_t alpha = data.alpha, blendidx = 0;
    NEG_ALPHA_TO_BLEND(alpha, blendidx, o);
    uint8_t pal = data.pal;

    int32_t tile;
    char t;

    // alignment
    // near-CODEDUP "case '\n':"
    {
        int32_t lines = GetStringNumLines(text, end);

        if ((data.f & TEXT_XJUSTIFY) || (data.f & TEXT_XRIGHT) || (data.f & TEXT_XCENTER))
        {
            int32_t const length = GetStringLineLength(text, end);

            int32_t linewidth = size.x;

            if (lines != 1)
            {
                char * const line = GetSubString(text, end, length);

                sizedata.str = line;
                linewidth = screentextGetSize(sizedata).x;

                Xfree(line);
            }

            if (data.f & TEXT_XJUSTIFY)
            {
                size.x = xbetween;

                xbetween = (length == 1) ? 0 : tabledivide32_noinline((xbetween - linewidth), (length - 1));

                linewidth = size.x;
            }

            if (data.f & TEXT_XRIGHT)
                origin.x = -(linewidth/data.zoom*data.zoom);
            else if (data.f & TEXT_XCENTER)
                origin.x = -(linewidth/2/data.zoom*data.zoom);
        }

        if (data.f & TEXT_YJUSTIFY)
        {
            const int32_t tempswap = ybetween;
            ybetween = (lines == 1) ? 0 : tabledivide32_noinline(ybetween - size.y, lines - 1);
            size.y = tempswap;
        }

        if (data.f & TEXT_YBOTTOM)
            origin.y = -(size.y/data.zoom*data.zoom);
        else if (data.f & TEXT_YCENTER)
            origin.y = -(size.y/2/data.zoom*data.zoom);
    }

    // loop through the string
    while (text < end && (t = *text))
    {
        int32_t angle = data.blockangle + data.charangle;

        // handle escape sequences
        if (t == '^' && Bisdigit(*(text + 1)) && !(data.f & TEXT_LITERALESCAPE))
        {
            char smallbuf[4];

            ++text;
            smallbuf[0] = *text;

            ++text;
            if (Bisdigit(*text))
            {
                smallbuf[1] = *text;
                smallbuf[2] = '\0';
                ++text;
            }
            else
                smallbuf[1] = '\0';

            if (!(data.f & TEXT_IGNOREESCAPE))
                pal = Batoi(smallbuf);

            continue;
        }

        // handle case bits
        if (data.f & TEXT_UPPERCASE)
        {
            if (data.f & TEXT_INVERTCASE) // optimization...?
            { // v^ important that these two ifs remain separate due to the else below
                if (Bisupper(t))
                    t = Btolower(t);
            }
            else if (Bislower(t))
                t = Btoupper(t);
        }
        else if (data.f & TEXT_INVERTCASE)
        {
            if (Bisupper(t))
                t = Btolower(t);
            else if (Bislower(t))
                t = Btoupper(t);
        }

        // translate the character to a tilenum
        tile = GetStringTile(data.font, &t, data.f);

        switch (t)
        {
        case '\t':
        case ' ':
        case '\n':
        case '\x7F':
            break;

        default:
        {
            vec2_t location{data.pos};

            AddCoordsFromRotation(&location, &Xdirection, origin.x);
            AddCoordsFromRotation(&location, &Ydirection, origin.y);

            AddCoordsFromRotation(&location, &Xdirection, pos.x);
            AddCoordsFromRotation(&location, &Ydirection, pos.y);

            rotatesprite_(location.x, location.y, data.zoom, angle, tile, data.shade, pal, o, alpha, blendidx, data.b1.x, data.b1.y, data.b2.x, data.b2.y);

            break;
        }
        }

        // reset this here because we haven't printed anything yet this loop
        extent.x = 0;

        // handle each character itself in the context of screen drawing
        switch (t)
        {
        case '\t':
        case ' ':
            // width
            extent.x = xspace;

            if (data.f & (TEXT_INTERNALSPACE|TEXT_TILESPACE))
            {
                char space = '.'; // this is subject to change as an implementation detail
                if (data.f & TEXT_TILESPACE)
                    space = '\x7F'; // tile after '~'
                tile = GetStringTile(data.font, &space, data.f);

                extent.x += (tilesiz[tile].x * data.zoom);
            }

            // prepare the height // near-CODEDUP the other two near-CODEDUPs for this section
            {
                int32_t tempyextent = yline;

                if (data.f & (TEXT_INTERNALLINE|TEXT_TILELINE))
                {
                    char line = 'A'; // this is subject to change as an implementation detail
                    if (data.f & TEXT_TILELINE)
                        line = '\x7F'; // tile after '~'
                    tile = GetStringTile(data.font, &line, data.f);

                    tempyextent += tilesiz[tile].y * data.zoom;
                }

                SetIfGreater(&extent.y, tempyextent);
            }

            if (t == '\t')
                extent.x <<= 2; // *= 4

            break;

        case '\n': // near-CODEDUP "if (wrap)"
            extent.x = 0;

            // reset the position
            pos.x = 0;

            // prepare the height
            {
                int32_t tempyextent = yline;

                if (data.f & (TEXT_INTERNALLINE|TEXT_TILELINE))
                {
                    char line = 'A'; // this is subject to change as an implementation detail
                    if (data.f & TEXT_TILELINE)
                        line = '\x7F'; // tile after '~'
                    tile = GetStringTile(data.font, &line, data.f);

                    tempyextent += tilesiz[tile].y * data.zoom;
                }

                SetIfGreater(&extent.y, tempyextent);
            }

            // move down the line height
            if (!(data.f & TEXT_YOFFSETZERO))
                pos.y += extent.y;

            // reset the current height
            extent.y = 0;

            // line spacing
            pos.y += ybetween;

            // near-CODEDUP "alignments"
            if ((data.f & TEXT_XJUSTIFY) || (data.f & TEXT_XRIGHT) || (data.f & TEXT_XCENTER))
            {
                int32_t const length = GetStringLineLength(text+1, end);

                char * const line = GetSubString(text+1, end, length);

                sizedata.str = line;
                int32_t linewidth = screentextGetSize(sizedata).x;

                Xfree(line);

                if (data.f & TEXT_XJUSTIFY)
                {
                    xbetween = (length == 1) ? 0 : tabledivide32_noinline(xbetween - linewidth, length - 1);

                    linewidth = size.x;
                }

                if (data.f & TEXT_XRIGHT)
                    origin.x = -linewidth;
                else if (data.f & TEXT_XCENTER)
                    origin.x = -(linewidth / 2);
            }

            break;

        default:
            // width
            extent.x = tilesiz[tile].x * data.zoom;

            if (CONSTWIDTHNUMS(data.f, t))
            {
                char numeral = '0'; // this is subject to change as an implementation detail
                extent.x = (tilesiz[GetStringTile(data.font, &numeral, data.f)].x-1) * data.zoom;
            }

            // height
            SetIfGreater(&extent.y, (tilesiz[tile].y * data.zoom));

            break;
        }

        // incrementing the coordinate counters
        {
            int32_t xoffset = 0;

            // advance the x coordinate
            if (!(data.f & TEXT_XOFFSETZERO) || CONSTWIDTHNUMS(data.f, t))
                xoffset += extent.x;

            // account for text spacing
            if (!CONSTWIDTHNUMS(data.f, t)
                && t != '\n')
                xoffset += xbetween;

            // line wrapping
            if (data.f & TEXT_LINEWRAP)
            {
                int32_t wrap = 0;
                const int32_t ang = data.blockangle % 2048;

                // it's safe to make some assumptions and not go through AddCoordsFromRotation() since we limit to four directions
                switch (ang)
                {
                case 0:
                    wrap = (data.pos.x + (pos.x + xoffset) > ((o & 2) ? (320<<16) : ((data.b2.x - LINEWRAP_MARGIN)<<16)));
                    break;
                case 512:
                    wrap = (data.pos.y + (pos.x + xoffset) > ((o & 2) ? (200<<16) : ((data.b2.y - LINEWRAP_MARGIN)<<16)));
                    break;
                case 1024:
                    wrap = (data.pos.x - (pos.x + xoffset) < ((o & 2) ? 0 : ((data.b1.x + LINEWRAP_MARGIN)<<16)));
                    break;
                case 1536:
                    wrap = (data.pos.y - (pos.x + xoffset) < ((o & 2) ? 0 : ((data.b1.y + LINEWRAP_MARGIN)<<16)));
                    break;
                }
                if (wrap) // near-CODEDUP "case '\n':"
                {
                    // reset the position
                    pos.x = 0;

                    // prepare the height
                    {
                        int32_t tempyextent = yline;

                        if (data.f & (TEXT_INTERNALLINE|TEXT_TILELINE))
                        {
                            char line = 'A'; // this is subject to change as an implementation detail
                            if (data.f & TEXT_TILELINE)
                                line = '\x7F'; // tile after '~'
                            tile = GetStringTile(data.font, &line, data.f);

                            tempyextent += tilesiz[tile].y * data.zoom;
                        }

                        SetIfGreater(&extent.y, tempyextent);
                    }

                    // move down the line height
                    if (!(data.f & TEXT_YOFFSETZERO))
                        pos.y += extent.y;

                    // reset the current height
                    extent.y = 0;

                    // line spacing
                    pos.y += ybetween;
                }
                else
                    pos.x += xoffset;
            }
            else
                pos.x += xoffset;
        }

        // iterate to the next character in the string
        ++text;
    }

    return size;
}

vec2_t screentextRenderShadow(ScreenText_t const & data, vec2_t shadowpos, int32_t shadowpal)
{
    ScreenText_t shadow{data};
    shadow.pos.x += mulscale16(shadowpos.x, data.zoom);
    shadow.pos.y += mulscale16(shadowpos.y, data.zoom);
    shadow.shade = 127;
    shadow.pal = shadowpal;
    screentextRender(shadow);

    return screentextRender(data);
}
