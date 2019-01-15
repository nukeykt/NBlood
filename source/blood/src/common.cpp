//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT

This file is part of NBlood.

NBlood is free software; you can redistribute it and/or
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
