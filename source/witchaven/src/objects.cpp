/***************************************************************************
 *   WHOBJ.C  - object animation code for Witchaven game                   *
 *                                                                         *
 ***************************************************************************/

#include "witchaven.h"
#include "objects.h"
#include "player.h"
#include "menu.h"
#include "sound.h"
#include "network.h"
#include "view.h"

struct point3d
{
    int32_t x;
    int32_t y;
    int32_t z;
};

point3d osprite[MAXSPRITESONSCREEN];

//extern int poisoned;
//extern int poisontime;

//extern short gbrightness;

int justwarpedcnt = 0;
extern int justwarpedfx;

extern int cartsnd;

extern int mapon;
extern int spellbooktics;
extern int spellbook;
extern int spellbookframe;
extern int spellbookflip;

short torchpattern[38] = { 2,2,2,3,3,3,4,4,5,5,6,6,4,4,6,6,4,4,6,6,4,4,6,6,4,4,6,6,5,5,4,4,3,3,3,2,2,2 };

char flashflag=0x00;
char tempbuf[50];
char scorebuf[50];
char healthbuf[50];
char potionbuf[50];

int debuginfo;

int scoretime;


void monitor()
{
    char svgah[30];

    Player* plr = &player[pyrn];

    if (plr->health < 21)
    {
        strcpy(tempbuf, "health critical");
        fancyfontscreen(18, 48, THEFONT, tempbuf);
    }

    if (justwarpedfx > 0)
    {
        justwarpedfx -= synctics;
        justwarpedcnt += synctics << 6;

        rotatesprite_fs(320 << 15, 200 << 15, justwarpedcnt << 9, 0, ANNIHILATE, 0, 0, 1 + 2);

        if (justwarpedfx <= 0) {
            justwarpedcnt = 0;
        }
    }

    if (plr->poisoned == 1)
    {
        if (plr->poisontime >= 0)
        {
            plr->poisontime -= synctics;

            //if(svga == 1)
            //    rotatesprite(200<<16,(0L+tilesizy[HELMET]>>2)<<16,16384,0,POISONICON,0,0,0x02);
            //else
            //    rotatesprite(230<<16,(0L+tilesizy[HELMET]>>1)<<16,16384,0,POISONICON,0,0,0x02);

            sethealth(0);

            if (plr->poisontime < 0)
            {
                startredflash(50);
                sethealth(-10);
                plr->poisontime = 7200;
            }
        }
    }

    if (plr->vampiretime > 0)
        plr->vampiretime -= synctics;

    if (plr->shockme >= 0)
        plr->shockme -= synctics;

    if (plr->helmettime > 0)
    {
        plr->helmettime -= synctics;
        if (svga == 1)
            rotatesprite_fs(270 << 16, ((0L + tilesiz[HELMET].y) >> 2) << 16, 32768, 0, HELMET, 0, 0, 0x02);
        else
            rotatesprite_fs(300 << 16, ((0L + tilesiz[HELMET].y) >> 1) << 16, 32768, 0, HELMET, 0, 0, 0x02);
    }

    DisplayStatusMessage();
    /*
        if (displaytime > 0)
        {
            fancyfontscreen(18, 24, THEFONT, displaybuf);
            displaytime -= ((int)synctics);
        }
    */
    if (plr->shadowtime >= 0)
        plr->shadowtime -= synctics;

    if (plr->nightglowtime >= 0)
    {
        plr->nightglowtime -= synctics;
        visibility = 256;
        if (plr->nightglowtime < 0)
            visibility = 1024;
    }

    if (plr->strongtime >= 0)
    {
        plr->strongtime -= synctics;
        startwhiteflash(10);
    }

    if (plr->invisibletime >= 0)
    {
        plr->invisibletime -= synctics;
    }

    if (plr->invincibletime >= 0)
    {
        plr->invincibletime -= synctics;
        startwhiteflash(10);
    }

    if (plr->manatime >= 0)
    {
        plr->manatime -= synctics;
        startredflash(20);
    }

    if (svga == 1)
    {
        if (plr->svgahealth > 0)
        {
            plr->svgahealth -= synctics;

            sprintf(svgah, "health %d", plr->health);
            fancyfontscreen(18, 44, THEFONT, svgah);

            if (plr->armor < 0)
                plr->armor = 0;

            sprintf(svgah, "armor %d", plr->armor);
            fancyfontscreen(18, 64, THEFONT, svgah);
        }
    }
}

// see if picked up any objects?
// JSA 4_27 play appropriate sounds pending object picked up

