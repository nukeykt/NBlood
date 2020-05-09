// Copyright 2020 Nuke.YKT, EDuke32 developers
// Polymost code: Copyright Ken Silverman, Copyright (c) 2018, Alex Dawson
#include "build.h"
#include "colmatch.h"
#include "reality.h"

tileinfo_t rt_tileinfo[RT_TILENUM];
int32_t rt_tilemap[MAXTILES];
intptr_t rt_waloff[RT_TILENUM];
char rt_walock[RT_TILENUM];

float rt_viewhorizang;

extern void (*gloadtile_n64)(int32_t dapic, int32_t dapal, int32_t tintpalnum, int32_t dashade, int32_t dameth, pthtyp* pth, int32_t doalloc);

static bool RT_TileLoad(int16_t tilenum);
static void rt_gloadtile_n64(int32_t dapic, int32_t dapal, int32_t tintpalnum, int32_t dashade, int32_t dameth, pthtyp* pth, int32_t doalloc);

void RT_LoadTiles(void)
{
    const int tileinfoOffset = 0x90bf0;
    Blseek(rt_group, tileinfoOffset, SEEK_SET);
    if (Bread(rt_group, rt_tileinfo, sizeof(rt_tileinfo)) != sizeof(rt_tileinfo))
    {
        initprintf("RT_LoadTiles: file read error");
        return;
    }

    Bmemset(rt_tilemap, -1, sizeof(rt_tilemap));
    Bmemset(rt_waloff, 0, sizeof(rt_waloff));
    Bmemset(rt_walock, 0, sizeof(rt_walock));

    for (int i = 0; i < RT_TILENUM; i++)
    {
        auto &t = rt_tileinfo[i];
        t.fileoff = B_BIG32(t.fileoff);
        t.waloff = B_BIG32(t.waloff);
        t.picanm = B_BIG32(t.picanm);
        t.sizx = B_BIG16(t.sizx);
        t.sizy = B_BIG16(t.sizy);
        t.filesiz = B_BIG16(t.filesiz);
        t.dimx = B_BIG16(t.dimx);
        t.dimy = B_BIG16(t.dimy);
        t.flags = B_BIG16(t.flags);
        t.tile = B_BIG16(t.tile);

        rt_tilemap[t.tile] = i;
        tilesiz[t.tile].x = t.sizx;
        tilesiz[t.tile].y = t.sizy;
        tileConvertAnimFormat(t.tile, t.picanm);
        tileUpdatePicSiz(t.tile);
    }

    rt_tileload_callback = RT_TileLoad;
    gloadtile_n64 = rt_gloadtile_n64;
#if 0
    for (auto& t : rt_tileinfo)
    {
        char *data = (char*)tileCreate(t.tile, t.sizx, t.sizy);
        int bufsize = 0;
        if (t.flags & RT_TILE8BIT)
        {
            bufsize = t.dimx * t.dimy;
        }
        else
        {
            bufsize = (t.dimx * t.dimy) / 2 + 32;
        }
        tileConvertAnimFormat(t.tile, t.picanm);
        char *inbuf = (char*)Xmalloc(t.filesiz);
        char *outbuf = (char*)Xmalloc(bufsize);
        Blseek(rt_group, dataOffset+t.fileoff, SEEK_SET);
        Bread(rt_group, inbuf, t.filesiz);
        if (RNCDecompress(inbuf, outbuf) == -1)
        {
            Bmemcpy(outbuf, inbuf, bufsize);
        }
        Xfree(inbuf);
        if (t.flags & RT_TILE8BIT)
        {
            for (int i = 0; i < t.sizx; i++)
            {
                for (int j = 0; j < t.sizy; j++)
                {
                    int ii = t.dimx - 1 - ((t.sizx - i - 1) * t.dimx) / t.sizx;
                    int jj = t.dimy - 1 - ((t.sizy - j - 1) * t.dimy) / t.sizy;
                    data[i*t.sizy+j] = outbuf[j*t.dimx+i];
                }
            }
        }
        else
        {
            int palremap[16];
            char *pix = outbuf+32;
            for (int i = 0; i < 16; i++)
            {
                int t = (outbuf[i*2+1] << 8) + outbuf[i*2];
                int r = (t >> 11) & 31;
                int g = (t >> 6) & 31;
                int b = (t >> 1) & 31;
                int a = (t >> 0) & 1;
                r = (r << 3) + (r >> 2);
                g = (g << 3) + (g >> 2);
                b = (b << 3) + (b >> 2);
                if (a == 0)
                    palremap[i] = 255;
                else
                {
                    palremap[i] = paletteGetClosestColor(r, g, b);
                }
            }
            for (int i = 0; i < t.sizx; i++)
            {
                for (int j = 0; j < t.sizy; j++)
                {
                    int ii = t.dimx - 1 - ((t.sizx - i - 1) * t.dimx) / t.sizx;
                    int jj = t.dimy - 1 - ((t.sizy - j - 1) * t.dimy) / t.sizy;
                    int ix = jj * t.dimx + ii;
                    char b = pix[ix>>1];
                    if (ix&1)
                        b &= 15;
                    else
                        b = (b >> 4) & 15;
                    data[i*t.sizy+j] = palremap[b];
                }
            }
        }
        Xfree(outbuf);
    }
#endif
}

