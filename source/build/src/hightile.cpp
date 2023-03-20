/*
 * High-colour textures support for Polymost
 * by Jonathon Fowler
 * See the included license file "BUILDLIC.TXT" for license info.
 */

#include "build.h"
#include "hightile.h"
#include "baselayer.h"
#include "compat.h"
#include "engine_priv.h"
#include "kplib.h"

polytint_t hictinting[MAXPALOOKUPS];

hicreplctyp *hicreplc[MAXTILES];
int32_t hicinitcounter = 0;
int32_t usehightile=1;

//
// find the index into hicreplc[] which contains the replacement tile particulars
//
hicreplctyp *hicfindsubst(int picnum, int palnum, int nozero)
{
    if (!hicreplc[picnum] || !hicinitcounter) return nullptr;

    uint8_t const tfn = tilefilenum[picnum];
    
    do
    {                
        for (auto hr = hicreplc[picnum]; hr; hr = hr->next)
            if ((hr->palnum == palnum) & (hr->tfn == tfn))
                return hr;

        if (!palnum | !!nozero)
            return nullptr;

        palnum = 0;
    } while (1);

    return nullptr;	// no replacement found
}

//
// this is separate because it's not worth passing an extra parameter which is "0" in 99.9999% of cases
// to the regular hicfindsubst() function
//
hicreplctyp *hicfindskybox(int picnum, int palnum, int nozero)
{
    if (!hicreplc[picnum] || !hicinitcounter) return nullptr;

    uint8_t const tfn = tilefilenum[picnum];
    
    do
    {        
        for (auto hr = hicreplc[picnum]; hr; hr = hr->next)
            if (hr->skybox && ((hr->palnum == palnum) & (hr->tfn == tfn)))
                return hr;

        if (!palnum | !!nozero)
            return nullptr;

        palnum = 0;
    } while (1);

    return nullptr;	// no replacement found
}


//
// hicinit()
//   Initialize the high-colour stuff to default.
//
void hicinit(void)
{
    int32_t i;

    for (i=0; i<MAXPALOOKUPS; i++)  	// all tints should be 100%
    {
        polytint_t & tint = hictinting[i];
        tint.r = tint.g = tint.b = 0xff;
        tint.f = 0;
    }

    if (hicinitcounter)
    {
        hicreplctyp *hr, *next;
        int32_t j;

        for (i=MAXTILES-1; i>=0; i--)
        {
            for (hr=hicreplc[i]; hr;)
            {
                next = hr->next;

                if (hr->skybox)
                {
                    for (j=5; j>=0; j--)
                        Xfree(hr->skybox->face[j]);
                    Xfree(hr->skybox);
                }

                Xfree(hr->filename);
                Xfree(hr);

                hr = next;
            }
        }
    }

    Bmemset(hicreplc,0,sizeof(hicreplc));

    hicinitcounter++;
}


//
// hicsetpalettetint(pal,r,g,b,sr,sg,sb,effect)
//   The tinting values represent a mechanism for emulating the effect of global sector
//   palette shifts on true-colour textures and only true-colour textures.
//   effect bitset: 1 = greyscale, 2 = invert
//
void hicsetpalettetint(int32_t palnum, char r, char g, char b, char sr, char sg, char sb, polytintflags_t effect)
{
    if ((uint32_t)palnum >= (uint32_t)MAXPALOOKUPS) return;
    if (!hicinitcounter) hicinit();

    polytint_t & tint = hictinting[palnum];
    tint.r = r;
    tint.g = g;
    tint.b = b;
    tint.sr = sr;
    tint.sg = sg;
    tint.sb = sb;
    tint.f = effect;
}


//
// hicsetsubsttex(picnum,pal,filen,alphacut)
//   Specifies a replacement graphic file for an ART tile.
//
int32_t hicsetsubsttex(int32_t picnum, int32_t palnum, const char *filen, float alphacut, float xscale, float yscale, float specpower, float specfactor, char flags)
{
    hicreplctyp *hr, *hrn;

    if ((uint32_t)picnum >= (uint32_t)MAXTILES) return -1;
    if ((uint32_t)palnum >= (uint32_t)MAXPALOOKUPS) return -1;
    if (!hicinitcounter) hicinit();
    uint8_t const tfn = tilefilenum[picnum];
    for (hr = hicreplc[picnum]; hr; hr = hr->next)
    {
        if ((hr->palnum == palnum) & (hr->tfn == tfn))
            break;
    }

    if (!hr)
    {
        // no replacement yet defined
        hrn = (hicreplctyp *)Xcalloc(1,sizeof(hicreplctyp));
        hrn->palnum = palnum;
        hrn->tfn = tfn;
    }
    else hrn = hr;

    // store into hicreplc the details for this replacement
    Xfree(hrn->filename);

    hrn->filename = Xstrdup(filen);
    hrn->alphacut = min(alphacut,1.f);
    hrn->scale.x = xscale;
    hrn->scale.y = yscale;
    hrn->specpower = specpower;
    hrn->specfactor = specfactor;
    hrn->flags = flags;
    if (hr == NULL)
    {
        hrn->next = hicreplc[picnum];
        hicreplc[picnum] = hrn;
    }

    //if (tilesiz[picnum].x<=0 || tilesiz[picnum].y<=0)
    //{
    //    static int32_t first=1;
    //    if (first)
    //    {
    //        LOG_F(WARNING, "Defined replacement for empty tile %d.", picnum);
    //        first = 0;
    //    }
    //}

    //printf("Replacement [%d,%d]: %s\n", picnum, palnum, hicreplc[i]->filename);

    return 0;
}


