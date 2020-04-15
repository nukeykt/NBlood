// Copyright: 2020 Nuke.YKT, EDuke32 developers
// License: GPLv2

#pragma once

#ifdef USE_OPENGL

#include "vfs.h"

extern buildvfs_kfd rt_group;


buildvfs_kfd RT_InitGRP(const char *filename);
void RT_Init(void);
int RT_PrepareScript(void);

#endif
