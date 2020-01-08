/***************************************************************************
 *   TEKSMK.C  -   smack flic stuff etc. for Tekwar                        *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#include "build.h"
#include "compat.h"
#include "baselayer.h"
#include "pragmas.h"
#include "cache1d.h"

#include "tekwar.h"

#include "smacker.h"

static smk smkmenu;
static unsigned char *smkbuf;
#define SMKPICNUM (MAXTILES-1)

void
smkplayseq(char *name)
{
     debugprintf("smkplayseq(\"%s\")\n", name);
}

void
smkopenmenu(char *name)
{
     unsigned long xsiz, ysiz;
     int fh, flen;

     fh = kopen4load(name, 0);
     if (fh < 0) {
          debugprintf("smkopenmenu(\"%s\") failed\n", name);
          return;
     }

     flen = kfilelength(fh);
     smkbuf = (unsigned char *)malloc(flen);

     if (smkbuf == NULL) {
          kclose(fh);
          debugprintf("smkopenmenu(\"%s\") malloc of %d bytes failed\n", name, flen);
          return;
     }

     kread(fh, smkbuf, flen);
     kclose(fh);
    
     smkmenu = smk_open_memory(smkbuf, flen);
     if (!smkmenu) {
          free(smkbuf);
          smkbuf = 0;
          debugprintf("smk_open_file(\"%s\") returned null\n", name);
          return;
     }

     // Smacker frames are decoded linewise, but BUILD expects its
     // tiles columnwise, so we will treat the tile as though it's
     // rotated 90 degrees and flipped horizontally.
    
     smk_enable_all(smkmenu, SMK_VIDEO_TRACK);
     smk_info_video(smkmenu, &xsiz, &ysiz, NULL);
     tilesizx[SMKPICNUM] = ysiz;
     tilesizy[SMKPICNUM] = xsiz;
}

void
smkmenuframe(int fn)
{
     unsigned char spal[768], *palptr;
     int i;
    
     if (!smkmenu) {
          return;
     }
    
     // Render the first frame as the menu background.
     smk_first(smkmenu);
    
     // Next, render the particular frame requested.
     smk_render_frame(smkmenu, fn-1);

     // Convert the palette to the VGA (0-63) range BUILD uses.
     palptr = smk_get_palette(smkmenu);
     for (i=0; i<768; i++, palptr++) {
          spal[i] = (*palptr) >> 2;
     }
    
     waloff[SMKPICNUM] = (intptr_t)smk_get_video(smkmenu);
     invalidatetile(SMKPICNUM, 0, -1);

     setbrightness(brightness, spal, 0);
}

void
smkshowmenu()
{
     if (!smkmenu) {
          return;
     }

     setview(0,0, xdim-1, ydim-1);
     clearview(0);
     rotatesprite(160<<16, 100<<16, divscale16(200, tilesizx[SMKPICNUM]),
                 512, SMKPICNUM, 0, 0, 2+4, 0, 0, xdim-1, ydim-1);
     nextpage();
}

void
smkclosemenu()
{
     if (!smkmenu) {
          return;
     }
    
     waloff[SMKPICNUM] = 0;
    
     smk_close(smkmenu);
     free(smkbuf);
     smkbuf = 0;
}
