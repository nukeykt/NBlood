// Copyright: 2020 Nuke.YKT, EDuke32 developers
// License: GPLv2
#include "compat.h"
#include "build.h"
#include "reality.h"
#include "../duke3d.h"
#include "../input.h"

const char *rt_scenestrings[] = {
    "WHEN WE LAST SAW DUKE NUKEM, THE "
    "EARTH'S ONLY REMAINING HOPE HAD "
    "SINGLE-HANDEDLY DEFEATED THE "
    "DESPICABLE DR. PROTON AND HIS "
    "LEGION OF EVIL ROBOTS. ",

    "WITH PROTON OUT OF THE WAY, "
    "DUKE RETURNED HOME TO A HERO'S "
    "WELCOME, AND WAS IMMEDIATELY "
    "ABDUCTED BY RIGELATINS - BEINGS "
    "GENERALLY KNOWN AS ALIEN SCUM "
    "FROM THE SEWERS OF SPACE. ",

    "USING EXPLOSIVES WISELY HIDDEN "
    "INSIDE A MOLAR, DUKE ESCAPED, "
    "DESTROYED THE RIGELATIN "
    "MOTHERSHIP AND CRUISED HOME IN A "
    "STOLEN, YET SUITABLY COOL, SHIP. ",

    "HIS LONG HAUL BACK TO EARTH, "
    "SHORTENED BY THOUGHTS OF ADORING "
    "CROWDS, CURVACEOUS BEACHES... ",

    "AND NO MORE FREAKIN' ALIENS! ",

    "WARNING!! INCOMING!! ",

    "HEY, EARTH? ANYBODY LISTENING? "
    "I GOT A LITTLE PROB... ",

    "AFTER WIPING THE BLOOD AND "
    "BRAINS FROM HIS BOOTS, DUKE "
    "EXPLORED THE ALIEN CRAFT. ",

    "MONITORS SHOWED A TITANIC ALIEN "
    "SHIP HOVERING ABOVE EARTH, WITH "
    "DOZENS OF SMALLER CRAFT "
    "OFF-LOADING COCOON-LIKE PODS. ",

    "ONE SHOWED THEM IN CLOSE-UP. "
    "THEY ALL HELD WOMEN, STILL "
    "ALIVE, JUST LIKE THE ONES DUKE "
    "HAD ENCOUNTERED. ",

    "DUKE GLOWERED IN THE PALE GREEN "
    "MONITOR LIGHT, AND SET THE "
    "AUTO-DESTRUCT SEQUENCE ON THE "
    "ALIEN SHIP. ",

    "HE STARED AT THE SCREEN ONCE "
    "MORE... ",

    "NOBODY MESSES WITH DUKE NUKEM! "
    "TIME FOR THAT VACATION, AND NO "
    "MORE FREAKIN' ALIENS!! ",

    "A HOLOGRAM FLICKERED ON, AND AN "
    "INCREDIBLY UGLY FACE SPOKE AN "
    "OMINOUS MESSAGE. ",

    "THE MOON ASSAULT OVERLORD HAS "
    "BEEN DEFEATED, AS HAS OUR "
    "BATTLELORD ON EARTH. ",

    "BUT WHILE DUKE NUKEM HAS BEEN "
    "DISTRACTED, OUR MAIN ATTACK "
    "FORCE HAS BEGUN IT'S FINAL "
    "ASSAULT ON EARTH. ",

    "WE SHALL OBLITERATE ALL "
    "RESISTANCE! "
};

