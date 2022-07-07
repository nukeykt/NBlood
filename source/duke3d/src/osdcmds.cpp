//-------------------------------------------------------------------------
/*
Copyright (C) 2010 EDuke32 developers and contributors

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

#include "osdcmds.h"

#include "cheats.h"
#include "cmdline.h"
#include "demo.h"  // g_firstDemoFile[]
#include "duke3d.h"
#include "menus.h"
#include "minicoro.h"
#include "osdfuncs.h"
#include "savegame.h"
#include "sbar.h"

#ifdef EDUKE32_TOUCH_DEVICES
#include "in_android.h"
#endif

#include "vfs.h"

struct osdcmd_cheatsinfo osdcmd_cheatsinfo_stat;
float r_ambientlight = 1.0, r_ambientlightrecip = 1.0;
int32_t r_pr_defaultlights = 1;

uint32_t cl_cheatmask;

static inline int osdcmd_quit(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);
    OSD_ShowDisplay(0);
    G_GameQuit();
    return OSDCMD_OK;
}

static int osdcmd_changelevel(osdcmdptr_t parm)
{
    int volume = 0;
    int level;

    if (!VOLUMEONE)
    {
        if (parm->numparms != 2) return OSDCMD_SHOWHELP;

        volume = strtol(parm->parms[0], NULL, 10) - 1;
        level = strtol(parm->parms[1], NULL, 10) - 1;
    }
    else
    {
        if (parm->numparms != 1) return OSDCMD_SHOWHELP;

        level = strtol(parm->parms[0], NULL, 10) - 1;
    }

    if (volume < 0 || level < 0)
        return OSDCMD_SHOWHELP;

    if (level > MAXLEVELS || g_mapInfo[volume * MAXLEVELS + level].filename == NULL)
    {
        LOG_F(WARNING, "changelevel: no map defined for episode %d level %d", volume + 1, level + 1);
        return OSDCMD_SHOWHELP;
    }

    if (numplayers > 1)
    {
        /*
        if (g_netServer)
            Net_NewGame(volume,level);
        else if (voting == -1)
        {
            ud.m_volume_number = volume;
            ud.m_level_number = level;

            if (g_player[myconnectindex].ps->i)
            {
                int32_t i;

                for (i=0; i<MAXPLAYERS; i++)
                {
                    g_player[i].vote = 0;
                    g_player[i].gotvote = 0;
                }

                g_player[myconnectindex].vote = g_player[myconnectindex].gotvote = 1;

                voting = myconnectindex;

                tempbuf[0] = PACKET_MAP_VOTE_INITIATE;
                tempbuf[1] = myconnectindex;
                tempbuf[2] = ud.m_volume_number;
                tempbuf[3] = ud.m_level_number;

                enet_peer_send(g_netClientPeer, CHAN_GAMESTATE, enet_packet_create(tempbuf, 4, ENET_PACKET_FLAG_RELIABLE));
            }
            if ((g_gametypeFlags[ud.m_coop] & GAMETYPE_PLAYERSFRIENDLY) && !(g_gametypeFlags[ud.m_coop] & GAMETYPE_TDM))
                ud.m_noexits = 0;

            M_OpenMenu(myconnectindex);
            Menu_Change(MENU_NETWAITVOTES);
        }
        */
        return OSDCMD_OK;
    }

    if (g_player[myconnectindex].ps->gm & MODE_GAME)
    {
        // in-game behave like a cheat
        osdcmd_cheatsinfo_stat.cheatnum = CHEAT_SCOTTY;
        osdcmd_cheatsinfo_stat.volume   = volume;
        osdcmd_cheatsinfo_stat.level    = level;
    }
    else
    {
        // out-of-game behave like a menu command
        osdcmd_cheatsinfo_stat.cheatnum = -1;

        ud.m_volume_number     = volume;
        ud.m_level_number      = level;

        ud.m_monsters_off      = 0;
        ud.monsters_off        = 0;

        ud.m_respawn_items     = 0;
        ud.m_respawn_inventory = 0;

        ud.multimode           = 1;

        G_NewGame_EnterLevel();
    }

    return OSDCMD_OK;
}

static int osdcmd_map(osdcmdptr_t parm)
{
    char filename[BMAX_PATH];

    const int32_t wildcardp = parm->numparms==1 &&
        (Bstrchr(parm->parms[0], '*') != NULL);

    if (parm->numparms != 1 || wildcardp)
    {
        BUILDVFS_FIND_REC *r;
        fnlist_t fnlist = FNLIST_INITIALIZER;
        int maxwidth = 0;

        if (wildcardp)
            maybe_append_ext(filename, sizeof(filename), parm->parms[0], ".map");
        else
            Bstrcpy(filename, "*.MAP");

        fnlist_getnames(&fnlist, "/", filename, -1, 0);

        for (r=fnlist.findfiles; r; r=r->next)
            maxwidth = max<int>(maxwidth, Bstrlen(r->name));

        if (maxwidth > 0)
        {
            int const cols = OSD_GetCols();
            int x = 0;
            maxwidth += 3;

            auto buf = (char*)Balloca(cols+maxwidth);
            buf[0] = 0;

            LOG_F(INFO, "Map listing:");

            auto buf2 = (char*)Balloca(cols);

            for (r=fnlist.findfiles; r; r=r->next)
            {
                buf2[0] = 0;

                size_t inc = 0;

                inc = Bsnprintf(buf2, cols, "%-*s", maxwidth, r->name);

                Bstrcat(buf, buf2);

                x += inc;

                if (x > cols-maxwidth)
                {
                    x = 0;
                    OSD_Printf("%s\n", buf);
                    buf[0] = 0;
                }
            }

            if (x)
                OSD_Printf("%s\n", buf);

            LOG_F(INFO, "Found %d maps.", fnlist.numfiles);
        }

        fnlist_clearnames(&fnlist);

        return OSDCMD_SHOWHELP;
    }

    maybe_append_ext(filename, sizeof(filename), parm->parms[0], ".map");

    buildvfs_kfd ii;
    if ((ii = kopen4loadfrommod(filename,0)) == buildvfs_kfd_invalid)
    {
        LOG_F(ERROR, "map: file %s not found.", filename);
        return OSDCMD_OK;
    }
    kclose(ii);

    boardfilename[0] = '/';
    boardfilename[1] = 0;
    strcat(boardfilename, filename);

    if (numplayers > 1)
    {
        /*
        if (g_netServer)
        {
            Net_SendUserMapName();
            ud.m_volume_number = 0;
            ud.m_level_number = 7;
            Net_NewGame(ud.m_volume_number, ud.m_level_number);
        }
        else if (voting == -1)
        {
            Net_SendUserMapName();

            ud.m_volume_number = 0;
            ud.m_level_number = 7;

            if (g_player[myconnectindex].ps->i)
            {
                int32_t i;

                for (i=0; i<MAXPLAYERS; i++)
                {
                    g_player[i].vote = 0;
                    g_player[i].gotvote = 0;
                }

                g_player[myconnectindex].vote = g_player[myconnectindex].gotvote = 1;
                voting = myconnectindex;

                tempbuf[0] = PACKET_MAP_VOTE_INITIATE;
                tempbuf[1] = myconnectindex;
                tempbuf[2] = ud.m_volume_number;
                tempbuf[3] = ud.m_level_number;

                enet_peer_send(g_netClientPeer, CHAN_GAMESTATE, enet_packet_create(tempbuf, 4, ENET_PACKET_FLAG_RELIABLE));
            }
            if ((g_gametypeFlags[ud.m_coop] & GAMETYPE_PLAYERSFRIENDLY) && !(g_gametypeFlags[ud.m_coop] & GAMETYPE_TDM))
                ud.m_noexits = 0;

            M_OpenMenu(myconnectindex);
            Menu_Change(MENU_NETWAITVOTES);
        }
        */
        return OSDCMD_OK;
    }

    osdcmd_cheatsinfo_stat.cheatnum = -1;
    ud.m_volume_number = 0;
    ud.m_level_number = 7;

    ud.m_monsters_off = ud.monsters_off = 0;

    ud.m_respawn_items = 0;
    ud.m_respawn_inventory = 0;

    ud.multimode = 1;

    if (g_player[myconnectindex].ps->gm & MODE_GAME)
        g_player[myconnectindex].ps->gm = MODE_RESTART|MODE_NEWGAME;
    else G_NewGame_EnterLevel();

    return OSDCMD_OK;
}

