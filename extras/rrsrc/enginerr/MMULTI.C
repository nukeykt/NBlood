// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <process.h>
#include <stdarg.h>
#include "pragmas.h"

#define MAXPLAYERS 16
#define BAKSIZ 16384
#define SIMULATEERRORS 0
#define SHOWSENDPACKETS 0
#define SHOWGETPACKETS 0
#define PRINTERRORS 0

#define updatecrc16(crc,dat) crc = (((crc<<8)&65535)^crctable[((((unsigned short)crc)>>8)&65535)^dat])

static long incnt[MAXPLAYERS], outcntplc[MAXPLAYERS], outcntend[MAXPLAYERS];
static char errorgotnum[MAXPLAYERS];
static char errorfixnum[MAXPLAYERS];
static char errorresendnum[MAXPLAYERS];
#if (PRINTERRORS)
	static char lasterrorgotnum[MAXPLAYERS];
#endif

long crctable[256];

static char lastpacket[576], inlastpacket = 0;
static short lastpacketfrom, lastpacketleng;

extern long totalclock;  //MUST EXTERN 1 ANNOYING VARIABLE FROM GAME
static long timeoutcount = 60, resendagaincount = 4, lastsendtime[MAXPLAYERS];

static short bakpacketptr[MAXPLAYERS][256], bakpacketlen[MAXPLAYERS][256];
static char bakpacketbuf[BAKSIZ];
static long bakpacketplc = 0;

short myconnectindex, numplayers;
short connecthead, connectpoint2[MAXPLAYERS];
char syncstate = 0;

extern int _argc;
extern char **_argv;

#define MAXPACKETSIZE 2048
typedef struct
{
	short intnum;                //communication between Game and the driver
	short command;               //1-send, 2-get
	short other;                 //dest for send, set by get (-1 = no packet)
	short numbytes;
	short myconnectindex;
	short numplayers;
	short gametype;              //gametype: 1-serial,2-modem,3-net
	short filler;
	char buffer[MAXPACKETSIZE];
	long longcalladdress;
} gcomtype;
static gcomtype *gcom;

static union REGS regs;

#pragma aux longcall =\
	"call eax",\
	parm [eax]\

callcommit()
{
	if (gcom->intnum&0xff00)
		longcall(gcom->longcalladdress);
	else
		int386(gcom->intnum,&regs,&regs);
}

initmultiplayers(char damultioption, char dacomrateoption, char dapriority)
{
	long i;
	char *parm, delims[4] = {'\\','-','/','\0'};

	initcrc();
	for(i=0;i<MAXPLAYERS;i++)
	{
		incnt[i] = 0L;
		outcntplc[i] = 0L;
		outcntend[i] = 0L;
		bakpacketlen[i][255] = -1;
	}

	for(i=_argc-1;i>0;i--)
		if ((parm = strtok(_argv[i],&delims[0])) != NULL)
			if (!stricmp("net",parm)) break;
	if (i == 0)
	{
		numplayers = 1; myconnectindex = 0;
		connecthead = 0; connectpoint2[0] = -1;
		return;
	}
	gcom = (gcomtype *)atol(_argv[i+1]);

	numplayers = gcom->numplayers;
	myconnectindex = gcom->myconnectindex-1;
#if (SIMULATEERRORS != 0)
	srand(myconnectindex*24572457+345356);
#endif
	connecthead = 0;
	for(i=0;i<numplayers-1;i++) connectpoint2[i] = i+1;
	connectpoint2[numplayers-1] = -1;

	for(i=0;i<numplayers;i++) lastsendtime[i] = totalclock;
}

initcrc()
{
	long i, j, k, a;

	for(j=0;j<256;j++)      //Calculate CRC table
	{
		k = (j<<8); a = 0;
		for(i=7;i>=0;i--)
		{
			if (((k^a)&0x8000) > 0)
				a = ((a<<1)&65535) ^ 0x1021;   //0x1021 = genpoly
			else
				a = ((a<<1)&65535);
			k = ((k<<1)&65535);
		}
		crctable[j] = (a&65535);
	}
}

setpackettimeout(long datimeoutcount, long daresendagaincount)
{
	long i;

	timeoutcount = datimeoutcount;
	resendagaincount = daresendagaincount;
	for(i=0;i<numplayers;i++) lastsendtime[i] = totalclock;
}

getcrc(char *buffer, short bufleng)
{
	long i, j;

	j = 0;
	for(i=bufleng-1;i>=0;i--) updatecrc16(j,buffer[i]);
	return(j&65535);
}

uninitmultiplayers()
{
}

sendlogon()
{
}

sendlogoff()
{
	long i;
	char tempbuf[2];

	tempbuf[0] = 255;
	tempbuf[1] = myconnectindex;
	for(i=connecthead;i>=0;i=connectpoint2[i])
		if (i != myconnectindex)
			sendpacket(i,tempbuf,2L);
}
getoutputcirclesize()
{
	return(0);
}

