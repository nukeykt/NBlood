/*********************************************************************
*
*   WHMENU.C - menu code for Witchaven game
*
*********************************************************************/

#include "witchaven.h"
#include "player.h"
#include "sound.h"
#include "menu.h"
#include "input.h"
#include "keyboard.h"

void asmwaitvrt(int parm1)
{
}

void asmsetpalette(uint8_t *pal)
{
}

int loopinstuff; //here it is

extern int musiclevel;
extern int digilevel;

extern int gameactivated;
extern int escapetomenu;

extern int mapon;
extern int thunderflash;
extern int thundertime;


int loadedgame = 0;
int loadgo = 0;

char typemessage[162], typemessageleng = 0, typemode = 0;


char scantoasc[128] = {
    0,0,'1','2','3','4','5','6','7','8','9','0','-','=',0,0,
    'q','w','e','r','t','y','u','i','o','p','[',']',0,0,'a','s',
    'd','f','g','h','j','k','l',';',39,'`',0,92,'z','x','c','v',
    'b','n','m',',','.','/',0,'*',0,32,0,0,0,0,0,0,
    0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+','1',
    '2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

char scantoascwithshift[128] = {
    0,0,'!','@','#','$','%','^','&','*','(',')','_','+',0,0,
    'Q','W','E','R','T','Y','U','I','O','P','{','}',0,0,'A','S',
    'D','F','G','H','J','K','L',':',34,'~',0,'|','Z','X','C','V',
    'B','N','M','<','>','?',0,'*',0,32,0,0,0,0,0,0,
    0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+','1',
    '2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

#define MAXSAVEDGAMES 5
char savedgamenames[MAXSAVEDGAMES][20];

extern int gameactivated;

//
// fancy font
//
// to use this function you will need to name the starting letter
// the function will then scan the string and display its chars

char fancy[41] = {  'a','b','c','d','e',
                    'f','g','h','i','j',
                    'k','l','m','n','o',
                    'p','q','r','s','t',
                    'u','v','w','x','y',
                    'z','0','1','2','3',
                    '4','5','6','7','8',
                    '9','!','?','-',':',' '};

#if 0
void loadsavetoscreen()
{
    if (svga == 0)
        permanentwritesprite(0L, 0, ZLOADSAVE, 0, 0, 0, 319, 199, 0);
    else {
        rotatesprite(0 << 16, 0 << 16, 65536, 0, VLOAD, 0, 0, 8 + 16, 0, 0, xdim - 1, ydim - 1);
        rotatesprite(0 << 16, 240 << 16, 65536, 0, VSAVE, 0, 0, 8 + 16, 0, 0, xdim - 1, ydim - 1);
    }
}

void menutoscreen()
{
    if (svga == 0)
        permanentwritesprite(0, 0, THEMAINMENUWITH, 0, 0, 0, 319, 199, 0);
    else
        rotatesprite(0 << 16, 0 << 16, 65536, 0, VMAIN, 0, 0, 8 + 16, 0, 0, xdim - 1, ydim - 1);
}

void menutoscreenblank()
{
    permanentwritesprite(0, 0, THEMAINMENU, 0, 0, 0, 319, 199, 0);
}
#endif

void itemtoscreen(int x, int y, short tilenum, int8_t shade, char pal)
{
    overwritesprite(x, y, tilenum, shade, 2, pal);
    //rotatesprite_fs(x, y, 65536, 0, dapic, dashade, dapal, 1024);
    //permanentwritesprite(x, y, dapic, dashade, x, y, xdim - 1, ydim - 1, dapal);
}

static void playrandomsound()
{
    SND_Sound(rand() % 60);
}

void fancyfont(int x, int y, short tilenum, char* string, char pal)
{
    int incr = 0;
    int exit = 0;
    int number;
    char temp[40];

    strcpy(temp, string);
    Bstrlwr(temp);
    int len = strlen(temp);

    for (int i = 0; i < len; i++)
    {
        for (int j = 0; j < 40; j++)
        {
            if (temp[i] == fancy[j]) {
                number = j;
            }
        }
        if (i == 0)
        {
            overwritesprite(x, y, tilenum + number, 0, 2, pal);
            incr += tilesiz[tilenum + number].x + 1;
        }
        else if (temp[i] != ' ')
        {
            overwritesprite(x + incr, y, tilenum + number, 0, 2, pal);
            incr += tilesiz[tilenum + number].x + 1;
        }
        else {
            incr += 8;
        }
    }
}

void fancyfontscreen(int x, int y, short tilenum, char* string)
{
    int incr = 0;
    int exit = 0;
    int number;
    char temp[40];

    strcpy(temp, string);
    Bstrlwr(temp);
    int len = strlen(temp);

    for (int i = 0; i < len; i++)
    {
        for (int j = 0; j < 40; j++)
        {
            if (temp[i] == fancy[j]) {
                number = j;
            }
        }
        if (i == 0) {
            overwritesprite(x, y, tilenum + number, 0, 0x02, 7);
            incr += tilesiz[tilenum + number].x + 1;
        }
        else if (temp[i] != ' ') {
            overwritesprite(x + incr, y, tilenum + number, 0, 0x02, 7);
            incr += tilesiz[tilenum + number].x + 1;
        }
        else {
            incr += 8;
        }
    }
}

int menuscreen()
{
    videoClearScreen(0);

    struct {
        int x;
        int y;
    }redpic[5] = {
        { 142,58 },
        { 140,80 },
        { 127,104 },
        { 184,126 },
        { 183,150 }
    };

    int exit = 0;
    int select = 0;
    short redpicnum;

    if (netgame) {
        return 0;
    }

    SND_CheckLoops();

    redpicnum = NEWGAMEGREEN;

    KB_FlushKeyboardQueue();
    KB_ClearKeysDown();

    int32_t goaltime = (int)totalclock + 10;

    if (svga == 1) {
        permanentwritesprite(0, 0, SVGAMENU, 0, 0, 0, 639, 239, 0);
        permanentwritesprite(0, 240, SVGAMENU2, 0, 0, 240, 639, 479, 0);
    }

    while (!exit)
    {
        handleevents();

        overwritesprite(0, 0, MAINMENU, 0, 2, 0);
        overwritesprite(127, 58, MENUSELECTIONS, 0, 2, 0);

        if (select < 5) {
            redpicnum = NEWGAMEGREEN + select;
            overwritesprite(redpic[select].x, redpic[select].y, redpicnum, 0, 2, 0);
        }

        videoNextPage();

        if ((int)totalclock >= goaltime)
        {
            goaltime = (int)totalclock + 10;

            if (keystatus[sc_DownArrow] || keystatus[sc_kpad_2])
            {
                playrandomsound();
                select++;
                if (select > 4)
                    select = 0;

                keystatus[sc_DownArrow] = 0;
                keystatus[sc_kpad_2] = 0;
            }

            if (keystatus[sc_UpArrow] || keystatus[sc_kpad_8])
            {
                playrandomsound();
                select--;
                if (select < 0)
                    select = 4;

                keystatus[sc_UpArrow] = 0;
                keystatus[sc_kpad_8] = 0;
            }
            if (keystatus[sc_Escape] > 0)
            {
                playrandomsound();

                if (gameactivated == 1) 
                {
                    lockclock = (int)totalclock;
                    exit = 1;
                }
                else {
                    select = 4;
                }
                keystatus[sc_Escape] = 0;
            }
            if (keystatus[sc_Return] > 0 || keystatus[sc_kpad_Enter] > 0)
            {
                keystatus[sc_Return] = 0;
                keystatus[sc_kpad_Enter] = 0;

                switch (select)
                {
                    case 0:
                    {
                        gameactivated = 0;

                        SND_Sting(S_STING1);
                        SND_FadeMusic();
                        srand((int)totalclock & 30000);

                        startnewgame();
                        gameactivated = 1;
                        exit = 1;
                        keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0; keystatus[sc_Escape] = 0;
                        break;
                    }
                    case 1:
                    {
                        playrandomsound();
                        loadsave();
                        if (loadgo == 1)
                            exit = 1;
                        loadgo = 0;
                        keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0; keystatus[sc_Escape] = 0;
                    }
                    break;
                    case 2:
                    {
                        playrandomsound();
                        thedifficulty();
                        keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0; keystatus[sc_Escape] = 0;
                    }
                    break;
                    case 3:
                    {
                        playrandomsound();
                        help();
                    }
                    break;
                    case 4:
                    {
                        playrandomsound();
                        quit();
                    }
                    break;
                    case 5:
                    {
                        lockclock = (int)totalclock;
                        exit = 1;
                    }
                    break;
                }
            }
        }

        // TEMP
        int time = (int)totalclock + 2;
        while ((int)totalclock < time)
        {
            handleevents();
        }
    }

    escapetomenu = 0;

    return 0;
}

void help()
{
    struct {
        int helpnames;
    }thenames[] = { WEAPONS,
                    SPELLS,
                    POTIONS,
                    WALKING,
                    FLYING,
                    CREDIT1,
                    CREDIT2,
                    CREDIT3,
                    CREDIT4,
                    BETAPAGE };

    int select = 0;
    int32_t goaltime = 0;
    int exit = 0;

    while (!exit)
    {
        handleevents();

        if ((int)totalclock >= goaltime)
        {
            overwritesprite(0, 0, MAINMENU, 0, 2, 0);
            overwritesprite(0, 0, thenames[select].helpnames, 0, 2, 0);

            videoNextPage();

            goaltime = (int)totalclock + 10;

            if (keystatus[sc_DownArrow]
                || keystatus[sc_RightArrow]
                || keystatus[sc_kpad_2]
                || keystatus[sc_kpad_6])
            {
                playrandomsound();
                select++;
                if (select > 9)
                    select = 9;

                KB_ClearKeysDown();
            }
            if (keystatus[sc_UpArrow]
                || keystatus[sc_LeftArrow]
                || keystatus[sc_kpad_8]
                || keystatus[sc_kpad_4])
            {
                playrandomsound();
                select--;
                if (select < 0)
                    select = 0;

                KB_ClearKeysDown();
            }
            if (keystatus[sc_Escape] > 0) {
                exit = 1;
                keystatus[sc_Escape] = 0;
            }
            if (keystatus[sc_Return] > 0 || keystatus[sc_kpad_Enter] > 0) {
                exit = 2;
                keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0;
            }
        }
    }
    keystatus[sc_Escape] = keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0;
}

void loadsave()
{
    int exit = 0;
    int select = 0;

    int32_t goaltime = (int)totalclock + 10;
/*
    overwritesprite(0, 0, MAINMENU, 0, 2, 0);
    overwritesprite(182, 80, LOADGREEN, 0, 2, 0);
    overwritesprite(182, 119, SAVERED, 0, 2, 0);
*/
    while (!exit)
    {
        handleevents();

        overwritesprite(0, 0, MAINMENU, 0, 2, 0);
        overwritesprite(182, 80, LOADGREEN, 0, 2, 0);
        overwritesprite(182, 119, SAVERED, 0, 2, 0);

        if (select == 0)
        {
            overwritesprite(182, 80, LOADGREEN, 0, 2, 0);
            overwritesprite(182, 119, SAVERED, 0, 2, 0);
        }
        else
        {
            overwritesprite(182, 80, LOADRED, 0, 2, 0);
            overwritesprite(182, 119, SAVEGREEN, 0, 2, 0);
        }

        if ((int)totalclock >= goaltime)
        {
            goaltime = (int)totalclock + 10;

            if (keystatus[sc_DownArrow] || keystatus[sc_kpad_2])
            {
                playrandomsound();
                select = 1;

                keystatus[sc_DownArrow] = 0;
                keystatus[sc_kpad_2] = 0;
            }
            if (keystatus[sc_UpArrow] || keystatus[sc_kpad_8])
            {
                playrandomsound();
                select = 0;

                keystatus[sc_UpArrow] = 0;
                keystatus[sc_kpad_8] = 0;
            }

            if (keystatus[sc_Escape] > 0) {
                exit = 1;
                keystatus[sc_Escape] = 0;
            }
            if (keystatus[sc_Return] > 0 || keystatus[sc_kpad_Enter] > 0)
            {
                exit = 2;
                keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0;
            }
        }
        videoNextPage();
    }

    keystatus[sc_Escape] = 0;

    if (exit == 2)
    {
        switch (select)
        {
            case 0:
            {
                loadgame();
            }
            break;
            case 1:
            {
                if (gameactivated == 1) {
                    savegame();
                }
            }
            break;
        }
    }
}

void quit()
{
    int exit = 0;

    overwritesprite(0, 0, MAINMENU, 0, 2, 0);
    overwritesprite(123, 79, AREYOUSURE, 0, 2, 0);
    videoNextPage();

    while (!exit)
    {
        handleevents();

        if (keystatus[sc_kpad_Enter] > 0 || keystatus[sc_Return] > 0 || keystatus[sc_Y] > 0) {
            exit = 1;
            keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0;
        }
        if (keystatus[sc_Escape] > 0 || keystatus[sc_N] > 0) {
            exit = 2;
            keystatus[sc_Escape] = 0;
        }
    }

    if (exit == 2) {
        keystatus[sc_Escape] = 0;
    }
    else
    {
        if (svga == 1)
        {
            permanentwritesprite(0, 0, SVGAORDER1, 0, 0, 0, 639, 239, 0);
            permanentwritesprite(0, 240, SVGAORDER2, 0, 0, 240, 639, 479, 0);
            videoNextPage();

            exit = 0;

            while (!exit)
            {
                handleevents();

                if (keystatus[sc_Space] > 0 || keystatus[sc_Escape] > 0) {
                    exit = 1;
                }
            }

            keystatus[sc_Space] = 0;
            keystatus[sc_Escape] = 0;
        }
        else
        {
            keystatus[sc_Space] = 0;
            keystatus[sc_Escape] = 0;
            exit = 0;

            while (!exit)
            {
                handleevents();

                if (keystatus[sc_Space] > 0 || keystatus[sc_Escape] > 0) {
                    exit = 1;
                }

                overwritesprite(0, 0, ORDER1, 0, 2, 0);
                videoNextPage();
            }

            keystatus[sc_Space] = 0;
            keystatus[sc_Escape] = 0;
            exit = 0;
            /*
            while( !exit ){
                if(keystatus[sc_Space] > 0 || keystatus[sc_Escape] > 0)
                    exit=1;
                overwritesprite(0,0,ORDER2,0,0,0);
                nextpage();
            }
            keystatus[sc_Space]=0;
            keystatus[sc_Escape]=0;
            exit=0;
            */
        }
        shutdown();
    }
}

void thedifficulty()
{
    struct {
        int x;
        int y;
    }redpic[4] = {
        { 148,146 },
        { 181,146 },
        { 215,144 },
        { 257,143 }
    };

    int exit = 0;
    int selected;
    int select;
    int select2 = 0;
    int select3 = goreon;
    int redpicnum;
    int pickone = 0;

    Player *plr = &player[pyrn];

    select = difficulty - 1;
    selected = select;

    keystatus[sc_Escape] = 0;
    keystatus[sc_Return] = 0;
    keystatus[sc_kpad_Enter] = 0;

    //loadtile(MAINMENU);
/*
    overwritesprite(0, 0, MAINMENU, 0, 2, 0);
    overwritesprite(127, 58, BLOODGOREGREEN, 0, 2, 0);
    overwritesprite(148, 114, DIFFICULTRED, 0, 2, 0);

    if (goreon == 1) {
        overwritesprite(180, 84, NOGORESHADOW, 0, 0x04, 0);
        overwritesprite(214, 81, GORESOLID, 0, 0, 0);
    }
    else {
        overwritesprite(180, 84, NOGORESOLID, 0, 0, 0);
        overwritesprite(214, 81, GORESHADOW, 0, 0x04, 0);
    }
*/
    int32_t goaltime = (int)totalclock + 10;

    while (!exit)
    {
        handleevents();

        overwritesprite(0, 0, MAINMENU, 0, 2, 0);
/*
        overwritesprite(127, 58, BLOODGOREGREEN, 0, 2, 0);
        overwritesprite(148, 114, DIFFICULTRED, 0, 2, 0);

        if (goreon == 1)
        {
            overwritesprite(180, 84, NOGORESHADOW, 0, 0x04, 0);
            overwritesprite(214, 81, GORESOLID, 0, 0, 0);
        }
        else
        {
            overwritesprite(180, 84, NOGORESOLID, 0, 0, 0);
            overwritesprite(214, 81, GORESHADOW, 0, 0x04, 0);
        }
*/
        if ((int)totalclock >= goaltime)
        {
            goaltime = (int)totalclock + 10;

            if (keystatus[sc_UpArrow] || keystatus[sc_kpad_8])
            {
                playrandomsound();
                select2 = 0;

                keystatus[sc_UpArrow] = 0;
                keystatus[sc_kpad_8] = 0;
            }
            if (keystatus[sc_DownArrow] || keystatus[sc_kpad_2])
            {
                playrandomsound();
                select2 = 1;

                keystatus[sc_DownArrow] = 0;
                keystatus[sc_kpad_2] = 0;
            }

            if (select2 == 0)
                pickone = 0;
            else
                pickone = 1;

            redpicnum = HORNYSKULL1 + select;

//          overwritesprite(0, 0, MAINMENU, 0, 2, 0);

            if (select2 == 0) {
                overwritesprite(148, 114, DIFFICULTRED, 0, 2, 0);
                overwritesprite(127, 58, BLOODGOREGREEN, 0, 2, 0);
            }
            else {
                overwritesprite(148, 114, DIFFICULTGREEN, 0, 2, 0);
                overwritesprite(127, 58, BLOODGORERED, 0, 2, 0);
            }

            overwritesprite(147, 143, HORNYBACK, 0, 2, 0);
            overwritesprite(redpic[select].x, redpic[select].y, redpicnum, 0, 2, 0);

            if (goreon == 1) {
                overwritesprite(180, 84, NOGORESHADOW, 0, 0x06, 0);
                overwritesprite(214, 81, GORESOLID, 0, 2, 0);
            }
            else {
                overwritesprite(180, 84, NOGORESOLID, 0, 2, 0);
                overwritesprite(214, 81, GORESHADOW, 0, 0x06, 0);
            }

            if (pickone == 1)
            {
                if (keystatus[sc_LeftArrow] || keystatus[sc_kpad_4])
                {
                    playrandomsound();
                    select--;
                    if (select < 0)
                        select = 0;

                    keystatus[sc_LeftArrow] = 0;
                    keystatus[sc_kpad_4] = 0;
                }
                if (keystatus[sc_RightArrow] || keystatus[sc_kpad_6])
                {
                    playrandomsound();
                    select++;
                    if (select > 3)
                        select = 3;

                    keystatus[sc_RightArrow] = 0;
                    keystatus[sc_kpad_6] = 0;
                }

                selected = select;
            }
            else
            {
                if (keystatus[sc_LeftArrow] || keystatus[sc_kpad_4])
                {
                    playrandomsound();
                    select3 = 0;

                    keystatus[sc_LeftArrow] = 0;
                    keystatus[sc_kpad_4] = 0;
                }
                if (keystatus[sc_RightArrow] || keystatus[sc_kpad_6])
                {
                    playrandomsound();
                    select3 = 1;

                    keystatus[sc_RightArrow] = 0;
                    keystatus[sc_kpad_6] = 0;
                }

                if (select3 == 0)
                    goreon = 0;
                else
                    goreon = 1;
            }
            if (keystatus[sc_Return] > 0 || keystatus[sc_kpad_Enter] > 0) {
                exit = 1;
                keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0;
            }
            if (keystatus[sc_Escape] > 0) {
                exit = 2;
                keystatus[sc_Escape] = 0;
            }

            videoNextPage();
        }
    }

    if (exit == 1)
    {
        switch (selected)
        {
            case 0:
            difficulty = 1;
            break;
            case 1:
            difficulty = 2;
            break;
            case 2:
            difficulty = 3;
            break;
            case 3:
            difficulty = 4;
            break;
        }
    }
}

void startnewgame()
{
    char tempshow2dsector[MAXSECTORS >> 3];
    char tempshow2dwall[MAXWALLS >> 3];
    char tempshow2dsprite[MAXSPRITES >> 3];
    int i;

    if (netgame) {
        SND_StartMusic(mapon - 1);
        //goto skip;
        return;
    }

    if (loadedgame == 0)
    {
        sprintf(boardname, "level%d.map", mapon);
        setupboard(boardname);

        initplayersprite();

        SND_StartMusic(mapon - 1);
    }
    else if (loadedgame == 1)
    {
        loadplayerstuff();
        for (i = 0; i < (MAXSECTORS >> 3); i++) tempshow2dsector[i] = show2dsector[i];
        for (i = 0; i < (MAXWALLS >> 3); i++) tempshow2dwall[i] = show2dwall[i];
        for (i = 0; i < (MAXSPRITES >> 3); i++) tempshow2dsprite[i] = show2dsprite[i];
        setupboard(boardname);
        for (i = 0; i < (MAXSECTORS >> 3); i++) show2dsector[i] = tempshow2dsector[i];
        for (i = 0; i < (MAXWALLS >> 3); i++) show2dwall[i] = tempshow2dwall[i];
        for (i = 0; i < (MAXSPRITES >> 3); i++) show2dsprite[i] = tempshow2dsprite[i];
        loadedgame = 0;
        SND_StartMusic(mapon - 1);
    }
#if 0
skip:
    if (plr->screensize < 320)
        permanentwritesprite(0, 0, BACKGROUND, 0, 0, 0, 319, 199, 0);

    if (plr->screensize <= 320) {
        permanentwritesprite(0, 200 - 46, NEWSTATUSBAR, 0, 0, 0, 319, 199, 0);
    }
    updatepics();
#endif
}

void loadgame()
{
    int select = 0;
    int exit = 0;

    int32_t goaltime = (int)totalclock + 10;

    for (int i = 0; i < MAXSAVEDGAMES; i++)
    {
        if (!savedgamedat(i)) {
            strcpy(savedgamenames[i], "empty");
        }
    }

    while (!exit)
    {
        handleevents();

        overwritesprite(0, 0, MAINMENU, 0, 2, 0);
        overwritesprite(138, 84, SAVENUMBERS, 0, 2, 0);
        overwritesprite(190, 48, LOADPIC, 0, 2, 0);

        for (int i = 0; i < MAXSAVEDGAMES; i++) {
            fancyfont(154, 81 + (i * 17), THEFONT, savedgamenames[i], i == select ? 7 : 0);
        }

        videoNextPage();

        if ((int)totalclock >= goaltime)
        {
            goaltime = (int)totalclock + 10;

            if (keystatus[sc_UpArrow] || keystatus[sc_kpad_8])
            {
                select--;
                if (select < 0)
                    select = 4;

                keystatus[sc_UpArrow] = 0;
                keystatus[sc_kpad_8] = 0;
            }
            if (keystatus[sc_DownArrow] || keystatus[sc_kpad_2])
            {
                select++;
                if (select > 4)
                    select = 0;

                keystatus[sc_DownArrow] = 0;
                keystatus[sc_kpad_2] = 0;
            }

            if (keystatus[sc_Escape] > 0) {
                exit = 1;
                keystatus[sc_Escape] = 0;
            }

            if (keystatus[sc_Return] > 0 || keystatus[sc_kpad_Enter] > 0)
            {
                keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0;

                if (strcmp(savedgamenames[select], "empty") == 0) {
                    exit = 0;
                }
                else {
                    exit = 2;
                    loadgo = 1;
                }
            }
        }
    }

    if (exit == 2)
    {
        keystatus[sc_Escape] = keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0;

        sprintf(boardname, "svgm%d.map", select);
        sprintf(loadgamename, "svgn%d.dat", select);

        loadedgame = 1;
        gameactivated = 1;
        startnewgame();
    }
}

void savegame()
{
    int exit = 0;
    int i;
    int select = 0;

    for (i = 0; i < MAXSAVEDGAMES; i++)
    {
        if (!savedgamedat(i)) {
            strcpy(savedgamenames[i], "EMPTY");
        }
    }

    int32_t goaltime = (int)totalclock + 10L;

    while (!exit)
    {
        handleevents();

        overwritesprite(0, 0, MAINMENU, 0, 2, 0);
        overwritesprite(138, 84, SAVENUMBERS, 0, 2, 0);
        overwritesprite(188, 50, SAVEPIC, 0, 2, 0);

        for (i = 0; i < MAXSAVEDGAMES; i++)  {
            fancyfont(154, 81 + (i * 17), THEFONT, savedgamenames[i], i == select ? 7 : 0);
        }

        videoNextPage();

        if ((int)totalclock >= goaltime)
        {
            goaltime = (int)totalclock + 10;

            if (keystatus[sc_UpArrow] || keystatus[sc_kpad_8])
            {
                select--;
                if (select < 0)
                    select = 4;

                keystatus[sc_UpArrow] = 0;
                keystatus[sc_kpad_8] = 0;
            }

            if (keystatus[sc_DownArrow] || keystatus[sc_kpad_2])
            {
                select++;
                if (select > 4)
                    select = 0;

                keystatus[sc_DownArrow] = 0;
                keystatus[sc_kpad_2] = 0;
            }

            if (keystatus[sc_Escape] > 0) {
                exit = 1;
                keystatus[sc_Escape] = 0;
            }

            if (keystatus[sc_Return] > 0 || keystatus[sc_kpad_Enter] > 0) {
                exit = 2;
                typemessageleng = 0;
                keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0;
                savegametext(select);
            }
        }
    }
}

void savegametext(int select)
{
    int exit = 0;
    int i, j;
    char tempbuf[BMAX_PATH];
    char temp[40];

    int typemessageleng = 0;

    Player *plr = &player[pyrn];

    keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0;

    for (i = 0; i < 128; i++)
        keystatus[i] = 0;

    strcpy(temp, "-");

    while (!exit)
    {
        handleevents();

        overwritesprite(0, 0, MAINMENU, 0, 2, 0);
        overwritesprite(138, 84, SAVENUMBERS, 0, 2, 0);
        overwritesprite(188, 50, SAVEPIC, 0, 2, 0);

        for (i = 0; i < MAXSAVEDGAMES; i++)
        {
            if (i == select) {
                strcpy(tempbuf, temp);
                fancyfont(154, 81 + (i * 17), THEFONT, tempbuf, 7);
            }
            else {
                strcpy(tempbuf, savedgamenames[i]);
                fancyfont(154, 81 + (i * 17), THEFONT, tempbuf, 0);
            }
        }

        if (keystatus[sc_BackSpace] > 0) // backspace
        {  
            if (typemessageleng > 0) {
                temp[typemessageleng] = '\0';
                typemessageleng--;
                temp[typemessageleng] = '\0';
            }
            else {
                strcpy(temp, "-");
                typemessageleng = 0;
            }
            keystatus[sc_BackSpace] = 0;
        }

        if (typemessageleng < 10)
        {
            for (i = 0; i < 128; i++)
            {
                if (keystatus[i] > 0
                    && keystatus[sc_BackSpace] == 0
                    && keystatus[sc_Escape] == 0
                    && keystatus[sc_Return] == 0
                    && keystatus[sc_kpad_Enter] == 0) 
                {
                    for (j = 0; j < 41; j++)
                    {
                        if (scantoasc[i] == ' ') {
                            continue;
                        }
                        else if (scantoasc[i] == fancy[j])
                        {
                            temp[typemessageleng] = fancy[j];
                            typemessageleng++;
                            temp[typemessageleng] = '\0';
                            keystatus[i] = 0;
                        }
                        else {
                            keystatus[i] = 0;
                        }
                    }
                }
            }
        }

        if (keystatus[sc_Escape] > 0) {
            exit = 1;
            keystatus[sc_Escape] = 0;
        }

        if (keystatus[sc_Return] > 0 || keystatus[sc_kpad_Enter] > 0)
        {
            vec3_t pos;
            pos.x = plr->x;
            pos.y = plr->y;
            pos.z = plr->z;

            if (typemessageleng > 0)
            {
                strcpy(savedgamenames[select], temp);

                sprintf(tempbuf, "svgm%d.map", select);

                saveboard(tempbuf, &pos, plr->ang, plr->sector);
                savedgamename(select);
            }
            else
            {
                sprintf(tempbuf, "svgm%d.map", select);

                saveboard(tempbuf, &pos, plr->ang, plr->sector);
                savedgamename(select);
            }
            exit = 2;
            keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0;
        }
        videoNextPage();
    }

    if (exit == 2) {
        keystatus[sc_Return] = keystatus[sc_kpad_Enter] = 0;
    }
}

int savedgamename(int gn)
{
    int tmpanimateptr[MAXANIMATES];
    char savefilename[BMAX_PATH];

    Player *plr = &player[pyrn];

    plr->screensize = 320;

    sprintf(savefilename, "svgn%d.dat", gn);

    int file = open(savefilename, O_BINARY|O_TRUNC|O_CREAT|O_WRONLY, S_IREAD|S_IWRITE);

    if (file != -1)
    {
        write(file, savedgamenames[gn], sizeof(savedgamenames[0]));
        write(file, &player[pyrn], sizeof(Player));
        write(file, &visibility, sizeof(visibility));
        write(file, &brightness, sizeof(brightness));
        write(file, &thunderflash, sizeof(thunderflash));
        write(file, &thundertime, sizeof(thundertime));

        write(file, &mapon, sizeof(mapon));

        write(file, &totalclock, sizeof(totalclock));
        write(file, &lockclock, sizeof(lockclock));
        write(file, &synctics, sizeof(synctics));

        // Warning: only works if all pointers are in sector structures!
        for (int i = MAXANIMATES - 1; i >= 0; i--)
            tmpanimateptr[i] = (int)((intptr_t)animateptr[i] - (intptr_t)sector);

        write(file, animateptr, MAXANIMATES << 2);
        write(file, animategoal, MAXANIMATES << 2);
        write(file, animatevel, MAXANIMATES << 2);
        write(file, &animatecnt, 4);

        write(file, show2dsector, MAXSECTORS >> 3);
        write(file, show2dwall, MAXWALLS >> 3);
        write(file, show2dsprite, MAXSPRITES >> 3);
        write(file, &automapping, sizeof(automapping));

        close(file);
        return 1;
    }
       
    return 0;
}

int savedgamedat(int gn)
{
    int  fh = 0, nr = 0;
    char fname[BMAX_PATH];
    char fsname[BMAX_PATH];

    sprintf(fname, "svgm%d.map", gn);
    sprintf(fsname, "svgn%d.dat", gn);

    if (access(fname, F_OK) != 0)
        return 0;

    if (access(fsname, F_OK) != 0)
        return 0;

    fh = open(fsname, O_RDONLY | O_BINARY);

    if (fh < 0)
        return 0;

    nr = read(fh, savedgamenames[gn], sizeof(savedgamenames[0]));

    close(fh);

    if (nr != sizeof(savedgamenames[0]))
        return 0;

    return 1;
}

void loadplayerstuff()
{
    int tmpanimateptr[MAXANIMATES];

    int fh = open(loadgamename, O_RDONLY | O_BINARY);

    read(fh, savedgamenames[0], sizeof(savedgamenames[0]));
    read(fh, &player[pyrn], sizeof(Player));
    read(fh, &visibility, sizeof(visibility));
    read(fh, &brightness, sizeof(brightness));
    read(fh, &thunderflash, sizeof(thunderflash));
    read(fh, &thundertime, sizeof(thundertime));

    read(fh, &mapon, sizeof(mapon));

    read(fh, &totalclock, sizeof(totalclock));
    read(fh, &lockclock, sizeof(lockclock));
    read(fh, &synctics, sizeof(synctics));

    // Warning: only works if all pointers are in sector structures!
    read(fh, tmpanimateptr, MAXANIMATES << 2);
    for (int i = MAXANIMATES - 1; i >= 0; i--)
        animateptr[i] = (int*)(tmpanimateptr[i] + (intptr_t)sector);

    read(fh, animategoal, MAXANIMATES << 2);
    read(fh, animatevel, MAXANIMATES << 2);
    read(fh, &animatecnt, 4);

    read(fh, show2dsector, MAXSECTORS >> 3);
    read(fh, show2dwall, MAXWALLS >> 3);
    read(fh, show2dsprite, MAXSPRITES >> 3);
    read(fh, &automapping, sizeof(automapping));

    close(fh);
}

/***************************************************************************
 *   screen effects for TEKWAR follow...fades, palette stuff, etc..        *
 *                                                                         *
 *                                                     12/15/94 Jeff S.    *
 ***************************************************************************/

char      dofadein = 0;

#define   DEADTIME       2   // # minutes of nothing happening
#define   FLASHINTERVAL  20

int32_t passlock, lastastep, lastbstep, astep, bstep;

extern char flashflag;


void screenfx()
{
    updatepaletteshifts();
}

void clearpal()
{
#if 0
    short     i;

    outp(PEL_WRITE_ADR, 0);
    for (i = 0; i < 768; i++)
        outp(PEL_DATA, 0x00);
#endif
}


uint8_t palette1[256][3], palette2[256][3];

void getpalette(uint8_t *palette)
{
#if 0 // TODO
    int   i;

    outp(PEL_READ_ADR,0);
    for( i=0; i<768; i++)
        *palette++ = inp(PEL_DATA);
#endif
}

void fillpalette(int red, int green, int blue)
{
#if 0 // TODO
    int   i;

    outp(PEL_WRITE_ADR,0);
    for( i=0; i<256; i++ ) {
        outp(PEL_DATA,red);
        outp(PEL_DATA,green);
        outp(PEL_DATA,blue);
    }
#endif
}


bool foggy = false;

void fadeout(int start, int end, int red, int green, int blue, int steps)
{
    int orig, delta;
    uint8_t* origptr, * newptr;

    asmwaitvrt(1);
    getpalette(&palette1[0][0]);
    memcpy(palette2, palette1, 768);

    for (int i = 0; i < steps; i++)
    {
        origptr = &palette1[start][0];
        newptr = &palette2[start][0];

        for (int j = start; j <= end; j++)
        {
            orig = *origptr++;
            delta = red - orig;
            *newptr++ = orig + delta * i / steps;
            orig = *origptr++;
            delta = green - orig;
            *newptr++ = orig + delta * i / steps;
            orig = *origptr++;
            delta = blue - orig;
            *newptr++ = orig + delta * i / steps;
        }

        asmwaitvrt(1);
        asmsetpalette(&palette2[0][0]);
    }

    if (!foggy) {
        fillpalette(red, green, blue);
    }
}

void fadein(int start, int end, int steps)
{
    int  i, j, delta;

    if (steps == 0) {
        return;
    }

    asmwaitvrt(1);
    getpalette(&palette1[0][0]);
    memcpy(&palette2[0][0], &palette1[0][0], sizeof(palette1));

    start *= 3;
    end = end * 3 + 2;

    // fade through intermediate frames
    for (i = 0; i < steps; i++)
    {
        for (j = start; j <= end; j++)
        {
            delta = palette[j] - palette1[0][j];
            palette2[0][j] = palette1[0][j] + delta * i / steps;
        }

        asmwaitvrt(1);
        asmsetpalette(&palette2[0][0]);
    }

    // final color
    asmsetpalette(palette);

    dofadein = 0;
    //clearkeys();
}

void fog1()
{
    if (!foggy) {
        foggy = true;
        fadeout(1, 254, 8, 8, 10, 2);
    }
    else {
        foggy = false;
        fadein(0, 255, 2);
    }
}

void fog2()
{
    char *lookptr = palookup[0];
    palookup[0] = palookup[1];
    palookup[1] = lookptr;
}

#if 0 // not used?
void makefxlookups()
{
    char palbuf[256];

    for (int i = 0; i < 256; i++) {
        palbuf[i] = *(palookup[0] + i);
    }

// TODO    makepalookup(1, palbuf, 60, 60, 60, 1);
}
#endif

#define   NUMWHITESHIFTS      3
#define   WHITESTEPS          20
#define   WHITETICS           6

#define   NUMREDSHIFTS        4
#define   REDSTEPS            8

#define   NUMGREENSHIFTS      4
#define   GREENSTEPS          8

#define   NUMBLUESHIFTS       4
#define   BLUESTEPS           8

uint8_t whiteshifts[NUMREDSHIFTS][768];
uint8_t redshifts[NUMREDSHIFTS][768];
uint8_t greenshifts[NUMGREENSHIFTS][768];
uint8_t blueshifts[NUMBLUESHIFTS][768];

int  redcount, whitecount, greencount, bluecount;
char palshifted;


void initpaletteshifts()
{
    char* workptr;
    int  delta;
    uint8_t *baseptr;

    for (int i = 1; i <= NUMREDSHIFTS; i++)
    {
        workptr = (char*)&redshifts[i - 1][0];
        baseptr = &palette[0];

        for (int j = 0; j <= 255; j++)
        {
            delta = 64 - *baseptr;
            *workptr++ = *baseptr++ + delta * i / REDSTEPS;
            delta = -*baseptr;
            *workptr++ = *baseptr++ + delta * i / REDSTEPS;
            delta = -*baseptr;
            *workptr++ = *baseptr++ + delta * i / REDSTEPS;
        }
    }

    for (int i = 1; i <= NUMWHITESHIFTS; i++)
    {
        workptr = (char*)&whiteshifts[i - 1][0];
        baseptr = &palette[0];

        for (int j = 0; j <= 255; j++)
        {
            delta = 64 - *baseptr;
            *workptr++ = *baseptr++ + delta * i / WHITESTEPS;
            delta = 62 - *baseptr;
            *workptr++ = *baseptr++ + delta * i / WHITESTEPS;
            delta = 0 - *baseptr;
            *workptr++ = *baseptr++ + delta * i / WHITESTEPS;
        }
    }

    for (int i = 1; i <= NUMGREENSHIFTS; i++)
    {
        workptr = (char*)&greenshifts[i - 1][0];
        baseptr = &palette[0];

        for (int j = 0; j <= 255; j++)
        {
            delta = -*baseptr;
            *workptr++ = *baseptr++ + delta * i / GREENSTEPS;
            delta = 64 - *baseptr;
            *workptr++ = *baseptr++ + delta * i / GREENSTEPS;
            delta = -*baseptr;
            *workptr++ = *baseptr++ + delta * i / GREENSTEPS;
        }
    }

    for (int i = 1; i <= NUMBLUESHIFTS; i++)
    {
        workptr = (char*)&blueshifts[i - 1][0];
        baseptr = &palette[0];

        for (int j = 0; j <= 255; j++)
        {
            delta = -*baseptr;
            *workptr++ = *baseptr++ + delta * i / BLUESTEPS;
            delta = -*baseptr;
            *workptr++ = *baseptr++ + delta * i / BLUESTEPS;
            delta = 64 - *baseptr;
            *workptr++ = *baseptr++ + delta * i / BLUESTEPS;
        }
    }
}

void startgreenflash(int greentime)
{
    greencount = 0;

    greencount += greentime;

    if (greencount < 0) {
        greencount = 0;
    }
}

void startblueflash(int bluetime)
{
    bluecount = 0;

    bluecount += bluetime;

    if (bluecount < 0) {
        bluecount = 0;
    }
}

void startredflash(int damage)
{
    redcount = 0;

    redcount += damage;

    if (redcount < 0) {
        redcount = 0;
    }
}

void startwhiteflash(int bonus)
{
    whitecount = 0;

    whitecount += bonus;

    if (whitecount < 0) {
        whitecount = 0;
    }
}

void updatepaletteshifts()
{
    int red, white, green, blue;

    if (whitecount)
    {
        white = whitecount / WHITETICS + 1;
        if (white > NUMWHITESHIFTS)
            white = NUMWHITESHIFTS;
        whitecount -= synctics;
        if (whitecount < 0)
            whitecount = 0;
    }
    else {
        white = 0;
    }

    if (redcount)
    {
        red = redcount / 10 + 1;
        if (red > NUMREDSHIFTS)
            red = NUMREDSHIFTS;
        redcount -= synctics;
        if (redcount < 0)
            redcount = 0;
    }
    else {
        red = 0;
    }

    if (greencount)
    {
        green = greencount / 10 + 1;
        if (green > NUMGREENSHIFTS)
            green = NUMGREENSHIFTS;
        greencount -= synctics;
        if (greencount < 0)
            greencount = 0;
    }
    else {
        green = 0;
    }

    if (bluecount)
    {
        blue = bluecount / 10 + 1;
        if (blue > NUMBLUESHIFTS)
            blue = NUMBLUESHIFTS;
        bluecount -= synctics;
        if (bluecount < 0)
            bluecount = 0;
    }
    else {
        blue = 0;
    }

    if (red)
    {
        asmwaitvrt(1);
        //asmsetpalette(redshifts[red - 1]);

        paletteSetColorTable(0, redshifts[red - 1]);

        palshifted = 1;
    }
    else if (white)
    {
        asmwaitvrt(1);
        //asmsetpalette(whiteshifts[white - 1]);

        paletteSetColorTable(0, whiteshifts[white - 1]);

        palshifted = 1;
    }
    else if (green)
    {
        asmwaitvrt(1);
        //asmsetpalette(greenshifts[green - 1]);

        paletteSetColorTable(0, greenshifts[green - 1]);

        palshifted = 1;
    }
    else if (blue)
    {
        asmwaitvrt(1);
        //asmsetpalette(blueshifts[blue - 1]);

        paletteSetColorTable(0, blueshifts[blue - 1]);

        palshifted = 1;
    }

    else if (palshifted)
    {
        asmwaitvrt(1);
        //asmsetpalette(&palette[0]);     // back to normal

        paletteSetColorTable(0, palette);

        setbrightness(gbrightness);
        palshifted = 0;
    }
}

void finishpaletteshifts()
{
    if (palshifted == 1)
    {
        palshifted = 0;
        asmwaitvrt(1);
//      asmsetpalette(&palette[0]);

        videoSetPalette(0, BASEPAL, 2 + 8); // TODO
    }
}