// demo <demonum or demofn> [<prof>]
//
// To profile a demo ("timedemo mode"), <prof> can be given in the range 0-8,
// which will start to replay it as fast as possible, rendering <prof> frames
// for each gametic.
//
// Notes:
//  * The demos should be recorded with demorec_diffs set to 0, so that the
//    game state updates are actually computed.
//  * Currently, the profiling can only be aborted on SDL 1.2 builds by
//    pressing any key.
//  * With <prof> greater than 1, interpolation should be calculated properly,
//    though this has not been verified by looking at the frames.
//  * When testing whether a change in the source has an effect on performance,
//    the variance of the run times MUST be taken into account (that is, the
//    replaying must be performed multiple times for the old and new versions,
//    etc.)
static int osdcmd_demo(osdcmdptr_t parm)
{
    if (numplayers > 1)
    {
        LOG_F(WARNING, "Command \"demo\" not allowed in multiplayer.");
        return OSDCMD_OK;
    }

    if (g_player[myconnectindex].ps->gm & MODE_GAME)
    {
        LOG_F(WARNING, "Command \"demo\" can only be used outside of an active game.");
        return OSDCMD_OK;
    }

    if (parm->numparms != 1 && parm->numparms != 2)
        return OSDCMD_SHOWHELP;

    {
        int32_t prof = parm->numparms==2 ? Batoi(parm->parms[1]) : -1;

        Demo_SetFirst(parm->parms[0]);
        Demo_PlayFirst(clamp(prof, -1, 8)+1, 0);
    }

    return OSDCMD_OK;
}

static int osdcmd_activatecheat(osdcmdptr_t parm)
{
    if (parm->numparms != 1)
        return OSDCMD_SHOWHELP;

    if (numplayers == 1 && g_player[myconnectindex].ps->gm & MODE_GAME)
        osdcmd_cheatsinfo_stat.cheatnum = Batoi(parm->parms[0]);
    else
        LOG_F(WARNING, "Command \"activatecheat\" can only be used in a single-player game.");

    return OSDCMD_OK;
}

static int osdcmd_god(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);
    if (numplayers == 1 && g_player[myconnectindex].ps->gm & MODE_GAME)
        osdcmd_cheatsinfo_stat.cheatnum = CHEAT_CORNHOLIO;
    else
        LOG_F(WARNING, "Command \"god\" can only be used in a single-player game.");

    return OSDCMD_OK;
}

static int osdcmd_maxhealth(osdcmdptr_t parm)
{
    if (parm->numparms != 1)
        return OSDCMD_SHOWHELP;

    auto pPlayer = g_player[myconnectindex].ps;

    if (numplayers == 1 && pPlayer->gm & MODE_GAME)
    {
        int const newHealth = Batoi(parm->parms[0]);
        pPlayer->max_player_health = newHealth;
        sprite[pPlayer->i].extra = newHealth;
    }
    else
        LOG_F(WARNING, "Command \"maxhealth\" can only be used in a single-player game.");

    return OSDCMD_OK;
}

static int osdcmd_noclip(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);

    if (numplayers == 1 && g_player[myconnectindex].ps->gm & MODE_GAME)
    {
        osdcmd_cheatsinfo_stat.cheatnum = CHEAT_CLIP;
    }
    else
    {
        LOG_F(WARNING, "Command \"noclip\" can only be used in a single-player game.");
    }

    return OSDCMD_OK;
}

static int osdcmd_restartsound(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);

    S_StopAllSounds();
    S_ClearSoundLocks();

    S_SoundShutdown();
    S_MusicShutdown();

    S_SoundStartup();
    S_MusicStartup();

    if (ud.config.MusicToggle)
        S_RestartMusic();

    return OSDCMD_OK;
}

static int osdcmd_music(osdcmdptr_t parm)
{
    if (parm->numparms == 1)
    {
        int32_t sel = G_GetMusicIdx(parm->parms[0]);

        if (sel == -1)
            return OSDCMD_SHOWHELP;

        if (sel == -2)
        {
            LOG_F(ERROR, "'%s' is not a valid episode/level number pair", parm->parms[0]);
            return OSDCMD_OK;
        }

        if (!S_TryPlayLevelMusic(sel))
        {
            G_PrintCurrentMusic();
        }
        else
        {
            LOG_F(WARNING, "No music defined for episode/level number pair '%s'", parm->parms[0]);
        }

        return OSDCMD_OK;
    }

    return OSDCMD_SHOWHELP;
}

int osdcmd_restartvid(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);
    videoResetMode();
    if (videoSetGameMode(ud.setup.fullscreen,ud.setup.xdim,ud.setup.ydim,ud.setup.bpp,ud.detail))
        G_GameExit("Unable to set video mode!");
    onvideomodechange(ud.setup.bpp>8);
    G_UpdateScreenArea();

    return OSDCMD_OK;
}

int osdcmd_restartmap(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);

    if (g_player[myconnectindex].ps->gm & MODE_GAME && ud.multimode == 1)
        g_player[myconnectindex].ps->gm = MODE_RESTART;

    return OSDCMD_OK;
}

static int osdcmd_vidmode(osdcmdptr_t parm)
{
    int32_t newbpp = ud.setup.bpp, newwidth = ud.setup.xdim,
            newheight = ud.setup.ydim, newfs = ud.setup.fullscreen;
    int32_t tmp;

    if (parm->numparms < 1 || parm->numparms > 4) return OSDCMD_SHOWHELP;

    switch (parm->numparms)
    {
    case 1: // bpp switch
        tmp = Batol(parm->parms[0]);
        if (!(tmp==8 || tmp==16 || tmp==32))
            return OSDCMD_SHOWHELP;
        newbpp = tmp;
        break;
    case 2: // res switch
        newwidth = Batol(parm->parms[0]);
        newheight = Batol(parm->parms[1]);
        break;
    case 3: // res & bpp switch
    case 4:
        newwidth = Batol(parm->parms[0]);
        newheight = Batol(parm->parms[1]);
        tmp = Batol(parm->parms[2]);
        if (!(tmp==8 || tmp==16 || tmp==32))
            return OSDCMD_SHOWHELP;
        newbpp = tmp;
        if (parm->numparms == 4)
            newfs = (Batol(parm->parms[3]) != 0);
        break;
    }

    if (videoSetGameMode(newfs,newwidth,newheight,newbpp,upscalefactor))
    {
        LOG_F(ERROR, "Failed to set video mode!");
        if (videoSetGameMode(ud.setup.fullscreen, ud.setup.xdim, ud.setup.ydim, ud.setup.bpp, upscalefactor))
            G_GameExit("Failed to set video mode!");
    }
    ud.setup.bpp = newbpp;
    ud.setup.xdim = newwidth;
    ud.setup.ydim = newheight;
    ud.setup.fullscreen = newfs;
    onvideomodechange(ud.setup.bpp>8);
    G_UpdateScreenArea();
    return OSDCMD_OK;
}

static int osdcmd_spawn(osdcmdptr_t parm)
{
    int32_t picnum = 0;
    uint16_t cstat=0;
    char pal=0;
    int16_t ang=0;
    int16_t set=0, idx;
    vec3_t vect;

    if (numplayers > 1 || !(g_player[myconnectindex].ps->gm & MODE_GAME))
    {
        LOG_F(WARNING, "Command \"spawn\" can only be used in a single-player game.");
        return OSDCMD_OK;
    }

    switch (parm->numparms)
    {
    case 7: // x,y,z
        vect.x = Batol(parm->parms[4]);
        vect.y = Batol(parm->parms[5]);
        vect.z = Batol(parm->parms[6]);
        set |= 8;
        fallthrough__;
    case 4: // ang
        ang = Batol(parm->parms[3]) & 2047;
        set |= 4;
        fallthrough__;
    case 3: // cstat
        cstat = (uint16_t)Batol(parm->parms[2]);
        set |= 2;
        fallthrough__;
    case 2: // pal
        pal = (uint8_t)Batol(parm->parms[1]);
        set |= 1;
        fallthrough__;
    case 1: // tile number
        if (isdigit(parm->parms[0][0]))
        {
            picnum = Batol(parm->parms[0]);
        }
        else
        {
            int32_t i, j;

            for (j=0; j<2; j++)
            {
                for (i=0; i<g_labelCnt; i++)
                {
                    if ((j == 0 && !Bstrcmp(label+(i<<6),     parm->parms[0])) ||
                        (j == 1 && !Bstrcasecmp(label+(i<<6), parm->parms[0])))
                    {
                        picnum = labelcode[i];
                        break;
                    }
                }

                if (i < g_labelCnt)
                    break;
            }
            if (i==g_labelCnt)
            {
                LOG_F(WARNING, "Invalid label for spawn command!");
                return OSDCMD_OK;
            }
        }

        if ((uint32_t)picnum >= MAXUSERTILES)
        {
            LOG_F(WARNING, "Invalid tile for spawn command!");
            return OSDCMD_OK;
        }
        break;

    default:
        return OSDCMD_SHOWHELP;
    }

    idx = A_Spawn(g_player[myconnectindex].ps->i, picnum);
    if (set & 1) sprite[idx].pal = (uint8_t)pal;
    if (set & 2) sprite[idx].cstat = (int16_t)cstat;
    if (set & 4) sprite[idx].ang = ang;
    if (set & 8)
    {
        if (setsprite(idx, &vect) < 0)
        {
            LOG_F(WARNING, "Invalid sprite coordinates for spawn command!");
            A_DeleteSprite(idx);
        }
    }

    return OSDCMD_OK;
}