void processobjs(Player* plr)
{
    int32_t dh, dx, dy, dz, i, nexti;
    int j;

    if (plr->sector < 0 || plr->sector >= numsectors) {
        return;
    }

    i = headspritesect[plr->sector];

    while (i != -1)
    {
        nexti = nextspritesect[i];
        dx = labs(plr->x - sprite[i].x);            // x distance to sprite
        dy = labs(plr->y - sprite[i].y);            // y distance to sprite
        dz = labs((plr->z >> 8) - (sprite[i].z >> 8));  // z distance to sprite
        dh = tilesiz[sprite[i].picnum].y >> 1;       // height of sprite

        if (dx + dy < PICKDISTANCE && dz - dh <= PICKHEIGHT)
        {
            switch (sprite[i].picnum)
            {
                case HELMET:
                StatusMessage(360, "Hero Time");
                deletesprite(i);
                addarmoramount(10);
                plr->helmettime = 7200;
                //JSA_DEMO3
                SND_Sound(S_STING1 + rand() % 2);
                addscoreamount(10);
                break;
                case PLATEARMOR:
                if (plr->armor <= 149)
                {
                    StatusMessage(360, "Plate Armor");
                    deletesprite(i);
                    plr->armortype = 3;
                    plr->armor = 0;
                    addarmoramount(150);
                    SND_Sound(S_POTION1);
                    addscoreamount(40);
                }
                break;
                case CHAINMAIL:
                if (plr->armor <= 99)
                {
                    StatusMessage(360, "Chain Mail");
                    deletesprite(i);
                    plr->armortype = 2;
                    plr->armor = 0;
                    addarmoramount(100);
                    SND_Sound(S_POTION1);
                    addscoreamount(20);
                }
                break;
                case LEATHERARMOR:
                if (plr->armor <= 49)
                {
                    StatusMessage(360, "Leather Armor");
                    deletesprite(i);
                    plr->armortype = 1;
                    plr->armor = 0;
                    addarmoramount(50);
                    SND_Sound(S_POTION1);
                    addscoreamount(10);
                }
                break;
                case GIFTBOX:
                playsound_loc(S_TREASURE1, sprite[i].x, sprite[i].y);

                j = rand() % 7;

                switch (j)
                {
                    case 0:
                    {
                        switch (rand() % 5)
                        {
                            case 0:
                            {
                                if (!potionspace(0))
                                    break;

                                StatusMessage(360, "Health Potion");
                                updatepotion(HEALTHPOTION);
                                setpotion(0);
                                SND_Sound(S_POTION1);
                                addscoreamount(10);
                            }
                            break;
                            case 1:
                            {
                                if (!potionspace(1))
                                    break;

                                StatusMessage(360, "Strength Potion");
                                updatepotion(STRENGTHPOTION);
                                setpotion(1);
                                SND_Sound(S_POTION1);
                                addscoreamount(20);
                            }
                            break;
                            case 2:
                            {
                                if (!potionspace(2))
                                    break;

                                StatusMessage(360, "Cure Poison Potion");
                                updatepotion(ARMORPOTION);
                                setpotion(2);
                                SND_Sound(S_POTION1);
                                addscoreamount(15);
                            }
                            break;
                            case 3:
                            {
                                if (!potionspace(3))
                                    break;
                                StatusMessage(360, "Resist Fire Potion");
                                updatepotion(FIREWALKPOTION);
                                setpotion(3);
                                SND_Sound(S_POTION1);
                                addscoreamount(15);
                            }
                            break;
                            case 4:
                            {
                                if (!potionspace(4))
                                    break;
                                StatusMessage(360, "Invisibility Potion");
                                updatepotion(INVISIBLEPOTION);
                                setpotion(4);
                                SND_Sound(S_POTION1);
                                addscoreamount(30);
                            }
                            break;
                        }
                        sprite[i].picnum = OPENCHEST;
                    }
                    break;
                    case 1:
                    switch (rand() % 8)
                    {
                        case 0:
                        if (plr->orbammo[0] < 10)
                        {
                            plr->orb[0] = 1;
                            plr->orbammo[0]++;
                            StatusMessage(360, "Scare Scroll");
                            SND_Sound(S_POTION1);
                        }
                        break;
                        case 1:
                        if (plr->orbammo[1] < 10)
                        {
                            plr->orb[1] = 1;
                            plr->orbammo[1]++;
                            StatusMessage(360, "Night Vision Scroll");
                            SND_Sound(S_POTION1);
                        }
                        break;
                        case 2:
                        if (plr->orbammo[2] < 10)
                        {
                            plr->orb[2] = 1;
                            plr->orbammo[2]++;
                            StatusMessage(360, "Freeze Scroll");
                            SND_Sound(S_POTION1);
                        }
                        break;
                        case 3:
                        if (plr->orbammo[3] < 10)
                        {
                            plr->orb[3] = 1;
                            plr->orbammo[3]++;
                            StatusMessage(360, "Magic Arrow Scroll");
                            SND_Sound(S_POTION1);
                        }
                        break;
                        case 4:
                        if (plr->orbammo[4] < 10)
                        {
                            plr->orb[4] = 1;
                            plr->orbammo[4]++;
                            StatusMessage(360, "Open Door Scroll");
                            SND_Sound(S_POTION1);
                        }
                        break;
                        case 5:
                        if (plr->orbammo[5] < 10)
                        {
                            plr->orb[5] = 1;
                            plr->orbammo[5]++;
                            StatusMessage(360, "Fly Scroll");
                            SND_Sound(S_POTION1);
                        }
                        break;
                        case 6:
                        if (plr->orbammo[6] < 10)
                        {
                            plr->orb[6] = 1;
                            plr->orbammo[6]++;
                            StatusMessage(360, "Fireball Scroll");
                            SND_Sound(S_POTION1);
                        }
                        break;
                        case 7:
                        if (plr->orbammo[7] < 10)
                        {
                            plr->orb[7] = 1;
                            plr->orbammo[7]++;
                            StatusMessage(360, "Nuke Scroll");
                            SND_Sound(S_POTION1);
                        }
                        break;
                    }
                    sprite[i].picnum = OPENCHEST;
                    break;
                    case 2:
                    sprite[i].picnum = OPENCHEST;
                    addscoreamount((rand() % 400) + 100);
                    StatusMessage(360, "Treasure Chest");
                    SND_Sound(S_POTION1);
                    break;
                    case 3:
                    //random weapon
                    switch ((rand() % 5) + 1)
                    {
                        case 1:
                        if (plr->ammo[1] < 12)
                        {
                            plr->weapon[1] = 1;
                            plr->ammo[1] = 40;
                            StatusMessage(360, "Dagger");
                            SND_Sound(S_POTION1);
                            autoweaponchange(1);
                            addscoreamount(10);
                        }
                        break;
                        case 2:
                        if (plr->ammo[3] < 12)
                        {
                            plr->weapon[3] = 1;
                            plr->ammo[3] = 55;
                            StatusMessage(360, "Morning Star");
                            SND_Sound(S_POTION1);
                            autoweaponchange(3);
                            addscoreamount(20);
                        }
                        break;
                        case 3:
                        if (plr->ammo[2] < 12)
                        {
                            plr->weapon[2] = 1;
                            plr->ammo[2] = 30;
                            StatusMessage(360, "Short Sword");
                            SND_Sound(S_POTION1);
                            autoweaponchange(2);
                            addscoreamount(10);
                        }
                        break;
                        case 4:
                        if (plr->ammo[5] < 12)
                        {
                            plr->weapon[5] = 1;
                            plr->ammo[5] = 100;
                            StatusMessage(360, "Battle axe");
                            SND_Sound(S_POTION1);
                            autoweaponchange(5);
                            addscoreamount(30);
                        }
                        break;
                        case 5:
                        if (plr->weapon[7] == 1)
                        {
                            plr->weapon[7] = 2;
                            plr->ammo[7] = 1;
                            StatusMessage(360, "Pike axe");
                            deletesprite(i);
                            SND_Sound(S_POTION1);
                            addscoreamount(30);
                        }
                        if (plr->weapon[7] == 2)
                        {
                            //plr->weapon[7] = 2;
                            plr->ammo[7]++;
                            StatusMessage(360, "Pike axe");
                            deletesprite(i);
                            SND_Sound(S_POTION1);
                            addscoreamount(30);
                        }
                        if (plr->weapon[7] < 1)
                        {
                            if (plr->ammo[7] < 12)
                            {
                                plr->weapon[7] = 1;
                                plr->ammo[7] = 30;
                                StatusMessage(360, "Pike axe");
                                deletesprite(i);
                                SND_Sound(S_POTION1);
                                autoweaponchange(7);
                                addscoreamount(30);
                            }
                        }
                        break;
                    }
                    sprite[i].picnum = OPENCHEST;
                    break;
                    case 4:
                    //random armor
                    switch (krand() & 4)
                    {
                        case 0:
                        StatusMessage(360, "Hero Time");
                        addarmoramount(10);
                        plr->helmettime = 7200;
                        SND_Sound(S_STING1 + rand() % 2);
                        break;
                        case 1:
                        if (plr->armor <= 149)
                        {
                            StatusMessage(360, "Plate Armor");
                            plr->armortype = 3;
                            plr->armor = 0;
                            addarmoramount(150);
                            SND_Sound(S_POTION1);
                            addscoreamount(40);
                        }
                        break;
                        case 2:
                        if (plr->armor <= 99)
                        {
                            StatusMessage(360, "Chain Mail");
                            plr->armortype = 2;
                            plr->armor = 0;
                            addarmoramount(100);
                            SND_Sound(S_POTION1);
                            addscoreamount(20);
                        }
                        break;
                        case 3:
                        if (plr->armor <= 49)
                        {
                            StatusMessage(360, "Leather Armor");
                            plr->armortype = 1;
                            plr->armor = 0;
                            addarmoramount(50);
                            SND_Sound(S_POTION1);
                            addscoreamount(20);
                        }
                        break;
                    }
                    sprite[i].picnum = OPENCHEST;
                    break;
                    case 5:
                    //poison chest
                    if ((krand() & 2) == 0)
                    {
                        plr->poisoned = 1;
                        plr->poisontime = 7200;
                        sethealth(-10);
                        StatusMessage(360, "Poisoned Chest");
                    }
                    else
                    {
                        deletesprite(i);
                        addscoreamount((krand() & 400) + 100);
                        StatusMessage(360, "Treasure Chest");
                        SND_Sound(S_POTION1);
                    }
                    break;
                    case 6:
                    for (j = 0; j < 8; j++)
                        explosion((int)i, sprite[i].x, sprite[i].y, sprite[i].z, sprite[i].owner);
                    playsound_loc(S_EXPLODE, sprite[i].x, sprite[i].y);
                    deletesprite(i);
                    break;
                    default:
                    sprite[i].picnum = OPENCHEST;
                    addscoreamount((rand() % 400) + 100);
                    StatusMessage(360, "Experience Gained");
                    SND_Sound(S_POTION1);
                    break;
                }
                break;

                case FLASKBLUE:
                if (!potionspace(0))
                    break;
                StatusMessage(360, "Health Potion");
                updatepotion(HEALTHPOTION);
                setpotion(0);
                deletesprite(i);
                SND_Sound(S_POTION1);
                addscoreamount(10);
                break;
                case FLASKRED:
                if (!potionspace(3))
                    break;
                StatusMessage(360, "Resist Fire Potion");
                updatepotion(FIREWALKPOTION);
                setpotion(3);
                deletesprite(i);
                SND_Sound(S_POTION1);
                addscoreamount(20);
                break;
                case FLASKGREEN:
                if (!potionspace(1))
                    break;
                StatusMessage(360, "Strength Potion");
                updatepotion(STRENGTHPOTION);
                setpotion(1);
                deletesprite(i);
                SND_Sound(S_POTION1);
                addscoreamount(15);
                break;
                case FLASKOCHRE:
                if (!potionspace(2))
                    break;
                StatusMessage(360, "Cure Poison Potion");
                updatepotion(ARMORPOTION);
                setpotion(2);
                deletesprite(i);
                SND_Sound(S_POTION1);
                addscoreamount(15);
                break;
                case FLASKTAN:
                if (!potionspace(4))
                    break;
                StatusMessage(360, "Invisibility Potion");
                updatepotion(INVISIBLEPOTION);
                setpotion(4);
                deletesprite(i);
                SND_Sound(S_POTION1);
                addscoreamount(30);
                break;
                case DIAMONDRING:
                plr->treasure[0] = 1;
                StatusMessage(360, "DIAMOND RING");
                plr->armor = 0;
                addarmoramount(200);
                plr->armortype = 3;
                deletesprite(i);
                SND_Sound(S_POTION1);
                addscoreamount(25);
                break;
                case SHADOWAMULET:
                plr->treasure[1] = 1;
                StatusMessage(360, "SHADOW AMULET");
                deletesprite(i);
                SND_Sound(S_POTION1);
                plr->shadowtime = 7500;
                addscoreamount(50);
                break;
                case GLASSSKULL:
                plr->treasure[2] = 1;
                StatusMessage(360, "GLASS SKULL");
                deletesprite(i);
                SND_Sound(S_POTION1);

                switch (plr->lvl)
                {
                    case 1:
                    plr->score = 2300;
                    break;
                    case 2:
                    plr->score = 4550;
                    break;
                    case 3:
                    plr->score = 9050;
                    break;
                    case 4:
                    plr->score = 18050;
                    break;
                    case 5:
                    plr->score = 36050;
                    break;
                    case 6:
                    plr->score = 75050;
                    break;
                    case 7:
                    plr->score = 180500;
                    break;
                    case 8:
                    plr->score = 280500;
                    break;
                }
                addscoreamount(10);
                break;
                case AHNK:
                plr->treasure[3] = 1;
                StatusMessage(360, "ANKH");
                plr->health = 0;
                sethealth(250);
                deletesprite(i);
                SND_Sound(S_POTION1);
                addscoreamount(100);
                break;
                case BLUESCEPTER:
                plr->treasure[4] = 1;
                StatusMessage(360, "Water walk scepter");
                deletesprite(i);
                SND_Sound(S_POTION1);
                addscoreamount(10);
                break;
                case YELLOWSCEPTER:
                plr->treasure[5] = 1;
                StatusMessage(360, "Fire walk scepter");
                deletesprite(i);
                SND_Sound(S_POTION1);
                addscoreamount(10);
                break;
                case ADAMANTINERING:
                // ring or protection +5
                plr->treasure[6] = 1;
                StatusMessage(360, "ADAMANTINE RING");
                deletesprite(i);
                SND_Sound(S_POTION1);
                addscoreamount(30);
                break;
                case ONYXRING:
                // protection from missile
                // anit-missile for level only
                // dont forget to cleanup values
                plr->treasure[7] = 1;
                StatusMessage(360, "ONYX RING");
                deletesprite(i);
                SND_Sound(S_POTION1);
                addscoreamount(35);
                break;
                case PENTAGRAM:
                if (sector[plr->sector].lotag == 4002)
                    break;
                else
                {
                    plr->treasure[8] = 1;
                    StatusMessage(360, "PENTAGRAM");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                }
                addscoreamount(100);
                break;
                case CRYSTALSTAFF:
                plr->treasure[9] = 1;
                StatusMessage(360, "CRYSTAL STAFF");
                plr->health = 0;
                sethealth(250);
                plr->armortype = 2;
                plr->armor = 0;
                addarmoramount(300);
                deletesprite(i);
                SND_Sound(S_POTION1);
                addscoreamount(150);
                break;
                case AMULETOFTHEMIST:
                plr->treasure[10] = 1;
                StatusMessage(360, "AMULET OF THE MIST");
                deletesprite(i);
                SND_Sound(S_POTION1);
                plr->invisibletime = 3200;
                addscoreamount(75);
                break;
                case HORNEDSKULL:
                plr->treasure[11] = 1;
                StatusMessage(360, "HORNED SKULL");
                deletesprite(i);
                SND_Sound(S_STING2);
                addscoreamount(750);
                break;
                case THEHORN:
                plr->treasure[12] = 1;
                StatusMessage(360, "Ornate Horn");
                deletesprite(i);
                SND_Sound(S_POTION1);
                plr->vampiretime = 7200;
                // gain 5-10 hp when you kill something
                // for 60 seconds
                addscoreamount(350);
                break;
                case SAPHIRERING:
                plr->treasure[13] = 1;
                StatusMessage(360, "SAPPHIRE RING");
                plr->armortype = 3;
                deletesprite(i);
                SND_Sound(S_POTION1);
                addscoreamount(25);
                break;
                case BRASSKEY:
                plr->treasure[14] = 1;
                StatusMessage(360, "BRASS KEY");
                deletesprite(i);
                SND_Sound(S_POTION1);
                keyspic();
                addscoreamount(15);
                break;
                case BLACKKEY:
                plr->treasure[15] = 1;
                StatusMessage(360, "BLACK KEY");
                deletesprite(i);
                SND_Sound(S_POTION1);
                keyspic();
                addscoreamount(15);
                break;
                case GLASSKEY:
                plr->treasure[16] = 1;
                StatusMessage(360, "GLASS KEY");
                deletesprite(i);
                SND_Sound(S_POTION1);
                keyspic();
                addscoreamount(15);
                break;
                case IVORYKEY:
                plr->treasure[17] = 1;
                StatusMessage(360, "IVORY KEY");
                deletesprite(i);
                SND_Sound(S_POTION1);
                keyspic();
                addscoreamount(15);
                break;
                case SCROLLSCARE:
                if (plr->orbammo[0] < 10)
                {
                    plr->orb[0] = 1;
                    plr->orbammo[0]++;
                    StatusMessage(360, "Scare Scroll");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                }
                break;
                case SCROLLNIGHT:
                if (plr->orbammo[1] < 10)
                {
                    plr->orb[1] = 1;
                    plr->orbammo[1]++;
                    StatusMessage(360, "Night Vision Scroll");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                }
                break;
                case SCROLLFREEZE:
                if (plr->orbammo[2] < 10)
                {
                    plr->orb[2] = 1;
                    plr->orbammo[2]++;
                    StatusMessage(360, "Freeze Scroll");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                }
                break;
                case SCROLLMAGIC:
                if (plr->orbammo[3] < 10)
                {
                    plr->orb[3] = 1;
                    plr->orbammo[3]++;
                    StatusMessage(360, "Magic Arrow Scroll");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                }
                break;
                case SCROLLOPEN:
                if (plr->orbammo[4] < 10)
                {
                    plr->orb[4] = 1;
                    plr->orbammo[4]++;
                    StatusMessage(360, "Open Door Scroll");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                }
                break;
                case SCROLLFLY:
                if (plr->orbammo[5] < 10)
                {
                    plr->orb[5] = 1;
                    plr->orbammo[5]++;
                    StatusMessage(360, "Fly Scroll");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                }
                break;
                case SCROLLFIREBALL:
                if (plr->orbammo[6] < 10)
                {
                    plr->orb[6] = 1;
                    plr->orbammo[6]++;
                    StatusMessage(360, "Fireball Scroll");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                }
                break;
                case SCROLLNUKE:
                if (plr->orbammo[7] < 10)
                {
                    plr->orb[7] = 1;
                    plr->orbammo[7]++;
                    StatusMessage(360, "Nuke Scroll");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                }
                break;
                case QUIVER:
                if (plr->ammo[6] < 100)
                {
                    plr->ammo[6] += 20;
                    if (plr->ammo[6] > 100)
                        plr->ammo[6] = 100;
                    StatusMessage(360, "Quiver of magic arrows");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                    addscoreamount(10);
                    //levelpic();
                }
                break;
                case BOW:
                plr->weapon[6] = 1;
                plr->ammo[6] += 10;
                if (plr->ammo[6] > 100)
                    plr->ammo[6] = 100;
                StatusMessage(360, "Magic bow");
                deletesprite(i);
                SND_Sound(S_POTION1);
                autoweaponchange(6);
                addscoreamount(10);
                break;
                case WEAPON1A:
                if (plr->ammo[1] < 12)
                {
                    plr->weapon[1] = 1;
                    plr->ammo[1] = 40;
                    StatusMessage(360, "Dagger");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                    autoweaponchange(1);
                    addscoreamount(10);
                }
                break;
                case GOBWEAPON:
                if (plr->ammo[2] < 12)
                {
                    plr->weapon[2] = 1;
                    plr->ammo[2] = 20;
                    StatusMessage(360, "Short sword");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                    autoweaponchange(2);
                    addscoreamount(10);
                }
                break;
                case WEAPON2:
                if (plr->ammo[3] < 12)
                {
                    plr->weapon[3] = 1;
                    plr->ammo[3] = 55;
                    StatusMessage(360, "Morning Star");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                    autoweaponchange(3);
                    addscoreamount(20);
                }
                break;
                case WEAPON3:
                if (plr->ammo[4] < 12)
                {
                    plr->weapon[4] = 1;
                    plr->ammo[4] = 80;
                    StatusMessage(360, "Broad Sword");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                    autoweaponchange(4);
                    addscoreamount(30);
                }
                break;
                case WEAPON4:
                if (plr->ammo[5] < 12)
                {
                    plr->weapon[5] = 1;
                    plr->ammo[5] = 100;
                    StatusMessage(360, "Battle axe");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                    autoweaponchange(5);
                    addscoreamount(30);
                }
                break;
                case THROWHALBERD:
                case WEAPON5:
                if (plr->ammo[9] < 12)
                {
                    plr->weapon[9] = 1;
                    plr->ammo[9] = 30;
                    StatusMessage(360, "Halberd");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                    autoweaponchange(9);
                    addscoreamount(30);
                }
                break;
                case SHIELD:
                if (plr->shieldpoints < 100)
                {
                    plr->shieldpoints = 100;
                    deletesprite(i);
                    StatusMessage(360, "Shield");
                    SND_Sound(S_POTION1);
                    addscoreamount(10);
                }
                break;
                case WEAPON5B:
                if (plr->ammo[9] < 12)
                {
                    deletesprite(i);
                }
                break;
                case THROWPIKE + 1:
                if (plr->weapon[7] == 1)
                {
                    plr->weapon[7] = 2;
                    plr->ammo[7] = 2;
                    StatusMessage(360, "Pike axe");
                    deletesprite(i);
                    SND_Sound(S_PICKUPAXE);
                    addscoreamount(30);
                    //levelpic();
                }
                if (plr->weapon[7] == 2)
                {
                    //plr->weapon[7] = 2;
                    plr->ammo[7]++;
                    StatusMessage(360, "Pike axe");
                    deletesprite(i);
                    SND_Sound(S_PICKUPAXE);
                    //score(30);
                    //levelpic();
                }
                if (plr->weapon[7] < 1)
                {
                    if (plr->ammo[7] < 12)
                    {
                        plr->weapon[7] = 1;
                        plr->ammo[7] = 30;
                        StatusMessage(360, "Pike axe");
                        deletesprite(i);
                        SND_Sound(S_POTION1);
                        autoweaponchange(7);
                        addscoreamount(30);
                    }
                }

                break;
                case WEAPON6:
                if (plr->weapon[7] == 1)
                {
                    plr->weapon[7] = 2;
                    plr->ammo[7] = 10;
                    StatusMessage(360, "Pike axe");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                    addscoreamount(30);
                    //levelpic();
                }
                if (plr->weapon[7] == 2)
                {
                    //plr->weapon[7] = 2;
                    plr->ammo[7] += 10;
                    StatusMessage(360, "Pike axe");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                    //score(30);
                    levelpic();
                }
                if (plr->weapon[7] < 1)
                {
                    if (plr->ammo[7] < 12)
                    {
                        plr->weapon[7] = 2;
                        plr->ammo[7] = 10;
                        StatusMessage(360, "Pike axe");
                        deletesprite(i);
                        SND_Sound(S_POTION1);
                        autoweaponchange(7);
                        addscoreamount(30);
                    }
                }
                break;
                case WEAPON7:
                if (plr->ammo[8] < 12)
                {
                    plr->weapon[8] = 1;
                    plr->ammo[8] = 250;
                    StatusMessage(360, "Magic sword");
                    deletesprite(i);
                    SND_Sound(S_POTION1);
                    autoweaponchange(8);
                    addscoreamount(30);
                }
                break;
                case GYSER:
                if (plr->manatime < 1)
                {
                    playsound_loc(S_FIREBALL, sprite[i].x, sprite[i].y);
                    sethealth(-1);
                    startredflash(30);
                }
                break;
                case SPIKEBLADE:
                case SPIKE:
                case SPIKEPOLE:
                if (plr->invincibletime <= 0)
                {
                    sethealth(-plr->health);
                    plr->horiz = 200;
                    plr->spiked = true;
                }
                break;
                case MONSTERBALL:
                if (plr->manatime < 1)
                    sethealth(-1);
                break;
            }
        }
        i = nexti;
    }
}