//
// hicsetskybox(picnum,pal,faces[6])
//   Specifies a graphic files making up a skybox.
//
int32_t hicsetskybox( int32_t picnum, int32_t palnum, char *faces[6], int32_t flags )
{
    hicreplctyp *hr, *hrn;
    int32_t j;

    if ((uint32_t)picnum >= (uint32_t)MAXTILES) return -1;
    if ((uint32_t)palnum >= (uint32_t)MAXPALOOKUPS) return -1;
    for (j=5; j>=0; j--) if (!faces[j]) return -1;
    if (!hicinitcounter) hicinit();
    uint8_t const tfn = tilefilenum[picnum];
    for (hr = hicreplc[picnum]; hr; hr = hr->next)
    {
        if ((hr->palnum == palnum) & (hr->tfn == tfn))
            break;
    }

    if (!hr)
    {
        // no replacement yet defined
        hrn = (hicreplctyp *)Xcalloc(1,sizeof(hicreplctyp));
        hrn->palnum = palnum;
        hrn->tfn = tfn;
    }
    else hrn = hr;

    if (!hrn->skybox)
        hrn->skybox = (struct hicskybox_t *)Xcalloc(1,sizeof(struct hicskybox_t));
    else
    {
        for (j=0; j<6; j++)
            DO_FREE_AND_NULL(hrn->skybox->face[j]);
    }

    // store each face's filename
    for (j=0; j<6; j++)
        hrn->skybox->face[j] = Xstrdup(faces[j]);

    hrn->flags = flags;
    hrn->scale = { 1.f, 1.f };

    if (hr == NULL)
    {
        hrn->next = hicreplc[picnum];
        hicreplc[picnum] = hrn;
    }

    return 0;
}


//
// hicclearsubst(picnum,pal)
//   Clears a replacement for an ART tile, including skybox faces.
//
int32_t hicclearsubst(int32_t picnum, int32_t palnum)
{
    hicreplctyp *hr, *hrn = NULL;

    if ((uint32_t)picnum >= (uint32_t)MAXTILES) return -1;
    if ((uint32_t)palnum >= (uint32_t)MAXPALOOKUPS) return -1;
    if (!hicinitcounter) return 0;
    uint8_t const tfn = tilefilenum[picnum];
    for (hr = hicreplc[picnum]; hr; hrn = hr, hr = hr->next)
    {
        if ((hr->palnum == palnum) & (hr->tfn == tfn))
            break;
    }

    if (!hr) return 0;

    Xfree(hr->filename);
    if (hr->skybox)
    {
        int32_t i;
        for (i=5; i>=0; i--)
            Xfree(hr->skybox->face[i]);
        Xfree(hr->skybox);
    }

    if (hrn) hrn->next = hr->next;
    else hicreplc[picnum] = hr->next;
    Xfree(hr);

    return 0;
}

void hictinting_applypixcolor(coltype* tcol, uint8_t pal, bool no_rb_swap)
{
    polytintflags_t const effect = hicfxmask(pal);
    polytint_t const& tint = hictinting[pal];

    int32_t r = (!no_rb_swap && glinfo.bgra) ? tint.b : tint.r;
    int32_t g = tint.g;
    int32_t b = (!no_rb_swap && glinfo.bgra) ? tint.r : tint.b;

    if (effect & HICTINT_GRAYSCALE)
    {
        tcol->g = tcol->r = tcol->b = (uint8_t)((tcol->b * GRAYSCALE_COEFF_RED) +
            (tcol->g * GRAYSCALE_COEFF_GREEN) +
            (tcol->r * GRAYSCALE_COEFF_BLUE));
    }

    if (effect & HICTINT_INVERT)
    {
        tcol->b = 255 - tcol->b;
        tcol->g = 255 - tcol->g;
        tcol->r = 255 - tcol->r;
    }

    if (effect & HICTINT_COLORIZE)
    {
        tcol->b = min((int32_t)((tcol->b) * b) >> 6, 255);
        tcol->g = min((int32_t)((tcol->g) * g) >> 6, 255);
        tcol->r = min((int32_t)((tcol->r) * r) >> 6, 255);
    }

    switch (effect & HICTINT_BLENDMASK)
    {
    case HICTINT_BLEND_SCREEN:
        tcol->b = 255 - (((255 - tcol->b) * (255 - b)) >> 8);
        tcol->g = 255 - (((255 - tcol->g) * (255 - g)) >> 8);
        tcol->r = 255 - (((255 - tcol->r) * (255 - r)) >> 8);
        break;
    case HICTINT_BLEND_OVERLAY:
        tcol->b = tcol->b < 128 ? (tcol->b * b) >> 7 : 255 - (((255 - tcol->b) * (255 - b)) >> 7);
        tcol->g = tcol->g < 128 ? (tcol->g * g) >> 7 : 255 - (((255 - tcol->g) * (255 - g)) >> 7);
        tcol->r = tcol->r < 128 ? (tcol->r * r) >> 7 : 255 - (((255 - tcol->r) * (255 - r)) >> 7);
        break;
    case HICTINT_BLEND_HARDLIGHT:
        tcol->b = b < 128 ? (tcol->b * b) >> 7 : 255 - (((255 - tcol->b) * (255 - b)) >> 7);
        tcol->g = g < 128 ? (tcol->g * g) >> 7 : 255 - (((255 - tcol->g) * (255 - g)) >> 7);
        tcol->r = r < 128 ? (tcol->r * r) >> 7 : 255 - (((255 - tcol->r) * (255 - r)) >> 7);
        break;
    }
}