static int osdcmd_setvar(osdcmdptr_t parm)
{
    if (numplayers > 1)
    {
        LOG_F(WARNING, "Command \"setvar\" can only be used in a single-player game.");
        return OSDCMD_OK;
    }

    if (parm->numparms != 2)
        return OSDCMD_SHOWHELP;

    int i = hash_find(&h_gamevars, parm->parms[1]);
    int const newValue = (i == -1) ? Batol(parm->parms[1]) : Gv_GetVar(i, g_player[myconnectindex].ps->i, myconnectindex);

    if ((i = hash_find(&h_gamevars, parm->parms[0])) >= 0)
    {
        Gv_SetVar(i, newValue, g_player[myconnectindex].ps->i, myconnectindex);

        LOG_F(INFO, "Variable '%s' now has value %d (input: %d)", aGameVars[i].szLabel,
                   Gv_GetVar(i, g_player[myconnectindex].ps->i, myconnectindex), newValue);
    }
    else
    {
        LOG_F(ERROR, "Invalid variable '%s' for setvar command!", parm->parms[0]);
        return OSDCMD_SHOWHELP;
    }

    return OSDCMD_OK;
}

static int osdcmd_addlogvar(osdcmdptr_t parm)
{
    if (numplayers > 1)
    {
        LOG_F(WARNING, "Command \"addlogvar\" can only be used in a single-player game.");
        return OSDCMD_OK;
    }

    if (parm->numparms != 1)
        return OSDCMD_SHOWHELP;

    int const i = hash_find(&h_gamevars, parm->parms[0]);

    if (i >= 0)
        LOG_F(INFO, "Variable '%s' has value %d, default %d", parm->parms[0], Gv_GetVar(i, g_player[screenpeek].ps->i, screenpeek), (int)aGameVars[i].defaultValue);
    else
    {
        LOG_F(ERROR, "Invalid variable '%s' for addlogvar command!", parm->parms[0]);
        return OSDCMD_SHOWHELP;
    }

    return OSDCMD_OK;
}

static int osdcmd_setactorvar(osdcmdptr_t parm)
{
    if (numplayers > 1)
    {
        LOG_F(WARNING, "Command \"setactorvar\" can only be used in a single-player game.");
        return OSDCMD_OK;
    }

    if (parm->numparms != 3)
        return OSDCMD_SHOWHELP;

    int16_t const spriteNum = Batol(parm->parms[0]);

    if ((unsigned)spriteNum >= MAXSPRITES)
    {
        LOG_F(ERROR, "Invalid sprite '%s' for setactorvar command!", parm->parms[0]);
        return OSDCMD_OK;
    }

    // get value to set
    int i = hash_find(&h_gamevars, parm->parms[2]);
    int const newValue = (i >= 0) ? Gv_GetVar(i, g_player[myconnectindex].ps->i, myconnectindex) : Batol(parm->parms[2]);

    if ((i = hash_find(&h_gamevars, parm->parms[1])) >= 0)
    {
        Gv_SetVar(i, newValue, spriteNum, myconnectindex);

        LOG_F(INFO, "Variable '%s' for sprite %d now has value %d (input: %d)", aGameVars[i].szLabel, spriteNum,
                   Gv_GetVar(i, g_player[myconnectindex].ps->i, myconnectindex), newValue);
    }
    else
    {
        LOG_F(ERROR, "Invalid variable '%s' for setactorvar command!", parm->parms[1]);
        return OSDCMD_SHOWHELP;
    }

    return OSDCMD_OK;
}

static int osdcmd_addpath(osdcmdptr_t parm)
{
    if (parm->numparms != 1)
        return OSDCMD_SHOWHELP;

    addsearchpath(parm->parms[0]);

    return OSDCMD_OK;
}

static int osdcmd_initgroupfile(osdcmdptr_t parm)
{
    if (parm->numparms != 1)
        return OSDCMD_SHOWHELP;

    initgroupfile(parm->parms[0]);

    return OSDCMD_OK;
}

static int osdcmd_cmenu(osdcmdptr_t parm)
{
    if (parm->numparms != 1)
        return OSDCMD_SHOWHELP;

    if (numplayers > 1)
    {
        LOG_F(WARNING, "Command \"cmenu\" can only be used in a single-player game.");
        return OSDCMD_OK;
    }

    if ((g_player[myconnectindex].ps->gm & MODE_MENU) != MODE_MENU)
        Menu_Open(myconnectindex);

    Menu_Change(Batol(parm->parms[0]));

    return OSDCMD_OK;
}




static int osdcmd_crosshaircolor(osdcmdptr_t parm)
{
    if (parm->numparms != 3)
    {
        LOG_F(INFO, "crosshaircolor: r:%d g:%d b:%d",CrosshairColors.r,CrosshairColors.g,CrosshairColors.b);
        return OSDCMD_SHOWHELP;
    }

    uint8_t const r = Batol(parm->parms[0]);
    uint8_t const g = Batol(parm->parms[1]);
    uint8_t const b = Batol(parm->parms[2]);

    G_SetCrosshairColor(r,g,b);
    
    if (!OSD_ParsingScript())
        LOG_F(INFO, "%s", parm->raw);

    return OSDCMD_OK;
}

static int osdcmd_give(osdcmdptr_t parm)
{
    int32_t i;

    if (numplayers != 1 || (g_player[myconnectindex].ps->gm & MODE_GAME) == 0 ||
            g_player[myconnectindex].ps->dead_flag != 0)
    {
        LOG_F(INFO, "Cannot use give command while dead or not in a single-player game.");
        return OSDCMD_OK;
    }

    if (parm->numparms != 1) return OSDCMD_SHOWHELP;

    if (!Bstrcasecmp(parm->parms[0], "all"))
    {
        osdcmd_cheatsinfo_stat.cheatnum = CHEAT_STUFF;
        return OSDCMD_OK;
    }
    else if (!Bstrcasecmp(parm->parms[0], "health"))
    {
        sprite[g_player[myconnectindex].ps->i].extra = g_player[myconnectindex].ps->max_player_health<<1;
        return OSDCMD_OK;
    }
    else if (!Bstrcasecmp(parm->parms[0], "weapons"))
    {
        osdcmd_cheatsinfo_stat.cheatnum = CHEAT_WEAPONS;
        return OSDCMD_OK;
    }
    else if (!Bstrcasecmp(parm->parms[0], "ammo"))
    {
        for (i=MAX_WEAPONS-(VOLUMEONE?6:1)-1; i>=PISTOL_WEAPON; i--)
            P_AddAmmo(g_player[myconnectindex].ps,i,g_player[myconnectindex].ps->max_ammo_amount[i]);
        return OSDCMD_OK;
    }
    else if (!Bstrcasecmp(parm->parms[0], "armor"))
    {
        g_player[myconnectindex].ps->inv_amount[GET_SHIELD] = 100;
        return OSDCMD_OK;
    }
    else if (!Bstrcasecmp(parm->parms[0], "keys"))
    {
        osdcmd_cheatsinfo_stat.cheatnum = CHEAT_KEYS;
        return OSDCMD_OK;
    }
    else if (!Bstrcasecmp(parm->parms[0], "inventory"))
    {
        osdcmd_cheatsinfo_stat.cheatnum = CHEAT_INVENTORY;
        return OSDCMD_OK;
    }
    return OSDCMD_SHOWHELP;
}

void onvideomodechange(int32_t newmode)
{
    uint8_t palid;

    // XXX?
    if (!newmode || g_player[screenpeek].ps->palette < BASEPALCOUNT)
        palid = g_player[screenpeek].ps->palette;
    else
        palid = BASEPAL;

#ifdef POLYMER
    if (videoGetRenderMode() == REND_POLYMER)
    {
        int32_t i = 0;

        while (i < MAXSPRITES)
        {
            if (practor[i].lightptr)
            {
                polymer_deletelight(practor[i].lightId);
                practor[i].lightptr = NULL;
                practor[i].lightId = -1;
            }
            i++;
        }
    }
#endif

    videoSetPalette(ud.brightness>>2, palid, 0);
    g_restorePalette = -1;
    g_crosshairSum = -1;
}

static int osdcmd_button(osdcmdptr_t parm)
{
    static char const s_gamefunc_[] = "gamefunc_";
    int constexpr strlen_gamefunc_  = ARRAY_SIZE(s_gamefunc_) - 1;

    char const *p = parm->name + strlen_gamefunc_;

//    if (g_player[myconnectindex].ps->gm == MODE_GAME) // only trigger these if in game
    CONTROL_ButtonFlags[CONFIG_FunctionNameToNum(p)] = 1; // FIXME

    return OSDCMD_OK;
}

const char *const ConsoleButtons[] =
{
    "mouse1", "mouse2", "mouse3", "mouse4", "mwheelup",
    "mwheeldn", "mouse5", "mouse6", "mouse7", "mouse8"
};