int potionspace(int vial)
{
    Player* plr = &player[pyrn];

    if (plr->potion[vial] > 9) {
        return 0;
    }
    else {
        return 1;
    }
}

void updatepotion(int vial)
{
    Player* plr = &player[pyrn];

    switch (vial)
    {
        case HEALTHPOTION:
        plr->potion[0]++;
        break;
        case STRENGTHPOTION:
        plr->potion[1]++;
        break;
        case ARMORPOTION:
        plr->potion[2]++;
        break;
        case FIREWALKPOTION:
        plr->potion[3]++;
        break;
        case INVISIBLEPOTION:
        plr->potion[4]++;
        break;
    }
    setpotion(plr->currentpotion);
}

void transformactors(Player* plr)
{
    int32_t i, k;
    tspriteptr_t tspr;

    for (i = 0, tspr = &tsprite[0]; i < spritesortcnt; i++, tspr++)
    {
        switch (tspr->picnum)
        {
            case GRONHAL:
            case GRONSW:
            case GRONSWATTACK:
            case GRONMU:
            case DEVILSTAND:
            case DEVIL:
            case DEVILATTACK:
            case SKULLY:
            case FATWITCH:
            case JUDY:
            case FREDSTAND:
            case FRED:
            case FREDATTACK:
            case GOBLINSTAND:
            case GOBLIN:
            case GOBLINATTACK:
            case MINOTAUR:
            case MINOTAURATTACK:
            k = getangle(tspr->x - plr->x, tspr->y - plr->y);
            k = (((tspr->ang + 3072 + 128 - k) & kAngleMask) >> 8) & 7;
            if (k <= 4)
            {
                tspr->picnum += (k << 2);
                tspr->cstat &= ~4;   //clear x-flipping bit
            }
            else
            {
                tspr->picnum += ((8 - k) << 2);
                tspr->cstat |= 4;    //set x-flipping bit
            }
            break;
            case SKELETON:
            case SKELETONATTACK:
            case KOBOLD:
            k = getangle(tspr->x - plr->x, tspr->y - plr->y);
            k = (((tspr->ang + 3072 + 128 - k) & kAngleMask) >> 8) & 7;
            if (k <= 4)
            {
                tspr->picnum += (k * 5);
                tspr->cstat &= ~4;   //clear x-flipping bit
            }
            else
            {
                tspr->picnum += ((8 - k) * 5);
                tspr->cstat |= 4;    //set x-flipping bit
            }
            break;
            case GRONMUATTACK:
            k = getangle(tspr->x - plr->x, tspr->y - plr->y);
            k = (((tspr->ang + 3072 + 128 - k) & kAngleMask) >> 8) & 7;
            if (k <= 4)
            {
                tspr->picnum += (k * 6);
                tspr->cstat &= ~4;   //clear x-flipping bit
            }
            else
            {
                tspr->picnum += ((8 - k) * 6);
                tspr->cstat |= 4;    //set x-flipping bit
            }
            break;
            case GRONHALATTACK:
            k = getangle(tspr->x - plr->x, tspr->y - plr->y);
            k = (((tspr->ang + 3072 + 128 - k) & kAngleMask) >> 8) & 7;
            if (k <= 4)
            {
                tspr->picnum += (k * 7);
                tspr->cstat &= ~4;   //clear x-flipping bit
            }
            else
            {
                tspr->picnum += ((8 - k) * 7);
                tspr->cstat |= 4;    //set x-flipping bit
            }
            break;
            case RAT:
            k = getangle(tspr->x - plr->x, tspr->y - plr->y);
            k = (((tspr->ang + 3072 + 128 - k) & kAngleMask) >> 8) & 7;
            if (k <= 4)
            {
                tspr->picnum += (k * 2);
                tspr->cstat &= ~4;   //clear x-flipping bit
            }
            else
            {
                tspr->picnum += ((8 - k) * 2);
                tspr->cstat |= 4;    //set x-flipping bit
            }
            break;
            case SPIDER:
            k = getangle(tspr->x - plr->x, tspr->y - plr->y);
            k = (((tspr->ang + 3072 + 128 - k) & kAngleMask) >> 8) & 7;
            if (k <= 4)
            {
                tspr->picnum += (k << 3);
                tspr->cstat &= ~4;   //clear x-flipping bit
            }
            else
            {
                tspr->picnum += ((8 - k) << 3);
                tspr->cstat |= 4;    //set x-flipping bit
            }
            break;
            case GUARDIAN:
            k = getangle(tspr->x - plr->x, tspr->y - plr->y);
            k = (((tspr->ang + 3072 + 128 - k) & kAngleMask) >> 8) & 7;
            if (k <= 4)
            {
                tspr->picnum += k;
                tspr->cstat &= ~4;   //clear x-flipping bit
            }
            else
            {
                tspr->picnum += (8 - k);
                tspr->cstat |= 4;    //set x-flipping bit
            }
            break;
        }

        /*
        k = tspr->statnum;
        if ((k >= 1) && (k <= 8) && (k != 2))  //Interpolate moving sprite
        {
            ospr = &osprite[tspr->owner];
            k = tspr->x-ospr->x; tspr->x = ospr->x;
            if (k != 0) tspr->x += mulscale(k,smoothratio,16);
            k = tspr->y-ospr->y; tspr->y = ospr->y;
            if (k != 0) tspr->y += mulscale(k,smoothratio,16);
            k = tspr->z-ospr->z; tspr->z = ospr->z;
            if (k != 0) tspr->z += mulscale(k,smoothratio,16);
        }
        */
    }
}

