#ifndef KEN_COMMON_GAME_H_
#define KEN_COMMON_GAME_H_
#pragma once

#include "compat.h"

#ifndef APPNAME
#define APPNAME     "EKenBuild"
#endif

#ifndef APPBASENAME
#define APPBASENAME "ekenbuild"
#endif

extern const char *G_DefaultGrpFile(void);
extern const char *G_GrpFile(void);

extern int g_useCwd;

void Ken_AddSearchPaths(void);

void Ken_ExtPreInit(int32_t argc, char const * const * argv);
void Ken_ExtInit(void);

void Ken_PostStartupWindow(void);

extern int32_t voxid_PLAYER, voxid_BROWNMONSTER;
void Ken_LoadVoxels(void);

#endif