setsocket(short newsocket)
{
}

sendpacket(long other, char *bufptr, long messleng)
{
	long i, j, k, l,cnt;
	unsigned short dacrc;

	if (numplayers < 2) return;

	i = 0;
	if (bakpacketlen[other][(outcntend[other]-1)&255] == messleng)
	{
		j = bakpacketptr[other][(outcntend[other]-1)&255];
		for(i=messleng-1;i>=0;i--)
			if (bakpacketbuf[(i+j)&(BAKSIZ-1)] != bufptr[i]) break;
	}
	bakpacketlen[other][outcntend[other]&255] = messleng;

	if (i < 0)   //Point to last packet to save space on bakpacketbuf
		bakpacketptr[other][outcntend[other]&255] = j;
	else
	{
		bakpacketptr[other][outcntend[other]&255] = bakpacketplc;
		for(i=0;i<messleng;i++)
			bakpacketbuf[(bakpacketplc+i)&(BAKSIZ-1)] = bufptr[i];
		bakpacketplc = ((bakpacketplc+messleng)&(BAKSIZ-1));
	}
	outcntend[other]++;

	lastsendtime[other] = totalclock;
	dosendpackets(other);
}

dosendpackets(long other)
{
	long i, j, k, messleng;
	unsigned short dacrc;

	if (outcntplc[other] == outcntend[other]) return;

#if (PRINTERRORS)
	if (errorgotnum[other] > lasterrorgotnum[other])
	{
		lasterrorgotnum[other]++;
		printf(" MeWant %ld",incnt[other]&255);
	}
#endif

	if (outcntplc[other]+1 == outcntend[other])
	{     //Send 1 sub-packet
		k = 0;
		gcom->buffer[k++] = (outcntplc[other]&255);
		gcom->buffer[k++] = (errorgotnum[other]&7)+((errorresendnum[other]&7)<<3);
		gcom->buffer[k++] = (incnt[other]&255);

		j = bakpacketptr[other][outcntplc[other]&255];
		messleng = bakpacketlen[other][outcntplc[other]&255];
		for(i=0;i<messleng;i++)
			gcom->buffer[k++] = bakpacketbuf[(i+j)&(BAKSIZ-1)];
		outcntplc[other]++;
	}
	else
	{     //Send 2 sub-packets
		k = 0;
		gcom->buffer[k++] = (outcntplc[other]&255);
		gcom->buffer[k++] = (errorgotnum[other]&7)+((errorresendnum[other]&7)<<3)+128;
		gcom->buffer[k++] = (incnt[other]&255);

			//First half-packet
		j = bakpacketptr[other][outcntplc[other]&255];
		messleng = bakpacketlen[other][outcntplc[other]&255];
		gcom->buffer[k++] = (char)(messleng&255);
		gcom->buffer[k++] = (char)(messleng>>8);
		for(i=0;i<messleng;i++)
			gcom->buffer[k++] = bakpacketbuf[(i+j)&(BAKSIZ-1)];
		outcntplc[other]++;

			//Second half-packet
		j = bakpacketptr[other][outcntplc[other]&255];
		messleng = bakpacketlen[other][outcntplc[other]&255];
		for(i=0;i<messleng;i++)
			gcom->buffer[k++] = bakpacketbuf[(i+j)&(BAKSIZ-1)];
		outcntplc[other]++;

	}

	dacrc = getcrc(gcom->buffer,k);
	gcom->buffer[k++] = (dacrc&255);
	gcom->buffer[k++] = (dacrc>>8);

	gcom->other = other+1;
	gcom->numbytes = k;

#if (SHOWSENDPACKETS)
	printf("Send(%ld): ",gcom->other);
	for(i=0;i<gcom->numbytes;i++) printf("%2x ",gcom->buffer[i]);
	printf("\n");
#endif

#if (SIMULATEERRORS != 0)
	if (!(rand()&SIMULATEERRORS)) gcom->buffer[rand()%gcom->numbytes] = (rand()&255);
	if (rand()&SIMULATEERRORS)
#endif
		{ gcom->command = 1; callcommit(); }
}

