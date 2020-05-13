
#ifndef __whinp_h__
#define __whinp_h__

#include "player.h"

extern int32_t lockclock;
extern int32_t angvel, svel, vel;

void keytimerstuff();
void processinput(Player *plr);

#endif
