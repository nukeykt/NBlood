//-------------------------------------------------------------------------
/*
Copyright (C) 2016 EDuke32 developers and contributors

This file is part of EDuke32.

EDuke32 is free software; you can redistribute it and/or
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

#include "duke3d.h"
#include "menus.h"
#include "savegame.h"

#include "vfs.h"
#include "gamestructures.h"

gamevar_t   aGameVars[MAXGAMEVARS];
gamearray_t aGameArrays[MAXGAMEARRAYS];
int32_t     g_gameVarCount   = 0;
int32_t     g_gameArrayCount = 0;

// pointers to weapon gamevar data
intptr_t *aplWeaponClip[MAX_WEAPONS];           // number of items in magazine
intptr_t *aplWeaponFireDelay[MAX_WEAPONS];      // delay to fire
intptr_t *aplWeaponFireSound[MAX_WEAPONS];      // Sound made when firing (each time for automatic)
intptr_t *aplWeaponFlags[MAX_WEAPONS];          // Flags for weapon
intptr_t *aplWeaponFlashColor[MAX_WEAPONS];     // Muzzle flash color
intptr_t *aplWeaponHoldDelay[MAX_WEAPONS];      // delay after release fire button to fire (0 for none)
intptr_t *aplWeaponInitialSound[MAX_WEAPONS];   // Sound made when weapon starts firing. zero for no sound
intptr_t *aplWeaponReload[MAX_WEAPONS];         // delay to reload (include fire)
intptr_t *aplWeaponReloadSound1[MAX_WEAPONS];   // Sound of magazine being removed
intptr_t *aplWeaponReloadSound2[MAX_WEAPONS];   // Sound of magazine being inserted
intptr_t *aplWeaponSelectSound[MAX_WEAPONS];    // Sound of weapon being selected
intptr_t *aplWeaponShoots[MAX_WEAPONS];         // what the weapon shoots
intptr_t *aplWeaponShotsPerBurst[MAX_WEAPONS];  // number of shots per 'burst' (one ammo per 'burst')
intptr_t *aplWeaponSound2Sound[MAX_WEAPONS];    // Alternate sound sound ID
intptr_t *aplWeaponSound2Time[MAX_WEAPONS];     // Alternate sound time
intptr_t *aplWeaponSpawn[MAX_WEAPONS];          // the item to spawn
intptr_t *aplWeaponSpawnTime[MAX_WEAPONS];      // the frame at which to spawn an item
intptr_t *aplWeaponTotalTime[MAX_WEAPONS];      // The total time the weapon is cycling before next fire.
intptr_t *aplWeaponWorksLike[MAX_WEAPONS];      // What original the weapon works like

// Frees the memory for the *values* of game variables and arrays. Resets their
// counts to zero. Call this function as many times as needed.
//
// Returns: old g_gameVarCount | (g_gameArrayCount<<16).
int Gv_Free(void)
{
    for (auto &gameVar : aGameVars)
    {
        if (gameVar.flags & GAMEVAR_USER_MASK)
            ALIGNED_FREE_AND_NULL(gameVar.pValues);
        gameVar.flags |= GAMEVAR_RESET;
    }

    for (auto & gameArray : aGameArrays)
    {
        if (gameArray.flags & GAMEARRAY_ALLOCATED)
            ALIGNED_FREE_AND_NULL(gameArray.pValues);
        gameArray.flags |= GAMEARRAY_RESET;
    }

    EDUKE32_STATIC_ASSERT(MAXGAMEVARS < 32768);
    int const varCount = g_gameVarCount | (g_gameArrayCount << 16);
    g_gameVarCount = g_gameArrayCount = 0;

    hash_init(&h_gamevars);
    hash_init(&h_arrays);

    return varCount;
}

// Calls Gv_Free() and in addition frees the labels of all game variables and
// arrays.
// Only call this function at exit
void Gv_Clear(void)
{
    Gv_Free();

    // Now, only do work that Gv_Free() hasn't done.
    for (auto & gameVar : aGameVars)
        DO_FREE_AND_NULL(gameVar.szLabel);

    for (auto & gameArray : aGameArrays)
        DO_FREE_AND_NULL(gameArray.szLabel);

    for (auto i : vmStructHashTablePtrs)
        hash_free(i);
}

static int Gv_GetVarIndex(const char *szGameLabel);
static int Gv_GetArrayIndex(const char *szArrayLabel);

static constexpr char const s_gamevars[] = "CON:vars";
static constexpr char const s_arrays[]   = "CON:arry";
static constexpr char const s_mapstate[] = "CON:wrld";
static constexpr char const s_EOF[] = "E32SAVEGAME_EOF";

#ifdef NDEBUG
# define A_(x) do { if (!(x)) return -__LINE__; } while (0)
#else
# define A_(x) Bassert(x)
#endif

static int Gv_SkipLZ4Block(buildvfs_kfd kFile, size_t const size)
{
    auto skipvarbuf = (intptr_t*) Xmalloc(size);
    A_(kdfread_LZ4(skipvarbuf, size, 1, kFile) == 1);
    Xfree(skipvarbuf);
    return 0;
}

static int const s_gv_len = Bstrlen(s_gamevars);
static int const s_ar_len = Bstrlen(s_arrays);

int Gv_ReadSave(buildvfs_kfd kFile)
{
#ifndef NDEBUG
    auto const startofs = ktell(kFile);
#endif
    int32_t savedVarCount;
    A_(!kread_and_test(kFile, &savedVarCount, sizeof(savedVarCount)));

    char buf[32];
    char *varlabels = nullptr;

    if (savedVarCount)
    {
        A_(!kread_and_test(kFile, buf, s_gv_len));
        A_(!Bmemcmp(buf, s_gamevars, s_gv_len));

        int32_t varLabelCount;
        A_(!kread_and_test(kFile, &varLabelCount, sizeof(varLabelCount)));

        varlabels = (char *)Xcalloc(varLabelCount, MAXVARLABEL);
        A_(kdfread_LZ4(varlabels, varLabelCount * MAXVARLABEL, 1, kFile) == 1);
        gamevar_t readVar;
        int32_t   varLabelIndex;

        for (native_t i = 0; i < savedVarCount; i++)
        {
            A_(!kread_and_test(kFile, &varLabelIndex, sizeof(varLabelIndex)));
            A_(!kread_and_test(kFile, &readVar, sizeof(gamevar_t)));

            int const index = Gv_GetVarIndex(&varlabels[varLabelIndex * MAXVARLABEL]);
            if (index >= 0)
                A_(!Bstrcmp(&varlabels[varLabelIndex * MAXVARLABEL], aGameVars[index].szLabel));
            if (index < 0 || aGameVars[index].flags & SAVEGAMEVARSKIPMASK || aGameVars[index].flags != readVar.flags)
            {
                DVLOG_F(LOG_DEBUG, "Gv_ReadSave(): skipping '%s'", &varlabels[varLabelIndex * MAXVARLABEL]);
                if (readVar.flags & GAMEVAR_PERPLAYER)
                    A_(!Gv_SkipLZ4Block(kFile, MAXPLAYERS * sizeof(readVar.pValues[0])));
                else if (readVar.flags & GAMEVAR_PERACTOR)
                    A_(!Gv_SkipLZ4Block(kFile, MAXSPRITES * sizeof(readVar.pValues[0])));
                continue;
            }

            auto &writeVar = aGameVars[index];

            if (readVar.flags & GAMEVAR_PERPLAYER)
                A_(kdfread_LZ4(writeVar.pValues, sizeof(writeVar.pValues[0]) * MAXPLAYERS, 1, kFile) == 1);
            else if (readVar.flags & GAMEVAR_PERACTOR)
                A_(kdfread_LZ4(writeVar.pValues, sizeof(writeVar.pValues[0]) * MAXSPRITES, 1, kFile) == 1);
            else
                writeVar.global = readVar.global;
        }
    }

    int32_t savedArrayCount;
    A_(!kread_and_test(kFile, &savedArrayCount, sizeof(savedArrayCount)));

    char *arrlabels = nullptr;

    if (savedArrayCount)
    {
        A_(!kread_and_test(kFile, buf, s_ar_len));
        A_(!Bmemcmp(buf, s_arrays, s_ar_len));

        int32_t arrayLabelCount;
        A_(!kread_and_test(kFile, &arrayLabelCount, sizeof(arrayLabelCount)));

        arrlabels = (char *)Xcalloc(arrayLabelCount, MAXARRAYLABEL);
        A_(kdfread_LZ4(arrlabels, arrayLabelCount * MAXARRAYLABEL, 1, kFile) == 1);

        int32_t     arrayLabelIndex, arrayAllocSize;
        gamearray_t readArray;

        for (native_t i = 0; i < savedArrayCount; i++)
        {
            A_(!kread_and_test(kFile, &arrayLabelIndex, sizeof(arrayLabelIndex)));
            A_(!kread_and_test(kFile, &readArray, sizeof(gamearray_t)));
            A_(!kread_and_test(kFile, &arrayAllocSize, sizeof(arrayAllocSize)));

            int const index = Gv_GetArrayIndex(&arrlabels[arrayLabelIndex * MAXARRAYLABEL]);
            if (index >= 0)
                A_(!Bstrcmp(&arrlabels[arrayLabelIndex * MAXARRAYLABEL], aGameArrays[index].szLabel));
            if (index < 0 || aGameArrays[index].flags & SAVEGAMEARRAYSKIPMASK || aGameArrays[index].flags != readArray.flags)
            {
                DVLOG_F(LOG_DEBUG, "Gv_ReadSave(): skipping '%s'", &arrlabels[arrayLabelIndex * MAXARRAYLABEL]);
                if (readArray.size != 0)
                    A_(!arrayAllocSize || !Gv_SkipLZ4Block(kFile, arrayAllocSize));
                continue;
            }

            auto &writeArray = aGameArrays[index];
            writeArray.size  = readArray.size;
            ALIGNED_FREE_AND_NULL(writeArray.pValues);

            if (readArray.size != 0)
            {
                A_((size_t)arrayAllocSize == Gv_GetArrayAllocSize(index));
                writeArray.pValues = (intptr_t *)Xaligned_alloc(ARRAY_ALIGNMENT, Gv_GetArrayAllocSize(index));
                A_(kdfread_LZ4(writeArray.pValues, Gv_GetArrayAllocSize(index), 1, kFile) == 1);
            }
        }
    }

    int32_t worldStateCount;
    A_(!kread_and_test(kFile, &worldStateCount, sizeof(worldStateCount)));

    if (worldStateCount)
    {
        int const s_ms_len = Bstrlen(s_mapstate);

        A_(!kread_and_test(kFile, buf, s_ms_len));
        A_(!Bmemcmp(buf, s_mapstate, s_ms_len));

        uint8_t savedStateMap[(MAXVOLUMES * MAXLEVELS + 7) >> 3] = {};
        A_(kdfread_LZ4(savedStateMap, sizeof(savedStateMap), 1, kFile) == 1);

        A_(!kread_and_test(kFile, &savedVarCount, sizeof(savedVarCount)));
        A_(!kread_and_test(kFile, &savedArrayCount, sizeof(savedArrayCount)));

        for (native_t i = 0; i < (MAXVOLUMES * MAXLEVELS); i++)
        {
            G_FreeMapState(i);

            if (!bitmap_test(savedStateMap, i))
                continue;

            g_mapInfo[i].savedstate = (mapstate_t *)Xaligned_alloc(ACTOR_VAR_ALIGNMENT, sizeof(mapstate_t));
            A_(kdfread_LZ4(g_mapInfo[i].savedstate, sizeof(mapstate_t), 1, kFile) == 1);

            mapstate_t &sv = *g_mapInfo[i].savedstate;

            sv_restoreactors(sv.actor);

            if (savedVarCount && varlabels)
            {
                A_(!kread_and_test(kFile, buf, s_gv_len));
                A_(!Bmemcmp(buf, s_gamevars, s_gv_len));

                Bmemset(sv.vars, 0, sizeof(sv.vars));

                gamevar_t readVar;
                int32_t   varLabelIndex;

                for (native_t j = 0; j < savedVarCount; j++)
                {
                    A_(!kread_and_test(kFile, &varLabelIndex, sizeof(varLabelIndex)));
                    A_(!kread_and_test(kFile, &readVar, sizeof(gamevar_t)));

                    int const index = Gv_GetVarIndex(&varlabels[varLabelIndex * MAXVARLABEL]);

                    if (index >= 0)
                    {
                        sv.vars[index] = nullptr;
                        A_(!Bstrcmp(&varlabels[varLabelIndex * MAXVARLABEL], aGameVars[index].szLabel));
                    }

                    if (index < 0 || aGameVars[index].flags & SAVEGAMEMAPSTATEVARSKIPMASK || aGameVars[index].flags != readVar.flags)
                    {
                        if (readVar.flags == INT_MAX)
                            continue;

                        DVLOG_F(LOG_DEBUG, "Gv_ReadSave(): skipping '%s'", &varlabels[varLabelIndex * MAXVARLABEL]);

                        if (readVar.flags & GAMEVAR_PERPLAYER)
                            A_(!Gv_SkipLZ4Block(kFile, MAXPLAYERS * sizeof(readVar.pValues[0])));
                        else if (readVar.flags & GAMEVAR_PERACTOR)
                            A_(!Gv_SkipLZ4Block(kFile, MAXSPRITES * sizeof(readVar.pValues[0])));
                        else
                        {
                            intptr_t dummy;
                            A_(!kread_and_test(kFile, &dummy, sizeof(dummy)));
                        }

                        continue;
                    }

                    if (readVar.flags & GAMEVAR_PERPLAYER)
                    {
                        sv.vars[index] = (intptr_t *)Xaligned_alloc(PLAYER_VAR_ALIGNMENT, MAXPLAYERS * sizeof(sv.vars[0][0]));
                        A_(kdfread_LZ4(sv.vars[index], sizeof(sv.vars[0][0]) * MAXPLAYERS, 1, kFile) == 1);
                    }
                    else if (readVar.flags & GAMEVAR_PERACTOR)
                    {
                        sv.vars[index] = (intptr_t *)Xaligned_alloc(ACTOR_VAR_ALIGNMENT, MAXSPRITES * sizeof(sv.vars[0][0]));
                        A_(kdfread_LZ4(sv.vars[index], sizeof(sv.vars[0][0]) * MAXSPRITES, 1, kFile) == 1);
                    }
                    else
                        A_(!kread_and_test(kFile, &sv.vars[index], sizeof(sv.vars[0][0])));
                }
            }

            if (savedArrayCount && arrlabels)
            {
                A_(!kread_and_test(kFile, buf, s_ar_len));
                A_(!Bmemcmp(buf, s_arrays, s_ar_len));

                Bmemset(sv.arrays, 0, sizeof(sv.arrays));

                int32_t arrayLabelIndex, arrayAllocSize;

                for (native_t j = 0; j < savedArrayCount; j++)
                {
                    A_(!kread_and_test(kFile, &arrayLabelIndex, sizeof(arrayLabelIndex)));

                    int const index = Gv_GetArrayIndex(&arrlabels[arrayLabelIndex * MAXARRAYLABEL]);
                    if (index >= 0)
                        A_(!Bstrcmp(&arrlabels[arrayLabelIndex * MAXARRAYLABEL], aGameArrays[index].szLabel));
                    if (index < 0 || (aGameArrays[index].flags & (GAMEARRAY_RESTORE|SAVEGAMEARRAYSKIPMASK)) != GAMEARRAY_RESTORE)
                    {
                        DVLOG_F(LOG_DEBUG, "Gv_ReadSave(): skipping '%s'", &arrlabels[arrayLabelIndex * MAXARRAYLABEL]);
                        A_(!kread_and_test(kFile, &arrayAllocSize, sizeof(arrayAllocSize)));
                        A_(!kread_and_test(kFile, &arrayAllocSize, sizeof(arrayAllocSize)));
                        A_(!arrayAllocSize || !Gv_SkipLZ4Block(kFile, arrayAllocSize));
                        continue;
                    }

                    A_(!kread_and_test(kFile, &sv.arraysiz[index], sizeof(sv.arraysiz[0])));
                    A_(!kread_and_test(kFile, &arrayAllocSize, sizeof(arrayAllocSize)));
                    A_((unsigned)arrayAllocSize == Gv_GetArrayAllocSizeForCount(index, sv.arraysiz[index]));
                    if (arrayAllocSize)
                        sv.arrays[index] = (intptr_t *)Xaligned_alloc(ARRAY_ALIGNMENT, arrayAllocSize);
                    A_(!arrayAllocSize || kdfread_LZ4(sv.arrays[index], arrayAllocSize, 1, kFile) == 1);
                }
            }
        }
    }

    A_(!kread_and_test(kFile, buf, Bstrlen(s_EOF)));
    A_(!Bmemcmp(buf, s_EOF, Bstrlen(s_EOF)));

    DVLOG_F(LOG_DEBUG, "Gv_ReadSave(): read %d bytes extended data from offset 0x%08x", ktell(kFile) - startofs, startofs);

    Xfree(varlabels);
    Xfree(arrlabels);

    Gv_InitWeaponPointers();
    Gv_RefreshPointers();

    return 0;
}
#undef A_

void Gv_WriteSave(buildvfs_FILE fil)
{
#ifndef NDEBUG
    int const startofs = buildvfs_ftell(fil);
#endif
    int32_t savedVarCount = 0;
    for (native_t i = 0; i < g_gameVarCount; i++)
    {
        if (aGameVars[i].flags & SAVEGAMEVARSKIPMASK)
            continue;

        savedVarCount++;
    }
    buildvfs_fwrite(&savedVarCount, sizeof(savedVarCount), 1, fil);

    char *varlabels = nullptr;

    if (savedVarCount)
    {
        buildvfs_fwrite(s_gamevars, s_gv_len, 1, fil);

        // this is the size of the label table, not the number of actual saved vars
        buildvfs_fwrite(&g_gameVarCount, sizeof(g_gameVarCount), 1, fil);

        varlabels = (char *)Xcalloc(g_gameVarCount, MAXVARLABEL);

        for (native_t i = 0; i < g_gameVarCount; i++)
        {
            if (aGameVars[i].flags & SAVEGAMEVARSKIPMASK)
                continue;
            Bmemcpy(&varlabels[i * MAXVARLABEL], aGameVars[i].szLabel, MAXVARLABEL);
        }

        dfwrite_LZ4(varlabels, g_gameVarCount * MAXVARLABEL, 1, fil);
        int writeCnt = 0;
        for (int32_t idx = 0; idx < g_gameVarCount; idx++)
        {
            EDUKE32_STATIC_ASSERT(sizeof(idx) == sizeof(int32_t));

            auto &var = aGameVars[idx];

            if (var.flags & SAVEGAMEVARSKIPMASK)
                continue;
            writeCnt++;
            buildvfs_fwrite(&idx, sizeof(idx), 1, fil);
            buildvfs_fwrite(&var, sizeof(gamevar_t), 1, fil);

            if (var.flags & GAMEVAR_PERPLAYER)
                dfwrite_LZ4(var.pValues, sizeof(var.pValues[0]) * MAXPLAYERS, 1, fil);
            else if (var.flags & GAMEVAR_PERACTOR)
                dfwrite_LZ4(var.pValues, sizeof(var.pValues[0]) * MAXSPRITES, 1, fil);
        }
        Bassert(savedVarCount == writeCnt);
    }

    int32_t savedArrayCount = 0;
    for (native_t i = 0; i < g_gameArrayCount; i++)
    {
        if (aGameArrays[i].flags & SAVEGAMEARRAYSKIPMASK)
            continue;

        savedArrayCount++;
    }
    buildvfs_fwrite(&savedArrayCount, sizeof(savedArrayCount), 1, fil);

    char *arrlabels = nullptr;

    if (savedArrayCount)
    {
        buildvfs_fwrite(s_arrays, s_ar_len, 1, fil);

        // this is the size of the label table, not the number of actual saved arrays
        buildvfs_fwrite(&g_gameArrayCount, sizeof(g_gameArrayCount), 1, fil);

        arrlabels = (char *)Xcalloc(g_gameArrayCount, MAXARRAYLABEL);

        for (native_t i = 0; i < g_gameArrayCount; i++)
        {
            if (aGameArrays[i].flags & SAVEGAMEARRAYSKIPMASK)
                continue;
            Bmemcpy(&arrlabels[i * MAXARRAYLABEL], aGameArrays[i].szLabel, MAXARRAYLABEL);
        }

        dfwrite_LZ4(arrlabels, g_gameArrayCount * MAXARRAYLABEL, 1, fil);

        for (int32_t idx = 0; idx < g_gameArrayCount; idx++)
        {
            EDUKE32_STATIC_ASSERT(sizeof(idx) == sizeof(int32_t));

            auto &array = aGameArrays[idx];

            if (array.flags & SAVEGAMEARRAYSKIPMASK)
                continue;

            // write for .size and .dwFlags (the rest are pointers):
            buildvfs_fwrite(&idx, sizeof(idx), 1, fil);
            buildvfs_fwrite(&array, sizeof(gamearray_t), 1, fil);

            int32_t arrayAllocSize = Gv_GetArrayAllocSize(idx);
            buildvfs_fwrite(&arrayAllocSize, sizeof(arrayAllocSize), 1, fil);

            if (arrayAllocSize > 0)
                dfwrite_LZ4(array.pValues, arrayAllocSize, 1, fil);
        }
    }


    uint8_t savedStateMap[(MAXVOLUMES * MAXLEVELS + 7) >> 3] = {};
    int32_t worldStateCount = 0;

    for (native_t i = 0; i < (MAXVOLUMES * MAXLEVELS); i++)
        if (g_mapInfo[i].savedstate != nullptr)
            bitmap_set(savedStateMap, i), ++worldStateCount;

    buildvfs_fwrite(&worldStateCount, sizeof(worldStateCount), 1, fil);

    if (worldStateCount)
    {
        buildvfs_fwrite(s_mapstate, Bstrlen(s_mapstate), 1, fil);
        dfwrite_LZ4(savedStateMap, sizeof(savedStateMap), 1, fil);

        // these are separate counts from the ones above because mapstate_t uses a more restrictive mask than the general savegame format
        savedVarCount = 0;
        for (native_t i = 0; i < g_gameVarCount; i++)
        {
            if (aGameVars[i].flags & SAVEGAMEMAPSTATEVARSKIPMASK)
                continue;

            savedVarCount++;
        }
        buildvfs_fwrite(&savedVarCount, sizeof(savedVarCount), 1, fil);

        savedArrayCount = 0;
        for (native_t i = 0; i < g_gameArrayCount; i++)
        {
            if ((aGameArrays[i].flags & (GAMEARRAY_RESTORE|SAVEGAMEARRAYSKIPMASK)) != GAMEARRAY_RESTORE)
                continue;

            savedArrayCount++;
        }
        buildvfs_fwrite(&savedArrayCount, sizeof(savedArrayCount), 1, fil);

        for (native_t i = 0; i < (MAXVOLUMES * MAXLEVELS); i++)
        {
            if (g_mapInfo[i].savedstate == nullptr)
                continue;

            mapstate_t &sv = *g_mapInfo[i].savedstate;

            sv_prepareactors(sv.actor);
            dfwrite_LZ4(g_mapInfo[i].savedstate, sizeof(mapstate_t), 1, fil);
            sv_restoreactors(sv.actor);

            if (savedVarCount)
            {
                buildvfs_fwrite(s_gamevars, s_gv_len, 1, fil);

                int writeCnt = 0;
                for (int32_t idx = 0; idx < g_gameVarCount; idx++)
                {
                    EDUKE32_STATIC_ASSERT(sizeof(idx) == sizeof(int32_t));

                    auto &var = aGameVars[idx];

                    if (var.flags & SAVEGAMEMAPSTATEVARSKIPMASK)
                        continue;

                    buildvfs_fwrite(&idx, sizeof(idx), 1, fil);
                    writeCnt++;
                    // these will be null if the mapstate comes from an old savegame with gamevars that were skipped during load
                    if ((var.flags& GAMEVAR_USER_MASK) && sv.vars[idx] == nullptr)
                    {
                        gamevar_t dummy = {};
                        dummy.flags = INT_MAX;
                        buildvfs_fwrite(&dummy, sizeof(dummy), 1, fil);
                        continue;
                    }

                    buildvfs_fwrite(&var, sizeof(var), 1, fil);

                    if (var.flags & GAMEVAR_PERPLAYER)
                        dfwrite_LZ4(sv.vars[idx], sizeof(sv.vars[0][0]) * MAXPLAYERS, 1, fil);
                    else if (var.flags & GAMEVAR_PERACTOR)
                        dfwrite_LZ4(sv.vars[idx], sizeof(sv.vars[0][0]) * MAXSPRITES, 1, fil);
                    else
                        buildvfs_fwrite(&sv.vars[idx], sizeof(sv.vars[0][0]), 1, fil);
                }
                Bassert(savedVarCount == writeCnt);
            }

            if (savedArrayCount)
            {
                buildvfs_fwrite(s_arrays, s_ar_len, 1, fil);

                for (int32_t idx = 0; idx < g_gameArrayCount; idx++)
                {
                    EDUKE32_STATIC_ASSERT(sizeof(idx) == sizeof(int32_t));

                    auto &array = aGameArrays[idx];

                    if ((array.flags & (GAMEARRAY_RESTORE|SAVEGAMEARRAYSKIPMASK)) != GAMEARRAY_RESTORE)
                        continue;

                    buildvfs_fwrite(&idx, sizeof(idx), 1, fil);
                    buildvfs_fwrite(&sv.arraysiz[idx], sizeof(sv.arraysiz[0]), 1, fil);
                    int32_t arrayAllocSize = Gv_GetArrayAllocSizeForCount(idx, sv.arraysiz[idx]);
                    buildvfs_fwrite(&arrayAllocSize, sizeof(arrayAllocSize), 1, fil);
                    if (arrayAllocSize > 0)
                        dfwrite_LZ4(sv.arrays[idx], arrayAllocSize, 1, fil);
                }
            }
        }
    }

    buildvfs_fwrite(s_EOF, Bstrlen(s_EOF), 1, fil);
    DVLOG_F(LOG_DEBUG, "Gv_WriteSave(): wrote %d bytes extended data at offset 0x%08x", (int)buildvfs_ftell(fil) - startofs, startofs);

    Xfree(varlabels);
    Xfree(arrlabels);
}

void Gv_DumpValues(void)
{
    buildprint("// Current Game Definitions\n\n");

    for (bssize_t i=0; i<g_gameVarCount; i++)
    {
        buildprint("gamevar ", aGameVars[i].szLabel, " ");

        if (aGameVars[i].flags & (GAMEVAR_INT32PTR))
            buildprint(*(int32_t *)aGameVars[i].global);
        else if (aGameVars[i].flags & (GAMEVAR_INT16PTR))
            buildprint(*(int16_t *)aGameVars[i].global);
        else
            buildprint(aGameVars[i].global);

        if (aGameVars[i].flags & (GAMEVAR_PERPLAYER))
            buildprint(" GAMEVAR_PERPLAYER");
        else if (aGameVars[i].flags & (GAMEVAR_PERACTOR))
            buildprint(" GAMEVAR_PERACTOR");
        else
            buildprint(" ", aGameVars[i].flags/* & (GAMEVAR_USER_MASK)*/);

        buildprint(" // ");
        if (aGameVars[i].flags & (GAMEVAR_SYSTEM))
            buildprint(" (system)");
        if (aGameVars[i].flags & (GAMEVAR_PTR_MASK))
            buildprint(" (pointer)");
        if (aGameVars[i].flags & (GAMEVAR_READONLY))
            buildprint(" (read only)");
        if (aGameVars[i].flags & (GAMEVAR_SPECIAL))
            buildprint(" (special)");
        buildprint("\n");
    }
    buildprint("\n// end of game definitions\n");
}