bool RT_TileLoad(int16_t tilenum)
{
    const int dataOffset = 0xc2270;
    int32_t const tileid = rt_tilemap[tilenum];
    if (tileid < 0)
        return false;
    auto &t = rt_tileinfo[tileid];
    int bufsize = 0;
    if (t.flags & RT_TILE8BIT)
    {
        bufsize = t.dimx * t.dimy;
    }
    else
    {
        bufsize = (t.dimx * t.dimy) / 2 + 32;
    }
    if (rt_waloff[tileid] == 0)
    {
        rt_walock[tileid] = CACHE1D_UNLOCKED;
        g_cache.allocateBlock(&rt_waloff[tileid], bufsize, &rt_walock[tileid]);
    }
    if (!rt_waloff[tileid])
        return false;
    char *inbuf = (char*)Xmalloc(t.filesiz);
    Blseek(rt_group, dataOffset+t.fileoff, SEEK_SET);
    Bread(rt_group, inbuf, t.filesiz);
    if (RNCDecompress(inbuf, (char*)rt_waloff[tileid]) == -1)
    {
        Bmemcpy((char*)rt_waloff[tileid], inbuf, bufsize);
    }
    Xfree(inbuf);

    if (waloff[tilenum])
    {
        char *data = (char*)waloff[tilenum];
        char *src = (char*)rt_waloff[tileid];
        if (t.flags & RT_TILE8BIT)
        {
            for (int i = 0; i < t.sizx; i++)
            {
                for (int j = 0; j < t.sizy; j++)
                {
                    int ii = t.dimx - 1 - ((t.sizx - i - 1) * t.dimx) / t.sizx;
                    int jj = t.dimy - 1 - ((t.sizy - j - 1) * t.dimy) / t.sizy;
                    data[i*t.sizy+j] = src[j*t.dimx+i];
                }
            }
        }
        else
        {
            int palremap[16];
            char *pix = src+32;
            for (int i = 0; i < 16; i++)
            {
                int t = (src[i*2+1] << 8) + src[i*2];
                int r = (t >> 11) & 31;
                int g = (t >> 6) & 31;
                int b = (t >> 1) & 31;
                int a = (t >> 0) & 1;
                r = (r << 3) + (r >> 2);
                g = (g << 3) + (g >> 2);
                b = (b << 3) + (b >> 2);
                if (a == 0)
                    palremap[i] = 255;
                else
                {
                    palremap[i] = paletteGetClosestColor(r, g, b);
                }
            }
            for (int i = 0; i < t.sizx; i++)
            {
                for (int j = 0; j < t.sizy; j++)
                {
                    int ii = t.dimx - 1 - ((t.sizx - i - 1) * t.dimx) / t.sizx;
                    int jj = t.dimy - 1 - ((t.sizy - j - 1) * t.dimy) / t.sizy;
                    int ix = jj * t.dimx + ii;
                    char b = pix[ix>>1];
                    if (ix&1)
                        b &= 15;
                    else
                        b = (b >> 4) & 15;
                    data[i*t.sizy+j] = palremap[b];
                }
            }
        }
    }

    return true;
}
void rt_gloadtile_n64(int32_t dapic, int32_t dapal, int32_t tintpalnum, int32_t dashade, int32_t dameth, pthtyp *pth, int32_t doalloc)
{
    int tileid = rt_tilemap[dapic];
    static int32_t fullbrightloadingpass = 0;
    vec2_16_t const & tsizart = tilesiz[dapic];
    vec2_t siz = { 0, 0 }, tsiz = { 0, 0 };
    int const picdim = tsiz.x*tsiz.y;
    char hasalpha = 0;
    tileinfo_t *tinfo = nullptr;

    if (tileid >= 0)
    {
        tinfo = &rt_tileinfo[tileid];
        tsiz.x = tinfo->dimx;
        tsiz.y = tinfo->dimy;
    }

    if (!glinfo.texnpot)
    {
        for (siz.x = 1; siz.x < tsiz.x; siz.x += siz.x) { }
        for (siz.y = 1; siz.y < tsiz.y; siz.y += siz.y) { }
    }
    else
    {
        if ((tsiz.x|tsiz.y) == 0)
            siz.x = siz.y = 1;
        else
            siz = tsiz;
    }

    coltype *pic = (coltype *)Xmalloc(siz.x*siz.y*sizeof(coltype));

    if (tileid < 0 || !rt_waloff[tileid])
    {
        //Force invalid textures to draw something - an almost purely transparency texture
        //This allows the Z-buffer to be updated for mirrors (which are invalidated textures)
        pic[0].r = pic[0].g = pic[0].b = 0; pic[0].a = 1;
        tsiz.x = tsiz.y = 1; hasalpha = 1;
    }
    else
    {
        int is8bit = (tinfo->flags & RT_TILE8BIT) != 0;
        for (bssize_t y = 0; y < siz.y; y++)
        {
            coltype *wpptr = &pic[y * siz.x];
            int32_t y2 = (y < tsiz.y) ? y : y - tsiz.y;

            for (bssize_t x = 0; x < siz.x; x++, wpptr++)
            {
                int32_t dacol;
                int32_t x2 = (x < tsiz.x) ? x : x-tsiz.x;

                if ((dameth & DAMETH_CLAMPED) && (x >= tsiz.x || y >= tsiz.y)) //Clamp texture
                {
                    wpptr->r = wpptr->g = wpptr->b = wpptr->a = 0;
                    continue;
                }

                if (is8bit)
                {
                    dacol = *(char *)(rt_waloff[tileid]+y2*tsiz.x+x2);
                    dacol = rt_palette[dapal][dacol];
                }
                else
                {
                    int o = y2 * tsiz.x + x2;
                    dacol = *(char *)(rt_waloff[tileid]+32+o/2);
                    if (o&1)
                        dacol &= 15;
                    else
                        dacol >>= 4;
                    if (!(dameth & DAMETH_N64_INTENSIVITY))
                    {
                        dacol = *(uint16_t*)(rt_waloff[tileid]+2*dacol);
                        dacol = B_LITTLE16(dacol);
                    }
                }

                if (dameth & DAMETH_N64_INTENSIVITY)
                {
                    int32_t i = (dacol << 4) | dacol;
                    wpptr->r = wpptr->g = wpptr->b = i;
                    hasalpha = 1;
                }
                else
                {
                    int32_t r = (dacol >> 11) & 31;
                    int32_t g = (dacol >> 6) & 31;
                    int32_t b = (dacol >> 1) & 31;
                    int32_t a = (dacol >> 0) & 1;

                    wpptr->r = (r << 3) + (r >> 2);
                    wpptr->g = (g << 3) + (g >> 2);
                    wpptr->b = (b << 3) + (b >> 2);

                    if (a == 0)
                    {
                        wpptr->a = 0;
                        hasalpha = 1;
                    }
                    else
                        wpptr->a = 255;
                }

#if 0
                bricolor((palette_t *)wpptr, dacol);

                if (tintpalnum >= 0)
                {
                    polytint_t const & tint = hictinting[tintpalnum];
                    polytintflags_t const effect = tint.f;
                    uint8_t const r = tint.r;
                    uint8_t const g = tint.g;
                    uint8_t const b = tint.b;

                    if (effect & HICTINT_GRAYSCALE)
                    {
                        wpptr->g = wpptr->r = wpptr->b = (uint8_t) ((wpptr->r * GRAYSCALE_COEFF_RED) +
                                                                (wpptr->g * GRAYSCALE_COEFF_GREEN) +
                                                                (wpptr->b * GRAYSCALE_COEFF_BLUE));
                    }

                    if (effect & HICTINT_INVERT)
                    {
                        wpptr->b = 255 - wpptr->b;
                        wpptr->g = 255 - wpptr->g;
                        wpptr->r = 255 - wpptr->r;
                    }

                    if (effect & HICTINT_COLORIZE)
                    {
                        wpptr->b = min((int32_t)((wpptr->b) * b) >> 6, 255);
                        wpptr->g = min((int32_t)((wpptr->g) * g) >> 6, 255);
                        wpptr->r = min((int32_t)((wpptr->r) * r) >> 6, 255);
                    }

                    switch (effect & HICTINT_BLENDMASK)
                    {
                        case HICTINT_BLEND_SCREEN:
                            wpptr->b = 255 - (((255 - wpptr->b) * (255 - b)) >> 8);
                            wpptr->g = 255 - (((255 - wpptr->g) * (255 - g)) >> 8);
                            wpptr->r = 255 - (((255 - wpptr->r) * (255 - r)) >> 8);
                            break;
                        case HICTINT_BLEND_OVERLAY:
                            wpptr->b = wpptr->b < 128 ? (wpptr->b * b) >> 7 : 255 - (((255 - wpptr->b) * (255 - b)) >> 7);
                            wpptr->g = wpptr->g < 128 ? (wpptr->g * g) >> 7 : 255 - (((255 - wpptr->g) * (255 - g)) >> 7);
                            wpptr->r = wpptr->r < 128 ? (wpptr->r * r) >> 7 : 255 - (((255 - wpptr->r) * (255 - r)) >> 7);
                            break;
                        case HICTINT_BLEND_HARDLIGHT:
                            wpptr->b = b < 128 ? (wpptr->b * b) >> 7 : 255 - (((255 - wpptr->b) * (255 - b)) >> 7);
                            wpptr->g = g < 128 ? (wpptr->g * g) >> 7 : 255 - (((255 - wpptr->g) * (255 - g)) >> 7);
                            wpptr->r = r < 128 ? (wpptr->r * r) >> 7 : 255 - (((255 - wpptr->r) * (255 - r)) >> 7);
                            break;
                    }
                }
#endif

                //swap r & b so that we deal with the data as BGRA
                uint8_t tmpR = wpptr->r;
                wpptr->r = wpptr->b;
                wpptr->b = tmpR;
            }
        }
    }

    if (doalloc) glGenTextures(1,(GLuint *)&pth->glpic); //# of textures (make OpenGL allocate structure)
    glBindTexture(GL_TEXTURE_2D, pth->glpic);

    // fixtransparency(pic,tsiz,siz,dameth);

#if 0
    if (polymost_want_npotytex(dameth, siz.y) && tsiz.x == siz.x && tsiz.y == siz.y)  // XXX
    {
        const int32_t nextpoty = 1 << ((picsiz[dapic] >> 4) + 1);
        const int32_t ydif = nextpoty - siz.y;
        coltype *paddedpic;

        Bassert(ydif < siz.y);

        paddedpic = (coltype *)Xrealloc(pic, siz.x * nextpoty * sizeof(coltype));

        pic = paddedpic;
        Bmemcpy(&pic[siz.x * siz.y], pic, siz.x * ydif * sizeof(coltype));
        siz.y = tsiz.y = nextpoty;

        npoty = 1;
    }
#endif

    if (!doalloc)
    {
        vec2_t pthSiz2 = pth->siz;
        if (!glinfo.texnpot)
        {
            for (pthSiz2.x = 1; pthSiz2.x < pth->siz.x; pthSiz2.x += pthSiz2.x) { }
            for (pthSiz2.y = 1; pthSiz2.y < pth->siz.y; pthSiz2.y += pthSiz2.y) { }
        }
        else
        {
            if ((pthSiz2.x|pthSiz2.y) == 0)
                pthSiz2.x = pthSiz2.y = 1;
            else
                pthSiz2 = pth->siz;
        }
        if (siz.x > pthSiz2.x ||
            siz.y > pthSiz2.y)
        {
            //POGO: grow our texture to hold the tile data
            doalloc = true;
        }
    }
    uploadtexture(doalloc, siz, GL_BGRA, pic, tsiz,
                    dameth | DAMETH_ARTIMMUNITY |
                    (dapic >= MAXUSERTILES ? (DAMETH_NOTEXCOMPRESS|DAMETH_NODOWNSIZE) : 0) | /* never process these short-lived tiles */
                    (hasalpha ? (DAMETH_HASALPHA|DAMETH_ONEBITALPHA) : 0));

    Xfree(pic);

    polymost_setuptexture(dameth, -1);

    pth->picnum = dapic;
    pth->palnum = dapal;
    pth->shade = dashade;
    pth->effects = 0;
    pth->flags = PTH_N64 | TO_PTH_CLAMPED(dameth) | TO_PTH_NOTRANSFIX(dameth) | (hasalpha*(PTH_HASALPHA|PTH_ONEBITALPHA)) | TO_PTH_N64_INTENSIVITY(dameth);
    pth->hicr = NULL;
    pth->siz = tsiz;
}

