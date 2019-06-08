#include "compat.h"
#include "build.h"
#include "common_game.h"
#include "gfx.h"

int gColor;

void Video_SetPixel(int page, int x, int y)
{
    UNREFERENCED_PARAMETER(page);
#ifdef USE_OPENGL
    if (videoGetRenderMode() >= REND_POLYMOST)
        return;
#endif
    videoBeginDrawing();
    char* dest = (char*)frameplace+ylookup[y]+x;
    *dest = (char)gColor;
    videoEndDrawing();
}

void Video_HLine(int page, int y, int x0, int x1)
{
    UNREFERENCED_PARAMETER(page);
#ifdef USE_OPENGL
    if (videoGetRenderMode() >= REND_POLYMOST)
        return;
#endif
    videoBeginDrawing();
    char* dest = (char*)frameplace+ylookup[y]+x0;
    memset(dest, gColor, x1-x0+1);
    videoEndDrawing();
}

void Video_VLine(int page, int x, int y0, int y1)
{
    UNREFERENCED_PARAMETER(page);
#ifdef USE_OPENGL
    if (videoGetRenderMode() >= REND_POLYMOST)
        return;
#endif
    videoBeginDrawing();
    char* dest = (char*)frameplace+ylookup[y0]+x;
    for (int i = 0; i < y1-y0+1; i++, dest += ylookup[1])
        *dest = (char)gColor;
    videoEndDrawing();
}

void Video_FillBox(int page, int x0, int y0, int x1, int y1)
{
    UNREFERENCED_PARAMETER(page);
#ifdef USE_OPENGL
    if (videoGetRenderMode() >= REND_POLYMOST)
        return;
#endif
    videoBeginDrawing();
    char* dest = (char*)frameplace+ylookup[y0]+x0;
    for (int i = 0; i < y1-y0; i++, dest += ylookup[1])
        memset(dest, gColor, x1-x0);
    videoEndDrawing();
}

void Video_BlitM2V(char* src, int bpl, int width, int height, int page, int x, int y)
{
    UNREFERENCED_PARAMETER(page);
#ifdef USE_OPENGL
    if (videoGetRenderMode() >= REND_POLYMOST)
        return;
#endif
    videoBeginDrawing();
    char* dest = (char*)frameplace+ylookup[y]+x;
    int i = height;
    do
    {
        memcpy(dest, src, width);
        src += bpl;
        dest += xdim;
    } while (--i);
    videoEndDrawing();
}

void Video_BlitMT2V(char* src, char tc, int bpl, int width, int height, int page, int x, int y)
{
    UNREFERENCED_PARAMETER(page);
#ifdef USE_OPENGL
    if (videoGetRenderMode() >= REND_POLYMOST)
        return;
#endif
    videoBeginDrawing();
    char* dest = (char*)frameplace+ylookup[y]+x;
    int i = height;
    do
    {
        int j = width;
        do
        {
            if (*src != tc)
                *dest = *src;
            src++;
            dest++;
        } while (--j);
        src += bpl-width;
        dest += xdim-width;
    } while (--i);
    videoEndDrawing();
}

void Video_BlitMono(char *src, char mask, int bpl, int width, int height, int page, int x, int y)
{
    UNREFERENCED_PARAMETER(page);
#ifdef USE_OPENGL
    if (videoGetRenderMode() >= REND_POLYMOST)
        return;
#endif
    videoBeginDrawing();
    char* dest = (char*)frameplace+ylookup[y]+x;
    int i = height;
    do
    {
        int j = width;
        do
        {
            if (*src&mask)
                *dest = (char)gColor;
            src++;
            dest++;
        } while (--j);
        src -= width;
        dest += xdim-width;
        if (mask&0x80)
        {
            mask = 1;
            src += bpl;
        }
        else
            mask <<= 1;
    } while (--i);
    videoEndDrawing();
}

Rect clipRect(0, 0, 320, 200);
int clipX0 = 0, clipY0 = 0, clipX1 = 320, clipY1 = 200;

void gfxDrawBitmap(QBITMAP *qbm, int x, int y)
{
    dassert(qbm != NULL);

    Rect bitmap(x, y, x+qbm->at2, y+qbm->at4);

    bitmap &= clipRect;

    if (!bitmap)
        return;

    Rect bitmap2 = bitmap;

    bitmap2.offset(-x, -y);

    int height = bitmap.height();
    int width = bitmap.width();

    char* p = qbm->atc;

    switch (qbm->at0)
    {
    case 0:
        Video_BlitM2V(p+bitmap2.y0*qbm->at6+bitmap2.x0, qbm->at6, width, height, 0, bitmap.x0, bitmap.y0);
        break;
    case 1:
        Video_BlitMT2V(p+bitmap2.y0*qbm->at6+bitmap2.x0, qbm->at1, qbm->at6, width, height, 0, bitmap.x0, bitmap.y0);
        break;
    }
}

void gfxPixel(int x, int y)
{
    if (clipRect.inside(x, y))
        Video_SetPixel(0, x, y);
}

void gfxHLine(int y, int x0, int x1)
{
    if (y < clipRect.y0 || y >= clipRect.y1)
        return;

    x0 = ClipLow(x0, clipRect.x0);
    x1 = ClipHigh(x1, clipRect.x1-1);
    if (x0 <= x1)
        Video_HLine(0, y, x0, x1);
}

void gfxVLine(int x, int y0, int y1)
{
    if (x < clipRect.x0 || x >= clipRect.x1)
        return;

    y0 = ClipLow(y0, clipRect.y0);
    y1 = ClipHigh(y1, clipRect.y1-1);
    if (y0 <= y1)
        Video_VLine(0, x, y0, y1);
}