// XXX: This function is very strange.
void Gv_ResetVars(void) /* this is called during a new game and nowhere else */
{
    Gv_Free();

    for (auto &aGameVar : aGameVars)
    {
        if (aGameVar.szLabel != NULL)
            Gv_NewVar(aGameVar.szLabel, (aGameVar.flags & GAMEVAR_NODEFAULT) ? aGameVar.global : aGameVar.defaultValue, aGameVar.flags);
    }

    for (auto &aGameArray : aGameArrays)
    {
        if (aGameArray.szLabel != NULL && aGameArray.flags & GAMEARRAY_RESET)
            Gv_NewArray(aGameArray.szLabel, aGameArray.pValues, aGameArray.size, aGameArray.flags);
    }
}

unsigned __fastcall Gv_GetArrayElementSize(int const arrayIdx)
{
    int typeSize = 0;

    switch (aGameArrays[arrayIdx].flags & GAMEARRAY_SIZE_MASK)
    {
        case 0: typeSize = sizeof(uintptr_t); break;
        case GAMEARRAY_INT8: typeSize = sizeof(uint8_t); break;
        case GAMEARRAY_INT16: typeSize = sizeof(uint16_t); break;
    }

    return typeSize;
}

void Gv_NewArray(const char *pszLabel, void *arrayptr, intptr_t asize, uint32_t dwFlags)
{
    Bassert(asize >= 0);

    if (EDUKE32_PREDICT_FALSE(g_gameArrayCount >= MAXGAMEARRAYS))
    {
        g_errorCnt++;
        C_ReportError(-1);
        LOG_F(ERROR, "%s:%d: too many arrays defined!",g_scriptFileName,g_lineNumber);
        return;
    }

    if (EDUKE32_PREDICT_FALSE(Bstrlen(pszLabel) > (MAXARRAYLABEL-1)))
    {
        g_errorCnt++;
        C_ReportError(-1);
        LOG_F(ERROR, "%s:%d: array name '%s' exceeds limit of %d characters.",g_scriptFileName,g_lineNumber,pszLabel, MAXARRAYLABEL);
        return;
    }

    int32_t i = hash_find(&h_arrays,pszLabel);

    if (EDUKE32_PREDICT_FALSE(i >=0 && !(aGameArrays[i].flags & GAMEARRAY_RESET)))
    {
        // found it it's a duplicate in error

        g_warningCnt++;
        C_ReportError(WARNING_DUPLICATEDEFINITION);
        return;
    }

    i = g_gameArrayCount;

    if (aGameArrays[i].szLabel == NULL)
        aGameArrays[i].szLabel = (char *)Xcalloc(MAXVARLABEL, sizeof(uint8_t));

    if (aGameArrays[i].szLabel != pszLabel)
        Bstrcpy(aGameArrays[i].szLabel,pszLabel);

    aGameArrays[i].flags = dwFlags & ~GAMEARRAY_RESET;
    aGameArrays[i].size  = asize;

    if (arrayptr)
        aGameArrays[i].pValues = (intptr_t *)arrayptr;
    else if (!(aGameArrays[i].flags & GAMEARRAY_SYSTEM))
    {
        if (aGameArrays[i].flags & GAMEARRAY_ALLOCATED)
            ALIGNED_FREE_AND_NULL(aGameArrays[i].pValues);

        int const allocSize = Gv_GetArrayAllocSize(i);

        aGameArrays[i].flags |= GAMEARRAY_ALLOCATED;
        if (allocSize > 0)
        {
            aGameArrays[i].pValues = (intptr_t *) Xaligned_alloc(ARRAY_ALIGNMENT, allocSize);
            Bmemset(aGameArrays[i].pValues, 0, allocSize);
        }
        else
        {
            aGameArrays[i].pValues = nullptr;
        }
    }

    g_gameArrayCount++;
    hash_add(&h_arrays, aGameArrays[i].szLabel, i, 1);
}

