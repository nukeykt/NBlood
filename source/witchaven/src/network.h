
#ifndef __whnet_h__
#define __whnet_h__

#include "player.h"

extern short teamscore[];
extern short teaminplay[];
extern short ihaveflag;

void netshootgun(short s, char guntype);
void netsendmove();
void netrestartplayer(Player *plr);
void netdamageactor(short s, short o);
void netgetmove();
void netdropflag();

void whnetmon();

#endif