void gfxFillBox(int x0, int y0, int x1, int y1)
{
    Rect box(x0, y0, x1, y1);

    box &= clipRect;

    if (!box.isEmpty())
        Video_FillBox(0, box.x0, box.y0, box.x1, box.y1);
}

void gfxSetClip(int x0, int y0, int x1, int y1)
{
    clipRect.x0 = x0;
    clipRect.y0 = y0;
    clipRect.x1 = x1;
    clipRect.y1 = y1;

    clipX0 = x0 << 8;
    clipY0 = y0 << 8;
    clipX1 = (x1 << 8)-1;
    clipY1 = (y1 << 8)-1;
}

extern char textfont[2048];
char* fontTable = textfont;

void printChar(int x, int y, char c)
{
    for (int i = 0; i < 8; i++)
    {
        int mask = 0x80;
        for (int j = 0; j < 8; j++, mask >>= 1)
        {
            char data = fontTable[c*8+i];
            if (data & mask)
                Video_SetPixel(0, x+j, y+i);
        }
    }
}

int gfxGetTextNLen(const char * pzText, QFONT *pFont, int a3)
{
    if (!pFont)
        return strlen(pzText)*8;
    int nLength = -pFont->at11;

    for (const char* s = pzText; *s != 0 && a3 > 0; s++, a3--)
    {
        nLength += pFont->at20[*s].ox+pFont->at11;
    }
    return nLength;
}

int gfxGetLabelLen(const char *pzLabel, QFONT *pFont)
{
    int nLength = 0;
    if (pFont)
        nLength = -pFont->at11;

    for (const char* s = pzLabel; *s != 0; s++)
    {
        if (*s == '&')
            continue;
        if (!pFont)
            nLength += 8;
        else
            nLength += pFont->at20[*s].ox+pFont->at11;
    }
    return nLength;
}

int gfxFindTextPos(const char *pzText, QFONT *pFont, int a3)
{
    if (!pFont)
    {
        return a3 / 8;
    }
    int nLength = -pFont->at11;
    int pos = 0;

    for (const char* s = pzText; *s != 0; s++, pos++)
    {
        nLength += pFont->at20[*s].ox+pFont->at11;
        if (nLength > a3)
            break;
    }
    return pos;
}

void gfxDrawText(int x, int y, int color, const char* pzText, QFONT* pFont)
{
    if (pFont)
        y += pFont->atf;

    gColor = color;

    for (const char* s = pzText; *s != 0; s++)
    {
        if (!pFont)
        {
            Rect rect1(x, y, x+8, y+8);
            if (clipRect.inside(rect1))
                printChar(x, y, *s);
            x += 8;
        }
        else
        {
            QFONTCHAR* pChar = &pFont->at20[*s];
            Rect rect1(x, y+pChar->oy, x+pChar->w, y+pChar->oy+pChar->h);

            rect1 &= clipRect;

            if (!rect1.isEmpty())
            {
                Rect rect2 = rect1;

                rect2.offset(-x, -(y+pChar->oy));

                switch (pFont->at6)
                {
                case 0:
                    Video_BlitMono(&pFont->at820[pChar->offset+(rect2.y0/8)*pChar->w+rect2.x0], 1<<(rect2.y0&7), pChar->w,
                        rect1.x1-rect1.x0, rect1.y1-rect1.y0, 0, rect1.x0, rect1.y0);
                    break;
                case 1:
                    Video_BlitMT2V(&pFont->at820[pChar->offset+rect2.y0*pChar->w+rect2.x0], pFont->at10, pChar->w,
                        rect1.x1-rect1.x0, rect1.y1-rect1.y0, 0, rect1.x0, rect1.y0);
                    break;
                }
            }
            x += pFont->at11 + pChar->ox;
        }
    }
}

void gfxDrawLabel(int x, int y, int color, const char* pzLabel, QFONT* pFont)
{
    if (pFont)
        y += pFont->atf;

    gColor = color;

    char v4 = 0;

    for (const char* s = pzLabel; *s != 0; s++)
    {
        if (*s == '&')
        {
            v4 = 1;
            continue;
        }
        if (!pFont)
        {
            Rect rect1(x, y, x+8, y+8);
            if (clipRect.inside(rect1))
            {
                printChar(x, y, *s);
                if (v4)
                    gfxHLine(y+8, x, x+6);
                x += 8;
            }
        }
        else
        {
            QFONTCHAR* pChar = &pFont->at20[*s];
            Rect rect1(x, y+pChar->oy, x+pChar->w, y+pChar->oy+pChar->h);

            rect1 &= clipRect;

            if (!rect1.isEmpty())
            {
                Rect rect2 = rect1;

                rect2.offset(-x, -(y+pChar->oy));

                switch (pFont->at6)
                {
                case 0:
                    Video_BlitMono(&pFont->at820[pChar->offset+(rect2.y0/8)*pChar->w+rect2.x0], 1<<(rect2.y0&7), pChar->w,
                        rect1.x1-rect1.x0, rect1.y1-rect1.y0, 0, rect1.x0, rect1.y0);
                    if (v4)
                        gfxHLine(y+2, x, x+pChar->w-1);
                    break;
                case 1:
                    Video_BlitMT2V(&pFont->at820[pChar->offset+rect2.y0*pChar->w+rect2.x0], pFont->at10, pChar->w,
                        rect1.x1-rect1.x0, rect1.y1-rect1.y0, 0, rect1.x0, rect1.y0);
                    if (v4)
                        gfxHLine(y+2, x, x+pChar->w-1);
                    break;
                }
            }
            x += pFont->at11 + pChar->ox;
        }
        v4 = 0;
    }
}