GLuint rt_shaderprogram;
GLuint rt_stexsamplerloc = -1;
GLuint rt_stexcombloc = -1;
GLuint rt_stexcomb = 0;
GLuint rt_scolor2loc = 0;
GLfloat rt_scolor2[4] = {
    0.f, 0.f, 0.f, 0.f
};

void RT_SetShader(void)
{
    glUseProgram(rt_shaderprogram);
    rt_stexsamplerloc = glGetUniformLocation(rt_shaderprogram, "s_texture");
    rt_stexcombloc = glGetUniformLocation(rt_shaderprogram, "u_texcomb");
    rt_scolor2loc = glGetUniformLocation(rt_shaderprogram, "u_color2");
    glUniform1i(rt_stexsamplerloc, 0);
    glUniform1f(rt_stexcombloc, rt_stexcomb);
    glUniform4fv(rt_scolor2loc, 1, rt_scolor2);
}

void RT_SetColor2(int r, int g, int b, int a)
{
    rt_scolor2[0] = r / 255.f;
    rt_scolor2[1] = g / 255.f;
    rt_scolor2[2] = b / 255.f;
    rt_scolor2[3] = a / 255.f;
    glUniform4fv(rt_scolor2loc, 1, rt_scolor2);
}

void RT_SetTexComb(int comb)
{
    if (rt_stexcomb != comb)
    {
        rt_stexcomb = comb;
        glUniform1f(rt_stexcombloc, rt_stexcomb);
    }
}

