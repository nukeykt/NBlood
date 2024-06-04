//-------------------------------------------------------------------------
/*
 Copyright (C) 2007 Jonathon Fowler <jf@jonof.id.au>

 This file is part of JFShadowWarrior

 Shadow Warrior is free software; you can redistribute it and/or
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
 */
//-------------------------------------------------------------------------

#include "build.h"
#include "baselayer.h"

#include "scriptfile.h"
#include "cache1d.h"
#include "crc32.h"

#include "grpscan.h"
#include "common_game.h"

#define SWREG12_CRC 0x7545319Fu
#define SWWD_CRC 0xA9AAA7B7u
#define SWTD_CRC 0xA1A65BE8u

static void process_tdragongrp(int32_t crcval)
{
    krename(crcval, 53, "TDCUSTOM.TXT");
}

static internalgrpfile grpfiles[] =
{
    { "Shadow Warrior",               SWREG12_CRC, 47536148, 0, 0, 0, nullptr },
    { "Shadow Warrior (Europe)",      0xD4A1E153u, 47536148, 0, 0, 0, nullptr }, // only difference: corrupt tile #28
    { "Shadow Warrior (UK)",          0x3EE68767u, 47536148, 0, 0, 0, nullptr }, // only differences: corrupt tiles #2811, #4349
    { "Shadow Warrior (Censored)",    0x1A8776D2u, 47537951, 0, 0, 0, nullptr }, // only difference: added CREDITS.TXT
    { "Shadow Warrior Shareware 1.0", 0xDAA6BECEu, 25702245, GAMEFLAG_SHAREWARE, 0, 0, nullptr },
    { "Shadow Warrior Shareware 1.1", 0xF21A6B35u, 25833456, GAMEFLAG_SHAREWARE, 0, 0, nullptr },
    { "Shadow Warrior Shareware 1.2", 0x08A7FA1Fu, 26056769, GAMEFLAG_SHAREWARE, 0, 0, nullptr },
    { "Shadow Warrior Mac Demo",      0x4227F535u, 26056769, GAMEFLAG_SHAREWARE, 0, 0, nullptr },
    { "Wanton Destruction",           SWWD_CRC, 48698128, GAMEFLAG_SWWD, GRP_HAS_DEPENDENCY, SWREG12_CRC, nullptr },
    { "Twin Dragon",                  SWTD_CRC, 12499012, GAMEFLAG_SWTD, GRP_HAS_DEPENDENCY, SWREG12_CRC, process_tdragongrp },
    { "Twin Dragon",                  0xACC8DCDEu, 6235578, GAMEFLAG_SWTD, GRP_HAS_DEPENDENCY, SWREG12_CRC, nullptr },
    { "Twin Dragon",                  0xB5B71277u, 6236287, GAMEFLAG_SWTD, GRP_HAS_DEPENDENCY, SWREG12_CRC, nullptr }, // commonly available repack
};
grpfile *foundgrps = NULL;

#define GRPCACHEFILE "grpfiles.cache"
static struct grpcache
{
    struct grpcache *next;
    char name[BMAX_PATH+1];
    int size;
    int mtime;
    uint32_t crcval;
} *grpcache = NULL, *usedgrpcache = NULL;

static int LoadGroupsCache(void)
{
    struct grpcache *fg;

    int fsize, fmtime, fcrcval;
    char *fname;

    scriptfile *script;

    script = scriptfile_fromfile(GRPCACHEFILE);
    if (!script) return -1;

    while (!scriptfile_eof(script))
    {
        if (scriptfile_getstring(script, &fname)) break;    // filename
        if (scriptfile_getnumber(script, &fsize)) break;    // filesize
        if (scriptfile_getnumber(script, &fmtime)) break;   // modification time
        if (scriptfile_getnumber(script, &fcrcval)) break;  // crc checksum

        fg = (struct grpcache*)Xcalloc(1, sizeof(struct grpcache));
        fg->next = grpcache;
        grpcache = fg;

        strncpy(fg->name, fname, BMAX_PATH);
        fg->size = fsize;
        fg->mtime = fmtime;
        fg->crcval = fcrcval;
    }

    scriptfile_close(script);
    return 0;
}

static void FreeGroupsCache(void)
{
    struct grpcache *fg;

    while (grpcache)
    {
        fg = grpcache->next;
        Xfree(grpcache);
        grpcache = fg;
    }
}

static void RemoveGroup(grpfile_t *igrp)
{
    for (grpfile_t *prev = NULL, *grp = foundgrps; grp; grp=grp->next)
    {
        if (grp == igrp)
        {
            if (grp == foundgrps)
                foundgrps = grp->next;
            else
                prev->next = grp->next;

            Xfree(grp->filename);
            Xfree(grp);

            return;
        }

        prev = grp;
    }
}

grpfile_t * FindGroup(uint32_t crcval)
{
    grpfile_t *grp;

    for (grp = foundgrps; grp; grp=grp->next)
    {
        if (grp->type->crcval == crcval)
            return grp;
    }

    return NULL;
}

static struct internalgrpfile const * FindGrpInfo(uint32_t crcval, int32_t size)
{
    for (struct internalgrpfile const & grptype : grpfiles)
    {
        if (grptype.crcval == crcval && grptype.size == size)
            return &grptype;
    }

    return NULL;
}