void newstatus(short sn, int  seq)
{
    //struct    spritetype    *spriteptr=&sprite[0];
    int j;

    switch (seq)
    {
        case WARPFX:
        changespritestat(sn, WARPFX);
        sprite[sn].lotag = 12;
        break;
        case CHARCOAL:
        changespritestat(sn, CHARCOAL);
        sprite[sn].lotag = 24;
        break;
        case BROKENVASE:
        changespritestat(sn, BROKENVASE);
        switch (sprite[sn].picnum)
        {
            case VASEA:
            playsound_loc(S_GLASSBREAK1 + (rand() % 3), sprite[sn].x, sprite[sn].y);
            sprite[sn].picnum = SHATTERVASE;
            break;
            case VASEB:
            playsound_loc(S_GLASSBREAK1 + (rand() % 3), sprite[sn].x, sprite[sn].y);
            sprite[sn].picnum = SHATTERVASE2;
            break;
            case VASEC:
            playsound_loc(S_GLASSBREAK1 + (rand() % 3), sprite[sn].x, sprite[sn].y);
            sprite[sn].picnum = SHATTERVASE3;
            break;
            case FBARRELFALL:
            case BARREL:
            playsound_loc(S_BARRELBREAK, sprite[sn].x, sprite[sn].y);
            sprite[sn].picnum = FSHATTERBARREL;
            break;
        }
        sprite[sn].lotag = 12;
        sprite[sn].cstat &= ~3;
        break;
        case DRAIN:
        changespritestat(sn, DRAIN);
        sprite[sn].lotag = 24;
        sprite[sn].pal = 7;
        break;
        case ANIMLEVERDN:
        playsound_loc(S_PULLCHAIN1, sprite[sn].x, sprite[sn].y);
        sprite[sn].picnum = LEVERUP;
        changespritestat(sn, ANIMLEVERDN);
        sprite[sn].lotag = 24;
        break;
        case ANIMLEVERUP:
        playsound_loc(S_PULLCHAIN1, sprite[sn].x, sprite[sn].y);
        sprite[sn].picnum = LEVERDOWN;
        changespritestat(sn, ANIMLEVERUP);
        sprite[sn].lotag = 24;
        break;
        case PULLTHECHAIN:
        playsound_loc(S_PULLCHAIN1, sprite[sn].x, sprite[sn].y);
        sprite[sn].picnum = PULLCHAIN1;
        changespritestat(sn, PULLTHECHAIN);
        SND_PlaySound(S_CHAIN1, 0, 0, 0, 0);
        sprite[sn].lotag = 24;
        break;
        case FROZEN:
        //JSA_NEW
        playsound_loc(S_FREEZE, sprite[sn].x, sprite[sn].y);
        changespritestat(sn, FROZEN);
        sprite[sn].lotag = 3600;
        break;
        case DEVILFIRE:
        changespritestat(sn, DEVILFIRE);
        sprite[sn].lotag = (krand() & 120) + 360;
        break;
        case DRIP:
        changespritestat(sn, DRIP);
        break;
        case BLOOD:
        changespritestat(sn, BLOOD);
        break;
        case WAR:
        changespritestat(sn, WAR);
        break;
        case PAIN:
        sprite[sn].lotag = 36;
        switch (sprite[sn].picnum)
        {
            case JUDY:
            case JUDYSIT:
            case JUDYATTACK1:
            case JUDYATTACK2:
            sprite[sn].lotag = 24;
            sprite[sn].picnum = JUDY;
            changespritestat(sn, PAIN);
            break;
            case FATWITCH:
            case FATWITCHATTACK:
            sprite[sn].lotag = 24;
            sprite[sn].picnum = FATWITCHDIE;
            changespritestat(sn, PAIN);
            break;
            case SKULLY:
            case SKULLYATTACK:
            sprite[sn].lotag = 24;
            sprite[sn].picnum = SKULLYDIE;
            changespritestat(sn, PAIN);
            break;
            case GUARDIAN:
            sprite[sn].lotag = 24;
            //sprite[sn].picnum=GUARDIANATTACK;
            playsound_loc(S_GUARDIANPAIN1 + (rand() % 2), sprite[sn].x, sprite[sn].y);
            sprite[sn].picnum = GUARDIANCHAR;
            changespritestat(sn, PAIN);
            break;
            case GRONHALPAIN:
            case GRONHAL:
            case GRONHALATTACK:
            sprite[sn].lotag = 24;
            sprite[sn].picnum = GRONHALPAIN;
            changespritestat(sn, PAIN);
            playsound_loc(S_GRONPAINA + rand() % 3, sprite[sn].x, sprite[sn].y);
            break;
            case GRONSWPAIN:
            case GRONSW:
            case GRONSWATTACK:
            sprite[sn].lotag = 24;
            sprite[sn].picnum = GRONSWPAIN;
            changespritestat(sn, PAIN);
            playsound_loc(S_GRONPAINA + rand() % 3, sprite[sn].x, sprite[sn].y);
            break;
            case GRONMUPAIN:
            case GRONMU:
            case GRONMUATTACK:
            sprite[sn].lotag = 24;
            sprite[sn].picnum = GRONMUPAIN;
            changespritestat(sn, PAIN);
            playsound_loc(S_GRONPAINA + rand() % 3, sprite[sn].x, sprite[sn].y);
            break;
            case KOBOLD:
            case KOBOLDATTACK:
            sprite[sn].picnum = KOBOLDDIE;
            changespritestat(sn, PAIN);
            playsound_loc(S_KPAIN1 + (rand() % 2), sprite[sn].x, sprite[sn].y);
            break;
            case DEVILSTAND:
            case DEVIL:
            case DEVILATTACK:
            playsound_loc(S_MPAIN1, sprite[sn].x, sprite[sn].y);
            sprite[sn].picnum = DEVILPAIN;
            changespritestat(sn, PAIN);
            break;
            case FREDSTAND:
            case FRED:
            case FREDATTACK:
            case FREDPAIN:
            sprite[sn].picnum = FREDPAIN;
            changespritestat(sn, PAIN);
            break;
            case GOBLINSTAND:
            case GOBLIN:
            case GOBLINCHILL:
            case GOBLINSURPRISE:
            case GOBLINATTACK:
            case GOBLINPAIN:
            sprite[sn].picnum = GOBLINPAIN;
            changespritestat(sn, PAIN);
            playsound_loc(S_GOBPAIN1 + (rand() % 2), sprite[sn].x, sprite[sn].y);
            break;
            case MINOTAUR:
            case MINOTAURATTACK:
            case MINOTAURPAIN:
            sprite[sn].picnum = MINOTAURPAIN;
            changespritestat(sn, PAIN);
            playsound_loc(S_MPAIN1, sprite[sn].x, sprite[sn].y);
            break;
            default:
            changespritestat(sn, FLEE);
            break;
        }
        break;
        case FLOCKSPAWN:
        sprite[sn].lotag = 36;
        sprite[sn].extra = 10;
        changespritestat(sn, FLOCKSPAWN);
        break;
        case FLOCK:
        sprite[sn].lotag = 128;
        sprite[sn].extra = 0;
        sprite[sn].pal = 0;
        changespritestat(sn, FLOCK);
        break;
        case FINDME:
        sprite[sn].lotag = 360;
        if (sprite[sn].picnum == RAT)
        {
            sprite[sn].ang = (((krand() & 512) - 256) + sprite[sn].ang + 1024) & kAngleMask;  // NEW
            changespritestat(sn, FLEE);
        }
        else
            changespritestat(sn, FINDME);
        break;
        case SKIRMISH:
        sprite[sn].lotag = 60;
        if (sprite[sn].picnum == RAT)
        {
            sprite[sn].ang = (((krand() & 512) - 256) + sprite[sn].ang + 1024) & kAngleMask;  // NEW
            changespritestat(sn, FLEE);
        }
        else
            changespritestat(sn, SKIRMISH);
        break;
        case CHILL:
        sprite[sn].lotag = 60;
        changespritestat(sn, CHILL);
        break;
        case WITCHSIT:
        sprite[sn].lotag = 12;
        changespritestat(sn, WITCHSIT);
        break;
        case DORMANT:
        sprite[sn].lotag = (krand() & 2047) + 2047;
        break;
        case ACTIVE:
        sprite[sn].lotag = 360;
        break;
        case FLEE:
        changespritestat(sn, FLEE);
        if (sprite[sn].picnum == DEVILATTACK &&
            sprite[sn].picnum == DEVIL)
            sprite[sn].lotag = 120 + (rand() & 360);
        else
            sprite[sn].lotag = 60;
        break;
        case BOB:
        changespritestat(sn, BOB);
        break;
        case LIFTUP:
        //JSA_DEMO3
        if (cartsnd == -1)
        {
            playsound_loc(S_CLUNK, sprite[sn].x, sprite[sn].y);
            cartsnd = SND_PlaySound(S_CHAIN1, sprite[sn].x, sprite[sn].y, 0, 5);
        }

        changespritestat(sn, LIFTUP);
        break;
        case LIFTDN:
        //JSA_DEMO3
        if (cartsnd == -1)
        {
            playsound_loc(S_CLUNK, sprite[sn].x, sprite[sn].y);
            cartsnd = SND_PlaySound(S_CHAIN1, sprite[sn].x, sprite[sn].y, 0, 5);
        }

        changespritestat(sn, LIFTDN);
        break;
        case SHOVE:
        sprite[sn].lotag = 128;
        changespritestat(sn, SHOVE);
        break;
        case SHATTER:
        changespritestat(sn, SHATTER);
        switch (sprite[sn].picnum)
        {
            case FBARRELFALL:
            sprite[sn].picnum = FSHATTERBARREL;
            break;
        }
        break;
        case YELL:
        changespritestat(sn, YELL);
        sprite[sn].lotag = 12;
        break;
        case ATTACK2:
        sprite[sn].lotag = 40;
        changespritestat(sn, ATTACK2);
        sprite[sn].picnum = DRAGONATTACK2;
        playsound_loc(S_DRAGON1 + (rand() % 3), sprite[sn].x, sprite[sn].y);
        fallthrough__;

        case ATTACK:
        sprite[sn].lotag = 64;
        changespritestat(sn, ATTACK);
        switch (sprite[sn].picnum)
        {
            case GRONHAL:
            sprite[sn].picnum = GRONHALATTACK;
            break;
            case GRONMU:
            sprite[sn].picnum = GRONMUATTACK;
            break;
            case GRONSW:
            sprite[sn].picnum = GRONSWATTACK;
            break;
            case KOBOLD:
            sprite[sn].picnum = KOBOLDATTACK;
            if (rand() % 10 > 4)
                playsound_loc(S_KSNARL1 + (rand() % 4), sprite[sn].x, sprite[sn].y);
            break;
            case DRAGON:
            if ((rand() % 10) > 3)
                playsound_loc(S_DRAGON1 + (rand() % 2), sprite[sn].x, sprite[sn].y);

            sprite[sn].picnum = DRAGONATTACK;
            break;
            case DEVILSTAND:
            case DEVIL:
            if ((rand() % 10) > 4)
                playsound_loc(S_DEMON1 + (rand() % 5), sprite[sn].x, sprite[sn].y);

            sprite[sn].picnum = DEVILATTACK;
            break;
            case FREDSTAND:
            case FRED:
            sprite[sn].picnum = FREDATTACK;
            /* RAF */

            break;
            case SKELETON:
            sprite[sn].picnum = SKELETONATTACK;
            break;
            case GOBLINSTAND:
            case GOBLIN:
            if ((rand() % 10) > 5)
                playsound_loc(S_GOBLIN1 + (rand() % 3), sprite[sn].x, sprite[sn].y);

            sprite[sn].picnum = GOBLINATTACK;
            break;
            case MINOTAUR:
            if ((rand() % 10) > 4)
                playsound_loc(S_MSNARL1 + (rand() % 3), sprite[sn].x, sprite[sn].y);

            sprite[sn].picnum = MINOTAURATTACK;
            break;
            case SKULLY:
            sprite[sn].picnum = SKULLYATTACK;
            break;
            case FATWITCH:
            if ((rand() % 10) > 4)
                playsound_loc(S_FATLAUGH, sprite[sn].x, sprite[sn].y);
            sprite[sn].picnum = FATWITCHATTACK;
            break;
            case JUDY:
            //sprite[sn].cstat=0;
            if (krand() % 2 == 0)
                sprite[sn].picnum = JUDYATTACK1;
            else
                sprite[sn].picnum = JUDYATTACK2;
            break;
            case WILLOW:
            playsound_loc(S_WISP + (rand() % 2), sprite[sn].x, sprite[sn].y);
            sprite[sn].pal = 7;
            break;
            case GUARDIAN:
            playsound_loc(S_GUARDIAN1 + (rand() % 2), sprite[sn].x, sprite[sn].y);
            sprite[sn].picnum = GUARDIANATTACK;
            break;
        }
        break;
        case FACE:
        changespritestat(sn, FACE);
        break;
        case STAND:
        //changespritestat(sn,STAND);
        changespritestat(sn, FACE);
        sprite[sn].lotag = 0;
        break;
        case CHASE:
        if (sprite[sn].picnum == RAT)
            changespritestat(sn, FLEE);
        else
            changespritestat(sn, CHASE);
        sprite[sn].lotag = 256;
        switch (sprite[sn].picnum)
        {
            //case GRONHAL:
            case GRONHALATTACK:
            if (sprite[sn].extra > 2)
                sprite[sn].picnum = GRONHAL;
            else
                sprite[sn].picnum = GRONMU;
            break;
            case GRONSWATTACK:
            sprite[sn].picnum = GRONSW;
            break;
            case GRONMUATTACK:
            if (sprite[sn].extra > 0)
                sprite[sn].picnum = GRONMU;
            else
                sprite[sn].picnum = GRONSW;
            break;
            case KOBOLDATTACK:
            sprite[sn].picnum = KOBOLD;
            break;
            case DRAGONATTACK2:
            case DRAGONATTACK:
            sprite[sn].picnum = DRAGON;
            break;
            case DEVILATTACK:
            sprite[sn].picnum = DEVIL;
            break;
            case FREDATTACK:
            sprite[sn].picnum = FRED;
            /* RAF */

            break;
            case SKELETONATTACK:
            sprite[sn].picnum = SKELETON;
            break;
            case GOBLINATTACK:
            /* RAF */
//JSA_DEMO3
            if (rand() % 10 > 2)
                playsound_loc(S_GOBLIN1 + (krand() % 3), sprite[sn].x, sprite[sn].y);

            sprite[sn].picnum = GOBLIN;
            break;
            case MINOTAURATTACK:
            //JSA_DEMO3
            playsound_loc(S_MSNARL1 + (rand() % 4), sprite[sn].x, sprite[sn].y);
            sprite[sn].picnum = MINOTAUR;
            break;
            case SKULLYATTACK:
            sprite[sn].picnum = SKULLY;
            break;
            case FATWITCHATTACK:
            sprite[sn].picnum = FATWITCH;
            break;
            case JUDYATTACK1:
            case JUDYATTACK2:
            //sprite[sn].cstat=0x03;
            sprite[sn].picnum = JUDY;
            break;
            case GUARDIANATTACK:
            sprite[sn].picnum = GUARDIAN;
            break;
            case WILLOW:
            sprite[sn].pal = 6;
            break;
        }
        break;
        case MISSILE:
        changespritestat(sn, MISSILE);
        break;
        case CAST:
        changespritestat(sn, CAST);
        sprite[sn].lotag = 12;
        switch (sprite[sn].picnum)
        {
            case GRONHALATTACK:
            sprite[sn].lotag = 24;
            break;
            case GRONMUATTACK:
            sprite[sn].lotag = 36;
            break;
        }
        break;
        case FX:
        changespritestat(sn, FX);
        break;
        case DIE:
        sprite[sn].cstat &= ~3;
        switch (sprite[sn].picnum)
        {
            case GRONHALPAIN:
            case GRONHAL:
            case GRONHALATTACK:
            sprite[sn].lotag = 20;
            sprite[sn].picnum = GRONHALDIE;
            playsound_loc(S_GRONDEATHA + rand() % 3, sprite[sn].x, sprite[sn].y);
            break;
            case GRONSWPAIN:
            case GRONSW:
            case GRONSWATTACK:
            sprite[sn].lotag = 20;
            sprite[sn].picnum = GRONSWDIE;
            playsound_loc(S_GRONDEATHA + rand() % 3, sprite[sn].x, sprite[sn].y);
            break;
            case GRONMUPAIN:
            case GRONMU:
            case GRONMUATTACK:
            sprite[sn].lotag = 20;
            sprite[sn].picnum = GRONMUDIE;
            playsound_loc(S_GRONDEATHA + rand() % 3, sprite[sn].x, sprite[sn].y);
            break;
            case FISH:
            case RAT:
            sprite[sn].lotag = 20;
            break;
            case KOBOLD:
            case KOBOLDATTACK:
            playsound_loc(S_KDIE1 + (rand() % 2), sprite[sn].x, sprite[sn].y);
            sprite[sn].lotag = 20;
            sprite[sn].picnum = KOBOLDDIE;
            break;
            case DRAGON:
            case DRAGONATTACK2:
            case DRAGONATTACK:
            playsound_loc(S_DEMONDIE1 + (rand() % 2), sprite[sn].x, sprite[sn].y);
            sprite[sn].lotag = 20;
            sprite[sn].picnum = DRAGONDIE;

            break;
            case DEVILSTAND:
            case DEVIL:
            case DEVILATTACK:
            playsound_loc(S_DEMONDIE1 + (rand() % 2), sprite[sn].x, sprite[sn].y);
            sprite[sn].lotag = 20;
            sprite[sn].picnum = DEVILDIE;
            break;
            case FREDSTAND:
            case FRED:
            case FREDATTACK:
            sprite[sn].lotag = 20;
            sprite[sn].picnum = FREDDIE;
            break;
            case SKELETON:
            case SKELETONATTACK:
            playsound_loc(S_SKELETONDIE, sprite[sn].x, sprite[sn].y);
            sprite[sn].lotag = 20;
            sprite[sn].picnum = SKELETONDIE;
            break;
            case GOBLINSTAND:
            case GOBLIN:
            case GOBLINCHILL:
            case GOBLINSURPRISE:
            case GOBLINATTACK:
            case GOBLINDIE:
            playsound_loc(S_GOBDIE1 + (rand() % 3), sprite[sn].x, sprite[sn].y);
            sprite[sn].lotag = 20;
            sprite[sn].picnum = GOBLINDIE;
            break;
            case MINOTAUR:
            case MINOTAURATTACK:
            playsound_loc(S_MDEATH1, sprite[sn].x, sprite[sn].y);
            sprite[sn].lotag = 10;
            sprite[sn].picnum = MINOTAURDIE;
            break;
            case SPIDER:
            sprite[sn].lotag = 10;
            sprite[sn].picnum = SPIDERDIE;
            break;
            case SKULLY:
            case SKULLYATTACK:
            sprite[sn].lotag = 20;
            sprite[sn].picnum = SKULLYDIE;
            playsound_loc(S_SKULLWITCHDIE, sprite[sn].x, sprite[sn].y);
            break;
            case FATWITCH:
            case FATWITCHATTACK:
            sprite[sn].lotag = 20;
            sprite[sn].picnum = FATWITCHDIE;
            playsound_loc(S_FATWITCHDIE, sprite[sn].x, sprite[sn].y);
            break;
            case JUDYSIT:
            case JUDYSTAND:
            case JUDY:
            case JUDYATTACK1:
            case JUDYATTACK2:
            //sprite[sn].cstat=0;
            sprite[sn].lotag = 20;
            if (mapon < 24)
            {
                for (j = 0; j < 8; j++)
                    trailingsmoke(sn);
                deletesprite(sn);
                goto outathere;
            }
            else
            {
                sprite[sn].picnum = JUDYDIE;
                playsound_loc(S_JUDYDIE, sprite[sn].x, sprite[sn].y);
            }
            break;
            case GUARDIAN:
            case GUARDIANATTACK:
            playsound_loc(S_GUARDIANDIE, sprite[sn].x, sprite[sn].y);
            for (j = 0; j < 4; j++)
                explosion((int)sn, sprite[sn].x, sprite[sn].y, sprite[sn].z, sprite[sn].owner);
            deletesprite(sn);
            addscoreamount(1500);
            goto outathere;
            break;
            case WILLOW:
            playsound_loc(S_WILLOWDIE, sprite[sn].x, sprite[sn].y);
            sprite[sn].pal = 0;
            sprite[sn].lotag = 20;
            sprite[sn].picnum = WILLOWEXPLO;
            break;
        }
        changespritestat(sn, DIE);
    outathere:
        break;

        case RESURECT:
        sprite[sn].lotag = 7200;
        switch (sprite[sn].picnum)
        {
            case GRONDEAD:
            sprite[sn].picnum = GRONDEAD;
            sprite[sn].cstat &= ~3;
            sprite[sn].extra = 3;
            changespritestat(sn, RESURECT);
            addscoreamount(200);
            break;
            case FISH:
            case RAT:
            sprite[sn].picnum = RAT;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, RESURECT);
            addscoreamount(5);
            break;
            case KOBOLDDEAD:
            sprite[sn].picnum = KOBOLDDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, RESURECT);
            addscoreamount(10);
            break;
            case DRAGONDEAD:
            sprite[sn].picnum = DRAGONDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, RESURECT);
            addscoreamount(4000);
            break;
            case DEVILDEAD:
            trailingsmoke(sn);
            sprite[sn].picnum = DEVILDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, RESURECT);
            addscoreamount(75);
            break;
            case FREDDEAD:
            sprite[sn].picnum = FREDDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, RESURECT);
            addscoreamount(40);
            break;
            case SKELETONDEAD:
            sprite[sn].picnum = SKELETONDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, RESURECT);
            addscoreamount(10);
            break;
            case GOBLINDEAD:
            sprite[sn].picnum = GOBLINDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, RESURECT);
            addscoreamount(25);
            break;
            case MINOTAURDEAD:
            sprite[sn].picnum = MINOTAURDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, RESURECT);
            addscoreamount(170);
            break;
            case SPIDERDEAD:
            sprite[sn].picnum = SPIDERDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, RESURECT);
            addscoreamount(5);
            break;
            case SKULLYDEAD:
            sprite[sn].picnum = SKULLYDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, RESURECT);
            addscoreamount(1000);
            break;
            case FATWITCHDEAD:
            sprite[sn].picnum = FATWITCHDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, RESURECT);
            addscoreamount(900);
            break;
            case JUDYDEAD:
            sprite[sn].picnum = JUDYDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, RESURECT);
            addscoreamount(7000);
            break;
            case GUARDIAN:
            case GUARDIANATTACK:
            case WILLOW:
            changespritestat(sn, 0);
            deletesprite((short)sn);
            addscoreamount(1500);
            break;
        }
        break;

        case DEAD:
        switch (sprite[sn].picnum)
        {
            case FISH:
            case RAT:
            sprite[sn].cstat &= ~3;
            changespritestat(sn, DEAD);
            addscoreamount(5);
            break;
            case GRONDEAD:
            sprite[sn].picnum = GRONDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, DEAD);
            addscoreamount(200);
            monsterweapon(sn);
            break;
            case KOBOLDDEAD:
            sprite[sn].picnum = KOBOLDDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, DEAD);
            addscoreamount(10);
            break;
            case DRAGONDEAD:
            sprite[sn].picnum = DRAGONDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, DEAD);
            addscoreamount(4000);
            break;
            case DEVILDEAD:
            trailingsmoke(sn);
            sprite[sn].picnum = DEVILDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, DEAD);
            addscoreamount(50);
            break;
            case FREDDEAD:
            sprite[sn].picnum = FREDDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, DEAD);
            addscoreamount(40);
            break;
            case SKELETONDEAD:
            sprite[sn].picnum = SKELETONDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, DEAD);
            break;
            case GOBLINDEAD:
            sprite[sn].picnum = GOBLINDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, DEAD);
            addscoreamount(25);
            if (rand() % 100 > 60)
                monsterweapon(sn);
            break;
            case MINOTAURDEAD:
            sprite[sn].picnum = MINOTAURDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, DEAD);
            addscoreamount(70);
            if (rand() % 100 > 60)
                monsterweapon(sn);
            break;
            case SPIDERDEAD:
            sprite[sn].picnum = SPIDERDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, DEAD);
            addscoreamount(5);
            break;
            case SKULLYDEAD:
            sprite[sn].picnum = SKULLYDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, DEAD);
            addscoreamount(100);
            break;
            case FATWITCHDEAD:
            sprite[sn].picnum = FATWITCHDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, DEAD);
            addscoreamount(900);
            break;
            case JUDYDEAD:
            sprite[sn].picnum = JUDYDEAD;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, DEAD);
            spawnapentagram(sn);
            addscoreamount(7000);
            break;
            case WILLOWEXPLO + 2:
            sprite[sn].pal = 0;
            sprite[sn].cstat &= ~3;
            changespritestat(sn, 0);
            deletesprite(sn);
            addscoreamount(150);
            break;
        }
        if (sector[sprite[sn].sectnum].floorpicnum == WATER || sector[sprite[sn].sectnum].floorpicnum == SLIME)
        {
            if (sprite[sn].picnum == MINOTAURDEAD)
            {
                sprite[sn].z += (8 << 8);

                vec3_t pos;
                pos.x = sprite[sn].x;
                pos.y = sprite[sn].y;
                pos.z = sprite[sn].z;

                setsprite(sn, &pos);
            }
        }
        if (sector[sprite[sn].sectnum].floorpicnum == LAVA
            || sector[sprite[sn].sectnum].floorpicnum == LAVA1
            || sector[sprite[sn].sectnum].floorpicnum == LAVA2)
        {
            trailingsmoke(sn);
            deletesprite(sn);
        }
        break;
    }
    //
    // the control variable for monster release
    //
}