void RT_GLInit(void)
{
    if (videoGetRenderMode() == REND_CLASSIC)
        return;
    const char* const RT_VERTEX_SHADER =
        "#version 110\n\
         \n\
         varying vec4 v_color;\n\
         varying float v_distance;\n\
         \n\
         const float c_zero = 0.0;\n\
         const float c_one  = 1.0;\n\
         \n\
         void main()\n\
         {\n\
            vec4 eyeCoordPosition = gl_ModelViewMatrix * gl_Vertex;\n\
            gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n\
            \n\
            eyeCoordPosition.xyz /= eyeCoordPosition.w;\n\
            gl_TexCoord[0] = gl_MultiTexCoord0;\n\
            \n\
            gl_FogFragCoord = abs(eyeCoordPosition.z);\n\
            //gl_FogFragCoord = clamp((gl_Fog.end-abs(eyeCoordPosition.z))*gl_Fog.scale, c_zero, c_one);\n\
            \n\
            v_color = gl_Color;\n\
         }\n\
         ";
    const char* const RT_FRAGMENT_SHADER =
        "#version 110\n\
         #extension GL_ARB_shader_texture_lod : enable\n\
         \n\
         uniform sampler2D s_texture;\n\
         uniform vec4 u_color2;\n\
         uniform float u_texcomb;\n\
         \n\
         varying vec4 v_color;\n\
         varying float v_distance;\n\
         \n\
         const float c_zero = 0.0;\n\
         const float c_one  = 1.0;\n\
         const float c_two  = 2.0;\n\
         \n\
         void main()\n\
         {\n\
         #ifdef GL_ARB_shader_texture_lod\n\
            //vec4 color = texture2DGradARB(s_texture, gl_TexCoord[0].xy, dFdx(gl_TexCoord[0].xy), dFdy(gl_TexCoord[0].xy));\n\
            vec4 color = texture2D(s_texture, gl_TexCoord[0].xy);\n\
         #else\n\
            vec2 transitionBlend = fwidth(floor(gl_TexCoord[0].xy));\n\
            transitionBlend = fwidth(transitionBlend)+transitionBlend;\n\
            vec2 texCoord = mix(fract(gl_TexCoord[0].xy), abs(c_one-mod(gl_TexCoord[0].xy+c_one, c_two)), transitionBlend);\n\
            vec4 color = texture2D(s_texture, u_texturePosSize.xy+texCoord);\n\
         #endif\n\
            \n\
            vec4 colorcomb;\n\
            colorcomb.rgb = mix(v_color.rgb, u_color2.rgb, color.r);\n\
            colorcomb.a = color.a * v_color.a;\n\
            color.rgb = v_color.rgb * color.rgb;\n\
            \n\
            color.a *= v_color.a;\n\
            \n\
            color = mix(color, colorcomb, u_texcomb);\n\
            \n\
            gl_FragData[0] = color;\n\
         }\n\
         ";

    rt_shaderprogram = glCreateProgram();
    GLuint vertexshaderid = polymost2_compileShader(GL_VERTEX_SHADER, RT_VERTEX_SHADER);
    GLuint fragmentshaderid = polymost2_compileShader(GL_FRAGMENT_SHADER, RT_FRAGMENT_SHADER);
    glAttachShader(rt_shaderprogram, vertexshaderid);
    glAttachShader(rt_shaderprogram, fragmentshaderid);
    glLinkProgram(rt_shaderprogram);
}

