//
// Common non-engine code/data for EDuke32 and Mapster32
//

#include "compat.h"
#include "build.h"
#include "baselayer.h"
#include "palette.h"

#ifdef _WIN32
# define NEED_SHLWAPI_H
# include "windows_inc.h"
# include "winbits.h"
# ifndef KEY_WOW64_64KEY
#  define KEY_WOW64_64KEY 0x0100
# endif
# ifndef KEY_WOW64_32KEY
#  define KEY_WOW64_32KEY 0x0200
# endif
#elif defined __APPLE__
# include "osxbits.h"
#endif

#include "common.h"
#include "common_game.h"

char UserPath[BMAX_PATH] = "/";
int kopen4loadfrommod(const char *fileName, char searchfirst)
{
    int kFile = -1;

    if (UserPath[0] != '/' || UserPath[1] != 0)
    {
        static char staticFileName[BMAX_PATH];
        Bsnprintf(staticFileName, sizeof(staticFileName), "%s/%s", UserPath, fileName);
        kFile = kopen4load(staticFileName, searchfirst);
    }

    return (kFile < 0) ? kopen4load(fileName, searchfirst) : kFile;
}