void firebreath(int i, int a, int b, int c)
{
    Player* plr = &player[pyrn];

    for (int k = 0; k <= a; k++)
    {
        int32_t j = insertsprite(sprite[i].sectnum, MISSILE);
        sprite[j].x = sprite[i].x;
        sprite[j].y = sprite[i].y;

        if (c == LOW) {
            sprite[j].z = sector[sprite[i].sectnum].floorz - (32 << 8);
        }
        else {
            sprite[j].z = sector[sprite[i].sectnum].floorz - (tilesiz[sprite[i].picnum].y << 7);
        }

        sprite[j].cstat = 0;
        sprite[j].picnum = MONSTERBALL;
        sprite[j].shade = -15;
        sprite[j].xrepeat = 128;
        sprite[j].yrepeat = 128;
        sprite[j].ang = ((((getangle(plr->x - sprite[j].x, plr->y - sprite[j].y) + (krand() & 15) - 8) + 2048) + ((b * 22) + (k * 10))) & kAngleMask);
        sprite[j].xvel = Cos(sprite[j].ang + 2048) >> 6;
        sprite[j].yvel = Sin(sprite[i].ang + 2048) >> 6;

        int32_t discrim = ksqrt((plr->x - sprite[j].x) * (plr->x - sprite[j].x)
            + (plr->y - sprite[j].y) * (plr->y - sprite[j].y));

        if (discrim == 0)  {
            discrim = 1;
        }

        if (c == HIGH) {
            sprite[j].zvel = ((plr->z + (32 << 8) - sprite[j].z) << 7) / discrim;
        }
        else {
            sprite[j].zvel = (((plr->z + (8 << 8)) - sprite[j].z) << 7) / discrim;//NEW
        }

        sprite[j].owner = i;
        sprite[j].clipdist = 16;
        sprite[j].lotag = 512;
        sprite[j].hitag = 0;
    }
}

void throwspank(int i)
{
    Player* plr = &player[pyrn];

    //JSA_DEMO3
    playsound_loc(S_WITCHTHROW, sprite[i].x, sprite[i].y);

    int32_t j = insertsprite(sprite[i].sectnum, MISSILE);
    sprite[j].x = sprite[i].x;
    sprite[j].y = sprite[i].y;
    sprite[j].z = sector[sprite[i].sectnum].floorz - ((tilesiz[sprite[i].picnum].y >> 1) << 8);
    sprite[j].cstat = 0;        //Hitscan does not hit other bullets
    sprite[j].picnum = FATSPANK;
    sprite[j].shade = -15;
    sprite[j].xrepeat = 64;
    sprite[j].yrepeat = 64;
    sprite[j].ang = (((getangle(plr->x - sprite[j].x, plr->y - sprite[j].y) + (krand() & 15) - 8) + 2048) & kAngleMask);
    sprite[j].xvel = Cos(sprite[j].ang + 2048) >> 6;
    sprite[j].yvel = Sin(sprite[j].ang + 2048) >> 6;

    int32_t discrim = ksqrt((plr->x - sprite[j].x) * (plr->x - sprite[j].x) + (plr->y - sprite[j].y) * (plr->y - sprite[j].y));
    if (discrim == 0)
        discrim = 1;

    sprite[j].zvel = ((plr->z + (48 << 8) - sprite[j].z) << 7) / discrim;
    sprite[j].owner = i;
    sprite[j].clipdist = 16;
    sprite[j].lotag = 512;
    sprite[j].hitag = 0;
    sprite[j].pal = 0;
}

//cast
void castspell(int i)
{
    Player* plr = &player[pyrn];

    int32_t j = insertsprite(sprite[i].sectnum, MISSILE);
    sprite[j].x = sprite[i].x;
    sprite[j].y = sprite[i].y;

    if (sprite[i].picnum == SPAWNFIREBALL)
    {
        sprite[j].z = sprite[i].z - ((tilesiz[sprite[i].picnum].y >> 1) << 8);
    }
    else
    {
        sprite[j].z = sector[sprite[i].sectnum].floorz - ((tilesiz[sprite[i].picnum].y >> 1) << 8);
    }

    sprite[j].cstat = 0;        //Hitscan does not hit other bullets
    sprite[j].picnum = MONSTERBALL;
    sprite[j].shade = -15;
    sprite[j].xrepeat = 64;
    sprite[j].yrepeat = 64;

    if (sprite[i].picnum == SPAWNFIREBALL)
    {
        sprite[j].ang = ((getangle(plr->x - sprite[j].x, plr->y - sprite[j].y) + 2048) & kAngleMask);
    }
    else
    {
        sprite[j].ang = (((getangle(plr->x - sprite[j].x, plr->y - sprite[j].y) + (krand() & 15) - 8) + 2048) & kAngleMask);
    }

    sprite[j].xvel = Cos(sprite[j].ang + 2048) >> 6;
    sprite[j].yvel = Sin(sprite[j].ang + 2048) >> 6;

    int32_t discrim = ksqrt((plr->x - sprite[j].x) * (plr->x - sprite[j].x) + (plr->y - sprite[j].y) * (plr->y - sprite[j].y));
    if (discrim == 0)
    {
        discrim = 1;
    }
    sprite[j].zvel = ((plr->z + (48 << 8) - sprite[j].z) << 7) / discrim;

    sprite[j].owner = i;
    sprite[j].clipdist = 16;
    sprite[j].lotag = 512;
    sprite[j].hitag = 0;
}