static int osdcmd_bind(osdcmdptr_t parm)
{
    if (parm->numparms==1 && !Bstrcasecmp(parm->parms[0],"showkeys"))
    {
        for (auto & s : sctokeylut)
            LOG_F(INFO, "%s", s.key);
        for (auto ConsoleButton : ConsoleButtons)
            LOG_F(INFO, "%s", ConsoleButton);
        return OSDCMD_OK;
    }

    if (parm->numparms==0)
    {
        int j=0;

        LOG_F(INFO, "Current key bindings:");

        for (int i=0; i<MAXBOUNDKEYS+MAXMOUSEBUTTONS; i++)
            if (CONTROL_KeyIsBound(i))
            {
                j++;
                LOG_F(INFO, "%-9s %s\"%s\"", CONTROL_KeyBinds[i].key, CONTROL_KeyBinds[i].repeat?"":"norepeat ",
                           CONTROL_KeyBinds[i].cmdstr);
            }

        if (j == 0)
            LOG_F(INFO, "No binds found.");

        return OSDCMD_OK;
    }

    int i, j, repeat;

    for (i=0; i < ARRAY_SSIZE(sctokeylut); i++)
    {
        if (!Bstrcasecmp(parm->parms[0], sctokeylut[i].key))
            break;
    }

    // didn't find the key
    if (i == ARRAY_SSIZE(sctokeylut))
    {
        for (i=0; i<MAXMOUSEBUTTONS; i++)
            if (!Bstrcasecmp(parm->parms[0],ConsoleButtons[i]))
                break;

        if (i >= MAXMOUSEBUTTONS)
            return OSDCMD_SHOWHELP;

        if (parm->numparms < 2)
        {
            if (CONTROL_KeyBinds[MAXBOUNDKEYS + i].cmdstr && CONTROL_KeyBinds[MAXBOUNDKEYS + i ].key)
                LOG_F(INFO, "%-9s %s\"%s\"", ConsoleButtons[i], CONTROL_KeyBinds[MAXBOUNDKEYS + i].repeat?"":"norepeat ",
                CONTROL_KeyBinds[MAXBOUNDKEYS + i].cmdstr);
            else LOG_F(INFO, "'%s' is unbound", ConsoleButtons[i]);
            return OSDCMD_OK;
        }

        j = 1;

        repeat = 1;
        if (!Bstrcasecmp(parm->parms[j],"norepeat"))
        {
            repeat = 0;
            j++;
        }

        Bstrcpy(tempbuf,parm->parms[j++]);
        for (; j<parm->numparms; j++)
        {
            Bstrcat(tempbuf," ");
            Bstrcat(tempbuf,parm->parms[j++]);
        }

        CONTROL_BindMouse(i, tempbuf, repeat, ConsoleButtons[i]);

        if (!OSD_ParsingScript())
            LOG_F(INFO, "%s", parm->raw);
        return OSDCMD_OK;
    }

    if (parm->numparms < 2)
    {
        if (CONTROL_KeyIsBound(sctokeylut[i].sc))
            LOG_F(INFO, "%-9s %s\"%s\"", sctokeylut[i].key, CONTROL_KeyBinds[sctokeylut[i].sc].repeat?"":"norepeat ",
                       CONTROL_KeyBinds[sctokeylut[i].sc].cmdstr);
        else LOG_F(INFO, "'%s' is unbound", sctokeylut[i].key);

        return OSDCMD_OK;
    }

    j = 1;

    repeat = 1;
    if (!Bstrcasecmp(parm->parms[j],"norepeat"))
    {
        repeat = 0;
        j++;
    }

    Bstrcpy(tempbuf,parm->parms[j++]);
    for (; j<parm->numparms; j++)
    {
        Bstrcat(tempbuf," ");
        Bstrcat(tempbuf,parm->parms[j++]);
    }

    CONTROL_BindKey(sctokeylut[i].sc, tempbuf, repeat, sctokeylut[i].key);

    char *cp = tempbuf;

    // Populate the keyboard config menu based on the bind.
    // Take care of processing one-to-many bindings properly, too.
    static char const s_gamefunc_[] = "gamefunc_";
    int constexpr strlen_gamefunc_  = ARRAY_SIZE(s_gamefunc_) - 1;

    while ((cp = Bstrstr(cp, s_gamefunc_)))
    {
        cp += strlen_gamefunc_;

        char *semi = Bstrchr(cp, ';');

        if (semi)
            *semi = 0;

        j = CONFIG_FunctionNameToNum(cp);

        if (semi)
            cp = semi+1;

        if (j != -1)
        {
            ud.config.KeyboardKeys[j][1] = ud.config.KeyboardKeys[j][0];
            ud.config.KeyboardKeys[j][0] = sctokeylut[i].sc;
//            CONTROL_MapKey(j, sctokeylut[i].sc, ud.config.KeyboardKeys[j][0]);

            if (j == gamefunc_Show_Console)
                OSD_CaptureKey(sctokeylut[i].sc);
        }
    }

    if (!OSD_ParsingScript())
        LOG_F(INFO, "%s", parm->raw);

    return OSDCMD_OK;
}

static int osdcmd_unbindall(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);

    for (int i = 0; i < MAXBOUNDKEYS; ++i)
        CONTROL_FreeKeyBind(i);

    for (int i = 0; i < MAXMOUSEBUTTONS; ++i)
        CONTROL_FreeMouseBind(i);

    for (auto &KeyboardKey : ud.config.KeyboardKeys)
        KeyboardKey[0] = KeyboardKey[1] = 0xff;

    if (!OSD_ParsingScript())
        LOG_F(INFO, "unbound all controls");

    return OSDCMD_OK;
}

static int osdcmd_unbind(osdcmdptr_t parm)
{
    if (parm->numparms != 1)
        return OSDCMD_SHOWHELP;

    for (auto & ConsoleKey : sctokeylut)
    {
        if (ConsoleKey.key && !Bstrcasecmp(parm->parms[0], ConsoleKey.key))
        {
            CONTROL_FreeKeyBind(ConsoleKey.sc);
            for (auto &KeyboardKey : ud.config.KeyboardKeys)
            {
                if (KeyboardKey[0] == ConsoleKey.sc)
                    KeyboardKey[0] = 0xff;

                if (KeyboardKey[1] == ConsoleKey.sc)
                    KeyboardKey[1] = 0xff;
            }
            LOG_F(INFO, "unbound %s", ConsoleKey.key);
            return OSDCMD_OK;
        }
    }

    for (int i = 0; i < MAXMOUSEBUTTONS; i++)
    {
        if (!Bstrcasecmp(parm->parms[0], ConsoleButtons[i]))
        {
            CONTROL_FreeMouseBind(i);
            LOG_F(INFO, "unbound %s", ConsoleButtons[i]);
            return OSDCMD_OK;
        }
    }

    return OSDCMD_SHOWHELP;
}

static int osdcmd_unbound(osdcmdptr_t parm)
{
    if (parm->numparms != 1)
        return OSDCMD_OK;

    int const gameFunc = CONFIG_FunctionNameToNum(parm->parms[0]);

    if (gameFunc != -1)
        ud.config.KeyboardKeys[gameFunc][0] = 0;

    return OSDCMD_OK;
}

static int osdcmd_quicksave(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);
    if (!(g_player[myconnectindex].ps->gm & MODE_GAME))
        LOG_F(INFO, "Can't quicksave while not in a game.");
    else g_doQuickSave = 1;
    return OSDCMD_OK;
}

static int osdcmd_quickload(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);
    if (!(g_player[myconnectindex].ps->gm & MODE_GAME))
        LOG_F(INFO, "Can't quickload while not in a game.");
    else g_doQuickSave = 2;
    return OSDCMD_OK;
}

static int osdcmd_screenshot(osdcmdptr_t parm)
{
//    KB_ClearKeysDown();
#ifndef EDUKE32_STANDALONE
    static const char *fn = "duke0000.png";
#else
    static const char *fn = "capt0000.png";
#endif

    if (parm->numparms == 1 && !Bstrcasecmp(parm->parms[0], "tga"))
        videoCaptureScreenTGA(fn, 0);
    else videoCaptureScreen(fn, 0);

    return OSDCMD_OK;
}

#if 0
static int osdcmd_savestate(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_PARAMETER(parm);
    G_SaveMapState();
    return OSDCMD_OK;
}

static int osdcmd_restorestate(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_PARAMETER(parm);
    G_RestoreMapState();
    return OSDCMD_OK;
}
#endif

static int osdcmd_inittimer(osdcmdptr_t parm)
{
    if (parm->numparms != 1)
    {
        LOG_F(INFO, "%dHz timer",g_timerTicsPerSecond);
        return OSDCMD_SHOWHELP;
    }

    G_InitTimer(Batol(parm->parms[0]));

    LOG_F(INFO, "%s", parm->raw);
    return OSDCMD_OK;
}

#if !defined NETCODE_DISABLE
static int osdcmd_name(osdcmdptr_t parm)
{
    char namebuf[32];

    if (parm->numparms != 1)
    {
        LOG_F(INFO, "'name' is '%s'",szPlayerName);
        return OSDCMD_SHOWHELP;
    }

    Bstrcpy(tempbuf,parm->parms[0]);

    while (Bstrlen(OSD_StripColors(namebuf,tempbuf)) > 10)
        tempbuf[Bstrlen(tempbuf)-1] = '\0';

    Bstrncpy(szPlayerName,tempbuf,sizeof(szPlayerName)-1);
    szPlayerName[sizeof(szPlayerName)-1] = '\0';
    CommandName = nullptr;
    LOG_F(INFO, "name %s",szPlayerName);

    Net_SendClientInfo();

    return OSDCMD_OK;
}