static void ProcessGroups(BUILDVFS_FIND_REC *srch, native_t maxsize)
{
    BUILDVFS_FIND_REC *sidx;
    struct grpcache *fg, *fgg;
    char *fn;
    struct Bstat st;

    static constexpr int ReadSize = 65536;

    auto buf = (uint8_t *)Xmalloc(ReadSize);

    for (sidx = srch; sidx; sidx = sidx->next)
    {
        for (fg = grpcache; fg; fg = fg->next)
        {
            if (!Bstrcmp(fg->name, sidx->name)) break;
        }

        if (fg)
        {
            if (findfrompath(sidx->name, &fn)) continue; // failed to resolve the filename
            if (Bstat(fn, &st))
            {
                Xfree(fn);
                continue;
            } // failed to stat the file
            Xfree(fn);
            if (st.st_size && fg->size == (int32_t)st.st_size && fg->mtime == (int32_t)st.st_mtime)
            {
                auto const grptype = FindGrpInfo(fg->crcval, fg->size);

                if (grptype)
                {
                    auto const grp = (struct grpfile *)Xcalloc(1, sizeof(struct grpfile));
                    grp->filename = Xstrdup(sidx->name);
                    grp->type = grptype;
                    grp->next = foundgrps;
                    foundgrps = grp;
                }

                fgg = (struct grpcache *)Xcalloc(1, sizeof(struct grpcache));
                strcpy(fgg->name, fg->name);
                fgg->size = fg->size;
                fgg->mtime = fg->mtime;
                fgg->crcval = fg->crcval;
                fgg->next = usedgrpcache;
                usedgrpcache = fgg;
                continue;
            }
        }

        {
            int32_t b, fh;
            int32_t crcval = 0;

            fh = openfrompath(sidx->name, BO_RDONLY|BO_BINARY, BS_IREAD);
            if (fh < 0) continue;
            if (Bfstat(fh, &st)) continue;
            if (!st.st_size || st.st_size > maxsize) continue;

            DLOG_F(INFO, " Checksumming %s...", sidx->name);
            do
            {
                b = read(fh, buf, ReadSize);
                if (b > 0) crcval = Bcrc32(buf, b, crcval);
            }
            while (b == ReadSize);
            close(fh);

            LOG_F(INFO, " %s has checksum 0x%08x", sidx->name, crcval);

            auto const grptype = FindGrpInfo(crcval, st.st_size);

            if (grptype)
            {
                auto const grp = (struct grpfile *)Xcalloc(1, sizeof(struct grpfile));
                grp->filename = Xstrdup(sidx->name);
                grp->type = grptype;
                grp->next = foundgrps;
                foundgrps = grp;
            }

            fgg = (struct grpcache *)Xcalloc(1, sizeof(struct grpcache));
            Bstrncpyz(fgg->name, sidx->name, BMAX_PATH);
            fgg->size = st.st_size;
            fgg->mtime = st.st_mtime;
            fgg->crcval = crcval;
            fgg->next = usedgrpcache;
            usedgrpcache = fgg;
        }
    }

    Xfree(buf);
}

int ScanGroups(void)
{
    struct grpcache *fg, *fgg;

    LOG_F(INFO, "Searching for game data...");

    LoadGroupsCache();

    native_t maxsize = 0;
    for (struct internalgrpfile const & grptype : grpfiles)
    {
        if (maxsize < grptype.size)
            maxsize = grptype.size;
    }

    static char const * extensions[] =
    {
        "*.grp",
        "*.ssi",
        "*.dat",
        "*.zip",
    };

    for (char const * extension : extensions)
    {
        BUILDVFS_FIND_REC *srch = klistpath("/", extension, BUILDVFS_FIND_FILE);
        ProcessGroups(srch, maxsize);
        klistfree(srch);
    }

    FreeGroupsCache();

    for (grpfile_t *grp = foundgrps; grp; grp=grp->next)
    {
        if (grp->type->flags & GRP_HAS_DEPENDENCY)
        {
            if (FindGroup(grp->type->dependency) == NULL) // couldn't find dependency
            {
                RemoveGroup(grp);
                grp = foundgrps;
                // start from the beginning so we can remove anything that depended on this grp
                continue;
            }
        }
    }

    if (usedgrpcache)
    {
        buildvfs_FILE fp = buildvfs_fopen_write(GRPCACHEFILE);
        if (fp)
        {
            for (fg = usedgrpcache; fg; fg=fgg)
            {
                fgg = fg->next;
                fprintf(fp, "\"%s\" %d %d %d\n", fg->name, fg->size, fg->mtime, fg->crcval);
                Xfree(fg);
            }
            buildvfs_fclose(fp);
        }
    }

    return 0;
}

void FreeGroups(void)
{
    struct grpfile *fg;

    while (foundgrps)
    {
        fg = foundgrps->next;
        Xfree(foundgrps->filename);
        Xfree(foundgrps);
        foundgrps = fg;
    }
}

void SW_LoadAddon()
{
#ifndef EDUKE32_STANDALONE
    uint32_t crc;

    switch (g_addonNum)
    {
    case 0:
        crc = SWREG12_CRC;
        break;
    case 1:
        crc = SWWD_CRC;
        break;
    case 2:
        crc = SWTD_CRC;
        break;
    default:
        return;
    }

    grpfile_t const * const grp = FindGroup(crc);

    if (grp)
        g_selectedGrp = grp;
#endif
}
