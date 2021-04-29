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

#include "compat.h"
#include "build.h"

#include "namesdyn.h"
#include "global.h"

#ifdef DYNTILEREMAP_ENABLE
# define DVPTR(x) &x
#else
# define DVPTR(x) NULL
#endif

int16_t DynamicTileMap[TILEMAPSIZE];

struct dynitem
{
    const char *str;
    int32_t *dynvalptr;
    const int16_t staticval;
};

static struct dynitem g_dynTileList[] =
{
    { "ACCESS_ICON",         DVPTR(ACCESS_ICON),         ACCESS_ICON__ },
    { "ACCESSCARD",          DVPTR(ACCESSCARD),          ACCESSCARD__ },
    { "ACCESSSWITCH",        DVPTR(ACCESSSWITCH),        ACCESSSWITCH__ },
    { "ACCESSSWITCH2",       DVPTR(ACCESSSWITCH2),       ACCESSSWITCH2__ },
    { "ACTIVATOR",           DVPTR(ACTIVATOR),           ACTIVATOR__ },
    { "ACTIVATORLOCKED",     DVPTR(ACTIVATORLOCKED),     ACTIVATORLOCKED__ },
    { "AIRTANK",             DVPTR(AIRTANK),             AIRTANK__ },
    { "AIRTANK_ICON",        DVPTR(AIRTANK_ICON),        AIRTANK_ICON__ },
    { "ALIENSWITCH",         DVPTR(ALIENSWITCH),         ALIENSWITCH__ },
    { "AMMO",                DVPTR(AMMO),                AMMO__ },
    { "AMMOBOX",             DVPTR(AMMOBOX),             AMMOBOX__ },
    { "AMMOLOTS",            DVPTR(AMMOLOTS),            AMMOLOTS__ },
    { "ANTENNA",             DVPTR(ANTENNA),             ANTENNA__ },
    { "APLAYER",             DVPTR(APLAYER),             APLAYER__ },
    { "APLAYERTOP",          DVPTR(APLAYERTOP),          APLAYERTOP__ },
    { "ARMJIB1",             DVPTR(ARMJIB1),             ARMJIB1__ },
    { "ARROW",               DVPTR(ARROW),               ARROW__ },
    { "ATM",                 DVPTR(ATM),                 ATM__ },
    { "ATMBROKE",            DVPTR(ATMBROKE),            ATMBROKE__ },
    { "ATOMICHEALTH",        DVPTR(ATOMICHEALTH),        ATOMICHEALTH__ },
    { "BANNER",              DVPTR(BANNER),              BANNER__ },
    { "BARBROKE",            DVPTR(BARBROKE),            BARBROKE__ },
    { "BATTERYAMMO",         DVPTR(BATTERYAMMO),         BATTERYAMMO__ },
    { "BETASCREEN",          DVPTR(BETASCREEN),          BETASCREEN__ },
    { "BETAVERSION",         DVPTR(BETAVERSION),         BETAVERSION__ },
    { "BGRATE1",             DVPTR(BGRATE1),             BGRATE1__ },
    { "BIGALPHANUM",         DVPTR(BIGALPHANUM),         BIGALPHANUM__ },
    { "BIGAPPOS",            DVPTR(BIGAPPOS),            BIGAPPOS__ },
    { "BIGCOLIN",            DVPTR(BIGCOLIN),            BIGCOLIN__ },
    { "BIGCOMMA",            DVPTR(BIGCOMMA),            BIGCOMMA__ },
    { "BIGFORCE",            DVPTR(BIGFORCE),            BIGFORCE__ },
    { "BIGHOLE",             DVPTR(BIGHOLE),             BIGHOLE__ },
    { "BIGORBIT1",           DVPTR(BIGORBIT1),           BIGORBIT1__ },
    { "BIGPERIOD",           DVPTR(BIGPERIOD),           BIGPERIOD__ },
    { "BIGQ",                DVPTR(BIGQ),                BIGQ__ },
    { "BIGSEMI",             DVPTR(BIGSEMI),             BIGSEMI__ },
    { "BIGX",                DVPTR(BIGX_),               BIGX__ },
    { "BLANKSCREEN",         DVPTR(BLANKSCREEN),         BLANKSCREEN__ },
    { "BLIMP",               DVPTR(BLIMP),               BLIMP__ },
    { "BLOOD",               DVPTR(BLOOD),               BLOOD__ },
    { "BLOODPOOL",           DVPTR(BLOODPOOL),           BLOODPOOL__ },
    { "BLOODSPLAT1",         DVPTR(BLOODSPLAT1),         BLOODSPLAT1__ },
    { "BLOODSPLAT2",         DVPTR(BLOODSPLAT2),         BLOODSPLAT2__ },
    { "BLOODSPLAT3",         DVPTR(BLOODSPLAT3),         BLOODSPLAT3__ },
    { "BLOODSPLAT4",         DVPTR(BLOODSPLAT4),         BLOODSPLAT4__ },
    { "BLOODYPOLE",          DVPTR(BLOODYPOLE),          BLOODYPOLE__ },
    { "BOLT1",               DVPTR(BOLT1),               BOLT1__ },
    { "BONUSSCREEN",         DVPTR(BONUSSCREEN),         BONUSSCREEN__ },
    { "BOOT_ICON",           DVPTR(BOOT_ICON),           BOOT_ICON__ },
    { "BOOTS",               DVPTR(BOOTS),               BOOTS__ },
    { "BORNTOBEWILDSCREEN",  DVPTR(BORNTOBEWILDSCREEN),  BORNTOBEWILDSCREEN__ },
    { "BOSS1",               DVPTR(BOSS1),               BOSS1__ },
    { "BOSS1LOB",            DVPTR(BOSS1LOB),            BOSS1LOB__ },
    { "BOSS1SHOOT",          DVPTR(BOSS1SHOOT),          BOSS1SHOOT__ },
    { "BOSS1STAYPUT",        DVPTR(BOSS1STAYPUT),        BOSS1STAYPUT__ },
    { "BOSS2",               DVPTR(BOSS2),               BOSS2__ },
    { "BOSS3",               DVPTR(BOSS3),               BOSS3__ },
    { "BOSS4",               DVPTR(BOSS4),               BOSS4__ },
    { "BOSS4STAYPUT",        DVPTR(BOSS4STAYPUT),        BOSS4STAYPUT__ },
    { "BOSSTOP",             DVPTR(BOSSTOP),             BOSSTOP__ },
    { "BOTTLE1",             DVPTR(BOTTLE1),             BOTTLE1__ },
    { "BOTTLE10",            DVPTR(BOTTLE10),            BOTTLE10__ },
    { "BOTTLE11",            DVPTR(BOTTLE11),            BOTTLE11__ },
    { "BOTTLE12",            DVPTR(BOTTLE12),            BOTTLE12__ },
    { "BOTTLE13",            DVPTR(BOTTLE13),            BOTTLE13__ },
    { "BOTTLE14",            DVPTR(BOTTLE14),            BOTTLE14__ },
    { "BOTTLE15",            DVPTR(BOTTLE15),            BOTTLE15__ },
    { "BOTTLE16",            DVPTR(BOTTLE16),            BOTTLE16__ },
    { "BOTTLE17",            DVPTR(BOTTLE17),            BOTTLE17__ },
    { "BOTTLE18",            DVPTR(BOTTLE18),            BOTTLE18__ },
    { "BOTTLE19",            DVPTR(BOTTLE19),            BOTTLE19__ },
    { "BOTTLE2",             DVPTR(BOTTLE2),             BOTTLE2__ },
    { "BOTTLE3",             DVPTR(BOTTLE3),             BOTTLE3__ },
    { "BOTTLE4",             DVPTR(BOTTLE4),             BOTTLE4__ },
    { "BOTTLE5",             DVPTR(BOTTLE5),             BOTTLE5__ },
    { "BOTTLE6",             DVPTR(BOTTLE6),             BOTTLE6__ },
    { "BOTTLE7",             DVPTR(BOTTLE7),             BOTTLE7__ },
    { "BOTTLE8",             DVPTR(BOTTLE8),             BOTTLE8__ },
    { "BOTTOMSTATUSBAR",     DVPTR(BOTTOMSTATUSBAR),     BOTTOMSTATUSBAR__ },
    { "BOUNCEMINE",          DVPTR(BOUNCEMINE),          BOUNCEMINE__ },
    { "BOX",                 DVPTR(BOX),                 BOX__ },
    { "BPANNEL1",            DVPTR(BPANNEL1),            BPANNEL1__ },
    { "BPANNEL3",            DVPTR(BPANNEL3),            BPANNEL3__ },
    { "BROKEFIREHYDRENT",    DVPTR(BROKEFIREHYDRENT),    BROKEFIREHYDRENT__ },
    { "BROKEHYDROPLANT",     DVPTR(BROKEHYDROPLANT),     BROKEHYDROPLANT__ },
    { "BROKENCHAIR",         DVPTR(BROKENCHAIR),         BROKENCHAIR__ },
    { "BULLETHOLE",          DVPTR(BULLETHOLE),          BULLETHOLE__ },
    { "BURNING",             DVPTR(BURNING),             BURNING__ },
    { "BURNING2",            DVPTR(BURNING2),            BURNING2__ },
    { "CACTUS",              DVPTR(CACTUS),              CACTUS__ },
    { "CACTUSBROKE",         DVPTR(CACTUSBROKE),         CACTUSBROKE__ },
    { "CAMCORNER",           DVPTR(CAMCORNER),           CAMCORNER__ },
    { "CAMERA1",             DVPTR(CAMERA1),             CAMERA1__ },
    { "CAMERALIGHT",         DVPTR(CAMERALIGHT),         CAMERALIGHT__ },
    { "CAMERAPOLE",          DVPTR(CAMERAPOLE),          CAMERAPOLE__ },
    { "CAMLIGHT",            DVPTR(CAMLIGHT),            CAMLIGHT__ },
    { "CANWITHSOMETHING",    DVPTR(CANWITHSOMETHING),    CANWITHSOMETHING__ },
    { "CANWITHSOMETHING2",   DVPTR(CANWITHSOMETHING2),   CANWITHSOMETHING2__ },
    { "CANWITHSOMETHING3",   DVPTR(CANWITHSOMETHING3),   CANWITHSOMETHING3__ },
    { "CANWITHSOMETHING4",   DVPTR(CANWITHSOMETHING4),   CANWITHSOMETHING4__ },
    { "CEILINGSTEAM",        DVPTR(CEILINGSTEAM),        CEILINGSTEAM__ },
    { "CHAINGUN",            DVPTR(CHAINGUN),            CHAINGUN__ },
    { "CHAINGUNSPRITE",      DVPTR(CHAINGUNSPRITE),      CHAINGUNSPRITE__ },
    { "CHAIR1",              DVPTR(CHAIR1),              CHAIR1__ },
    { "CHAIR2",              DVPTR(CHAIR2),              CHAIR2__ },
    { "CHAIR3",              DVPTR(CHAIR3),              CHAIR3__ },
    { "CIRCLEPANNEL",        DVPTR(CIRCLEPANNEL),        CIRCLEPANNEL__ },
    { "CIRCLEPANNELBROKE",   DVPTR(CIRCLEPANNELBROKE),   CIRCLEPANNELBROKE__ },
    { "CLOUDYOCEAN",         DVPTR(CLOUDYOCEAN),         CLOUDYOCEAN__ },
    { "CLOUDYSKIES",         DVPTR(CLOUDYSKIES),         CLOUDYSKIES__ },
    { "COLA",                DVPTR(COLA),                COLA__ },
    { "COLAMACHINE",         DVPTR(COLAMACHINE),         COLAMACHINE__ },
    { "COMMANDER",           DVPTR(COMMANDER),           COMMANDER__ },
    { "COMMANDERSTAYPUT",    DVPTR(COMMANDERSTAYPUT),    COMMANDERSTAYPUT__ },
    { "CONE",                DVPTR(CONE),                CONE__ },
    { "COOLEXPLOSION1",      DVPTR(COOLEXPLOSION1),      COOLEXPLOSION1__ },
    { "CRACK1",              DVPTR(CRACK1),              CRACK1__ },
    { "CRACK2",              DVPTR(CRACK2),              CRACK2__ },
    { "CRACK3",              DVPTR(CRACK3),              CRACK3__ },
    { "CRACK4",              DVPTR(CRACK4),              CRACK4__ },
    { "CRACKKNUCKLES",       DVPTR(CRACKKNUCKLES),       CRACKKNUCKLES__ },
    { "CRANE",               DVPTR(CRANE),               CRANE__ },
    { "CRANEPOLE",           DVPTR(CRANEPOLE),           CRANEPOLE__ },
    { "CROSSHAIR",           DVPTR(CROSSHAIR),           CROSSHAIR__ },
    { "CRYSTALAMMO",         DVPTR(CRYSTALAMMO),         CRYSTALAMMO__ },
    { "CYCLER",              DVPTR(CYCLER),              CYCLER__ },
    { "DEVISTATOR",          DVPTR(DEVISTATOR),          DEVISTATOR__ },
    { "DEVISTATORAMMO",      DVPTR(DEVISTATORAMMO),      DEVISTATORAMMO__ },
    { "DEVISTATORSPRITE",    DVPTR(DEVISTATORSPRITE),    DEVISTATORSPRITE__ },
    { "DIGITALNUM",          DVPTR(DIGITALNUM),          DIGITALNUM__ },
    { "DIPSWITCH",           DVPTR(DIPSWITCH),           DIPSWITCH__ },
    { "DIPSWITCH2",          DVPTR(DIPSWITCH2),          DIPSWITCH2__ },
    { "DIPSWITCH3",          DVPTR(DIPSWITCH3),          DIPSWITCH3__ },
    { "DOLPHIN1",            DVPTR(DOLPHIN1),            DOLPHIN1__ },
    { "DOLPHIN2",            DVPTR(DOLPHIN2),            DOLPHIN2__ },
    { "DOMELITE",            DVPTR(DOMELITE),            DOMELITE__ },
    { "DOORSHOCK",           DVPTR(DOORSHOCK),           DOORSHOCK__ },
    { "DOORTILE1",           DVPTR(DOORTILE1),           DOORTILE1__ },
    { "DOORTILE10",          DVPTR(DOORTILE10),          DOORTILE10__ },
    { "DOORTILE11",          DVPTR(DOORTILE11),          DOORTILE11__ },
    { "DOORTILE12",          DVPTR(DOORTILE12),          DOORTILE12__ },
    { "DOORTILE14",          DVPTR(DOORTILE14),          DOORTILE14__ },
    { "DOORTILE15",          DVPTR(DOORTILE15),          DOORTILE15__ },
    { "DOORTILE16",          DVPTR(DOORTILE16),          DOORTILE16__ },
    { "DOORTILE17",          DVPTR(DOORTILE17),          DOORTILE17__ },
    { "DOORTILE18",          DVPTR(DOORTILE18),          DOORTILE18__ },
    { "DOORTILE19",          DVPTR(DOORTILE19),          DOORTILE19__ },
    { "DOORTILE2",           DVPTR(DOORTILE2),           DOORTILE2__ },
    { "DOORTILE20",          DVPTR(DOORTILE20),          DOORTILE20__ },
    { "DOORTILE21",          DVPTR(DOORTILE21),          DOORTILE21__ },
    { "DOORTILE22",          DVPTR(DOORTILE22),          DOORTILE22__ },
    { "DOORTILE23",          DVPTR(DOORTILE23),          DOORTILE23__ },
    { "DOORTILE3",           DVPTR(DOORTILE3),           DOORTILE3__ },
    { "DOORTILE4",           DVPTR(DOORTILE4),           DOORTILE4__ },
    { "DOORTILE5",           DVPTR(DOORTILE5),           DOORTILE5__ },
    { "DOORTILE6",           DVPTR(DOORTILE6),           DOORTILE6__ },
    { "DOORTILE7",           DVPTR(DOORTILE7),           DOORTILE7__ },
    { "DOORTILE8",           DVPTR(DOORTILE8),           DOORTILE8__ },
    { "DOORTILE9",           DVPTR(DOORTILE9),           DOORTILE9__ },
    { "DREALMS",             DVPTR(DREALMS),             DREALMS__ },
    { "DRONE",               DVPTR(DRONE),               DRONE__ },
    { "DUCK",                DVPTR(DUCK),                DUCK__ },
    { "DUKECAR",             DVPTR(DUKECAR),             DUKECAR__ },
    { "DUKEGUN",             DVPTR(DUKEGUN),             DUKEGUN__ },
    { "DUKELEG",             DVPTR(DUKELEG),             DUKELEG__ },
    { "DUKELYINGDEAD",       DVPTR(DUKELYINGDEAD),       DUKELYINGDEAD__ },
    { "DUKENUKEM",           DVPTR(DUKENUKEM),           DUKENUKEM__ },
    { "DUKETAG",             DVPTR(DUKETAG),             DUKETAG__ },
    { "DUKETORSO",           DVPTR(DUKETORSO),           DUKETORSO__ },
    { "EGG",                 DVPTR(EGG),                 EGG__ },
    { "ENDALPHANUM",         DVPTR(ENDALPHANUM),         ENDALPHANUM__ },
    { "EXPLODINGBARREL",     DVPTR(EXPLODINGBARREL),     EXPLODINGBARREL__ },
    { "EXPLODINGBARREL2",    DVPTR(EXPLODINGBARREL2),    EXPLODINGBARREL2__ },
    { "EXPLOSION2",          DVPTR(EXPLOSION2),          EXPLOSION2__ },
    { "EXPLOSION2BOT",       DVPTR(EXPLOSION2BOT),       EXPLOSION2BOT__ },
    { "F1HELP",              DVPTR(F1HELP),              F1HELP__ },
    { "FANSHADOW",           DVPTR(FANSHADOW),           FANSHADOW__ },
    { "FANSHADOWBROKE",      DVPTR(FANSHADOWBROKE),      FANSHADOWBROKE__ },
    { "FANSPRITE",           DVPTR(FANSPRITE),           FANSPRITE__ },
    { "FANSPRITEBROKE",      DVPTR(FANSPRITEBROKE),      FANSPRITEBROKE__ },
    { "FECES",               DVPTR(FECES),               FECES__ },
    { "FEM1",                DVPTR(FEM1),                FEM1__ },
    { "FEM10",               DVPTR(FEM10),               FEM10__ },
    { "FEM2",                DVPTR(FEM2),                FEM2__ },
    { "FEM3",                DVPTR(FEM3),                FEM3__ },
    { "FEM4",                DVPTR(FEM4),                FEM4__ },
    { "FEM5",                DVPTR(FEM5),                FEM5__ },
    { "FEM6",                DVPTR(FEM6),                FEM6__ },
    { "FEM6PAD",             DVPTR(FEM6PAD),             FEM6PAD__ },
    { "FEM7",                DVPTR(FEM7),                FEM7__ },
    { "FEM8",                DVPTR(FEM8),                FEM8__ },
    { "FEM9",                DVPTR(FEM9),                FEM9__ },
    { "FEMMAG1",             DVPTR(FEMMAG1),             FEMMAG1__ },
    { "FEMMAG2",             DVPTR(FEMMAG2),             FEMMAG2__ },
    { "FEMPIC1",             DVPTR(FEMPIC1),             FEMPIC1__ },
    { "FEMPIC2",             DVPTR(FEMPIC2),             FEMPIC2__ },
    { "FEMPIC3",             DVPTR(FEMPIC3),             FEMPIC3__ },
    { "FEMPIC4",             DVPTR(FEMPIC4),             FEMPIC4__ },
    { "FEMPIC5",             DVPTR(FEMPIC5),             FEMPIC5__ },
    { "FEMPIC6",             DVPTR(FEMPIC6),             FEMPIC6__ },
    { "FEMPIC7",             DVPTR(FEMPIC7),             FEMPIC7__ },
    { "FETUS",               DVPTR(FETUS),               FETUS__ },
    { "FETUSBROKE",          DVPTR(FETUSBROKE),          FETUSBROKE__ },
    { "FETUSJIB",            DVPTR(FETUSJIB),            FETUSJIB__ },
    { "FIRE",                DVPTR(FIRE),                FIRE__ },
    { "FIRE2",               DVPTR(FIRE2),               FIRE2__ },
    { "FIREBARREL",          DVPTR(FIREBARREL),          FIREBARREL__ },
    { "FIREEXT",             DVPTR(FIREEXT),             FIREEXT__ },
    { "FIRELASER",           DVPTR(FIRELASER),           FIRELASER__ },
    { "FIREVASE",            DVPTR(FIREVASE),            FIREVASE__ },
    { "FIRSTAID",            DVPTR(FIRSTAID),            FIRSTAID__ },
    { "FIRSTAID_ICON",       DVPTR(FIRSTAID_ICON),       FIRSTAID_ICON__ },
    { "FIRSTGUN",            DVPTR(FIRSTGUN),            FIRSTGUN__ },
    { "FIRSTGUNRELOAD",      DVPTR(FIRSTGUNRELOAD),      FIRSTGUNRELOAD__ },
    { "FIRSTGUNSPRITE",      DVPTR(FIRSTGUNSPRITE),      FIRSTGUNSPRITE__ },
    { "FIST",                DVPTR(FIST),                FIST__ },
    { "FLOORFLAME",          DVPTR(FLOORFLAME),          FLOORFLAME__ },
    { "FLOORPLASMA",         DVPTR(FLOORPLASMA),         FLOORPLASMA__ },
    { "FLOORSLIME",          DVPTR(FLOORSLIME),          FLOORSLIME__ },
    { "FOF",                 DVPTR(FOF),                 FOF__ },
    { "FOODOBJECT16",        DVPTR(FOODOBJECT16),        FOODOBJECT16__ },
    { "FOOTPRINTS",          DVPTR(FOOTPRINTS),          FOOTPRINTS__ },
    { "FOOTPRINTS2",         DVPTR(FOOTPRINTS2),         FOOTPRINTS2__ },
    { "FOOTPRINTS3",         DVPTR(FOOTPRINTS3),         FOOTPRINTS3__ },
    { "FOOTPRINTS4",         DVPTR(FOOTPRINTS4),         FOOTPRINTS4__ },
    { "FORCERIPPLE",         DVPTR(FORCERIPPLE),         FORCERIPPLE__ },
    { "FORCESPHERE",         DVPTR(FORCESPHERE),         FORCESPHERE__ },
    { "FRAGBAR",             DVPTR(FRAGBAR),             FRAGBAR__ },
    { "FRAMEEFFECT1",        DVPTR(FRAMEEFFECT1),        FRAMEEFFECT1__ },
    { "FRAMEEFFECT1_13",     DVPTR(FRAMEEFFECT1_13),     FRAMEEFFECT1_13__ },
    { "FRANKENSTINESWITCH",  DVPTR(FRANKENSTINESWITCH),  FRANKENSTINESWITCH__ },
    { "FREEZE",              DVPTR(FREEZE),              FREEZE__ },
    { "FREEZEAMMO",          DVPTR(FREEZEAMMO),          FREEZEAMMO__ },
    { "FREEZEBLAST",         DVPTR(FREEZEBLAST),         FREEZEBLAST__ },
    { "FREEZESPRITE",        DVPTR(FREEZESPRITE),        FREEZESPRITE__ },
    { "FUELPOD",             DVPTR(FUELPOD),             FUELPOD__ },
    { "GENERICPOLE",         DVPTR(GENERICPOLE),         GENERICPOLE__ },
    { "GENERICPOLE2",        DVPTR(GENERICPOLE2),        GENERICPOLE2__ },
    { "GLASS",               DVPTR(GLASS),               GLASS__ },
    { "GLASS2",              DVPTR(GLASS2),              GLASS2__ },
    { "GLASSPIECES",         DVPTR(GLASSPIECES),         GLASSPIECES__ },
    { "GPSPEED",             DVPTR(GPSPEED),             GPSPEED__ },
    { "GRATE1",              DVPTR(GRATE1),              GRATE1__ },
    { "GREENSLIME",          DVPTR(GREENSLIME),          GREENSLIME__ },
    { "GROWAMMO",            DVPTR(GROWAMMO),            GROWAMMO__ },
    { "GROWSPARK",           DVPTR(GROWSPARK),           GROWSPARK__ },
    { "GROWSPRITEICON",      DVPTR(GROWSPRITEICON),      GROWSPRITEICON__ },
    { "HANDHOLDINGACCESS",   DVPTR(HANDHOLDINGACCESS),   HANDHOLDINGACCESS__ },
    { "HANDHOLDINGLASER",    DVPTR(HANDHOLDINGLASER),    HANDHOLDINGLASER__ },
    { "HANDPRINTSWITCH",     DVPTR(HANDPRINTSWITCH),     HANDPRINTSWITCH__ },
    { "HANDREMOTE",          DVPTR(HANDREMOTE),          HANDREMOTE__ },
    { "HANDSWITCH",          DVPTR(HANDSWITCH),          HANDSWITCH__ },
    { "HANDTHROW",           DVPTR(HANDTHROW),           HANDTHROW__ },
    { "HANGLIGHT",           DVPTR(HANGLIGHT),           HANGLIGHT__ },
    { "HBOMBAMMO",           DVPTR(HBOMBAMMO),           HBOMBAMMO__ },
    { "HEADJIB1",            DVPTR(HEADJIB1),            HEADJIB1__ },
    { "HEALTHBOX",           DVPTR(HEALTHBOX),           HEALTHBOX__ },
    { "HEAT_ICON",           DVPTR(HEAT_ICON),           HEAT_ICON__ },
    { "HEATSENSOR",          DVPTR(HEATSENSOR),          HEATSENSOR__ },
    { "HEAVYHBOMB",          DVPTR(HEAVYHBOMB),          HEAVYHBOMB__ },
    { "HELECOPT",            DVPTR(HELECOPT),            HELECOPT__ },
    { "HOLODUKE",            DVPTR(HOLODUKE),            HOLODUKE__ },
    { "HOLODUKE_ICON",       DVPTR(HOLODUKE_ICON),       HOLODUKE_ICON__ },
    { "HORSEONSIDE",         DVPTR(HORSEONSIDE),         HORSEONSIDE__ },
    { "HOTMEAT",             DVPTR(HOTMEAT),             HOTMEAT__ },
    { "HURTRAIL",            DVPTR(HURTRAIL),            HURTRAIL__ },
    { "HYDRENT",             DVPTR(HYDRENT),             HYDRENT__ },
    { "HYDROPLANT",          DVPTR(HYDROPLANT),          HYDROPLANT__ },
    { "INDY",                DVPTR(INDY),                INDY__ },
    { "INGAMEDUKETHREEDEE",  DVPTR(INGAMEDUKETHREEDEE),  INGAMEDUKETHREEDEE__ },
    { "INNERJAW",            DVPTR(INNERJAW),            INNERJAW__ },
    { "INVENTORYBOX",        DVPTR(INVENTORYBOX),        INVENTORYBOX__ },
    { "IVUNIT",              DVPTR(IVUNIT),              IVUNIT__ },
    { "JETPACK",             DVPTR(JETPACK),             JETPACK__ },
    { "JETPACK_ICON",        DVPTR(JETPACK_ICON),        JETPACK_ICON__ },
    { "JIBS1",               DVPTR(JIBS1),               JIBS1__ },
    { "JIBS2",               DVPTR(JIBS2),               JIBS2__ },
    { "JIBS3",               DVPTR(JIBS3),               JIBS3__ },
    { "JIBS4",               DVPTR(JIBS4),               JIBS4__ },
    { "JIBS5",               DVPTR(JIBS5),               JIBS5__ },
    { "JIBS6",               DVPTR(JIBS6),               JIBS6__ },
    { "JURYGUY",             DVPTR(JURYGUY),             JURYGUY__ },
    { "KILLSICON",           DVPTR(KILLSICON),           KILLSICON__ },
    { "KNEE",                DVPTR(KNEE),                KNEE__ },
    { "LA",                  DVPTR(LA),                  LA__ },
    { "LASERLINE",           DVPTR(LASERLINE),           LASERLINE__ },
    { "LASERSITE",           DVPTR(LASERSITE),           LASERSITE__ },
    { "LEGJIB1",             DVPTR(LEGJIB1),             LEGJIB1__ },
    { "LETTER",              DVPTR(LETTER),              LETTER__ },
    { "LIGHTSWITCH",         DVPTR(LIGHTSWITCH),         LIGHTSWITCH__ },
    { "LIGHTSWITCH2",        DVPTR(LIGHTSWITCH2),        LIGHTSWITCH2__ },
    { "LIZMAN",              DVPTR(LIZMAN),              LIZMAN__ },
    { "LIZMANARM1",          DVPTR(LIZMANARM1),          LIZMANARM1__ },
    { "LIZMANFEEDING",       DVPTR(LIZMANFEEDING),       LIZMANFEEDING__ },
    { "LIZMANHEAD1",         DVPTR(LIZMANHEAD1),         LIZMANHEAD1__ },
    { "LIZMANJUMP",          DVPTR(LIZMANJUMP),          LIZMANJUMP__ },
    { "LIZMANLEG1",          DVPTR(LIZMANLEG1),          LIZMANLEG1__ },
    { "LIZMANSPITTING",      DVPTR(LIZMANSPITTING),      LIZMANSPITTING__ },
    { "LIZMANSTAYPUT",       DVPTR(LIZMANSTAYPUT),       LIZMANSTAYPUT__ },
    { "LIZTROOP",            DVPTR(LIZTROOP),            LIZTROOP__ },
    { "LIZTROOPDUCKING",     DVPTR(LIZTROOPDUCKING),     LIZTROOPDUCKING__ },
    { "LIZTROOPJETPACK",     DVPTR(LIZTROOPJETPACK),     LIZTROOPJETPACK__ },
    { "LIZTROOPJUSTSIT",     DVPTR(LIZTROOPJUSTSIT),     LIZTROOPJUSTSIT__ },
    { "LIZTROOPONTOILET",    DVPTR(LIZTROOPONTOILET),    LIZTROOPONTOILET__ },
    { "LIZTROOPRUNNING",     DVPTR(LIZTROOPRUNNING),     LIZTROOPRUNNING__ },
    { "LIZTROOPSHOOT",       DVPTR(LIZTROOPSHOOT),       LIZTROOPSHOOT__ },
    { "LIZTROOPSTAYPUT",     DVPTR(LIZTROOPSTAYPUT),     LIZTROOPSTAYPUT__ },
    { "LOADSCREEN",          DVPTR(LOADSCREEN),          LOADSCREEN__ },
    { "LOCATORS",            DVPTR(LOCATORS),            LOCATORS__ },
    { "LOCKSWITCH1",         DVPTR(LOCKSWITCH1),         LOCKSWITCH1__ },
    { "LOOGIE",              DVPTR(LOOGIE),              LOOGIE__ },
    { "LUKE",                DVPTR(LUKE),                LUKE__ },
    { "MAIL",                DVPTR(MAIL),                MAIL__ },
    { "MAN",                 DVPTR(MAN),                 MAN__ },
    { "MAN2",                DVPTR(MAN2),                MAN2__ },
    { "MASKWALL1",           DVPTR(MASKWALL1),           MASKWALL1__ },
    { "MASKWALL10",          DVPTR(MASKWALL10),          MASKWALL10__ },
    { "MASKWALL11",          DVPTR(MASKWALL11),          MASKWALL11__ },
    { "MASKWALL12",          DVPTR(MASKWALL12),          MASKWALL12__ },
    { "MASKWALL13",          DVPTR(MASKWALL13),          MASKWALL13__ },
    { "MASKWALL14",          DVPTR(MASKWALL14),          MASKWALL14__ },
    { "MASKWALL15",          DVPTR(MASKWALL15),          MASKWALL15__ },
    { "MASKWALL2",           DVPTR(MASKWALL2),           MASKWALL2__ },
    { "MASKWALL3",           DVPTR(MASKWALL3),           MASKWALL3__ },
    { "MASKWALL4",           DVPTR(MASKWALL4),           MASKWALL4__ },
    { "MASKWALL5",           DVPTR(MASKWALL5),           MASKWALL5__ },
    { "MASKWALL6",           DVPTR(MASKWALL6),           MASKWALL6__ },
    { "MASKWALL7",           DVPTR(MASKWALL7),           MASKWALL7__ },
    { "MASKWALL8",           DVPTR(MASKWALL8),           MASKWALL8__ },
    { "MASKWALL9",           DVPTR(MASKWALL9),           MASKWALL9__ },
    { "MASTERSWITCH",        DVPTR(MASTERSWITCH),        MASTERSWITCH__ },
    { "MENUBAR",             DVPTR(MENUBAR),             MENUBAR__ },
    { "MENUSCREEN",          DVPTR(MENUSCREEN),          MENUSCREEN__ },
    { "MIKE",                DVPTR(MIKE),                MIKE__ },
    { "MINIFONT",            DVPTR(MINIFONT),            MINIFONT__ },
    { "MIRROR",              DVPTR(MIRROR),              MIRROR__ },
    { "MIRRORBROKE",         DVPTR(MIRRORBROKE),         MIRRORBROKE__ },
    { "MONEY",               DVPTR(MONEY),               MONEY__ },
    { "MONK",                DVPTR(MONK),                MONK__ },
    { "MOONSKY1",            DVPTR(MOONSKY1),            MOONSKY1__ },
    { "MORTER",              DVPTR(MORTER),              MORTER__ },
    { "MOVIECAMERA",         DVPTR(MOVIECAMERA),         MOVIECAMERA__ },
    { "MULTISWITCH",         DVPTR(MULTISWITCH),         MULTISWITCH__ },
    { "MUSICANDSFX",         DVPTR(MUSICANDSFX),         MUSICANDSFX__ },
    { "NAKED1",              DVPTR(NAKED1),              NAKED1__ },
    { "NATURALLIGHTNING",    DVPTR(NATURALLIGHTNING),    NATURALLIGHTNING__ },
    { "NEON1",               DVPTR(NEON1),               NEON1__ },
    { "NEON2",               DVPTR(NEON2),               NEON2__ },
    { "NEON3",               DVPTR(NEON3),               NEON3__ },
    { "NEON4",               DVPTR(NEON4),               NEON4__ },
    { "NEON5",               DVPTR(NEON5),               NEON5__ },
    { "NEON6",               DVPTR(NEON6),               NEON6__ },
    { "NEWBEAST",            DVPTR(NEWBEAST),            NEWBEAST__ },
    { "NEWBEASTSTAYPUT",     DVPTR(NEWBEASTSTAYPUT),     NEWBEASTSTAYPUT__ },
    { "NUKEBARREL",          DVPTR(NUKEBARREL),          NUKEBARREL__ },
    { "NUKEBARRELDENTED",    DVPTR(NUKEBARRELDENTED),    NUKEBARRELDENTED__ },
    { "NUKEBARRELLEAKED",    DVPTR(NUKEBARRELLEAKED),    NUKEBARRELLEAKED__ },
    { "NUKEBUTTON",          DVPTR(NUKEBUTTON),          NUKEBUTTON__ },
    { "OCEANSPRITE1",        DVPTR(OCEANSPRITE1),        OCEANSPRITE1__ },
    { "OCEANSPRITE2",        DVPTR(OCEANSPRITE2),        OCEANSPRITE2__ },
    { "OCEANSPRITE3",        DVPTR(OCEANSPRITE3),        OCEANSPRITE3__ },
    { "OCEANSPRITE4",        DVPTR(OCEANSPRITE4),        OCEANSPRITE4__ },
    { "OCEANSPRITE5",        DVPTR(OCEANSPRITE5),        OCEANSPRITE5__ },
    { "OCTABRAIN",           DVPTR(OCTABRAIN),           OCTABRAIN__ },
    { "OCTABRAINSTAYPUT",    DVPTR(OCTABRAINSTAYPUT),    OCTABRAINSTAYPUT__ },
    { "OJ",                  DVPTR(OJ),                  OJ__ },
    { "OOZ",                 DVPTR(OOZ),                 OOZ__ },
    { "OOZ2",                DVPTR(OOZ2),                OOZ2__ },
    { "OOZFILTER",           DVPTR(OOZFILTER),           OOZFILTER__ },
    { "ORDERING",            DVPTR(ORDERING),            ORDERING__ },
    { "ORGANTIC",            DVPTR(ORGANTIC),            ORGANTIC__ },
    { "PANNEL1",             DVPTR(PANNEL1),             PANNEL1__ },
    { "PANNEL2",             DVPTR(PANNEL2),             PANNEL2__ },
    { "PANNEL3",             DVPTR(PANNEL3),             PANNEL3__ },
    { "PAPER",               DVPTR(PAPER),               PAPER__ },
    { "PIGCOP",              DVPTR(PIGCOP),              PIGCOP__ },
    { "PIGCOPDIVE",          DVPTR(PIGCOPDIVE),          PIGCOPDIVE__ },
    { "PIGCOPSTAYPUT",       DVPTR(PIGCOPSTAYPUT),       PIGCOPSTAYPUT__ },
    { "PIPE1",               DVPTR(PIPE1),               PIPE1__ },
    { "PIPE1B",              DVPTR(PIPE1B),              PIPE1B__ },
    { "PIPE2",               DVPTR(PIPE2),               PIPE2__ },
    { "PIPE2B",              DVPTR(PIPE2B),              PIPE2B__ },
    { "PIPE3",               DVPTR(PIPE3),               PIPE3__ },
    { "PIPE3B",              DVPTR(PIPE3B),              PIPE3B__ },
    { "PIPE4",               DVPTR(PIPE4),               PIPE4__ },
    { "PIPE4B",              DVPTR(PIPE4B),              PIPE4B__ },
    { "PIPE5",               DVPTR(PIPE5),               PIPE5__ },
    { "PIPE5B",              DVPTR(PIPE5B),              PIPE5B__ },
    { "PIPE6",               DVPTR(PIPE6),               PIPE6__ },
    { "PIPE6B",              DVPTR(PIPE6B),              PIPE6B__ },
    { "PLAYERONWATER",       DVPTR(PLAYERONWATER),       PLAYERONWATER__ },
    { "PLUG",                DVPTR(PLUG),                PLUG__ },
    { "PLUTOPAKSPRITE",      DVPTR(PLUTOPAKSPRITE),      PLUTOPAKSPRITE__ },
    { "POCKET",              DVPTR(POCKET),              POCKET__ },
    { "PODFEM1",             DVPTR(PODFEM1),             PODFEM1__ },
    { "POT1",                DVPTR(POT1),                POT1__ },
    { "POT2",                DVPTR(POT2),                POT2__ },
    { "POT3",                DVPTR(POT3),                POT3__ },
    { "POWERSWITCH1",        DVPTR(POWERSWITCH1),        POWERSWITCH1__ },
    { "POWERSWITCH2",        DVPTR(POWERSWITCH2),        POWERSWITCH2__ },
    { "PUKE",                DVPTR(PUKE),                PUKE__ },
    { "PULLSWITCH",          DVPTR(PULLSWITCH),          PULLSWITCH__ },
    { "PURPLELAVA",          DVPTR(PURPLELAVA),          PURPLELAVA__ },
    { "QUEBALL",             DVPTR(QUEBALL),             QUEBALL__ },
    { "RADIUSEXPLOSION",     DVPTR(RADIUSEXPLOSION),     RADIUSEXPLOSION__ },
    { "RAT",                 DVPTR(RAT),                 RAT__ },
    { "REACTOR",             DVPTR(REACTOR),             REACTOR__ },
    { "REACTOR2",            DVPTR(REACTOR2),            REACTOR2__ },
    { "REACTOR2BURNT",       DVPTR(REACTOR2BURNT),       REACTOR2BURNT__ },
    { "REACTOR2SPARK",       DVPTR(REACTOR2SPARK),       REACTOR2SPARK__ },
    { "REACTORBURNT",        DVPTR(REACTORBURNT),        REACTORBURNT__ },
    { "REACTORSPARK",        DVPTR(REACTORSPARK),        REACTORSPARK__ },
    { "RECON",               DVPTR(RECON),               RECON__ },
    { "RESPAWN",             DVPTR(RESPAWN),             RESPAWN__ },
    { "RESPAWNMARKERGREEN",  DVPTR(RESPAWNMARKERGREEN),  RESPAWNMARKERGREEN__ },
    { "RESPAWNMARKERRED",    DVPTR(RESPAWNMARKERRED),    RESPAWNMARKERRED__ },
    { "RESPAWNMARKERYELLOW", DVPTR(RESPAWNMARKERYELLOW), RESPAWNMARKERYELLOW__ },
    { "ROTATEGUN",           DVPTR(ROTATEGUN),           ROTATEGUN__ },
    { "RPG",                 DVPTR(RPG),                 RPG__ },
    { "RPGAMMO",             DVPTR(RPGAMMO),             RPGAMMO__ },
    { "RPGGUN",              DVPTR(RPGGUN),              RPGGUN__ },
    { "RPGSPRITE",           DVPTR(RPGSPRITE),           RPGSPRITE__ },
    { "RUBBERCAN",           DVPTR(RUBBERCAN),           RUBBERCAN__ },
    { "SATELITE",            DVPTR(SATELITE),            SATELITE__ },
    { "SCALE",               DVPTR(SCALE),               SCALE__ },
    { "SCRAP1",              DVPTR(SCRAP1),              SCRAP1__ },
    { "SCRAP2",              DVPTR(SCRAP2),              SCRAP2__ },
    { "SCRAP3",              DVPTR(SCRAP3),              SCRAP3__ },
    { "SCRAP4",              DVPTR(SCRAP4),              SCRAP4__ },
    { "SCRAP5",              DVPTR(SCRAP5),              SCRAP5__ },
    { "SCRAP6",              DVPTR(SCRAP6),              SCRAP6__ },
    { "SCREENBREAK1",        DVPTR(SCREENBREAK1),        SCREENBREAK1__ },
    { "SCREENBREAK10",       DVPTR(SCREENBREAK10),       SCREENBREAK10__ },
    { "SCREENBREAK11",       DVPTR(SCREENBREAK11),       SCREENBREAK11__ },
    { "SCREENBREAK12",       DVPTR(SCREENBREAK12),       SCREENBREAK12__ },
    { "SCREENBREAK13",       DVPTR(SCREENBREAK13),       SCREENBREAK13__ },
    { "SCREENBREAK14",       DVPTR(SCREENBREAK14),       SCREENBREAK14__ },
    { "SCREENBREAK15",       DVPTR(SCREENBREAK15),       SCREENBREAK15__ },
    { "SCREENBREAK16",       DVPTR(SCREENBREAK16),       SCREENBREAK16__ },
    { "SCREENBREAK17",       DVPTR(SCREENBREAK17),       SCREENBREAK17__ },
    { "SCREENBREAK18",       DVPTR(SCREENBREAK18),       SCREENBREAK18__ },
    { "SCREENBREAK19",       DVPTR(SCREENBREAK19),       SCREENBREAK19__ },
    { "SCREENBREAK2",        DVPTR(SCREENBREAK2),        SCREENBREAK2__ },
    { "SCREENBREAK3",        DVPTR(SCREENBREAK3),        SCREENBREAK3__ },
    { "SCREENBREAK4",        DVPTR(SCREENBREAK4),        SCREENBREAK4__ },
    { "SCREENBREAK5",        DVPTR(SCREENBREAK5),        SCREENBREAK5__ },
    { "SCREENBREAK6",        DVPTR(SCREENBREAK6),        SCREENBREAK6__ },
    { "SCREENBREAK7",        DVPTR(SCREENBREAK7),        SCREENBREAK7__ },
    { "SCREENBREAK8",        DVPTR(SCREENBREAK8),        SCREENBREAK8__ },
    { "SCREENBREAK9",        DVPTR(SCREENBREAK9),        SCREENBREAK9__ },
    { "SCUBAMASK",           DVPTR(SCUBAMASK),           SCUBAMASK__ },
    { "SECTOREFFECTOR",      DVPTR(SECTOREFFECTOR),      SECTOREFFECTOR__ },
    { "SEENINE",             DVPTR(SEENINE),             SEENINE__ },
    { "SEENINEDEAD",         DVPTR(SEENINEDEAD),         SEENINEDEAD__ },
    { "SELECTDIR",           DVPTR(SELECTDIR),           SELECTDIR__ },
    { "SHARK",               DVPTR(SHARK),               SHARK__ },
    { "SHELL",               DVPTR(SHELL),               SHELL__ },
    { "SHIELD",              DVPTR(SHIELD),              SHIELD__ },
    { "SHOTGUN",             DVPTR(SHOTGUN),             SHOTGUN__ },
    { "SHOTGUNAMMO",         DVPTR(SHOTGUNAMMO),         SHOTGUNAMMO__ },
    { "SHOTGUNSHELL",        DVPTR(SHOTGUNSHELL),        SHOTGUNSHELL__ },
    { "SHOTGUNSPRITE",       DVPTR(SHOTGUNSPRITE),       SHOTGUNSPRITE__ },
    { "SHOTSPARK1",          DVPTR(SHOTSPARK1),          SHOTSPARK1__ },
    { "SHRINKER",            DVPTR(SHRINKER),            SHRINKER__ },
    { "SHRINKEREXPLOSION",   DVPTR(SHRINKEREXPLOSION),   SHRINKEREXPLOSION__ },
    { "SHRINKERSPRITE",      DVPTR(SHRINKERSPRITE),      SHRINKERSPRITE__ },
    { "SHRINKSPARK",         DVPTR(SHRINKSPARK),         SHRINKSPARK__ },
    { "SIDEBOLT1",           DVPTR(SIDEBOLT1),           SIDEBOLT1__ },
    { "SIGN1",               DVPTR(SIGN1),               SIGN1__ },
    { "SIGN2",               DVPTR(SIGN2),               SIGN2__ },
    { "SIXPAK",              DVPTR(SIXPAK),              SIXPAK__ },
    { "SLIDEBAR",            DVPTR(SLIDEBAR),            SLIDEBAR__ },
    { "SLOTDOOR",            DVPTR(SLOTDOOR),            SLOTDOOR__ },
    { "SMALLFNTCURSOR",      DVPTR(SMALLFNTCURSOR),      SMALLFNTCURSOR__ },
    { "SMALLSMOKE",          DVPTR(SMALLSMOKE),          SMALLSMOKE__ },
    { "SOLARPANNEL",         DVPTR(SOLARPANNEL),         SOLARPANNEL__ },
    { "SPACEDOORSWITCH",     DVPTR(SPACEDOORSWITCH),     SPACEDOORSWITCH__ },
    { "SPACELIGHTSWITCH",    DVPTR(SPACELIGHTSWITCH),    SPACELIGHTSWITCH__ },
    { "SPACEMARINE",         DVPTR(SPACEMARINE),         SPACEMARINE__ },
    { "SPEAKER",             DVPTR(SPEAKER),             SPEAKER__ },
    { "SPINNINGNUKEICON",    DVPTR(SPINNINGNUKEICON),    SPINNINGNUKEICON__ },
    { "SPIT",                DVPTR(SPIT),                SPIT__ },
    { "SPOTLITE",            DVPTR(SPOTLITE),            SPOTLITE__ },
    { "STAINGLASS1",         DVPTR(STAINGLASS1),         STAINGLASS1__ },
    { "STALL",               DVPTR(STALL),               STALL__ },
    { "STALLBROKE",          DVPTR(STALLBROKE),          STALLBROKE__ },
    { "STARTALPHANUM",       DVPTR(STARTALPHANUM),       STARTALPHANUM__ },
    { "STATIC",              DVPTR(STATIC),              STATIC__ },
    { "STATUE",              DVPTR(STATUE),              STATUE__ },
    { "STATUEFLASH",         DVPTR(STATUEFLASH),         STATUEFLASH__ },
    { "STEAM",               DVPTR(STEAM),               STEAM__ },
    { "STEROIDS",            DVPTR(STEROIDS),            STEROIDS__ },
    { "STEROIDS_ICON",       DVPTR(STEROIDS_ICON),       STEROIDS_ICON__ },
    { "STRIPEBALL",          DVPTR(STRIPEBALL),          STRIPEBALL__ },
    { "SUSHIPLATE1",         DVPTR(SUSHIPLATE1),         SUSHIPLATE1__ },
    { "SUSHIPLATE2",         DVPTR(SUSHIPLATE2),         SUSHIPLATE2__ },
    { "SUSHIPLATE3",         DVPTR(SUSHIPLATE3),         SUSHIPLATE3__ },
    { "SUSHIPLATE4",         DVPTR(SUSHIPLATE4),         SUSHIPLATE4__ },
    { "SUSHIPLATE5",         DVPTR(SUSHIPLATE5),         SUSHIPLATE5__ },
    { "TAMPON",              DVPTR(TAMPON),              TAMPON__ },
    { "TANK",                DVPTR(TANK),                TANK__ },
    { "TARGET",              DVPTR(TARGET),              TARGET__ },
    { "TECHLIGHT2",          DVPTR(TECHLIGHT2),          TECHLIGHT2__ },
    { "TECHLIGHT4",          DVPTR(TECHLIGHT4),          TECHLIGHT4__ },
    { "TECHLIGHTBUST2",      DVPTR(TECHLIGHTBUST2),      TECHLIGHTBUST2__ },
    { "TECHLIGHTBUST4",      DVPTR(TECHLIGHTBUST4),      TECHLIGHTBUST4__ },
    { "TECHSWITCH",          DVPTR(TECHSWITCH),          TECHSWITCH__ },
    { "TENSCREEN",           DVPTR(TENSCREEN),           TENSCREEN__ },
    { "TEXTBOX",             DVPTR(TEXTBOX),             TEXTBOX__ },
    { "TEXTSTORY",           DVPTR(TEXTSTORY),           TEXTSTORY__ },
    { "THREEBYFIVE",         DVPTR(THREEBYFIVE),         THREEBYFIVE__ },
    { "THREEDEE",            DVPTR(THREEDEE),            THREEDEE__ },
    { "TIP",                 DVPTR(TIP),                 TIP__ },
    { "TIRE",                DVPTR(TIRE),                TIRE__ },
    { "TOILET",              DVPTR(TOILET),              TOILET__ },
    { "TOILETBROKE",         DVPTR(TOILETBROKE),         TOILETBROKE__ },
    { "TOILETWATER",         DVPTR(TOILETWATER),         TOILETWATER__ },
    { "TONGUE",              DVPTR(TONGUE),              TONGUE__ },
    { "TOUCHPLATE",          DVPTR(TOUCHPLATE),          TOUCHPLATE__ },
    { "TOUGHGAL",            DVPTR(TOUGHGAL),            TOUGHGAL__ },
    { "TRANSPORTERBEAM",     DVPTR(TRANSPORTERBEAM),     TRANSPORTERBEAM__ },
    { "TRANSPORTERSTAR",     DVPTR(TRANSPORTERSTAR),     TRANSPORTERSTAR__ },
    { "TRASH",               DVPTR(TRASH),               TRASH__ },
    { "TREE1",               DVPTR(TREE1),               TREE1__ },
    { "TREE2",               DVPTR(TREE2),               TREE2__ },
    { "TRIPBOMB",            DVPTR(TRIPBOMB),            TRIPBOMB__ },
    { "TRIPBOMBSPRITE",      DVPTR(TRIPBOMBSPRITE),      TRIPBOMBSPRITE__ },
    { "TRIPODCAMERA",        DVPTR(TRIPODCAMERA),        TRIPODCAMERA__ },
    { "VACUUM",              DVPTR(VACUUM),              VACUUM__ },
    { "VASE",                DVPTR(VASE),                VASE__ },
    { "VENDMACHINE",         DVPTR(VENDMACHINE),         VENDMACHINE__ },
    { "VICTORY1",            DVPTR(VICTORY1),            VICTORY1__ },
    { "VIEWBORDER",          DVPTR(VIEWBORDER),          VIEWBORDER__ },
    { "VIEWSCREEN",          DVPTR(VIEWSCREEN),          VIEWSCREEN__ },
    { "VIEWSCREEN2",         DVPTR(VIEWSCREEN2),         VIEWSCREEN2__ },
    { "W_FORCEFIELD",        DVPTR(W_FORCEFIELD),        W_FORCEFIELD__ },
    { "W_HITTECHWALL1",      DVPTR(W_HITTECHWALL1),      W_HITTECHWALL1__ },
    { "W_HITTECHWALL10",     DVPTR(W_HITTECHWALL10),     W_HITTECHWALL10__ },
    { "W_HITTECHWALL15",     DVPTR(W_HITTECHWALL15),     W_HITTECHWALL15__ },
    { "W_HITTECHWALL16",     DVPTR(W_HITTECHWALL16),     W_HITTECHWALL16__ },
    { "W_HITTECHWALL2",      DVPTR(W_HITTECHWALL2),      W_HITTECHWALL2__ },
    { "W_HITTECHWALL3",      DVPTR(W_HITTECHWALL3),      W_HITTECHWALL3__ },
    { "W_HITTECHWALL4",      DVPTR(W_HITTECHWALL4),      W_HITTECHWALL4__ },
    { "W_MILKSHELF",         DVPTR(W_MILKSHELF),         W_MILKSHELF__ },
    { "W_MILKSHELFBROKE",    DVPTR(W_MILKSHELFBROKE),    W_MILKSHELFBROKE__ },
    { "W_NUMBERS",           DVPTR(W_NUMBERS),           W_NUMBERS__ },
    { "W_SCREENBREAK",       DVPTR(W_SCREENBREAK),       W_SCREENBREAK__ },
    { "W_TECHWALL1",         DVPTR(W_TECHWALL1),         W_TECHWALL1__ },
    { "W_TECHWALL10",        DVPTR(W_TECHWALL10),        W_TECHWALL10__ },
    { "W_TECHWALL11",        DVPTR(W_TECHWALL11),        W_TECHWALL11__ },
    { "W_TECHWALL12",        DVPTR(W_TECHWALL12),        W_TECHWALL12__ },
    { "W_TECHWALL13",        DVPTR(W_TECHWALL13),        W_TECHWALL13__ },
    { "W_TECHWALL14",        DVPTR(W_TECHWALL14),        W_TECHWALL14__ },
    { "W_TECHWALL15",        DVPTR(W_TECHWALL15),        W_TECHWALL15__ },
    { "W_TECHWALL16",        DVPTR(W_TECHWALL16),        W_TECHWALL16__ },
    { "W_TECHWALL2",         DVPTR(W_TECHWALL2),         W_TECHWALL2__ },
    { "W_TECHWALL3",         DVPTR(W_TECHWALL3),         W_TECHWALL3__ },
    { "W_TECHWALL4",         DVPTR(W_TECHWALL4),         W_TECHWALL4__ },
    { "W_TECHWALL5",         DVPTR(W_TECHWALL5),         W_TECHWALL5__ },
    { "W_TECHWALL6",         DVPTR(W_TECHWALL6),         W_TECHWALL6__ },
    { "W_TECHWALL7",         DVPTR(W_TECHWALL7),         W_TECHWALL7__ },
    { "W_TECHWALL8",         DVPTR(W_TECHWALL8),         W_TECHWALL8__ },
    { "W_TECHWALL9",         DVPTR(W_TECHWALL9),         W_TECHWALL9__ },
    { "WAITTOBESEATED",      DVPTR(WAITTOBESEATED),      WAITTOBESEATED__ },
    { "WALLBLOOD1",          DVPTR(WALLBLOOD1),          WALLBLOOD1__ },
    { "WALLBLOOD2",          DVPTR(WALLBLOOD2),          WALLBLOOD2__ },
    { "WALLBLOOD3",          DVPTR(WALLBLOOD3),          WALLBLOOD3__ },
    { "WALLBLOOD4",          DVPTR(WALLBLOOD4),          WALLBLOOD4__ },
    { "WALLBLOOD5",          DVPTR(WALLBLOOD5),          WALLBLOOD5__ },
    { "WALLBLOOD7",          DVPTR(WALLBLOOD7),          WALLBLOOD7__ },
    { "WALLBLOOD8",          DVPTR(WALLBLOOD8),          WALLBLOOD8__ },
    { "WALLLIGHT1",          DVPTR(WALLLIGHT1),          WALLLIGHT1__ },
    { "WALLLIGHT2",          DVPTR(WALLLIGHT2),          WALLLIGHT2__ },
    { "WALLLIGHT3",          DVPTR(WALLLIGHT3),          WALLLIGHT3__ },
    { "WALLLIGHT4",          DVPTR(WALLLIGHT4),          WALLLIGHT4__ },
    { "WALLLIGHTBUST1",      DVPTR(WALLLIGHTBUST1),      WALLLIGHTBUST1__ },
    { "WALLLIGHTBUST2",      DVPTR(WALLLIGHTBUST2),      WALLLIGHTBUST2__ },
    { "WALLLIGHTBUST3",      DVPTR(WALLLIGHTBUST3),      WALLLIGHTBUST3__ },
    { "WALLLIGHTBUST4",      DVPTR(WALLLIGHTBUST4),      WALLLIGHTBUST4__ },
    { "WATERBUBBLE",         DVPTR(WATERBUBBLE),         WATERBUBBLE__ },
    { "WATERBUBBLEMAKER",    DVPTR(WATERBUBBLEMAKER),    WATERBUBBLEMAKER__ },
    { "WATERDRIP",           DVPTR(WATERDRIP),           WATERDRIP__ },
    { "WATERDRIPSPLASH",     DVPTR(WATERDRIPSPLASH),     WATERDRIPSPLASH__ },
    { "WATERFOUNTAIN",       DVPTR(WATERFOUNTAIN),       WATERFOUNTAIN__ },
    { "WATERFOUNTAINBROKE",  DVPTR(WATERFOUNTAINBROKE),  WATERFOUNTAINBROKE__ },
    { "WATERSPLASH2",        DVPTR(WATERSPLASH2),        WATERSPLASH2__ },
    { "WATERTILE2",          DVPTR(WATERTILE2),          WATERTILE2__ },
    { "WEATHERWARN",         DVPTR(WEATHERWARN),         WEATHERWARN__ },
    { "WINDOWBORDER1",       DVPTR(WINDOWBORDER1),       WINDOWBORDER1__ },
    { "WINDOWBORDER2",       DVPTR(WINDOWBORDER2),       WINDOWBORDER2__ },
    { "WOMAN",               DVPTR(WOMAN),               WOMAN__ },
    { "WOODENHORSE",         DVPTR(WOODENHORSE),         WOODENHORSE__ },
    { "XXXSTACY",            DVPTR(XXXSTACY),            XXXSTACY__ },
    { "WIDESCREENSTATUSBAR", DVPTR(WIDESCREENSTATUSBAR), WIDESCREENSTATUSBAR__ },
    { "RPGGUNWIDE",          DVPTR(RPGGUNWIDE),          RPGGUNWIDE__ },
    { "FIRSTGUNRELOADWIDE",  DVPTR(FIRSTGUNRELOADWIDE),  FIRSTGUNRELOADWIDE__ },
    { "FREEZEWIDE",          DVPTR(FREEZEWIDE),          FREEZEWIDE__ },
    { "FREEZEFIREWIDE",      DVPTR(FREEZEFIREWIDE),      FREEZEFIREWIDE__ },
    { "SHRINKERWIDE",        DVPTR(SHRINKERWIDE),        SHRINKERWIDE__ },
    { "CRACKKNUCKLESWIDE",   DVPTR(CRACKKNUCKLESWIDE),   CRACKKNUCKLESWIDE__ },
    { "FLAMETHROWERSPRITE",  DVPTR(FLAMETHROWERSPRITE),  FLAMETHROWERSPRITE__ },
    { "FLAMETHROWERAMMO",    DVPTR(FLAMETHROWERAMMO),    FLAMETHROWERAMMO__ },
    { "FLAMETHROWER",        DVPTR(FLAMETHROWER),        FLAMETHROWER__ },
    { "FLAMETHROWERFIRE",    DVPTR(FLAMETHROWERFIRE),    FLAMETHROWERFIRE__ },
    { "FLAMETHROWERFLAME",   DVPTR(FLAMETHROWERFLAME),   FLAMETHROWERFLAME__ },
    { "FLAMETHROWERPILOT",   DVPTR(FLAMETHROWERPILOT),   FLAMETHROWERPILOT__ },
    { "FIREBALL",            DVPTR(FIREBALL),            FIREBALL__ },
    { "ONFIRE",              DVPTR(ONFIRE),              ONFIRE__ },
    { "ONFIRESMOKE",         DVPTR(ONFIRESMOKE),         ONFIRESMOKE__ },
    { "BURNEDCORPSE",        DVPTR(BURNEDCORPSE),        BURNEDCORPSE__ },
    { "WHISPYSMOKE",         DVPTR(WHISPYSMOKE),         WHISPYSMOKE__ },
    { "FIREFLY",             DVPTR(FIREFLY),             FIREFLY__ },
    { "FIREFLYSHRINKEFFECT", DVPTR(FIREFLYSHRINKEFFECT), FIREFLYSHRINKEFFECT__ },
    { "FIREFLYGROWEFFECT",   DVPTR(FIREFLYGROWEFFECT),   FIREFLYGROWEFFECT__ },
    { "FIREFLYFLYINGEFFECT", DVPTR(FIREFLYFLYINGEFFECT), FIREFLYFLYINGEFFECT__ },
    { "BOSS5",               DVPTR(BOSS5),               BOSS5__ },
    { "BOSS5STAYPUT",        DVPTR(BOSS5STAYPUT),        BOSS5STAYPUT__ },
    { "LAVAPOOL",            DVPTR(LAVAPOOL),            LAVAPOOL__ },
    { "LAVASPLASH",          DVPTR(LAVASPLASH),          LAVASPLASH__ },
    { "LAVAPOOLBUBBLE",      DVPTR(LAVAPOOLBUBBLE),      LAVAPOOLBUBBLE__ },
    { "BOSS2STAYPUT",        DVPTR(BOSS2STAYPUT),        BOSS2STAYPUT__ },
    { "BOSS3STAYPUT",        DVPTR(BOSS3STAYPUT),        BOSS3STAYPUT__ },
    { "E32_TILE5736",        DVPTR(E32_TILE5736),        E32_TILE5736__ },
    { "E32_TILE5737",        DVPTR(E32_TILE5737),        E32_TILE5737__ },
    { "E32_TILE5846",        DVPTR(E32_TILE5846),        E32_TILE5846__ },
 };

