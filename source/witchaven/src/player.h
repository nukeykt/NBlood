
#ifndef __player_h__
#define __player_h__

#include "build.h"

#define MAXNUMORBS   8
#define MAXTREASURES 18
#define MAXWEAPONS   10
#define MAXPOTIONS   5
#define PLAYERHEIGHT 48

struct Player
{
    int32_t x, y, z;
    int ang;
    int horiz;
    int zoom;
    int height;
    int hvel;
    short sector, oldsector;
    short screensize;
    short spritenum;
    uint8_t dimension;
    uint32_t flags;
    int weapon[MAXWEAPONS];
    int ammo[MAXWEAPONS];
    int orbammo[MAXNUMORBS];
    int treasure[MAXTREASURES];
    int orbactive[MAXNUMORBS];
    int orb[MAXNUMORBS];
    int potion[MAXPOTIONS];
    int lvl;
    int score;
    int health;
    int svgahealth;
    int maxhealth;
    int armor;
    int armortype;
    uint8_t onsomething;
    int fallz;

    int8_t currentpotion;
    int8_t currweapon;
    int8_t selectedgun;
    int8_t currentorb;
    int8_t currweaponfired;
    int8_t currweaponanim;
    int8_t currweaponattackstyle;
    int8_t shieldpoints;
    int8_t poisoned;
    bool currweaponflip;
    bool spiked;
    bool playerdie;
    int invincibletime;

    bool hasshot;
    bool orbshot;

    int16_t helmettime;
    int16_t shadowtime;
    int16_t nightglowtime;
    int16_t strongtime;
    int16_t manatime;
    int16_t vampiretime;
    int16_t shockme;
    int16_t poisontime;
    int16_t invisibletime;

    fix16_t q16angle, q16oangle;
    fix16_t q16horiz, q16ohoriz;
    vec3_t opos;
};

extern Player player[MAXPLAYERS];

extern int pyrn;
extern int autohoriz;

void addarmoramount(int nArmor);
void addscoreamount(int nScore);
void drawweapons(Player *plr);
void drawarmor();
void drawpotionpic();
void drawscore();
void draworbpic();
void spikeheart(Player *plr);
void updatepics();
void sethealth(int hp);
void healthpic();
void setpotion(int nPotion);
//void score(int score);
void keyspic();
//void armorpic(int arm);
void autoweaponchange(int dagun);
void levelpic();
void shootgun(Player *plr, short daang, char guntype);
void potiontext();
int checkweapondist(short i, int x, int y, int z, char guntype);
void castaorb(Player *plr);
//void orbpic(int currentorb);
void goesupalevel(Player *plr);
void loadnewlevel(int mapon);
void displayspelltext();
void captureflagpic();
void fragspic();
void initplayersprite();
void plrfireweapon(Player *plr);
int lvlspellcheck(Player* plr);
void activatedaorb(Player* plr);
void usapotion(Player* plr);
void plruse(Player* plr);
void playerdead(Player* plr);
void autothehoriz(Player* plr);
void singleshot(short bstatus);
void weaponchange();
void checkcheat();
void victory();

#endif