void Gv_NewVar(const char *pszLabel, intptr_t lValue, uint32_t dwFlags)
{
    if (EDUKE32_PREDICT_FALSE(g_gameVarCount >= MAXGAMEVARS))
    {
        g_errorCnt++;
        C_ReportError(-1);
        LOG_F(ERROR, "%s:%d: too many gamevars defined!",g_scriptFileName,g_lineNumber);
        return;
    }

    if (EDUKE32_PREDICT_FALSE(Bstrlen(pszLabel) > (MAXVARLABEL-1)))
    {
        g_errorCnt++;
        C_ReportError(-1);
        LOG_F(ERROR, "%s:%d: variable name '%s' exceeds limit of %d characters.",g_scriptFileName,g_lineNumber,pszLabel, MAXVARLABEL);
        return;
    }

    int gV = hash_find(&h_gamevars,pszLabel);

    if (gV >= 0 && !(aGameVars[gV].flags & GAMEVAR_RESET))
    {
        // found it...
        if (EDUKE32_PREDICT_FALSE(aGameVars[gV].flags & (GAMEVAR_PTR_MASK)))
        {
            C_ReportError(-1);
            LOG_F(WARNING, "%s:%d: cannot redefine internal gamevar '%s'.",g_scriptFileName,g_lineNumber,label+(g_labelCnt<<6));
            return;
        }
        else if (EDUKE32_PREDICT_FALSE(!(aGameVars[gV].flags & GAMEVAR_SYSTEM)))
        {
            // it's a duplicate in error
            g_warningCnt++;
            C_ReportError(WARNING_DUPLICATEDEFINITION);
            return;
        }
    }

    if (gV == -1)
        gV = g_gameVarCount;

    auto &newVar = aGameVars[gV];

    // If it's a user gamevar...
    if ((newVar.flags & GAMEVAR_SYSTEM) == 0)
    {
        // Allocate and set its label
        if (newVar.szLabel == NULL)
            newVar.szLabel = (char *)Xcalloc(MAXVARLABEL,sizeof(uint8_t));

        if (newVar.szLabel != pszLabel)
            Bstrcpy(newVar.szLabel,pszLabel);

        // and the flags
        newVar.flags=dwFlags;

        // only free if per-{actor,player}
        if (newVar.flags & GAMEVAR_USER_MASK)
            ALIGNED_FREE_AND_NULL(newVar.pValues);
    }

    // if existing is system, they only get to change default value....
    newVar.defaultValue = lValue;
    newVar.flags &= ~GAMEVAR_RESET;

    if (gV == g_gameVarCount)
    {
        // we're adding a new one.
        hash_add(&h_gamevars, newVar.szLabel, g_gameVarCount++, 0);
    }

    // Set initial values. (Or, override values for system gamevars.)
    if (newVar.flags & GAMEVAR_PERPLAYER)
    {
        if (!newVar.pValues)
        {
            newVar.pValues = (intptr_t *) Xaligned_alloc(PLAYER_VAR_ALIGNMENT, MAXPLAYERS * sizeof(intptr_t));
            Bmemset(newVar.pValues, 0, MAXPLAYERS * sizeof(intptr_t));
        }
        for (bssize_t j=MAXPLAYERS-1; j>=0; --j)
            newVar.pValues[j]=lValue;
//        std::fill_n(newVar.pValues, MAXPLAYERS, lValue);
    }
    else if (newVar.flags & GAMEVAR_PERACTOR)
    {
        if (!newVar.pValues)
        {
            newVar.pValues = (intptr_t *) Xaligned_alloc(ACTOR_VAR_ALIGNMENT, MAXSPRITES * sizeof(intptr_t));
            Bmemset(newVar.pValues, 0, MAXSPRITES * sizeof(intptr_t));
        }
        for (bssize_t j=MAXSPRITES-1; j>=0; --j)
            newVar.pValues[j]=lValue;
    }
    else newVar.global = lValue;
}