void skullycastspell(int i)
{
    Player* plr = &player[pyrn];

    int32_t j = insertsprite(sprite[i].sectnum, MISSILE);
    sprite[j].x = sprite[i].x;
    sprite[j].y = sprite[i].y;

    if (sprite[i].picnum == SPAWNFIREBALL)
    {
        sprite[j].z = sprite[i].z - ((tilesiz[sprite[i].picnum].y >> 1) << 8);
    }
    else
    {
        sprite[j].z = sector[sprite[i].sectnum].floorz - ((tilesiz[sprite[i].picnum].y >> 1) << 8);
    }

    sprite[j].cstat = 0;        //Hitscan does not hit other bullets
    sprite[j].picnum = PLASMA;
    sprite[j].shade = -15;
    sprite[j].xrepeat = 64;
    sprite[j].yrepeat = 64;

    if (sprite[i].picnum == SPAWNFIREBALL)
    {
        sprite[j].ang = ((getangle(plr->x - sprite[j].x, plr->y - sprite[j].y) + 2048) & kAngleMask);
    }
    else
    {
        sprite[j].ang = (((getangle(plr->x - sprite[j].x, plr->y - sprite[j].y) + (krand() & 15) - 8) + 2048) & kAngleMask);
    }

    sprite[j].xvel = Cos(sprite[j].ang + 2048) >> 6;
    sprite[j].yvel = Sin(sprite[j].ang + 2048) >> 6;

    int32_t discrim = ksqrt((plr->x - sprite[j].x) * (plr->x - sprite[j].x) + (plr->y - sprite[j].y) * (plr->y - sprite[j].y));
    if (discrim == 0)
    {
        discrim = 1;
    }

    sprite[j].zvel = ((plr->z + (48 << 8) - sprite[j].z) << 7) / discrim;

    sprite[j].owner = i;
    sprite[j].clipdist = 16;
    sprite[j].lotag = 512;
    sprite[j].hitag = 0;
    sprite[j].pal = 7;
}

// heatseeker check
int checkheat(int i)
{
    short daang2, daang, dahoriz;
    int32_t daz2;
    int32_t x, y, z, dasectnum;

    Player* plr = &player[pyrn];

    daang = sprite[i].ang;
    dahoriz = plr->horiz;

    x = sprite[i].x;
    y = sprite[i].y;
    z = sprite[i].z;

    dasectnum = sprite[i].sectnum;

    daang2 = ((daang + 2048 + (krand() & 31) - 16) & kAngleMask);
    daz2 = ((100 - dahoriz) * 2000) + ((krand() - 32768) >> 1);

    vec3_t pos;
    pos.x = x;
    pos.y = y;
    pos.z = z;

    hitdata_t hitinfo;

    hitscan(&pos, dasectnum, Cos(daang2 + 2048), Sin(daang2 + 2048), daz2, &hitinfo, CLIPMASK1); // CHECKME - is CLIPMASK1 correct for vesion of this function that hadn't got cliptype param?

    if (hitinfo.sprite > 0 && hitinfo.sprite != plr->spritenum)
    {
        sprite[i].ang = (getangle(sprite[hitinfo.sprite].x - sprite[i].x, sprite[hitinfo.sprite].y - sprite[i].y) & kAngleMask);
        return 1;
    }
    else
    {
        sprite[i].ang = plr->ang;
        if (plr->horiz > 100)
        {
            sprite[i].z -= plr->horiz;
        }
    }
    return 0;
}

int checkfacing(int i, int x, int y)
{
    if (Cos(sprite[i].ang + 2048) * (x - sprite[i].x) + Sin(sprite[i].ang + 2048) * (y - sprite[i].y) >= 0) {
        return 1;
    }
    else {
        return 0;
    }
}

int checkmedusadist(int i, int x, int y, int z, int lvl)
{
    int32_t attackdist = 1024 + (lvl << 9);

    if ((labs(x - sprite[i].x) + labs(y - sprite[i].y) < attackdist)
        && (labs((z >> 8) - ((sprite[i].z >> 8) - (tilesiz[sprite[i].picnum].y >> 1))) <= 120))
    {
        return 1;
    }
    else {
        return 0;
    }
}

int checkdist(int i, int x, int y, int z)
{
    int attackdist;

    switch (sprite[i].picnum)
    {
        case GRONHALATTACK:
        case GRONHAL:
        attackdist = 1024 + 512;
        break;
        case GRONMU:
        case GRONMUATTACK:
        attackdist = 2048;
        break;
        case GRONSW:
        case GRONSWATTACK:
        attackdist = 1024 + 256;
        break;
        case MINOTAUR:
        case MINOTAURATTACK:
        attackdist = 1024 + 512;
        break;
        case FRED:
        case FREDATTACK:
        attackdist = 1024 + 256;
        break;
        case LFIRE:
        case SFIRE:
        case SKELETONATTACK:
        case GOBLIN:
        case GOBLINATTACK:
        case KOBOLD:
        case KOBOLDATTACK:
        attackdist = 1024;
        break;
        case GUARDIAN:
        case GUARDIANATTACK:
        attackdist = 4096;
        break;
        case SKULLY:
        case SKULLYATTACK:
        case FATWITCH:
        case FATWITCHATTACK:
        case JUDY:
        case JUDYATTACK1:
        case JUDYATTACK2:
        case DEVIL:
        case DEVILATTACK:
        attackdist = 2048;
        break;
        case FISH:
        case RAT:
        case WILLOW:
        attackdist = 512;
        break;
        default:
        attackdist = 512;
    }

    if (((sprite[i].picnum > DRAGONATTACK + 2) && (sprite[i].picnum < DRAGONATTACK + 17))
        || ((sprite[i].picnum > DRAGONATTACK2) && (sprite[i].picnum < DRAGONATTACK2 + 5)))
        attackdist = 2048;

    if (sprite[i].picnum == SPIDER)
    {
        if ((labs(x - sprite[i].x) + labs(y - sprite[i].y) < attackdist)
            && (labs((z >> 8) - ((sprite[i].z >> 8) - (tilesiz[sprite[i].picnum].y >> 1))) <= 60))
            return 1;
        else
            return 0;
    }
    else
    {
        if ((labs(x - sprite[i].x) + labs(y - sprite[i].y) < attackdist)
            && (labs((z >> 8) - ((sprite[i].z >> 8) - (tilesiz[sprite[i].picnum].y >> 1))) <= 120))
            return 1;
        else
            return 0;
    }
}

void checkspeed(int i, int* dax, int* day, int speed)
{
    sprite[i].z = sector[sprite[i].sectnum].floorz;

    *dax = Cos(sprite[i].ang) >> speed;
    *day = Sin(sprite[i].ang) >> speed;
}

int checksight(int i, short* daang)
{
    Player* plr = &player[pyrn];

    if (plr->invisibletime > 0)
    {
        *daang = ((krand() & 512) - 256) & kAngleMask;
        return 0;
    }

    if (cansee(plr->x, plr->y, plr->z, plr->sector,
        sprite[i].x, sprite[i].y, sprite[i].z - (tilesiz[sprite[i].picnum].y << 7), sprite[i].sectnum) == 1
        && plr->invisibletime < 0)
    {

        *daang = (getangle(plr->x - sprite[i].x, plr->y - sprite[i].y) & kAngleMask);

        if (((sprite[i].ang + 2048 - *daang) & kAngleMask) < 1024)
            sprite[i].ang = ((sprite[i].ang + 2048 - (synctics << 1)) & kAngleMask);
        else
            sprite[i].ang = ((sprite[i].ang + (synctics << 1)) & kAngleMask);

        return 1;
    }
    else
    {
        daang = 0;
    }

    return 0;
}

void checkmove(int i, int dax, int day, short* movestat)
{
    Player* plr = &player[pyrn];

    *movestat = movesprite(i, (Cos(sprite[i].ang) * synctics) << 3, ((sintable[sprite[i].ang]) * synctics) << 3, 0, 4 << 8, 4 << 8, 2);

    if (*movestat != 0)
    {
        if ((krand() & 1) == 0) {
            sprite[i].ang = (getangle(plr->x - sprite[i].x, plr->y - sprite[i].y) & kAngleMask);
        }
        else {
            sprite[i].ang = ((sprite[i].ang + (synctics)) & kAngleMask);
        }
    }
}

void attack(int i)
{
    int k;

    Player* plr = &player[pyrn];

    if (plr->invincibletime > 0)
        return;

    if (plr->treasure[6] == 1 && (krand() & 32) > 16)
        return;

    if ((krand() & 15) < plr->armortype + 10)
        return;

    if (plr->shieldpoints > 0 && plr->selectedgun < 5)
    {
        if (krand() % 100 > 80)
        {
            playsound_loc(S_SWORD1 + rand() % 3, plr->x, plr->y);
            return;
        }
        else
        {
            if (krand() % 100 > 50)
            {
                plr->shieldpoints -= 2;
                playsound_loc(S_SWORD1 + rand() % 3, plr->x, plr->y);
                return;
            }
            else
            {
                plr->shieldpoints -= 2;
            }
        }

        if (plr->shieldpoints <= 0)
        {
            StatusMessage(360, "Shield useless");
        }
    }

    k = rand() % 100;

    if (k > (plr->armortype << 3))
        k = 15;
    else
        k = 5;

    switch (sprite[i].picnum)
    {
        case SPIDER:
        k = 5;
        fallthrough__; // note this is original source behaviour
        case FISH:
        case RAT:
        k = 3;
        break;
        case SKELETONATTACK:
        playsound_loc(S_RIP1 + (rand() % 3), sprite[i].x, sprite[i].y);
        if (rand() % 2)
            playsound_loc(S_GORE1 + (rand() % 4), sprite[i].x, sprite[i].y);
        if (rand() % 2)
            playsound_loc(S_BREATH1 + (rand() % 6), sprite[i].x, sprite[i].y);
        k >>= 2;
        break;

        case DEVIL:
        case DEVILATTACK:
        playsound_loc(S_DEMONTHROW, sprite[i].x, sprite[i].y);
        k >>= 2;
        break;

        case KOBOLDATTACK:
        playsound_loc(S_GENSWING, sprite[i].x, sprite[i].y);
        if (rand() % 10 > 4)
        {
            playsound_loc(S_KOBOLDHIT, plr->x, plr->y);
            playsound_loc(S_BREATH1 + (rand() % 6), plr->x, plr->y);
        }
        k >>= 2;
        break;
        case FREDATTACK:
        /* RAF */
        k >>= 3;
        break;
        case GOBLINATTACK:
        playsound_loc(S_GENSWING, sprite[i].x, sprite[i].y);
        if (rand() % 10 > 4)
            playsound_loc(S_SWORD1 + (rand() % 6), sprite[i].x, sprite[i].y);
        k >>= 2;
        break;
        case GRONSWATTACK:
        if (sprite[i].shade > 30)
            k >>= 1;
        playsound_loc(S_GENSWING, sprite[i].x, sprite[i].y);
        if (rand() % 10 > 3)
            playsound_loc(S_SWORD1 + (rand() % 6), sprite[i].x, sprite[i].y);

        break;
        case MINOTAURATTACK:
        playsound_loc(S_GENSWING, sprite[i].x, sprite[i].y);
        if (rand() % 10 > 4)
            playsound_loc(S_SWORD1 + (rand() % 6), sprite[i].x, sprite[i].y);

        //
        // GET RID OF THIS
        //
        //k>>=1;// JUST FOR THE DEMO
        //
        //
        break;
    }

    if (plr->shieldpoints > 0)
        k >>= 1;

    switch (plr->armortype)
    {
        case 0: //none
        sethealth(-k);
        break;
        case 1: //leather
        addarmoramount(-4);
        sethealth(-k >> 1);
        fallthrough__;
        case 2: //chain
        addarmoramount(-2);
        sethealth(-k >> 2);
        break;
        case 3: //plate
        addarmoramount(-1);
        if ((krand() & 32) > 24)
        {
            sethealth(-k >> 2);
        }
        break;
    }

    startredflash(k);
    setbrightness(gbrightness);
}

void fireballblast(int32_t i)
{
    for (int k = 0; k < 8; k++)
    {
        int32_t j = insertsprite(sprite[i].sectnum, FX);
        sprite[j].x = sprite[i].x;
        sprite[j].y = sprite[i].y;
        sprite[j].z = sprite[i].z;

        sprite[j].cstat = 2;
        sprite[j].picnum = FIREBALL;
        sprite[j].shade = 0;

        sprite[j].xrepeat = 8;    // 25% normal size
        sprite[j].yrepeat = 8;    // 25% normal size
        sprite[j].ang = (k << 8);
        sprite[j].xvel = ((krand() & 511) - 256) << 3;
        sprite[j].yvel = ((krand() & 511) - 256) << 3;
        sprite[j].zvel = ((krand() & 511) - 256) << 3;

        sprite[j].owner = sprite[i].owner;
        sprite[j].lotag = 196;
        sprite[j].hitag = 0;
    }
}