static int osdcmd_dumpmapstate(osdfuncparm_t const * const)
{
    // this command takes no parameters

    DumpMapStateHistory();

    return OSDCMD_OK;
}

static int osdcmd_playerinfo(osdfuncparm_t const * const)
{
    LOG_F(INFO, "Your player index is %d.", myconnectindex);

    for(int32_t playerIndex = 0; playerIndex < MAXPLAYERS; playerIndex++)
    {
        if(g_player[playerIndex].ps == nullptr)
            LOG_F(INFO, "g_player[%d]: ps unallocated.", playerIndex);
        else
            LOG_F(INFO, "g_player[%d]: ps->i is %d.", playerIndex, g_player[playerIndex].ps->i);
    }

    return OSDCMD_OK;
}

static int osdcmd_disconnect(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);
    g_netDisconnect = 1;
    return OSDCMD_OK;
}

static int osdcmd_connect(osdcmdptr_t parm)
{
    if (parm->numparms != 1)
        return OSDCMD_SHOWHELP;

    Net_Connect(parm->parms[0]);
    G_BackToMenu();
    return OSDCMD_OK;
}

static int osdcmd_password(osdcmdptr_t parm)
{
    if (parm->numparms < 1)
    {
        Bmemset(g_netPassword, 0, sizeof(g_netPassword));
        return OSDCMD_OK;
    }
    Bstrncpy(g_netPassword, (parm->raw) + 9, sizeof(g_netPassword)-1);

    return OSDCMD_OK;
}

static int osdcmd_listplayers(osdcmdptr_t parm)
{
    ENetPeer *currentPeer;
    char ipaddr[32];

    if (parm && parm->numparms != 0)
        return OSDCMD_SHOWHELP;

    if (!g_netServer)
    {
        LOG_F(INFO, "Only the server can list active clients.");
        return OSDCMD_OK;
    }

    LOG_F(INFO, "Connected clients:");

    for (currentPeer = g_netServer -> peers;
            currentPeer < & g_netServer -> peers [g_netServer -> peerCount];
            ++ currentPeer)
    {
        if (currentPeer -> state != ENET_PEER_STATE_CONNECTED)
            continue;

        enet_address_get_host_ip(&currentPeer->address, ipaddr, sizeof(ipaddr));
        LOG_F(INFO, "%s %s", ipaddr, g_player[(intptr_t)currentPeer->data].user_name);
    }

    return OSDCMD_OK;
}

#if 0
static int osdcmd_kick(osdcmdptr_t parm)
{
    ENetPeer *currentPeer;
    uint32_t hexaddr;

    if (parm->numparms != 1)
        return OSDCMD_SHOWHELP;

    if (!g_netServer)
    {
        LOG_F(INFO, "Only the server can kick players.");
        return OSDCMD_OK;
    }

    for (currentPeer = g_netServer -> peers;
            currentPeer < & g_netServer -> peers [g_netServer -> peerCount];
            ++ currentPeer)
    {
        if (currentPeer -> state != ENET_PEER_STATE_CONNECTED)
            continue;

        sscanf(parm->parms[0],"%" SCNx32 "", &hexaddr);

        if (currentPeer->address.host == hexaddr)
        {
            VLOG_F(LOG_NET, "Kicking %s (%x)", g_player[(intptr_t)currentPeer->data].user_name, currentPeer->address.host);
            enet_peer_disconnect(currentPeer, DISC_KICKED);
            return OSDCMD_OK;
        }
    }

    LOG_F(INFO, "Player %s not found.", parm->parms[0]);
    osdcmd_listplayers(NULL);

    return OSDCMD_OK;
}

static int osdcmd_kickban(osdcmdptr_t parm)
{
    ENetPeer *currentPeer;
    uint32_t hexaddr;

    if (parm->numparms != 1)
        return OSDCMD_SHOWHELP;

    if (!g_netServer)
    {
        LOG_F(INFO, "Only the server can ban players.");
        return OSDCMD_OK;
    }

    for (currentPeer = g_netServer -> peers;
            currentPeer < & g_netServer -> peers [g_netServer -> peerCount];
            ++ currentPeer)
    {
        if (currentPeer -> state != ENET_PEER_STATE_CONNECTED)
            continue;

        sscanf(parm->parms[0],"%" SCNx32 "", &hexaddr);

        // TODO: implement banning logic

        if (currentPeer->address.host == hexaddr)
        {
            char ipaddr[32];

            enet_address_get_host_ip(&currentPeer->address, ipaddr, sizeof(ipaddr));
            LOG_F(LOG_NET, "Host %s is now banned.", ipaddr);
            VLOG_F(LOG_NET, "Kicking %s (%x)", g_player[(intptr_t)currentPeer->data].user_name, currentPeer->address.host);
            enet_peer_disconnect(currentPeer, DISC_BANNED);
            return OSDCMD_OK;
        }
    }

    LOG_F(INFO, "Player %s not found.", parm->parms[0]);
    osdcmd_listplayers(NULL);

    return OSDCMD_OK;
}
#endif
#endif

static int osdcmd_purgesaves(osdcmdptr_t UNUSED(parm))
{
    UNREFERENCED_CONST_PARAMETER(parm);
    G_DeleteOldSaves();
    return OSDCMD_OK;
}

static int osdcmd_cvar_set_game(osdcmdptr_t parm)
{
    static char const prefix_snd[] = "snd_";
    static char const prefix_mus[] = "mus_";

    int const r = osdcmd_cvar_set(parm);

    if (r != OSDCMD_OK || parm->numparms < 1) return r;

    if (!Bstrcasecmp(parm->name, "r_upscalefactor"))
    {
        if (in3dmode())
        {
            videoSetGameMode(fullscreen, xres, yres, bpp, ud.detail);
        }
    }
    else if (!Bstrcasecmp(parm->name, "r_size"))
    {
        ud.statusbarmode = (ud.screen_size < 8);
        G_UpdateScreenArea();
    }
    else if (!Bstrcasecmp(parm->name, "r_stacksize"))
    {
        if (g_frameStackSize % 65536 == 0)
        {
            LOG_F(WARNING, "Stack size specified is a multiple of 64K. Adjusting...");
            g_frameStackSize = (g_frameStackSize >> 4) * 17;
        }
        g_restartFrameRoutine = 1;
    }
    else if (!Bstrcasecmp(parm->name, "r_ambientlight"))
    {
        if (r_ambientlight == 0)
            r_ambientlightrecip = 256.f;
        else r_ambientlightrecip = 1.f/r_ambientlight;
    }
    else if (!Bstrcasecmp(parm->name, "in_mouse"))
    {
        CONTROL_MouseEnabled = (ud.setup.usemouse && CONTROL_MousePresent);
    }
    else if (!Bstrcasecmp(parm->name, "in_joystick"))
    {
        CONTROL_JoystickEnabled = (ud.setup.usejoystick && CONTROL_JoyPresent);
    }
    else if (!Bstrcasecmp(parm->name, "vid_gamma"))
    {
        ud.brightness = GAMMA_CALC;
        ud.brightness <<= 2;
        videoSetPalette(ud.brightness>>2,g_player[myconnectindex].ps->palette,0);
    }
    else if (!Bstrcasecmp(parm->name, "vid_brightness") || !Bstrcasecmp(parm->name, "vid_contrast"))
    {
        videoSetPalette(ud.brightness>>2,g_player[myconnectindex].ps->palette,0);
    }
    else if (!Bstrcasecmp(parm->name, "snd_enabled") || !Bstrcasecmp(parm->name, "snd_numvoices"))
    {
        if (!FX_WarmedUp())
            return r;

        S_SoundShutdown();
        S_SoundStartup();

        S_ClearSoundLocks();
    }
    else if (!Bstrncasecmp(parm->name, prefix_snd, ARRAY_SIZE(prefix_snd)-1))
    {
        if (!FX_WarmedUp())
            return r;

        if (ASS_MIDISoundDriver == ASS_OPL3 || ASS_MIDISoundDriver == ASS_SF2 || MusicIsWaveform)
        {
            // music that we generate and send through sound
            songposition pos = {};

            if (MusicIsWaveform)
                FX_GetPosition(MusicVoice, (int *)&pos.tick);
            else
                MUSIC_GetSongPosition(&pos);

            S_MusicShutdown();
            S_SoundShutdown();

            S_SoundStartup();
            S_MusicStartup();

            S_ClearSoundLocks();

            if (ud.config.MusicToggle)
            {
                S_RestartMusic();

                if (MusicIsWaveform)
                    FX_SetPosition(MusicVoice, (int)pos.tick);
                else
                    MUSIC_SetSongPosition(pos.measure, pos.beat, pos.tick);
            }
        }
        else
        {
            S_SoundShutdown();
            S_SoundStartup();

            S_ClearSoundLocks();
        }
    }
    else if (!Bstrcasecmp(parm->name, "mus_volume"))
        S_MusicVolume(Batol(parm->parms[0]));
    else if (!Bstrncasecmp(parm->name, prefix_mus, ARRAY_SIZE(prefix_mus)-1))
    {
        if (!MUSIC_WarmedUp())
            return r;

        songposition pos = {};

        if (MusicIsWaveform)
            FX_GetPosition(MusicVoice, (int *)&pos.tick);
        else
            MUSIC_GetSongPosition(&pos);

        S_MusicShutdown();
        S_MusicStartup();

        if (ud.config.MusicToggle)
        {
            S_RestartMusic();

            if (MusicIsWaveform)
                FX_SetPosition(MusicVoice, (int)pos.tick);
            else
                MUSIC_SetSongPosition(pos.measure, pos.beat, pos.tick);
        }
    }
    else if (!Bstrcasecmp(parm->name, "hud_scale")
             || !Bstrcasecmp(parm->name, "hud_statusbarmode")
             || !Bstrcasecmp(parm->name, "r_rotatespritenowidescreen"))
    {
        G_UpdateScreenArea();
    }
    else if (!Bstrcasecmp(parm->name, "skill"))
    {
        if (numplayers > 1)
            return r;

        ud.player_skill = ud.m_player_skill;
    }
    else if (!Bstrcasecmp(parm->name, "color"))
    {
        ud.color = G_CheckPlayerColor(ud.color);
        g_player[0].ps->palookup = g_player[0].pcolor = ud.color;
    }
    else if (!Bstrcasecmp(parm->name, "osdscale"))
    {
        osdrscale = 1.f/osdscale;

        if (xdim && ydim)
            OSD_ResizeDisplay(xdim, ydim);
    }
    else if (!Bstrcasecmp(parm->name, "wchoice"))
    {
        if (parm->numparms == 1)
        {
            if (g_forceWeaponChoice) // rewrite ud.wchoice because osdcmd_cvar_set already changed it
            {
                int j = 0;

                while (j < 10)
                {
                    ud.wchoice[j] = g_player[myconnectindex].wchoice[j] + '0';
                    j++;
                }

                ud.wchoice[j] = 0;
            }
            else
            {
                char const *c = parm->parms[0];

                if (*c)
                {
                    int j = 0;

                    while (*c && j < 10)
                    {
                        g_player[myconnectindex].wchoice[j] = *c - '0';
                        c++;
                        j++;
                    }

                    while (j < 10)
                    {
                        if (j == 9)
                            g_player[myconnectindex].wchoice[9] = 1;
                        else
                            g_player[myconnectindex].wchoice[j] = 2;

                        j++;
                    }
                }
            }

            g_forceWeaponChoice = 0;
        }

        /*    Net_SendClientInfo();*/
    }

    return r;
}