static float x_vt = 160.f;
static float y_vt = 120.f;
static float vp_scale = 1.f;
static float rt_globaldepth;

void RT_DisplayTileWorld(float x, float y, float sx, float sy, int16_t picnum, int flags)
{
    int xflip = (flags & 4) != 0;
    int yflip = (flags & 4) != 0;

    sx *= vp_scale;
    sy *= vp_scale;

    float xoff = picanm[picnum].xofs * sx / 6.f;
    if (xflip)
        xoff = -xoff;

    x -= xoff * 2.f;

    float sizx = tilesiz[picnum].x * sx / 6.f;
    float sizy = tilesiz[picnum].y * sy / 6.f;

    if (sizx < 1.f && sizy < 1.f)
        return;

    float x1 = x - sizx;
    float x2 = x + sizx;
    float y1 = y - sizy;
    float y2 = y + sizy;

    if (!waloff[picnum])
        tileLoad(picnum);
    
    int method = DAMETH_CLAMPED | DAMETH_N64 | DAMETH_N64_INTENSIVITY;
    pthtyp *pth = texcache_fetch(picnum, 0, 0, method);

    if (!pth)
        return;

    glBindTexture(GL_TEXTURE_2D, pth->glpic);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glOrtho(0, 320.f, 240.f, 0, -1.f, 1.f);
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0, 0); glVertex3f(x1, y1, rt_globaldepth);
    glTexCoord2f(1, 0); glVertex3f(x2, y1, rt_globaldepth);
    glTexCoord2f(1, 1); glVertex3f(x2, y2, rt_globaldepth);
    glTexCoord2f(0, 1); glVertex3f(x1, y2, rt_globaldepth);
    glEnd();
    glPopMatrix();
}