static int Gv_GetVarIndex(const char *szGameLabel)
{
    int const gameVar = hash_find(&h_gamevars,szGameLabel);

    if (EDUKE32_PREDICT_FALSE((unsigned)gameVar >= MAXGAMEVARS))
        return -1;

    return gameVar;
}

static int Gv_GetArrayIndex(const char *szArrayLabel)
{
    int const arrayIdx = hash_find(&h_arrays, szArrayLabel);

    if (EDUKE32_PREDICT_FALSE((unsigned)arrayIdx >= MAXGAMEARRAYS))
        return -1;

    return arrayIdx;
}

size_t __fastcall Gv_GetArrayAllocSizeForCount(int const arrayIdx, size_t const count)
{
    if (aGameArrays[arrayIdx].flags & GAMEARRAY_BITMAP)
        return (count + 7) >> 3;

    return count * Gv_GetArrayElementSize(arrayIdx);
}

size_t __fastcall Gv_GetArrayCountForAllocSize(int const arrayIdx, size_t const filelength)
{
    if (aGameArrays[arrayIdx].flags & GAMEARRAY_BITMAP)
        return filelength << 3;

    size_t const elementSize = Gv_GetArrayElementSize(arrayIdx);
    size_t const denominator = min(elementSize, sizeof(uint32_t));

    Bassert(denominator);

    return tabledivide64(filelength + denominator - 1, denominator);
}

#define CHECK_INDEX(range)                                              \
    if (EDUKE32_PREDICT_FALSE((unsigned)arrayIndex >= (unsigned)range)) \
    {                                                                   \
        returnValue = arrayIndex;                                       \
        goto badindex;                                                  \
    }

static int __fastcall Gv_GetArrayOrStruct(int const gameVar, int const spriteNum, int const playerNum)
{
    int const gv = gameVar & (MAXGAMEVARS-1);
    int returnValue = 0;

    if (gameVar & GV_FLAG_STRUCT)  // struct shortcut vars
    {
        int       arrayIndexVar = *insptr++;
        int       arrayIndex    = Gv_GetVar(arrayIndexVar, spriteNum, playerNum);
        int const labelNum      = *insptr++;

        switch (gv - g_structVarIDs)
        {
            case STRUCT_SPRITE_INTERNAL__:
                CHECK_INDEX(MAXSPRITES);
                returnValue = VM_GetStruct(ActorLabels[labelNum].flags, (intptr_t *)((intptr_t)&sprite[arrayIndex] + ActorLabels[labelNum].offset));
                break;

            case STRUCT_ACTOR_INTERNAL__:
                CHECK_INDEX(MAXSPRITES);
                returnValue = VM_GetStruct(ActorLabels[labelNum].flags, (intptr_t *)((intptr_t)&actor[arrayIndex] + ActorLabels[labelNum].offset));
                break;

            // no THISACTOR check here because we convert those cases to setvarvar
            case STRUCT_ACTORVAR: returnValue = Gv_GetVar(labelNum, arrayIndex, vm.playerNum); break;
            case STRUCT_PLAYERVAR: returnValue = Gv_GetVar(labelNum, vm.spriteNum, arrayIndex); break;

            case STRUCT_SECTOR:
                if (arrayIndexVar == g_thisActorVarID)
                    arrayIndex = vm.pSprite->sectnum;
                CHECK_INDEX(MAXSECTORS);
                returnValue = VM_GetSector(arrayIndex, labelNum);
                break;

            case STRUCT_SECTOR_INTERNAL__:
                if (arrayIndexVar == g_thisActorVarID)
                    arrayIndex = vm.pSprite->sectnum;
                CHECK_INDEX(MAXSECTORS);
                returnValue = VM_GetStruct(SectorLabels[labelNum].flags, (intptr_t *)((intptr_t)&sector[arrayIndex] + SectorLabels[labelNum].offset));
                break;

            case STRUCT_WALL:
                CHECK_INDEX(MAXWALLS);
                returnValue = VM_GetWall(arrayIndex, labelNum);
                break;

            case STRUCT_WALL_INTERNAL__:
                CHECK_INDEX(MAXWALLS);
                returnValue = VM_GetStruct(WallLabels[labelNum].flags, (intptr_t *)((intptr_t)&wall[arrayIndex] + WallLabels[labelNum].offset));
                break;

            case STRUCT_SPRITE:
                CHECK_INDEX(MAXSPRITES);
                arrayIndexVar = (ActorLabels[labelNum].flags & LABEL_HASPARM2) ? Gv_GetVar(*insptr++, spriteNum, playerNum) : 0;
                returnValue = VM_GetSprite(arrayIndex, labelNum, arrayIndexVar);
                break;

            case STRUCT_SPRITEEXT_INTERNAL__:
                CHECK_INDEX(MAXSPRITES);
                returnValue = VM_GetStruct(ActorLabels[labelNum].flags, (intptr_t *)((intptr_t)&spriteext[arrayIndex] + ActorLabels[labelNum].offset));
                break;

            case STRUCT_TSPR:
                CHECK_INDEX(MAXSPRITES);
                returnValue = VM_GetStruct(TsprLabels[labelNum].flags, (intptr_t *)((intptr_t)(spriteext[arrayIndex].tspr) + TsprLabels[labelNum].offset));
                break;

            case STRUCT_PLAYER:
                if (arrayIndexVar == g_thisActorVarID)
                    arrayIndex = vm.playerNum;
                CHECK_INDEX(MAXPLAYERS);
                arrayIndexVar = (EDUKE32_PREDICT_FALSE(PlayerLabels[labelNum].flags & LABEL_HASPARM2)) ? Gv_GetVar(*insptr++, spriteNum, playerNum) : 0;
                returnValue = VM_GetPlayer(arrayIndex, labelNum, arrayIndexVar);
                break;

            case STRUCT_PLAYER_INTERNAL__:
                if (arrayIndexVar == g_thisActorVarID)
                    arrayIndex = vm.playerNum;
                CHECK_INDEX(MAXPLAYERS);
                returnValue = VM_GetStruct(PlayerLabels[labelNum].flags, (intptr_t *)((intptr_t)&g_player[arrayIndex].ps[0] + PlayerLabels[labelNum].offset));
                break;

            case STRUCT_THISPROJECTILE:
                CHECK_INDEX(MAXSPRITES);
                returnValue = VM_GetActiveProjectile(arrayIndex, labelNum);
                break;

            case STRUCT_PROJECTILE:
                CHECK_INDEX(MAXTILES);
                returnValue = VM_GetProjectile(arrayIndex, labelNum);
                break;

            case STRUCT_TILEDATA:
                if (arrayIndexVar == g_thisActorVarID)
                    arrayIndex = vm.pSprite->picnum;
                CHECK_INDEX(MAXTILES);
                returnValue = VM_GetTileData(arrayIndex, labelNum);
                break;

            case STRUCT_PALDATA:
                if (arrayIndexVar == g_thisActorVarID)
                    arrayIndex = vm.pSprite->pal;
                CHECK_INDEX(MAXPALOOKUPS);
                returnValue = VM_GetPalData(arrayIndex, labelNum);
                break;

            case STRUCT_INPUT:
                if (arrayIndexVar == g_thisActorVarID)
                    arrayIndex = vm.playerNum;
                CHECK_INDEX(MAXPLAYERS);
                returnValue = VM_GetPlayerInput(arrayIndex, labelNum);
                break;

            case STRUCT_USERDEF:
                arrayIndexVar = (EDUKE32_PREDICT_FALSE(UserdefsLabels[labelNum].flags & LABEL_HASPARM2)) ? Gv_GetVar(*insptr++) : 0;
                returnValue   = VM_GetUserdef(labelNum, arrayIndexVar);
                break;
        }
    }
    else // if (gameVar & GV_FLAG_ARRAY)
    {
        int const arrayIndex = Gv_GetVar(*insptr++, spriteNum, playerNum);

        CHECK_INDEX(aGameArrays[gv].size);
        returnValue = Gv_GetArrayValue(gv, arrayIndex);
    }

    return returnValue;

badindex:
    LOG_F(ERROR, "%s:%d: Invalid index %d for '%s'", VM_FILENAME(insptr), VM_DECODE_LINE_NUMBER(g_tw), returnValue,
                  (gameVar & GV_FLAG_ARRAY) ? aGameArrays[gv].szLabel : aGameVars[gv].szLabel);
    return -1;
}

