
#ifndef __objects_h__
#define __objects_h__

#include "player.h"

void transformactors(Player *plr);
void processobjs(Player *plr);
void monitor();
void updatepotion(int vial);
int potionspace(int vial);
void explosion(int i, int x, int y, int z, short owner);
void trailingsmoke(short i);
void monsterweapon(short i);
void spawnapentagram(short sn);
int movesprite(short spritenum, int dx, int dy, int dz, int ceildist, int flordist, char cliptype);
void icecubes(int i, int x, int y, int z, short owner);
int movesprite(short spritenum, int dx, int dy, int dz, int ceildist, int flordist, char cliptype);
void madenoise(int val, int x, int y, int z);
void medusa(short j);
void newstatus(short sn, int  seq);
int checkmedusadist(int i, int x, int y, int z, int lvl);
void nukespell(short j);
int checkdist(int i, int x, int y, int z);
void firebreath(int i, int a, int b, int c);
void castspell(int i);
void throwspank(int i);
void throwhalberd(int s);
void monsternoise(short i);
void checkmove(int i, int dax, int day, short* movestat);
int checksight(int i, short* daang);
void checkspeed(int i, int* dax, int* day, int speed);
void checkspeed(int i, int* dax, int* day, int speed);
void spawnabaddy(short i, short monster);
void skullycastspell(int i);
void makeafire(int i, int firetype);
void explosion2(int i, int x, int y, int z, short owner);
int damageactor(int hitobject, int i);
void attack(int i);
void randompotion(short i);
void trowajavlin(int s);

#endif