static int osdcmd_cvar_set_multi(osdcmdptr_t parm)
{
    int const r = osdcmd_cvar_set_game(parm);

    if (r != OSDCMD_OK) return r;

    G_UpdatePlayerFromMenu();

    return r;
}

static int osdcmd_locale(osdcmdptr_t parm)
{
    if (parm->numparms != 1)
        return OSDCMD_SHOWHELP;

    localeSetCurrent(parm->parms[0]);

    return OSDCMD_OK;
}

#define CVAR_BOOL_OPTSTR ":\n 0: disabled\n 1: enabled"

int32_t registerosdcommands(void)
{
    FX_InitCvars();

    static osdcvardata_t cvars_game[] =
    {
        { "benchmarkmode", "benchmark mode:\n 0: off\n 1: performance test\n 2: generate reference screenshots for correctness testing", (void *) &g_BenchmarkMode, CVAR_INT|CVAR_NOSAVE, 0, 2 },

        { "crosshair", "display crosshair" CVAR_BOOL_OPTSTR, (void *)&ud.crosshair, CVAR_BOOL, 0, 1 },

        { "cl_autoaim", "weapon autoaim:\n 0: none\n 1: hitscan only\n 2: projectiles only\n 3: everything", (void *)&ud.config.AutoAim, CVAR_INT|CVAR_MULTI, 0, 3 },
        { "cl_autorun", "player autorun" CVAR_BOOL_OPTSTR, (void *)&ud.auto_run, CVAR_BOOL, 0, 1 },

        { "cl_autosave", "save game at checkpoints" CVAR_BOOL_OPTSTR, (void *) &ud.autosave, CVAR_BOOL, 0, 1 },
        { "cl_autosavedeletion", "automatically delete old checkpoint saves" CVAR_BOOL_OPTSTR, (void *) &ud.autosavedeletion, CVAR_BOOL, 0, 1 },
        { "cl_maxautosaves", "number of autosaves kept before deleting the oldest", (void *) &ud.maxautosaves, CVAR_INT, 1, 100 },

#if !defined NETCODE_DISABLE
        { "cl_automsg", "automatically send messages to all players" CVAR_BOOL_OPTSTR, (void *)&ud.automsg, CVAR_BOOL, 0, 1 },
        { "cl_autovote", "automatic vote yes for multiplayer map changes" CVAR_BOOL_OPTSTR, (void *)&ud.autovote, CVAR_BOOL, 0, 1 },
        { "cl_obituaries", "print player death messages in multiplayer" CVAR_BOOL_OPTSTR, (void *)&ud.obituaries, CVAR_BOOL, 0, 1 },
        { "cl_idplayers", "display player names when aiming at opponents in multiplayer" CVAR_BOOL_OPTSTR, (void *)&ud.idplayers, CVAR_BOOL, 0, 1 },
#endif

        { "cl_cheatmask", "bitmask controlling cheats unlocked in menu", (void *)&cl_cheatmask, CVAR_UINT, 0, ~0 },
        { "cl_democams", "third-person cameras in demos" CVAR_BOOL_OPTSTR, (void *)&ud.democams, CVAR_BOOL, 0, 1 },
        { "cl_runmode", "run key behavior with cl_autorun enabled:\n 0: walk\n 1: do nothing", (void *)&ud.runkey_mode, CVAR_BOOL, 0, 1 },

        { "cl_showcoords", "DEBUG: coordinate display", (void *)&ud.coords, CVAR_INT, 0,
#ifdef USE_OPENGL
          2
#else
          1
#endif
        },

        { "cl_viewbob", "player head bobbing" CVAR_BOOL_OPTSTR, (void *)&ud.viewbob, CVAR_BOOL, 0, 1 },

        { "cl_weaponsway", "weapon sways when moving" CVAR_BOOL_OPTSTR, (void *)&ud.weaponsway, CVAR_BOOL, 0, 1 },
        { "cl_weaponswitch", "bitmask controlling switching weapon when out of ammo or a new weapon is picked up:\n 0: disabled\n 1: if new\n 2: if empty\n 4: determined by wchoice", (void *)&ud.weaponswitch, CVAR_INT|CVAR_MULTI, 0, 7 },

        { "cl_keybindorder", "default order for the keybind menu (overridden by custom order, requires a restart):\n 0: classic\n 1: modern", (void *)&cvar_kbo_type, CVAR_BOOL, 0, 1 },
        { "cl_kbconfirm", "on assigning a keyboard bind, will display a confirmation dialog if key is already assigned:\n 0: disabled\n 1: enabled", (void *)&cvar_kbconfirm, CVAR_BOOL, 0, 1 },


        { "color", "player palette", (void *)&ud.color, CVAR_INT|CVAR_MULTI, 0, MAXPALOOKUPS-1 },

        { "crosshairscale","crosshair size", (void *)&ud.crosshairscale, CVAR_INT, 10, 100 },

        { "demorec_diffs","differential recording in demos" CVAR_BOOL_OPTSTR,(void *)&demorec_diffs_cvar, CVAR_BOOL, 0, 1 },
        { "demorec_force","forced demo recording" CVAR_BOOL_OPTSTR,(void *)&demorec_force_cvar, CVAR_BOOL|CVAR_NOSAVE, 0, 1 },
        {
            "demorec_difftics","sets game tic interval after which a diff is recorded",
            (void *)&demorec_difftics_cvar, CVAR_INT, 2, 60*REALGAMETICSPERSEC
        },
        { "demorec_diffcompress","Compression method for diffs. (0: none, 1: KSLZW)",(void *)&demorec_diffcompress_cvar, CVAR_BOOL, 0, 1 },
        { "demorec_synccompress","Compression method for input. (0: none, 1: KSLZW)",(void *)&demorec_synccompress_cvar, CVAR_BOOL, 0, 1 },
        { "demorec_seeds","record random seed for later sync checking" CVAR_BOOL_OPTSTR,(void *)&demorec_seeds_cvar, CVAR_BOOL, 0, 1 },
        { "demoplay_diffs","use diffs in demo playback" CVAR_BOOL_OPTSTR,(void *)&demoplay_diffs, CVAR_BOOL, 0, 1 },
        { "demoplay_showsync","enables display of sync status",(void *)&demoplay_showsync, CVAR_BOOL, 0, 1 },

        { "fov", "change the field of view", (void *)&ud.fov, CVAR_INT, 60, 140 },

        { "hud_althud", "alternate mini-hud" CVAR_BOOL_OPTSTR, (void *)&ud.althud, CVAR_BOOL, 0, 1 },
        { "hud_custom", "change the custom hud", (void *)&ud.statusbarcustom, CVAR_INT, 0, ud.statusbarrange },
        { "hud_position", "align status bar to top of screen" CVAR_BOOL_OPTSTR, (void *)&ud.hudontop, CVAR_BOOL, 0, 1 },
        { "hud_bgstretch", "stretch background images to fill screen" CVAR_BOOL_OPTSTR, (void *)&ud.bgstretch, CVAR_BOOL, 0, 1 },
        { "hud_messages", "display item pickup messages in HUD" CVAR_BOOL_OPTSTR, (void *)&ud.fta_on, CVAR_BOOL, 0, 1 },
        { "hud_messagetime", "length of time to display multiplayer chat messages", (void *)&ud.msgdisptime, CVAR_INT, 0, 3600 },
        { "hud_numbertile", "first tile in alt hud number set", (void *)&althud_numbertile, CVAR_INT, 0, MAXUSERTILES-10 },
        { "hud_numberpal", "pal for alt hud numbers", (void *)&althud_numberpal, CVAR_INT, 0, MAXPALOOKUPS-1 },
        { "hud_shadows", "display shadows under HUD elements" CVAR_BOOL_OPTSTR, (void *)&althud_shadows, CVAR_BOOL, 0, 1 },
        { "hud_flashing", "health flashes in HUD when player is near death" CVAR_BOOL_OPTSTR, (void *)&althud_flashing, CVAR_BOOL, 0, 1 },
        { "hud_glowingquotes", "\"glowing\" on-screen messages" CVAR_BOOL_OPTSTR, (void *)&hud_glowingquotes, CVAR_BOOL, 0, 1 },
        { "hud_scale","changes the hud scale", (void *)&ud.statusbarscale, CVAR_INT|CVAR_FUNCPTR, 50, 100 },
        { "hud_showmapname", "display level or map name on load" CVAR_BOOL_OPTSTR, (void *)&hud_showmapname, CVAR_BOOL, 0, 1 },
        { "hud_stats", "on-screen display of level time, secrets found, and enemies killed" CVAR_BOOL_OPTSTR, (void *)&ud.levelstats, CVAR_BOOL, 0, 1 },
        { "hud_textscale", "multiplayer chat message size", (void *)&ud.textscale, CVAR_INT, 100, 400 },
        { "hud_weaponscale","weapon scale", (void *)&ud.weaponscale, CVAR_INT, 10, 100 },
        { "hud_statusbarmode", "status bar draws over player view" CVAR_BOOL_OPTSTR, (void *)&ud.statusbarmode, CVAR_BOOL|CVAR_FUNCPTR, 0, 1 },

#ifdef EDUKE32_TOUCH_DEVICES
        { "hud_hidestick", "hide the touch input stick", (void *)&droidinput.hideStick, CVAR_BOOL, 0, 1 },
#endif

        {
            "in_joystickaimweight", "weight joystick aiming input towards whichever axis is moving the most at any given time",
            (void *)&ud.config.JoystickAimWeight, CVAR_INT, 0, 10
        },

        {
            "in_joystickviewcentering", "automatically center the player's view vertically when moving while using a controller",
            (void *)&ud.config.JoystickViewCentering, CVAR_INT, 0, 10
        },

        {
            "in_joystickviewleveling", "automatically adjusts the player's view vertically to aim at enemies",
            (void *)&ud.config.JoystickAimAssist, CVAR_BOOL, 0, 1
        },

        { "in_joystick","use joystick or controller input" CVAR_BOOL_OPTSTR,(void *)&ud.setup.usejoystick, CVAR_BOOL|CVAR_FUNCPTR, 0, 1 },
        { "in_mouse","use mouse input" CVAR_BOOL_OPTSTR,(void *)&ud.setup.usemouse, CVAR_BOOL|CVAR_FUNCPTR, 0, 1 },

        { "in_aimmode", "DEPRECATED: hold button for mouse aim" CVAR_BOOL_OPTSTR, (void *)&ud.mouseaiming, CVAR_BOOL, 0, 1 },
        { "in_mousemode", "DEPRECATED: vertical mouse aiming" CVAR_BOOL_OPTSTR, (void *)&g_myAimMode, CVAR_BOOL, 0, 1 },

        {
            "in_mousebias", "DEPRECATED: emulates the original mouse code's weighting of input towards whichever axis is moving the most at any given time",
            (void *)&ud.config.MouseBias, CVAR_INT, 0, 32
        },

        { "in_mouseflip", "invert vertical mouse movement" CVAR_BOOL_OPTSTR, (void *)&ud.mouseflip, CVAR_BOOL, 0, 1 },
/*
        { "in_rumble", "controller rumble factor", (void*)&ud.rumble, CVAR_INT, 0, 4 },
*/
        { "in_rumble", "game controller rumble support" CVAR_BOOL_OPTSTR, (void*)&ud.config.controllerRumble, CVAR_BOOL, 0, 1 },
        { "mus_enabled", "music subsystem" CVAR_BOOL_OPTSTR, (void *)&ud.config.MusicToggle, CVAR_BOOL|CVAR_FUNCPTR, 0, 1 },
        { "mus_device", "music device", (void*)& ud.config.MusicDevice, CVAR_INT|CVAR_FUNCPTR, 0, ASS_NumSoundCards },
        { "mus_volume", "controls music volume", (void *)&ud.config.MusicVolume, CVAR_INT|CVAR_FUNCPTR, 0, 255 },

        { "osdhightile", "use content pack assets for console text if available" CVAR_BOOL_OPTSTR, (void *)&osdhightile, CVAR_BOOL, 0, 1 },
        { "osdscale", "console text size", (void *)&osdscale, CVAR_FLOAT|CVAR_FUNCPTR, 1, 4 },

        { "r_camrefreshdelay", "minimum delay between security camera sprite updates, 120 = 1 second", (void *)&ud.camera_time, CVAR_INT, 1, 240 },
        { "r_drawweapon", "draw player weapon" CVAR_BOOL_OPTSTR "\n 2: icon only", (void *)&ud.drawweapon, CVAR_INT, 0, 2 },
        { "r_showfps", "show the frame rate counter" CVAR_BOOL_OPTSTR "\n 2: extra timing data\n 3: excessive timing data", (void *)&ud.showfps, CVAR_INT, 0, 3 },
        { "r_showfpsperiod", "time in seconds before averaging min and max stats for r_showfps 2+", (void *)&ud.frameperiod, CVAR_INT, 0, 5 },
        { "r_shadows", "in-game entities cast simple shadows" CVAR_BOOL_OPTSTR, (void *)&ud.shadows, CVAR_BOOL, 0, 1 },
        { "r_size", "change size of viewable area", (void *)&ud.screen_size, CVAR_INT|CVAR_FUNCPTR, 0, 64 },
        { "r_stacksize", "change frame drawing routine stack size", (void *)&g_frameStackSize, CVAR_INT|CVAR_FUNCPTR, DRAWFRAME_MIN_STACK_SIZE, DRAWFRAME_MAX_STACK_SIZE },
        { "r_rotatespritenowidescreen", "pass bit 1024 to all CON rotatesprite calls", (void *)&g_rotatespriteNoWidescreen, CVAR_BOOL|CVAR_FUNCPTR, 0, 1 },
        { "r_upscalefactor", "increase performance by rendering at upscalefactor less than the screen resolution and upscale to the full resolution in the software renderer", (void *)&ud.detail, CVAR_INT|CVAR_FUNCPTR, 1, 16 },
        { "r_precache", "precache art assets during level load" CVAR_BOOL_OPTSTR, (void *)&ud.config.useprecache, CVAR_BOOL, 0, 1 },

        { "r_ambientlight", "sets the global map light level",(void *)&r_ambientlight, CVAR_FLOAT|CVAR_FUNCPTR, 0, 10 },

        { "r_pr_defaultlights", "default polymer lights:\n 0: off\n 1: on",(void *)&r_pr_defaultlights, CVAR_BOOL, 0, 1 },

        { "skill","changes the game skill setting", (void *)&ud.m_player_skill, CVAR_INT|CVAR_FUNCPTR|CVAR_NOSAVE/*|CVAR_NOMULTI*/, 0, 5 },

        { "snd_ambience", "ambient sounds" CVAR_BOOL_OPTSTR, (void *)&ud.config.AmbienceToggle, CVAR_BOOL, 0, 1 },
        { "snd_enabled", "sound effects" CVAR_BOOL_OPTSTR, (void *)&ud.config.SoundToggle, CVAR_BOOL|CVAR_FUNCPTR, 0, 1 },
        { "snd_fxvolume", "volume of sound effects", (void *)&ud.config.FXVolume, CVAR_INT, 0, 255 },
        { "snd_mixrate", "sound mixing rate", (void *)&ud.config.MixRate, CVAR_INT|CVAR_FUNCPTR, 0, 48000 },
        { "snd_numchannels", "the number of sound channels", (void *)&ud.config.NumChannels, CVAR_INT|CVAR_FUNCPTR, 0, 2 },
        { "snd_numvoices", "the number of concurrent sounds", (void *)&ud.config.NumVoices, CVAR_INT|CVAR_FUNCPTR, 1, MAXVOICES },
#ifdef ASS_REVERSESTEREO
        { "snd_reversestereo", "reverses the stereo channels", (void *)&ud.config.ReverseStereo, CVAR_BOOL, 0, 1 },
#endif
        { "snd_speech", "bitmask controlling player speech", (void *)&ud.config.VoiceToggle, CVAR_INT, 0, 5 },
#ifdef FORMAT_UPGRADE_ELIGIBLE
        { "snd_tryformats", "discover and use replacement audio in .flac and .ogg formats if available" CVAR_BOOL_OPTSTR, (void *)&g_maybeUpgradeSoundFormats, CVAR_BOOL, 0, 1 },
        { "mus_tryformats", "discover and use replacement music in .flac and .ogg formats if available (requires snd_tryformats 1)" CVAR_BOOL_OPTSTR, (void *)&g_maybeUpgradeMusic,
          CVAR_BOOL, 0, 1 },
#endif
        { "team", "change team in multiplayer", (void *)&ud.team, CVAR_INT|CVAR_MULTI, 0, 3 },

#ifdef EDUKE32_TOUCH_DEVICES
        { "touch_sens_move_x","touch input sensitivity for moving forward/back", (void *)&droidinput.forward_sens, CVAR_FLOAT, 1, 9 },
        { "touch_sens_move_y","touch input sensitivity for strafing", (void *)&droidinput.strafe_sens, CVAR_FLOAT, 1, 9 },
        { "touch_sens_look_x", "touch input sensitivity for turning left/right", (void *) &droidinput.yaw_sens, CVAR_FLOAT, 1, 9 },
        { "touch_sens_look_y", "touch input sensitivity for looking up/down", (void *) &droidinput.pitch_sens, CVAR_FLOAT, 1, 9 },
        { "touch_invert", "invert look up/down touch input", (void *) &droidinput.invertLook, CVAR_BOOL, 0, 1 },
#endif
        { "vm_preempt", "drawing preempts CON VM" CVAR_BOOL_OPTSTR, (void *)&g_vm_preempt, CVAR_BOOL|CVAR_NOSAVE, 0, 1 },
        { "wchoice","weapon priority for automatically switching on empty or pickup", (void *)ud.wchoice, CVAR_STRING|CVAR_FUNCPTR, 0, MAX_WEAPONS },
    };

    osdcmd_cheatsinfo_stat.cheatnum = -1;

    for (auto & cv : cvars_game)
    {
        switch (cv.flags & (CVAR_FUNCPTR|CVAR_MULTI))
        {
            case CVAR_FUNCPTR:
                OSD_RegisterCvar(&cv, osdcmd_cvar_set_game); break;
            case CVAR_MULTI:
            case CVAR_FUNCPTR|CVAR_MULTI:
                OSD_RegisterCvar(&cv, osdcmd_cvar_set_multi); break;
            default:
                OSD_RegisterCvar(&cv, osdcmd_cvar_set); break;
        }
    }

#if !defined NETCODE_DISABLE
    OSD_RegisterFunction("connect","connect: connects to a multiplayer game", osdcmd_connect);
    OSD_RegisterFunction("disconnect","disconnect: disconnects from the local multiplayer game", osdcmd_disconnect);
    OSD_RegisterFunction("dumpmapstates", "Dumps current snapshots to CL/Srv_MapStates.bin", osdcmd_dumpmapstate);
#if 0
    OSD_RegisterFunction("kick","kick <id>: kicks a multiplayer client.  See listplayers.", osdcmd_kick);
    OSD_RegisterFunction("kickban","kickban <id>: kicks a multiplayer client and prevents them from reconnecting.  See listplayers.", osdcmd_kickban);
#endif
    OSD_RegisterFunction("listplayers","listplayers: lists currently connected multiplayer clients", osdcmd_listplayers);
    OSD_RegisterFunction("name","name: change your multiplayer nickname", osdcmd_name);
    OSD_RegisterFunction("password","password: sets multiplayer game password", osdcmd_password);
    OSD_RegisterFunction("playerinfo", "Prints information about the current player", osdcmd_playerinfo);
#endif

    if (VOLUMEONE)
        OSD_RegisterFunction("changelevel","changelevel <level>: warps to the given level", osdcmd_changelevel);
    else
    {
        OSD_RegisterFunction("changelevel","changelevel <volume> <level>: warps to the given level", osdcmd_changelevel);
        OSD_RegisterFunction("map","map <mapfile>: loads the given user map", osdcmd_map);
        OSD_RegisterFunction("demo","demo <demofile or demonum>: starts the given demo", osdcmd_demo);
    }

    OSD_RegisterFunction("addpath","addpath <path>: adds path to game filesystem", osdcmd_addpath);
    OSD_RegisterFunction("bind",R"(bind <key> <string>: associates a keypress with a string of console input. Type "bind showkeys" for a list of keys and "listsymbols" for a list of valid console commands.)", osdcmd_bind);
    OSD_RegisterFunction("cmenu","cmenu <#>: jumps to menu", osdcmd_cmenu);
    OSD_RegisterFunction("crosshaircolor","crosshaircolor: changes the crosshair color", osdcmd_crosshaircolor);

    for (auto & func : gamefunctions)
    {
        if (func[0] == '\0')
            continue;

//        if (!Bstrcmp(gamefunctions[i],"Show_Console")) continue;

        Bsprintf(tempbuf, "gamefunc_%s", func);

        char *const t = Bstrtolower(Xstrdup(tempbuf));

        Bstrcat(tempbuf, ": game button");

        OSD_RegisterFunction(t, Xstrdup(tempbuf), osdcmd_button);
    }

    OSD_RegisterFunction("give","give <all|health|weapons|ammo|armor|keys|inventory>: gives requested item", osdcmd_give);
    OSD_RegisterFunction("god","god: toggles god mode", osdcmd_god);
    OSD_RegisterFunction("activatecheat","activatecheat <id>: activates a cheat code", osdcmd_activatecheat);
    OSD_RegisterFunction("maxhealth", "maxhealth <amount>: sets the player's maximum health", osdcmd_maxhealth);

    OSD_RegisterFunction("initgroupfile","initgroupfile <path>: adds a grp file into the game filesystem", osdcmd_initgroupfile);
    OSD_RegisterFunction("inittimer","debug", osdcmd_inittimer);

    OSD_RegisterFunction("locale","locale: changes the locale", osdcmd_locale);

    OSD_RegisterFunction("music","music E<ep>L<lev>: change music", osdcmd_music);

    OSD_RegisterFunction("noclip","noclip: toggles clipping mode", osdcmd_noclip);

    OSD_RegisterFunction("purgesaves", "purgesaves: deletes obsolete and unreadable save files", osdcmd_purgesaves);

    OSD_RegisterFunction("quicksave","quicksave: performs a quick save", osdcmd_quicksave);
    OSD_RegisterFunction("quickload","quickload: performs a quick load", osdcmd_quickload);
    OSD_RegisterFunction("quit","quit: exits the game immediately", osdcmd_quit);
    OSD_RegisterFunction("exit","exit: exits the game immediately", osdcmd_quit);

    OSD_RegisterFunction("restartmap", "restartmap: restarts the current map", osdcmd_restartmap);
    OSD_RegisterFunction("restartsound","restartsound: reinitializes the sound system",osdcmd_restartsound);
    OSD_RegisterFunction("restartvid","restartvid: reinitializes the video mode",osdcmd_restartvid);
    OSD_RegisterFunction("addlogvar","addlogvar <gamevar>: prints the value of a gamevar", osdcmd_addlogvar);
    OSD_RegisterFunction("setvar","setvar <gamevar> <value>: sets the value of a gamevar", osdcmd_setvar);
    OSD_RegisterFunction("setvarvar","setvarvar <gamevar1> <gamevar2>: sets the value of <gamevar1> to <gamevar2>", osdcmd_setvar);
    OSD_RegisterFunction("setactorvar","setactorvar <actor#> <gamevar> <value>: sets the value of <actor#>'s <gamevar> to <value>", osdcmd_setactorvar);
    OSD_RegisterFunction("screenshot","screenshot [format]: takes a screenshot.", osdcmd_screenshot);

    OSD_RegisterFunction("spawn","spawn <picnum> [palnum] [cstat] [ang] [x y z]: spawns a sprite with the given properties",osdcmd_spawn);

    OSD_RegisterFunction("unbind","unbind <key>: unbinds a key", osdcmd_unbind);
    OSD_RegisterFunction("unbindall","unbindall: unbinds all keys", osdcmd_unbindall);
    OSD_RegisterFunction("unbound", NULL, osdcmd_unbound);

    OSD_RegisterFunction("vidmode","vidmode <xdim> <ydim> <bpp> <fullscreen>: change the video mode",osdcmd_vidmode);
#ifdef USE_OPENGL
    baselayer_osdcmd_vidmode_func = osdcmd_vidmode;
#endif

    return 0;
}
