
#ifndef __whdefs_h__
#define __whdefs_h__

//
// WHDEFS.H
//

#define   STATUSHEIGHT   46
#define   STATUSSCREEN   (YDIM-STATUSHEIGHT)

#define   ACTIVATESECTOR      1
#define   ACTIVATESECTORONCE  2

#define   DOORUPTAG      6
#define   DOORDOWNTAG    7
#define   DOORSPLITHOR   8
#define   DOORSPLITVER   9
#define   DOORSWINGTAG   13
#define   DOORBOX        16

#define   PLATFORMELEVTAG 1000
#define   BOXELEVTAG     1003

#define   SECTOR         1
#define   WALL           2
#define   SPRITE         3

#define   SECTOREFFECT   104
#define   PULSELIGHT     0
#define   FLICKERLIGHT   1
#define   DELAYEFFECT    2
#define   XPANNING       3

#define   DOORDELAY      480 // 4 second delay for doors to close
#define   DOORSPEED      128
#define   ELEVSPEED      256

#define   PICKDISTANCE   512 // for picking up sprites
#define   PICKHEIGHT     40

#define   JETPACKPIC     93   // sprites available to pick up

#define   MAXSWINGDOORS  32
#define   MAXANIMATES    512

#define   JETPACKITEM    0
#define   SHOTGUNITEM    1

#define   SHOTGUNPIC     101
#define   SHOTGUNVIEW    102

#endif