struct {
    int type;
    int y;
    const char *string;
} rt_credits[] = {
    2, 0xfa, "DUKE NUKEM 64",
    2, 0x122, "DEVELOPED BY",
    1, 0x136, "EUROCOM ENTERTAINMENT SOFTWARE",
    2, 0x154, "PRODUCER",
    1, 0x168, "RICK RAYMO",
    2, 0x186, "PROGRAMMING",
    1, 0x19a, "ASHLEY FINNEY",
    1, 0x1a9, "SIMON MILLS",
    2, 0x1c7, "LEVEL DESIGN",
    1, 0x1db, "BILL BEACHAM",
    1, 0x1ea, "KEV HARVEY",
    2, 0x208, "GRAPHICS",
    1, 0x21c, "STEVE BAMFORD",
    1, 0x22b, "ANDY BEE",
    1, 0x23a, "ROB BENTON",
    1, 0x249, "NICK DRY",
    1, 0x258, "PAUL GREGORY",
    1, 0x267, "DARREN HYLAND",
    1, 0x276, "ADRIAN MANNION",
    2, 0x294, "AUDIO",
    1, 0x2a8, "NEIL BALDWIN",
    1, 0x2b7, "STEVE DUCKWORTH",
    2, 0x2d5, "PROJECT MANAGEMENT",
    1, 0x2e9, "HUGH BINNS",
    1, 0x2f8, "BILL BEACHAM",
    2, 0x316, "QUALITY APPROVAL",
    1, 0x32a, "MIKE BOTHAM",
    1, 0x339, "ANDY COLLINS",
    1, 0x348, "AARON JENKINS",
    2, 0x366, "GT EUROPE PRODUCER",
    1, 0x37a, "ROB LETTS",
    2, 0x398, "EUROPEAN LOCALISATION",
    1, 0x3ac, "CARA MCMULLAN",
    2, 0x3ca, "MANUAL",
    1, 0x3de, "NIC LAVROFF",
    1, 0x3ed, "BILL BEACHAM",
    2, 0x40b, "VOICE TALENT",
    1, 0x41f, "JON ST. JOHN AS 'DUKE NUKEM'",
    2, 0x43d, "SPECIAL THANKS TO",
    1, 0x451, "PAUL BATES",
    1, 0x460, "TIM ROGERS",
    1, 0x46f, "MAT SNEAP",
    1, 0x47e, "MR WHIPPLE AND THE MARIO CLUB",
    1, 0x48d, "KEN SILVERMAN",
    1, 0x49c, "TODD REPLOGLE",
    1, 0x4ab, "SCOTT MILLER",
    1, 0x4ba, "GEORGE BROUSSARD",
    1, 0x4c9, "TONY KEE",
    1, 0x4d8, "DAN HARNETT",
    1, 0x4e7, "MAX TAYLOR",
    1, 0x4f6, "STEW KOSOY",
    1, 0x505, "GRAEME BOXALL",
    1, 0x514, "JIM TRIPP",
    2, 0x546, "DUKE NUKEM 3D",
    2, 0x56e, "3D REALMS TEAM",
    2, 0x596, "ORIGINAL CONCEPT",
    1, 0x5aa, "TODD REPLOGLE",
    1, 0x5b9, "ALLEN H. BLUM III",
    2, 0x5d7, "EXECUTIVE PRODUCER",
    1, 0x5eb, "GEORGE BROUSSARD",
    2, 0x609, "PRODUCER",
    1, 0x61d, "GREG MALONE",
    2, 0x63b, "BUILD ENGINE AND TOOLS",
    1, 0x64f, "KEN SILVERMAN",
    2, 0x66d, "GAME PROGRAMMING",
    1, 0x681, "TODD REPLOGLE",
    2, 0x69f, "NETWORK LAYER",
    1, 0x6b3, "MARK DOCHTERMANN",
    2, 0x6d1, "MAP DESIGN",
    1, 0x6e5, "ALLEN H. BLUM III",
    1, 0x6f4, "RICHARD GRAY",
    2, 0x712, "3D MODELLING",
    1, 0x726, "CHUCK JONES",
    1, 0x735, "SAPPHIRE",
    2, 0x753, "ARTWORK",
    1, 0x767, "DIRK JONES",
    1, 0x776, "STEPHEN HORNBACK",
    1, 0x785, "JAMES STOREY",
    1, 0x794, "DAVID DEMARET",
    1, 0x7a3, "DOUGLAS R. WOOD",
    2, 0x7c1, "SOUND ENGINE",
    1, 0x7d5, "JIM DOSE",
    2, 0x7f3, "SOUND AND MUSIC",
    1, 0x807, "ROBERT PRINCE",
    1, 0x816, "LEE JACKSON",
    2, 0x834, "VOICE PRODUCER",
    1, 0x848, "LANI MINELLA",
    2, 0x866, "VOICE TALENT",
    1, 0x87a, "JON ST. JOHN AS 'DUKE NUKEM'",
};