void makeafire(int i, int firetype)
{
    int32_t j = insertsprite(sprite[i].sectnum, FIRE);

    sprite[j].x = sprite[i].x + (rand() & 1024) - 512;
    sprite[j].y = sprite[i].y + (rand() & 1024) - 512;
    sprite[j].z = sprite[i].z;

    sprite[j].cstat = 0;
    sprite[j].xrepeat = 64;
    sprite[j].yrepeat = 64;

    sprite[j].shade = 0;

    sprite[j].clipdist = 64;
    sprite[j].owner = sprite[i].owner;
    sprite[j].lotag = 2047;
    sprite[j].hitag = 0;
    changespritestat(j, FIRE);
}

void explosion(int i, int x, int y, int z, short owner)
{
    int32_t j = insertsprite(sprite[i].sectnum, EXPLO);

    sprite[j].x = x + (krand() & 1024) - 512;
    sprite[j].y = y + (krand() & 1024) - 512;
    sprite[j].z = z;

    sprite[j].cstat = 0;        //Hitscan does not hit smoke on wall
    sprite[j].cstat &= ~3;
    sprite[j].picnum = MONSTERBALL;
    sprite[j].shade = -15;
    sprite[j].xrepeat = 64;
    sprite[j].yrepeat = 64;
    sprite[j].ang = krand() & kAngleMask;
    sprite[j].xvel = ((krand() & 511) - 256);
    sprite[j].yvel = ((krand() & 511) - 256);
    sprite[j].zvel = ((krand() & 511) - 256);
    sprite[j].owner = sprite[i].owner;
    sprite[j].lotag = 256;
    sprite[j].hitag = 0;
    sprite[j].pal = 0;
}

void explosion2(int i, int x, int y, int z, short owner)
{
    int32_t j = insertsprite(sprite[i].sectnum, EXPLO);

    sprite[j].x = x + (krand() & 256) - 128;
    sprite[j].y = y + (krand() & 256) - 128;
    sprite[j].z = z;

    sprite[j].cstat = 0;
    sprite[j].cstat &= ~3;
    sprite[j].picnum = MONSTERBALL;
    sprite[j].shade = -25;
    sprite[j].xrepeat = 64;
    sprite[j].yrepeat = 64;
    sprite[j].ang = krand() & kAngleMask;
    sprite[j].xvel = ((krand() & 256) - 128);
    sprite[j].yvel = ((krand() & 256) - 128);
    sprite[j].zvel = ((krand() & 256) - 128);
    sprite[j].owner = sprite[i].owner;
    sprite[j].lotag = 128;
    sprite[j].hitag = 0;
    sprite[j].pal = 0;
}

void trailingsmoke(short i)
{
    int32_t j = insertsprite(sprite[i].sectnum, SMOKE);

    sprite[j].x = sprite[i].x;
    sprite[j].y = sprite[i].y;
    sprite[j].z = sprite[i].z;
    sprite[j].cstat = 0x03;
    sprite[j].cstat &= ~3;
    sprite[j].picnum = SMOKEFX;
    sprite[j].shade = 0;
    sprite[j].xrepeat = 128;
    sprite[j].yrepeat = 128;
    sprite[j].pal = 0;

    sprite[j].owner = sprite[i].owner;
    sprite[j].lotag = 256;
    sprite[j].hitag = 0;
}

void icecubes(int i, int x, int y, int z, short owner)
{
    int32_t j = insertsprite(sprite[i].sectnum, FX);

    sprite[j].x = x;
    sprite[j].y = y;

    sprite[j].z = sector[sprite[i].sectnum].floorz - (PLAYERHEIGHT << 8) + (rand() & 4096);

    sprite[j].cstat = 0;        //Hitscan does not hit smoke on wall
    sprite[j].picnum = ICECUBE;
    sprite[j].shade = -16;
    sprite[j].xrepeat = 16;
    sprite[j].yrepeat = 16;

    sprite[j].ang = ((rand() & 1023) - 1024) & kAngleMask;
    sprite[j].xvel = ((rand() & 1023) - 512);
    sprite[j].yvel = ((rand() & 1023) - 512);
    sprite[j].zvel = ((rand() & 1023) - 512);

    sprite[j].pal = 6;
    sprite[j].owner = sprite[i].owner;
    sprite[j].lotag = 999;
    sprite[j].hitag = 0;
}

int damageactor(int hitobject, int i)
{
    int32_t j, k;

    Player* plr = &player[pyrn];

    j = (hitobject & 4095);     //j is the spritenum that the bullet (spritenum i) hit

      //if ( j == plr->spritenum && j != sprite[i].owner) {
    if (j == plr->spritenum && sprite[i].owner == 4096)
        return(0);

    if (j == plr->spritenum && sprite[i].owner != 4096)
    {
        if (plr->invincibletime < 0 || plr->manatime < 0)
            if ((krand() & 9) == 0)
                playsound_loc(S_PLRPAIN1 + (rand() % 2), sprite[i].x, sprite[i].y);

        if (plr->manatime > 0
            && sprite[i].picnum != FATSPANK
            && sprite[i].picnum != PLASMA)
            deletesprite(i);
        else
        {
            if (sprite[i].picnum == DART)
            {
                plr->poisoned = 1;
                plr->poisontime = 7200;
                StatusMessage(360, "Poisoned");
            }

            if (netgame)
            {
                netdamageactor(j, i);
            }
            else
            {
                if (sprite[i].picnum == PLASMA)
                    sethealth(-((krand() & 15) + 15));
                else if (sprite[i].picnum == FATSPANK)
                {
                    playsound_loc(S_GORE1A + (rand() % 3), plr->x, plr->y);
                    sethealth(-((krand() & 10) + 10));
                    if (rand() % 100 > 90)
                    {
                        plr->poisoned = 1;
                        plr->poisontime = 7200;
                        StatusMessage(360, "Poisoned");
                    }
                }
                else
                {
                    sethealth(-((krand() & 5) + 5));
                }
            }
            //                sethealth(-((krand()&5)+5));
            startredflash(10);
        }
    }

    if (j != plr->spritenum && netgame == 0)
        if (sprite[i].owner != j)
        {
            switch (sprite[j].picnum)
            {
                case BARREL:
                case VASEA:
                case VASEB:
                case VASEC:
                sprite[j].hitag = 0;
                sprite[j].lotag = 0;
                newstatus(j, BROKENVASE);
                break;

                case GRONHAL:
                case GRONHALATTACK:
                case GRONMU:
                case GRONMUATTACK:
                case GRONSW:
                case GRONSWATTACK:
                case KOBOLD:
                case KOBOLDATTACK:
                case DEVIL:
                case DEVILATTACK:
                case FRED:
                case FREDATTACK:
                case GOBLIN:
                case GOBLINATTACK:
                case GOBLINCHILL:
                case MINOTAUR:
                case MINOTAURATTACK:
                case SPIDER:
                case SKELETON:
                case SKELETONATTACK:
                case FATWITCH:
                case FATWITCHATTACK:
                switch (sprite[i].picnum)
                {
                    case PLASMA:
                    case MONSTERBALL:
                    sprite[j].hitag -= 40;
                    break;
                    case FATSPANK:
                    case BULLET:
                    sprite[j].hitag -= 10;
                    break;
                    case FIREBALL:
                    sprite[j].hitag -= 3;
                    break;
                    case DISTORTIONBLAST:
                    sprite[j].hitag = 10;
                    break;
                    case DART:
                    case WALLARROW:
                    case HORIZSPIKEBLADE:
                    case THROWHALBERD:
                    sprite[j].hitag -= 30;
                    break;
                    case THROWPIKE:
                    if (rand() % 2)
                        playsound_loc(S_GORE1A + rand() % 2, sprite[i].x, sprite[i].y);
                    sprite[j].hitag -= 30;
                    break;
                    case BARREL:
                    sprite[j].hitag -= 100;
                    break;
                }
                if (sprite[j].hitag <= 0)
                {
                    newstatus(j, DIE);
                    deletesprite(i);
                    return(1);
                }
                else
                {
                    newstatus(j, PAIN);
                    return(1);
                }
                break;
                case GRONHALPAIN:
                case GRONMUPAIN:
                case GRONSWPAIN:
                case KOBOLDDIE:
                case DEVILDIE:
                case FREDDIE:
                case GOBLINDIE:
                case MINOTAURDIE:
                case SPIDERDIE:
                case SKELETONDIE:
                //JSA_NEW //why is this here it's in whplr
                //raf because monsters could shatter a guy thats been frozen
                for (k = 0; k < 32; k++)
                    icecubes(j, sprite[j].x, sprite[j].y, sprite[j].z, j);
                deletesprite(j);
                return(1);
                default:
                deletesprite(i);
                return(1);
            }
        }

    return 0;
}

void nukespell(short j)
{
    switch (sprite[j].picnum)
    {
        case WILLOW:
        case SPIDER:
        deletesprite(j);
        addscoreamount(10);
        break;
        case KOBOLD:
        case KOBOLDATTACK:
        sprite[j].picnum = KOBOLDCHAR;
        newstatus(j, CHARCOAL);
        sprite[j].pal = 0;
        sprite[j].cstat |= 1;
        addscoreamount(150);
        break;
        case DEVIL:
        case DEVILATTACK:
        sprite[j].picnum = DEVILCHAR;
        newstatus(j, CHARCOAL);
        sprite[j].pal = 0;
        sprite[j].cstat |= 1;
        addscoreamount(150);
        break;
        case GOBLIN:
        case GOBLINATTACK:
        case GOBLINCHILL:
        sprite[j].picnum = GOBLINCHAR;
        newstatus(j, CHARCOAL);
        sprite[j].pal = 0;
        sprite[j].cstat |= 1;
        addscoreamount(150);
        break;
        case MINOTAUR:
        case MINOTAURATTACK:
        sprite[j].picnum = MINOTAURCHAR;
        newstatus(j, CHARCOAL);
        sprite[j].pal = 0;
        sprite[j].cstat |= 1;
        addscoreamount(150);
        break;
        case SKELETONATTACK:
        case SKELETON:
        sprite[j].picnum = SKELETONCHAR;
        newstatus(j, CHARCOAL);
        sprite[j].pal = 0;
        sprite[j].cstat |= 1;
        addscoreamount(150);
        break;
        case GRONHAL:
        case GRONHALATTACK:
        case GRONMU:
        case GRONMUATTACK:
        case GRONSW:
        case GRONSWATTACK:
        sprite[j].picnum = GRONCHAR;
        newstatus(j, CHARCOAL);
        sprite[j].pal = 0;
        sprite[j].cstat |= 1;
        addscoreamount(150);
        break;
        case DRAGON:
        case DRAGONATTACK:
        case DRAGONATTACK2:
        sprite[j].picnum = DRAGONCHAR;
        newstatus(j, CHARCOAL);
        sprite[j].pal = 0;
        sprite[j].cstat |= 1;
        addscoreamount(150);
        break;
        case GUARDIAN:
        case GUARDIANATTACK:
        sprite[j].picnum = GUARDIANCHAR;
        newstatus(j, CHARCOAL);
        sprite[j].pal = 0;
        sprite[j].cstat |= 1;
        addscoreamount(150);
        break;
        case FATWITCH:
        case FATWITCHATTACK:
        sprite[j].picnum = FATWITCHCHAR;
        newstatus(j, CHARCOAL);
        sprite[j].pal = 0;
        sprite[j].cstat |= 1;
        addscoreamount(150);
        break;
        case SKULLY:
        case SKULLYATTACK:
        sprite[j].picnum = SKULLYCHAR;
        newstatus(j, CHARCOAL);
        sprite[j].pal = 0;
        sprite[j].cstat |= 1;
        addscoreamount(150);
        break;
        case JUDY:
        case JUDYATTACK1:
        case JUDYATTACK2:
        if (mapon < 24)
        {
            sprite[j].picnum = JUDYCHAR;
            newstatus(j, CHARCOAL);
            sprite[j].pal = 0;
            sprite[j].cstat |= 1;
            addscoreamount(150);
        }
        break;
    }
}

void medusa(short j)
{
    newstatus(j, FROZEN);

    switch (sprite[j].picnum)
    {
        case KOBOLD:
        case KOBOLDATTACK:
        sprite[j].picnum = KOBOLDDIE;
        break;
        case DEVIL:
        case DEVILATTACK:
        sprite[j].picnum = DEVILDIE;
        break;
        case FRED:
        case FREDATTACK:
        sprite[j].picnum = FREDDIE;
        break;
        case GOBLIN:
        case GOBLINATTACK:
        sprite[j].picnum = GOBLINDIE;
        break;
        case MINOTAUR:
        case MINOTAURATTACK:
        sprite[j].picnum = MINOTAURDIE;
        break;
        case SPIDER:
        sprite[j].picnum = SPIDERDIE;
        break;
        case SKELETON:
        sprite[j].picnum = SKELETONDIE;
        break;
        case GRONHAL:
        case GRONHALATTACK:
        sprite[j].picnum = GRONHALDIE;
        break;
        case GRONMU:
        case GRONMUATTACK:
        sprite[j].picnum = GRONMUDIE;
        break;
        case GRONSW:
        case GRONSWATTACK:
        sprite[j].picnum = GRONSWDIE;
        break;
    }
    sprite[j].pal = 6;
    sprite[j].cstat |= 1;
    addscoreamount(100);
}