static FORCE_INLINE int __fastcall getvar__(int const gameVar, int const spriteNum, int const playerNum)
{
    if (gameVar & (GV_FLAG_STRUCT|GV_FLAG_ARRAY))
        return Gv_GetArrayOrStruct(gameVar, spriteNum, playerNum);
    else if (gameVar == g_thisActorVarID)
        return spriteNum;
    else if (gameVar == GV_FLAG_CONSTANT)
        return *insptr++;
    else
    {
        auto const &var = aGameVars[gameVar & (MAXGAMEVARS-1)];

        int returnValue = 0;

        switch (var.flags & (GAMEVAR_USER_MASK|GAMEVAR_PTR_MASK))
        {
            default: returnValue = var.global; break;
            case GAMEVAR_PERACTOR:  returnValue = var.pValues[spriteNum & (MAXSPRITES-1)]; break;
            case GAMEVAR_PERPLAYER: returnValue = var.pValues[playerNum & (MAXPLAYERS-1)];break;
            case GAMEVAR_RAWQ16PTR:
            case GAMEVAR_INT32PTR: returnValue = *(int32_t *)var.global; break;
            case GAMEVAR_INT16PTR: returnValue = *(int16_t *)var.global; break;
            case GAMEVAR_Q16PTR:   returnValue = fix16_to_int(*(fix16_t *)var.global); break;
        }

        return NEGATE_ON_CONDITION(returnValue, gameVar & GV_FLAG_NEGATIVE);
    }
}

#undef CHECK_INDEX

int __fastcall Gv_GetVar(int const gameVar, int const spriteNum, int const playerNum) { return getvar__(gameVar, spriteNum, playerNum); }
int __fastcall Gv_GetVar(int const gameVar) { return getvar__(gameVar, vm.spriteNum, vm.playerNum); }

void __fastcall Gv_GetManyVars(int const numVars, int32_t * const outBuf)
{
    for (native_t j = 0; j < numVars; ++j)
        outBuf[j] = getvar__(*insptr++, vm.spriteNum, vm.playerNum);
}

static FORCE_INLINE void __fastcall setvar__(int const gameVar, int const newValue, int const spriteNum, int const playerNum)
{
    gamevar_t &var = aGameVars[gameVar];
    switch (var.flags & (GAMEVAR_USER_MASK|GAMEVAR_PTR_MASK))
    {
        default: var.global = newValue; break;
        case GAMEVAR_PERPLAYER: var.pValues[playerNum & (MAXPLAYERS-1)] = newValue; break;
        case GAMEVAR_PERACTOR:  var.pValues[spriteNum & (MAXSPRITES-1)] = newValue; break;
        case GAMEVAR_RAWQ16PTR:
        case GAMEVAR_INT32PTR: *((int32_t *)var.global) = (int32_t)newValue; break;
        case GAMEVAR_INT16PTR: *((int16_t *)var.global) = (int16_t)newValue; break;
        case GAMEVAR_Q16PTR:    *(fix16_t *)var.global  = fix16_from_int((int16_t)newValue); break;
    }
}

void __fastcall Gv_SetVar(int const gameVar, int const newValue) { setvar__(gameVar, newValue, vm.spriteNum, vm.playerNum); }
void __fastcall Gv_SetVar(int const gameVar, int const newValue, int const spriteNum, int const playerNum)
{
    setvar__(gameVar, newValue, spriteNum, playerNum);
}

int Gv_GetVarByLabel(const char *szGameLabel, int const defaultValue, int const spriteNum, int const playerNum)
{
    int const gameVar = hash_find(&h_gamevars, szGameLabel);
    return EDUKE32_PREDICT_TRUE(gameVar >= 0) ? Gv_GetVar(gameVar, spriteNum, playerNum) : defaultValue;
}

static intptr_t *Gv_GetVarDataPtr(const char *szGameLabel)
{
    int const gameVar = hash_find(&h_gamevars, szGameLabel);

    if (EDUKE32_PREDICT_FALSE((unsigned)gameVar >= MAXGAMEVARS))
        return NULL;

    gamevar_t &var = aGameVars[gameVar];

    if (var.flags & (GAMEVAR_USER_MASK | GAMEVAR_PTR_MASK))
        return var.pValues;

    return &(var.global);
}

