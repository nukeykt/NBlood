
#ifndef __menu_h__
#define __menu_h__

#include <stdint.h>

void screenfx();
void initpaletteshifts();
int menuscreen();
void startredflash(int damage);
void fancyfontscreen(int x, int y, short tilenum, char* string);
void startwhiteflash(int bonus);
void fancyfont(int x, int y, short tilenum, char* string, char pal);
void startblueflash(int bluetime);
void startgreenflash(int greentime);
void startredflash(int damage);
void startnewgame();
void loadsave();
void thedifficulty();
void help();
void quit();
void loadgame();
void savegame();
void loadplayerstuff();
int savedgamedat(int gn);
void savegametext(int select);
int savedgamename(int gn);
void updatepaletteshifts();
void itemtoscreen(int x, int y, short tilenum, int8_t shade, char pal);

#endif