// New movesprite using getzrange.  Note that I made the getzrange
// parameters global (&globhiz,&globhihit,&globloz,&globlohit) so they
// don't need to be passed everywhere.  Also this should make this
// movesprite function compatible with the older movesprite functions.
int movesprite(short spritenum, int dx, int dy, int dz, int ceildist, int flordist, char cliptype)
{
    int32_t daz, zoffs;
    short retval, dasectnum, tempshort;
    spritetype* spr;
    unsigned int dcliptype = CLIPMASK0;

    switch (cliptype)
    {
        case NORMALCLIP:
        case CLIFFCLIP:
        dcliptype = CLIPMASK0; break;
        case PROJECTILECLIP:
        dcliptype = CLIPMASK1; break;
    }

    spr = &sprite[spritenum];

    if ((spr->cstat & 128) == 0)
        zoffs = -((tilesiz[spr->picnum].y * spr->yrepeat) << 1);
    else
        zoffs = 0;

    dasectnum = spr->sectnum;  //Can't modify sprite sectors directly becuase of linked lists
    daz = spr->z + zoffs;  //Must do this if not using the new centered centering (of course)

    retval = clipmove_old(&spr->x, &spr->y, &daz, &dasectnum, dx, dy, ((int)spr->clipdist) << 2, ceildist, flordist, dcliptype);

    if ((dasectnum != spr->sectnum) && (dasectnum >= 0))
        changespritesect(spritenum, dasectnum);

    //Set the blocking bit to 0 temporarly so getzrange doesn't pick up
    //its own sprite
    tempshort = spr->cstat; spr->cstat &= ~1;

    getzrange_old(spr->x, spr->y, spr->z - 1, spr->sectnum,
        &globhiz, &globhihit, &globloz, &globlohit,
        ((int)spr->clipdist) << 2, dcliptype);

    spr->cstat = tempshort;

    daz = spr->z + zoffs + dz;
    if ((daz <= globhiz) || (daz > globloz))
    {
        if (retval != 0) return(retval);
        return(16384 + dasectnum);
    }
    spr->z = daz - zoffs;
    return retval;
}

void guardianfire(short i, int k, Player* plr)
{
    short daang = (daang + (k << 2)) & kAngleMask; // TODO - fixme
    int32_t j = insertsprite(sprite[i].sectnum, MISSILE);
    sprite[j].x = sprite[i].x;
    sprite[j].y = sprite[i].y;
    sprite[j].z = sprite[i].z + (8 << 8) + ((krand() & 10) << 8);
    sprite[j].cstat = 0;
    sprite[j].picnum = PLASMA;
    sprite[j].shade = -32;
    sprite[j].pal = 0;
    sprite[j].xrepeat = 16;
    sprite[j].yrepeat = 16;
    sprite[j].ang = daang;

    sprite[j].xvel = Cos(daang + 2048) >> 5;
    sprite[j].yvel = Sin(daang + 2048) >> 5;

    int discrim = ksqrt((plr->x - sprite[j].x) * (plr->x - sprite[j].x) + (plr->y - sprite[j].y) * (plr->y - sprite[j].y));
    if (discrim == 0)
        discrim = 1;

    sprite[j].zvel = ((plr->z + (48 << 8) - sprite[j].z) << 7) / discrim;

    sprite[j].owner = 4096;
    sprite[j].lotag = 256;
    sprite[j].hitag = 0;
    sprite[j].clipdist = 48;
}

void trowajavlin(int s)
{
    int32_t j = insertsprite(sprite[s].sectnum, JAVLIN);

    sprite[j].x = sprite[s].x;
    sprite[j].y = sprite[s].y;
    sprite[j].z = sprite[s].z;

    sprite[j].cstat = 21;

    switch (sprite[s].lotag)
    {
        case 91:
        sprite[j].picnum = WALLARROW;
        sprite[j].ang = ((sprite[s].ang + 2048) - 512) & kAngleMask;
        sprite[j].xrepeat = 16;
        sprite[j].yrepeat = 48;
        sprite[j].clipdist = 24;
        break;
        case 92:
        sprite[j].picnum = DART;
        sprite[j].ang = ((sprite[s].ang + 2048) - 512) & kAngleMask;
        sprite[j].xrepeat = 64;
        sprite[j].yrepeat = 64;
        sprite[j].clipdist = 16;
        break;
        case 93:
        sprite[j].picnum = HORIZSPIKEBLADE;
        sprite[j].ang = ((sprite[s].ang + 2048) - 512) & kAngleMask;
        sprite[j].xrepeat = 16;
        sprite[j].yrepeat = 48;
        sprite[j].clipdist = 32;
        break;
        case 94:
        sprite[j].picnum = THROWPIKE;
        sprite[j].ang = ((sprite[s].ang + 2048) - 512) & kAngleMask;
        sprite[j].xrepeat = 24;
        sprite[j].yrepeat = 24;
        sprite[j].clipdist = 32;
        break;
    }

    sprite[j].extra = sprite[s].ang;
    sprite[j].shade = -15;
    sprite[j].xvel = ((krand() & 256) - 128);
    sprite[j].yvel = ((krand() & 256) - 128);
    sprite[j].zvel = ((krand() & 256) - 128);
    sprite[j].owner = 0;
    sprite[j].lotag = 0;
    sprite[j].hitag = 0;
    sprite[j].pal = 0;
}

void throwhalberd(int s)
{
    int32_t j = insertsprite(sprite[s].sectnum, JAVLIN);

    sprite[j].x = sprite[s].x;
    sprite[j].y = sprite[s].y;
    sprite[j].z = sprite[s].z - (32 << 8);

    sprite[j].cstat = 17;

    sprite[j].picnum = THROWHALBERD;
    sprite[j].ang = ((sprite[s].ang + 2048) - 512) & kAngleMask;
    sprite[j].xrepeat = 8;
    sprite[j].yrepeat = 16;
    sprite[j].clipdist = 32;

    sprite[j].extra = sprite[s].ang;
    sprite[j].shade = -15;
    sprite[j].xvel = ((krand() & 256) - 128);
    sprite[j].yvel = ((krand() & 256) - 128);
    sprite[j].zvel = ((krand() & 256) - 128);
    sprite[j].owner = s;
    sprite[j].lotag = 0;
    sprite[j].hitag = 0;
    sprite[j].pal = 0;

    sprite[j].cstat = 0;
    int32_t daz = (((sprite[j].zvel) * synctics) >> 3);

    movesprite(j, (Cos(sprite[j].extra) * synctics) << 7, (Sin(sprite[j].extra) * synctics) << 7, daz, 4 << 8, 4 << 8, 1);

    sprite[j].cstat = 21;
}

void monsterweapon(short i)
{
    int32_t j = insertsprite(sprite[i].sectnum, 0);

    if (sprite[i].picnum == GOBLINDEAD)
    {
        sprite[j].x = sprite[i].x;
        sprite[j].y = sprite[i].y;
        sprite[j].z = sprite[i].z - (24 << 8);
        sprite[j].shade = -15;
        sprite[j].cstat = 0;
        sprite[j].cstat &= ~3;
        sprite[j].xrepeat = 16;
        sprite[j].yrepeat = 16;
        sprite[j].picnum = GOBWEAPON;
        sprite[j].pal = 0;
    }
    else if (sprite[i].picnum == MINOTAURDEAD)
    {
        sprite[j].x = sprite[i].x;
        sprite[j].y = sprite[i].y;
        sprite[j].z = sprite[i].z - (24 << 8);
        sprite[j].shade = -15;
        sprite[j].cstat = 0;
        sprite[j].cstat &= ~3;
        if (rand() % 100 > 50)
        {
            sprite[j].xrepeat = 25;
            sprite[j].yrepeat = 20;
            sprite[j].picnum = WEAPON4;
        }
        else
        {
            sprite[j].xrepeat = 20;
            sprite[j].yrepeat = 15;
            sprite[j].picnum = WEAPON6;
        }
        sprite[j].pal = 0;
    }
    else
    {
        sprite[j].x = sprite[i].x;
        sprite[j].y = sprite[i].y;
        sprite[j].z = sprite[i].z - (24 << 8);
        sprite[j].shade = -15;
        sprite[j].cstat = 0;
        sprite[j].cstat &= ~3;
        sprite[j].xrepeat = 25;
        sprite[j].yrepeat = 20;

        int k = krand() % 4;

        switch (k)
        {
            case 0:
            sprite[j].picnum = WEAPON3;
            sprite[j].xrepeat = 25;
            sprite[j].yrepeat = 20;
            break;
            case 1:
            sprite[j].picnum = WEAPON5;
            sprite[j].xrepeat = 25;
            sprite[j].yrepeat = 20;
            break;
            case 2:
            sprite[j].picnum = WEAPON6;
            sprite[j].xrepeat = 20;
            sprite[j].yrepeat = 15;
            break;
            case 3:
            sprite[j].picnum = SHIELD;
            sprite[j].xrepeat = 32;
            sprite[j].yrepeat = 32;
            break;
        }
        sprite[j].pal = 0;
    }
}

// VAL 1-3
void madenoise(int val, int x, int y, int z)
{
    int16_t i = headspritestat[FACE];

    while (i >= 0)
    {
        int16_t nexti = nextspritestat[i];
        if (labs(x - sprite[i].x) + labs(y - sprite[i].y) < (val * 4096))
        {
            newstatus(i, FINDME);
        }
        i = nexti;
    }
}

void monsternoise(short i)
{
    return;
    #if 0
    switch (sprite[i].picnum)
    {
        case KOBOLD:
        if (rand() % 100 > 98)
            playsound_loc(S_KSNARL1 + (rand() % 4), sprite[i].x, sprite[i].y);
        break;
        case FRED:
        case GOBLIN:
        case MINOTAUR:
        case SPIDER:
        case SKELETON:
        break;
    }
    #endif
}

void randompotion(short i)
{
    if (rand() % 100 > 20)
        return;

    int32_t j = insertsprite(sprite[i].sectnum, 0);

    sprite[j].x = sprite[i].x;
    sprite[j].y = sprite[i].y;
    sprite[j].z = sprite[i].z - (12 << 8);
    sprite[j].shade = -12;
    sprite[j].pal = 0;
    sprite[j].cstat = 0;
    sprite[j].cstat &= ~3;
    sprite[j].xrepeat = 64;
    sprite[j].yrepeat = 64;
    sprite[j].picnum = FLASKBLUE + krand() % 4;
}

void spawnabaddy(short i, short monster)
{
    int32_t j = insertsprite(sprite[i].sectnum, FACE);

    sprite[j].x = sprite[i].x + (krand() & 2048) - 1024;
    sprite[j].y = sprite[i].y + (krand() & 2048) - 1024;
    sprite[j].z = sprite[i].z;

    switch (monster)
    {
        case WILLOW:
        sprite[j].xrepeat = 32;
        sprite[j].yrepeat = 32;
        sprite[j].clipdist = 64;
        sprite[j].hitag = adjusthp(400);
        sprite[j].lotag = 100;
        sprite[j].cstat |= 0x101;
        sprite[j].pal = 0;
        sprite[j].shade = 0;
        sprite[j].picnum = WILLOW;
        break;
        case SPIDER:
        sprite[j].xrepeat = 12;
        sprite[j].yrepeat = 9;
        sprite[j].clipdist = 32;
        sprite[j].hitag = adjusthp(5);
        sprite[j].lotag = 100;
        sprite[j].cstat |= 0x101;
        sprite[j].pal = 0;
        sprite[j].shade = 0;
        sprite[j].picnum = SPIDER;
        break;
        case GRONSW:
        sprite[j].xrepeat = 30;
        sprite[j].yrepeat = 30;
        sprite[j].clipdist = 64;
        sprite[j].hitag = adjusthp(300);
        sprite[j].lotag = 100;
        sprite[j].cstat |= 0x101;
        sprite[j].extra = 0;
        sprite[j].pal = 0;
        sprite[j].shade = 0;
        sprite[j].picnum = GRONSW;
        break;
        case SKELETON:
        sprite[j].xrepeat = 24;
        sprite[j].yrepeat = 24;
        sprite[j].clipdist = 64;
        sprite[j].hitag = adjusthp(30);
        sprite[j].lotag = 100;
        sprite[j].cstat |= 0x101;
        sprite[j].pal = 0;
        sprite[j].shade = 0;
        sprite[j].picnum = SKELETON;
        break;
    }

    vec3_t pos;
    pos.x = sprite[j].x;
    pos.y = sprite[j].y;
    pos.z = sprite[j].z;

    setsprite(j, &pos);
}

void spawnapentagram(short sn)
{
    int32_t j = insertsprite(sprite[sn].sectnum, 0);

    sprite[j].x = sprite[sn].x;
    sprite[j].y = sprite[sn].y;
    sprite[j].z = sprite[sn].z - (8 << 8);
    sprite[j].xrepeat = sprite[j].yrepeat = 64;
    sprite[j].pal = 0;
    sprite[j].shade = -15;
    sprite[j].cstat = 0;
    sprite[j].clipdist = 64;
    sprite[j].lotag = 0;
    sprite[j].hitag = 0;
    sprite[j].extra = 0;
    sprite[j].picnum = PENTAGRAM;

    vec3_t pos;
    pos.x = sprite[j].x;
    pos.y = sprite[j].y;
    pos.z = sprite[j].z;

    setsprite(j, &pos);
}
