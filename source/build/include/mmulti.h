// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
//
// This file has been modified from Ken Silverman's original release
// by Jonathon Fowler (jf@jonof.id.au)
// by the EDuke32 team (development@voidpoint.com)

#ifndef mmulti_h
#define mmulti_h

#include "build.h"

#define MMULTI_MODE_MS  0
#define MMULTI_MODE_P2P 1

extern int myconnectindex, numplayers, networkmode;
extern int connecthead, connectpoint2[MAXPLAYERS];
extern unsigned char syncstate;

void initsingleplayers(void);
void initmultiplayers(int argc, char const * const argv[]);
int initmultiplayersparms(int argc, char const * const argv[]);
int initmultiplayerscycle(void);

void setpackettimeout(int datimeoutcount, int daresendagaincount);
void uninitmultiplayers(void);
void sendlogon(void);
void sendlogoff(void);
int getoutputcirclesize(void);
void setsocket(int newsocket);
void sendpacket(int other, unsigned char *bufptr, int messleng);
int getpacket(int *other, unsigned char *bufptr);
void flushpackets(void);
void genericmultifunction(int other, unsigned char *bufptr, int messleng, int command);

#define mmulti_initsingleplayers initsingleplayers
#define mmulti_initmultiplayers initmultiplayers
#define mmulti_initmultiplayersparms initmultiplayersparms
#define mmulti_initmultiplayerscycle initmultiplayerscycle
#define mmulti_setpackettimeout setpackettimeout
#define mmulti_uninitmultiplayers uninitmultiplayers
#define mmulti_sendlogon sendlogon
#define mmulti_sendlogoff sendlogoff
#define mmulti_sendpacket sendpacket
#define mmulti_getpacket getpacket
#define mmulti_flushpackets flushpackets
#define mmulti_genericmultifunction genericmultifunction

#endif	// mmulti_h