float rt_sky_color[2][3];

static float rt_globalhoriz;
static float rt_globalposx, rt_globalposy, rt_globalposz;
static float rt_globalang;

void setfxcolor(int a1, int a2, int a3, int a4, int a5, int a6)
{
    glColor4f(a1/255.f, a2/255.f, a3/255.f, 1.f);
    RT_SetColor2(a4, a5, a6, 255);
    RT_SetTexComb(1);
}

void RT_DisplaySky(void)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_FALSE);
    rt_globaldepth = 0.f;
    setfxcolor(rt_sky_color[0][0], rt_sky_color[0][1], rt_sky_color[0][2], rt_sky_color[1][0], rt_sky_color[1][1], rt_sky_color[1][2]);
    RT_DisplayTileWorld(x_vt, y_vt + rt_globalhoriz - 100.f, 52.f, 103.f, 3976, 0);
    RT_SetTexComb(0);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

void RT_DisablePolymost()
{
    RT_SetShader();
    RT_SetTexComb(0);
}

void RT_EnablePolymost()
{
    glDisable(GL_CULL_FACE);
    polymost_resetVertexPointers();
    polymost_setFogEnabled(true);
    polymost_usePaletteIndexing(true);
}

void RT_SetupMatrix(void)
{
    float dx = 512.f * cosf(rt_globalang / (1024.f / fPI));
    float dy = 512.f * sinf(rt_globalang / (1024.f / fPI));
    float dz = -(rt_globalhoriz - 100.f) * 4.f;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    bgluPerspective(60.f, 4.f/3.f, 1.f, 16384.f);
    bgluLookAt(rt_globalposx*2, rt_globalposy*2, rt_globalposz*2, (rt_globalposx*2 + dx), (rt_globalposy*2 + dy), (rt_globalposz*2 + dz), 0.f, 0.f, -1.f);
}