#ifdef DYNTILEREMAP_ENABLE
int32_t ACCESS_ICON         = ACCESS_ICON__;
int32_t ACCESSCARD          = ACCESSCARD__;
int32_t ACCESSSWITCH        = ACCESSSWITCH__;
int32_t ACCESSSWITCH2       = ACCESSSWITCH2__;
int32_t ACTIVATOR           = ACTIVATOR__;
int32_t ACTIVATORLOCKED     = ACTIVATORLOCKED__;
int32_t AIRTANK             = AIRTANK__;
int32_t AIRTANK_ICON        = AIRTANK_ICON__;
int32_t ALIENSWITCH         = ALIENSWITCH__;
int32_t AMMO                = AMMO__;
int32_t AMMOBOX             = AMMOBOX__;
int32_t AMMOLOTS            = AMMOLOTS__;
int32_t ANTENNA             = ANTENNA__;
int32_t APLAYER             = APLAYER__;
int32_t APLAYERTOP          = APLAYERTOP__;
int32_t ARMJIB1             = ARMJIB1__;
int32_t ARROW               = ARROW__;
int32_t ATM                 = ATM__;
int32_t ATMBROKE            = ATMBROKE__;
int32_t ATOMICHEALTH        = ATOMICHEALTH__;
int32_t BANNER              = BANNER__;
int32_t BARBROKE            = BARBROKE__;
int32_t BATTERYAMMO         = BATTERYAMMO__;
int32_t BETASCREEN          = BETASCREEN__;
int32_t BETAVERSION         = BETAVERSION__;
int32_t BGRATE1             = BGRATE1__;
int32_t BIGALPHANUM         = BIGALPHANUM__;
int32_t BIGAPPOS            = BIGAPPOS__;
int32_t BIGCOLIN            = BIGCOLIN__;
int32_t BIGCOMMA            = BIGCOMMA__;
int32_t BIGFORCE            = BIGFORCE__;
int32_t BIGHOLE             = BIGHOLE__;
int32_t BIGORBIT1           = BIGORBIT1__;
int32_t BIGPERIOD           = BIGPERIOD__;
int32_t BIGQ                = BIGQ__;
int32_t BIGSEMI             = BIGSEMI__;
int32_t BIGX_               = BIGX__; // "BIGX" clashes on the Wii?
int32_t BLANKSCREEN         = BLANKSCREEN__;
int32_t BLIMP               = BLIMP__;
int32_t BLOOD               = BLOOD__;
int32_t BLOODPOOL           = BLOODPOOL__;
int32_t BLOODSPLAT1         = BLOODSPLAT1__;
int32_t BLOODSPLAT2         = BLOODSPLAT2__;
int32_t BLOODSPLAT3         = BLOODSPLAT3__;
int32_t BLOODSPLAT4         = BLOODSPLAT4__;
int32_t BLOODYPOLE          = BLOODYPOLE__;
int32_t BOLT1               = BOLT1__;
int32_t BONUSSCREEN         = BONUSSCREEN__;
int32_t BOOT_ICON           = BOOT_ICON__;
int32_t BOOTS               = BOOTS__;
int32_t BORNTOBEWILDSCREEN  = BORNTOBEWILDSCREEN__;
int32_t BOSS1               = BOSS1__;
int32_t BOSS1LOB            = BOSS1LOB__;
int32_t BOSS1SHOOT          = BOSS1SHOOT__;
int32_t BOSS1STAYPUT        = BOSS1STAYPUT__;
int32_t BOSS2               = BOSS2__;
int32_t BOSS3               = BOSS3__;
int32_t BOSS4               = BOSS4__;
int32_t BOSS4STAYPUT        = BOSS4STAYPUT__;
int32_t BOSSTOP             = BOSSTOP__;
int32_t BOTTLE1             = BOTTLE1__;
int32_t BOTTLE10            = BOTTLE10__;
int32_t BOTTLE11            = BOTTLE11__;
int32_t BOTTLE12            = BOTTLE12__;
int32_t BOTTLE13            = BOTTLE13__;
int32_t BOTTLE14            = BOTTLE14__;
int32_t BOTTLE15            = BOTTLE15__;
int32_t BOTTLE16            = BOTTLE16__;
int32_t BOTTLE17            = BOTTLE17__;
int32_t BOTTLE18            = BOTTLE18__;
int32_t BOTTLE19            = BOTTLE19__;
int32_t BOTTLE2             = BOTTLE2__;
int32_t BOTTLE3             = BOTTLE3__;
int32_t BOTTLE4             = BOTTLE4__;
int32_t BOTTLE5             = BOTTLE5__;
int32_t BOTTLE6             = BOTTLE6__;
int32_t BOTTLE7             = BOTTLE7__;
int32_t BOTTLE8             = BOTTLE8__;
int32_t BOTTOMSTATUSBAR     = BOTTOMSTATUSBAR__;
int32_t BOUNCEMINE          = BOUNCEMINE__;
int32_t BOX                 = BOX__;
int32_t BPANNEL1            = BPANNEL1__;
int32_t BPANNEL3            = BPANNEL3__;
int32_t BROKEFIREHYDRENT    = BROKEFIREHYDRENT__;
int32_t BROKEHYDROPLANT     = BROKEHYDROPLANT__;
int32_t BROKENCHAIR         = BROKENCHAIR__;
int32_t BULLETHOLE          = BULLETHOLE__;
int32_t BURNING             = BURNING__;
int32_t BURNING2            = BURNING2__;
int32_t CACTUS              = CACTUS__;
int32_t CACTUSBROKE         = CACTUSBROKE__;
int32_t CAMCORNER           = CAMCORNER__;
int32_t CAMERA1             = CAMERA1__;
int32_t CAMERALIGHT         = CAMERALIGHT__;
int32_t CAMERAPOLE          = CAMERAPOLE__;
int32_t CAMLIGHT            = CAMLIGHT__;
int32_t CANWITHSOMETHING    = CANWITHSOMETHING__;
int32_t CANWITHSOMETHING2   = CANWITHSOMETHING2__;
int32_t CANWITHSOMETHING3   = CANWITHSOMETHING3__;
int32_t CANWITHSOMETHING4   = CANWITHSOMETHING4__;
int32_t CEILINGSTEAM        = CEILINGSTEAM__;
int32_t CHAINGUN            = CHAINGUN__;
int32_t CHAINGUNSPRITE      = CHAINGUNSPRITE__;
int32_t CHAIR1              = CHAIR1__;
int32_t CHAIR2              = CHAIR2__;
int32_t CHAIR3              = CHAIR3__;
int32_t CIRCLEPANNEL        = CIRCLEPANNEL__;
int32_t CIRCLEPANNELBROKE   = CIRCLEPANNELBROKE__;
int32_t CLOUDYOCEAN         = CLOUDYOCEAN__;
int32_t CLOUDYSKIES         = CLOUDYSKIES__;
int32_t COLA                = COLA__;
int32_t COLAMACHINE         = COLAMACHINE__;
int32_t COMMANDER           = COMMANDER__;
int32_t COMMANDERSTAYPUT    = COMMANDERSTAYPUT__;
int32_t CONE                = CONE__;
int32_t COOLEXPLOSION1      = COOLEXPLOSION1__;
int32_t CRACK1              = CRACK1__;
int32_t CRACK2              = CRACK2__;
int32_t CRACK3              = CRACK3__;
int32_t CRACK4              = CRACK4__;
int32_t CRACKKNUCKLES       = CRACKKNUCKLES__;
int32_t CRANE               = CRANE__;
int32_t CRANEPOLE           = CRANEPOLE__;
int32_t CROSSHAIR           = CROSSHAIR__;
int32_t CRYSTALAMMO         = CRYSTALAMMO__;
int32_t CYCLER              = CYCLER__;
int32_t DEVISTATOR          = DEVISTATOR__;
int32_t DEVISTATORAMMO      = DEVISTATORAMMO__;
int32_t DEVISTATORSPRITE    = DEVISTATORSPRITE__;
int32_t DIGITALNUM          = DIGITALNUM__;
int32_t DIPSWITCH           = DIPSWITCH__;
int32_t DIPSWITCH2          = DIPSWITCH2__;
int32_t DIPSWITCH3          = DIPSWITCH3__;
int32_t DOLPHIN1            = DOLPHIN1__;
int32_t DOLPHIN2            = DOLPHIN2__;
int32_t DOMELITE            = DOMELITE__;
int32_t DOORSHOCK           = DOORSHOCK__;
int32_t DOORTILE1           = DOORTILE1__;
int32_t DOORTILE10          = DOORTILE10__;
int32_t DOORTILE11          = DOORTILE11__;
int32_t DOORTILE12          = DOORTILE12__;
int32_t DOORTILE14          = DOORTILE14__;
int32_t DOORTILE15          = DOORTILE15__;
int32_t DOORTILE16          = DOORTILE16__;
int32_t DOORTILE17          = DOORTILE17__;
int32_t DOORTILE18          = DOORTILE18__;
int32_t DOORTILE19          = DOORTILE19__;
int32_t DOORTILE2           = DOORTILE2__;
int32_t DOORTILE20          = DOORTILE20__;
int32_t DOORTILE21          = DOORTILE21__;
int32_t DOORTILE22          = DOORTILE22__;
int32_t DOORTILE23          = DOORTILE23__;
int32_t DOORTILE3           = DOORTILE3__;
int32_t DOORTILE4           = DOORTILE4__;
int32_t DOORTILE5           = DOORTILE5__;
int32_t DOORTILE6           = DOORTILE6__;
int32_t DOORTILE7           = DOORTILE7__;
int32_t DOORTILE8           = DOORTILE8__;
int32_t DOORTILE9           = DOORTILE9__;
int32_t DREALMS             = DREALMS__;
int32_t DRONE               = DRONE__;
int32_t DUCK                = DUCK__;
int32_t DUKECAR             = DUKECAR__;
int32_t DUKEGUN             = DUKEGUN__;
int32_t DUKELEG             = DUKELEG__;
int32_t DUKELYINGDEAD       = DUKELYINGDEAD__;
int32_t DUKENUKEM           = DUKENUKEM__;
int32_t DUKETAG             = DUKETAG__;
int32_t DUKETORSO           = DUKETORSO__;
int32_t EGG                 = EGG__;
int32_t ENDALPHANUM         = ENDALPHANUM__;
int32_t EXPLODINGBARREL     = EXPLODINGBARREL__;
int32_t EXPLODINGBARREL2    = EXPLODINGBARREL2__;
int32_t EXPLOSION2          = EXPLOSION2__;
int32_t EXPLOSION2BOT       = EXPLOSION2BOT__;
int32_t F1HELP              = F1HELP__;
int32_t FANSHADOW           = FANSHADOW__;
int32_t FANSHADOWBROKE      = FANSHADOWBROKE__;
int32_t FANSPRITE           = FANSPRITE__;
int32_t FANSPRITEBROKE      = FANSPRITEBROKE__;
int32_t FECES               = FECES__;
int32_t FEM1                = FEM1__;
int32_t FEM10               = FEM10__;
int32_t FEM2                = FEM2__;
int32_t FEM3                = FEM3__;
int32_t FEM4                = FEM4__;
int32_t FEM5                = FEM5__;
int32_t FEM6                = FEM6__;
int32_t FEM6PAD             = FEM6PAD__;
int32_t FEM7                = FEM7__;
int32_t FEM8                = FEM8__;
int32_t FEM9                = FEM9__;
int32_t FEMMAG1             = FEMMAG1__;
int32_t FEMMAG2             = FEMMAG2__;
int32_t FEMPIC1             = FEMPIC1__;
int32_t FEMPIC2             = FEMPIC2__;
int32_t FEMPIC3             = FEMPIC3__;
int32_t FEMPIC4             = FEMPIC4__;
int32_t FEMPIC5             = FEMPIC5__;
int32_t FEMPIC6             = FEMPIC6__;
int32_t FEMPIC7             = FEMPIC7__;
int32_t FETUS               = FETUS__;
int32_t FETUSBROKE          = FETUSBROKE__;
int32_t FETUSJIB            = FETUSJIB__;
int32_t FIRE                = FIRE__;
int32_t FIRE2               = FIRE2__;
int32_t FIREBARREL          = FIREBARREL__;
int32_t FIREEXT             = FIREEXT__;
int32_t FIRELASER           = FIRELASER__;
int32_t FIREVASE            = FIREVASE__;
int32_t FIRSTAID            = FIRSTAID__;
int32_t FIRSTAID_ICON       = FIRSTAID_ICON__;
int32_t FIRSTGUN            = FIRSTGUN__;
int32_t FIRSTGUNRELOAD      = FIRSTGUNRELOAD__;
int32_t FIRSTGUNSPRITE      = FIRSTGUNSPRITE__;
int32_t FIST                = FIST__;
int32_t FLOORFLAME          = FLOORFLAME__;
int32_t FLOORPLASMA         = FLOORPLASMA__;
int32_t FLOORSLIME          = FLOORSLIME__;
int32_t FOF                 = FOF__;
int32_t FOODOBJECT16        = FOODOBJECT16__;
int32_t FOOTPRINTS          = FOOTPRINTS__;
int32_t FOOTPRINTS2         = FOOTPRINTS2__;
int32_t FOOTPRINTS3         = FOOTPRINTS3__;
int32_t FOOTPRINTS4         = FOOTPRINTS4__;
int32_t FORCERIPPLE         = FORCERIPPLE__;
int32_t FORCESPHERE         = FORCESPHERE__;
int32_t FRAGBAR             = FRAGBAR__;
int32_t FRAMEEFFECT1        = FRAMEEFFECT1__;
int32_t FRAMEEFFECT1_13     = FRAMEEFFECT1_13__;
int32_t FRANKENSTINESWITCH  = FRANKENSTINESWITCH__;
int32_t FREEZE              = FREEZE__;
int32_t FREEZEAMMO          = FREEZEAMMO__;
int32_t FREEZEBLAST         = FREEZEBLAST__;
int32_t FREEZESPRITE        = FREEZESPRITE__;
int32_t FUELPOD             = FUELPOD__;
int32_t GENERICPOLE         = GENERICPOLE__;
int32_t GENERICPOLE2        = GENERICPOLE2__;
int32_t GLASS               = GLASS__;
int32_t GLASS2              = GLASS2__;
int32_t GLASSPIECES         = GLASSPIECES__;
int32_t GPSPEED             = GPSPEED__;
int32_t GRATE1              = GRATE1__;
int32_t GREENSLIME          = GREENSLIME__;
int32_t GROWAMMO            = GROWAMMO__;
int32_t GROWSPARK           = GROWSPARK__;
int32_t GROWSPRITEICON      = GROWSPRITEICON__;
int32_t HANDHOLDINGACCESS   = HANDHOLDINGACCESS__;
int32_t HANDHOLDINGLASER    = HANDHOLDINGLASER__;
int32_t HANDPRINTSWITCH     = HANDPRINTSWITCH__;
int32_t HANDREMOTE          = HANDREMOTE__;
int32_t HANDSWITCH          = HANDSWITCH__;
int32_t HANDTHROW           = HANDTHROW__;
int32_t HANGLIGHT           = HANGLIGHT__;
int32_t HBOMBAMMO           = HBOMBAMMO__;
int32_t HEADJIB1            = HEADJIB1__;
int32_t HEALTHBOX           = HEALTHBOX__;
int32_t HEAT_ICON           = HEAT_ICON__;
int32_t HEATSENSOR          = HEATSENSOR__;
int32_t HEAVYHBOMB          = HEAVYHBOMB__;
int32_t HELECOPT            = HELECOPT__;
int32_t HOLODUKE            = HOLODUKE__;
int32_t HOLODUKE_ICON       = HOLODUKE_ICON__;
int32_t HORSEONSIDE         = HORSEONSIDE__;
int32_t HOTMEAT             = HOTMEAT__;
int32_t HURTRAIL            = HURTRAIL__;
int32_t HYDRENT             = HYDRENT__;
int32_t HYDROPLANT          = HYDROPLANT__;
int32_t INDY                = INDY__;
int32_t INGAMEDUKETHREEDEE  = INGAMEDUKETHREEDEE__;
int32_t INNERJAW            = INNERJAW__;
int32_t INVENTORYBOX        = INVENTORYBOX__;
int32_t IVUNIT              = IVUNIT__;
int32_t JETPACK             = JETPACK__;
int32_t JETPACK_ICON        = JETPACK_ICON__;
int32_t JIBS1               = JIBS1__;
int32_t JIBS2               = JIBS2__;
int32_t JIBS3               = JIBS3__;
int32_t JIBS4               = JIBS4__;
int32_t JIBS5               = JIBS5__;
int32_t JIBS6               = JIBS6__;
int32_t JURYGUY             = JURYGUY__;
int32_t KILLSICON           = KILLSICON__;
int32_t KNEE                = KNEE__;
int32_t LA                  = LA__;
int32_t LASERLINE           = LASERLINE__;
int32_t LASERSITE           = LASERSITE__;
int32_t LEGJIB1             = LEGJIB1__;
int32_t LETTER              = LETTER__;
int32_t LIGHTSWITCH         = LIGHTSWITCH__;
int32_t LIGHTSWITCH2        = LIGHTSWITCH2__;
int32_t LIZMAN              = LIZMAN__;
int32_t LIZMANARM1          = LIZMANARM1__;
int32_t LIZMANFEEDING       = LIZMANFEEDING__;
int32_t LIZMANHEAD1         = LIZMANHEAD1__;
int32_t LIZMANJUMP          = LIZMANJUMP__;
int32_t LIZMANLEG1          = LIZMANLEG1__;
int32_t LIZMANSPITTING      = LIZMANSPITTING__;
int32_t LIZMANSTAYPUT       = LIZMANSTAYPUT__;
int32_t LIZTROOP            = LIZTROOP__;
int32_t LIZTROOPDUCKING     = LIZTROOPDUCKING__;
int32_t LIZTROOPJETPACK     = LIZTROOPJETPACK__;
int32_t LIZTROOPJUSTSIT     = LIZTROOPJUSTSIT__;
int32_t LIZTROOPONTOILET    = LIZTROOPONTOILET__;
int32_t LIZTROOPRUNNING     = LIZTROOPRUNNING__;
int32_t LIZTROOPSHOOT       = LIZTROOPSHOOT__;
int32_t LIZTROOPSTAYPUT     = LIZTROOPSTAYPUT__;
int32_t LOADSCREEN          = LOADSCREEN__;
int32_t LOCATORS            = LOCATORS__;
int32_t LOCKSWITCH1         = LOCKSWITCH1__;
int32_t LOOGIE              = LOOGIE__;
int32_t LUKE                = LUKE__;
int32_t MAIL                = MAIL__;
int32_t MAN                 = MAN__;
int32_t MAN2                = MAN2__;
int32_t MASKWALL1           = MASKWALL1__;
int32_t MASKWALL10          = MASKWALL10__;
int32_t MASKWALL11          = MASKWALL11__;
int32_t MASKWALL12          = MASKWALL12__;
int32_t MASKWALL13          = MASKWALL13__;
int32_t MASKWALL14          = MASKWALL14__;
int32_t MASKWALL15          = MASKWALL15__;
int32_t MASKWALL2           = MASKWALL2__;
int32_t MASKWALL3           = MASKWALL3__;
int32_t MASKWALL4           = MASKWALL4__;
int32_t MASKWALL5           = MASKWALL5__;
int32_t MASKWALL6           = MASKWALL6__;
int32_t MASKWALL7           = MASKWALL7__;
int32_t MASKWALL8           = MASKWALL8__;
int32_t MASKWALL9           = MASKWALL9__;
int32_t MASTERSWITCH        = MASTERSWITCH__;
int32_t MENUBAR             = MENUBAR__;
int32_t MENUSCREEN          = MENUSCREEN__;
int32_t MIKE                = MIKE__;
int32_t MINIFONT            = MINIFONT__;
int32_t MIRROR              = MIRROR__;
int32_t MIRRORBROKE         = MIRRORBROKE__;
int32_t MONEY               = MONEY__;
int32_t MONK                = MONK__;
int32_t MOONSKY1            = MOONSKY1__;
int32_t MORTER              = MORTER__;
int32_t MOVIECAMERA         = MOVIECAMERA__;
int32_t MULTISWITCH         = MULTISWITCH__;
int32_t MUSICANDSFX         = MUSICANDSFX__;
int32_t NAKED1              = NAKED1__;
int32_t NATURALLIGHTNING    = NATURALLIGHTNING__;
int32_t NEON1               = NEON1__;
int32_t NEON2               = NEON2__;
int32_t NEON3               = NEON3__;
int32_t NEON4               = NEON4__;
int32_t NEON5               = NEON5__;
int32_t NEON6               = NEON6__;
int32_t NEWBEAST            = NEWBEAST__;
int32_t NEWBEASTSTAYPUT     = NEWBEASTSTAYPUT__;
int32_t NUKEBARREL          = NUKEBARREL__;
int32_t NUKEBARRELDENTED    = NUKEBARRELDENTED__;
int32_t NUKEBARRELLEAKED    = NUKEBARRELLEAKED__;
int32_t NUKEBUTTON          = NUKEBUTTON__;
int32_t OCEANSPRITE1        = OCEANSPRITE1__;
int32_t OCEANSPRITE2        = OCEANSPRITE2__;
int32_t OCEANSPRITE3        = OCEANSPRITE3__;
int32_t OCEANSPRITE4        = OCEANSPRITE4__;
int32_t OCEANSPRITE5        = OCEANSPRITE5__;
int32_t OCTABRAIN           = OCTABRAIN__;
int32_t OCTABRAINSTAYPUT    = OCTABRAINSTAYPUT__;
int32_t OJ                  = OJ__;
int32_t OOZ                 = OOZ__;
int32_t OOZ2                = OOZ2__;
int32_t OOZFILTER           = OOZFILTER__;
int32_t ORDERING            = ORDERING__;
int32_t ORGANTIC            = ORGANTIC__;
int32_t PANNEL1             = PANNEL1__;
int32_t PANNEL2             = PANNEL2__;
int32_t PANNEL3             = PANNEL3__;
int32_t PAPER               = PAPER__;
int32_t PIGCOP              = PIGCOP__;
int32_t PIGCOPDIVE          = PIGCOPDIVE__;
int32_t PIGCOPSTAYPUT       = PIGCOPSTAYPUT__;
int32_t PIPE1               = PIPE1__;
int32_t PIPE1B              = PIPE1B__;
int32_t PIPE2               = PIPE2__;
int32_t PIPE2B              = PIPE2B__;
int32_t PIPE3               = PIPE3__;
int32_t PIPE3B              = PIPE3B__;
int32_t PIPE4               = PIPE4__;
int32_t PIPE4B              = PIPE4B__;
int32_t PIPE5               = PIPE5__;
int32_t PIPE5B              = PIPE5B__;
int32_t PIPE6               = PIPE6__;
int32_t PIPE6B              = PIPE6B__;
int32_t PLAYERONWATER       = PLAYERONWATER__;
int32_t PLUG                = PLUG__;
int32_t PLUTOPAKSPRITE      = PLUTOPAKSPRITE__;
int32_t POCKET              = POCKET__;
int32_t PODFEM1             = PODFEM1__;
int32_t POT1                = POT1__;
int32_t POT2                = POT2__;
int32_t POT3                = POT3__;
int32_t POWERSWITCH1        = POWERSWITCH1__;
int32_t POWERSWITCH2        = POWERSWITCH2__;
int32_t PUKE                = PUKE__;
int32_t PULLSWITCH          = PULLSWITCH__;
int32_t PURPLELAVA          = PURPLELAVA__;
int32_t QUEBALL             = QUEBALL__;
int32_t RADIUSEXPLOSION     = RADIUSEXPLOSION__;
int32_t RAT                 = RAT__;
int32_t REACTOR             = REACTOR__;
int32_t REACTOR2            = REACTOR2__;
int32_t REACTOR2BURNT       = REACTOR2BURNT__;
int32_t REACTOR2SPARK       = REACTOR2SPARK__;
int32_t REACTORBURNT        = REACTORBURNT__;
int32_t REACTORSPARK        = REACTORSPARK__;
int32_t RECON               = RECON__;
int32_t RESPAWN             = RESPAWN__;
int32_t RESPAWNMARKERGREEN  = RESPAWNMARKERGREEN__;
int32_t RESPAWNMARKERRED    = RESPAWNMARKERRED__;
int32_t RESPAWNMARKERYELLOW = RESPAWNMARKERYELLOW__;
int32_t ROTATEGUN           = ROTATEGUN__;
int32_t RPG                 = RPG__;
int32_t RPGAMMO             = RPGAMMO__;
int32_t RPGGUN              = RPGGUN__;
int32_t RPGSPRITE           = RPGSPRITE__;
int32_t RUBBERCAN           = RUBBERCAN__;
int32_t SATELITE            = SATELITE__;
int32_t SCALE               = SCALE__;
int32_t SCRAP1              = SCRAP1__;
int32_t SCRAP2              = SCRAP2__;
int32_t SCRAP3              = SCRAP3__;
int32_t SCRAP4              = SCRAP4__;
int32_t SCRAP5              = SCRAP5__;
int32_t SCRAP6              = SCRAP6__;
int32_t SCREENBREAK1        = SCREENBREAK1__;
int32_t SCREENBREAK10       = SCREENBREAK10__;
int32_t SCREENBREAK11       = SCREENBREAK11__;
int32_t SCREENBREAK12       = SCREENBREAK12__;
int32_t SCREENBREAK13       = SCREENBREAK13__;
int32_t SCREENBREAK14       = SCREENBREAK14__;
int32_t SCREENBREAK15       = SCREENBREAK15__;
int32_t SCREENBREAK16       = SCREENBREAK16__;
int32_t SCREENBREAK17       = SCREENBREAK17__;
int32_t SCREENBREAK18       = SCREENBREAK18__;
int32_t SCREENBREAK19       = SCREENBREAK19__;
int32_t SCREENBREAK2        = SCREENBREAK2__;
int32_t SCREENBREAK3        = SCREENBREAK3__;
int32_t SCREENBREAK4        = SCREENBREAK4__;
int32_t SCREENBREAK5        = SCREENBREAK5__;
int32_t SCREENBREAK6        = SCREENBREAK6__;
int32_t SCREENBREAK7        = SCREENBREAK7__;
int32_t SCREENBREAK8        = SCREENBREAK8__;
int32_t SCREENBREAK9        = SCREENBREAK9__;
int32_t SCUBAMASK           = SCUBAMASK__;
int32_t SECTOREFFECTOR      = SECTOREFFECTOR__;
int32_t SEENINE             = SEENINE__;
int32_t SEENINEDEAD         = SEENINEDEAD__;
int32_t SELECTDIR           = SELECTDIR__;
int32_t SHARK               = SHARK__;
int32_t SHELL               = SHELL__;
int32_t SHIELD              = SHIELD__;
int32_t SHOTGUN             = SHOTGUN__;
int32_t SHOTGUNAMMO         = SHOTGUNAMMO__;
int32_t SHOTGUNSHELL        = SHOTGUNSHELL__;
int32_t SHOTGUNSPRITE       = SHOTGUNSPRITE__;
int32_t SHOTSPARK1          = SHOTSPARK1__;
int32_t SHRINKER            = SHRINKER__;
int32_t SHRINKEREXPLOSION   = SHRINKEREXPLOSION__;
int32_t SHRINKERSPRITE      = SHRINKERSPRITE__;
int32_t SHRINKSPARK         = SHRINKSPARK__;
int32_t SIDEBOLT1           = SIDEBOLT1__;
int32_t SIGN1               = SIGN1__;
int32_t SIGN2               = SIGN2__;
int32_t SIXPAK              = SIXPAK__;
int32_t SLIDEBAR            = SLIDEBAR__;
int32_t SLOTDOOR            = SLOTDOOR__;
int32_t SMALLFNTCURSOR      = SMALLFNTCURSOR__;
int32_t SMALLSMOKE          = SMALLSMOKE__;
int32_t SOLARPANNEL         = SOLARPANNEL__;
int32_t SPACEDOORSWITCH     = SPACEDOORSWITCH__;
int32_t SPACELIGHTSWITCH    = SPACELIGHTSWITCH__;
int32_t SPACEMARINE         = SPACEMARINE__;
int32_t SPEAKER             = SPEAKER__;
int32_t SPINNINGNUKEICON    = SPINNINGNUKEICON__;
int32_t SPIT                = SPIT__;
int32_t SPOTLITE            = SPOTLITE__;
int32_t STAINGLASS1         = STAINGLASS1__;
int32_t STALL               = STALL__;
int32_t STALLBROKE          = STALLBROKE__;
int32_t STARTALPHANUM       = STARTALPHANUM__;
int32_t STATIC              = STATIC__;
int32_t STATUE              = STATUE__;
int32_t STATUEFLASH         = STATUEFLASH__;
int32_t STEAM               = STEAM__;
int32_t STEROIDS            = STEROIDS__;
int32_t STEROIDS_ICON       = STEROIDS_ICON__;
int32_t STRIPEBALL          = STRIPEBALL__;
int32_t SUSHIPLATE1         = SUSHIPLATE1__;
int32_t SUSHIPLATE2         = SUSHIPLATE2__;
int32_t SUSHIPLATE3         = SUSHIPLATE3__;
int32_t SUSHIPLATE4         = SUSHIPLATE4__;
int32_t SUSHIPLATE5         = SUSHIPLATE5__;
int32_t TAMPON              = TAMPON__;
int32_t TANK                = TANK__;
int32_t TARGET              = TARGET__;
int32_t TECHLIGHT2          = TECHLIGHT2__;
int32_t TECHLIGHT4          = TECHLIGHT4__;
int32_t TECHLIGHTBUST2      = TECHLIGHTBUST2__;
int32_t TECHLIGHTBUST4      = TECHLIGHTBUST4__;
int32_t TECHSWITCH          = TECHSWITCH__;
int32_t TENSCREEN           = TENSCREEN__;
int32_t TEXTBOX             = TEXTBOX__;
int32_t TEXTSTORY           = TEXTSTORY__;
int32_t THREEBYFIVE         = THREEBYFIVE__;
int32_t THREEDEE            = THREEDEE__;
int32_t TIP                 = TIP__;
int32_t TIRE                = TIRE__;
int32_t TOILET              = TOILET__;
int32_t TOILETBROKE         = TOILETBROKE__;
int32_t TOILETWATER         = TOILETWATER__;
int32_t TONGUE              = TONGUE__;
int32_t TOUCHPLATE          = TOUCHPLATE__;
int32_t TOUGHGAL            = TOUGHGAL__;
int32_t TRANSPORTERBEAM     = TRANSPORTERBEAM__;
int32_t TRANSPORTERSTAR     = TRANSPORTERSTAR__;
int32_t TRASH               = TRASH__;
int32_t TREE1               = TREE1__;
int32_t TREE2               = TREE2__;
int32_t TRIPBOMB            = TRIPBOMB__;
int32_t TRIPBOMBSPRITE      = TRIPBOMBSPRITE__;
int32_t TRIPODCAMERA        = TRIPODCAMERA__;
int32_t VACUUM              = VACUUM__;
int32_t VASE                = VASE__;
int32_t VENDMACHINE         = VENDMACHINE__;
int32_t VICTORY1            = VICTORY1__;
int32_t VIEWBORDER          = VIEWBORDER__;
int32_t VIEWSCREEN          = VIEWSCREEN__;
int32_t VIEWSCREEN2         = VIEWSCREEN2__;
int32_t W_FORCEFIELD        = W_FORCEFIELD__;
int32_t W_HITTECHWALL1      = W_HITTECHWALL1__;
int32_t W_HITTECHWALL10     = W_HITTECHWALL10__;
int32_t W_HITTECHWALL15     = W_HITTECHWALL15__;
int32_t W_HITTECHWALL16     = W_HITTECHWALL16__;
int32_t W_HITTECHWALL2      = W_HITTECHWALL2__;
int32_t W_HITTECHWALL3      = W_HITTECHWALL3__;
int32_t W_HITTECHWALL4      = W_HITTECHWALL4__;
int32_t W_MILKSHELF         = W_MILKSHELF__;
int32_t W_MILKSHELFBROKE    = W_MILKSHELFBROKE__;
int32_t W_NUMBERS           = W_NUMBERS__;
int32_t W_SCREENBREAK       = W_SCREENBREAK__;
int32_t W_TECHWALL1         = W_TECHWALL1__;
int32_t W_TECHWALL10        = W_TECHWALL10__;
int32_t W_TECHWALL11        = W_TECHWALL11__;
int32_t W_TECHWALL12        = W_TECHWALL12__;
int32_t W_TECHWALL13        = W_TECHWALL13__;
int32_t W_TECHWALL14        = W_TECHWALL14__;
int32_t W_TECHWALL15        = W_TECHWALL15__;
int32_t W_TECHWALL16        = W_TECHWALL16__;
int32_t W_TECHWALL2         = W_TECHWALL2__;
int32_t W_TECHWALL3         = W_TECHWALL3__;
int32_t W_TECHWALL4         = W_TECHWALL4__;
int32_t W_TECHWALL5         = W_TECHWALL5__;
int32_t W_TECHWALL6         = W_TECHWALL6__;
int32_t W_TECHWALL7         = W_TECHWALL7__;
int32_t W_TECHWALL8         = W_TECHWALL8__;
int32_t W_TECHWALL9         = W_TECHWALL9__;
int32_t WAITTOBESEATED      = WAITTOBESEATED__;
int32_t WALLBLOOD1          = WALLBLOOD1__;
int32_t WALLBLOOD2          = WALLBLOOD2__;
int32_t WALLBLOOD3          = WALLBLOOD3__;
int32_t WALLBLOOD4          = WALLBLOOD4__;
int32_t WALLBLOOD5          = WALLBLOOD5__;
int32_t WALLBLOOD7          = WALLBLOOD7__;
int32_t WALLBLOOD8          = WALLBLOOD8__;
int32_t WALLLIGHT1          = WALLLIGHT1__;
int32_t WALLLIGHT2          = WALLLIGHT2__;
int32_t WALLLIGHT3          = WALLLIGHT3__;
int32_t WALLLIGHT4          = WALLLIGHT4__;
int32_t WALLLIGHTBUST1      = WALLLIGHTBUST1__;
int32_t WALLLIGHTBUST2      = WALLLIGHTBUST2__;
int32_t WALLLIGHTBUST3      = WALLLIGHTBUST3__;
int32_t WALLLIGHTBUST4      = WALLLIGHTBUST4__;
int32_t WATERBUBBLE         = WATERBUBBLE__;
int32_t WATERBUBBLEMAKER    = WATERBUBBLEMAKER__;
int32_t WATERDRIP           = WATERDRIP__;
int32_t WATERDRIPSPLASH     = WATERDRIPSPLASH__;
int32_t WATERFOUNTAIN       = WATERFOUNTAIN__;
int32_t WATERFOUNTAINBROKE  = WATERFOUNTAINBROKE__;
int32_t WATERSPLASH2        = WATERSPLASH2__;
int32_t WATERTILE2          = WATERTILE2__;
int32_t WEATHERWARN         = WEATHERWARN__;
int32_t WINDOWBORDER1       = WINDOWBORDER1__;
int32_t WINDOWBORDER2       = WINDOWBORDER2__;
int32_t WOMAN               = WOMAN__;
int32_t WOODENHORSE         = WOODENHORSE__;
int32_t XXXSTACY            = XXXSTACY__;
int32_t WIDESCREENSTATUSBAR = WIDESCREENSTATUSBAR__;
int32_t RPGGUNWIDE          = RPGGUNWIDE__;
int32_t FIRSTGUNRELOADWIDE  = FIRSTGUNRELOADWIDE__;
int32_t FREEZEWIDE          = FREEZEWIDE__;
int32_t FREEZEFIREWIDE      = FREEZEFIREWIDE__;
int32_t SHRINKERWIDE        = SHRINKERWIDE__;
int32_t CRACKKNUCKLESWIDE   = CRACKKNUCKLESWIDE__;
int32_t FLAMETHROWERSPRITE  = FLAMETHROWERSPRITE__;
int32_t FLAMETHROWERAMMO    = FLAMETHROWERAMMO__;
int32_t FLAMETHROWER        = FLAMETHROWER__;
int32_t FLAMETHROWERFIRE    = FLAMETHROWERFIRE__;
int32_t FLAMETHROWERFLAME   = FLAMETHROWERFLAME__;
int32_t FLAMETHROWERPILOT   = FLAMETHROWERPILOT__;
int32_t FIREBALL            = FIREBALL__;
int32_t ONFIRE              = ONFIRE__;
int32_t ONFIRESMOKE         = ONFIRESMOKE__;
int32_t BURNEDCORPSE        = BURNEDCORPSE__;
int32_t WHISPYSMOKE         = WHISPYSMOKE__;
int32_t FIREFLY             = FIREFLY__;
int32_t FIREFLYSHRINKEFFECT = FIREFLYSHRINKEFFECT__;
int32_t FIREFLYGROWEFFECT   = FIREFLYGROWEFFECT__;
int32_t FIREFLYFLYINGEFFECT = FIREFLYFLYINGEFFECT__;
int32_t BOSS5               = BOSS5__;
int32_t BOSS5STAYPUT        = BOSS5STAYPUT__;
int32_t LAVAPOOL            = LAVAPOOL__;
int32_t LAVASPLASH          = LAVASPLASH__;
int32_t LAVAPOOLBUBBLE      = LAVAPOOLBUBBLE__;
int32_t BOSS2STAYPUT        = BOSS2STAYPUT__;
int32_t BOSS3STAYPUT        = BOSS3STAYPUT__;
int32_t E32_TILE5736        = E32_TILE5736__;
int32_t E32_TILE5737        = E32_TILE5737__;
int32_t E32_TILE5846        = E32_TILE5846__;