static int intro_state = 0;
static int intro_sndcnt = 0;

void RT_GameText(float x, float y, const char *text)
{
    if (x == -1)
    {
        G_ScreenText(STARTALPHANUM, 160 * 65536.f, y * 65536.f, 65536, 0, 0, text, 0, 0, ROTATESPRITE_FULL16, 0, 7 * 65536, 0, 0, 0,
            TEXT_XCENTER | TEXT_N64COORDS | TEXT_N64NOPAL, 0, 0, xdim, ydim);
        return;
    }
    G_ScreenText(STARTALPHANUM, x * 65536.f, y * 65536.f, 65536, 0, 0, text, 0, 0, ROTATESPRITE_FULL16, 0, 7 * 65536, 0, 0, 0,
        TEXT_N64COORDS | TEXT_N64NOPAL, 0, 0, xdim, ydim);
}

void RT_MenuText(float x, float y, const char *text)
{
    if (x == -1)
    {
        G_ScreenText(BIGALPHANUM, 160 * 65535.f, y * 65535.f, 65536, 0, 0, text, 0, 0, ROTATESPRITE_FULL16, 0, 12 * 65536, 0, 0, 0,
            TEXT_XCENTER | TEXT_N64COORDS | TEXT_N64NOPAL | TEXT_BIGALPHANUM, 0, 0, xdim, ydim);
        return;
    }
    G_ScreenText(BIGALPHANUM, x * 65535.f, y * 65535.f, 65536, 0, 0, text, 0, 0, ROTATESPRITE_FULL16, 0, 12 * 65536, 0, 0, 0,
        TEXT_N64COORDS | TEXT_N64NOPAL | TEXT_BIGALPHANUM, 0, 0, xdim, ydim);
}


void RT_IntroAdvance(bool advance)
{
    if (advance)
    {
        intro_state++;
        ototalclock = totalclock;
        intro_sndcnt = 0;
    }
}

void RT_IntroText(int textId, int totaltime)
{
    char textline[34] = "";
    int intro_time = (int)(totalclock - ototalclock);
    int y = textId < 7 ? 20 : 180;
    int alpha = 0;
    if (totaltime - 16 * 4 < intro_time)
        alpha = (totaltime - intro_time) * 4;
    else
        alpha = min(intro_time * 4, 256);

    int l = strlen(rt_scenestrings[textId]);
    switch (textId)
    {
    case 4:
    case 6:
    case 12:
        RT_RotateSpriteSetColor(127, 255, 127, alpha);
        break;
    case 5:
        RT_RotateSpriteSetColor(255, 127, 127, alpha);
        break;
    case 14:
    case 15:
    case 16:
        RT_RotateSpriteSetColor(255, 127, 255, alpha);
        break;
    default:
        RT_RotateSpriteSetColor(255, 255, 255, alpha);
        break;
    }
    RT_RenderUnsetScissor();

    for (int i = 0; i < l;)
    {
        const char *t = rt_scenestrings[textId];
        int j;
        for (j = 0; j < 33 && i + j < l; j++)
            textline[j] = t[i + j];
        textline[j] = '\0';
        while (textline[j] != ' ' && j > 0)
        {
            textline[j] = '\0';
            j--;
        }
        RT_GameText(-1, y, textline);
        i += j + 1;
        y += 10;
    }
}

