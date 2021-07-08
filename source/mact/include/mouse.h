//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
Modifications for JonoF's port by Jonathon Fowler (jf@jonof.id.au)
*/
//-------------------------------------------------------------------------

#pragma once

#ifndef mouse_h_
#define mouse_h_

#include "baselayer.h"

#ifdef __cplusplus
extern "C" {
#endif

enum MouseButtonFlags : uint8_t
{
    M_LEFTBUTTON   = 1,
    M_RIGHTBUTTON  = 2,
    M_MIDDLEBUTTON = 4,
    M_XBUTTON      = 8,
    M_WHEELUP      = 16,
    M_WHEELDOWN    = 32,
};

extern float   CONTROL_MouseSensitivity;
extern float   CONTROL_MouseAxesSensitivity[2];

static inline bool MOUSE_Startup(void)
{
    mouseInit();
    return ((inputdevices & DEV_MOUSE) == DEV_MOUSE);
}

static inline void    MOUSE_Shutdown(void)         { mouseUninit(); }
static inline int32_t MOUSE_GetButtons(void)       { return mouseReadButtons(); }
static inline void    MOUSE_ClearButton(int32_t b) { g_mouseBits &= ~b; }
static inline void    MOUSE_ClearAllButtons(void)  { g_mouseBits = 0; }

#ifdef __cplusplus
}
#endif
#endif /* __mouse_h */
