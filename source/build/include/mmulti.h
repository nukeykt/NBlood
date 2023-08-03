// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
//
// This file has been modified from Ken Silverman's original release
// by Jonathon Fowler (jf@jonof.id.au)
// by the EDuke32 team (development@voidpoint.com)

#ifndef mmulti_h_
#define mmulti_h_

#define MAXMULTIPLAYERS 16

enum { MMULTI_MODE_MS = 0, MMULTI_MODE_P2P };

extern int myconnectindex, numplayers, networkmode;
extern int connecthead, connectpoint2[MAXMULTIPLAYERS];
extern char syncstate;
extern int natfree; //Addfaz NatFree

int initmultiplayersparms(int argc, const char * const *argv);
int initmultiplayerscycle(void);

void mmulti_initmultiplayers(int argc, const char * const *argv, char damultioption, char dacomrateoption, char dapriority);
void mmulti_setpackettimeout(int datimeoutcount, int daresendagaincount);
void mmulti_uninitmultiplayers(void);
void mmulti_sendlogon(void);
void mmulti_sendlogoff(void);
int mmulti_getoutputcirclesize(void);
void mmulti_sendpacket(int other, unsigned char *bufptr, int messleng);
int mmulti_getpacket(int *other, unsigned char *bufptr);
void mmulti_flushpackets(void);
void mmulti_generic(int other, unsigned char *bufptr, int messleng, int command);
int isvalidipaddress(const char *st);

void nfIncCP(void); //Addfaz NatFree
int nfCheckHF(int other); //Addfaz NatFree
int nfCheckCP(int other); //Addfaz NatFree

#define initmultiplayers mmulti_initmultiplayers
#define sendpacket mmulti_sendpacket
#define setpackettimeout mmulti_setpackettimeout
#define uninitmultiplayers mmulti_uninitmultiplayers
#define sendlogon mmulti_sendlogon
#define sendlogoff mmulti_sendlogoff
#define getoutputcirclesize mmulti_getoutputcirclesize
#define getpacket mmulti_getpacket
#define flushpackets mmulti_flushpackets
#define genericmultifunction mmulti_generic

#endif  // mmulti_h_