void RT_Intro(void)
{
    I_ClearAllInput();
    ototalclock = totalclock;
    intro_state = 1;
    S_TryPlaySpecialMusic(MUS_INTRO);
    bool playing = true;
    while (playing && !I_CheckAllInput())
    {
        if (engineFPSLimit())
        {
            RT_RenderUnsetScissor();
            videoClearScreen(0L);
            int intro_time = (int)(totalclock - ototalclock);
            RT_DisablePolymost(0);
            switch (intro_state)
            {
            case 0:
            {
                int alpha = max(256 - intro_time * 2, 0);
                RT_RotateSpriteSetColor(alpha, alpha, alpha, 256);
                RT_RotateSprite(160, 120, 100, 100, 0xe56, RTRS_SCALED);
                RT_IntroAdvance(alpha == 0);
                break;
            }
            case 1:
            {
                RT_IntroText(0, 160 * 4);
                RT_IntroAdvance(intro_time >= 160 * 4);
                break;
            }
            case 2:
            {
                RT_RenderScissor(91, 68, 229, 172);
                int alpha = min(intro_time * 2, 255);
                RT_RotateSpriteSetColor(alpha, alpha, alpha, 256);
                RT_RotateSprite(160, 120, 100, 100, 0xf56, RTRS_SCALED);
                RT_IntroText(1, 160 * 4);
                RT_IntroAdvance(intro_time >= 160 * 4);
                break;
            }
            case 3:
            {
                if (intro_time >= 60 * 4 && intro_sndcnt == 0)
                {
                    S_PlaySound(0xba);
                    intro_sndcnt++;
                }
                RT_RenderScissor(91, 68, 229, 172);
                int alpha = min(intro_time * 2, 255);
                RT_RotateSpriteSetColor(255, 255, 255, 256);
                RT_RotateSprite(160, 120, 100, 100, 0xf56, RTRS_SCALED);
                if (intro_time >= 60 * 4)
                {
                    float x = min<float>(intro_time * (1.f/4.f) - 60.f, 97.f);
                    float z = max<float>(50.f - (intro_time * (1.f/4.f) + 40.f) * 0.25f, 0.75);
                    RT_RotateSprite(x + 100, 120, 100.f / z, 100.f /z, 0xf57, RTRS_SCALED);
                }
                RT_IntroText(2, 160 * 4);
                RT_IntroAdvance(intro_time >= 160 * 4);
                break;
            }
            case 4:
            {
                float z = max(140.f - max(intro_time - 20 * 4, 0) * (1.f/4.f), 100.f);
                RT_RotateSpriteSetColor(255, 255, 255, 256);
                RT_RotateSprite(160, 120, z, z, 0xf58, RTRS_SCALED);
                RT_IntroText(3, 130 * 4);
                RT_IntroAdvance(intro_time >= 130 * 4);
                break;
            }
            case 5:
            {
                float z = max(140.f - max(intro_time - 20 * 4, 0) * (1.f/4.f), 100.f);
                RT_RotateSpriteSetColor(255, 255, 255, 256);
                RT_RotateSprite(160, 120, 100, 100, 0xf5a, RTRS_SCALED);
                RT_IntroAdvance(intro_time >= 40 * 4);
                break;
            }
            case 6:
            {
                int alpha = clamp(intro_time - 80, 0, 256);
                RT_RotateSpriteSetColor(255, 255, 255, 256 - alpha);
                RT_RotateSprite(160, 120, 100, 100, 0xf59, RTRS_SCALED);
                RT_RotateSpriteSetColor(255, 255, 255, alpha);
                RT_RotateSprite(160, 120, 100, 100, 0xf5b, RTRS_SCALED);
                RT_IntroText(4, 154 * 4);
                RT_IntroAdvance(intro_time >= 154 * 4);
                break;
            }
            case 7:
            {
                if (intro_time >= 4 && intro_sndcnt == 0)
                {
                    S_PlaySound(0x2d);
                    intro_sndcnt++;
                }
                if (intro_time >= 16*4 && intro_sndcnt == 1)
                {
                    S_PlaySound(0x37);
                    intro_sndcnt++;
                }
                if (intro_time >= 32*4 && intro_sndcnt == 2)
                {
                    S_PlaySound(0xb4);
                    intro_sndcnt++;
                }
                RT_RenderScissor(91, 68, 229, 172);
                RT_RotateSpriteSetColor(255, 255, 255, 256);
                float z = intro_time * (1.f/4.f) * 2.3f + 100.f;
                if ((intro_time & 32) == 0 && intro_time < 64 * 4)
                    RT_RotateSprite(160.f + intro_time * (1.f/4.f), 120.f + intro_time * (1.f/8.f), z, z, 0xf59, RTRS_SCALED);
                else
                    RT_RotateSprite(160.f, 120.f, 100.f, 100.f, 0xf5c, RTRS_SCALED);
                RT_IntroText(5, 79 * 4);
                RT_IntroAdvance(intro_time >= 79 * 4);
                break;
            }
            case 8:
            {
                if (intro_time >= 4 && intro_sndcnt == 0)
                {
                    S_PlaySound(0xc3);
                    intro_sndcnt++;
                }
                int o = 230 - intro_time * 2;
                RT_RotateSpriteSetColor(255, 255, 255, 256);
                RT_RotateSprite(160 + o, 120 - o / 3.f, 100, 100, 0xf5d, RTRS_SCALED);
                RT_IntroText(6, 57 * 4);
                RT_IntroAdvance(o < -230);
                break;
            }
            case 9:
            {
                if (intro_time >= 4 && intro_sndcnt == 0)
                {
                    S_StopMusic();
                    S_PlaySound(8);
                    intro_sndcnt++;
                }
                int alpha = max(376 - intro_time * 3 / 2, 0);
                RT_RotateSpriteSetColor(alpha, alpha, alpha, 256);
                float z = intro_time * (1.f/4.f) * 2.35f + 75.f;
                RT_RotateSprite(160, 120, z, z, 0xf5e, RTRS_SCALED);
                RT_IntroAdvance(alpha == 0);
                break;
            }
            default:
                playing = false;
                break;
            }
            RT_EnablePolymost();
            videoNextPage();

            G_HandleAsync();
        }
    }

    RT_RenderUnsetScissor();
    videoClearScreen(0L);
    videoNextPage();
}

