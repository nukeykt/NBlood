
#ifndef __whtag_h__
#define __whtag_h__

#include "player.h"
#include "whdefs.h"

struct swingdoor_t
{
    int wall[8];
    int sector;
    int angopen;
    int angclosed;
    int angopendir;
    int ang;
    int anginc;
    int x[8];
    int y[8];
};

extern swingdoor_t swingdoor[MAXSWINGDOORS];

void animatetags(Player *plr);
void operatesector(int s);
void operatesprite(int s);

#endif
