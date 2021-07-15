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
#ifndef soundsdyn_h__
#define soundsdyn_h__

#define DYNSOUNDREMAP_ENABLE


#define KICK_HIT__            0
#define PISTOL_RICOCHET__     1
#define PISTOL_BODYHIT__      2
#define PISTOL_FIRE__         3
#define EJECT_CLIP__          4
#define INSERT_CLIP__         5
#define CHAINGUN_FIRE__       6
#define RPG_SHOOT__           7
#define POOLBALLHIT__         8
#define RPG_EXPLODE__         9
#define CAT_FIRE__            10
#define SHRINKER_FIRE__       11
#define PIPEBOMB_BOUNCE__     13
#define PIPEBOMB_EXPLODE__    14
#define LASERTRIP_ONWALL__    15
#define LASERTRIP_ARMING__    16
#define LASERTRIP_EXPLODE__   17
#define VENT_BUST__           18
#define GLASS_BREAKING__      19
#define GLASS_HEAVYBREAK__    20
#define SHORT_CIRCUIT__       21
#define ITEM_SPLASH__         22
#define DUKE_GASP__           25
#define SLIM_RECOG__          26
#define DUKE_URINATE__        28
#define ENDSEQVOL3SND2__      29
#define ENDSEQVOL3SND3__      30
#define DUKE_CRACK__          33
#define SLIM_ATTACK__         34
#define SOMETHINGHITFORCE__   35
#define DUKE_DRINKING__       36
#define DUKE_GRUNT__          38
#define DUKE_HARTBEAT__       39
#define DUKE_ONWATER__        40
#define DUKE_LAND__           42
#define DUKE_WALKINDUCTS__    43
#define DUKE_UNDERWATER__     48
#define DUKE_JETPACK_ON__     49
#define DUKE_JETPACK_IDLE__   50
#define DUKE_JETPACK_OFF__    51
#define DUKETALKTOBOSS__      56
#define SQUISHED__            69
#define TELEPORTER__          70
#define ELEVATOR_ON__         71
#define ELEVATOR_OFF__        73
#define SUBWAY__              75
#define SWITCH_ON__           76
#define FLUSH_TOILET__        79
#define EARTHQUAKE__          81
#define END_OF_LEVEL_WARN__   83
#define WIND_AMBIENCE__       91
#define SOMETHING_DRIPPING__  92
#define BOS1_RECOG__          97
#define BOS2_RECOG__          102
#define DUKE_GETWEAPON2__     107
#define SHOTGUN_FIRE__        109
#define PRED_RECOG__          111
#define CAPT_RECOG__          117
#define PIG_RECOG__           121
#define RECO_ROAM__           125
#define RECO_RECOG__          126
#define RECO_ATTACK__         127
#define RECO_PAIN__           128
#define DRON_RECOG__          131
#define COMM_RECOG__          136
#define OCTA_RECOG__          141
#define TURR_RECOG__          146
#define SLIM_DYING__          149
#define BOS3_RECOG__          151
#define BOS1_WALK__           156
#define THUD__                158
#define WIERDSHOT_FLY__       160
#define SLIM_ROAM__           163
#define SHOTGUN_COCK__        169
#define GENERIC_AMBIENCE17__  177
#define BONUS_SPEECH1__       195
#define BONUS_SPEECH2__       196
#define BONUS_SPEECH3__       197
#define BONUS_SPEECH4__       199
#define DUKE_LAND_HURT__      200
#define DUKE_SEARCH2__        207
#define DUKE_CRACK2__         208
#define DUKE_SEARCH__         209
#define DUKE_GET__            210
#define DUKE_LONGTERM_PAIN__  211
#define MONITOR_ACTIVE__      212
#define NITEVISION_ONOFF__    213
#define DUKE_CRACK_FIRST__    215
#define DUKE_USEMEDKIT__      216
#define DUKE_TAKEPILLS__      217
#define DUKE_PISSRELIEF__     218
#define SELECT_WEAPON__       219
#define JIBBED_ACTOR5__       226
#define JIBBED_ACTOR6__       227
#define DUKE_GOTHEALTHATLOW__ 229
#define BOSSTALKTODUKE__      230
#define WAR_AMBIENCE2__       232
#define EXITMENUSOUND__       243
#define FLY_BY__              244
#define DUKE_SCREAM__         245
#define SHRINKER_HIT__        246
#define RATTY__               247
#define BONUSMUSIC__          249
#define DUKE_GETWEAPON6__     264
#define ALIEN_SWITCH1__       272
#define RIPHEADNECK__         284
#define ENDSEQVOL3SND4__      288
#define ENDSEQVOL3SND5__      289
#define ENDSEQVOL3SND6__      290
#define ENDSEQVOL3SND7__      291
#define ENDSEQVOL3SND8__      292
#define ENDSEQVOL3SND9__      293
#define WHIPYOURASS__         294
#define ENDSEQVOL2SND1__      295
#define ENDSEQVOL2SND2__      296
#define ENDSEQVOL2SND3__      297
#define ENDSEQVOL2SND4__      298
#define ENDSEQVOL2SND5__      299
#define ENDSEQVOL2SND6__      300
#define ENDSEQVOL2SND7__      301
#define SOMETHINGFROZE__      303
#define WIND_REPEAT__         308
#define BOS4_RECOG__          342
#define LIGHTNING_SLAP__      351
#define THUNDER__             352
#define INTRO4_1__            363
#define INTRO4_2__            364
#define INTRO4_3__            365
#define INTRO4_4__            366
#define INTRO4_5__            367
#define INTRO4_6__            368
#define BOSS4_DEADSPEECH__    370
#define BOSS4_FIRSTSEE__      371
#define VOL4ENDSND1__         384
#define VOL4ENDSND2__         385
#define EXPANDERSHOOT__       388
#define INTRO4_B__            392
#define BIGBANG__             393
#define FLAMETHROWER_INTRO__  398
#define FLAMETHROWER_LOOP__   399
#define FLAMETHROWER_END__    400
#define E5L7_DUKE_QUIT_YOU__  401