void RT_BossScene(void)
{
    I_ClearAllInput();
    ototalclock = totalclock;
    intro_state = 0;
    bool playing = true;

    int tile = rt_levelnum == 6 ? 0xe6b : 0xe6c;
    int text = rt_levelnum == 6 ? 7 : 12;

    while (playing)
    {
        if (engineFPSLimit())
        {
            if (I_CheckAllInput() && intro_state < 6)
            {
                I_ClearAllInput();
                intro_state = 6;
            }
            videoClearScreen(0L);
            int intro_time = (int)(totalclock - ototalclock);
            RT_DisablePolymost(0);
            switch (intro_state)
            {
            case 0:
            {
                int alpha = min(intro_time * 4, 255);
                RT_RotateSpriteSetColor(alpha, alpha, alpha, 256);
                RT_RotateSprite(160, 120, 100, 100, tile, RTRS_SCALED);
                RT_IntroAdvance(intro_time * 4 >= 256);
                break;
            }
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            {
                RT_RotateSpriteSetColor(255, 255, 255, 256);
                RT_RotateSprite(160, 120, 100, 100, tile, RTRS_SCALED);
                RT_IntroText(text + intro_state - 1, 150 * 4);
                RT_IntroAdvance(intro_time >= 150 * 4);
                break;
            }
            case 6:
            {
                int alpha = max(256 - intro_time * 4, 0);
                RT_RotateSpriteSetColor(alpha, alpha, alpha, 256);
                RT_RotateSprite(160, 120, 100, 100, tile, RTRS_SCALED);
                RT_IntroAdvance(alpha == 0);
                break;
            }
            default:
                playing = false;
                break;
            }
            RT_EnablePolymost();
            videoNextPage();

            G_HandleAsync();
        }
    }

    videoClearScreen(0L);
    videoNextPage();
}

void RT_CreditsText(int type, float y, const char *string)
{
    if (y >= 0 && y <= 240)
    {
        int alpha = 0;
        if (y > 192)
            alpha = (3328 - y * 16);
        else
            alpha = min<int>(y * 16 - 256, 256);
        RT_RotateSpriteSetColor(255, 255, 255, alpha);
        switch (type)
        {
        case 1:
            RT_GameText(-1, y, string);
            break;
        case 2:
        default:
            RT_MenuText(-1, y, string);
            break;
        }
    }
}

