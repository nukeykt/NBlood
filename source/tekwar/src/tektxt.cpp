#include "build.h"
#include "baselayer.h"
#include "cache1d.h"
#include "mmulti.h"

#include "tekwar.h"

#if 0

/***************************************************************************
 *   TEKTXT.C  - text based & MiSCcellaneous routines for Tekwar game      *
 *   Uses BIOS modes for compatibility and scrolling.                      *
 *   Includes CD code.                                                     *
 *                                                     12/14/94 Les Bird   *
 ***************************************************************************/

#define   CDRELEASE
#define   COPYRIGHTSTRING     "INTRACORP1995TW"

#endif

char      bypasscdcheck=0;

void
crash(char *s,...)
{
     va_list argptr;
     
     musicoff();
     sendlogoff();
     uninitmultiplayers();
     uninitsb();
     cduninit();
     uninittimer();
     uninitinput();
     uninitengine();
     uninitgroupfile();
     teksavesetup();
     
     va_start(argptr,s);
     vprintf(s,argptr);
     va_end(argptr);
     printf("\n");
     exit(0);
}
