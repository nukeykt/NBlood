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

using glyph_t = ScreenTextGlyph_t;

// get the string length until the next newline
static inline int32_t GetStringLineLength(glyph_t const * const start, glyph_t const * const end)
{
    glyph_t const * text = start;

    while (text < end && !screentextGlyphIsNewline(*text))
        ++text;

    return text - start;
}

static inline int32_t GetStringNumLines(glyph_t const * text, glyph_t const * const end)
{
    int32_t count = 1;

    while (text < end)
    {
        if (screentextGlyphIsNewline(*text))
            ++count;
        ++text;
    }

    return count;
}

// qstrdim
vec2_t screentextGetSize(ScreenTextSize_t const & data)
{
    if (data.text == NULL)
        return {};

    // optimization: justification in both directions
    if ((data.f & TEXT_XJUSTIFY) && (data.f & TEXT_YJUSTIFY))
        return data.between;

    glyph_t const * text = data.text;
    glyph_t const * const end = data.text + data.len;

    vec2_t size{}; // eventually the return value
    vec2_t pos{}; // holds the coordinate position as we draw each character tile of the string
    vec2_t extent{}; // holds the x-width of each character and the greatest y-height of each line
    vec2_t offset{}; // temporary; holds the last movement made in both directions

    // handle zooming where applicable
    int32_t xspace = mulscale16(data.empty.x, data.zoom);
    int32_t yline = mulscale16(data.empty.y, data.zoom);
    int32_t xbetween = mulscale16(data.between.x, data.zoom);
    int32_t ybetween = mulscale16(data.between.y, data.zoom);
    int32_t constwidth = mulscale16(data.constwidth, data.zoom);
    // size/width/height/spacing/offset values should be multiplied or scaled by zoom (since 100% is 65536, the same as 1<<16)

    glyph_t glyph;
    int constwidthactive = 0;

    // loop through the string
    while (text < end && (glyph = *text))
    {
        // reset this here because we haven't printed anything yet this loop
        extent.x = 0;

        // reset this here because the act of printing something on this line means that we include the margin above in the total size
        offset.y = 0;

        // handle each character itself in the context of screen drawing
        if (screentextGlyphIsControlCode(glyph))
        {
            if (screentextGlyphIsSpace(glyph))
            {
                // width
                extent.x = xspace;

                // prepare the height
                SetIfGreater(&extent.y, yline);
            }
            else if (screentextGlyphIsTab(glyph))
            {
                // width
                extent.x = xspace << 2; // * 4

                // prepare the height
                SetIfGreater(&extent.y, yline);
            }
            else if (screentextGlyphIsNewline(glyph))
            {
                extent.x = 0;

                // save the position
                if (!(data.f & TEXT_XOFFSETZERO)) // we want the entire offset to count as the character width
                    pos.x -= offset.x;
                SetIfGreater(&size.x, pos.x);

                // reset the position
                pos.x = 0;

                // prepare the height
                SetIfGreater(&extent.y, yline);

                // move down the line height
                if (!(data.f & TEXT_YOFFSETZERO))
                    pos.y += extent.y;

                // reset the current height
                extent.y = 0;

                // line spacing
                offset.y = (data.f & TEXT_YJUSTIFY) ? 0 : ybetween; // ternary to prevent overflow
                pos.y += offset.y;
            }
            else if (screentextGlyphIsPalChange(glyph))
            {
                ++text;
                continue;
            }
            else if (screentextGlyphIsConstWidth(glyph))
            {
                constwidthactive = 1;
                ++text;
                continue;
            }
        }
        else
        {
            uint16_t const tile = screentextGlyphGetTile(glyph);

            // width
            extent.x = constwidthactive ? constwidth : tilesiz[tile].x * data.zoom;

            // height
            SetIfGreater(&extent.y, (tilesiz[tile].y * data.zoom));
        }

        // incrementing the coordinate counters
        offset.x = 0;

        // advance the x coordinate
        if (!(data.f & TEXT_XOFFSETZERO) || constwidthactive)
            offset.x += extent.x;

        // account for text spacing
        if (!constwidthactive
            && !screentextGlyphIsNewline(glyph)
            && !(data.f & TEXT_XJUSTIFY)) // to prevent overflow
            offset.x += xbetween;

        pos.x += offset.x;

        // save some trouble with calculation in case the line breaks
        if (!(data.f & TEXT_XOFFSETZERO) || constwidthactive)
            offset.x -= extent.x;

        // iterate to the next character in the string
        ++text;

        // reset at end of loop
        constwidthactive = 0;
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
    if (data.text == NULL)
        return {};

    glyph_t const * text = data.text;
    glyph_t const * const end = data.text + data.len;

    ScreenTextSize_t sizedata{data.size};
    sizedata.f &= ~(TEXT_XJUSTIFY|TEXT_YJUSTIFY);
    if (data.f & TEXT_XJUSTIFY)
        sizedata.between.x = 0;
    if (data.f & TEXT_YJUSTIFY)
        sizedata.between.y = 0;

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
    int32_t constwidth = mulscale16(data.constwidth, data.zoom);
    // size/width/height/spacing/offset values should be multiplied or scaled by zoom (since 100% is 65536, the same as 1<<16)

    int32_t alpha = data.alpha, blendidx = 0, o = data.o;
    NEG_ALPHA_TO_BLEND(alpha, blendidx, o);
    uint8_t pal = data.pal;

    glyph_t glyph;
    int constwidthactive = 0;

    // near-CODEDUP "alignments"
    {
        int32_t lines = GetStringNumLines(text, end);

        if ((data.f & TEXT_XJUSTIFY) || (data.f & TEXT_XRIGHT) || (data.f & TEXT_XCENTER))
        {
            int32_t const length = GetStringLineLength(text, end);

            int32_t linewidth = size.x;

            if (lines != 1)
            {
                sizedata.text = text;
                sizedata.len = length;
                linewidth = screentextGetSize(sizedata).x;
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
    while (text < end && (glyph = *text))
    {
        // reset this here because we haven't printed anything yet this loop
        extent.x = 0;

        int32_t angle = data.blockangle + data.charangle;

        // handle each character itself in the context of screen drawing
        if (screentextGlyphIsControlCode(glyph))
        {
            if (screentextGlyphIsSpace(glyph))
            {
                // width
                extent.x = xspace;

                // prepare the height
                SetIfGreater(&extent.y, yline);
            }
            else if (screentextGlyphIsTab(glyph))
            {
                // width
                extent.x = xspace << 2; // * 4

                // prepare the height
                SetIfGreater(&extent.y, yline);
            }
            else if (screentextGlyphIsNewline(glyph))
            {
                extent.x = 0;

                // reset the position
                pos.x = 0;

                // prepare the height
                SetIfGreater(&extent.y, yline);

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

                    sizedata.text = text+1;
                    sizedata.len = length;
                    int32_t linewidth = screentextGetSize(sizedata).x;

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
            }
            else if (screentextGlyphIsPalChange(glyph))
            {
                pal = screentextGlyphGetPalChange(glyph);
                ++text;
                continue;
            }
            else if (screentextGlyphIsConstWidth(glyph))
            {
                constwidthactive = 1;
                ++text;
                continue;
            }
        }
        else
        {
            uint16_t const tile = screentextGlyphGetTile(glyph);
            vec2_t location{data.pos};

            AddCoordsFromRotation(&location, &Xdirection, origin.x);
            AddCoordsFromRotation(&location, &Ydirection, origin.y);

            AddCoordsFromRotation(&location, &Xdirection, pos.x);
            AddCoordsFromRotation(&location, &Ydirection, pos.y);

            rotatesprite_(location.x, location.y, data.zoom, angle, tile, data.shade, pal, o, alpha, blendidx, data.b1.x, data.b1.y, data.b2.x, data.b2.y);

            // width
            extent.x = constwidthactive ? constwidth : tilesiz[tile].x * data.zoom;

            // height
            SetIfGreater(&extent.y, (tilesiz[tile].y * data.zoom));
        }

        // incrementing the coordinate counters
        int32_t xoffset = 0;

        // advance the x coordinate
        if (!(data.f & TEXT_XOFFSETZERO) || constwidthactive)
            xoffset += extent.x;

        // account for text spacing
        if (!constwidthactive
            && !screentextGlyphIsNewline(glyph))
            xoffset += xbetween;

        pos.x += xoffset;

        // iterate to the next character in the string
        ++text;

        // reset at end of loop
        constwidthactive = 0;
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

#include <string>
#include <unordered_map>

using locale_map_t = std::unordered_map<std::string, std::string>;
static std::unordered_map<std::string, locale_map_t> localeList{{"en", {}}};
static locale_map_t * currentLocale;

LocalePtr_t localeGetPtr(const char * localeName)
{
    locale_map_t & myLocale = localeList[localeName];
    return LocalePtr_t{&myLocale};
}

void localeDefineMapping(LocalePtr_t localePtr, const char * key, const char * val)
{
    auto & myLocale = *(locale_map_t *)localePtr.opaque;
    myLocale[key] = val;
}

void localeMaybeDefineMapping(LocalePtr_t localePtr, const char * key, const char * val)
{
    auto & myLocale = *(locale_map_t *)localePtr.opaque;
    myLocale.emplace(key, val);
}

void localeSetCurrent(const char * localeName)
{
    auto iter = localeList.find(localeName);
    if (iter == localeList.end())
        return;

    locale_map_t & myLocale = iter->second;
    currentLocale = &myLocale;
}

const char * localeLookup(const char * str)
{
    if (currentLocale == nullptr)
        return str;

    auto iter = currentLocale->find(str);
    if (iter == currentLocale->end())
        return str;

    return iter->second.c_str();
}