void RT_FinalBossScene(void)
{
    I_ClearAllInput();
    ototalclock = totalclock;
    intro_state = 0;
    int soundcnt = 0;
    bool playing = true;

    while (playing)
    {
        if (engineFPSLimit())
        {
            if (I_CheckAllInput())
            {
                if (intro_state < 6)
                {
                    I_ClearAllInput();
                    intro_state = 6;
                    ototalclock = totalclock;
                }
                else if (intro_state == 6)
                {
                    I_ClearAllInput();
                    intro_state++;
                }
            }
            videoClearScreen(0L);
            int intro_time = (int)(totalclock - ototalclock);
            RT_DisablePolymost(0);
            switch (intro_state)
            {
            case 0:
            {
                int alpha = min(intro_time * 4, 255);
                RT_RotateSpriteSetColor(alpha, alpha, alpha, 256);
                RT_RotateSprite(160, 120, 100, 100, 0xe6d, RTRS_SCALED);
                RT_IntroAdvance(intro_time >= 60 * 4);
                break;
            }
            case 1:
            {
                if (intro_time >= 4)
                {
                    if (soundcnt < 1)
                    {
                        S_PlaySound(0x15);
                        soundcnt = 1;
                    }
                }
                RT_RotateSpriteSetColor(255, 255, 255, 256);
                RT_RotateSprite(160, 120, 100, 100, 0xe6e, RTRS_SCALED);
                RT_IntroAdvance(intro_time >= 60 * 4);
                break;
            }
            case 2:
            {
                RT_RotateSpriteSetColor(255, 255, 255, 256);
                RT_RotateSprite(160, 120, 100, 100, 0xe6f, RTRS_SCALED);
                RT_IntroAdvance(intro_time >= 60 * 4);
                break;
            }
            case 3:
            {
                if (intro_time >= 4)
                {
                    if (soundcnt < 2)
                    {
                        S_PlaySound(0x2c);
                        soundcnt = 2;
                    }
                }
                RT_RotateSpriteSetColor(255, 255, 255, 256);
                RT_RotateSprite(160, 120, 100, 100, 0xe70, RTRS_SCALED);
                RT_IntroAdvance(intro_time >= 60 * 4);
                break;
            }
            case 4:
            {
                if (intro_time >= 4)
                {
                    if (soundcnt < 3)
                    {
                        S_PlaySound(0x19);
                        soundcnt = 3;
                    }
                }
                RT_RotateSpriteSetColor(255, 255, 255, 256);
                RT_RotateSprite(160, 120, 100, 100, 0xe71, RTRS_SCALED);
                RT_IntroAdvance(intro_time >= 60 * 4);
                break;
            }
            case 5:
            {
                if (intro_time >= 4)
                {
                    if (soundcnt < 4)
                    {
                        S_PlaySound(0x18);
                        soundcnt = 4;
                    }
                }
                int alpha = max(60 * 16 - intro_time * 4, 0);
                RT_RotateSpriteSetColor(alpha, alpha, alpha, 256);
                RT_RotateSprite(160, 120, 100, 100, 0xe72, RTRS_SCALED);
                RT_IntroAdvance(intro_time >= 60 * 4);
                break;
            }
            case 6:
            {
                if (soundcnt < 5)
                {
                    S_PlaySpecialMusicOrNothing(MUS_INTRO);
                    soundcnt = 5;
                }
                int alpha = min(intro_time * 4, 255);
                RT_RotateSpriteSetColor(alpha, alpha, alpha, 256);
                RT_RotateSprite(160, 120, 100, 100, 0xe56, RTRS_SCALED);
                float y = -intro_time * (1.f / 8.f);
                for (int i = 0; i < ARRAY_SIZE(rt_credits); i++)
                {
                    RT_CreditsText(rt_credits[i].type, rt_credits[i].y + y, rt_credits[i].string);
                }
                RT_IntroAdvance(y + 2190 < 0);
                break;
            }
            default:
                playing = false;
                break;
            }
            RT_EnablePolymost();
            videoNextPage();

            G_HandleAsync();
        }
    }
    I_ClearAllInput();

    videoClearScreen(0L);
    videoNextPage();
}