extern inthashtable_t h_dsound;

void G_InitDynamicSounds(void);

#ifdef DYNSOUNDREMAP_ENABLE

void G_ProcessDynamicSoundMapping(const char *szLabel, int32_t lValue);

void initsoundhashnames(void);
void freesoundhashnames(void);

extern int32_t ALIEN_SWITCH1;
extern int32_t BIGBANG;
extern int32_t BONUS_SPEECH1;
extern int32_t BONUS_SPEECH2;
extern int32_t BONUS_SPEECH3;
extern int32_t BONUS_SPEECH4;
extern int32_t BONUSMUSIC;
extern int32_t BOS1_RECOG;
extern int32_t BOS1_WALK;
extern int32_t BOS2_RECOG;
extern int32_t BOS3_RECOG;
extern int32_t BOS4_RECOG;
extern int32_t BOSS4_DEADSPEECH;
extern int32_t BOSS4_FIRSTSEE;
extern int32_t BOSSTALKTODUKE;
extern int32_t CAPT_RECOG;
extern int32_t CAT_FIRE;
extern int32_t CHAINGUN_FIRE;
extern int32_t COMM_RECOG;
extern int32_t DRON_RECOG;
extern int32_t DUKE_CRACK;
extern int32_t DUKE_CRACK_FIRST;
extern int32_t DUKE_CRACK2;
extern int32_t DUKE_DRINKING;
extern int32_t DUKE_GASP;
extern int32_t DUKE_GET;
extern int32_t DUKE_GETWEAPON2;
extern int32_t DUKE_GETWEAPON6;
extern int32_t DUKE_GOTHEALTHATLOW;
extern int32_t DUKE_GRUNT;
extern int32_t DUKE_HARTBEAT;
extern int32_t DUKE_JETPACK_IDLE;
extern int32_t DUKE_JETPACK_OFF;
extern int32_t DUKE_JETPACK_ON;
extern int32_t DUKE_LAND;
extern int32_t DUKE_LAND_HURT;
extern int32_t DUKE_LONGTERM_PAIN;
extern int32_t DUKE_ONWATER;
extern int32_t DUKE_PISSRELIEF;
extern int32_t DUKE_SCREAM;
extern int32_t DUKE_SEARCH;
extern int32_t DUKE_SEARCH2;
extern int32_t DUKE_TAKEPILLS;
extern int32_t DUKE_UNDERWATER;
extern int32_t DUKE_URINATE;
extern int32_t DUKE_USEMEDKIT;
extern int32_t DUKE_WALKINDUCTS;
extern int32_t DUKETALKTOBOSS;
extern int32_t EARTHQUAKE;
extern int32_t EJECT_CLIP;
extern int32_t ELEVATOR_OFF;
extern int32_t ELEVATOR_ON;
extern int32_t END_OF_LEVEL_WARN;
extern int32_t ENDSEQVOL2SND1;
extern int32_t ENDSEQVOL2SND2;
extern int32_t ENDSEQVOL2SND3;
extern int32_t ENDSEQVOL2SND4;
extern int32_t ENDSEQVOL2SND5;
extern int32_t ENDSEQVOL2SND6;
extern int32_t ENDSEQVOL2SND7;
extern int32_t ENDSEQVOL3SND2;
extern int32_t ENDSEQVOL3SND3;
extern int32_t ENDSEQVOL3SND4;
extern int32_t ENDSEQVOL3SND5;
extern int32_t ENDSEQVOL3SND6;
extern int32_t ENDSEQVOL3SND7;
extern int32_t ENDSEQVOL3SND8;
extern int32_t ENDSEQVOL3SND9;
extern int32_t EXITMENUSOUND;
extern int32_t EXPANDERSHOOT;
extern int32_t FLUSH_TOILET;
extern int32_t FLY_BY;
extern int32_t GENERIC_AMBIENCE17;
extern int32_t GLASS_BREAKING;
extern int32_t GLASS_HEAVYBREAK;
extern int32_t INSERT_CLIP;
extern int32_t INTRO4_1;
extern int32_t INTRO4_2;
extern int32_t INTRO4_3;
extern int32_t INTRO4_4;
extern int32_t INTRO4_5;
extern int32_t INTRO4_6;
extern int32_t INTRO4_B;
extern int32_t ITEM_SPLASH;
extern int32_t JIBBED_ACTOR5;
extern int32_t JIBBED_ACTOR6;
extern int32_t KICK_HIT;
extern int32_t LASERTRIP_ARMING;
extern int32_t LASERTRIP_EXPLODE;
extern int32_t LASERTRIP_ONWALL;
extern int32_t LIGHTNING_SLAP;
extern int32_t MONITOR_ACTIVE;
extern int32_t NITEVISION_ONOFF;
extern int32_t OCTA_RECOG;
extern int32_t PIG_RECOG;
extern int32_t PIPEBOMB_BOUNCE;
extern int32_t PIPEBOMB_EXPLODE;
extern int32_t PISTOL_BODYHIT;
extern int32_t PISTOL_FIRE;
extern int32_t PISTOL_RICOCHET;
extern int32_t POOLBALLHIT;
extern int32_t PRED_RECOG;
extern int32_t RATTY;
extern int32_t RECO_ATTACK;
extern int32_t RECO_PAIN;
extern int32_t RECO_RECOG;
extern int32_t RECO_ROAM;
extern int32_t RIPHEADNECK;
extern int32_t RPG_EXPLODE;
extern int32_t RPG_SHOOT;
extern int32_t SELECT_WEAPON;
extern int32_t SHORT_CIRCUIT;
extern int32_t SHOTGUN_COCK;
extern int32_t SHOTGUN_FIRE;
extern int32_t SHRINKER_FIRE;
extern int32_t SHRINKER_HIT;
extern int32_t SLIM_ATTACK;
extern int32_t SLIM_DYING;
extern int32_t SLIM_RECOG;
extern int32_t SLIM_ROAM;
extern int32_t SOMETHING_DRIPPING;
extern int32_t SOMETHINGFROZE;
extern int32_t SOMETHINGHITFORCE;
extern int32_t SQUISHED;
extern int32_t SUBWAY;
extern int32_t SWITCH_ON;
extern int32_t TELEPORTER;
extern int32_t THUD;
extern int32_t THUNDER;
extern int32_t TURR_RECOG;
extern int32_t VENT_BUST;
extern int32_t VOL4ENDSND1;
extern int32_t VOL4ENDSND2;
extern int32_t WAR_AMBIENCE2;
extern int32_t WHIPYOURASS;
extern int32_t WIERDSHOT_FLY;
extern int32_t WIND_AMBIENCE;
extern int32_t WIND_REPEAT;
extern int32_t FLAMETHROWER_INTRO;
extern int32_t FLAMETHROWER_LOOP;
extern int32_t FLAMETHROWER_END;
extern int32_t E5L7_DUKE_QUIT_YOU;

#define DYNAMICSOUNDMAP(Soundnum) (inthash_find(&h_dsound, Soundnum))

#else  /* if !defined DYNSOUNDREMAP_ENABLE */

#define G_ProcessDynamicSoundMapping(x, y) ((void)(0))

#define initsoundhashnames() ((void)0)
#define freesoundhashnames() ((void)0)

#include "soundefs.h"

#define DYNAMICSOUNDMAP(Soundnum) (Soundnum)

#endif
#endif // soundsdyn_h__