short getpacket (short *other, char *bufptr)
{
	long i, j, messleng;
	unsigned short dacrc;

	if (numplayers < 2) return(0);

	for(i=connecthead;i>=0;i=connectpoint2[i])
		if (i != myconnectindex)
		{
			if (totalclock < lastsendtime[i]) lastsendtime[i] = totalclock;
			if (totalclock > lastsendtime[i]+timeoutcount)
			{
#if (PRINTERRORS)
					printf(" TimeOut!");
#endif
					errorgotnum[i] = errorfixnum[i]+1;

					if ((outcntplc[i] == outcntend[i]) && (outcntplc[i] > 0))
						{ outcntplc[i]--; lastsendtime[i] = totalclock; }
					else
						lastsendtime[i] += resendagaincount;
					dosendpackets(i);
				//}
			}
		}

	if (inlastpacket != 0)
	{
			//2ND half of good double-packet
		inlastpacket = 0;
		*other = lastpacketfrom;
		memcpy(bufptr,lastpacket,lastpacketleng);
		return(lastpacketleng);
	}

	gcom->command = 2;
	callcommit();

#if (SHOWGETPACKETS)
	if (gcom->other != -1)
	{
		printf(" Get(%ld): ",gcom->other);
		for(i=0;i<gcom->numbytes;i++) printf("%2x ",gcom->buffer[i]);
		printf("\n");
	}
#endif

	if (gcom->other < 0) return(0);
	*other = gcom->other-1;

	messleng = gcom->numbytes;

	dacrc = ((unsigned short)gcom->buffer[messleng-2]);
	dacrc += (((unsigned short)gcom->buffer[messleng-1])<<8);
	if (dacrc != getcrc(gcom->buffer,messleng-2))        //CRC check
	{
#if (PRINTERRORS)
		printf("\n%ld CRC",gcom->buffer[0]);
#endif
		errorgotnum[*other] = errorfixnum[*other]+1;
		return(0);
	}

	while ((errorfixnum[*other]&7) != ((gcom->buffer[1]>>3)&7))
		errorfixnum[*other]++;

	if ((gcom->buffer[1]&7) != (errorresendnum[*other]&7))
	{
		errorresendnum[*other]++;
		outcntplc[*other] = (outcntend[*other]&0xffffff00)+gcom->buffer[2];
		if (outcntplc[*other] > outcntend[*other]) outcntplc[*other] -= 256;
	}

	if (gcom->buffer[0] != (incnt[*other]&255))   //CNT check
	{
		if (((incnt[*other]-gcom->buffer[0])&255) > 32)
		{
			errorgotnum[*other] = errorfixnum[*other]+1;
#if (PRINTERRORS)
			printf("\n%ld CNT",gcom->buffer[0]);
#endif
		}
#if (PRINTERRORS)
		else
		{
			if (!(gcom->buffer[1]&128))           //single else double packet
				printf("\n%ld cnt",gcom->buffer[0]);
			else
			{
				if (((gcom->buffer[0]+1)&255) == (incnt[*other]&255))
				{
								 //GOOD! Take second half of double packet
#if (PRINTERRORS)
					printf("\n%ld-%ld .û ",gcom->buffer[0],(gcom->buffer[0]+1)&255);
#endif
					messleng = ((long)gcom->buffer[3]) + (((long)gcom->buffer[4])<<8);
					lastpacketleng = gcom->numbytes-7-messleng;
					memcpy(bufptr,&gcom->buffer[messleng+5],lastpacketleng);
					incnt[*other]++;
					return(lastpacketleng);
				}
				else
					printf("\n%ld-%ld cnt ",gcom->buffer[0],(gcom->buffer[0]+1)&255);
			}
		}
#endif
		return(0);
	}

		//PACKET WAS GOOD!
	if ((gcom->buffer[1]&128) == 0)           //Single packet
	{
#if (PRINTERRORS)
		printf("\n%ld û  ",gcom->buffer[0]);
#endif

		messleng = gcom->numbytes-5;

		memcpy(bufptr,&gcom->buffer[3],messleng);

		incnt[*other]++;
		return(messleng);
	}

														 //Double packet
#if (PRINTERRORS)
	printf("\n%ld-%ld ûû ",gcom->buffer[0],(gcom->buffer[0]+1)&255);
#endif

	messleng = ((long)gcom->buffer[3]) + (((long)gcom->buffer[4])<<8);
	lastpacketleng = gcom->numbytes-7-messleng;
	inlastpacket = 1; lastpacketfrom = *other;

	memcpy(bufptr,&gcom->buffer[5],messleng);
	memcpy(lastpacket,&gcom->buffer[messleng+5],lastpacketleng);

	incnt[*other] += 2;
	return(messleng);
}

flushpackets()
{
	/*long i;

	if (numplayers < 2) return;

	do
	{
		gcom->command = 2;
		callcommit();
	} while (gcom->other >= 0);

	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		incnt[i] = 0L;
		outcntplc[i] = 0L;
		outcntend[i] = 0L;
		errorgotnum[i] = 0;
		errorfixnum[i] = 0;
		errorresendnum[i] = 0;
		lastsendtime[i] = totalclock;
	}*/
}

genericmultifunction(long other, char *bufptr, long messleng, long command)
{
	if (numplayers < 2) return;

	gcom->command = command;
	gcom->numbytes = min(messleng,MAXPACKETSIZE);
	copybuf(bufptr,gcom->buffer,(gcom->numbytes+3)>>2);
	gcom->other = other+1;
	callcommit();
}
