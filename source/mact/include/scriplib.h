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
*/
//-------------------------------------------------------------------------

#pragma once

#ifndef scriplib_public_h_
#define scriplib_public_h_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t     SCRIPT_Init(char const *name);
void        SCRIPT_Free(int32_t scripthandle);
int32_t     SCRIPT_Load(char const *filename);
void        SCRIPT_Save(int32_t scripthandle, char const *filename);
int32_t     SCRIPT_NumberSections(int32_t scripthandle);
char const *SCRIPT_Section(int32_t scripthandle, int32_t which);
int32_t     SCRIPT_NumberEntries(int32_t scripthandle, char const *sectionname);
char const *SCRIPT_Entry(int32_t scripthandle, char const *sectionname, int32_t which);
char const *SCRIPT_GetRaw(int32_t scripthandle, char const *sectionname, char const *entryname);
int32_t     SCRIPT_GetStringPtr(int32_t scripthandle, char const *sectionname, char const *entryname, char **dest);
int32_t     SCRIPT_GetString(int32_t scripthandle, char const *sectionname, char const *entryname, char *dest);
int32_t     SCRIPT_GetDoubleString(int32_t scripthandle, const char *sectionname, const char *entryname, char *dest1, char *dest2);
int32_t     SCRIPT_GetNumber(int32_t scripthandle, const char *sectionname, const char *entryname, int32_t *number);
int32_t     SCRIPT_GetBoolean(int32_t scripthandle, char const *sectionname, char const *entryname, int32_t *boole);
int32_t     SCRIPT_GetDouble(int32_t scripthandle, const char * sectionname, const char * entryname, double * number);
void        SCRIPT_PutSection(int32_t scripthandle, char const *sectionname);
void        SCRIPT_PutRaw(int32_t scripthandle, char const *sectionname, char const *entryname, char const *raw);
void        SCRIPT_PutString(int32_t scripthandle, char const *sectionname, char const *entryname, const char *string);
void        SCRIPT_PutDoubleString(int32_t scripthandle, const char *sectionname, const char *entryname, const char *string1, const char *string2);
void        SCRIPT_PutNumber(int32_t scripthandle, const char *sectionname, const char *entryname, int32_t number, int32_t hexadecimal, int32_t defaultvalue);
void        SCRIPT_PutBoolean(int32_t scripthandle, char const *sectionname, char const *entryname, int32_t boole);
void        SCRIPT_PutDouble(int32_t scripthandle, char const *sectionname, char const *entryname, double number, int32_t defaultvalue);

#ifdef __cplusplus
}
#endif
#endif