void Gv_ResetSystemDefaults(void)
{
    // call many times...
    char aszBuf[64];

    //AddLog("ResetWeaponDefaults");

    for (int weaponNum = 0; weaponNum < MAX_WEAPONS; ++weaponNum)
    {
        for (int playerNum = 0; playerNum < MAXPLAYERS; ++playerNum)
        {
            Bsprintf(aszBuf, "WEAPON%d_CLIP", weaponNum);
            aplWeaponClip[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_RELOAD", weaponNum);
            aplWeaponReload[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_FIREDELAY", weaponNum);
            aplWeaponFireDelay[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_TOTALTIME", weaponNum);
            aplWeaponTotalTime[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_HOLDDELAY", weaponNum);
            aplWeaponHoldDelay[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_FLAGS", weaponNum);
            aplWeaponFlags[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_SHOOTS", weaponNum);
            aplWeaponShoots[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            if ((unsigned)aplWeaponShoots[weaponNum][playerNum] >= MAXTILES)
                aplWeaponShoots[weaponNum][playerNum] = 0;
            Bsprintf(aszBuf, "WEAPON%d_SPAWNTIME", weaponNum);
            aplWeaponSpawnTime[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_SPAWN", weaponNum);
            aplWeaponSpawn[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_SHOTSPERBURST", weaponNum);
            aplWeaponShotsPerBurst[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_WORKSLIKE", weaponNum);
            aplWeaponWorksLike[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_INITIALSOUND", weaponNum);
            aplWeaponInitialSound[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_FIRESOUND", weaponNum);
            aplWeaponFireSound[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_SOUND2TIME", weaponNum);
            aplWeaponSound2Time[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_SOUND2SOUND", weaponNum);
            aplWeaponSound2Sound[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_RELOADSOUND1", weaponNum);
            aplWeaponReloadSound1[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_RELOADSOUND2", weaponNum);
            aplWeaponReloadSound2[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_SELECTSOUND", weaponNum);
            aplWeaponSelectSound[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
            Bsprintf(aszBuf, "WEAPON%d_FLASHCOLOR", weaponNum);
            aplWeaponFlashColor[weaponNum][playerNum] = Gv_GetVarByLabel(aszBuf, 0, -1, playerNum);
        }
    }

    g_aimAngleVarID  = Gv_GetVarIndex("AUTOAIMANGLE");
    g_angRangeVarID  = Gv_GetVarIndex("ANGRANGE");
    g_hitagVarID     = Gv_GetVarIndex("HITAG");
    g_lotagVarID     = Gv_GetVarIndex("LOTAG");
    g_returnVarID    = Gv_GetVarIndex("RETURN");
    g_structVarIDs   = Gv_GetVarIndex("sprite");
    g_textureVarID   = Gv_GetVarIndex("TEXTURE");
    g_thisActorVarID = Gv_GetVarIndex("THISACTOR");
    g_weaponVarID    = Gv_GetVarIndex("WEAPON");
    g_worksLikeVarID = Gv_GetVarIndex("WORKSLIKE");
    g_zRangeVarID    = Gv_GetVarIndex("ZRANGE");

    for (auto & tile : g_tile)
        if (tile.defproj)
            *tile.proj = *tile.defproj;

    static int constexpr statnumList[] = { STAT_DEFAULT, STAT_ACTOR, STAT_STANDABLE, STAT_MISC, STAT_ZOMBIEACTOR, STAT_FALLER, STAT_PLAYER };

    Bmemset(g_radiusDmgStatnums, 0, sizeof(g_radiusDmgStatnums));

    for (int i = 0; i < ARRAY_SSIZE(statnumList); ++i)
        bitmap_set(g_radiusDmgStatnums, statnumList[i]);

    //AddLog("EOF:ResetWeaponDefaults");
}

// Will set members that were overridden at CON translation time to 1.
// For example, if
//   gamevar WEAPON1_SHOOTS 2200 GAMEVAR_PERPLAYER
// was specified at file scope, g_weaponOverridden[1].Shoots will be 1.
weapondata_t g_weaponOverridden[MAX_WEAPONS];

static weapondata_t weapondefaults[MAX_WEAPONS] = {
    /*
        WorksLike, Clip, Reload, FireDelay, TotalTime, HoldDelay,
        Flags,
        Shoots, SpawnTime, Spawn, ShotsPerBurst, InitialSound, FireSound, Sound2Time, Sound2Sound,
        ReloadSound1, ReloadSound2, SelectSound, FlashColor
    */
#ifndef EDUKE32_STANDALONE
    {
        KNEE_WEAPON, 0, 0, 7, 14, 0,
        WEAPON_NOVISIBLE | WEAPON_RANDOMRESTART | WEAPON_AUTOMATIC,
        KNEE__, 0, 0, 0, 0, 0, 0,
        0, EJECT_CLIP__, INSERT_CLIP__, 0, 0
    },

    {
        PISTOL_WEAPON, 12, 27, 2, 5, 0,
        WEAPON_RELOAD_TIMING,
        SHOTSPARK1__, 2, SHELL__, 0, 0, PISTOL_FIRE__, 0, 0,
        EJECT_CLIP__, INSERT_CLIP__, INSERT_CLIP__, 255+(95<<8)
    },

    {
        SHOTGUN_WEAPON, 0, 13, 4, 30, 0,
        WEAPON_CHECKATRELOAD,
        SHOTGUN__, 24, SHOTGUNSHELL__, 7, 0, SHOTGUN_FIRE__, 15, SHOTGUN_COCK__,
        EJECT_CLIP__, INSERT_CLIP__, SHOTGUN_COCK__, 255+(95<<8)
    },

    {
        CHAINGUN_WEAPON, 0, 0, 3, 12, 3,
        WEAPON_AUTOMATIC | WEAPON_FIREEVERYTHIRD | WEAPON_AMMOPERSHOT | WEAPON_SPAWNTYPE3 | WEAPON_RESET,
        CHAINGUN__, 1, SHELL__, 0, 0, CHAINGUN_FIRE__, 0, 0,
        EJECT_CLIP__, INSERT_CLIP__, SELECT_WEAPON__, 255+(95<<8)
    },

    {
        RPG_WEAPON, 0, 0, 4, 20, 0,
        0,
        RPG__, 0, 0, 0, 0, 0, 0, 0,
        EJECT_CLIP__, INSERT_CLIP__, SELECT_WEAPON__, 255+(95<<8)
    },

    {
        HANDBOMB_WEAPON, 0, 30, 6, 19, 12,
        WEAPON_THROWIT,
        HEAVYHBOMB__, 0, 0, 0, 0, 0, 0,
        0, EJECT_CLIP__, INSERT_CLIP__, 0, 0
    },

    {
        SHRINKER_WEAPON, 0, 0, 10, 12, 0,
        WEAPON_GLOWS,
        SHRINKER__, 0, 0, 0, SHRINKER_FIRE__, 0, 0, 0,
        EJECT_CLIP__, INSERT_CLIP__, SELECT_WEAPON__, 176+(252<<8)+(120<<16)
    },

    {
        DEVISTATOR_WEAPON, 0, 0, 3, 6, 5,
        WEAPON_FIREEVERYOTHER | WEAPON_AMMOPERSHOT,
        RPG__, 0, 0, 2, CAT_FIRE__, 0, 0, 0,
        EJECT_CLIP__, INSERT_CLIP__, SELECT_WEAPON__, 255+(95<<8)
    },

    {
        TRIPBOMB_WEAPON, 0, 16, 3, 16, 7,
        WEAPON_NOVISIBLE | WEAPON_STANDSTILL | WEAPON_CHECKATRELOAD,
        HANDHOLDINGLASER__, 0, 0, 0, 0, 0, 0,
        0, EJECT_CLIP__, INSERT_CLIP__, 0, 0
    },

    {
        FREEZE_WEAPON, 0, 0, 3, 5, 0,
        WEAPON_RESET,
        FREEZEBLAST__, 0, 0, 0, CAT_FIRE__, CAT_FIRE__, 0, 0,
        EJECT_CLIP__, INSERT_CLIP__, SELECT_WEAPON__, 72+(88<<8)+(140<<16)
    },

    {
        HANDREMOTE_WEAPON, 0, 10, 2, 10, 0,
        WEAPON_BOMB_TRIGGER | WEAPON_NOVISIBLE,
        0, 0, 0, 0, 0, 0, 0,
        0, EJECT_CLIP__, INSERT_CLIP__, 0, 0
    },

    {
        GROW_WEAPON, 0, 0, 3, 5, 0,
        WEAPON_GLOWS,
        GROWSPARK__, 0, 0, 0, 0, EXPANDERSHOOT__, 0, 0,
        EJECT_CLIP__, INSERT_CLIP__, SELECT_WEAPON__, 216+(52<<8)+(20<<16)
    },

    {
        FLAMETHROWER_WEAPON, 0, 0, 2, 16, 0,
        WEAPON_RESET,
        FIREBALL__, 0, 0, 0, FLAMETHROWER_INTRO__, FLAMETHROWER_INTRO__, 0, 0,
        EJECT_CLIP__, INSERT_CLIP__, SELECT_WEAPON__, 216+(52<<8)+(20<<16)
    },
#endif
};

// KEEPINSYNC with what is contained above
// XXX: ugly
static int32_t G_StaticToDynamicTile(int32_t const tile)
{
    switch (tile)
    {
#ifndef EDUKE32_STANDALONE
    case CHAINGUN__: return CHAINGUN;
    case FREEZEBLAST__: return FREEZEBLAST;
    case GROWSPARK__: return GROWSPARK;
    case HANDHOLDINGLASER__: return HANDHOLDINGLASER;
    case HEAVYHBOMB__: return HEAVYHBOMB;
    case KNEE__: return KNEE;
    case RPG__: return RPG;
    case SHELL__: return SHELL;
    case SHOTGUNSHELL__: return SHOTGUNSHELL;
    case SHOTGUN__: return SHOTGUN;
    case SHOTSPARK1__: return SHOTSPARK1;
    case SHRINKER__: return SHRINKER;
#endif
    default: return tile;
    }
}

static int32_t G_StaticToDynamicSound(int32_t const sound)
{
    switch (sound)
    {
#ifndef EDUKE32_STANDALONE
    case CAT_FIRE__: return CAT_FIRE;
    case CHAINGUN_FIRE__: return CHAINGUN_FIRE;
    case EJECT_CLIP__: return EJECT_CLIP;
    case EXPANDERSHOOT__: return EXPANDERSHOOT;
    case INSERT_CLIP__: return INSERT_CLIP;
    case PISTOL_FIRE__: return PISTOL_FIRE;
    case SELECT_WEAPON__: return SELECT_WEAPON;
    case SHOTGUN_FIRE__: return SHOTGUN_FIRE;
    case SHOTGUN_COCK__: return SHOTGUN_COCK;
    case SHRINKER_FIRE__: return SHRINKER_FIRE;
#endif
    default: return sound;
    }
}

// Initialize WEAPONx_* gamevars
#define ADDWEAPONVAR(Weapidx, Membname) do { \
    Bsprintf(aszBuf, "WEAPON%d_" #Membname, Weapidx); \
    Bstrupr(aszBuf); \
    Gv_NewVar(aszBuf, weapondefaults[Weapidx].Membname, GAMEVAR_PERPLAYER | GAMEVAR_SYSTEM); \
} while (0)

// Finish a default weapon member after CON translation. If it was not
// overridden from CON itself (see example at g_weaponOverridden[]), we set
// both the weapondefaults[] entry (probably dead by now) and the live value.
#define FINISH_WEAPON_DEFAULT_X(What, i, Membname) do {  \
    if (!g_weaponOverridden[i].Membname) \
    { \
        weapondefaults[i].Membname = G_StaticToDynamic##What(weapondefaults[i].Membname); \
    } \
} while (0)

#define FINISH_WEAPON_DEFAULT_TILE(i, Membname) FINISH_WEAPON_DEFAULT_X(Tile, i, Membname)
#define FINISH_WEAPON_DEFAULT_SOUND(i, Membname) FINISH_WEAPON_DEFAULT_X(Sound, i, Membname)

// Process the dynamic {tile,sound} mappings after CON has been translated.
// We cannot do this before, because the dynamic maps are not yet set up then.
void Gv_FinalizeWeaponDefaults(void)
{
    for (int i=0; i<MAX_WEAPONS; i++)
    {
        FINISH_WEAPON_DEFAULT_TILE(i, Shoots);
        FINISH_WEAPON_DEFAULT_TILE(i, Spawn);

        FINISH_WEAPON_DEFAULT_SOUND(i, InitialSound);
        FINISH_WEAPON_DEFAULT_SOUND(i, FireSound);
        FINISH_WEAPON_DEFAULT_SOUND(i, ReloadSound1);
        FINISH_WEAPON_DEFAULT_SOUND(i, Sound2Sound);
        FINISH_WEAPON_DEFAULT_SOUND(i, ReloadSound2);
        FINISH_WEAPON_DEFAULT_SOUND(i, SelectSound);
    }
}
#undef FINISH_WEAPON_DEFAULT_SOUND
#undef FINISH_WEAPON_DEFAULT_TILE
#undef FINISH_WEAPON_DEFAULT_X

static int32_t lastvisinc;

static void Gv_AddSystemVars(void)
{
    // only call ONCE

    // special vars for struct access
    // KEEPINSYNC gamedef.h: enum QuickStructureAccess_t (including order)
    Gv_NewVar("sprite",         -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("__sprite__",     -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("__actor__",      -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("__spriteext__",  -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("sector",         -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("__sector__",     -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("wall",           -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("__wall__",       -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("player",         -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("__player__",     -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("actorvar",       -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("playervar",      -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("tspr",           -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("projectile",     -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("thisprojectile", -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("userdef",        -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("input",          -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("tiledata",       -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);
    Gv_NewVar("paldata",        -1, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_SPECIAL);

#ifndef EDUKE32_STANDALONE
    if (NAM_WW2GI)
    {
        weapondefaults[PISTOL_WEAPON].Clip   = 20;
        weapondefaults[PISTOL_WEAPON].Reload = 50;
        weapondefaults[PISTOL_WEAPON].Flags |= WEAPON_HOLSTER_CLEARS_CLIP;

        weapondefaults[SHRINKER_WEAPON].TotalTime = 30;

        weapondefaults[GROW_WEAPON].TotalTime = 30;

        if (WW2GI)
        {
            weapondefaults[KNEE_WEAPON].HoldDelay = 14;
            weapondefaults[KNEE_WEAPON].Reload    = 30;

            weapondefaults[PISTOL_WEAPON].Flags |= WEAPON_AUTOMATIC;

            weapondefaults[SHOTGUN_WEAPON].TotalTime = 31;

            weapondefaults[CHAINGUN_WEAPON].FireDelay = 1;
            weapondefaults[CHAINGUN_WEAPON].HoldDelay = 10;
            weapondefaults[CHAINGUN_WEAPON].Reload    = 30;
            weapondefaults[CHAINGUN_WEAPON].SpawnTime = 0;

            weapondefaults[RPG_WEAPON].Reload = 30;

            weapondefaults[DEVISTATOR_WEAPON].FireDelay     = 2;
            weapondefaults[DEVISTATOR_WEAPON].Flags         = WEAPON_FIREEVERYOTHER;
            weapondefaults[DEVISTATOR_WEAPON].Reload        = 30;
            weapondefaults[DEVISTATOR_WEAPON].ShotsPerBurst = 0;
            weapondefaults[DEVISTATOR_WEAPON].TotalTime     = 5;

            weapondefaults[TRIPBOMB_WEAPON].Flags     = WEAPON_STANDSTILL;
            weapondefaults[TRIPBOMB_WEAPON].HoldDelay = 0;
            weapondefaults[TRIPBOMB_WEAPON].Reload    = 30;

            weapondefaults[FREEZE_WEAPON].Flags = WEAPON_FIREEVERYOTHER;

            weapondefaults[HANDREMOTE_WEAPON].Reload = 30;

            weapondefaults[GROW_WEAPON].InitialSound = EXPANDERSHOOT;
        }
    }
#endif

    char aszBuf[64];

    for (int i=0; i<MAX_WEAPONS; i++)
    {
        ADDWEAPONVAR(i, Clip);
        ADDWEAPONVAR(i, FireDelay);
        ADDWEAPONVAR(i, FireSound);
        ADDWEAPONVAR(i, Flags);
        ADDWEAPONVAR(i, FlashColor);
        ADDWEAPONVAR(i, HoldDelay);
        ADDWEAPONVAR(i, InitialSound);
        ADDWEAPONVAR(i, Reload);
        ADDWEAPONVAR(i, ReloadSound1);
        ADDWEAPONVAR(i, ReloadSound2);
        ADDWEAPONVAR(i, SelectSound);
        ADDWEAPONVAR(i, Shoots);
        ADDWEAPONVAR(i, ShotsPerBurst);
        ADDWEAPONVAR(i, Sound2Sound);
        ADDWEAPONVAR(i, Sound2Time);
        ADDWEAPONVAR(i, Spawn);
        ADDWEAPONVAR(i, SpawnTime);
        ADDWEAPONVAR(i, TotalTime);
        ADDWEAPONVAR(i, WorksLike);
    }

#ifndef EDUKE32_STANDALONE
    Gv_NewVar("GRENADE_LIFETIME",      NAM_GRENADE_LIFETIME,                    GAMEVAR_SYSTEM | GAMEVAR_PERPLAYER);
    Gv_NewVar("GRENADE_LIFETIME_VAR",  NAM_GRENADE_LIFETIME_VAR,                GAMEVAR_SYSTEM | GAMEVAR_PERPLAYER);
    Gv_NewVar("PIPEBOMB_CONTROL", NAM_WW2GI ? PIPEBOMB_TIMER : PIPEBOMB_REMOTE, GAMEVAR_SYSTEM | GAMEVAR_PERPLAYER);
    Gv_NewVar("STICKYBOMB_LIFETIME",   NAM_GRENADE_LIFETIME,                    GAMEVAR_SYSTEM | GAMEVAR_PERPLAYER);
    Gv_NewVar("STICKYBOMB_LIFETIME_VAR", NAM_GRENADE_LIFETIME_VAR,              GAMEVAR_SYSTEM | GAMEVAR_PERPLAYER);
    Gv_NewVar("TRIPBOMB_CONTROL",      TRIPBOMB_TRIPWIRE,                       GAMEVAR_SYSTEM | GAMEVAR_PERPLAYER);
#endif

    Gv_NewVar("ANGRANGE",              18,                                      GAMEVAR_SYSTEM | GAMEVAR_PERPLAYER);
    Gv_NewVar("AUTOAIMANGLE",          0,                                       GAMEVAR_SYSTEM | GAMEVAR_PERPLAYER);
    Gv_NewVar("COOP",                  (intptr_t)&ud.coop,                      GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("FFIRE",                 (intptr_t)&ud.ffire,                     GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("HITAG",                 0,                                       GAMEVAR_SYSTEM);
    Gv_NewVar("LEVEL",                 (intptr_t)&ud.level_number,              GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);
    Gv_NewVar("LOTAG",                 0,                                       GAMEVAR_SYSTEM);
    Gv_NewVar("MARKER",                (intptr_t)&ud.marker,                    GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("MONSTERS_OFF",          (intptr_t)&ud.monsters_off,              GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("MULTIMODE",             (intptr_t)&ud.multimode,                 GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("NUMSECTORS",            (intptr_t)&numsectors,                   GAMEVAR_SYSTEM | GAMEVAR_INT16PTR | GAMEVAR_READONLY);
    Gv_NewVar("NUMWALLS",              (intptr_t)&numwalls,                     GAMEVAR_SYSTEM | GAMEVAR_INT16PTR | GAMEVAR_READONLY);
    Gv_NewVar("Numsprites",            (intptr_t)&Numsprites,                   GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);
    Gv_NewVar("RESPAWN_INVENTORY",     (intptr_t)&ud.respawn_inventory,         GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("RESPAWN_ITEMS",         (intptr_t)&ud.respawn_items,             GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("RESPAWN_MONSTERS",      (intptr_t)&ud.respawn_monsters,          GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("RETURN",                0,                                       GAMEVAR_SYSTEM);
    Gv_NewVar("TEXTURE",               0,                                       GAMEVAR_SYSTEM);
    Gv_NewVar("THISACTOR",             0,                                       GAMEVAR_SYSTEM | GAMEVAR_READONLY);
    Gv_NewVar("VOLUME",                (intptr_t)&ud.volume_number,             GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);
    Gv_NewVar("WEAPON",                0,                                       GAMEVAR_SYSTEM | GAMEVAR_PERPLAYER | GAMEVAR_READONLY);
    Gv_NewVar("WORKSLIKE",             0,                                       GAMEVAR_SYSTEM | GAMEVAR_PERPLAYER | GAMEVAR_READONLY);
    Gv_NewVar("ZRANGE",                4,                                       GAMEVAR_SYSTEM | GAMEVAR_PERPLAYER);

    Gv_NewVar("automapping",           (intptr_t)&automapping,                  GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("cameraang",             (intptr_t)&ud.cameraq16ang,              GAMEVAR_SYSTEM | GAMEVAR_Q16PTR);
    Gv_NewVar("cameraclock",           (intptr_t)&g_cameraClock,                GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("cameradist",            (intptr_t)&g_cameraDistance,             GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("camerahoriz",           (intptr_t)&ud.cameraq16horiz,            GAMEVAR_SYSTEM | GAMEVAR_Q16PTR);
    Gv_NewVar("cameraq16ang",          (intptr_t)&ud.cameraq16ang,              GAMEVAR_SYSTEM | GAMEVAR_RAWQ16PTR);
    Gv_NewVar("cameraq16horiz",        (intptr_t)&ud.cameraq16horiz,            GAMEVAR_SYSTEM | GAMEVAR_RAWQ16PTR);
    Gv_NewVar("camerasect",            (intptr_t)&ud.camerasect,                GAMEVAR_SYSTEM | GAMEVAR_INT16PTR);
    Gv_NewVar("camerax",               (intptr_t)&ud.camerapos.x,               GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("cameray",               (intptr_t)&ud.camerapos.y,               GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("cameraz",               (intptr_t)&ud.camerapos.z,               GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("current_menu",          (intptr_t)&g_currentMenu,                GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);
    Gv_NewVar("currentweapon",         (intptr_t)&hudweap.cur,                  GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("display_mirror",        (intptr_t)&display_mirror,               GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("framerate",             (intptr_t)&g_frameRate,                  GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);
    Gv_NewVar("gametype_flags",        (intptr_t)&g_gametypeFlags[ud.coop],     GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("gravitationalconstant", (intptr_t)&g_spriteGravity,              GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("gs",                    (intptr_t)&hudweap.shade,                GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("gun_pos",               (intptr_t)&hudweap.gunposy,              GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("lastsavepos",           (intptr_t)&g_lastAutoSaveArbitraryID,    GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("lastvisinc",            (intptr_t)&lastvisinc,                   GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("looking_angSR1",        (intptr_t)&hudweap.lookhalfang,          GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("looking_arc",           (intptr_t)&hudweap.lookhoriz,            GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("myconnectindex",        (intptr_t)&myconnectindex,               GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);
    Gv_NewVar("numplayers",            (intptr_t)&numplayers,                   GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);
    Gv_NewVar("numsectors",            (intptr_t)&numsectors,                   GAMEVAR_SYSTEM | GAMEVAR_INT16PTR | GAMEVAR_READONLY);
    Gv_NewVar("randomseed",            (intptr_t)&randomseed,                   GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("screenpeek",            (intptr_t)&screenpeek,                   GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);
    Gv_NewVar("totalclock",            (intptr_t)&totalclock,                   GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);
    Gv_NewVar("viewingrange",          (intptr_t)&viewingrange,                 GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);
    Gv_NewVar("weapon_xoffset",        (intptr_t)&hudweap.gunposx,              GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("weaponcount",           (intptr_t)&hudweap.count,                GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
    Gv_NewVar("windowx1",              (intptr_t)&windowxy1.x,                  GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);
    Gv_NewVar("windowx2",              (intptr_t)&windowxy2.x,                  GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);
    Gv_NewVar("windowy1",              (intptr_t)&windowxy1.y,                  GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);
    Gv_NewVar("windowy2",              (intptr_t)&windowxy2.y,                  GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);
    Gv_NewVar("xdim",                  (intptr_t)&xdim,                         GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);
    Gv_NewVar("ydim",                  (intptr_t)&ydim,                         GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);
    Gv_NewVar("yxaspect",              (intptr_t)&yxaspect,                     GAMEVAR_SYSTEM | GAMEVAR_INT32PTR | GAMEVAR_READONLY);

#ifdef USE_OPENGL
    Gv_NewVar("rendmode", (intptr_t)&rendmode, GAMEVAR_READONLY | GAMEVAR_SYSTEM | GAMEVAR_INT32PTR);
#else
    Gv_NewVar("rendmode", 0, GAMEVAR_READONLY | GAMEVAR_SYSTEM);
#endif

    // SYSTEM_GAMEARRAY
    Gv_NewArray("gotpic",            (void *)&gotpic[0],              MAXTILES,   GAMEARRAY_SYSTEM | GAMEARRAY_BITMAP);
    Gv_NewArray("gotsector",         (void *)&gotsector[0],           MAXSECTORS, GAMEARRAY_SYSTEM | GAMEARRAY_BITMAP);
    Gv_NewArray("radiusdmgstatnums", (void *)&g_radiusDmgStatnums[0], MAXSTATUS,  GAMEARRAY_SYSTEM | GAMEARRAY_BITMAP);
    Gv_NewArray("show2dsector",      (void *)&show2dsector[0],        MAXSECTORS, GAMEARRAY_SYSTEM | GAMEARRAY_BITMAP);
    Gv_NewArray("tilesizx",          (void *)&tilesiz[0].x,           MAXTILES,   GAMEARRAY_SYSTEM | GAMEARRAY_STRIDE2 | GAMEARRAY_READONLY | GAMEARRAY_INT16);
    Gv_NewArray("tilesizy",          (void *)&tilesiz[0].y,           MAXTILES,   GAMEARRAY_SYSTEM | GAMEARRAY_STRIDE2 | GAMEARRAY_READONLY | GAMEARRAY_INT16);
}

#undef ADDWEAPONVAR

void Gv_Init(void)
{
    // already initialized
    if (aGameVars[0].flags)
        return;

    // Set up weapon defaults, g_playerWeapon[][].
    Gv_AddSystemVars();
    Gv_InitWeaponPointers();
    Gv_ResetSystemDefaults();
}

void Gv_InitWeaponPointers(void)
{
    char aszBuf[64];
    // called from game Init AND when level is loaded...

    //AddLog("Gv_InitWeaponPointers");

    for (int i=(MAX_WEAPONS-1); i>=0; i--)
    {
        Bsprintf(aszBuf, "WEAPON%d_CLIP", i);
        aplWeaponClip[i] = Gv_GetVarDataPtr(aszBuf);

        if (!aplWeaponClip[i])
        {
            LOG_F(ERROR, "NULL weapon! WTF?! %s", aszBuf);
            // Bexit(EXIT_SUCCESS);
            G_Shutdown();
        }

        Bsprintf(aszBuf, "WEAPON%d_RELOAD", i);
        aplWeaponReload[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_FIREDELAY", i);
        aplWeaponFireDelay[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_TOTALTIME", i);
        aplWeaponTotalTime[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_HOLDDELAY", i);
        aplWeaponHoldDelay[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_FLAGS", i);
        aplWeaponFlags[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_SHOOTS", i);
        aplWeaponShoots[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_SPAWNTIME", i);
        aplWeaponSpawnTime[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_SPAWN", i);
        aplWeaponSpawn[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_SHOTSPERBURST", i);
        aplWeaponShotsPerBurst[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_WORKSLIKE", i);
        aplWeaponWorksLike[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_INITIALSOUND", i);
        aplWeaponInitialSound[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_FIRESOUND", i);
        aplWeaponFireSound[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_SOUND2TIME", i);
        aplWeaponSound2Time[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_SOUND2SOUND", i);
        aplWeaponSound2Sound[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_RELOADSOUND1", i);
        aplWeaponReloadSound1[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_RELOADSOUND2", i);
        aplWeaponReloadSound2[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_SELECTSOUND", i);
        aplWeaponSelectSound[i] = Gv_GetVarDataPtr(aszBuf);
        Bsprintf(aszBuf, "WEAPON%d_FLASHCOLOR", i);
        aplWeaponFlashColor[i] = Gv_GetVarDataPtr(aszBuf);
    }
}

void Gv_RefreshPointers(void)
{
    aGameVars[Gv_GetVarIndex("COOP")].global              = (intptr_t)&ud.coop;
    aGameVars[Gv_GetVarIndex("FFIRE")].global             = (intptr_t)&ud.ffire;
    aGameVars[Gv_GetVarIndex("LEVEL")].global             = (intptr_t)&ud.level_number;
    aGameVars[Gv_GetVarIndex("MARKER")].global            = (intptr_t)&ud.marker;
    aGameVars[Gv_GetVarIndex("MONSTERS_OFF")].global      = (intptr_t)&ud.monsters_off;
    aGameVars[Gv_GetVarIndex("MULTIMODE")].global         = (intptr_t)&ud.multimode;
    aGameVars[Gv_GetVarIndex("NUMSECTORS")].global        = (intptr_t)&numsectors;
    aGameVars[Gv_GetVarIndex("NUMWALLS")].global          = (intptr_t)&numwalls;
    aGameVars[Gv_GetVarIndex("Numsprites")].global        = (intptr_t)&Numsprites;
    aGameVars[Gv_GetVarIndex("RESPAWN_INVENTORY")].global = (intptr_t)&ud.respawn_inventory;
    aGameVars[Gv_GetVarIndex("RESPAWN_ITEMS")].global     = (intptr_t)&ud.respawn_items;
    aGameVars[Gv_GetVarIndex("RESPAWN_MONSTERS")].global  = (intptr_t)&ud.respawn_monsters;
    aGameVars[Gv_GetVarIndex("VOLUME")].global            = (intptr_t)&ud.volume_number;

    aGameVars[Gv_GetVarIndex("automapping")].global       = (intptr_t)&automapping;
    aGameVars[Gv_GetVarIndex("cameraang")].global         = (intptr_t)&ud.cameraq16ang;  // XXX FIXME
    aGameVars[Gv_GetVarIndex("cameraclock")].global       = (intptr_t)&g_cameraClock;
    aGameVars[Gv_GetVarIndex("cameradist")].global        = (intptr_t)&g_cameraDistance;
    aGameVars[Gv_GetVarIndex("camerahoriz")].global       = (intptr_t)&ud.cameraq16horiz;  // XXX FIXME
    aGameVars[Gv_GetVarIndex("cameraq16ang")].global      = (intptr_t)&ud.cameraq16ang;    // XXX FIXME
    aGameVars[Gv_GetVarIndex("cameraq16horiz")].global    = (intptr_t)&ud.cameraq16horiz;  // XXX FIXME
    aGameVars[Gv_GetVarIndex("camerasect")].global        = (intptr_t)&ud.camerasect;
    aGameVars[Gv_GetVarIndex("camerax")].global           = (intptr_t)&ud.camerapos.x;
    aGameVars[Gv_GetVarIndex("cameray")].global           = (intptr_t)&ud.camerapos.y;
    aGameVars[Gv_GetVarIndex("cameraz")].global           = (intptr_t)&ud.camerapos.z;
    aGameVars[Gv_GetVarIndex("current_menu")].global      = (intptr_t)&g_currentMenu;
    aGameVars[Gv_GetVarIndex("currentweapon")].global     = (intptr_t)&hudweap.cur;
    aGameVars[Gv_GetVarIndex("display_mirror")].global    = (intptr_t)&display_mirror;
    aGameVars[Gv_GetVarIndex("framerate")].global         = (intptr_t)&g_frameRate;
    aGameVars[Gv_GetVarIndex("gametype_flags")].global    = (intptr_t)&g_gametypeFlags[ud.coop];
    aGameVars[Gv_GetVarIndex("gravitationalconstant")].global =
                                                            (intptr_t)&g_spriteGravity;
    aGameVars[Gv_GetVarIndex("gs")].global                = (intptr_t)&hudweap.shade;
    aGameVars[Gv_GetVarIndex("gun_pos")].global           = (intptr_t)&hudweap.gunposy;
    aGameVars[Gv_GetVarIndex("lastsavepos")].global       = (intptr_t)&g_lastAutoSaveArbitraryID;
    aGameVars[Gv_GetVarIndex("lastvisinc")].global        = (intptr_t)&lastvisinc;
    aGameVars[Gv_GetVarIndex("looking_angSR1")].global    = (intptr_t)&hudweap.lookhalfang;
    aGameVars[Gv_GetVarIndex("looking_arc")].global       = (intptr_t)&hudweap.lookhoriz;
    aGameVars[Gv_GetVarIndex("myconnectindex")].global    = (intptr_t)&myconnectindex;
    aGameVars[Gv_GetVarIndex("numplayers")].global        = (intptr_t)&numplayers;
    aGameVars[Gv_GetVarIndex("numsectors")].global        = (intptr_t)&numsectors;
    aGameVars[Gv_GetVarIndex("randomseed")].global        = (intptr_t)&randomseed;
    aGameVars[Gv_GetVarIndex("screenpeek")].global        = (intptr_t)&screenpeek;
    aGameVars[Gv_GetVarIndex("totalclock")].global        = (intptr_t)&totalclock;
    aGameVars[Gv_GetVarIndex("viewingrange")].global      = (intptr_t)&viewingrange;
    aGameVars[Gv_GetVarIndex("weapon_xoffset")].global    = (intptr_t)&hudweap.gunposx;
    aGameVars[Gv_GetVarIndex("weaponcount")].global       = (intptr_t)&hudweap.count;
    aGameVars[Gv_GetVarIndex("windowx1")].global          = (intptr_t)&windowxy1.x;
    aGameVars[Gv_GetVarIndex("windowx2")].global          = (intptr_t)&windowxy2.x;
    aGameVars[Gv_GetVarIndex("windowy1")].global          = (intptr_t)&windowxy1.y;
    aGameVars[Gv_GetVarIndex("windowy2")].global          = (intptr_t)&windowxy2.y;
    aGameVars[Gv_GetVarIndex("xdim")].global              = (intptr_t)&xdim;
    aGameVars[Gv_GetVarIndex("ydim")].global              = (intptr_t)&ydim;
    aGameVars[Gv_GetVarIndex("yxaspect")].global          = (intptr_t)&yxaspect;

#ifdef USE_OPENGL
    aGameVars[Gv_GetVarIndex("rendmode")].global = (intptr_t)&rendmode;
#endif

    aGameArrays[Gv_GetArrayIndex("gotpic")].pValues = (intptr_t *)&gotpic[0];
    aGameArrays[Gv_GetArrayIndex("tilesizx")].pValues = (intptr_t *)&tilesiz[0].x;
    aGameArrays[Gv_GetArrayIndex("tilesizy")].pValues = (intptr_t *)&tilesiz[0].y;
}
