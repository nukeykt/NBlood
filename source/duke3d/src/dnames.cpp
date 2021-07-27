//-------------------------------------------------------------------------
/*
Copyright (C) EDuke32 developers and contributors

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

#include "dnames.h"
#include "global.h"

#ifdef USE_DNAMES

struct dynitem g_dynSoundList[] =
{
    { "ALIEN_SWITCH1",       &ALIEN_SWITCH1,      ALIEN_SWITCH1__ },
    { "BIGBANG",             &BIGBANG,            BIGBANG__ },
    { "BONUS_SPEECH1",       &BONUS_SPEECH1,      BONUS_SPEECH1__ },
    { "BONUS_SPEECH2",       &BONUS_SPEECH2,      BONUS_SPEECH2__ },
    { "BONUS_SPEECH3",       &BONUS_SPEECH3,      BONUS_SPEECH3__ },
    { "BONUS_SPEECH4",       &BONUS_SPEECH4,      BONUS_SPEECH4__ },
    { "BONUSMUSIC",          &BONUSMUSIC,         BONUSMUSIC__ },
    { "BOS1_RECOG",          &BOS1_RECOG,         BOS1_RECOG__ },
    { "BOS1_WALK",           &BOS1_WALK,          BOS1_WALK__ },
    { "BOS2_RECOG",          &BOS2_RECOG,         BOS2_RECOG__ },
    { "BOS3_RECOG",          &BOS3_RECOG,         BOS3_RECOG__ },
    { "BOS4_RECOG",          &BOS4_RECOG,         BOS4_RECOG__ },
    { "BOSS4_DEADSPEECH",    &BOSS4_DEADSPEECH,   BOSS4_DEADSPEECH__ },
    { "BOSS4_FIRSTSEE",      &BOSS4_FIRSTSEE,     BOSS4_FIRSTSEE__ },
    { "BOSSTALKTODUKE",      &BOSSTALKTODUKE,     BOSSTALKTODUKE__ },
    { "CAPT_RECOG",          &CAPT_RECOG,         CAPT_RECOG__ },
    { "CAT_FIRE",            &CAT_FIRE,           CAT_FIRE__ },
    { "CHAINGUN_FIRE",       &CHAINGUN_FIRE,      CHAINGUN_FIRE__ },
    { "COMM_RECOG",          &COMM_RECOG,         COMM_RECOG__ },
    { "DRON_RECOG",          &DRON_RECOG,         DRON_RECOG__ },
    { "DUKE_CRACK",          &DUKE_CRACK,          DUKE_CRACK__ },
    { "DUKE_CRACK_FIRST",    &DUKE_CRACK_FIRST,    DUKE_CRACK_FIRST__ },
    { "DUKE_CRACK2",         &DUKE_CRACK2,         DUKE_CRACK2__ },
    { "DUKE_DRINKING",       &DUKE_DRINKING,       DUKE_DRINKING__ },
    { "DUKE_GASP",           &DUKE_GASP,           DUKE_GASP__ },
    { "DUKE_GET",            &DUKE_GET,            DUKE_GET__ },
    { "DUKE_GETWEAPON2",     &DUKE_GETWEAPON2,     DUKE_GETWEAPON2__ },
    { "DUKE_GETWEAPON6",     &DUKE_GETWEAPON6,     DUKE_GETWEAPON6__ },
    { "DUKE_GOTHEALTHATLOW", &DUKE_GOTHEALTHATLOW, DUKE_GOTHEALTHATLOW__ },
    { "DUKE_GRUNT",          &DUKE_GRUNT,          DUKE_GRUNT__ },
    { "DUKE_HARTBEAT",       &DUKE_HARTBEAT,       DUKE_HARTBEAT__ },
    { "DUKE_JETPACK_IDLE",   &DUKE_JETPACK_IDLE,   DUKE_JETPACK_IDLE__ },
    { "DUKE_JETPACK_OFF",    &DUKE_JETPACK_OFF,    DUKE_JETPACK_OFF__ },
    { "DUKE_JETPACK_ON",     &DUKE_JETPACK_ON,     DUKE_JETPACK_ON__ },
    { "DUKE_LAND",           &DUKE_LAND,           DUKE_LAND__ },
    { "DUKE_LAND_HURT",      &DUKE_LAND_HURT,      DUKE_LAND_HURT__ },
    { "DUKE_LONGTERM_PAIN",  &DUKE_LONGTERM_PAIN,  DUKE_LONGTERM_PAIN__ },
    { "DUKE_ONWATER",        &DUKE_ONWATER,        DUKE_ONWATER__ },
    { "DUKE_PISSRELIEF",     &DUKE_PISSRELIEF,     DUKE_PISSRELIEF__ },
    { "DUKE_SCREAM",         &DUKE_SCREAM,         DUKE_SCREAM__ },
    { "DUKE_SEARCH",         &DUKE_SEARCH,         DUKE_SEARCH__ },
    { "DUKE_SEARCH2",        &DUKE_SEARCH2,        DUKE_SEARCH2__ },
    { "DUKE_TAKEPILLS",      &DUKE_TAKEPILLS,      DUKE_TAKEPILLS__ },
    { "DUKE_UNDERWATER",     &DUKE_UNDERWATER,     DUKE_UNDERWATER__ },
    { "DUKE_URINATE",        &DUKE_URINATE,        DUKE_URINATE__ },
    { "DUKE_USEMEDKIT",      &DUKE_USEMEDKIT,      DUKE_USEMEDKIT__ },
    { "DUKE_WALKINDUCTS",    &DUKE_WALKINDUCTS,    DUKE_WALKINDUCTS__ },
    { "DUKETALKTOBOSS",      &DUKETALKTOBOSS,      DUKETALKTOBOSS__ },
    { "EARTHQUAKE",          &EARTHQUAKE,         EARTHQUAKE__ },
    { "EJECT_CLIP",          &EJECT_CLIP,         EJECT_CLIP__ },
    { "ELEVATOR_OFF",        &ELEVATOR_OFF,       ELEVATOR_OFF__ },
    { "ELEVATOR_ON",         &ELEVATOR_ON,        ELEVATOR_ON__ },
    { "END_OF_LEVEL_WARN",   &END_OF_LEVEL_WARN,  END_OF_LEVEL_WARN__ },
    { "ENDSEQVOL2SND1",      &ENDSEQVOL2SND1,     ENDSEQVOL2SND1__ },
    { "ENDSEQVOL2SND2",      &ENDSEQVOL2SND2,     ENDSEQVOL2SND2__ },
    { "ENDSEQVOL2SND3",      &ENDSEQVOL2SND3,     ENDSEQVOL2SND3__ },
    { "ENDSEQVOL2SND4",      &ENDSEQVOL2SND4,     ENDSEQVOL2SND4__ },
    { "ENDSEQVOL2SND5",      &ENDSEQVOL2SND5,     ENDSEQVOL2SND5__ },
    { "ENDSEQVOL2SND6",      &ENDSEQVOL2SND6,     ENDSEQVOL2SND6__ },
    { "ENDSEQVOL2SND7",      &ENDSEQVOL2SND7,     ENDSEQVOL2SND7__ },
    { "ENDSEQVOL3SND2",      &ENDSEQVOL3SND2,     ENDSEQVOL3SND2__ },
    { "ENDSEQVOL3SND3",      &ENDSEQVOL3SND3,     ENDSEQVOL3SND3__ },
    { "ENDSEQVOL3SND4",      &ENDSEQVOL3SND4,     ENDSEQVOL3SND4__ },
    { "ENDSEQVOL3SND5",      &ENDSEQVOL3SND5,     ENDSEQVOL3SND5__ },
    { "ENDSEQVOL3SND6",      &ENDSEQVOL3SND6,     ENDSEQVOL3SND6__ },
    { "ENDSEQVOL3SND7",      &ENDSEQVOL3SND7,     ENDSEQVOL3SND7__ },
    { "ENDSEQVOL3SND8",      &ENDSEQVOL3SND8,     ENDSEQVOL3SND8__ },
    { "ENDSEQVOL3SND9",      &ENDSEQVOL3SND9,     ENDSEQVOL3SND9__ },
    { "EXITMENUSOUND",       &EXITMENUSOUND,      EXITMENUSOUND__ },
    { "EXPANDERSHOOT",       &EXPANDERSHOOT,      EXPANDERSHOOT__ },
    { "FLUSH_TOILET",        &FLUSH_TOILET,       FLUSH_TOILET__ },
    { "FLY_BY",              &FLY_BY,             FLY_BY__ },
    { "GENERIC_AMBIENCE17",  &GENERIC_AMBIENCE17, GENERIC_AMBIENCE17__ },
    { "GLASS_BREAKING",      &GLASS_BREAKING,     GLASS_BREAKING__ },
    { "GLASS_HEAVYBREAK",    &GLASS_HEAVYBREAK,   GLASS_HEAVYBREAK__ },
    { "INSERT_CLIP",         &INSERT_CLIP,        INSERT_CLIP__ },
    { "INTRO4_1",            &INTRO4_1,           INTRO4_1__ },
    { "INTRO4_2",            &INTRO4_2,           INTRO4_2__ },
    { "INTRO4_3",            &INTRO4_3,           INTRO4_3__ },
    { "INTRO4_4",            &INTRO4_4,           INTRO4_4__ },
    { "INTRO4_5",            &INTRO4_5,           INTRO4_5__ },
    { "INTRO4_6",            &INTRO4_6,           INTRO4_6__ },
    { "INTRO4_B",            &INTRO4_B,           INTRO4_B__ },
    { "ITEM_SPLASH",         &ITEM_SPLASH,        ITEM_SPLASH__ },
    { "JIBBED_ACTOR5",       &JIBBED_ACTOR5,      JIBBED_ACTOR5__ },
    { "JIBBED_ACTOR6",       &JIBBED_ACTOR6,      JIBBED_ACTOR6__ },
    { "KICK_HIT",            &KICK_HIT,           KICK_HIT__ },
    { "LASERTRIP_ARMING",    &LASERTRIP_ARMING,   LASERTRIP_ARMING__ },
    { "LASERTRIP_EXPLODE",   &LASERTRIP_EXPLODE,  LASERTRIP_EXPLODE__ },
    { "LASERTRIP_ONWALL",    &LASERTRIP_ONWALL,   LASERTRIP_ONWALL__ },
    { "LIGHTNING_SLAP",      &LIGHTNING_SLAP,     LIGHTNING_SLAP__ },
    { "MONITOR_ACTIVE",      &MONITOR_ACTIVE,     MONITOR_ACTIVE__ },
    { "NITEVISION_ONOFF",    &NITEVISION_ONOFF,   NITEVISION_ONOFF__ },
    { "OCTA_RECOG",          &OCTA_RECOG,         OCTA_RECOG__ },
    { "PIG_RECOG",           &PIG_RECOG,          PIG_RECOG__ },
    { "PIPEBOMB_BOUNCE",     &PIPEBOMB_BOUNCE,    PIPEBOMB_BOUNCE__ },
    { "PIPEBOMB_EXPLODE",    &PIPEBOMB_EXPLODE,   PIPEBOMB_EXPLODE__ },
    { "PISTOL_BODYHIT",      &PISTOL_BODYHIT,     PISTOL_BODYHIT__ },
    { "PISTOL_FIRE",         &PISTOL_FIRE,        PISTOL_FIRE__ },
    { "PISTOL_RICOCHET",     &PISTOL_RICOCHET,    PISTOL_RICOCHET__ },
    { "POOLBALLHIT",         &POOLBALLHIT,        POOLBALLHIT__ },
    { "PRED_RECOG",          &PRED_RECOG,         PRED_RECOG__ },
    { "RATTY",               &RATTY,              RATTY__ },
    { "RECO_ATTACK",         &RECO_ATTACK,        RECO_ATTACK__ },
    { "RECO_PAIN",           &RECO_PAIN,          RECO_PAIN__ },
    { "RECO_RECOG",          &RECO_RECOG,         RECO_RECOG__ },
    { "RECO_ROAM",           &RECO_ROAM,          RECO_ROAM__ },
    { "RIPHEADNECK",         &RIPHEADNECK,        RIPHEADNECK__ },
    { "RPG_EXPLODE",         &RPG_EXPLODE,        RPG_EXPLODE__ },
    { "RPG_SHOOT",           &RPG_SHOOT,          RPG_SHOOT__ },
    { "SELECT_WEAPON",       &SELECT_WEAPON,      SELECT_WEAPON__ },
    { "SHORT_CIRCUIT",       &SHORT_CIRCUIT,      SHORT_CIRCUIT__ },
    { "SHOTGUN_COCK",        &SHOTGUN_COCK,       SHOTGUN_COCK__ },
    { "SHOTGUN_FIRE",        &SHOTGUN_FIRE,       SHOTGUN_FIRE__ },
    { "SHRINKER_FIRE",       &SHRINKER_FIRE,      SHRINKER_FIRE__ },
    { "SHRINKER_HIT",        &SHRINKER_HIT,       SHRINKER_HIT__ },
    { "SLIM_ATTACK",         &SLIM_ATTACK,        SLIM_ATTACK__ },
    { "SLIM_DYING",          &SLIM_DYING,         SLIM_DYING__ },
    { "SLIM_RECOG",          &SLIM_RECOG,         SLIM_RECOG__ },
    { "SLIM_ROAM",           &SLIM_ROAM,          SLIM_ROAM__ },
    { "SOMETHING_DRIPPING",  &SOMETHING_DRIPPING, SOMETHING_DRIPPING__ },
    { "SOMETHINGFROZE",      &SOMETHINGFROZE,     SOMETHINGFROZE__ },
    { "SOMETHINGHITFORCE",   &SOMETHINGHITFORCE,  SOMETHINGHITFORCE__ },
    { "SQUISHED",            &SQUISHED,           SQUISHED__ },
    { "SUBWAY",              &SUBWAY,             SUBWAY__ },
    { "SWITCH_ON",           &SWITCH_ON,          SWITCH_ON__ },
    { "TELEPORTER",          &TELEPORTER,         TELEPORTER__ },
    { "THUD",                &THUD,               THUD__ },
    { "THUNDER",             &THUNDER,            THUNDER__ },
    { "TURR_RECOG",          &TURR_RECOG,         TURR_RECOG__ },
    { "VENT_BUST",           &VENT_BUST,          VENT_BUST__ },
    { "VOL4ENDSND1",         &VOL4ENDSND1,        VOL4ENDSND1__ },
    { "VOL4ENDSND2",         &VOL4ENDSND2,        VOL4ENDSND2__ },
    { "WAR_AMBIENCE2",       &WAR_AMBIENCE2,      WAR_AMBIENCE2__ },
    { "WHIPYOURASS",         &WHIPYOURASS,        WHIPYOURASS__ },
    { "WIERDSHOT_FLY",       &WIERDSHOT_FLY,      WIERDSHOT_FLY__ },
    { "WIND_AMBIENCE",       &WIND_AMBIENCE,      WIND_AMBIENCE__ },
    { "WIND_REPEAT",         &WIND_REPEAT,        WIND_REPEAT__ },
    { "FLAMETHROWER_INTRO",  &FLAMETHROWER_INTRO, FLAMETHROWER_INTRO__ },
    { "FLAMETHROWER_LOOP",   &FLAMETHROWER_LOOP,  FLAMETHROWER_LOOP__ },
    { "FLAMETHROWER_END",    &FLAMETHROWER_END,   FLAMETHROWER_END__ },
    { "E5L7_DUKE_QUIT_YOU",  &E5L7_DUKE_QUIT_YOU, E5L7_DUKE_QUIT_YOU__ },
 };

struct dynitem g_dynTileList[] =
{
    { "ACCESS_ICON",         &ACCESS_ICON,         ACCESS_ICON__ },
    { "ACCESSCARD",          &ACCESSCARD,          ACCESSCARD__ },
    { "ACCESSSWITCH",        &ACCESSSWITCH,        ACCESSSWITCH__ },
    { "ACCESSSWITCH2",       &ACCESSSWITCH2,       ACCESSSWITCH2__ },
    { "ACTIVATOR",           &ACTIVATOR,           ACTIVATOR__ },
    { "ACTIVATORLOCKED",     &ACTIVATORLOCKED,     ACTIVATORLOCKED__ },
    { "AIRTANK",             &AIRTANK,             AIRTANK__ },
    { "AIRTANK_ICON",        &AIRTANK_ICON,        AIRTANK_ICON__ },
    { "ALIENSWITCH",         &ALIENSWITCH,         ALIENSWITCH__ },
    { "AMMO",                &AMMO,                AMMO__ },
    { "AMMOBOX",             &AMMOBOX,             AMMOBOX__ },
    { "AMMOLOTS",            &AMMOLOTS,            AMMOLOTS__ },
    { "ANTENNA",             &ANTENNA,             ANTENNA__ },
    { "APLAYER",             &APLAYER,             APLAYER__ },
    { "APLAYERTOP",          &APLAYERTOP,          APLAYERTOP__ },
    { "ARMJIB1",             &ARMJIB1,             ARMJIB1__ },
    { "ARROW",               &ARROW,               ARROW__ },
    { "ATM",                 &ATM,                 ATM__ },
    { "ATMBROKE",            &ATMBROKE,            ATMBROKE__ },
    { "ATOMICHEALTH",        &ATOMICHEALTH,        ATOMICHEALTH__ },
    { "BANNER",              &BANNER,              BANNER__ },
    { "BARBROKE",            &BARBROKE,            BARBROKE__ },
    { "BATTERYAMMO",         &BATTERYAMMO,         BATTERYAMMO__ },
    { "BETASCREEN",          &BETASCREEN,          BETASCREEN__ },
    { "BETAVERSION",         &BETAVERSION,         BETAVERSION__ },
    { "BGRATE1",             &BGRATE1,             BGRATE1__ },
    { "BIGALPHANUM",         &BIGALPHANUM,         BIGALPHANUM__ },
    { "BIGAPPOS",            &BIGAPPOS,            BIGAPPOS__ },
    { "BIGCOLIN",            &BIGCOLIN,            BIGCOLIN__ },
    { "BIGCOMMA",            &BIGCOMMA,            BIGCOMMA__ },
    { "BIGFORCE",            &BIGFORCE,            BIGFORCE__ },
    { "BIGHOLE",             &BIGHOLE,             BIGHOLE__ },
    { "BIGORBIT1",           &BIGORBIT1,           BIGORBIT1__ },
    { "BIGPERIOD",           &BIGPERIOD,           BIGPERIOD__ },
    { "BIGQ",                &BIGQ,                BIGQ__ },
    { "BIGSEMI",             &BIGSEMI,             BIGSEMI__ },
    { "BIGX",                &BIGX_,               BIGX__ },
    { "BLANKSCREEN",         &BLANKSCREEN,         BLANKSCREEN__ },
    { "BLIMP",               &BLIMP,               BLIMP__ },
    { "BLOOD",               &BLOOD,               BLOOD__ },
    { "BLOODPOOL",           &BLOODPOOL,           BLOODPOOL__ },
    { "BLOODSPLAT1",         &BLOODSPLAT1,         BLOODSPLAT1__ },
    { "BLOODSPLAT2",         &BLOODSPLAT2,         BLOODSPLAT2__ },
    { "BLOODSPLAT3",         &BLOODSPLAT3,         BLOODSPLAT3__ },
    { "BLOODSPLAT4",         &BLOODSPLAT4,         BLOODSPLAT4__ },
    { "BLOODYPOLE",          &BLOODYPOLE,          BLOODYPOLE__ },
    { "BOLT1",               &BOLT1,               BOLT1__ },
    { "BONUSSCREEN",         &BONUSSCREEN,         BONUSSCREEN__ },
    { "BOOT_ICON",           &BOOT_ICON,           BOOT_ICON__ },
    { "BOOTS",               &BOOTS,               BOOTS__ },
    { "BORNTOBEWILDSCREEN",  &BORNTOBEWILDSCREEN,  BORNTOBEWILDSCREEN__ },
    { "BOSS1",               &BOSS1,               BOSS1__ },
    { "BOSS1LOB",            &BOSS1LOB,            BOSS1LOB__ },
    { "BOSS1SHOOT",          &BOSS1SHOOT,          BOSS1SHOOT__ },
    { "BOSS1STAYPUT",        &BOSS1STAYPUT,        BOSS1STAYPUT__ },
    { "BOSS2",               &BOSS2,               BOSS2__ },
    { "BOSS3",               &BOSS3,               BOSS3__ },
    { "BOSS4",               &BOSS4,               BOSS4__ },
    { "BOSS4STAYPUT",        &BOSS4STAYPUT,        BOSS4STAYPUT__ },
    { "BOSSTOP",             &BOSSTOP,             BOSSTOP__ },
    { "BOTTLE1",             &BOTTLE1,             BOTTLE1__ },
    { "BOTTLE10",            &BOTTLE10,            BOTTLE10__ },
    { "BOTTLE11",            &BOTTLE11,            BOTTLE11__ },
    { "BOTTLE12",            &BOTTLE12,            BOTTLE12__ },
    { "BOTTLE13",            &BOTTLE13,            BOTTLE13__ },
    { "BOTTLE14",            &BOTTLE14,            BOTTLE14__ },
    { "BOTTLE15",            &BOTTLE15,            BOTTLE15__ },
    { "BOTTLE16",            &BOTTLE16,            BOTTLE16__ },
    { "BOTTLE17",            &BOTTLE17,            BOTTLE17__ },
    { "BOTTLE18",            &BOTTLE18,            BOTTLE18__ },
    { "BOTTLE19",            &BOTTLE19,            BOTTLE19__ },
    { "BOTTLE2",             &BOTTLE2,             BOTTLE2__ },
    { "BOTTLE3",             &BOTTLE3,             BOTTLE3__ },
    { "BOTTLE4",             &BOTTLE4,             BOTTLE4__ },
    { "BOTTLE5",             &BOTTLE5,             BOTTLE5__ },
    { "BOTTLE6",             &BOTTLE6,             BOTTLE6__ },
    { "BOTTLE7",             &BOTTLE7,             BOTTLE7__ },
    { "BOTTLE8",             &BOTTLE8,             BOTTLE8__ },
    { "BOTTOMSTATUSBAR",     &BOTTOMSTATUSBAR,     BOTTOMSTATUSBAR__ },
    { "BOUNCEMINE",          &BOUNCEMINE,          BOUNCEMINE__ },
    { "BOX",                 &BOX,                 BOX__ },
    { "BPANNEL1",            &BPANNEL1,            BPANNEL1__ },
    { "BPANNEL3",            &BPANNEL3,            BPANNEL3__ },
    { "BROKEFIREHYDRENT",    &BROKEFIREHYDRENT,    BROKEFIREHYDRENT__ },
    { "BROKEHYDROPLANT",     &BROKEHYDROPLANT,     BROKEHYDROPLANT__ },
    { "BROKENCHAIR",         &BROKENCHAIR,         BROKENCHAIR__ },
    { "BULLETHOLE",          &BULLETHOLE,          BULLETHOLE__ },
    { "BURNING",             &BURNING,             BURNING__ },
    { "BURNING2",            &BURNING2,            BURNING2__ },
    { "CACTUS",              &CACTUS,              CACTUS__ },
    { "CACTUSBROKE",         &CACTUSBROKE,         CACTUSBROKE__ },
    { "CAMCORNER",           &CAMCORNER,           CAMCORNER__ },
    { "CAMERA1",             &CAMERA1,             CAMERA1__ },
    { "CAMERALIGHT",         &CAMERALIGHT,         CAMERALIGHT__ },
    { "CAMERAPOLE",          &CAMERAPOLE,          CAMERAPOLE__ },
    { "CAMLIGHT",            &CAMLIGHT,            CAMLIGHT__ },
    { "CANWITHSOMETHING",    &CANWITHSOMETHING,    CANWITHSOMETHING__ },
    { "CANWITHSOMETHING2",   &CANWITHSOMETHING2,   CANWITHSOMETHING2__ },
    { "CANWITHSOMETHING3",   &CANWITHSOMETHING3,   CANWITHSOMETHING3__ },
    { "CANWITHSOMETHING4",   &CANWITHSOMETHING4,   CANWITHSOMETHING4__ },
    { "CEILINGSTEAM",        &CEILINGSTEAM,        CEILINGSTEAM__ },
    { "CHAINGUN",            &CHAINGUN,            CHAINGUN__ },
    { "CHAINGUNSPRITE",      &CHAINGUNSPRITE,      CHAINGUNSPRITE__ },
    { "CHAIR1",              &CHAIR1,              CHAIR1__ },
    { "CHAIR2",              &CHAIR2,              CHAIR2__ },
    { "CHAIR3",              &CHAIR3,              CHAIR3__ },
    { "CIRCLEPANNEL",        &CIRCLEPANNEL,        CIRCLEPANNEL__ },
    { "CIRCLEPANNELBROKE",   &CIRCLEPANNELBROKE,   CIRCLEPANNELBROKE__ },
    { "CLOUDYOCEAN",         &CLOUDYOCEAN,         CLOUDYOCEAN__ },
    { "CLOUDYSKIES",         &CLOUDYSKIES,         CLOUDYSKIES__ },
    { "COLA",                &COLA,                COLA__ },
    { "COLAMACHINE",         &COLAMACHINE,         COLAMACHINE__ },
    { "COMMANDER",           &COMMANDER,           COMMANDER__ },
    { "COMMANDERSTAYPUT",    &COMMANDERSTAYPUT,    COMMANDERSTAYPUT__ },
    { "CONE",                &CONE,                CONE__ },
    { "COOLEXPLOSION1",      &COOLEXPLOSION1,      COOLEXPLOSION1__ },
    { "CRACK1",              &CRACK1,              CRACK1__ },
    { "CRACK2",              &CRACK2,              CRACK2__ },
    { "CRACK3",              &CRACK3,              CRACK3__ },
    { "CRACK4",              &CRACK4,              CRACK4__ },
    { "CRACKKNUCKLES",       &CRACKKNUCKLES,       CRACKKNUCKLES__ },
    { "CRANE",               &CRANE,               CRANE__ },
    { "CRANEPOLE",           &CRANEPOLE,           CRANEPOLE__ },
    { "CROSSHAIR",           &CROSSHAIR,           CROSSHAIR__ },
    { "CRYSTALAMMO",         &CRYSTALAMMO,         CRYSTALAMMO__ },
    { "CYCLER",              &CYCLER,              CYCLER__ },
    { "DEVISTATOR",          &DEVISTATOR,          DEVISTATOR__ },
    { "DEVISTATORAMMO",      &DEVISTATORAMMO,      DEVISTATORAMMO__ },
    { "DEVISTATORSPRITE",    &DEVISTATORSPRITE,    DEVISTATORSPRITE__ },
    { "DIGITALNUM",          &DIGITALNUM,          DIGITALNUM__ },
    { "DIPSWITCH",           &DIPSWITCH,           DIPSWITCH__ },
    { "DIPSWITCH2",          &DIPSWITCH2,          DIPSWITCH2__ },
    { "DIPSWITCH3",          &DIPSWITCH3,          DIPSWITCH3__ },
    { "DOLPHIN1",            &DOLPHIN1,            DOLPHIN1__ },
    { "DOLPHIN2",            &DOLPHIN2,            DOLPHIN2__ },
    { "DOMELITE",            &DOMELITE,            DOMELITE__ },
    { "DOORSHOCK",           &DOORSHOCK,           DOORSHOCK__ },
    { "DOORTILE1",           &DOORTILE1,           DOORTILE1__ },
    { "DOORTILE10",          &DOORTILE10,          DOORTILE10__ },
    { "DOORTILE11",          &DOORTILE11,          DOORTILE11__ },
    { "DOORTILE12",          &DOORTILE12,          DOORTILE12__ },
    { "DOORTILE14",          &DOORTILE14,          DOORTILE14__ },
    { "DOORTILE15",          &DOORTILE15,          DOORTILE15__ },
    { "DOORTILE16",          &DOORTILE16,          DOORTILE16__ },
    { "DOORTILE17",          &DOORTILE17,          DOORTILE17__ },
    { "DOORTILE18",          &DOORTILE18,          DOORTILE18__ },
    { "DOORTILE19",          &DOORTILE19,          DOORTILE19__ },
    { "DOORTILE2",           &DOORTILE2,           DOORTILE2__ },
    { "DOORTILE20",          &DOORTILE20,          DOORTILE20__ },
    { "DOORTILE21",          &DOORTILE21,          DOORTILE21__ },
    { "DOORTILE22",          &DOORTILE22,          DOORTILE22__ },
    { "DOORTILE23",          &DOORTILE23,          DOORTILE23__ },
    { "DOORTILE3",           &DOORTILE3,           DOORTILE3__ },
    { "DOORTILE4",           &DOORTILE4,           DOORTILE4__ },
    { "DOORTILE5",           &DOORTILE5,           DOORTILE5__ },
    { "DOORTILE6",           &DOORTILE6,           DOORTILE6__ },
    { "DOORTILE7",           &DOORTILE7,           DOORTILE7__ },
    { "DOORTILE8",           &DOORTILE8,           DOORTILE8__ },
    { "DOORTILE9",           &DOORTILE9,           DOORTILE9__ },
    { "DREALMS",             &DREALMS,             DREALMS__ },
    { "DRONE",               &DRONE,               DRONE__ },
    { "DUCK",                &DUCK,                DUCK__ },
    { "DUKECAR",             &DUKECAR,             DUKECAR__ },
    { "DUKEGUN",             &DUKEGUN,             DUKEGUN__ },
    { "DUKELEG",             &DUKELEG,             DUKELEG__ },
    { "DUKELYINGDEAD",       &DUKELYINGDEAD,       DUKELYINGDEAD__ },
    { "DUKENUKEM",           &DUKENUKEM,           DUKENUKEM__ },
    { "DUKETAG",             &DUKETAG,             DUKETAG__ },
    { "DUKETORSO",           &DUKETORSO,           DUKETORSO__ },
    { "EGG",                 &EGG,                 EGG__ },
    { "ENDALPHANUM",         &ENDALPHANUM,         ENDALPHANUM__ },
    { "EXPLODINGBARREL",     &EXPLODINGBARREL,     EXPLODINGBARREL__ },
    { "EXPLODINGBARREL2",    &EXPLODINGBARREL2,    EXPLODINGBARREL2__ },
    { "EXPLOSION2",          &EXPLOSION2,          EXPLOSION2__ },
    { "EXPLOSION2BOT",       &EXPLOSION2BOT,       EXPLOSION2BOT__ },
    { "F1HELP",              &F1HELP,              F1HELP__ },
    { "FANSHADOW",           &FANSHADOW,           FANSHADOW__ },
    { "FANSHADOWBROKE",      &FANSHADOWBROKE,      FANSHADOWBROKE__ },
    { "FANSPRITE",           &FANSPRITE,           FANSPRITE__ },
    { "FANSPRITEBROKE",      &FANSPRITEBROKE,      FANSPRITEBROKE__ },
    { "FECES",               &FECES,               FECES__ },
    { "FEM1",                &FEM1,                FEM1__ },
    { "FEM10",               &FEM10,               FEM10__ },
    { "FEM2",                &FEM2,                FEM2__ },
    { "FEM3",                &FEM3,                FEM3__ },
    { "FEM4",                &FEM4,                FEM4__ },
    { "FEM5",                &FEM5,                FEM5__ },
    { "FEM6",                &FEM6,                FEM6__ },
    { "FEM6PAD",             &FEM6PAD,             FEM6PAD__ },
    { "FEM7",                &FEM7,                FEM7__ },
    { "FEM8",                &FEM8,                FEM8__ },
    { "FEM9",                &FEM9,                FEM9__ },
    { "FEMMAG1",             &FEMMAG1,             FEMMAG1__ },
    { "FEMMAG2",             &FEMMAG2,             FEMMAG2__ },
    { "FEMPIC1",             &FEMPIC1,             FEMPIC1__ },
    { "FEMPIC2",             &FEMPIC2,             FEMPIC2__ },
    { "FEMPIC3",             &FEMPIC3,             FEMPIC3__ },
    { "FEMPIC4",             &FEMPIC4,             FEMPIC4__ },
    { "FEMPIC5",             &FEMPIC5,             FEMPIC5__ },
    { "FEMPIC6",             &FEMPIC6,             FEMPIC6__ },
    { "FEMPIC7",             &FEMPIC7,             FEMPIC7__ },
    { "FETUS",               &FETUS,               FETUS__ },
    { "FETUSBROKE",          &FETUSBROKE,          FETUSBROKE__ },
    { "FETUSJIB",            &FETUSJIB,            FETUSJIB__ },
    { "FIRE",                &FIRE,                FIRE__ },
    { "FIRE2",               &FIRE2,               FIRE2__ },
    { "FIREBARREL",          &FIREBARREL,          FIREBARREL__ },
    { "FIREEXT",             &FIREEXT,             FIREEXT__ },
    { "FIRELASER",           &FIRELASER,           FIRELASER__ },
    { "FIREVASE",            &FIREVASE,            FIREVASE__ },
    { "FIRSTAID",            &FIRSTAID,            FIRSTAID__ },
    { "FIRSTAID_ICON",       &FIRSTAID_ICON,       FIRSTAID_ICON__ },
    { "FIRSTGUN",            &FIRSTGUN,            FIRSTGUN__ },
    { "FIRSTGUNRELOAD",      &FIRSTGUNRELOAD,      FIRSTGUNRELOAD__ },
    { "FIRSTGUNSPRITE",      &FIRSTGUNSPRITE,      FIRSTGUNSPRITE__ },
    { "FIST",                &FIST,                FIST__ },
    { "FLOORFLAME",          &FLOORFLAME,          FLOORFLAME__ },
    { "FLOORPLASMA",         &FLOORPLASMA,         FLOORPLASMA__ },
    { "FLOORSLIME",          &FLOORSLIME,          FLOORSLIME__ },
    { "FOF",                 &FOF,                 FOF__ },
    { "FOODOBJECT16",        &FOODOBJECT16,        FOODOBJECT16__ },
    { "FOOTPRINTS",          &FOOTPRINTS,          FOOTPRINTS__ },
    { "FOOTPRINTS2",         &FOOTPRINTS2,         FOOTPRINTS2__ },
    { "FOOTPRINTS3",         &FOOTPRINTS3,         FOOTPRINTS3__ },
    { "FOOTPRINTS4",         &FOOTPRINTS4,         FOOTPRINTS4__ },
    { "FORCERIPPLE",         &FORCERIPPLE,         FORCERIPPLE__ },
    { "FORCESPHERE",         &FORCESPHERE,         FORCESPHERE__ },
    { "FRAGBAR",             &FRAGBAR,             FRAGBAR__ },
    { "FRAMEEFFECT1",        &FRAMEEFFECT1,        FRAMEEFFECT1__ },
    { "FRAMEEFFECT1_13",     &FRAMEEFFECT1_13,     FRAMEEFFECT1_13__ },
    { "FRANKENSTINESWITCH",  &FRANKENSTINESWITCH,  FRANKENSTINESWITCH__ },
    { "FREEZE",              &FREEZE,              FREEZE__ },
    { "FREEZEAMMO",          &FREEZEAMMO,          FREEZEAMMO__ },
    { "FREEZEBLAST",         &FREEZEBLAST,         FREEZEBLAST__ },
    { "FREEZESPRITE",        &FREEZESPRITE,        FREEZESPRITE__ },
    { "FUELPOD",             &FUELPOD,             FUELPOD__ },
    { "GENERICPOLE",         &GENERICPOLE,         GENERICPOLE__ },
    { "GENERICPOLE2",        &GENERICPOLE2,        GENERICPOLE2__ },
    { "GLASS",               &GLASS,               GLASS__ },
    { "GLASS2",              &GLASS2,              GLASS2__ },
    { "GLASSPIECES",         &GLASSPIECES,         GLASSPIECES__ },
    { "GPSPEED",             &GPSPEED,             GPSPEED__ },
    { "GRATE1",              &GRATE1,              GRATE1__ },
    { "GREENSLIME",          &GREENSLIME,          GREENSLIME__ },
    { "GROWAMMO",            &GROWAMMO,            GROWAMMO__ },
    { "GROWSPARK",           &GROWSPARK,           GROWSPARK__ },
    { "GROWSPRITEICON",      &GROWSPRITEICON,      GROWSPRITEICON__ },
    { "HANDHOLDINGACCESS",   &HANDHOLDINGACCESS,   HANDHOLDINGACCESS__ },
    { "HANDHOLDINGLASER",    &HANDHOLDINGLASER,    HANDHOLDINGLASER__ },
    { "HANDPRINTSWITCH",     &HANDPRINTSWITCH,     HANDPRINTSWITCH__ },
    { "HANDREMOTE",          &HANDREMOTE,          HANDREMOTE__ },
    { "HANDSWITCH",          &HANDSWITCH,          HANDSWITCH__ },
    { "HANDTHROW",           &HANDTHROW,           HANDTHROW__ },
    { "HANGLIGHT",           &HANGLIGHT,           HANGLIGHT__ },
    { "HBOMBAMMO",           &HBOMBAMMO,           HBOMBAMMO__ },
    { "HEADJIB1",            &HEADJIB1,            HEADJIB1__ },
    { "HEALTHBOX",           &HEALTHBOX,           HEALTHBOX__ },
    { "HEAT_ICON",           &HEAT_ICON,           HEAT_ICON__ },
    { "HEATSENSOR",          &HEATSENSOR,          HEATSENSOR__ },
    { "HEAVYHBOMB",          &HEAVYHBOMB,          HEAVYHBOMB__ },
    { "HELECOPT",            &HELECOPT,            HELECOPT__ },
    { "HOLODUKE",            &HOLODUKE,            HOLODUKE__ },
    { "HOLODUKE_ICON",       &HOLODUKE_ICON,       HOLODUKE_ICON__ },
    { "HORSEONSIDE",         &HORSEONSIDE,         HORSEONSIDE__ },
    { "HOTMEAT",             &HOTMEAT,             HOTMEAT__ },
    { "HURTRAIL",            &HURTRAIL,            HURTRAIL__ },
    { "HYDRENT",             &HYDRENT,             HYDRENT__ },
    { "HYDROPLANT",          &HYDROPLANT,          HYDROPLANT__ },
    { "INDY",                &INDY,                INDY__ },
    { "INGAMEDUKETHREEDEE",  &INGAMEDUKETHREEDEE,  INGAMEDUKETHREEDEE__ },
    { "INNERJAW",            &INNERJAW,            INNERJAW__ },
    { "INVENTORYBOX",        &INVENTORYBOX,        INVENTORYBOX__ },
    { "IVUNIT",              &IVUNIT,              IVUNIT__ },
    { "JETPACK",             &JETPACK,             JETPACK__ },
    { "JETPACK_ICON",        &JETPACK_ICON,        JETPACK_ICON__ },
    { "JIBS1",               &JIBS1,               JIBS1__ },
    { "JIBS2",               &JIBS2,               JIBS2__ },
    { "JIBS3",               &JIBS3,               JIBS3__ },
    { "JIBS4",               &JIBS4,               JIBS4__ },
    { "JIBS5",               &JIBS5,               JIBS5__ },
    { "JIBS6",               &JIBS6,               JIBS6__ },
    { "JURYGUY",             &JURYGUY,             JURYGUY__ },
    { "KILLSICON",           &KILLSICON,           KILLSICON__ },
    { "KNEE",                &KNEE,                KNEE__ },
    { "LA",                  &LA,                  LA__ },
    { "LASERLINE",           &LASERLINE,           LASERLINE__ },
    { "LASERSITE",           &LASERSITE,           LASERSITE__ },
    { "LEGJIB1",             &LEGJIB1,             LEGJIB1__ },
    { "LETTER",              &LETTER,              LETTER__ },
    { "LIGHTSWITCH",         &LIGHTSWITCH,         LIGHTSWITCH__ },
    { "LIGHTSWITCH2",        &LIGHTSWITCH2,        LIGHTSWITCH2__ },
    { "LIZMAN",              &LIZMAN,              LIZMAN__ },
    { "LIZMANARM1",          &LIZMANARM1,          LIZMANARM1__ },
    { "LIZMANFEEDING",       &LIZMANFEEDING,       LIZMANFEEDING__ },
    { "LIZMANHEAD1",         &LIZMANHEAD1,         LIZMANHEAD1__ },
    { "LIZMANJUMP",          &LIZMANJUMP,          LIZMANJUMP__ },
    { "LIZMANLEG1",          &LIZMANLEG1,          LIZMANLEG1__ },
    { "LIZMANSPITTING",      &LIZMANSPITTING,      LIZMANSPITTING__ },
    { "LIZMANSTAYPUT",       &LIZMANSTAYPUT,       LIZMANSTAYPUT__ },
    { "LIZTROOP",            &LIZTROOP,            LIZTROOP__ },
    { "LIZTROOPDUCKING",     &LIZTROOPDUCKING,     LIZTROOPDUCKING__ },
    { "LIZTROOPJETPACK",     &LIZTROOPJETPACK,     LIZTROOPJETPACK__ },
    { "LIZTROOPJUSTSIT",     &LIZTROOPJUSTSIT,     LIZTROOPJUSTSIT__ },
    { "LIZTROOPONTOILET",    &LIZTROOPONTOILET,    LIZTROOPONTOILET__ },
    { "LIZTROOPRUNNING",     &LIZTROOPRUNNING,     LIZTROOPRUNNING__ },
    { "LIZTROOPSHOOT",       &LIZTROOPSHOOT,       LIZTROOPSHOOT__ },
    { "LIZTROOPSTAYPUT",     &LIZTROOPSTAYPUT,     LIZTROOPSTAYPUT__ },
    { "LOADSCREEN",          &LOADSCREEN,          LOADSCREEN__ },
    { "LOCATORS",            &LOCATORS,            LOCATORS__ },
    { "LOCKSWITCH1",         &LOCKSWITCH1,         LOCKSWITCH1__ },
    { "LOOGIE",              &LOOGIE,              LOOGIE__ },
    { "LUKE",                &LUKE,                LUKE__ },
    { "MAIL",                &MAIL,                MAIL__ },
    { "MAN",                 &MAN,                 MAN__ },
    { "MAN2",                &MAN2,                MAN2__ },
    { "MASKWALL1",           &MASKWALL1,           MASKWALL1__ },
    { "MASKWALL10",          &MASKWALL10,          MASKWALL10__ },
    { "MASKWALL11",          &MASKWALL11,          MASKWALL11__ },
    { "MASKWALL12",          &MASKWALL12,          MASKWALL12__ },
    { "MASKWALL13",          &MASKWALL13,          MASKWALL13__ },
    { "MASKWALL14",          &MASKWALL14,          MASKWALL14__ },
    { "MASKWALL15",          &MASKWALL15,          MASKWALL15__ },
    { "MASKWALL2",           &MASKWALL2,           MASKWALL2__ },
    { "MASKWALL3",           &MASKWALL3,           MASKWALL3__ },
    { "MASKWALL4",           &MASKWALL4,           MASKWALL4__ },
    { "MASKWALL5",           &MASKWALL5,           MASKWALL5__ },
    { "MASKWALL6",           &MASKWALL6,           MASKWALL6__ },
    { "MASKWALL7",           &MASKWALL7,           MASKWALL7__ },
    { "MASKWALL8",           &MASKWALL8,           MASKWALL8__ },
    { "MASKWALL9",           &MASKWALL9,           MASKWALL9__ },
    { "MASTERSWITCH",        &MASTERSWITCH,        MASTERSWITCH__ },
    { "MENUBAR",             &MENUBAR,             MENUBAR__ },
    { "MENUSCREEN",          &MENUSCREEN,          MENUSCREEN__ },
    { "MIKE",                &MIKE,                MIKE__ },
    { "MINIFONT",            &MINIFONT,            MINIFONT__ },
    { "MIRROR",              &MIRROR,              MIRROR__ },
    { "MIRRORBROKE",         &MIRRORBROKE,         MIRRORBROKE__ },
    { "MONEY",               &MONEY,               MONEY__ },
    { "MONK",                &MONK,                MONK__ },
    { "MOONSKY1",            &MOONSKY1,            MOONSKY1__ },
    { "MORTER",              &MORTER,              MORTER__ },
    { "MOVIECAMERA",         &MOVIECAMERA,         MOVIECAMERA__ },
    { "MULTISWITCH",         &MULTISWITCH,         MULTISWITCH__ },
    { "MUSICANDSFX",         &MUSICANDSFX,         MUSICANDSFX__ },
    { "NAKED1",              &NAKED1,              NAKED1__ },
    { "NATURALLIGHTNING",    &NATURALLIGHTNING,    NATURALLIGHTNING__ },
    { "NEON1",               &NEON1,               NEON1__ },
    { "NEON2",               &NEON2,               NEON2__ },
    { "NEON3",               &NEON3,               NEON3__ },
    { "NEON4",               &NEON4,               NEON4__ },
    { "NEON5",               &NEON5,               NEON5__ },
    { "NEON6",               &NEON6,               NEON6__ },
    { "NEWBEAST",            &NEWBEAST,            NEWBEAST__ },
    { "NEWBEASTSTAYPUT",     &NEWBEASTSTAYPUT,     NEWBEASTSTAYPUT__ },
    { "NUKEBARREL",          &NUKEBARREL,          NUKEBARREL__ },
    { "NUKEBARRELDENTED",    &NUKEBARRELDENTED,    NUKEBARRELDENTED__ },
    { "NUKEBARRELLEAKED",    &NUKEBARRELLEAKED,    NUKEBARRELLEAKED__ },
    { "NUKEBUTTON",          &NUKEBUTTON,          NUKEBUTTON__ },
    { "OCEANSPRITE1",        &OCEANSPRITE1,        OCEANSPRITE1__ },
    { "OCEANSPRITE2",        &OCEANSPRITE2,        OCEANSPRITE2__ },
    { "OCEANSPRITE3",        &OCEANSPRITE3,        OCEANSPRITE3__ },
    { "OCEANSPRITE4",        &OCEANSPRITE4,        OCEANSPRITE4__ },
    { "OCEANSPRITE5",        &OCEANSPRITE5,        OCEANSPRITE5__ },
    { "OCTABRAIN",           &OCTABRAIN,           OCTABRAIN__ },
    { "OCTABRAINSTAYPUT",    &OCTABRAINSTAYPUT,    OCTABRAINSTAYPUT__ },
    { "OJ",                  &OJ,                  OJ__ },
    { "OOZ",                 &OOZ,                 OOZ__ },
    { "OOZ2",                &OOZ2,                OOZ2__ },
    { "OOZFILTER",           &OOZFILTER,           OOZFILTER__ },
    { "ORDERING",            &ORDERING,            ORDERING__ },
    { "ORGANTIC",            &ORGANTIC,            ORGANTIC__ },
    { "PANNEL1",             &PANNEL1,             PANNEL1__ },
    { "PANNEL2",             &PANNEL2,             PANNEL2__ },
    { "PANNEL3",             &PANNEL3,             PANNEL3__ },
    { "PAPER",               &PAPER,               PAPER__ },
    { "PIGCOP",              &PIGCOP,              PIGCOP__ },
    { "PIGCOPDIVE",          &PIGCOPDIVE,          PIGCOPDIVE__ },
    { "PIGCOPSTAYPUT",       &PIGCOPSTAYPUT,       PIGCOPSTAYPUT__ },
    { "PIPE1",               &PIPE1,               PIPE1__ },
    { "PIPE1B",              &PIPE1B,              PIPE1B__ },
    { "PIPE2",               &PIPE2,               PIPE2__ },
    { "PIPE2B",              &PIPE2B,              PIPE2B__ },
    { "PIPE3",               &PIPE3,               PIPE3__ },
    { "PIPE3B",              &PIPE3B,              PIPE3B__ },
    { "PIPE4",               &PIPE4,               PIPE4__ },
    { "PIPE4B",              &PIPE4B,              PIPE4B__ },
    { "PIPE5",               &PIPE5,               PIPE5__ },
    { "PIPE5B",              &PIPE5B,              PIPE5B__ },
    { "PIPE6",               &PIPE6,               PIPE6__ },
    { "PIPE6B",              &PIPE6B,              PIPE6B__ },
    { "PLAYERONWATER",       &PLAYERONWATER,       PLAYERONWATER__ },
    { "PLUG",                &PLUG,                PLUG__ },
    { "PLUTOPAKSPRITE",      &PLUTOPAKSPRITE,      PLUTOPAKSPRITE__ },
    { "POCKET",              &POCKET,              POCKET__ },
    { "PODFEM1",             &PODFEM1,             PODFEM1__ },
    { "POT1",                &POT1,                POT1__ },
    { "POT2",                &POT2,                POT2__ },
    { "POT3",                &POT3,                POT3__ },
    { "POWERSWITCH1",        &POWERSWITCH1,        POWERSWITCH1__ },
    { "POWERSWITCH2",        &POWERSWITCH2,        POWERSWITCH2__ },
    { "PUKE",                &PUKE,                PUKE__ },
    { "PULLSWITCH",          &PULLSWITCH,          PULLSWITCH__ },
    { "PURPLELAVA",          &PURPLELAVA,          PURPLELAVA__ },
    { "QUEBALL",             &QUEBALL,             QUEBALL__ },
    { "RADIUSEXPLOSION",     &RADIUSEXPLOSION,     RADIUSEXPLOSION__ },
    { "RAT",                 &RAT,                 RAT__ },
    { "REACTOR",             &REACTOR,             REACTOR__ },
    { "REACTOR2",            &REACTOR2,            REACTOR2__ },
    { "REACTOR2BURNT",       &REACTOR2BURNT,       REACTOR2BURNT__ },
    { "REACTOR2SPARK",       &REACTOR2SPARK,       REACTOR2SPARK__ },
    { "REACTORBURNT",        &REACTORBURNT,        REACTORBURNT__ },
    { "REACTORSPARK",        &REACTORSPARK,        REACTORSPARK__ },
    { "RECON",               &RECON,               RECON__ },
    { "RESPAWN",             &RESPAWN,             RESPAWN__ },
    { "RESPAWNMARKERGREEN",  &RESPAWNMARKERGREEN,  RESPAWNMARKERGREEN__ },
    { "RESPAWNMARKERRED",    &RESPAWNMARKERRED,    RESPAWNMARKERRED__ },
    { "RESPAWNMARKERYELLOW", &RESPAWNMARKERYELLOW, RESPAWNMARKERYELLOW__ },
    { "ROTATEGUN",           &ROTATEGUN,           ROTATEGUN__ },
    { "RPG",                 &RPG,                 RPG__ },
    { "RPGAMMO",             &RPGAMMO,             RPGAMMO__ },
    { "RPGGUN",              &RPGGUN,              RPGGUN__ },
    { "RPGSPRITE",           &RPGSPRITE,           RPGSPRITE__ },
    { "RUBBERCAN",           &RUBBERCAN,           RUBBERCAN__ },
    { "SATELITE",            &SATELITE,            SATELITE__ },
    { "SCALE",               &SCALE,               SCALE__ },
    { "SCRAP1",              &SCRAP1,              SCRAP1__ },
    { "SCRAP2",              &SCRAP2,              SCRAP2__ },
    { "SCRAP3",              &SCRAP3,              SCRAP3__ },
    { "SCRAP4",              &SCRAP4,              SCRAP4__ },
    { "SCRAP5",              &SCRAP5,              SCRAP5__ },
    { "SCRAP6",              &SCRAP6,              SCRAP6__ },
    { "SCREENBREAK1",        &SCREENBREAK1,        SCREENBREAK1__ },
    { "SCREENBREAK10",       &SCREENBREAK10,       SCREENBREAK10__ },
    { "SCREENBREAK11",       &SCREENBREAK11,       SCREENBREAK11__ },
    { "SCREENBREAK12",       &SCREENBREAK12,       SCREENBREAK12__ },
    { "SCREENBREAK13",       &SCREENBREAK13,       SCREENBREAK13__ },
    { "SCREENBREAK14",       &SCREENBREAK14,       SCREENBREAK14__ },
    { "SCREENBREAK15",       &SCREENBREAK15,       SCREENBREAK15__ },
    { "SCREENBREAK16",       &SCREENBREAK16,       SCREENBREAK16__ },
    { "SCREENBREAK17",       &SCREENBREAK17,       SCREENBREAK17__ },
    { "SCREENBREAK18",       &SCREENBREAK18,       SCREENBREAK18__ },
    { "SCREENBREAK19",       &SCREENBREAK19,       SCREENBREAK19__ },
    { "SCREENBREAK2",        &SCREENBREAK2,        SCREENBREAK2__ },
    { "SCREENBREAK3",        &SCREENBREAK3,        SCREENBREAK3__ },
    { "SCREENBREAK4",        &SCREENBREAK4,        SCREENBREAK4__ },
    { "SCREENBREAK5",        &SCREENBREAK5,        SCREENBREAK5__ },
    { "SCREENBREAK6",        &SCREENBREAK6,        SCREENBREAK6__ },
    { "SCREENBREAK7",        &SCREENBREAK7,        SCREENBREAK7__ },
    { "SCREENBREAK8",        &SCREENBREAK8,        SCREENBREAK8__ },
    { "SCREENBREAK9",        &SCREENBREAK9,        SCREENBREAK9__ },
    { "SCUBAMASK",           &SCUBAMASK,           SCUBAMASK__ },
    { "SECTOREFFECTOR",      &SECTOREFFECTOR,      SECTOREFFECTOR__ },
    { "SEENINE",             &SEENINE,             SEENINE__ },
    { "SEENINEDEAD",         &SEENINEDEAD,         SEENINEDEAD__ },
    { "SELECTDIR",           &SELECTDIR,           SELECTDIR__ },
    { "SHARK",               &SHARK,               SHARK__ },
    { "SHELL",               &SHELL,               SHELL__ },
    { "SHIELD",              &SHIELD,              SHIELD__ },
    { "SHOTGUN",             &SHOTGUN,             SHOTGUN__ },
    { "SHOTGUNAMMO",         &SHOTGUNAMMO,         SHOTGUNAMMO__ },
    { "SHOTGUNSHELL",        &SHOTGUNSHELL,        SHOTGUNSHELL__ },
    { "SHOTGUNSPRITE",       &SHOTGUNSPRITE,       SHOTGUNSPRITE__ },
    { "SHOTSPARK1",          &SHOTSPARK1,          SHOTSPARK1__ },
    { "SHRINKER",            &SHRINKER,            SHRINKER__ },
    { "SHRINKEREXPLOSION",   &SHRINKEREXPLOSION,   SHRINKEREXPLOSION__ },
    { "SHRINKERSPRITE",      &SHRINKERSPRITE,      SHRINKERSPRITE__ },
    { "SHRINKSPARK",         &SHRINKSPARK,         SHRINKSPARK__ },
    { "SIDEBOLT1",           &SIDEBOLT1,           SIDEBOLT1__ },
    { "SIGN1",               &SIGN1,               SIGN1__ },
    { "SIGN2",               &SIGN2,               SIGN2__ },
    { "SIXPAK",              &SIXPAK,              SIXPAK__ },
    { "SLIDEBAR",            &SLIDEBAR,            SLIDEBAR__ },
    { "SLOTDOOR",            &SLOTDOOR,            SLOTDOOR__ },
    { "SMALLFNTCURSOR",      &SMALLFNTCURSOR,      SMALLFNTCURSOR__ },
    { "SMALLSMOKE",          &SMALLSMOKE,          SMALLSMOKE__ },
    { "SOLARPANNEL",         &SOLARPANNEL,         SOLARPANNEL__ },
    { "SPACEDOORSWITCH",     &SPACEDOORSWITCH,     SPACEDOORSWITCH__ },
    { "SPACELIGHTSWITCH",    &SPACELIGHTSWITCH,    SPACELIGHTSWITCH__ },
    { "SPACEMARINE",         &SPACEMARINE,         SPACEMARINE__ },
    { "SPEAKER",             &SPEAKER,             SPEAKER__ },
    { "SPINNINGNUKEICON",    &SPINNINGNUKEICON,    SPINNINGNUKEICON__ },
    { "SPIT",                &SPIT,                SPIT__ },
    { "SPOTLITE",            &SPOTLITE,            SPOTLITE__ },
    { "STAINGLASS1",         &STAINGLASS1,         STAINGLASS1__ },
    { "STALL",               &STALL,               STALL__ },
    { "STALLBROKE",          &STALLBROKE,          STALLBROKE__ },
    { "STARTALPHANUM",       &STARTALPHANUM,       STARTALPHANUM__ },
    { "STATIC",              &STATIC,              STATIC__ },
    { "STATUE",              &STATUE,              STATUE__ },
    { "STATUEFLASH",         &STATUEFLASH,         STATUEFLASH__ },
    { "STEAM",               &STEAM,               STEAM__ },
    { "STEROIDS",            &STEROIDS,            STEROIDS__ },
    { "STEROIDS_ICON",       &STEROIDS_ICON,       STEROIDS_ICON__ },
    { "STRIPEBALL",          &STRIPEBALL,          STRIPEBALL__ },
    { "SUSHIPLATE1",         &SUSHIPLATE1,         SUSHIPLATE1__ },
    { "SUSHIPLATE2",         &SUSHIPLATE2,         SUSHIPLATE2__ },
    { "SUSHIPLATE3",         &SUSHIPLATE3,         SUSHIPLATE3__ },
    { "SUSHIPLATE4",         &SUSHIPLATE4,         SUSHIPLATE4__ },
    { "SUSHIPLATE5",         &SUSHIPLATE5,         SUSHIPLATE5__ },
    { "TAMPON",              &TAMPON,              TAMPON__ },
    { "TANK",                &TANK,                TANK__ },
    { "TARGET",              &TARGET,              TARGET__ },
    { "TECHLIGHT2",          &TECHLIGHT2,          TECHLIGHT2__ },
    { "TECHLIGHT4",          &TECHLIGHT4,          TECHLIGHT4__ },
    { "TECHLIGHTBUST2",      &TECHLIGHTBUST2,      TECHLIGHTBUST2__ },
    { "TECHLIGHTBUST4",      &TECHLIGHTBUST4,      TECHLIGHTBUST4__ },
    { "TECHSWITCH",          &TECHSWITCH,          TECHSWITCH__ },
    { "TENSCREEN",           &TENSCREEN,           TENSCREEN__ },
    { "TEXTBOX",             &TEXTBOX,             TEXTBOX__ },
    { "TEXTSTORY",           &TEXTSTORY,           TEXTSTORY__ },
    { "THREEBYFIVE",         &THREEBYFIVE,         THREEBYFIVE__ },
    { "THREEDEE",            &THREEDEE,            THREEDEE__ },
    { "TIP",                 &TIP,                 TIP__ },
    { "TIRE",                &TIRE,                TIRE__ },
    { "TOILET",              &TOILET,              TOILET__ },
    { "TOILETBROKE",         &TOILETBROKE,         TOILETBROKE__ },
    { "TOILETWATER",         &TOILETWATER,         TOILETWATER__ },
    { "TONGUE",              &TONGUE,              TONGUE__ },
    { "TOUCHPLATE",          &TOUCHPLATE,          TOUCHPLATE__ },
    { "TOUGHGAL",            &TOUGHGAL,            TOUGHGAL__ },
    { "TRANSPORTERBEAM",     &TRANSPORTERBEAM,     TRANSPORTERBEAM__ },
    { "TRANSPORTERSTAR",     &TRANSPORTERSTAR,     TRANSPORTERSTAR__ },
    { "TRASH",               &TRASH,               TRASH__ },
    { "TREE1",               &TREE1,               TREE1__ },
    { "TREE2",               &TREE2,               TREE2__ },
    { "TRIPBOMB",            &TRIPBOMB,            TRIPBOMB__ },
    { "TRIPBOMBSPRITE",      &TRIPBOMBSPRITE,      TRIPBOMBSPRITE__ },
    { "TRIPODCAMERA",        &TRIPODCAMERA,        TRIPODCAMERA__ },
    { "VACUUM",              &VACUUM,              VACUUM__ },
    { "VASE",                &VASE,                VASE__ },
    { "VENDMACHINE",         &VENDMACHINE,         VENDMACHINE__ },
    { "VICTORY1",            &VICTORY1,            VICTORY1__ },
    { "VIEWBORDER",          &VIEWBORDER,          VIEWBORDER__ },
    { "VIEWSCREEN",          &VIEWSCREEN,          VIEWSCREEN__ },
    { "VIEWSCREEN2",         &VIEWSCREEN2,         VIEWSCREEN2__ },
    { "W_FORCEFIELD",        &W_FORCEFIELD,        W_FORCEFIELD__ },
    { "W_HITTECHWALL1",      &W_HITTECHWALL1,      W_HITTECHWALL1__ },
    { "W_HITTECHWALL10",     &W_HITTECHWALL10,     W_HITTECHWALL10__ },
    { "W_HITTECHWALL15",     &W_HITTECHWALL15,     W_HITTECHWALL15__ },
    { "W_HITTECHWALL16",     &W_HITTECHWALL16,     W_HITTECHWALL16__ },
    { "W_HITTECHWALL2",      &W_HITTECHWALL2,      W_HITTECHWALL2__ },
    { "W_HITTECHWALL3",      &W_HITTECHWALL3,      W_HITTECHWALL3__ },
    { "W_HITTECHWALL4",      &W_HITTECHWALL4,      W_HITTECHWALL4__ },
    { "W_MILKSHELF",         &W_MILKSHELF,         W_MILKSHELF__ },
    { "W_MILKSHELFBROKE",    &W_MILKSHELFBROKE,    W_MILKSHELFBROKE__ },
    { "W_NUMBERS",           &W_NUMBERS,           W_NUMBERS__ },
    { "W_SCREENBREAK",       &W_SCREENBREAK,       W_SCREENBREAK__ },
    { "W_TECHWALL1",         &W_TECHWALL1,         W_TECHWALL1__ },
    { "W_TECHWALL10",        &W_TECHWALL10,        W_TECHWALL10__ },
    { "W_TECHWALL11",        &W_TECHWALL11,        W_TECHWALL11__ },
    { "W_TECHWALL12",        &W_TECHWALL12,        W_TECHWALL12__ },
    { "W_TECHWALL13",        &W_TECHWALL13,        W_TECHWALL13__ },
    { "W_TECHWALL14",        &W_TECHWALL14,        W_TECHWALL14__ },
    { "W_TECHWALL15",        &W_TECHWALL15,        W_TECHWALL15__ },
    { "W_TECHWALL16",        &W_TECHWALL16,        W_TECHWALL16__ },
    { "W_TECHWALL2",         &W_TECHWALL2,         W_TECHWALL2__ },
    { "W_TECHWALL3",         &W_TECHWALL3,         W_TECHWALL3__ },
    { "W_TECHWALL4",         &W_TECHWALL4,         W_TECHWALL4__ },
    { "W_TECHWALL5",         &W_TECHWALL5,         W_TECHWALL5__ },
    { "W_TECHWALL6",         &W_TECHWALL6,         W_TECHWALL6__ },
    { "W_TECHWALL7",         &W_TECHWALL7,         W_TECHWALL7__ },
    { "W_TECHWALL8",         &W_TECHWALL8,         W_TECHWALL8__ },
    { "W_TECHWALL9",         &W_TECHWALL9,         W_TECHWALL9__ },
    { "WAITTOBESEATED",      &WAITTOBESEATED,      WAITTOBESEATED__ },
    { "WALLBLOOD1",          &WALLBLOOD1,          WALLBLOOD1__ },
    { "WALLBLOOD2",          &WALLBLOOD2,          WALLBLOOD2__ },
    { "WALLBLOOD3",          &WALLBLOOD3,          WALLBLOOD3__ },
    { "WALLBLOOD4",          &WALLBLOOD4,          WALLBLOOD4__ },
    { "WALLBLOOD5",          &WALLBLOOD5,          WALLBLOOD5__ },
    { "WALLBLOOD7",          &WALLBLOOD7,          WALLBLOOD7__ },
    { "WALLBLOOD8",          &WALLBLOOD8,          WALLBLOOD8__ },
    { "WALLLIGHT1",          &WALLLIGHT1,          WALLLIGHT1__ },
    { "WALLLIGHT2",          &WALLLIGHT2,          WALLLIGHT2__ },
    { "WALLLIGHT3",          &WALLLIGHT3,          WALLLIGHT3__ },
    { "WALLLIGHT4",          &WALLLIGHT4,          WALLLIGHT4__ },
    { "WALLLIGHTBUST1",      &WALLLIGHTBUST1,      WALLLIGHTBUST1__ },
    { "WALLLIGHTBUST2",      &WALLLIGHTBUST2,      WALLLIGHTBUST2__ },
    { "WALLLIGHTBUST3",      &WALLLIGHTBUST3,      WALLLIGHTBUST3__ },
    { "WALLLIGHTBUST4",      &WALLLIGHTBUST4,      WALLLIGHTBUST4__ },
    { "WATERBUBBLE",         &WATERBUBBLE,         WATERBUBBLE__ },
    { "WATERBUBBLEMAKER",    &WATERBUBBLEMAKER,    WATERBUBBLEMAKER__ },
    { "WATERDRIP",           &WATERDRIP,           WATERDRIP__ },
    { "WATERDRIPSPLASH",     &WATERDRIPSPLASH,     WATERDRIPSPLASH__ },
    { "WATERFOUNTAIN",       &WATERFOUNTAIN,       WATERFOUNTAIN__ },
    { "WATERFOUNTAINBROKE",  &WATERFOUNTAINBROKE,  WATERFOUNTAINBROKE__ },
    { "WATERSPLASH2",        &WATERSPLASH2,        WATERSPLASH2__ },
    { "WATERTILE2",          &WATERTILE2,          WATERTILE2__ },
    { "WEATHERWARN",         &WEATHERWARN,         WEATHERWARN__ },
    { "WINDOWBORDER1",       &WINDOWBORDER1,       WINDOWBORDER1__ },
    { "WINDOWBORDER2",       &WINDOWBORDER2,       WINDOWBORDER2__ },
    { "WOMAN",               &WOMAN,               WOMAN__ },
    { "WOODENHORSE",         &WOODENHORSE,         WOODENHORSE__ },
    { "XXXSTACY",            &XXXSTACY,            XXXSTACY__ },
    { "WIDESCREENSTATUSBAR", &WIDESCREENSTATUSBAR, WIDESCREENSTATUSBAR__ },
    { "RPGGUNWIDE",          &RPGGUNWIDE,          RPGGUNWIDE__ },
    { "FIRSTGUNRELOADWIDE",  &FIRSTGUNRELOADWIDE,  FIRSTGUNRELOADWIDE__ },
    { "FREEZEWIDE",          &FREEZEWIDE,          FREEZEWIDE__ },
    { "FREEZEFIREWIDE",      &FREEZEFIREWIDE,      FREEZEFIREWIDE__ },
    { "SHRINKERWIDE",        &SHRINKERWIDE,        SHRINKERWIDE__ },
    { "CRACKKNUCKLESWIDE",   &CRACKKNUCKLESWIDE,   CRACKKNUCKLESWIDE__ },
    { "FLAMETHROWERSPRITE",  &FLAMETHROWERSPRITE,  FLAMETHROWERSPRITE__ },
    { "FLAMETHROWERAMMO",    &FLAMETHROWERAMMO,    FLAMETHROWERAMMO__ },
    { "FLAMETHROWER",        &FLAMETHROWER,        FLAMETHROWER__ },
    { "FLAMETHROWERFIRE",    &FLAMETHROWERFIRE,    FLAMETHROWERFIRE__ },
    { "FLAMETHROWERFLAME",   &FLAMETHROWERFLAME,   FLAMETHROWERFLAME__ },
    { "FLAMETHROWERPILOT",   &FLAMETHROWERPILOT,   FLAMETHROWERPILOT__ },
    { "FIREBALL",            &FIREBALL,            FIREBALL__ },
    { "ONFIRE",              &ONFIRE,              ONFIRE__ },
    { "ONFIRESMOKE",         &ONFIRESMOKE,         ONFIRESMOKE__ },
    { "BURNEDCORPSE",        &BURNEDCORPSE,        BURNEDCORPSE__ },
    { "WHISPYSMOKE",         &WHISPYSMOKE,         WHISPYSMOKE__ },
    { "FIREFLY",             &FIREFLY,             FIREFLY__ },
    { "FIREFLYSHRINKEFFECT", &FIREFLYSHRINKEFFECT, FIREFLYSHRINKEFFECT__ },
    { "FIREFLYGROWEFFECT",   &FIREFLYGROWEFFECT,   FIREFLYGROWEFFECT__ },
    { "FIREFLYFLYINGEFFECT", &FIREFLYFLYINGEFFECT, FIREFLYFLYINGEFFECT__ },
    { "BOSS5",               &BOSS5,               BOSS5__ },
    { "BOSS5STAYPUT",        &BOSS5STAYPUT,        BOSS5STAYPUT__ },
    { "LAVAPOOL",            &LAVAPOOL,            LAVAPOOL__ },
    { "LAVASPLASH",          &LAVASPLASH,          LAVASPLASH__ },
    { "LAVAPOOLBUBBLE",      &LAVAPOOLBUBBLE,      LAVAPOOLBUBBLE__ },
    { "BOSS2STAYPUT",        &BOSS2STAYPUT,        BOSS2STAYPUT__ },
    { "BOSS3STAYPUT",        &BOSS3STAYPUT,        BOSS3STAYPUT__ },
    { "E32_TILE5736",        &E32_TILE5736,        E32_TILE5736__ },
    { "E32_TILE5737",        &E32_TILE5737,        E32_TILE5737__ },
    { "E32_TILE5846",        &E32_TILE5846,        E32_TILE5846__ },
 };

inthashtable_t h_dynamictilemap = { nullptr, 640 };
inthashtable_t h_dsound = { nullptr, INTHASH_SIZE(ARRAY_SIZE(g_dynSoundList)) };

int16_t ALIEN_SWITCH1       = ALIEN_SWITCH1__;
int16_t BIGBANG             = BIGBANG__;
int16_t BONUS_SPEECH1       = BONUS_SPEECH1__;
int16_t BONUS_SPEECH2       = BONUS_SPEECH2__;
int16_t BONUS_SPEECH3       = BONUS_SPEECH3__;
int16_t BONUS_SPEECH4       = BONUS_SPEECH4__;
int16_t BONUSMUSIC          = BONUSMUSIC__;
int16_t BOS1_RECOG          = BOS1_RECOG__;
int16_t BOS1_WALK           = BOS1_WALK__;
int16_t BOS2_RECOG          = BOS2_RECOG__;
int16_t BOS3_RECOG          = BOS3_RECOG__;
int16_t BOS4_RECOG          = BOS4_RECOG__;
int16_t BOSS4_DEADSPEECH    = BOSS4_DEADSPEECH__;
int16_t BOSS4_FIRSTSEE      = BOSS4_FIRSTSEE__;
int16_t BOSSTALKTODUKE      = BOSSTALKTODUKE__;
int16_t CAPT_RECOG          = CAPT_RECOG__;
int16_t CAT_FIRE            = CAT_FIRE__;
int16_t CHAINGUN_FIRE       = CHAINGUN_FIRE__;
int16_t COMM_RECOG          = COMM_RECOG__;
int16_t DRON_RECOG          = DRON_RECOG__;
int16_t DUKE_CRACK          = DUKE_CRACK__;
int16_t DUKE_CRACK_FIRST    = DUKE_CRACK_FIRST__;
int16_t DUKE_CRACK2         = DUKE_CRACK2__;
int16_t DUKE_DRINKING       = DUKE_DRINKING__;
int16_t DUKE_GASP           = DUKE_GASP__;
int16_t DUKE_GET            = DUKE_GET__;
int16_t DUKE_GETWEAPON2     = DUKE_GETWEAPON2__;
int16_t DUKE_GETWEAPON6     = DUKE_GETWEAPON6__;
int16_t DUKE_GOTHEALTHATLOW = DUKE_GOTHEALTHATLOW__;
int16_t DUKE_GRUNT          = DUKE_GRUNT__;
int16_t DUKE_HARTBEAT       = DUKE_HARTBEAT__;
int16_t DUKE_JETPACK_IDLE   = DUKE_JETPACK_IDLE__;
int16_t DUKE_JETPACK_OFF    = DUKE_JETPACK_OFF__;
int16_t DUKE_JETPACK_ON     = DUKE_JETPACK_ON__;
int16_t DUKE_LAND           = DUKE_LAND__;
int16_t DUKE_LAND_HURT      = DUKE_LAND_HURT__;
int16_t DUKE_LONGTERM_PAIN  = DUKE_LONGTERM_PAIN__;
int16_t DUKE_ONWATER        = DUKE_ONWATER__;
int16_t DUKE_PISSRELIEF     = DUKE_PISSRELIEF__;
int16_t DUKE_SCREAM         = DUKE_SCREAM__;
int16_t DUKE_SEARCH         = DUKE_SEARCH__;
int16_t DUKE_SEARCH2        = DUKE_SEARCH2__;
int16_t DUKE_TAKEPILLS      = DUKE_TAKEPILLS__;
int16_t DUKE_UNDERWATER     = DUKE_UNDERWATER__;
int16_t DUKE_URINATE        = DUKE_URINATE__;
int16_t DUKE_USEMEDKIT      = DUKE_USEMEDKIT__;
int16_t DUKE_WALKINDUCTS    = DUKE_WALKINDUCTS__;
int16_t DUKETALKTOBOSS      = DUKETALKTOBOSS__;
int16_t EARTHQUAKE          = EARTHQUAKE__;
int16_t EJECT_CLIP          = EJECT_CLIP__;
int16_t ELEVATOR_OFF        = ELEVATOR_OFF__;
int16_t ELEVATOR_ON         = ELEVATOR_ON__;
int16_t END_OF_LEVEL_WARN   = END_OF_LEVEL_WARN__;
int16_t ENDSEQVOL2SND1      = ENDSEQVOL2SND1__;
int16_t ENDSEQVOL2SND2      = ENDSEQVOL2SND2__;
int16_t ENDSEQVOL2SND3      = ENDSEQVOL2SND3__;
int16_t ENDSEQVOL2SND4      = ENDSEQVOL2SND4__;
int16_t ENDSEQVOL2SND5      = ENDSEQVOL2SND5__;
int16_t ENDSEQVOL2SND6      = ENDSEQVOL2SND6__;
int16_t ENDSEQVOL2SND7      = ENDSEQVOL2SND7__;
int16_t ENDSEQVOL3SND2      = ENDSEQVOL3SND2__;
int16_t ENDSEQVOL3SND3      = ENDSEQVOL3SND3__;
int16_t ENDSEQVOL3SND4      = ENDSEQVOL3SND4__;
int16_t ENDSEQVOL3SND5      = ENDSEQVOL3SND5__;
int16_t ENDSEQVOL3SND6      = ENDSEQVOL3SND6__;
int16_t ENDSEQVOL3SND7      = ENDSEQVOL3SND7__;
int16_t ENDSEQVOL3SND8      = ENDSEQVOL3SND8__;
int16_t ENDSEQVOL3SND9      = ENDSEQVOL3SND9__;
int16_t EXITMENUSOUND       = EXITMENUSOUND__;
int16_t EXPANDERSHOOT       = EXPANDERSHOOT__;
int16_t FLUSH_TOILET        = FLUSH_TOILET__;
int16_t FLY_BY              = FLY_BY__;
int16_t GENERIC_AMBIENCE17  = GENERIC_AMBIENCE17__;
int16_t GLASS_BREAKING      = GLASS_BREAKING__;
int16_t GLASS_HEAVYBREAK    = GLASS_HEAVYBREAK__;
int16_t INSERT_CLIP         = INSERT_CLIP__;
int16_t INTRO4_1            = INTRO4_1__;
int16_t INTRO4_2            = INTRO4_2__;
int16_t INTRO4_3            = INTRO4_3__;
int16_t INTRO4_4            = INTRO4_4__;
int16_t INTRO4_5            = INTRO4_5__;
int16_t INTRO4_6            = INTRO4_6__;
int16_t INTRO4_B            = INTRO4_B__;
int16_t ITEM_SPLASH         = ITEM_SPLASH__;
int16_t JIBBED_ACTOR5       = JIBBED_ACTOR5__;
int16_t JIBBED_ACTOR6       = JIBBED_ACTOR6__;
int16_t KICK_HIT            = KICK_HIT__;
int16_t LASERTRIP_ARMING    = LASERTRIP_ARMING__;
int16_t LASERTRIP_EXPLODE   = LASERTRIP_EXPLODE__;
int16_t LASERTRIP_ONWALL    = LASERTRIP_ONWALL__;
int16_t LIGHTNING_SLAP      = LIGHTNING_SLAP__;
int16_t MONITOR_ACTIVE      = MONITOR_ACTIVE__;
int16_t NITEVISION_ONOFF    = NITEVISION_ONOFF__;
int16_t OCTA_RECOG          = OCTA_RECOG__;
int16_t PIG_RECOG           = PIG_RECOG__;
int16_t PIPEBOMB_BOUNCE     = PIPEBOMB_BOUNCE__;
int16_t PIPEBOMB_EXPLODE    = PIPEBOMB_EXPLODE__;
int16_t PISTOL_BODYHIT      = PISTOL_BODYHIT__;
int16_t PISTOL_FIRE         = PISTOL_FIRE__;
int16_t PISTOL_RICOCHET     = PISTOL_RICOCHET__;
int16_t POOLBALLHIT         = POOLBALLHIT__;
int16_t PRED_RECOG          = PRED_RECOG__;
int16_t RATTY               = RATTY__;
int16_t RECO_ATTACK         = RECO_ATTACK__;
int16_t RECO_PAIN           = RECO_PAIN__;
int16_t RECO_RECOG          = RECO_RECOG__;
int16_t RECO_ROAM           = RECO_ROAM__;
int16_t RIPHEADNECK         = RIPHEADNECK__;
int16_t RPG_EXPLODE         = RPG_EXPLODE__;
int16_t RPG_SHOOT           = RPG_SHOOT__;
int16_t SELECT_WEAPON       = SELECT_WEAPON__;
int16_t SHORT_CIRCUIT       = SHORT_CIRCUIT__;
int16_t SHOTGUN_COCK        = SHOTGUN_COCK__;
int16_t SHOTGUN_FIRE        = SHOTGUN_FIRE__;
int16_t SHRINKER_FIRE       = SHRINKER_FIRE__;
int16_t SHRINKER_HIT        = SHRINKER_HIT__;
int16_t SLIM_ATTACK         = SLIM_ATTACK__;
int16_t SLIM_DYING          = SLIM_DYING__;
int16_t SLIM_RECOG          = SLIM_RECOG__;
int16_t SLIM_ROAM           = SLIM_ROAM__;
int16_t SOMETHING_DRIPPING  = SOMETHING_DRIPPING__;
int16_t SOMETHINGFROZE      = SOMETHINGFROZE__;
int16_t SOMETHINGHITFORCE   = SOMETHINGHITFORCE__;
int16_t SQUISHED            = SQUISHED__;
int16_t SUBWAY              = SUBWAY__;
int16_t SWITCH_ON           = SWITCH_ON__;
int16_t TELEPORTER          = TELEPORTER__;
int16_t THUD                = THUD__;
int16_t THUNDER             = THUNDER__;
int16_t TURR_RECOG          = TURR_RECOG__;
int16_t VENT_BUST           = VENT_BUST__;
int16_t VOL4ENDSND1         = VOL4ENDSND1__;
int16_t VOL4ENDSND2         = VOL4ENDSND2__;
int16_t WAR_AMBIENCE2       = WAR_AMBIENCE2__;
int16_t WHIPYOURASS         = WHIPYOURASS__;
int16_t WIERDSHOT_FLY       = WIERDSHOT_FLY__;
int16_t WIND_AMBIENCE       = WIND_AMBIENCE__;
int16_t WIND_REPEAT         = WIND_REPEAT__;
int16_t FLAMETHROWER_INTRO  = FLAMETHROWER_INTRO__;
int16_t FLAMETHROWER_LOOP   = FLAMETHROWER_LOOP__;
int16_t FLAMETHROWER_END    = FLAMETHROWER_END__;
int16_t E5L7_DUKE_QUIT_YOU  = E5L7_DUKE_QUIT_YOU__;

int16_t ACCESS_ICON         = ACCESS_ICON__;
int16_t ACCESSCARD          = ACCESSCARD__;
int16_t ACCESSSWITCH        = ACCESSSWITCH__;
int16_t ACCESSSWITCH2       = ACCESSSWITCH2__;
int16_t ACTIVATOR           = ACTIVATOR__;
int16_t ACTIVATORLOCKED     = ACTIVATORLOCKED__;
int16_t AIRTANK             = AIRTANK__;
int16_t AIRTANK_ICON        = AIRTANK_ICON__;
int16_t ALIENSWITCH         = ALIENSWITCH__;
int16_t AMMO                = AMMO__;
int16_t AMMOBOX             = AMMOBOX__;
int16_t AMMOLOTS            = AMMOLOTS__;
int16_t ANTENNA             = ANTENNA__;
int16_t APLAYER             = APLAYER__;
int16_t APLAYERTOP          = APLAYERTOP__;
int16_t ARMJIB1             = ARMJIB1__;
int16_t ARROW               = ARROW__;
int16_t ATM                 = ATM__;
int16_t ATMBROKE            = ATMBROKE__;
int16_t ATOMICHEALTH        = ATOMICHEALTH__;
int16_t BANNER              = BANNER__;
int16_t BARBROKE            = BARBROKE__;
int16_t BATTERYAMMO         = BATTERYAMMO__;
int16_t BETASCREEN          = BETASCREEN__;
int16_t BETAVERSION         = BETAVERSION__;
int16_t BGRATE1             = BGRATE1__;
int16_t BIGALPHANUM         = BIGALPHANUM__;
int16_t BIGAPPOS            = BIGAPPOS__;
int16_t BIGCOLIN            = BIGCOLIN__;
int16_t BIGCOMMA            = BIGCOMMA__;
int16_t BIGFORCE            = BIGFORCE__;
int16_t BIGHOLE             = BIGHOLE__;
int16_t BIGORBIT1           = BIGORBIT1__;
int16_t BIGPERIOD           = BIGPERIOD__;
int16_t BIGQ                = BIGQ__;
int16_t BIGSEMI             = BIGSEMI__;
int16_t BIGX_               = BIGX__; // "BIGX" clashes on the Wii?
int16_t BLANKSCREEN         = BLANKSCREEN__;
int16_t BLIMP               = BLIMP__;
int16_t BLOOD               = BLOOD__;
int16_t BLOODPOOL           = BLOODPOOL__;
int16_t BLOODSPLAT1         = BLOODSPLAT1__;
int16_t BLOODSPLAT2         = BLOODSPLAT2__;
int16_t BLOODSPLAT3         = BLOODSPLAT3__;
int16_t BLOODSPLAT4         = BLOODSPLAT4__;
int16_t BLOODYPOLE          = BLOODYPOLE__;
int16_t BOLT1               = BOLT1__;
int16_t BONUSSCREEN         = BONUSSCREEN__;
int16_t BOOT_ICON           = BOOT_ICON__;
int16_t BOOTS               = BOOTS__;
int16_t BORNTOBEWILDSCREEN  = BORNTOBEWILDSCREEN__;
int16_t BOSS1               = BOSS1__;
int16_t BOSS1LOB            = BOSS1LOB__;
int16_t BOSS1SHOOT          = BOSS1SHOOT__;
int16_t BOSS1STAYPUT        = BOSS1STAYPUT__;
int16_t BOSS2               = BOSS2__;
int16_t BOSS3               = BOSS3__;
int16_t BOSS4               = BOSS4__;
int16_t BOSS4STAYPUT        = BOSS4STAYPUT__;
int16_t BOSSTOP             = BOSSTOP__;
int16_t BOTTLE1             = BOTTLE1__;
int16_t BOTTLE10            = BOTTLE10__;
int16_t BOTTLE11            = BOTTLE11__;
int16_t BOTTLE12            = BOTTLE12__;
int16_t BOTTLE13            = BOTTLE13__;
int16_t BOTTLE14            = BOTTLE14__;
int16_t BOTTLE15            = BOTTLE15__;
int16_t BOTTLE16            = BOTTLE16__;
int16_t BOTTLE17            = BOTTLE17__;
int16_t BOTTLE18            = BOTTLE18__;
int16_t BOTTLE19            = BOTTLE19__;
int16_t BOTTLE2             = BOTTLE2__;
int16_t BOTTLE3             = BOTTLE3__;
int16_t BOTTLE4             = BOTTLE4__;
int16_t BOTTLE5             = BOTTLE5__;
int16_t BOTTLE6             = BOTTLE6__;
int16_t BOTTLE7             = BOTTLE7__;
int16_t BOTTLE8             = BOTTLE8__;
int16_t BOTTOMSTATUSBAR     = BOTTOMSTATUSBAR__;
int16_t BOUNCEMINE          = BOUNCEMINE__;
int16_t BOX                 = BOX__;
int16_t BPANNEL1            = BPANNEL1__;
int16_t BPANNEL3            = BPANNEL3__;
int16_t BROKEFIREHYDRENT    = BROKEFIREHYDRENT__;
int16_t BROKEHYDROPLANT     = BROKEHYDROPLANT__;
int16_t BROKENCHAIR         = BROKENCHAIR__;
int16_t BULLETHOLE          = BULLETHOLE__;
int16_t BURNING             = BURNING__;
int16_t BURNING2            = BURNING2__;
int16_t CACTUS              = CACTUS__;
int16_t CACTUSBROKE         = CACTUSBROKE__;
int16_t CAMCORNER           = CAMCORNER__;
int16_t CAMERA1             = CAMERA1__;
int16_t CAMERALIGHT         = CAMERALIGHT__;
int16_t CAMERAPOLE          = CAMERAPOLE__;
int16_t CAMLIGHT            = CAMLIGHT__;
int16_t CANWITHSOMETHING    = CANWITHSOMETHING__;
int16_t CANWITHSOMETHING2   = CANWITHSOMETHING2__;
int16_t CANWITHSOMETHING3   = CANWITHSOMETHING3__;
int16_t CANWITHSOMETHING4   = CANWITHSOMETHING4__;
int16_t CEILINGSTEAM        = CEILINGSTEAM__;
int16_t CHAINGUN            = CHAINGUN__;
int16_t CHAINGUNSPRITE      = CHAINGUNSPRITE__;
int16_t CHAIR1              = CHAIR1__;
int16_t CHAIR2              = CHAIR2__;
int16_t CHAIR3              = CHAIR3__;
int16_t CIRCLEPANNEL        = CIRCLEPANNEL__;
int16_t CIRCLEPANNELBROKE   = CIRCLEPANNELBROKE__;
int16_t CLOUDYOCEAN         = CLOUDYOCEAN__;
int16_t CLOUDYSKIES         = CLOUDYSKIES__;
int16_t COLA                = COLA__;
int16_t COLAMACHINE         = COLAMACHINE__;
int16_t COMMANDER           = COMMANDER__;
int16_t COMMANDERSTAYPUT    = COMMANDERSTAYPUT__;
int16_t CONE                = CONE__;
int16_t COOLEXPLOSION1      = COOLEXPLOSION1__;
int16_t CRACK1              = CRACK1__;
int16_t CRACK2              = CRACK2__;
int16_t CRACK3              = CRACK3__;
int16_t CRACK4              = CRACK4__;
int16_t CRACKKNUCKLES       = CRACKKNUCKLES__;
int16_t CRANE               = CRANE__;
int16_t CRANEPOLE           = CRANEPOLE__;
int16_t CROSSHAIR           = CROSSHAIR__;
int16_t CRYSTALAMMO         = CRYSTALAMMO__;
int16_t CYCLER              = CYCLER__;
int16_t DEVISTATOR          = DEVISTATOR__;
int16_t DEVISTATORAMMO      = DEVISTATORAMMO__;
int16_t DEVISTATORSPRITE    = DEVISTATORSPRITE__;
int16_t DIGITALNUM          = DIGITALNUM__;
int16_t DIPSWITCH           = DIPSWITCH__;
int16_t DIPSWITCH2          = DIPSWITCH2__;
int16_t DIPSWITCH3          = DIPSWITCH3__;
int16_t DOLPHIN1            = DOLPHIN1__;
int16_t DOLPHIN2            = DOLPHIN2__;
int16_t DOMELITE            = DOMELITE__;
int16_t DOORSHOCK           = DOORSHOCK__;
int16_t DOORTILE1           = DOORTILE1__;
int16_t DOORTILE10          = DOORTILE10__;
int16_t DOORTILE11          = DOORTILE11__;
int16_t DOORTILE12          = DOORTILE12__;
int16_t DOORTILE14          = DOORTILE14__;
int16_t DOORTILE15          = DOORTILE15__;
int16_t DOORTILE16          = DOORTILE16__;
int16_t DOORTILE17          = DOORTILE17__;
int16_t DOORTILE18          = DOORTILE18__;
int16_t DOORTILE19          = DOORTILE19__;
int16_t DOORTILE2           = DOORTILE2__;
int16_t DOORTILE20          = DOORTILE20__;
int16_t DOORTILE21          = DOORTILE21__;
int16_t DOORTILE22          = DOORTILE22__;
int16_t DOORTILE23          = DOORTILE23__;
int16_t DOORTILE3           = DOORTILE3__;
int16_t DOORTILE4           = DOORTILE4__;
int16_t DOORTILE5           = DOORTILE5__;
int16_t DOORTILE6           = DOORTILE6__;
int16_t DOORTILE7           = DOORTILE7__;
int16_t DOORTILE8           = DOORTILE8__;
int16_t DOORTILE9           = DOORTILE9__;
int16_t DREALMS             = DREALMS__;
int16_t DRONE               = DRONE__;
int16_t DUCK                = DUCK__;
int16_t DUKECAR             = DUKECAR__;
int16_t DUKEGUN             = DUKEGUN__;
int16_t DUKELEG             = DUKELEG__;
int16_t DUKELYINGDEAD       = DUKELYINGDEAD__;
int16_t DUKENUKEM           = DUKENUKEM__;
int16_t DUKETAG             = DUKETAG__;
int16_t DUKETORSO           = DUKETORSO__;
int16_t EGG                 = EGG__;
int16_t ENDALPHANUM         = ENDALPHANUM__;
int16_t EXPLODINGBARREL     = EXPLODINGBARREL__;
int16_t EXPLODINGBARREL2    = EXPLODINGBARREL2__;
int16_t EXPLOSION2          = EXPLOSION2__;
int16_t EXPLOSION2BOT       = EXPLOSION2BOT__;
int16_t F1HELP              = F1HELP__;
int16_t FANSHADOW           = FANSHADOW__;
int16_t FANSHADOWBROKE      = FANSHADOWBROKE__;
int16_t FANSPRITE           = FANSPRITE__;
int16_t FANSPRITEBROKE      = FANSPRITEBROKE__;
int16_t FECES               = FECES__;
int16_t FEM1                = FEM1__;
int16_t FEM10               = FEM10__;
int16_t FEM2                = FEM2__;
int16_t FEM3                = FEM3__;
int16_t FEM4                = FEM4__;
int16_t FEM5                = FEM5__;
int16_t FEM6                = FEM6__;
int16_t FEM6PAD             = FEM6PAD__;
int16_t FEM7                = FEM7__;
int16_t FEM8                = FEM8__;
int16_t FEM9                = FEM9__;
int16_t FEMMAG1             = FEMMAG1__;
int16_t FEMMAG2             = FEMMAG2__;
int16_t FEMPIC1             = FEMPIC1__;
int16_t FEMPIC2             = FEMPIC2__;
int16_t FEMPIC3             = FEMPIC3__;
int16_t FEMPIC4             = FEMPIC4__;
int16_t FEMPIC5             = FEMPIC5__;
int16_t FEMPIC6             = FEMPIC6__;
int16_t FEMPIC7             = FEMPIC7__;
int16_t FETUS               = FETUS__;
int16_t FETUSBROKE          = FETUSBROKE__;
int16_t FETUSJIB            = FETUSJIB__;
int16_t FIRE                = FIRE__;
int16_t FIRE2               = FIRE2__;
int16_t FIREBARREL          = FIREBARREL__;
int16_t FIREEXT             = FIREEXT__;
int16_t FIRELASER           = FIRELASER__;
int16_t FIREVASE            = FIREVASE__;
int16_t FIRSTAID            = FIRSTAID__;
int16_t FIRSTAID_ICON       = FIRSTAID_ICON__;
int16_t FIRSTGUN            = FIRSTGUN__;
int16_t FIRSTGUNRELOAD      = FIRSTGUNRELOAD__;
int16_t FIRSTGUNSPRITE      = FIRSTGUNSPRITE__;
int16_t FIST                = FIST__;
int16_t FLOORFLAME          = FLOORFLAME__;
int16_t FLOORPLASMA         = FLOORPLASMA__;
int16_t FLOORSLIME          = FLOORSLIME__;
int16_t FOF                 = FOF__;
int16_t FOODOBJECT16        = FOODOBJECT16__;
int16_t FOOTPRINTS          = FOOTPRINTS__;
int16_t FOOTPRINTS2         = FOOTPRINTS2__;
int16_t FOOTPRINTS3         = FOOTPRINTS3__;
int16_t FOOTPRINTS4         = FOOTPRINTS4__;
int16_t FORCERIPPLE         = FORCERIPPLE__;
int16_t FORCESPHERE         = FORCESPHERE__;
int16_t FRAGBAR             = FRAGBAR__;
int16_t FRAMEEFFECT1        = FRAMEEFFECT1__;
int16_t FRAMEEFFECT1_13     = FRAMEEFFECT1_13__;
int16_t FRANKENSTINESWITCH  = FRANKENSTINESWITCH__;
int16_t FREEZE              = FREEZE__;
int16_t FREEZEAMMO          = FREEZEAMMO__;
int16_t FREEZEBLAST         = FREEZEBLAST__;
int16_t FREEZESPRITE        = FREEZESPRITE__;
int16_t FUELPOD             = FUELPOD__;
int16_t GENERICPOLE         = GENERICPOLE__;
int16_t GENERICPOLE2        = GENERICPOLE2__;
int16_t GLASS               = GLASS__;
int16_t GLASS2              = GLASS2__;
int16_t GLASSPIECES         = GLASSPIECES__;
int16_t GPSPEED             = GPSPEED__;
int16_t GRATE1              = GRATE1__;
int16_t GREENSLIME          = GREENSLIME__;
int16_t GROWAMMO            = GROWAMMO__;
int16_t GROWSPARK           = GROWSPARK__;
int16_t GROWSPRITEICON      = GROWSPRITEICON__;
int16_t HANDHOLDINGACCESS   = HANDHOLDINGACCESS__;
int16_t HANDHOLDINGLASER    = HANDHOLDINGLASER__;
int16_t HANDPRINTSWITCH     = HANDPRINTSWITCH__;
int16_t HANDREMOTE          = HANDREMOTE__;
int16_t HANDSWITCH          = HANDSWITCH__;
int16_t HANDTHROW           = HANDTHROW__;
int16_t HANGLIGHT           = HANGLIGHT__;
int16_t HBOMBAMMO           = HBOMBAMMO__;
int16_t HEADJIB1            = HEADJIB1__;
int16_t HEALTHBOX           = HEALTHBOX__;
int16_t HEAT_ICON           = HEAT_ICON__;
int16_t HEATSENSOR          = HEATSENSOR__;
int16_t HEAVYHBOMB          = HEAVYHBOMB__;
int16_t HELECOPT            = HELECOPT__;
int16_t HOLODUKE            = HOLODUKE__;
int16_t HOLODUKE_ICON       = HOLODUKE_ICON__;
int16_t HORSEONSIDE         = HORSEONSIDE__;
int16_t HOTMEAT             = HOTMEAT__;
int16_t HURTRAIL            = HURTRAIL__;
int16_t HYDRENT             = HYDRENT__;
int16_t HYDROPLANT          = HYDROPLANT__;
int16_t INDY                = INDY__;
int16_t INGAMEDUKETHREEDEE  = INGAMEDUKETHREEDEE__;
int16_t INNERJAW            = INNERJAW__;
int16_t INVENTORYBOX        = INVENTORYBOX__;
int16_t IVUNIT              = IVUNIT__;
int16_t JETPACK             = JETPACK__;
int16_t JETPACK_ICON        = JETPACK_ICON__;
int16_t JIBS1               = JIBS1__;
int16_t JIBS2               = JIBS2__;
int16_t JIBS3               = JIBS3__;
int16_t JIBS4               = JIBS4__;
int16_t JIBS5               = JIBS5__;
int16_t JIBS6               = JIBS6__;
int16_t JURYGUY             = JURYGUY__;
int16_t KILLSICON           = KILLSICON__;
int16_t KNEE                = KNEE__;
int16_t LA                  = LA__;
int16_t LASERLINE           = LASERLINE__;
int16_t LASERSITE           = LASERSITE__;
int16_t LEGJIB1             = LEGJIB1__;
int16_t LETTER              = LETTER__;
int16_t LIGHTSWITCH         = LIGHTSWITCH__;
int16_t LIGHTSWITCH2        = LIGHTSWITCH2__;
int16_t LIZMAN              = LIZMAN__;
int16_t LIZMANARM1          = LIZMANARM1__;
int16_t LIZMANFEEDING       = LIZMANFEEDING__;
int16_t LIZMANHEAD1         = LIZMANHEAD1__;
int16_t LIZMANJUMP          = LIZMANJUMP__;
int16_t LIZMANLEG1          = LIZMANLEG1__;
int16_t LIZMANSPITTING      = LIZMANSPITTING__;
int16_t LIZMANSTAYPUT       = LIZMANSTAYPUT__;
int16_t LIZTROOP            = LIZTROOP__;
int16_t LIZTROOPDUCKING     = LIZTROOPDUCKING__;
int16_t LIZTROOPJETPACK     = LIZTROOPJETPACK__;
int16_t LIZTROOPJUSTSIT     = LIZTROOPJUSTSIT__;
int16_t LIZTROOPONTOILET    = LIZTROOPONTOILET__;
int16_t LIZTROOPRUNNING     = LIZTROOPRUNNING__;
int16_t LIZTROOPSHOOT       = LIZTROOPSHOOT__;
int16_t LIZTROOPSTAYPUT     = LIZTROOPSTAYPUT__;
int16_t LOADSCREEN          = LOADSCREEN__;
int16_t LOCATORS            = LOCATORS__;
int16_t LOCKSWITCH1         = LOCKSWITCH1__;
int16_t LOOGIE              = LOOGIE__;
int16_t LUKE                = LUKE__;
int16_t MAIL                = MAIL__;
int16_t MAN                 = MAN__;
int16_t MAN2                = MAN2__;
int16_t MASKWALL1           = MASKWALL1__;
int16_t MASKWALL10          = MASKWALL10__;
int16_t MASKWALL11          = MASKWALL11__;
int16_t MASKWALL12          = MASKWALL12__;
int16_t MASKWALL13          = MASKWALL13__;
int16_t MASKWALL14          = MASKWALL14__;
int16_t MASKWALL15          = MASKWALL15__;
int16_t MASKWALL2           = MASKWALL2__;
int16_t MASKWALL3           = MASKWALL3__;
int16_t MASKWALL4           = MASKWALL4__;
int16_t MASKWALL5           = MASKWALL5__;
int16_t MASKWALL6           = MASKWALL6__;
int16_t MASKWALL7           = MASKWALL7__;
int16_t MASKWALL8           = MASKWALL8__;
int16_t MASKWALL9           = MASKWALL9__;
int16_t MASTERSWITCH        = MASTERSWITCH__;
int16_t MENUBAR             = MENUBAR__;
int16_t MENUSCREEN          = MENUSCREEN__;
int16_t MIKE                = MIKE__;
int16_t MINIFONT            = MINIFONT__;
int16_t MIRROR              = MIRROR__;
int16_t MIRRORBROKE         = MIRRORBROKE__;
int16_t MONEY               = MONEY__;
int16_t MONK                = MONK__;
int16_t MOONSKY1            = MOONSKY1__;
int16_t MORTER              = MORTER__;
int16_t MOVIECAMERA         = MOVIECAMERA__;
int16_t MULTISWITCH         = MULTISWITCH__;
int16_t MUSICANDSFX         = MUSICANDSFX__;
int16_t NAKED1              = NAKED1__;
int16_t NATURALLIGHTNING    = NATURALLIGHTNING__;
int16_t NEON1               = NEON1__;
int16_t NEON2               = NEON2__;
int16_t NEON3               = NEON3__;
int16_t NEON4               = NEON4__;
int16_t NEON5               = NEON5__;
int16_t NEON6               = NEON6__;
int16_t NEWBEAST            = NEWBEAST__;
int16_t NEWBEASTSTAYPUT     = NEWBEASTSTAYPUT__;
int16_t NUKEBARREL          = NUKEBARREL__;
int16_t NUKEBARRELDENTED    = NUKEBARRELDENTED__;
int16_t NUKEBARRELLEAKED    = NUKEBARRELLEAKED__;
int16_t NUKEBUTTON          = NUKEBUTTON__;
int16_t OCEANSPRITE1        = OCEANSPRITE1__;
int16_t OCEANSPRITE2        = OCEANSPRITE2__;
int16_t OCEANSPRITE3        = OCEANSPRITE3__;
int16_t OCEANSPRITE4        = OCEANSPRITE4__;
int16_t OCEANSPRITE5        = OCEANSPRITE5__;
int16_t OCTABRAIN           = OCTABRAIN__;
int16_t OCTABRAINSTAYPUT    = OCTABRAINSTAYPUT__;
int16_t OJ                  = OJ__;
int16_t OOZ                 = OOZ__;
int16_t OOZ2                = OOZ2__;
int16_t OOZFILTER           = OOZFILTER__;
int16_t ORDERING            = ORDERING__;
int16_t ORGANTIC            = ORGANTIC__;
int16_t PANNEL1             = PANNEL1__;
int16_t PANNEL2             = PANNEL2__;
int16_t PANNEL3             = PANNEL3__;
int16_t PAPER               = PAPER__;
int16_t PIGCOP              = PIGCOP__;
int16_t PIGCOPDIVE          = PIGCOPDIVE__;
int16_t PIGCOPSTAYPUT       = PIGCOPSTAYPUT__;
int16_t PIPE1               = PIPE1__;
int16_t PIPE1B              = PIPE1B__;
int16_t PIPE2               = PIPE2__;
int16_t PIPE2B              = PIPE2B__;
int16_t PIPE3               = PIPE3__;
int16_t PIPE3B              = PIPE3B__;
int16_t PIPE4               = PIPE4__;
int16_t PIPE4B              = PIPE4B__;
int16_t PIPE5               = PIPE5__;
int16_t PIPE5B              = PIPE5B__;
int16_t PIPE6               = PIPE6__;
int16_t PIPE6B              = PIPE6B__;
int16_t PLAYERONWATER       = PLAYERONWATER__;
int16_t PLUG                = PLUG__;
int16_t PLUTOPAKSPRITE      = PLUTOPAKSPRITE__;
int16_t POCKET              = POCKET__;
int16_t PODFEM1             = PODFEM1__;
int16_t POT1                = POT1__;
int16_t POT2                = POT2__;
int16_t POT3                = POT3__;
int16_t POWERSWITCH1        = POWERSWITCH1__;
int16_t POWERSWITCH2        = POWERSWITCH2__;
int16_t PUKE                = PUKE__;
int16_t PULLSWITCH          = PULLSWITCH__;
int16_t PURPLELAVA          = PURPLELAVA__;
int16_t QUEBALL             = QUEBALL__;
int16_t RADIUSEXPLOSION     = RADIUSEXPLOSION__;
int16_t RAT                 = RAT__;
int16_t REACTOR             = REACTOR__;
int16_t REACTOR2            = REACTOR2__;
int16_t REACTOR2BURNT       = REACTOR2BURNT__;
int16_t REACTOR2SPARK       = REACTOR2SPARK__;
int16_t REACTORBURNT        = REACTORBURNT__;
int16_t REACTORSPARK        = REACTORSPARK__;
int16_t RECON               = RECON__;
int16_t RESPAWN             = RESPAWN__;
int16_t RESPAWNMARKERGREEN  = RESPAWNMARKERGREEN__;
int16_t RESPAWNMARKERRED    = RESPAWNMARKERRED__;
int16_t RESPAWNMARKERYELLOW = RESPAWNMARKERYELLOW__;
int16_t ROTATEGUN           = ROTATEGUN__;
int16_t RPG                 = RPG__;
int16_t RPGAMMO             = RPGAMMO__;
int16_t RPGGUN              = RPGGUN__;
int16_t RPGSPRITE           = RPGSPRITE__;
int16_t RUBBERCAN           = RUBBERCAN__;
int16_t SATELITE            = SATELITE__;
int16_t SCALE               = SCALE__;
int16_t SCRAP1              = SCRAP1__;
int16_t SCRAP2              = SCRAP2__;
int16_t SCRAP3              = SCRAP3__;
int16_t SCRAP4              = SCRAP4__;
int16_t SCRAP5              = SCRAP5__;
int16_t SCRAP6              = SCRAP6__;
int16_t SCREENBREAK1        = SCREENBREAK1__;
int16_t SCREENBREAK10       = SCREENBREAK10__;
int16_t SCREENBREAK11       = SCREENBREAK11__;
int16_t SCREENBREAK12       = SCREENBREAK12__;
int16_t SCREENBREAK13       = SCREENBREAK13__;
int16_t SCREENBREAK14       = SCREENBREAK14__;
int16_t SCREENBREAK15       = SCREENBREAK15__;
int16_t SCREENBREAK16       = SCREENBREAK16__;
int16_t SCREENBREAK17       = SCREENBREAK17__;
int16_t SCREENBREAK18       = SCREENBREAK18__;
int16_t SCREENBREAK19       = SCREENBREAK19__;
int16_t SCREENBREAK2        = SCREENBREAK2__;
int16_t SCREENBREAK3        = SCREENBREAK3__;
int16_t SCREENBREAK4        = SCREENBREAK4__;
int16_t SCREENBREAK5        = SCREENBREAK5__;
int16_t SCREENBREAK6        = SCREENBREAK6__;
int16_t SCREENBREAK7        = SCREENBREAK7__;
int16_t SCREENBREAK8        = SCREENBREAK8__;
int16_t SCREENBREAK9        = SCREENBREAK9__;
int16_t SCUBAMASK           = SCUBAMASK__;
int16_t SECTOREFFECTOR      = SECTOREFFECTOR__;
int16_t SEENINE             = SEENINE__;
int16_t SEENINEDEAD         = SEENINEDEAD__;
int16_t SELECTDIR           = SELECTDIR__;
int16_t SHARK               = SHARK__;
int16_t SHELL               = SHELL__;
int16_t SHIELD              = SHIELD__;
int16_t SHOTGUN             = SHOTGUN__;
int16_t SHOTGUNAMMO         = SHOTGUNAMMO__;
int16_t SHOTGUNSHELL        = SHOTGUNSHELL__;
int16_t SHOTGUNSPRITE       = SHOTGUNSPRITE__;
int16_t SHOTSPARK1          = SHOTSPARK1__;
int16_t SHRINKER            = SHRINKER__;
int16_t SHRINKEREXPLOSION   = SHRINKEREXPLOSION__;
int16_t SHRINKERSPRITE      = SHRINKERSPRITE__;
int16_t SHRINKSPARK         = SHRINKSPARK__;
int16_t SIDEBOLT1           = SIDEBOLT1__;
int16_t SIGN1               = SIGN1__;
int16_t SIGN2               = SIGN2__;
int16_t SIXPAK              = SIXPAK__;
int16_t SLIDEBAR            = SLIDEBAR__;
int16_t SLOTDOOR            = SLOTDOOR__;
int16_t SMALLFNTCURSOR      = SMALLFNTCURSOR__;
int16_t SMALLSMOKE          = SMALLSMOKE__;
int16_t SOLARPANNEL         = SOLARPANNEL__;
int16_t SPACEDOORSWITCH     = SPACEDOORSWITCH__;
int16_t SPACELIGHTSWITCH    = SPACELIGHTSWITCH__;
int16_t SPACEMARINE         = SPACEMARINE__;
int16_t SPEAKER             = SPEAKER__;
int16_t SPINNINGNUKEICON    = SPINNINGNUKEICON__;
int16_t SPIT                = SPIT__;
int16_t SPOTLITE            = SPOTLITE__;
int16_t STAINGLASS1         = STAINGLASS1__;
int16_t STALL               = STALL__;
int16_t STALLBROKE          = STALLBROKE__;
int16_t STARTALPHANUM       = STARTALPHANUM__;
int16_t STATIC              = STATIC__;
int16_t STATUE              = STATUE__;
int16_t STATUEFLASH         = STATUEFLASH__;
int16_t STEAM               = STEAM__;
int16_t STEROIDS            = STEROIDS__;
int16_t STEROIDS_ICON       = STEROIDS_ICON__;
int16_t STRIPEBALL          = STRIPEBALL__;
int16_t SUSHIPLATE1         = SUSHIPLATE1__;
int16_t SUSHIPLATE2         = SUSHIPLATE2__;
int16_t SUSHIPLATE3         = SUSHIPLATE3__;
int16_t SUSHIPLATE4         = SUSHIPLATE4__;
int16_t SUSHIPLATE5         = SUSHIPLATE5__;
int16_t TAMPON              = TAMPON__;
int16_t TANK                = TANK__;
int16_t TARGET              = TARGET__;
int16_t TECHLIGHT2          = TECHLIGHT2__;
int16_t TECHLIGHT4          = TECHLIGHT4__;
int16_t TECHLIGHTBUST2      = TECHLIGHTBUST2__;
int16_t TECHLIGHTBUST4      = TECHLIGHTBUST4__;
int16_t TECHSWITCH          = TECHSWITCH__;
int16_t TENSCREEN           = TENSCREEN__;
int16_t TEXTBOX             = TEXTBOX__;
int16_t TEXTSTORY           = TEXTSTORY__;
int16_t THREEBYFIVE         = THREEBYFIVE__;
int16_t THREEDEE            = THREEDEE__;
int16_t TIP                 = TIP__;
int16_t TIRE                = TIRE__;
int16_t TOILET              = TOILET__;
int16_t TOILETBROKE         = TOILETBROKE__;
int16_t TOILETWATER         = TOILETWATER__;
int16_t TONGUE              = TONGUE__;
int16_t TOUCHPLATE          = TOUCHPLATE__;
int16_t TOUGHGAL            = TOUGHGAL__;
int16_t TRANSPORTERBEAM     = TRANSPORTERBEAM__;
int16_t TRANSPORTERSTAR     = TRANSPORTERSTAR__;
int16_t TRASH               = TRASH__;
int16_t TREE1               = TREE1__;
int16_t TREE2               = TREE2__;
int16_t TRIPBOMB            = TRIPBOMB__;
int16_t TRIPBOMBSPRITE      = TRIPBOMBSPRITE__;
int16_t TRIPODCAMERA        = TRIPODCAMERA__;
int16_t VACUUM              = VACUUM__;
int16_t VASE                = VASE__;
int16_t VENDMACHINE         = VENDMACHINE__;
int16_t VICTORY1            = VICTORY1__;
int16_t VIEWBORDER          = VIEWBORDER__;
int16_t VIEWSCREEN          = VIEWSCREEN__;
int16_t VIEWSCREEN2         = VIEWSCREEN2__;
int16_t W_FORCEFIELD        = W_FORCEFIELD__;
int16_t W_HITTECHWALL1      = W_HITTECHWALL1__;
int16_t W_HITTECHWALL10     = W_HITTECHWALL10__;
int16_t W_HITTECHWALL15     = W_HITTECHWALL15__;
int16_t W_HITTECHWALL16     = W_HITTECHWALL16__;
int16_t W_HITTECHWALL2      = W_HITTECHWALL2__;
int16_t W_HITTECHWALL3      = W_HITTECHWALL3__;
int16_t W_HITTECHWALL4      = W_HITTECHWALL4__;
int16_t W_MILKSHELF         = W_MILKSHELF__;
int16_t W_MILKSHELFBROKE    = W_MILKSHELFBROKE__;
int16_t W_NUMBERS           = W_NUMBERS__;
int16_t W_SCREENBREAK       = W_SCREENBREAK__;
int16_t W_TECHWALL1         = W_TECHWALL1__;
int16_t W_TECHWALL10        = W_TECHWALL10__;
int16_t W_TECHWALL11        = W_TECHWALL11__;
int16_t W_TECHWALL12        = W_TECHWALL12__;
int16_t W_TECHWALL13        = W_TECHWALL13__;
int16_t W_TECHWALL14        = W_TECHWALL14__;
int16_t W_TECHWALL15        = W_TECHWALL15__;
int16_t W_TECHWALL16        = W_TECHWALL16__;
int16_t W_TECHWALL2         = W_TECHWALL2__;
int16_t W_TECHWALL3         = W_TECHWALL3__;
int16_t W_TECHWALL4         = W_TECHWALL4__;
int16_t W_TECHWALL5         = W_TECHWALL5__;
int16_t W_TECHWALL6         = W_TECHWALL6__;
int16_t W_TECHWALL7         = W_TECHWALL7__;
int16_t W_TECHWALL8         = W_TECHWALL8__;
int16_t W_TECHWALL9         = W_TECHWALL9__;
int16_t WAITTOBESEATED      = WAITTOBESEATED__;
int16_t WALLBLOOD1          = WALLBLOOD1__;
int16_t WALLBLOOD2          = WALLBLOOD2__;
int16_t WALLBLOOD3          = WALLBLOOD3__;
int16_t WALLBLOOD4          = WALLBLOOD4__;
int16_t WALLBLOOD5          = WALLBLOOD5__;
int16_t WALLBLOOD7          = WALLBLOOD7__;
int16_t WALLBLOOD8          = WALLBLOOD8__;
int16_t WALLLIGHT1          = WALLLIGHT1__;
int16_t WALLLIGHT2          = WALLLIGHT2__;
int16_t WALLLIGHT3          = WALLLIGHT3__;
int16_t WALLLIGHT4          = WALLLIGHT4__;
int16_t WALLLIGHTBUST1      = WALLLIGHTBUST1__;
int16_t WALLLIGHTBUST2      = WALLLIGHTBUST2__;
int16_t WALLLIGHTBUST3      = WALLLIGHTBUST3__;
int16_t WALLLIGHTBUST4      = WALLLIGHTBUST4__;
int16_t WATERBUBBLE         = WATERBUBBLE__;
int16_t WATERBUBBLEMAKER    = WATERBUBBLEMAKER__;
int16_t WATERDRIP           = WATERDRIP__;
int16_t WATERDRIPSPLASH     = WATERDRIPSPLASH__;
int16_t WATERFOUNTAIN       = WATERFOUNTAIN__;
int16_t WATERFOUNTAINBROKE  = WATERFOUNTAINBROKE__;
int16_t WATERSPLASH2        = WATERSPLASH2__;
int16_t WATERTILE2          = WATERTILE2__;
int16_t WEATHERWARN         = WEATHERWARN__;
int16_t WINDOWBORDER1       = WINDOWBORDER1__;
int16_t WINDOWBORDER2       = WINDOWBORDER2__;
int16_t WOMAN               = WOMAN__;
int16_t WOODENHORSE         = WOODENHORSE__;
int16_t XXXSTACY            = XXXSTACY__;
int16_t WIDESCREENSTATUSBAR = WIDESCREENSTATUSBAR__;
int16_t RPGGUNWIDE          = RPGGUNWIDE__;
int16_t FIRSTGUNRELOADWIDE  = FIRSTGUNRELOADWIDE__;
int16_t FREEZEWIDE          = FREEZEWIDE__;
int16_t FREEZEFIREWIDE      = FREEZEFIREWIDE__;
int16_t SHRINKERWIDE        = SHRINKERWIDE__;
int16_t CRACKKNUCKLESWIDE   = CRACKKNUCKLESWIDE__;
int16_t FLAMETHROWERSPRITE  = FLAMETHROWERSPRITE__;
int16_t FLAMETHROWERAMMO    = FLAMETHROWERAMMO__;
int16_t FLAMETHROWER        = FLAMETHROWER__;
int16_t FLAMETHROWERFIRE    = FLAMETHROWERFIRE__;
int16_t FLAMETHROWERFLAME   = FLAMETHROWERFLAME__;
int16_t FLAMETHROWERPILOT   = FLAMETHROWERPILOT__;
int16_t FIREBALL            = FIREBALL__;
int16_t ONFIRE              = ONFIRE__;
int16_t ONFIRESMOKE         = ONFIRESMOKE__;
int16_t BURNEDCORPSE        = BURNEDCORPSE__;
int16_t WHISPYSMOKE         = WHISPYSMOKE__;
int16_t FIREFLY             = FIREFLY__;
int16_t FIREFLYSHRINKEFFECT = FIREFLYSHRINKEFFECT__;
int16_t FIREFLYGROWEFFECT   = FIREFLYGROWEFFECT__;
int16_t FIREFLYFLYINGEFFECT = FIREFLYFLYINGEFFECT__;
int16_t BOSS5               = BOSS5__;
int16_t BOSS5STAYPUT        = BOSS5STAYPUT__;
int16_t LAVAPOOL            = LAVAPOOL__;
int16_t LAVASPLASH          = LAVASPLASH__;
int16_t LAVAPOOLBUBBLE      = LAVAPOOLBUBBLE__;
int16_t BOSS2STAYPUT        = BOSS2STAYPUT__;
int16_t BOSS3STAYPUT        = BOSS3STAYPUT__;
int16_t E32_TILE5736        = E32_TILE5736__;
int16_t E32_TILE5737        = E32_TILE5737__;
int16_t E32_TILE5846        = E32_TILE5846__;

static hashtable_t h_names = { 1024, NULL };

void G_ProcessDynamicNameMapping(const char *szLabel, struct dynitem *list, int16_t lValue)
{
    if ((unsigned)lValue >= MAXTILES || !szLabel)
        return;

    int const i = hash_find(&h_names,szLabel);

    if (i>=0)
    {
        struct dynitem *di = &list[i];
#ifdef DEBUGGINGAIDS
        if (g_scriptDebug && di->staticval != lValue)
            OSD_Printf("REMAP %s (%d) --> %d\n", di->str, di->staticval, lValue);
#endif
        if (!Bstrcmp(di->str, szLabel))
            *di->dynvalptr = lValue;
    }
}

void inithashnames(void)
{
    hash_init(&h_names);

    for (int i=0; i < ARRAY_SSIZE(g_dynTileList); i++)
        hash_add(&h_names, g_dynTileList[i].str, i, 0);

    for (int i=0; i < ARRAY_SSIZE(g_dynSoundList); i++)
        hash_add(&h_names, g_dynSoundList[i].str, i, 0);
}

void freehashnames(void)
{
    hash_free(&h_names);
}

#endif

// This is run after all CON defines have been processed to set up the
// dynamic->static tile mapping.
void G_InitDynamicNames(void)
{
#ifdef USE_DNAMES
    inthash_init(&h_dynamictilemap);

    for (auto & i : g_dynTileList)
        tileSetMapping(*(i.dynvalptr), i.staticval);
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

    inthash_init(&h_dsound);

    for (auto & i : g_dynSoundList)
#ifdef USE_DNAMES
        soundSetMapping(*(i.dynvalptr), i.staticval);
#else
        soundSetMapping(i.staticval, i.staticval);
#endif
}