static hashtable_t h_names = {512, NULL};

void G_ProcessDynamicTileMapping(const char *szLabel, int32_t lValue)
{
    if ((unsigned)lValue >= MAXTILES || !szLabel)
        return;

    int const i = hash_find(&h_names,szLabel);

    if (i>=0)
    {
        struct dynitem *di = &g_dynTileList[i];
#ifdef DEBUGGINGAIDS
        if (g_scriptDebug && di->staticval != lValue)
            OSD_Printf("REMAP %s (%d) --> %d\n", di->str, di->staticval, lValue);
#endif
        *di->dynvalptr = lValue;
    }
}

void inithashnames(void)
{
    hash_init(&h_names);

    for (int i=0; i < ARRAY_SSIZE(g_dynTileList); i++)
        hash_add(&h_names, g_dynTileList[i].str, i, 0);
}

void freehashnames(void)
{
    hash_free(&h_names);
}
#endif

// This is run after all CON define's have been processed to set up the
// dynamic->static tile mapping.
void G_InitDynamicTiles(void)
{
    Bmemset(DynamicTileMap, 0, sizeof(DynamicTileMap));

    for (auto & i : g_dynTileList)
#ifdef DYNTILEREMAP_ENABLE
        DynamicTileMap[*(i.dynvalptr)] = i.staticval;
#else
        DynamicTileMap[i.staticval] = i.staticval;
#endif

    g_blimpSpawnItems[0] = RPGSPRITE;
    g_blimpSpawnItems[1] = CHAINGUNSPRITE;
    g_blimpSpawnItems[2] = DEVISTATORAMMO;
    g_blimpSpawnItems[3] = RPGAMMO;
    g_blimpSpawnItems[4] = RPGAMMO;
    g_blimpSpawnItems[5] = JETPACK;
    g_blimpSpawnItems[6] = SHIELD;
    g_blimpSpawnItems[7] = FIRSTAID;
    g_blimpSpawnItems[8] = STEROIDS;
    g_blimpSpawnItems[9] = RPGAMMO;
    g_blimpSpawnItems[10] = RPGAMMO;
    g_blimpSpawnItems[11] = RPGSPRITE;
    g_blimpSpawnItems[12] = RPGAMMO;
    g_blimpSpawnItems[13] = FREEZESPRITE;
    g_blimpSpawnItems[14] = FREEZEAMMO;

    WeaponPickupSprites[0] = KNEE;
    WeaponPickupSprites[1] = FIRSTGUNSPRITE;
    WeaponPickupSprites[2] = SHOTGUNSPRITE;
    WeaponPickupSprites[3] = CHAINGUNSPRITE;
    WeaponPickupSprites[4] = RPGSPRITE;
    WeaponPickupSprites[5] = HEAVYHBOMB;
    WeaponPickupSprites[6] = SHRINKERSPRITE;
    WeaponPickupSprites[7] = DEVISTATORSPRITE;
    WeaponPickupSprites[8] = TRIPBOMBSPRITE;
    WeaponPickupSprites[9] = FREEZESPRITE;
    WeaponPickupSprites[10] = HEAVYHBOMB;
    WeaponPickupSprites[11] = SHRINKERSPRITE;
    WeaponPickupSprites[12] = FLAMETHROWERSPRITE;

    // ouch... the big background image takes up a fuckload of memory and takes a second to load!
#ifdef EDUKE32_GLES
    MENUSCREEN = LOADSCREEN = BETASCREEN;
#endif
}