void RT_Bonus(void)
{
    G_UpdateAppTitle();
    videoClearScreen(0L);
    videoNextPage();

    S_StopMusic();
    FX_StopAllSounds();
    S_ClearSoundLocks();
    FX_SetReverb(0L);
    CONTROL_BindsEnabled = 1; // so you can use your screenshot bind on the score screens

    if (ud.multimode == 1 || ud.coop)
    {
        if (rt_levelnum == 6 || rt_levelnum == 17)
        {
            RT_BossScene();
        }
        if (rt_levelnum == 27)
        {
            RT_FinalBossScene();

            I_ClearAllInput();

            videoClearScreen(0);
            videoNextPage();
            return;
        }
    }

    totalclock = 0;

    int bonus_state = 0;
    int soundcnt = 0;
    int soundcnt2 = 0;
    int soundcnt3 = 0;
    float bonus_oalpha = 0.f;

    S_StopMusic();
    FX_StopAllSounds();
    S_ClearSoundLocks();

    I_ClearAllInput();
    do
    {
        G_HandleAsync();
        MUSIC_Update();
        if (engineFPSLimit())
        {
            videoClearScreen(0L);
            RT_DisablePolymost(0);
            int buttons = 0;
            float bonus_alpha = 0.f;
            if (bonus_state == 0)
            {
                bonus_alpha = min(((int)totalclock * 4) * (1.f / 256.f), 1.f);
                buttons = I_CheckAllInput() != 0;
            }
            else
            {
                bonus_alpha = bonus_oalpha - ((int)totalclock - (int)ototalclock) * (1.f / 64.f);
                buttons = 0;
            }
            if (buttons)
            {
                if (soundcnt == 0)
                {
                    S_PlaySound(12);
                    soundcnt++;
                    bonus_state = 69;
                    bonus_oalpha = bonus_alpha;
                    ototalclock = totalclock;
                }
            }
            if (bonus_state != 69 || bonus_alpha > 0)
            {
                /*if (ud.multimode > 2 && !ud.coop)
                {
                    bonusmp();
                }
                else*/
                {
                    char buf[8];
                    RT_RotateSpriteSetColor(bonus_alpha * 255.f, bonus_alpha * 255.f, bonus_alpha * 255.f, 256);
                    RT_RotateSprite(160.f, 120.f, 100.f, 100.f, 0xf09, RTRS_SCALED);
                    int v3 = max((int)totalclock - 112 * 4, 0);
                    int dukeframe;
                    if (v3 < 20 * 4)
                        dukeframe = 0;
                    else
                    {
                        dukeframe = 2;
                        if (v3 < 36 * 4)
                            dukeframe = (v3 - 20 * 4) / 16;
                    }
                    if (v3 >= 28 * 4)
                    {
                        if (!soundcnt2)
                        {
                            S_PlaySound(0x83);
                            soundcnt2++;
                        }
                    }
                    RT_RotateSprite(79, 137.5, 100, 100, 0xf0a + dukeframe, RTRS_SCALED); // hack
                    RT_RotateSprite(79, 138, 100, 100, 0xf0a + dukeframe, RTRS_SCALED);
                    RT_RotateSpriteSetColor(bonus_alpha * 150.f, bonus_alpha * 200.f, bonus_alpha * 255.f, 256);
                    RT_MenuText(-1, 30, rt_level_names[rt_levelnum]);
                    RT_GameText(-1, 47, "COMPLETED");
                    int kills = g_player[0].ps->actors_killed;
                    int secrets = g_player[0].ps->secret_rooms;
                    int babes = g_player[0].ps->dn64_36e;
                    int secretstotal = g_player[0].ps->max_secret_rooms;
                    int babestotal = g_player[0].ps->dn64_36d;
                    int killstotal = 0;
                    for (int i = headspritestat[1]; i != -1; i = nextspritestat[i])
                    {
                        if (A_CheckEnemySprite(&sprite[i]) && sprite[i].picnum != RAT && sprite[i].extra > 0)
                        {
                            killstotal++;
                        }
                    }
                    for (int i = headspritestat[2]; i != -1; i = nextspritestat[i])
                    {
                        if (A_CheckEnemySprite(&sprite[i]) && sprite[i].picnum != RAT && sprite[i].extra > 0)
                        {
                            killstotal++;
                        }
                    }
                    /*if ((buttons & 0x8000) && controllerpak_port != -1)
                    {
                        S_PlaySound(12);
                        bonus_state = 42;
                    }
                    if (bonus_state == 42 && bonus_alpa <= 0.f)
                    {
                        // savegame
                    }
                    */
                    if ((int)totalclock > 16 * 4)
                    {
                        RT_RotateSpriteSetColor(bonus_alpha * 150, bonus_alpha * 255, bonus_alpha * 150, 256);
                        RT_GameText(0x8c, 0x46, "ENEMIES KILLED");
                        sprintf(buf, "%d", kills);
                        RT_GameText(0x10e, 0x46, buf);
                    }
                    if ((int)totalclock > 32 * 4)
                    {
                        RT_RotateSpriteSetColor(bonus_alpha * 255, bonus_alpha * 150, bonus_alpha * 150, 256);
                        RT_GameText(0x8c, 0x50, "ENEMIES LEFT");
                        sprintf(buf, "%d", killstotal);
                        RT_GameText(0x10e, 0x50, buf);
                    }
                    if ((int)totalclock > 48 * 4)
                    {
                        RT_RotateSpriteSetColor(bonus_alpha * 150, bonus_alpha * 255, bonus_alpha * 150, 256);
                        RT_GameText(0x8c, 0x69, "SECRETS FOUND");
                        sprintf(buf, "%d", secrets);
                        RT_GameText(0x10e, 0x69, buf);
                    }
                    if ((int)totalclock > 64 * 4)
                    {
                        RT_RotateSpriteSetColor(bonus_alpha * 255, bonus_alpha * 150, bonus_alpha * 150, 256);
                        RT_GameText(0x8c, 0x73, "SECRETS MISSED");
                        sprintf(buf, "%d", secretstotal - secrets);
                        RT_GameText(0x10e, 0x73, buf);
                    }
                    if ((int)totalclock > 80 * 4)
                    {
                        RT_RotateSpriteSetColor(bonus_alpha * 150, bonus_alpha * 255, bonus_alpha * 150, 256);
                        RT_GameText(0x8c, 0x8c, "BABES SAVED");
                        sprintf(buf, "%d", babes);
                        RT_GameText(0x10e, 0x8c, buf);
                    }
                    if ((int)totalclock > 96 * 4)
                    {
                        RT_RotateSpriteSetColor(bonus_alpha * 255, bonus_alpha * 150, bonus_alpha * 150, 256);
                        RT_GameText(0x8c, 0x96, "BABES LEFT");
                        sprintf(buf, "%d", babestotal - babes);
                        RT_GameText(0x10e, 0x96, buf);
                    }
                    int cnt = (int)totalclock >> 6;
                    if (soundcnt3 <= cnt && cnt < 7)
                    {
                        S_PlaySound(12);
                        soundcnt3 = cnt+1;
                    }
                    int alpha2 = (192 + (sintable[((int)totalclock & 255) * 8] / 256)) * bonus_alpha;
                    RT_RotateSpriteSetColor(alpha2, alpha2, alpha2, 256);
                    RT_GameText(-1, 0xc3, "PRESS START TO ENTER");
                    RT_MenuText(-1, 0xcd, rt_level_names[RT_NextLevel()]);
                    RT_RotateSpriteSetColor(255, 255, 255, 256);
                }
            }
            else
            {
                RT_EnablePolymost();
                break;
            }
            RT_EnablePolymost();
            videoNextPage();
        }
    } while (1);

    I_ClearAllInput();

    videoClearScreen(0);
    videoNextPage();
}
