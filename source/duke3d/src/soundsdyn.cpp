//-------------------------------------------------------------------------
/*
Copyright (C) 2013 EDuke32 developers and contributors

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
#include "sounds.h"
#include "soundsdyn.h"
#include "global.h"

#ifdef DYNSOUNDREMAP_ENABLE
# define DVPTR(x) &x
#else
# define DVPTR(x) NULL
#endif

struct dynitem
{
    const char *str;
    int32_t *dynvalptr;
    const int16_t staticval;
};

static struct dynitem g_dynSoundList[] =
{
    { "ALIEN_SWITCH1",       DVPTR(ALIEN_SWITCH1),      ALIEN_SWITCH1__ },
    { "BIGBANG",             DVPTR(BIGBANG),            BIGBANG__ },
    { "BONUS_SPEECH1",       DVPTR(BONUS_SPEECH1),      BONUS_SPEECH1__ },
    { "BONUS_SPEECH2",       DVPTR(BONUS_SPEECH2),      BONUS_SPEECH2__ },
    { "BONUS_SPEECH3",       DVPTR(BONUS_SPEECH3),      BONUS_SPEECH3__ },
    { "BONUS_SPEECH4",       DVPTR(BONUS_SPEECH4),      BONUS_SPEECH4__ },
    { "BONUSMUSIC",          DVPTR(BONUSMUSIC),         BONUSMUSIC__ },
    { "BOS1_RECOG",          DVPTR(BOS1_RECOG),         BOS1_RECOG__ },
    { "BOS1_WALK",           DVPTR(BOS1_WALK),          BOS1_WALK__ },
    { "BOS2_RECOG",          DVPTR(BOS2_RECOG),         BOS2_RECOG__ },
    { "BOS3_RECOG",          DVPTR(BOS3_RECOG),         BOS3_RECOG__ },
    { "BOS4_RECOG",          DVPTR(BOS4_RECOG),         BOS4_RECOG__ },
    { "BOSS4_DEADSPEECH",    DVPTR(BOSS4_DEADSPEECH),   BOSS4_DEADSPEECH__ },
    { "BOSS4_FIRSTSEE",      DVPTR(BOSS4_FIRSTSEE),     BOSS4_FIRSTSEE__ },
    { "BOSSTALKTODUKE",      DVPTR(BOSSTALKTODUKE),     BOSSTALKTODUKE__ },
    { "CAPT_RECOG",          DVPTR(CAPT_RECOG),         CAPT_RECOG__ },
    { "CAT_FIRE",            DVPTR(CAT_FIRE),           CAT_FIRE__ },
    { "CHAINGUN_FIRE",       DVPTR(CHAINGUN_FIRE),      CHAINGUN_FIRE__ },
    { "COMM_RECOG",          DVPTR(COMM_RECOG),         COMM_RECOG__ },
    { "DRON_RECOG",          DVPTR(DRON_RECOG),         DRON_RECOG__ },
    { "DUKE_CRACK",          DVPTR(DUKE_CRACK),          DUKE_CRACK__ },
    { "DUKE_CRACK_FIRST",    DVPTR(DUKE_CRACK_FIRST),    DUKE_CRACK_FIRST__ },
    { "DUKE_CRACK2",         DVPTR(DUKE_CRACK2),         DUKE_CRACK2__ },
    { "DUKE_DRINKING",       DVPTR(DUKE_DRINKING),       DUKE_DRINKING__ },
    { "DUKE_GASP",           DVPTR(DUKE_GASP),           DUKE_GASP__ },
    { "DUKE_GET",            DVPTR(DUKE_GET),            DUKE_GET__ },
    { "DUKE_GETWEAPON2",     DVPTR(DUKE_GETWEAPON2),     DUKE_GETWEAPON2__ },
    { "DUKE_GETWEAPON6",     DVPTR(DUKE_GETWEAPON6),     DUKE_GETWEAPON6__ },
    { "DUKE_GOTHEALTHATLOW", DVPTR(DUKE_GOTHEALTHATLOW), DUKE_GOTHEALTHATLOW__ },
    { "DUKE_GRUNT",          DVPTR(DUKE_GRUNT),          DUKE_GRUNT__ },
    { "DUKE_HARTBEAT",       DVPTR(DUKE_HARTBEAT),       DUKE_HARTBEAT__ },
    { "DUKE_JETPACK_IDLE",   DVPTR(DUKE_JETPACK_IDLE),   DUKE_JETPACK_IDLE__ },
    { "DUKE_JETPACK_OFF",    DVPTR(DUKE_JETPACK_OFF),    DUKE_JETPACK_OFF__ },
    { "DUKE_JETPACK_ON",     DVPTR(DUKE_JETPACK_ON),     DUKE_JETPACK_ON__ },
    { "DUKE_LAND",           DVPTR(DUKE_LAND),           DUKE_LAND__ },
    { "DUKE_LAND_HURT",      DVPTR(DUKE_LAND_HURT),      DUKE_LAND_HURT__ },
    { "DUKE_LONGTERM_PAIN",  DVPTR(DUKE_LONGTERM_PAIN),  DUKE_LONGTERM_PAIN__ },
    { "DUKE_ONWATER",        DVPTR(DUKE_ONWATER),        DUKE_ONWATER__ },
    { "DUKE_PISSRELIEF",     DVPTR(DUKE_PISSRELIEF),     DUKE_PISSRELIEF__ },
    { "DUKE_SCREAM",         DVPTR(DUKE_SCREAM),         DUKE_SCREAM__ },
    { "DUKE_SEARCH",         DVPTR(DUKE_SEARCH),         DUKE_SEARCH__ },
    { "DUKE_SEARCH2",        DVPTR(DUKE_SEARCH2),        DUKE_SEARCH2__ },
    { "DUKE_TAKEPILLS",      DVPTR(DUKE_TAKEPILLS),      DUKE_TAKEPILLS__ },
    { "DUKE_UNDERWATER",     DVPTR(DUKE_UNDERWATER),     DUKE_UNDERWATER__ },
    { "DUKE_URINATE",        DVPTR(DUKE_URINATE),        DUKE_URINATE__ },
    { "DUKE_USEMEDKIT",      DVPTR(DUKE_USEMEDKIT),      DUKE_USEMEDKIT__ },
    { "DUKE_WALKINDUCTS",    DVPTR(DUKE_WALKINDUCTS),    DUKE_WALKINDUCTS__ },
    { "DUKETALKTOBOSS",      DVPTR(DUKETALKTOBOSS),      DUKETALKTOBOSS__ },
    { "EARTHQUAKE",          DVPTR(EARTHQUAKE),         EARTHQUAKE__ },
    { "EJECT_CLIP",          DVPTR(EJECT_CLIP),         EJECT_CLIP__ },
    { "ELEVATOR_OFF",        DVPTR(ELEVATOR_OFF),       ELEVATOR_OFF__ },
    { "ELEVATOR_ON",         DVPTR(ELEVATOR_ON),        ELEVATOR_ON__ },
    { "END_OF_LEVEL_WARN",   DVPTR(END_OF_LEVEL_WARN),  END_OF_LEVEL_WARN__ },
    { "ENDSEQVOL2SND1",      DVPTR(ENDSEQVOL2SND1),     ENDSEQVOL2SND1__ },
    { "ENDSEQVOL2SND2",      DVPTR(ENDSEQVOL2SND2),     ENDSEQVOL2SND2__ },
    { "ENDSEQVOL2SND3",      DVPTR(ENDSEQVOL2SND3),     ENDSEQVOL2SND3__ },
    { "ENDSEQVOL2SND4",      DVPTR(ENDSEQVOL2SND4),     ENDSEQVOL2SND4__ },
    { "ENDSEQVOL2SND5",      DVPTR(ENDSEQVOL2SND5),     ENDSEQVOL2SND5__ },
    { "ENDSEQVOL2SND6",      DVPTR(ENDSEQVOL2SND6),     ENDSEQVOL2SND6__ },
    { "ENDSEQVOL2SND7",      DVPTR(ENDSEQVOL2SND7),     ENDSEQVOL2SND7__ },
    { "ENDSEQVOL3SND2",      DVPTR(ENDSEQVOL3SND2),     ENDSEQVOL3SND2__ },
    { "ENDSEQVOL3SND3",      DVPTR(ENDSEQVOL3SND3),     ENDSEQVOL3SND3__ },
    { "ENDSEQVOL3SND4",      DVPTR(ENDSEQVOL3SND4),     ENDSEQVOL3SND4__ },
    { "ENDSEQVOL3SND5",      DVPTR(ENDSEQVOL3SND5),     ENDSEQVOL3SND5__ },
    { "ENDSEQVOL3SND6",      DVPTR(ENDSEQVOL3SND6),     ENDSEQVOL3SND6__ },
    { "ENDSEQVOL3SND7",      DVPTR(ENDSEQVOL3SND7),     ENDSEQVOL3SND7__ },
    { "ENDSEQVOL3SND8",      DVPTR(ENDSEQVOL3SND8),     ENDSEQVOL3SND8__ },
    { "ENDSEQVOL3SND9",      DVPTR(ENDSEQVOL3SND9),     ENDSEQVOL3SND9__ },
    { "EXITMENUSOUND",       DVPTR(EXITMENUSOUND),      EXITMENUSOUND__ },
    { "EXPANDERSHOOT",       DVPTR(EXPANDERSHOOT),      EXPANDERSHOOT__ },
    { "FLUSH_TOILET",        DVPTR(FLUSH_TOILET),       FLUSH_TOILET__ },
    { "FLY_BY",              DVPTR(FLY_BY),             FLY_BY__ },
    { "GENERIC_AMBIENCE17",  DVPTR(GENERIC_AMBIENCE17), GENERIC_AMBIENCE17__ },
    { "GLASS_BREAKING",      DVPTR(GLASS_BREAKING),     GLASS_BREAKING__ },
    { "GLASS_HEAVYBREAK",    DVPTR(GLASS_HEAVYBREAK),   GLASS_HEAVYBREAK__ },
    { "INSERT_CLIP",         DVPTR(INSERT_CLIP),        INSERT_CLIP__ },
    { "INTRO4_1",            DVPTR(INTRO4_1),           INTRO4_1__ },
    { "INTRO4_2",            DVPTR(INTRO4_2),           INTRO4_2__ },
    { "INTRO4_3",            DVPTR(INTRO4_3),           INTRO4_3__ },
    { "INTRO4_4",            DVPTR(INTRO4_4),           INTRO4_4__ },
    { "INTRO4_5",            DVPTR(INTRO4_5),           INTRO4_5__ },
    { "INTRO4_6",            DVPTR(INTRO4_6),           INTRO4_6__ },
    { "INTRO4_B",            DVPTR(INTRO4_B),           INTRO4_B__ },
    { "ITEM_SPLASH",         DVPTR(ITEM_SPLASH),        ITEM_SPLASH__ },
    { "JIBBED_ACTOR5",       DVPTR(JIBBED_ACTOR5),      JIBBED_ACTOR5__ },
    { "JIBBED_ACTOR6",       DVPTR(JIBBED_ACTOR6),      JIBBED_ACTOR6__ },
    { "KICK_HIT",            DVPTR(KICK_HIT),           KICK_HIT__ },
    { "LASERTRIP_ARMING",    DVPTR(LASERTRIP_ARMING),   LASERTRIP_ARMING__ },
    { "LASERTRIP_EXPLODE",   DVPTR(LASERTRIP_EXPLODE),  LASERTRIP_EXPLODE__ },
    { "LASERTRIP_ONWALL",    DVPTR(LASERTRIP_ONWALL),   LASERTRIP_ONWALL__ },
    { "LIGHTNING_SLAP",      DVPTR(LIGHTNING_SLAP),     LIGHTNING_SLAP__ },
    { "MONITOR_ACTIVE",      DVPTR(MONITOR_ACTIVE),     MONITOR_ACTIVE__ },
    { "NITEVISION_ONOFF",    DVPTR(NITEVISION_ONOFF),   NITEVISION_ONOFF__ },
    { "OCTA_RECOG",          DVPTR(OCTA_RECOG),         OCTA_RECOG__ },
    { "PIG_RECOG",           DVPTR(PIG_RECOG),          PIG_RECOG__ },
    { "PIPEBOMB_BOUNCE",     DVPTR(PIPEBOMB_BOUNCE),    PIPEBOMB_BOUNCE__ },
    { "PIPEBOMB_EXPLODE",    DVPTR(PIPEBOMB_EXPLODE),   PIPEBOMB_EXPLODE__ },
    { "PISTOL_BODYHIT",      DVPTR(PISTOL_BODYHIT),     PISTOL_BODYHIT__ },
    { "PISTOL_FIRE",         DVPTR(PISTOL_FIRE),        PISTOL_FIRE__ },
    { "PISTOL_RICOCHET",     DVPTR(PISTOL_RICOCHET),    PISTOL_RICOCHET__ },
    { "POOLBALLHIT",         DVPTR(POOLBALLHIT),        POOLBALLHIT__ },
    { "PRED_RECOG",          DVPTR(PRED_RECOG),         PRED_RECOG__ },
    { "RATTY",               DVPTR(RATTY),              RATTY__ },
    { "RECO_ATTACK",         DVPTR(RECO_ATTACK),        RECO_ATTACK__ },
    { "RECO_PAIN",           DVPTR(RECO_PAIN),          RECO_PAIN__ },
    { "RECO_RECOG",          DVPTR(RECO_RECOG),         RECO_RECOG__ },
    { "RECO_ROAM",           DVPTR(RECO_ROAM),          RECO_ROAM__ },
    { "RIPHEADNECK",         DVPTR(RIPHEADNECK),        RIPHEADNECK__ },
    { "RPG_EXPLODE",         DVPTR(RPG_EXPLODE),        RPG_EXPLODE__ },
    { "RPG_SHOOT",           DVPTR(RPG_SHOOT),          RPG_SHOOT__ },
    { "SELECT_WEAPON",       DVPTR(SELECT_WEAPON),      SELECT_WEAPON__ },
    { "SHORT_CIRCUIT",       DVPTR(SHORT_CIRCUIT),      SHORT_CIRCUIT__ },
    { "SHOTGUN_COCK",        DVPTR(SHOTGUN_COCK),       SHOTGUN_COCK__ },
    { "SHOTGUN_FIRE",        DVPTR(SHOTGUN_FIRE),       SHOTGUN_FIRE__ },
    { "SHRINKER_FIRE",       DVPTR(SHRINKER_FIRE),      SHRINKER_FIRE__ },
    { "SHRINKER_HIT",        DVPTR(SHRINKER_HIT),       SHRINKER_HIT__ },
    { "SLIM_ATTACK",         DVPTR(SLIM_ATTACK),        SLIM_ATTACK__ },
    { "SLIM_DYING",          DVPTR(SLIM_DYING),         SLIM_DYING__ },
    { "SLIM_RECOG",          DVPTR(SLIM_RECOG),         SLIM_RECOG__ },
    { "SLIM_ROAM",           DVPTR(SLIM_ROAM),          SLIM_ROAM__ },
    { "SOMETHING_DRIPPING",  DVPTR(SOMETHING_DRIPPING), SOMETHING_DRIPPING__ },
    { "SOMETHINGFROZE",      DVPTR(SOMETHINGFROZE),     SOMETHINGFROZE__ },
    { "SOMETHINGHITFORCE",   DVPTR(SOMETHINGHITFORCE),  SOMETHINGHITFORCE__ },
    { "SQUISHED",            DVPTR(SQUISHED),           SQUISHED__ },
    { "SUBWAY",              DVPTR(SUBWAY),             SUBWAY__ },
    { "SWITCH_ON",           DVPTR(SWITCH_ON),          SWITCH_ON__ },
    { "TELEPORTER",          DVPTR(TELEPORTER),         TELEPORTER__ },
    { "THUD",                DVPTR(THUD),               THUD__ },
    { "THUNDER",             DVPTR(THUNDER),            THUNDER__ },
    { "TURR_RECOG",          DVPTR(TURR_RECOG),         TURR_RECOG__ },
    { "VENT_BUST",           DVPTR(VENT_BUST),          VENT_BUST__ },
    { "VOL4ENDSND1",         DVPTR(VOL4ENDSND1),        VOL4ENDSND1__ },
    { "VOL4ENDSND2",         DVPTR(VOL4ENDSND2),        VOL4ENDSND2__ },
    { "WAR_AMBIENCE2",       DVPTR(WAR_AMBIENCE2),      WAR_AMBIENCE2__ },
    { "WHIPYOURASS",         DVPTR(WHIPYOURASS),        WHIPYOURASS__ },
    { "WIERDSHOT_FLY",       DVPTR(WIERDSHOT_FLY),      WIERDSHOT_FLY__ },
    { "WIND_AMBIENCE",       DVPTR(WIND_AMBIENCE),      WIND_AMBIENCE__ },
    { "WIND_REPEAT",         DVPTR(WIND_REPEAT),        WIND_REPEAT__ },
    { "FLAMETHROWER_INTRO",  DVPTR(FLAMETHROWER_INTRO), FLAMETHROWER_INTRO__ },
    { "FLAMETHROWER_LOOP",   DVPTR(FLAMETHROWER_LOOP),  FLAMETHROWER_LOOP__ },
    { "FLAMETHROWER_END",    DVPTR(FLAMETHROWER_END),   FLAMETHROWER_END__ },
    { "E5L7_DUKE_QUIT_YOU",  DVPTR(E5L7_DUKE_QUIT_YOU), E5L7_DUKE_QUIT_YOU__ },
 };

inthashtable_t h_dsound = { NULL, INTHASH_SIZE(ARRAY_SIZE(g_dynSoundList)) };

#ifdef DYNSOUNDREMAP_ENABLE

int32_t ALIEN_SWITCH1       = ALIEN_SWITCH1__;
int32_t BIGBANG             = BIGBANG__;
int32_t BONUS_SPEECH1       = BONUS_SPEECH1__;
int32_t BONUS_SPEECH2       = BONUS_SPEECH2__;
int32_t BONUS_SPEECH3       = BONUS_SPEECH3__;
int32_t BONUS_SPEECH4       = BONUS_SPEECH4__;
int32_t BONUSMUSIC          = BONUSMUSIC__;
int32_t BOS1_RECOG          = BOS1_RECOG__;
int32_t BOS1_WALK           = BOS1_WALK__;
int32_t BOS2_RECOG          = BOS2_RECOG__;
int32_t BOS3_RECOG          = BOS3_RECOG__;
int32_t BOS4_RECOG          = BOS4_RECOG__;
int32_t BOSS4_DEADSPEECH    = BOSS4_DEADSPEECH__;
int32_t BOSS4_FIRSTSEE      = BOSS4_FIRSTSEE__;
int32_t BOSSTALKTODUKE      = BOSSTALKTODUKE__;
int32_t CAPT_RECOG          = CAPT_RECOG__;
int32_t CAT_FIRE            = CAT_FIRE__;
int32_t CHAINGUN_FIRE       = CHAINGUN_FIRE__;
int32_t COMM_RECOG          = COMM_RECOG__;
int32_t DRON_RECOG          = DRON_RECOG__;
int32_t DUKE_CRACK          = DUKE_CRACK__;
int32_t DUKE_CRACK_FIRST    = DUKE_CRACK_FIRST__;
int32_t DUKE_CRACK2         = DUKE_CRACK2__;
int32_t DUKE_DRINKING       = DUKE_DRINKING__;
int32_t DUKE_GASP           = DUKE_GASP__;
int32_t DUKE_GET            = DUKE_GET__;
int32_t DUKE_GETWEAPON2     = DUKE_GETWEAPON2__;
int32_t DUKE_GETWEAPON6     = DUKE_GETWEAPON6__;
int32_t DUKE_GOTHEALTHATLOW = DUKE_GOTHEALTHATLOW__;
int32_t DUKE_GRUNT          = DUKE_GRUNT__;
int32_t DUKE_HARTBEAT       = DUKE_HARTBEAT__;
int32_t DUKE_JETPACK_IDLE   = DUKE_JETPACK_IDLE__;
int32_t DUKE_JETPACK_OFF    = DUKE_JETPACK_OFF__;
int32_t DUKE_JETPACK_ON     = DUKE_JETPACK_ON__;
int32_t DUKE_LAND           = DUKE_LAND__;
int32_t DUKE_LAND_HURT      = DUKE_LAND_HURT__;
int32_t DUKE_LONGTERM_PAIN  = DUKE_LONGTERM_PAIN__;
int32_t DUKE_ONWATER        = DUKE_ONWATER__;
int32_t DUKE_PISSRELIEF     = DUKE_PISSRELIEF__;
int32_t DUKE_SCREAM         = DUKE_SCREAM__;
int32_t DUKE_SEARCH         = DUKE_SEARCH__;
int32_t DUKE_SEARCH2        = DUKE_SEARCH2__;
int32_t DUKE_TAKEPILLS      = DUKE_TAKEPILLS__;
int32_t DUKE_UNDERWATER     = DUKE_UNDERWATER__;
int32_t DUKE_URINATE        = DUKE_URINATE__;
int32_t DUKE_USEMEDKIT      = DUKE_USEMEDKIT__;
int32_t DUKE_WALKINDUCTS    = DUKE_WALKINDUCTS__;
int32_t DUKETALKTOBOSS      = DUKETALKTOBOSS__;
int32_t EARTHQUAKE          = EARTHQUAKE__;
int32_t EJECT_CLIP          = EJECT_CLIP__;
int32_t ELEVATOR_OFF        = ELEVATOR_OFF__;
int32_t ELEVATOR_ON         = ELEVATOR_ON__;
int32_t END_OF_LEVEL_WARN   = END_OF_LEVEL_WARN__;
int32_t ENDSEQVOL2SND1      = ENDSEQVOL2SND1__;
int32_t ENDSEQVOL2SND2      = ENDSEQVOL2SND2__;
int32_t ENDSEQVOL2SND3      = ENDSEQVOL2SND3__;
int32_t ENDSEQVOL2SND4      = ENDSEQVOL2SND4__;
int32_t ENDSEQVOL2SND5      = ENDSEQVOL2SND5__;
int32_t ENDSEQVOL2SND6      = ENDSEQVOL2SND6__;
int32_t ENDSEQVOL2SND7      = ENDSEQVOL2SND7__;
int32_t ENDSEQVOL3SND2      = ENDSEQVOL3SND2__;
int32_t ENDSEQVOL3SND3      = ENDSEQVOL3SND3__;
int32_t ENDSEQVOL3SND4      = ENDSEQVOL3SND4__;
int32_t ENDSEQVOL3SND5      = ENDSEQVOL3SND5__;
int32_t ENDSEQVOL3SND6      = ENDSEQVOL3SND6__;
int32_t ENDSEQVOL3SND7      = ENDSEQVOL3SND7__;
int32_t ENDSEQVOL3SND8      = ENDSEQVOL3SND8__;
int32_t ENDSEQVOL3SND9      = ENDSEQVOL3SND9__;
int32_t EXITMENUSOUND       = EXITMENUSOUND__;
int32_t EXPANDERSHOOT       = EXPANDERSHOOT__;
int32_t FLUSH_TOILET        = FLUSH_TOILET__;
int32_t FLY_BY              = FLY_BY__;
int32_t GENERIC_AMBIENCE17  = GENERIC_AMBIENCE17__;
int32_t GLASS_BREAKING      = GLASS_BREAKING__;
int32_t GLASS_HEAVYBREAK    = GLASS_HEAVYBREAK__;
int32_t INSERT_CLIP         = INSERT_CLIP__;
int32_t INTRO4_1            = INTRO4_1__;
int32_t INTRO4_2            = INTRO4_2__;
int32_t INTRO4_3            = INTRO4_3__;
int32_t INTRO4_4            = INTRO4_4__;
int32_t INTRO4_5            = INTRO4_5__;
int32_t INTRO4_6            = INTRO4_6__;
int32_t INTRO4_B            = INTRO4_B__;
int32_t ITEM_SPLASH         = ITEM_SPLASH__;
int32_t JIBBED_ACTOR5       = JIBBED_ACTOR5__;
int32_t JIBBED_ACTOR6       = JIBBED_ACTOR6__;
int32_t KICK_HIT            = KICK_HIT__;
int32_t LASERTRIP_ARMING    = LASERTRIP_ARMING__;
int32_t LASERTRIP_EXPLODE   = LASERTRIP_EXPLODE__;
int32_t LASERTRIP_ONWALL    = LASERTRIP_ONWALL__;
int32_t LIGHTNING_SLAP      = LIGHTNING_SLAP__;
int32_t MONITOR_ACTIVE      = MONITOR_ACTIVE__;
int32_t NITEVISION_ONOFF    = NITEVISION_ONOFF__;
int32_t OCTA_RECOG          = OCTA_RECOG__;
int32_t PIG_RECOG           = PIG_RECOG__;
int32_t PIPEBOMB_BOUNCE     = PIPEBOMB_BOUNCE__;
int32_t PIPEBOMB_EXPLODE    = PIPEBOMB_EXPLODE__;
int32_t PISTOL_BODYHIT      = PISTOL_BODYHIT__;
int32_t PISTOL_FIRE         = PISTOL_FIRE__;
int32_t PISTOL_RICOCHET     = PISTOL_RICOCHET__;
int32_t POOLBALLHIT         = POOLBALLHIT__;
int32_t PRED_RECOG          = PRED_RECOG__;
int32_t RATTY               = RATTY__;
int32_t RECO_ATTACK         = RECO_ATTACK__;
int32_t RECO_PAIN           = RECO_PAIN__;
int32_t RECO_RECOG          = RECO_RECOG__;
int32_t RECO_ROAM           = RECO_ROAM__;
int32_t RIPHEADNECK         = RIPHEADNECK__;
int32_t RPG_EXPLODE         = RPG_EXPLODE__;
int32_t RPG_SHOOT           = RPG_SHOOT__;
int32_t SELECT_WEAPON       = SELECT_WEAPON__;
int32_t SHORT_CIRCUIT       = SHORT_CIRCUIT__;
int32_t SHOTGUN_COCK        = SHOTGUN_COCK__;
int32_t SHOTGUN_FIRE        = SHOTGUN_FIRE__;
int32_t SHRINKER_FIRE       = SHRINKER_FIRE__;
int32_t SHRINKER_HIT        = SHRINKER_HIT__;
int32_t SLIM_ATTACK         = SLIM_ATTACK__;
int32_t SLIM_DYING          = SLIM_DYING__;
int32_t SLIM_RECOG          = SLIM_RECOG__;
int32_t SLIM_ROAM           = SLIM_ROAM__;
int32_t SOMETHING_DRIPPING  = SOMETHING_DRIPPING__;
int32_t SOMETHINGFROZE      = SOMETHINGFROZE__;
int32_t SOMETHINGHITFORCE   = SOMETHINGHITFORCE__;
int32_t SQUISHED            = SQUISHED__;
int32_t SUBWAY              = SUBWAY__;
int32_t SWITCH_ON           = SWITCH_ON__;
int32_t TELEPORTER          = TELEPORTER__;
int32_t THUD                = THUD__;
int32_t THUNDER             = THUNDER__;
int32_t TURR_RECOG          = TURR_RECOG__;
int32_t VENT_BUST           = VENT_BUST__;
int32_t VOL4ENDSND1         = VOL4ENDSND1__;
int32_t VOL4ENDSND2         = VOL4ENDSND2__;
int32_t WAR_AMBIENCE2       = WAR_AMBIENCE2__;
int32_t WHIPYOURASS         = WHIPYOURASS__;
int32_t WIERDSHOT_FLY       = WIERDSHOT_FLY__;
int32_t WIND_AMBIENCE       = WIND_AMBIENCE__;
int32_t WIND_REPEAT         = WIND_REPEAT__;
int32_t FLAMETHROWER_INTRO  = FLAMETHROWER_INTRO__;
int32_t FLAMETHROWER_LOOP   = FLAMETHROWER_LOOP__;
int32_t FLAMETHROWER_END    = FLAMETHROWER_END__;
int32_t E5L7_DUKE_QUIT_YOU  = E5L7_DUKE_QUIT_YOU__;

static hashtable_t h_names = {512, NULL};

void G_ProcessDynamicSoundMapping(const char *szLabel, int32_t lValue)
{
    Bassert((unsigned)lValue < (unsigned)g_highestSoundIdx && szLabel);

    int const i = hash_find(&h_names,szLabel);

    if (i>=0)
    {
        struct dynitem *di = &g_dynSoundList[i];
#ifdef DEBUGGINGAIDS
        if (g_scriptDebug && di->staticval != lValue)
            OSD_Printf("REMAP %s (%d) --> %d\n", di->str, di->staticval, lValue);
#endif
        *di->dynvalptr = lValue;
    }
}

void initsoundhashnames(void)
{
    hash_init(&h_names);

    for (int i=0; i < ARRAY_SSIZE(g_dynSoundList); i++)
        hash_add(&h_names, g_dynSoundList[i].str, i, 0);
}

void freesoundhashnames(void)
{
    hash_free(&h_names);
}
#endif

// This is run after all CON define's have been processed to set up the
// dynamic->static sound mapping.
void G_InitDynamicSounds(void)
{
    inthash_init(&h_dsound);

    for (auto & i : g_dynSoundList)
#ifdef DYNSOUNDREMAP_ENABLE
        inthash_add(&h_dsound, *(i.dynvalptr), i.staticval, 0);
#else
        inthash_add(&h_dsound, i.staticval, i.staticval, 0);
#endif
}