void RT_DrawCeiling(int sectnum)
{
    auto sect = &rt_sector[sectnum];
    RT_SetTexComb(0);
    int method = DAMETH_N64;
    pthtyp *pth = texcache_fetch(sector[sectnum].ceilingpicnum, 0, 0, method);
    if (pth)
        glBindTexture(GL_TEXTURE_2D, pth->glpic);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < sect->ceilingvertexnum * 3; i++)
    {
        auto vtx = rt_sectvtx[sect->ceilingvertexptr+i];
        float x = vtx.x * 2;
        float y = vtx.y * 2;
        float z = getceilzofslope(sectnum, vtx.x * 2, vtx.y * 2) / 16.f;
        glTexCoord2f(vtx.u / 2048.f, vtx.v / 2048.f); glVertex3f(x, y, z);
    }
    glEnd();
}

void RT_DrawFloor(int sectnum)
{
    auto sect = &rt_sector[sectnum];
    RT_SetTexComb(0);
    int method = DAMETH_N64;
    pthtyp *pth = texcache_fetch(sector[sectnum].floorpicnum, 0, 0, method);
    if (pth)
        glBindTexture(GL_TEXTURE_2D, pth->glpic);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < sect->floorvertexnum * 3; i++)
    {
        auto vtx = rt_sectvtx[sect->floorvertexptr+i];
        float x = vtx.x * 2;
        float y = vtx.y * 2;
        float z = getflorzofslope(sectnum, vtx.x * 2, vtx.y * 2) / 16.f;
        glTexCoord2f(vtx.u / 2048.f, vtx.v / 2048.f); glVertex3f(x, y, z);
    }
    glEnd();
}

void RT_DrawRooms(int x, int y, int z, fix16_t ang, fix16_t horiz, int16_t sectnum)
{
    RT_DisablePolymost();
#if 0
    // Test code
    int32_t method = 0;
    pthtyp* testpth = texcache_fetch(26, 0, 0, method);
    glBindTexture(GL_TEXTURE_2D, testpth->glpic);
    glViewport(0, 0, xdim, ydim);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 320.f, 240.f, 0, -1.f, 1.f);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glColor3f(1, 1, 1);
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(1, 0); glVertex2f(96.f, 0);
    glTexCoord2f(1, 1); glVertex2f(96.f, 40.f);
    glTexCoord2f(0, 1); glVertex2f(0, 40.f);
    glEnd();
#endif
    
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    rt_globalposx = x * 0.5f;
    rt_globalposy = y * 0.5f;
    rt_globalposz = z * (1.f/32.f);
    rt_globalhoriz = fix16_to_float(horiz);
    rt_globalang = fix16_to_float(ang);
    RT_SetupMatrix();
    RT_DisplaySky();

    glColor4f(1.f, 1.f, 1.f, 1.f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    for (int i = 0; i < numsectors; i++)
    {
        RT_DrawCeiling(i);
        RT_DrawFloor(i);
    }

    RT_EnablePolymost();
}

