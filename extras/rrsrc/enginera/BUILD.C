// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include "build.h"
#include "pragmas.h"

#pragma intrinsic(min);
#pragma intrinsic(max);

#define MAXMENUFILES 256
#define updatecrc16(crc,dat) (crc = (((crc<<8)&65535)^crctable[((((unsigned short)crc)>>8)&65535)^dat]))
static long crctable[256];
static char kensig[24];

extern void ExtLoadMap(const char *mapname);
extern void ExtSaveMap(const char *mapname);
extern const char *ExtGetSectorCaption(short sectnum);
extern const char *ExtGetWallCaption(short wallnum);
extern const char *ExtGetSpriteCaption(short spritenum);
extern void ExtShowSectorData(short sectnum);
extern void ExtShowWallData(short wallnum);
extern void ExtShowSpriteData(short spritenum);
extern void ExtEditSectorData(short sectnum);
extern void ExtEditWallData(short wallnum);
extern void ExtEditSpriteData(short spritenum);

void (__interrupt __far *oldtimerhandler)();
void __interrupt __far timerhandler(void);

#define KEYFIFOSIZ 64
void (__interrupt __far *oldkeyhandler)();
void __interrupt __far keyhandler(void);
volatile char keystatus[256], keyfifo[KEYFIFOSIZ], keyfifoplc, keyfifoend;
volatile char readch, oldreadch, extended, keytemp;

long vel, svel, angvel;

#define NUMKEYS 19
char buildkeys[NUMKEYS] =
{
	0xc8,0xd0,0xcb,0xcd,0x2a,0x9d,0x1d,0x39,
	0x1e,0x2c,0xd1,0xc9,0x33,0x34,
	0x9c,0x1c,0xd,0xc,0xf,
};

long posx, posy, posz, horiz = 100;
short ang, cursectnum;
long hvel;

static long synctics = 0, lockclock = 0;

extern long stereomode;
extern char vgacompatible;

extern char picsiz[MAXTILES];
extern long startposx, startposy, startposz;
extern short startang, startsectnum;
extern long frameplace, pageoffset, ydim16;

extern long cachesize, artsize;

static short oldmousebstatus = 0, brightness = 0;
long zlock = 0x7fffffff, zmode = 0, whitecol, kensplayerheight = 32;
short defaultspritecstat = 0;

static short localartfreq[MAXTILES];
static short localartlookup[MAXTILES], localartlookupnum;

char tempbuf[4096];

char names[MAXTILES][17];

short asksave = 0;
extern short editstatus, searchit;
extern long searchx, searchy;                          //search input
extern short searchsector, searchwall, searchstat;     //search output

extern short pointhighlight, linehighlight, highlightcnt;
short grid = 3, gridlock = 1, showtags = 1;
long zoom = 768, gettilezoom = 1;

long numsprites;

short highlight[MAXWALLS];
short highlightsector[MAXSECTORS], highlightsectorcnt = -1;
extern char textfont[128][8];

static char pskysearch[MAXSECTORS];

short temppicnum, tempcstat, templotag, temphitag, tempextra;
char tempshade, temppal, tempvis, tempxrepeat, tempyrepeat;
char somethingintab = 255;
static char boardfilename[13], oboardfilename[13];

static long repeatcountx, repeatcounty;

static char menuname[MAXMENUFILES][17], curpath[80], menupath[80];
static long menunamecnt, menuhighlight;

static long fillist[640];

static char scantoasc[128] =
{
	0,0,'1','2','3','4','5','6','7','8','9','0','-','=',0,0,
	'q','w','e','r','t','y','u','i','o','p','[',']',0,0,'a','s',
	'd','f','g','h','j','k','l',';',39,'`',0,92,'z','x','c','v',
	'b','n','m',',','.','/',0,'*',0,32,0,0,0,0,0,0,
	0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+','1',
	'2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};
static char scantoascwithshift[128] =
{
	0,0,'!','@','#','$','%','^','&','*','(',')','_','+',0,0,
	'Q','W','E','R','T','Y','U','I','O','P','{','}',0,0,'A','S',
	'D','F','G','H','J','K','L',':',34,'~',0,'|','Z','X','C','V',
	'B','N','M','<','>','?',0,'*',0,32,0,0,0,0,0,0,
	0,0,0,0,0,0,0,'7','8','9','-','4','5','6','+','1',
	'2','3','0','.',0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

#pragma aux fillscreen16 =\
	"mov dx, 0x3ce",\
	"shl ax, 8",\
	"out dx, ax",\
	"mov ax, 0xff08",\
	"out dx, ax",\
	"shr ecx, 5",\
	"add edi, 0xa0000",\
	"rep stosd",\
	parm [edi][eax][ecx]\
	modify [edx]\

main(short int argc,char **argv)
{
	char ch, quitflag;
	long i, j, k;

	editstatus = 1;
	if (argc >= 2)
	{
		strcpy(&boardfilename,argv[1]);
		if (strchr(boardfilename,'.') == 0)
			strcat(boardfilename,".map");
	}
	else
		strcpy(&boardfilename,"newboard.map");

	ExtInit();
	initkeys();
	inittimer();

	loadpics("tiles000.art");
	loadnames();

	strcpy(kensig,"BUILD by Ken Silverman");
	initcrc();

	if (setgamemode(vidoption,xdim,ydim) < 0)
	{
		ExtUnInit();
		uninitkeys();
		uninittimer();
		printf("%ld * %ld not supported in this graphics mode\n",xdim,ydim);
		exit(0);
	}

	k = 0;
	for(i=0;i<256;i++)
	{
		j = ((long)palette[i*3])+((long)palette[i*3+1])+((long)palette[i*3+2]);
		if (j > k) { k = j; whitecol = i; }
	}

	initmenupaths(argv[0]);
	menunamecnt = 0;
	menuhighlight = 0;

	for(i=0;i<MAXSECTORS;i++) sector[i].extra = -1;
	for(i=0;i<MAXWALLS;i++) wall[i].extra = -1;
	for(i=0;i<MAXSPRITES;i++) sprite[i].extra = -1;

	if (loadboard(boardfilename,&posx,&posy,&posz,&ang,&cursectnum) == -1)
	{
		initspritelists();
		posx = 32768;
		posy = 32768;
		posz = 0;
		ang = 1536;
		numsectors = 0;
		numwalls = 0;
		cursectnum = -1;
		overheadeditor();
		keystatus[buildkeys[14]] = 0;
	}
	else
	{
		 ExtLoadMap(boardfilename);
	}

	updatenumsprites();

	startposx = posx;
	startposy = posy;
	startposz = posz;
	startang = ang;
	startsectnum = cursectnum;

	totalclock = 0;

	quitflag = 0;
	while (quitflag == 0)
	{
		ExtPreCheckKeys();

		drawrooms(posx,posy,posz,ang,horiz,cursectnum);
		ExtAnalyzeSprites();
		drawmasks();

		ExtCheckKeys();

		nextpage();
		synctics = totalclock-lockclock;
		lockclock += synctics;

		if (keystatus[1] > 0)
		{
			keystatus[1] = 0;
			printext256(0,0,whitecol,0,"Really want to quit?",0);

			nextpage();
			synctics = totalclock-lockclock;
			lockclock += synctics;

			while ((keystatus[1]|keystatus[0x1c]|keystatus[0x39]|keystatus[0x31]) == 0)
			{
				if (keystatus[0x15] != 0) { quitflag = 1; break; }
			}
		}
	}
	uninittimer();
	uninitkeys();
	ExtUnInit();
	uninitengine();
	setvmode(0x3);

	if (asksave)
	{
		printf("Save changes?\n");
		do
		{
			ch = getch();
		}
		while ((ch != 'y') && (ch != 'Y') && (ch != 'n') && (ch != 'N'));
		if ((ch == 'y') || (ch == 'Y'))
		{
			updatesector(startposx,startposy,&startsectnum);
			saveboard(boardfilename,&startposx,&startposy,&startposz,&startang,&startsectnum);
			ExtSaveMap(boardfilename);
		}
	}
	printf("Memory status: %ld(%ld) bytes\n",cachesize,artsize);
	printf("%s\n",kensig);
	return(0);
}

showmouse()
{
	long i;

	for(i=1;i<=4;i++)
	{
		plotpixel(searchx+i,searchy,whitecol);
		plotpixel(searchx-i,searchy,whitecol);
		plotpixel(searchx,searchy-i,whitecol);
		plotpixel(searchx,searchy+i,whitecol);
	}
}

editinput()
{
	char smooshyalign, repeatpanalign, *ptr, buffer[80];
	short sectnum, nextsectnum, startwall, endwall, dasector, daang;
	short mousx, mousy, mousz, bstatus;
	long i, j, k, cnt, templong, doubvel, changedir, wallfind[2], daz[2];
	long dashade[2], goalz, xvect, yvect, hiz, loz;
	short hitsect, hitwall, hitsprite;
	long hitx, hity, hitz, dax, day, hihit, lohit;

	if (keystatus[0x57] > 0)  //F11 - brightness
	{
		keystatus[0x57] = 0;
		brightness++;
		if (brightness > 16) brightness = 0;
		setbrightness(brightness,palette);
	}
	if (keystatus[88] > 0)   //F12
	{
		screencapture("captxxxx.pcx",keystatus[0x2a]|keystatus[0x36]);
		keystatus[88] = 0;
	}

	mousz = 0;
	getmousevalues(&mousx,&mousy,&bstatus);
	searchx += (mousx>>1);
	searchy += (mousy>>1);
	if (searchx < 4) searchx = 4;
	if (searchy < 4) searchy = 4;
	if (searchx > xdim-5) searchx = xdim-5;
	if (searchy > ydim-5) searchy = ydim-5;
	showmouse();

	if (keystatus[0x3b] > 0) posx--;
	if (keystatus[0x3c] > 0) posx++;
	if (keystatus[0x3d] > 0) posy--;
	if (keystatus[0x3e] > 0) posy++;
	if (keystatus[0x43] > 0) ang--;
	if (keystatus[0x44] > 0) ang++;

	if (angvel != 0)          //ang += angvel * constant
	{                         //ENGINE calculates angvel for you
		doubvel = synctics;
		if (keystatus[buildkeys[4]] > 0)  //Lt. shift makes turn velocity 50% faster
			doubvel += (synctics>>1);
		ang += ((angvel*doubvel)>>4);
		ang = (ang+2048)&2047;
	}
	if ((vel|svel) != 0)
	{
		doubvel = synctics;
		if (keystatus[buildkeys[4]] > 0)     //Lt. shift doubles forward velocity
			doubvel += synctics;
		xvect = 0, yvect = 0;
		if (vel != 0)
		{
			xvect += ((vel*doubvel*(long)sintable[(ang+2560)&2047])>>3);
			yvect += ((vel*doubvel*(long)sintable[(ang+2048)&2047])>>3);
		}
		if (svel != 0)
		{
			xvect += ((svel*doubvel*(long)sintable[(ang+2048)&2047])>>3);
			yvect += ((svel*doubvel*(long)sintable[(ang+1536)&2047])>>3);
		}
		clipmove(&posx,&posy,&posz,&cursectnum,xvect,yvect,128L,4L<<8,4L<<8,CLIPMASK0);
	}
	getzrange(posx,posy,posz,cursectnum,&hiz,&hihit,&loz,&lohit,128L,CLIPMASK0);

	if (keystatus[0x3a] > 0)
	{
		zmode++;
		if (zmode == 3) zmode = 0;
		if (zmode == 1) zlock = (loz-posz)&0xfffffc00;
		keystatus[0x3a] = 0;
	}

	if (zmode == 0)
	{
		goalz = loz-(kensplayerheight<<8);   //playerheight pixels above floor
		if (goalz < hiz+(16<<8))   //ceiling&floor too close
			goalz = ((loz+hiz)>>1);
		goalz += mousz;
		if (keystatus[buildkeys[8]] > 0)                            //A (stand high)
		{
			if (keystatus[0x1d] > 0)
				horiz = max(-100,horiz-((keystatus[buildkeys[4]]+1)<<2));
			else
			{
				goalz -= (16<<8);
				if (keystatus[buildkeys[4]] > 0)    //Either shift key
					goalz -= (24<<8);
			}
		}
		if (keystatus[buildkeys[9]] > 0)                            //Z (stand low)
		{
			if (keystatus[0x1d] > 0)
				horiz = min(300,horiz+((keystatus[buildkeys[4]]+1)<<2));
			else
			{
				goalz += (12<<8);
				if (keystatus[buildkeys[4]] > 0)    //Either shift key
					goalz += (12<<8);
			}
		}

		if (goalz != posz)
		{
			if (posz < goalz) hvel += 32;
			if (posz > goalz) hvel = ((goalz-posz)>>3);

			posz += hvel;
			if (posz > loz-(4<<8)) posz = loz-(4<<8), hvel = 0;
			if (posz < hiz+(4<<8)) posz = hiz+(4<<8), hvel = 0;
		}
	}
	else
	{
		goalz = posz;
		if (keystatus[buildkeys[8]] > 0)                            //A
		{
			if (keystatus[0x1d] > 0)
				horiz = max(-100,horiz-((keystatus[buildkeys[4]]+1)<<2));
			else
			{
				if (zmode != 1)
					goalz -= (8<<8);
				else
				{
					zlock += (4<<8);
					keystatus[buildkeys[8]] = 0;
				}
			}
		}
		if (keystatus[buildkeys[9]] > 0)                            //Z (stand low)
		{
			if (keystatus[0x1d] > 0)
				horiz = min(300,horiz+((keystatus[buildkeys[4]]+1)<<2));
			else
			{
				if (zmode != 1)
					goalz += (8<<8);
				else if (zlock > 0)
				{
					zlock -= (4<<8);
					keystatus[buildkeys[9]] = 0;
				}
			}
		}

		if (goalz < hiz+(4<<8)) goalz = hiz+(4<<8);
		if (goalz > loz-(4<<8)) goalz = loz-(4<<8);
		if (zmode == 1) goalz = loz-zlock;
		if (goalz < hiz+(4<<8)) goalz = ((loz+hiz)>>1);  //ceiling&floor too close
		if (zmode == 1) posz = goalz;

		if (goalz != posz)
		{
			if (posz < goalz) hvel += (32<<keystatus[buildkeys[4]]);
			if (posz > goalz) hvel -= (32<<keystatus[buildkeys[4]]);

			posz += hvel;

			if (posz > loz-(4<<8)) posz = loz-(4<<8), hvel = 0;
			if (posz < hiz+(4<<8)) posz = hiz+(4<<8), hvel = 0;
		}
		else
			hvel = 0;
	}

	searchit = 2;
	if (searchstat >= 0)
	{
		if ((bstatus&1) > 0)
			searchit = 0;
		if (keystatus[0x4a] > 0)  // -
		{
			keystatus[0x4a] = 0;
			if ((keystatus[0x38]|keystatus[0xb8]) > 0)  //ALT
			{
				if ((keystatus[0x1d]|keystatus[0x9d]) > 0)  //CTRL
				{
					if (visibility < 16384) visibility += visibility;
				}
				else
				{
					if ((keystatus[0x2a]|keystatus[0x36]) == 0)
						k = 16; else k = 1;

					if (highlightsectorcnt >= 0)
						for(i=0;i<highlightsectorcnt;i++)
							if (highlightsector[i] == searchsector)
							{
								while (k > 0)
								{
									for(i=0;i<highlightsectorcnt;i++)
									{
										sector[highlightsector[i]].visibility++;
										if (sector[highlightsector[i]].visibility == 240)
											sector[highlightsector[i]].visibility = 239;
									}
									k--;
								}
								break;
							}
					while (k > 0)
					{
						sector[searchsector].visibility++;
						if (sector[searchsector].visibility == 240)
							sector[searchsector].visibility = 239;
						k--;
					}
					asksave = 1;
				}
			}
			else
			{
				k = 0;
				if (highlightsectorcnt >= 0)
				{
					for(i=0;i<highlightsectorcnt;i++)
						if (highlightsector[i] == searchsector)
						{
							k = 1;
							break;
						}
				}

				if (k == 0)
				{
					if (searchstat == 0) wall[searchwall].shade++;
					if (searchstat == 1) sector[searchsector].ceilingshade++;
					if (searchstat == 2) sector[searchsector].floorshade++;
					if (searchstat == 3) sprite[searchwall].shade++;
					if (searchstat == 4) wall[searchwall].shade++;
				}
				else
				{
					for(i=0;i<highlightsectorcnt;i++)
					{
						dasector = highlightsector[i];

						sector[dasector].ceilingshade++;        //sector shade
						sector[dasector].floorshade++;

						startwall = sector[dasector].wallptr;   //wall shade
						endwall = startwall + sector[dasector].wallnum - 1;
						for(j=startwall;j<=endwall;j++)
							wall[j].shade++;

						j = headspritesect[dasector];           //sprite shade
						while (j != -1)
						{
							sprite[j].shade++;
							j = nextspritesect[j];
						}
					}
				}
				asksave = 1;
			}
		}
		if (keystatus[0x4e] > 0)  // +
		{
			keystatus[0x4e] = 0;
			if ((keystatus[0x38]|keystatus[0xb8]) > 0)  //ALT
			{
				if ((keystatus[0x1d]|keystatus[0x9d]) > 0)  //CTRL
				{
					if (visibility > 32) visibility >>= 1;
				}
				else
				{
					if ((keystatus[0x2a]|keystatus[0x36]) == 0)
						k = 16; else k = 1;

					if (highlightsectorcnt >= 0)
						for(i=0;i<highlightsectorcnt;i++)
							if (highlightsector[i] == searchsector)
							{
								while (k > 0)
								{
									for(i=0;i<highlightsectorcnt;i++)
									{
										sector[highlightsector[i]].visibility--;
										if (sector[highlightsector[i]].visibility == 239)
											sector[highlightsector[i]].visibility = 240;
									}
									k--;
								}
								break;
							}
					while (k > 0)
					{
						sector[searchsector].visibility--;
						if (sector[searchsector].visibility == 239)
							sector[searchsector].visibility = 240;
						k--;
					}
					asksave = 1;
				}
			}
			else
			{
				k = 0;
				if (highlightsectorcnt >= 0)
				{
					for(i=0;i<highlightsectorcnt;i++)
						if (highlightsector[i] == searchsector)
						{
							k = 1;
							break;
						}
				}

				if (k == 0)
				{
					if (searchstat == 0) wall[searchwall].shade--;
					if (searchstat == 1) sector[searchsector].ceilingshade--;
					if (searchstat == 2) sector[searchsector].floorshade--;
					if (searchstat == 3) sprite[searchwall].shade--;
					if (searchstat == 4) wall[searchwall].shade--;
				}
				else
				{
					for(i=0;i<highlightsectorcnt;i++)
					{
						dasector = highlightsector[i];

						sector[dasector].ceilingshade--;        //sector shade
						sector[dasector].floorshade--;

						startwall = sector[dasector].wallptr;   //wall shade
						endwall = startwall + sector[dasector].wallnum - 1;
						for(j=startwall;j<=endwall;j++)
							wall[j].shade--;

						j = headspritesect[dasector];           //sprite shade
						while (j != -1)
						{
							sprite[j].shade--;
							j = nextspritesect[j];
						}
					}
				}
				asksave = 1;
			}
		}
		if (keystatus[0xc9] > 0) // PGUP
		{
			k = 0;
			if (highlightsectorcnt >= 0)
			{
				for(i=0;i<highlightsectorcnt;i++)
					if (highlightsector[i] == searchsector)
					{
						k = 1;
						break;
					}
			}

			if ((searchstat == 0) || (searchstat == 1))
			{
				if (k == 0)
				{
					i = headspritesect[searchsector];
					while (i != -1)
					{
						templong = getceilzofslope(searchsector,sprite[i].x,sprite[i].y);
						templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<2);
						if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
						if (sprite[i].z == templong)
							sprite[i].z -= 1024;
						i = nextspritesect[i];
					}
					sector[searchsector].ceilingz -= 1024;
				}
				else
				{
					for(j=0;j<highlightsectorcnt;j++)
					{
						i = headspritesect[highlightsector[j]];
						while (i != -1)
						{
							templong = getceilzofslope(highlightsector[j],sprite[i].x,sprite[i].y);
							templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<2);
							if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
							if (sprite[i].z == templong)
								sprite[i].z -= 1024;
							i = nextspritesect[i];
						}
						sector[highlightsector[j]].ceilingz -= 1024;
					}
				}
			}
			if (searchstat == 2)
			{
				if (k == 0)
				{
					i = headspritesect[searchsector];
					while (i != -1)
					{
						templong = getflorzofslope(searchsector,sprite[i].x,sprite[i].y);
						if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
						if (sprite[i].z == templong)
							sprite[i].z -= 1024;
						i = nextspritesect[i];
					}
					sector[searchsector].floorz -= 1024;
				}
				else
				{
					for(j=0;j<highlightsectorcnt;j++)
					{
						i = headspritesect[highlightsector[j]];
						while (i != -1)
						{
							templong = getflorzofslope(highlightsector[j],sprite[i].x,sprite[i].y);
							if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
							if (sprite[i].z == templong)
								sprite[i].z -= 1024;
							i = nextspritesect[i];
						}
						sector[highlightsector[j]].floorz -= 1024;
					}
				}
			}
			if (sector[searchsector].floorz < sector[searchsector].ceilingz)
				sector[searchsector].floorz = sector[searchsector].ceilingz;
			if (searchstat == 3)
			{
				if ((keystatus[0x1d]|keystatus[0x9d]) > 0)  //CTRL - put sprite on ceiling
				{
					sprite[searchwall].z = getceilzofslope(searchsector,sprite[searchwall].x,sprite[searchwall].y);
					if (sprite[searchwall].cstat&128) sprite[searchwall].z -= ((tilesizy[sprite[searchwall].picnum]*sprite[searchwall].yrepeat)<<1);
					if ((sprite[searchwall].cstat&48) != 32)
						sprite[searchwall].z += ((tilesizy[sprite[searchwall].picnum]*sprite[searchwall].yrepeat)<<2);
				}
				else
				{
					k = 0;
					if (highlightcnt >= 0)
						for(i=0;i<highlightcnt;i++)
							if (highlight[i] == searchwall+16384)
							{
								k = 1;
								break;
							}

					if (k == 0)
						sprite[searchwall].z -= (4<<8);
					else
					{
						for(i=0;i<highlightcnt;i++)
							if ((highlight[i]&0xc000) == 16384)
								sprite[highlight[i]&16383].z -= (4<<8);
					}
				}
			}
			asksave = 1;
			keystatus[0xc9] = 0;
		}
		if (keystatus[0xd1] > 0) // PGDN
		{
			k = 0;
			if (highlightsectorcnt >= 0)
			{
				for(i=0;i<highlightsectorcnt;i++)
					if (highlightsector[i] == searchsector)
					{
						k = 1;
						break;
					}
			}

			if ((searchstat == 0) || (searchstat == 1))
			{
				if (k == 0)
				{
					i = headspritesect[searchsector];
					while (i != -1)
					{
						templong = getceilzofslope(searchsector,sprite[i].x,sprite[i].y);
						if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
						templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<2);
						if (sprite[i].z == templong)
							sprite[i].z += 1024;
						i = nextspritesect[i];
					}
					sector[searchsector].ceilingz += 1024;
				}
				else
				{
					for(j=0;j<highlightsectorcnt;j++)
					{
						i = headspritesect[highlightsector[j]];
						while (i != -1)
						{
							templong = getceilzofslope(highlightsector[j],sprite[i].x,sprite[i].y);
							if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
							templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<2);
							if (sprite[i].z == templong)
								sprite[i].z += 1024;
							i = nextspritesect[i];
						}
						sector[highlightsector[j]].ceilingz += 1024;
					}
				}
			}
			if (searchstat == 2)
			{
				if (k == 0)
				{
					i = headspritesect[searchsector];
					while (i != -1)
					{
						templong = getflorzofslope(searchsector,sprite[i].x,sprite[i].y);
						if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
						if (sprite[i].z == templong)
							sprite[i].z += 1024;
						i = nextspritesect[i];
					}
					sector[searchsector].floorz += 1024;
				}
				else
				{
					for(j=0;j<highlightsectorcnt;j++)
					{
						i = headspritesect[highlightsector[j]];
						while (i != -1)
						{
							templong = getflorzofslope(highlightsector[j],sprite[i].x,sprite[i].y);
							if (sprite[i].cstat&128) templong += ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
							if (sprite[i].z == templong)
								sprite[i].z += 1024;
							i = nextspritesect[i];
						}
						sector[highlightsector[j]].floorz += 1024;
					}
				}
			}
			if (sector[searchsector].ceilingz > sector[searchsector].floorz)
				sector[searchsector].ceilingz = sector[searchsector].floorz;
			if (searchstat == 3)
			{
				if ((keystatus[0x1d]|keystatus[0x9d]) > 0)  //CTRL - put sprite on ground
				{
					sprite[searchwall].z = getflorzofslope(searchsector,sprite[searchwall].x,sprite[searchwall].y);
					if (sprite[searchwall].cstat&128) sprite[searchwall].z -= ((tilesizy[sprite[searchwall].picnum]*sprite[searchwall].yrepeat)<<1);
				}
				else
				{
					k = 0;
					if (highlightcnt >= 0)
						for(i=0;i<highlightcnt;i++)
							if (highlight[i] == searchwall+16384)
							{
								k = 1;
								break;
							}

					if (k == 0)
						sprite[searchwall].z += (4<<8);
					else
					{
						for(i=0;i<highlightcnt;i++)
							if ((highlight[i]&0xc000) == 16384)
								sprite[highlight[i]&16383].z += (4<<8);
					}
				}
			}
			asksave = 1;
			keystatus[0xd1] = 0;
		}
		if (keystatus[0x0f] > 0)  //TAB
		{
			if (searchstat == 0)
			{
				temppicnum = wall[searchwall].picnum;
				tempshade = wall[searchwall].shade;
				temppal = wall[searchwall].pal;
				tempxrepeat = wall[searchwall].xrepeat;
				tempyrepeat = wall[searchwall].yrepeat;
				tempcstat = wall[searchwall].cstat;
				templotag = wall[searchwall].lotag;
				temphitag = wall[searchwall].hitag;
				tempextra = wall[searchwall].extra;
			}
			if (searchstat == 1)
			{
				temppicnum = sector[searchsector].ceilingpicnum;
				tempshade = sector[searchsector].ceilingshade;
				temppal = sector[searchsector].ceilingpal;
				tempvis = sector[searchsector].visibility;
				tempxrepeat = sector[searchsector].ceilingxpanning;
				tempyrepeat = sector[searchsector].ceilingypanning;
				tempcstat = sector[searchsector].ceilingstat;
				templotag = sector[searchsector].lotag;
				temphitag = sector[searchsector].hitag;
				tempextra = sector[searchsector].extra;
			}
			if (searchstat == 2)
			{
				temppicnum = sector[searchsector].floorpicnum;
				tempshade = sector[searchsector].floorshade;
				temppal = sector[searchsector].floorpal;
				tempvis = sector[searchsector].visibility;
				tempxrepeat = sector[searchsector].floorxpanning;
				tempyrepeat = sector[searchsector].floorypanning;
				tempcstat = sector[searchsector].floorstat;
				templotag = sector[searchsector].lotag;
				temphitag = sector[searchsector].hitag;
				tempextra = sector[searchsector].extra;
			}
			if (searchstat == 3)
			{
				temppicnum = sprite[searchwall].picnum;
				tempshade = sprite[searchwall].shade;
				temppal = sprite[searchwall].pal;
				tempxrepeat = sprite[searchwall].xrepeat;
				tempyrepeat = sprite[searchwall].yrepeat;
				tempcstat = sprite[searchwall].cstat;
				templotag = sprite[searchwall].lotag;
				temphitag = sprite[searchwall].hitag;
				tempextra = sprite[searchwall].extra;
			}
			if (searchstat == 4)
			{
				temppicnum = wall[searchwall].overpicnum;
				tempshade = wall[searchwall].shade;
				temppal = wall[searchwall].pal;
				tempxrepeat = wall[searchwall].xrepeat;
				tempyrepeat = wall[searchwall].yrepeat;
				tempcstat = wall[searchwall].cstat;
				templotag = wall[searchwall].lotag;
				temphitag = wall[searchwall].hitag;
				tempextra = wall[searchwall].extra;
			}
			somethingintab = searchstat;
			keystatus[0x0f] = 0;
		}
		if (keystatus[0x1c] > 0) //Left ENTER
		{
			if ((keystatus[0x2a]|keystatus[0x36]) > 0)       //Either shift key
			{
				if (((searchstat == 0) || (searchstat == 4)) && ((keystatus[0x1d]|keystatus[0x9d]) > 0))  //Ctrl-shift Enter (auto-shade)
				{
					dashade[0] = 127;
					dashade[1] = -128;
					i = searchwall;
					do
					{
						if ((long)wall[i].shade < dashade[0]) dashade[0] = wall[i].shade;
						if ((long)wall[i].shade > dashade[1]) dashade[1] = wall[i].shade;

						i = wall[i].point2;
					}
					while (i != searchwall);

					daang = getangle(wall[wall[searchwall].point2].x-wall[searchwall].x,wall[wall[searchwall].point2].y-wall[searchwall].y);
					i = searchwall;
					do
					{
						j = getangle(wall[wall[i].point2].x-wall[i].x,wall[wall[i].point2].y-wall[i].y);
						k = ((j+2048-daang)&2047);
						if (k > 1024)
							k = 2048-k;
						wall[i].shade = dashade[0]+mulscale10(k,dashade[1]-dashade[0]);

						i = wall[i].point2;
					}
					while (i != searchwall);
				}
				else if (somethingintab < 255)
				{
					if (searchstat == 0) wall[searchwall].shade = tempshade, wall[searchwall].pal = temppal;
					if (searchstat == 1)
					{
						sector[searchsector].ceilingshade = tempshade, sector[searchsector].ceilingpal = temppal;
						if ((somethingintab == 1) || (somethingintab == 2))
							sector[searchsector].visibility = tempvis;
					}
					if (searchstat == 2)
					{
						sector[searchsector].floorshade = tempshade, sector[searchsector].floorpal = temppal;
						if ((somethingintab == 1) || (somethingintab == 2))
							sector[searchsector].visibility = tempvis;
					}
					if (searchstat == 3) sprite[searchwall].shade = tempshade, sprite[searchwall].pal = temppal;
					if (searchstat == 4) wall[searchwall].shade = tempshade, wall[searchwall].pal = temppal;
				}
			}
			else if (((searchstat == 0) || (searchstat == 4)) && ((keystatus[0x1d]|keystatus[0x9d]) > 0) && (somethingintab < 255))  //Either ctrl key
			{
				i = searchwall;
				do
				{
					wall[i].picnum = temppicnum;
					wall[i].shade = tempshade;
					wall[i].pal = temppal;
					if ((somethingintab == 0) || (somethingintab == 4))
					{
						wall[i].xrepeat = tempxrepeat;
						wall[i].yrepeat = tempyrepeat;
						wall[i].cstat = tempcstat;
					}
					fixrepeats((short)i);
					i = wall[i].point2;
				}
				while (i != searchwall);
			}
			else if (((searchstat == 1) || (searchstat == 2)) && ((keystatus[0x1d]|keystatus[0x9d]) > 0) && (somethingintab < 255))  //Either ctrl key
			{
				clearbuf((long)(&pskysearch[0]),(long)((numsectors+3)>>2),0L);
				if (searchstat == 1)
				{
					i = searchsector;
					if ((sector[i].ceilingstat&1) > 0)
						pskysearch[i] = 1;

					while (pskysearch[i] == 1)
					{
						sector[i].ceilingpicnum = temppicnum;
						sector[i].ceilingshade = tempshade;
						sector[i].ceilingpal = temppal;
						if ((somethingintab == 1) || (somethingintab == 2))
						{
							sector[i].ceilingxpanning = tempxrepeat;
							sector[i].ceilingypanning = tempyrepeat;
							sector[i].ceilingstat = tempcstat;
						}
						pskysearch[i] = 2;

						startwall = sector[i].wallptr;
						endwall = startwall + sector[i].wallnum - 1;
						for(j=startwall;j<=endwall;j++)
						{
							k = wall[j].nextsector;
							if (k >= 0)
								if ((sector[k].ceilingstat&1) > 0)
									if (pskysearch[k] == 0)
										pskysearch[k] = 1;
						}

						for(j=0;j<numsectors;j++)
							if (pskysearch[j] == 1)
								i = j;
					}
				}
				if (searchstat == 2)
				{
					i = searchsector;
					if ((sector[i].floorstat&1) > 0)
						pskysearch[i] = 1;

					while (pskysearch[i] == 1)
					{
						sector[i].floorpicnum = temppicnum;
						sector[i].floorshade = tempshade;
						sector[i].floorpal = temppal;
						if ((somethingintab == 1) || (somethingintab == 2))
						{
							sector[i].floorxpanning = tempxrepeat;
							sector[i].floorypanning = tempyrepeat;
							sector[i].floorstat = tempcstat;
						}
						pskysearch[i] = 2;

						startwall = sector[i].wallptr;
						endwall = startwall + sector[i].wallnum - 1;
						for(j=startwall;j<=endwall;j++)
						{
							k = wall[j].nextsector;
							if (k >= 0)
								if ((sector[k].floorstat&1) > 0)
									if (pskysearch[k] == 0)
										pskysearch[k] = 1;
						}

						for(j=0;j<numsectors;j++)
							if (pskysearch[j] == 1)
								i = j;
					}
				}
			}
			else if (somethingintab < 255)
			{
				if (searchstat == 0)
				{
					wall[searchwall].picnum = temppicnum;
					wall[searchwall].shade = tempshade;
					wall[searchwall].pal = temppal;
					if (somethingintab == 0)
					{
						wall[searchwall].xrepeat = tempxrepeat;
						wall[searchwall].yrepeat = tempyrepeat;
						wall[searchwall].cstat = tempcstat;
						wall[searchwall].lotag = templotag;
						wall[searchwall].hitag = temphitag;
						wall[searchwall].extra = tempextra;
					}
					fixrepeats(searchwall);
				}
				if (searchstat == 1)
				{
					sector[searchsector].ceilingpicnum = temppicnum;
					sector[searchsector].ceilingshade = tempshade;
					sector[searchsector].ceilingpal = temppal;
					if ((somethingintab == 1) || (somethingintab == 2))
					{
						sector[searchsector].ceilingxpanning = tempxrepeat;
						sector[searchsector].ceilingypanning = tempyrepeat;
						sector[searchsector].ceilingstat = tempcstat;
						sector[searchsector].visibility = tempvis;
						sector[searchsector].lotag = templotag;
						sector[searchsector].hitag = temphitag;
						sector[searchsector].extra = tempextra;
					}
				}
				if (searchstat == 2)
				{
					sector[searchsector].floorpicnum = temppicnum;
					sector[searchsector].floorshade = tempshade;
					sector[searchsector].floorpal = temppal;
					if ((somethingintab == 1) || (somethingintab == 2))
					{
						sector[searchsector].floorxpanning= tempxrepeat;
						sector[searchsector].floorypanning= tempyrepeat;
						sector[searchsector].floorstat = tempcstat;
						sector[searchsector].visibility = tempvis;
						sector[searchsector].lotag = templotag;
						sector[searchsector].hitag = temphitag;
						sector[searchsector].extra = tempextra;
					}
				}
				if (searchstat == 3)
				{
					sprite[searchwall].picnum = temppicnum;
					if ((tilesizx[temppicnum] <= 0) || (tilesizy[temppicnum] <= 0))
					{
						j = 0;
						for(k=0;k<MAXTILES;k++)
							if ((tilesizx[k] > 0) && (tilesizy[k] > 0))
							{
								j = k;
								break;
							}
						sprite[searchwall].picnum = j;
					}
					sprite[searchwall].shade = tempshade;
					sprite[searchwall].pal = temppal;
					if (somethingintab == 3)
					{
						sprite[searchwall].xrepeat = tempxrepeat;
						sprite[searchwall].yrepeat = tempyrepeat;
						if (sprite[searchwall].xrepeat < 1) sprite[searchwall].xrepeat = 1;
						if (sprite[searchwall].yrepeat < 1) sprite[searchwall].yrepeat = 1;
						sprite[searchwall].cstat = tempcstat;
						sprite[searchwall].lotag = templotag;
						sprite[searchwall].hitag = temphitag;
						sprite[searchwall].extra = tempextra;
					}
				}
				if (searchstat == 4)
				{
					wall[searchwall].overpicnum = temppicnum;
					if (wall[searchwall].nextwall >= 0)
						wall[wall[searchwall].nextwall].overpicnum = temppicnum;
					wall[searchwall].shade = tempshade;
					wall[searchwall].pal = temppal;
					if (somethingintab == 4)
					{
						wall[searchwall].xrepeat = tempxrepeat;
						wall[searchwall].yrepeat = tempyrepeat;
						wall[searchwall].cstat = tempcstat;
						wall[searchwall].lotag = templotag;
						wall[searchwall].hitag = temphitag;
						wall[searchwall].extra = tempextra;
					}
					fixrepeats(searchwall);
				}
			}
			asksave = 1;
			keystatus[0x1c] = 0;
		}
		if (keystatus[0x2e] > 0)      //C
		{
			keystatus[0x2e] = 0;
			if (keystatus[0x38] > 0)    //Alt-C
			{
				if (somethingintab < 255)
				{
					switch(searchstat)
					{
						case 0:
							j = wall[searchwall].picnum;
							for(i=0;i<numwalls;i++)
								if (wall[i].picnum == j) wall[i].picnum = temppicnum;
							break;
						 case 1:
							j = sector[searchsector].ceilingpicnum;
							for(i=0;i<numsectors;i++)
								if (sector[i].ceilingpicnum == j) sector[i].ceilingpicnum = temppicnum;
							break;
						 case 2:
							j = sector[searchsector].floorpicnum;
							for(i=0;i<numsectors;i++)
								if (sector[i].floorpicnum == j) sector[i].floorpicnum = temppicnum;
							break;
						 case 3:
							 j = sprite[searchwall].picnum;
							 for(i=0;i<MAXSPRITES;i++)
								 if (sprite[i].statnum < MAXSTATUS)
									 if (sprite[i].picnum == j) sprite[i].picnum = temppicnum;
							 break;
						 case 4:
							 j = wall[searchwall].overpicnum;
							 for(i=0;i<numwalls;i++)
								 if (wall[i].overpicnum == j) wall[i].overpicnum = temppicnum;
							 break;
					}
				}
			}
			else    //C
			{
				if (searchstat == 3)
				{
					sprite[searchwall].cstat ^= 128;
					asksave = 1;
				}
			}
		}
		if (keystatus[0x2f] > 0)  //V
		{
			if (searchstat == 0) templong = wall[searchwall].picnum;
			if (searchstat == 1) templong = sector[searchsector].ceilingpicnum;
			if (searchstat == 2) templong = sector[searchsector].floorpicnum;
			if (searchstat == 3) templong = sprite[searchwall].picnum;
			if (searchstat == 4) templong = wall[searchwall].overpicnum;
			templong = gettile(templong);
			if (searchstat == 0) wall[searchwall].picnum = templong;
			if (searchstat == 1) sector[searchsector].ceilingpicnum = templong;
			if (searchstat == 2) sector[searchsector].floorpicnum = templong;
			if (searchstat == 3) sprite[searchwall].picnum = templong;
			if (searchstat == 4)
			{
				wall[searchwall].overpicnum = templong;
				if (wall[searchwall].nextwall >= 0)
					 wall[wall[searchwall].nextwall].overpicnum = templong;
			}
			asksave = 1;
			keystatus[0x2f] = 0;
		}

		if (keystatus[0x1a])  // [
		{
			keystatus[0x1a] = 0;
			if (keystatus[0x38]|keystatus[0xb8])
			{
				i = wall[searchwall].nextsector;
				if (i >= 0)
					switch(searchstat)
					{
						case 0: case 1: case 4:
							alignceilslope(searchsector,wall[searchwall].x,wall[searchwall].y,getceilzofslope(i,wall[searchwall].x,wall[searchwall].y));
							break;
						case 2:
							alignflorslope(searchsector,wall[searchwall].x,wall[searchwall].y,getflorzofslope(i,wall[searchwall].x,wall[searchwall].y));
							break;
					}
			}
			else
			{
				i = 512;
				if (keystatus[0x36]) i = 8;
				if (keystatus[0x2a]) i = 1;

				if (searchstat == 1)
				{
					if (!(sector[searchsector].ceilingstat&2))
						sector[searchsector].ceilingheinum = 0;
					sector[searchsector].ceilingheinum = max(sector[searchsector].ceilingheinum-i,-32768);
				}
				if (searchstat == 2)
				{
					if (!(sector[searchsector].floorstat&2))
						sector[searchsector].floorheinum = 0;
					sector[searchsector].floorheinum = max(sector[searchsector].floorheinum-i,-32768);
				}
			}

			if (sector[searchsector].ceilingheinum == 0)
				sector[searchsector].ceilingstat &= ~2;
			else
				sector[searchsector].ceilingstat |= 2;

			if (sector[searchsector].floorheinum == 0)
				sector[searchsector].floorstat &= ~2;
			else
				sector[searchsector].floorstat |= 2;
			asksave = 1;
		}
		if (keystatus[0x1b])  // ]
		{
			keystatus[0x1b] = 0;
			if (keystatus[0x38]|keystatus[0xb8])
			{
				i = wall[searchwall].nextsector;
				if (i >= 0)
					switch(searchstat)
					{
						case 1:
							alignceilslope(searchsector,wall[searchwall].x,wall[searchwall].y,getceilzofslope(i,wall[searchwall].x,wall[searchwall].y));
							break;
						case 0: case 2: case 4:
							alignflorslope(searchsector,wall[searchwall].x,wall[searchwall].y,getflorzofslope(i,wall[searchwall].x,wall[searchwall].y));
							break;
					}
			}
			else
			{
				i = 512;
				if (keystatus[0x36]) i = 8;
				if (keystatus[0x2a]) i = 1;

				if (searchstat == 1)
				{
					if (!(sector[searchsector].ceilingstat&2))
						sector[searchsector].ceilingheinum = 0;
					sector[searchsector].ceilingheinum = min(sector[searchsector].ceilingheinum+i,32767);
				}
				if (searchstat == 2)
				{
					if (!(sector[searchsector].floorstat&2))
						sector[searchsector].floorheinum = 0;
					sector[searchsector].floorheinum = min(sector[searchsector].floorheinum+i,32767);
				}
			}

			if (sector[searchsector].ceilingheinum == 0)
				sector[searchsector].ceilingstat &= ~2;
			else
				sector[searchsector].ceilingstat |= 2;

			if (sector[searchsector].floorheinum == 0)
				sector[searchsector].floorstat &= ~2;
			else
				sector[searchsector].floorstat |= 2;

			asksave = 1;
		}

		smooshyalign = keystatus[0x4c];
		repeatpanalign = (keystatus[0x2a]|keystatus[0x36]);
		if ((keystatus[0x4b]|keystatus[0x4d]) > 0)  // 4 & 6 (keypad)
		{
			repeatcountx++;

			if ((repeatcountx == 1) || (repeatcountx > 16))
			{
				changedir = 0;
				if (keystatus[0x4b] > 0) changedir = -1;
				if (keystatus[0x4d] > 0) changedir = 1;

				if ((searchstat == 0) || (searchstat == 4))
				{
					if (repeatpanalign == 0)
						wall[searchwall].xrepeat = changechar(wall[searchwall].xrepeat,changedir,smooshyalign,1);
					else
						wall[searchwall].xpanning = changechar(wall[searchwall].xpanning,changedir,smooshyalign,0);
				}
				if ((searchstat == 1) || (searchstat == 2))
				{
					if (searchstat == 1)
						sector[searchsector].ceilingxpanning = changechar(sector[searchsector].ceilingxpanning,changedir,smooshyalign,0);
					else
						sector[searchsector].floorxpanning = changechar(sector[searchsector].floorxpanning,changedir,smooshyalign,0);
				}
				if (searchstat == 3)
				{
					sprite[searchwall].xrepeat = changechar(sprite[searchwall].xrepeat,changedir,smooshyalign,1);
					if (sprite[searchwall].xrepeat < 4)
						sprite[searchwall].xrepeat = 4;
				}
				asksave = 1;
			}
		}
		else
			repeatcountx = 0;

		if ((keystatus[0x48]|keystatus[0x50]) > 0)  // 2 & 8 (keypad)
		{
			repeatcounty++;

			if ((repeatcounty == 1) || (repeatcounty > 16))
			{
				changedir = 0;
				if (keystatus[0x48] > 0) changedir = -1;
				if (keystatus[0x50] > 0) changedir = 1;

				if ((searchstat == 0) || (searchstat == 4))
				{
					if (repeatpanalign == 0)
						wall[searchwall].yrepeat = changechar(wall[searchwall].yrepeat,changedir,smooshyalign,1);
					else
						wall[searchwall].ypanning = changechar(wall[searchwall].ypanning,changedir,smooshyalign,0);
				}
				if ((searchstat == 1) || (searchstat == 2))
				{
					if (searchstat == 1)
						sector[searchsector].ceilingypanning = changechar(sector[searchsector].ceilingypanning,changedir,smooshyalign,0);
					else
						sector[searchsector].floorypanning = changechar(sector[searchsector].floorypanning,changedir,smooshyalign,0);
				}
				if (searchstat == 3)
				{
					sprite[searchwall].yrepeat = changechar(sprite[searchwall].yrepeat,changedir,smooshyalign,1);
					if (sprite[searchwall].yrepeat < 4)
						sprite[searchwall].yrepeat = 4;
				}
				asksave = 1;
			}
		}
		else
			repeatcounty = 0;

		if (keystatus[0x33] > 0) // , Search & fix panning to the left (3D)
		{
			if (searchstat == 3)
			{
				i = searchwall;
				if ((keystatus[0x2a]|keystatus[0x36]) > 0)
					sprite[i].ang = ((sprite[i].ang+2048-1)&2047);
				else
				{
					sprite[i].ang = ((sprite[i].ang+2048-128)&2047);
					keystatus[0x33] = 0;
				}
			}
		}
		if (keystatus[0x34] > 0) // . Search & fix panning to the right (3D)
		{
			if ((searchstat == 0) || (searchstat == 4))
			{
				AutoAlignWalls((long)searchwall,0L);

				/*wallfind[0] = searchwall;
				cnt = 4096;
				do
				{
					wallfind[1] = wall[wallfind[0]].point2;
					j = -1;
					if (wall[wallfind[1]].picnum == wall[searchwall].picnum)
						j = wallfind[1];
					k = wallfind[1];

					while ((wall[wallfind[1]].nextwall >= 0) && (wall[wall[wallfind[1]].nextwall].point2 != k))
					{
						i = wall[wall[wallfind[1]].nextwall].point2;   //break if going around in circles on red lines with same picture on both sides
						if (wallfind[1] == wall[wall[i].nextwall].point2)
							break;

						wallfind[1] = wall[wall[wallfind[1]].nextwall].point2;
						if (wall[wallfind[1]].picnum == wall[searchwall].picnum)
							j = wallfind[1];
					}
					wallfind[1] = j;

					if ((j >= 0) && (wallfind[1] != searchwall))
					{
						j = (wall[wallfind[0]].xpanning+(wall[wallfind[0]].xrepeat<<3)) % tilesizx[wall[wallfind[0]].picnum];
						wall[wallfind[1]].cstat &= ~8;    //Set to non-flip
						wall[wallfind[1]].cstat |= 4;     //Set y-orientation
						wall[wallfind[1]].xpanning = j;

						for(k=0;k<2;k++)
						{
							sectnum = sectorofwall((short)wallfind[k]);
							nextsectnum = wall[wallfind[k]].nextsector;

							if (nextsectnum == -1)
							{
								if ((wall[wallfind[k]].cstat&4) == 0)
									daz[k] = sector[sectnum].ceilingz;
								else
									daz[k] = sector[sectnum].floorz;
							}
							else                                      //topstep
							{
								if (sector[nextsectnum].ceilingz > sector[sectnum].ceilingz)
									daz[k] = sector[nextsectnum].ceilingz;
								else if (sector[nextsectnum].floorz < sector[sectnum].floorz)
									daz[k] = sector[nextsectnum].floorz;
							}
						}

						j = (picsiz[wall[searchwall].picnum]>>4);
						if ((1<<j) != tilesizy[wall[searchwall].picnum]) j++;

						j = ((wall[wallfind[0]].ypanning+(((daz[1]-daz[0])*wall[wallfind[0]].yrepeat)>>(j+3)))&255);
						wall[wallfind[1]].ypanning = j;
						wall[wallfind[1]].yrepeat = wall[wallfind[0]].yrepeat;
						if (nextsectnum >= 0)
							if (sector[nextsectnum].ceilingz >= sector[sectnum].ceilingz)
								if (sector[nextsectnum].floorz <= sector[sectnum].floorz)
								{
									if (wall[wall[wallfind[1]].nextwall].picnum == wall[searchwall].picnum)
									{
										wall[wall[wallfind[1]].nextwall].yrepeat = wall[wallfind[0]].yrepeat;
										if ((wall[wall[wallfind[1]].nextwall].cstat&4) == 0)
											daz[1] = sector[nextsectnum].floorz;
										else
											daz[1] = sector[sectnum].ceilingz;
										wall[wall[wallfind[1]].nextwall].ypanning = j;
									}
								}
					}
					wallfind[0] = wallfind[1];
					cnt--;
				}
				while ((wall[wallfind[0]].picnum == wall[searchwall].picnum) && (wallfind[0] != searchwall) && (cnt > 0));
				*/

				keystatus[0x34] = 0;
			}
			if (searchstat == 3)
			{
				i = searchwall;
				if ((keystatus[0x2a]|keystatus[0x36]) > 0)
					sprite[i].ang = ((sprite[i].ang+2048+1)&2047);
				else
				{
					sprite[i].ang = ((sprite[i].ang+2048+128)&2047);
					keystatus[0x34] = 0;
				}
			}
		}
		if (keystatus[0x35] > 0)  // /?     Reset panning&repeat to 0
		{
			if ((searchstat == 0) || (searchstat == 4))
			{
				wall[searchwall].xpanning = 0;
				wall[searchwall].ypanning = 0;
				wall[searchwall].xrepeat = 8;
				wall[searchwall].yrepeat = 8;
				wall[searchwall].cstat = 0;
				fixrepeats((short)searchwall);
			}
			if (searchstat == 1)
			{
				sector[searchsector].ceilingxpanning = 0;
				sector[searchsector].ceilingypanning = 0;
				sector[searchsector].ceilingstat &= ~2;
				sector[searchsector].ceilingheinum = 0;
			}
			if (searchstat == 2)
			{
				sector[searchsector].floorxpanning = 0;
				sector[searchsector].floorypanning = 0;
				sector[searchsector].floorstat &= ~2;
				sector[searchsector].floorheinum = 0;
			}
			if (searchstat == 3)
			{
				if ((keystatus[0x2a]|keystatus[0x36]) > 0)
				{
					sprite[searchwall].xrepeat = sprite[searchwall].yrepeat;
				}
				else
				{
					sprite[searchwall].xrepeat = 64;
					sprite[searchwall].yrepeat = 64;
				}
			}
			keystatus[0x35] = 0;
			asksave = 1;
		}

		if (keystatus[0x19] > 0)  // P (parallaxing sky)
		{
			if ((keystatus[0x1d]|keystatus[0x9d]) > 0)
			{
				parallaxtype++;
				if (parallaxtype == 3)
					parallaxtype = 0;
			}
			else if ((keystatus[0x38]|keystatus[0xb8]) > 0)
			{
				switch (searchstat)
				{
					case 0: case 4:
						strcpy(buffer,"Wall pal: ");
						wall[searchwall].pal = getnumber256(buffer,wall[searchwall].pal,256L);
						break;
					case 1:
						strcpy(buffer,"Ceiling pal: ");
						sector[searchsector].ceilingpal = getnumber256(buffer,sector[searchsector].ceilingpal,256L);
						break;
					case 2:
						strcpy(buffer,"Floor pal: ");
						sector[searchsector].floorpal = getnumber256(buffer,sector[searchsector].floorpal,256L);
						break;
					case 3:
						strcpy(buffer,"Sprite pal: ");
						sprite[searchwall].pal = getnumber256(buffer,sprite[searchwall].pal,256L);
						break;
				}
			}
			else
			{
				if ((searchstat == 0) || (searchstat == 1) || (searchstat == 4))
				{
					sector[searchsector].ceilingstat ^= 1;
					asksave = 1;
				}
				else if (searchstat == 2)
				{
					sector[searchsector].floorstat ^= 1;
					asksave = 1;
				}
			}
			keystatus[0x19] = 0;
		}

		if (keystatus[0x20] != 0)   //Alt-D  (adjust sprite[].clipdist)
		{
			keystatus[0x20] = 0;
			if ((keystatus[0x38]|keystatus[0xb8]) > 0)
			{
				if (searchstat == 3)
				{
					strcpy(buffer,"Sprite clipdist: ");
					sprite[searchwall].clipdist = getnumber256(buffer,sprite[searchwall].clipdist,256L);
				}
			}
		}

		if (keystatus[0x30] > 0)  // B (clip Blocking xor) (3D)
		{
			if (searchstat == 3)
			{
				sprite[searchwall].cstat ^= 1;
				sprite[searchwall].cstat &= ~256;
				sprite[searchwall].cstat |= ((sprite[searchwall].cstat&1)<<8);
				asksave = 1;
			}
			else
			{
				wall[searchwall].cstat ^= 1;
				wall[searchwall].cstat &= ~64;
				if ((wall[searchwall].nextwall >= 0) && ((keystatus[0x2a]|keystatus[0x36]) == 0))
				{
					wall[wall[searchwall].nextwall].cstat &= ~(1+64);
					wall[wall[searchwall].nextwall].cstat |= (wall[searchwall].cstat&1);
				}
				asksave = 1;
			}
			keystatus[0x30] = 0;
		}
		if (keystatus[0x14] > 0)  // T (transluscence for sprites/masked walls)
		{
			/*if (searchstat == 1)   //Set masked/transluscent ceilings/floors
			{
				i = (sector[searchsector].ceilingstat&(128+256));
				sector[searchsector].ceilingstat &= ~(128+256);
				switch(i)
				{
					case 0: sector[searchsector].ceilingstat |= 128; break;
					case 128: sector[searchsector].ceilingstat |= 256; break;
					case 256: sector[searchsector].ceilingstat |= 384; break;
					case 384: sector[searchsector].ceilingstat |= 0; break;
				}
				asksave = 1;
			}
			if (searchstat == 2)
			{
				i = (sector[searchsector].floorstat&(128+256));
				sector[searchsector].floorstat &= ~(128+256);
				switch(i)
				{
					case 0: sector[searchsector].floorstat |= 128; break;
					case 128: sector[searchsector].floorstat |= 256; break;
					case 256: sector[searchsector].floorstat |= 384; break;
					case 384: sector[searchsector].floorstat |= 0; break;
				}
				asksave = 1;
			}*/
			if (searchstat == 3)
			{
				if ((sprite[searchwall].cstat&2) == 0)
					sprite[searchwall].cstat |= 2;
				else if ((sprite[searchwall].cstat&512) == 0)
					sprite[searchwall].cstat |= 512;
				else
					sprite[searchwall].cstat &= ~(2+512);
				asksave = 1;
			}
			if (searchstat == 4)
			{
				if ((wall[searchwall].cstat&128) == 0)
					wall[searchwall].cstat |= 128;
				else if ((wall[searchwall].cstat&512) == 0)
					wall[searchwall].cstat |= 512;
				else
					wall[searchwall].cstat &= ~(128+512);

				if (wall[searchwall].nextwall >= 0)
				{
					wall[wall[searchwall].nextwall].cstat &= ~(128+512);
					wall[wall[searchwall].nextwall].cstat |= (wall[searchwall].cstat&(128+512));
				}
				asksave = 1;
			}
			keystatus[0x14] = 0;
		}

		if (keystatus[0x2] > 0)  // 1 (make 1-way wall)
		{
			if (searchstat != 3)
			{
				wall[searchwall].cstat ^= 32;
				asksave = 1;
			}
			else
			{
				sprite[searchwall].cstat ^= 64;
				i = sprite[searchwall].cstat;
				if ((i&48) == 32)
				{
					sprite[searchwall].cstat &= ~8;
					if ((i&64) > 0)
						if (posz > sprite[searchwall].z)
							sprite[searchwall].cstat |= 8;
				}
				asksave = 1;
			}
			keystatus[0x2] = 0;
		}
		if (keystatus[0x3] > 0)  // 2 (bottom wall swapping)
		{
			if (searchstat != 3)
			{
				wall[searchwall].cstat ^= 2;
				asksave = 1;
			}
			keystatus[0x3] = 0;
		}
		if (keystatus[0x18] > 0)  // O (top/bottom orientation - for doors)
		{
			if ((searchstat == 0) || (searchstat == 4))
			{
				wall[searchwall].cstat ^= 4;
				asksave = 1;
			}
			if (searchstat == 3)   // O (ornament onto wall) (2D)
			{
				asksave = 1;
				i = searchwall;

				hitscan(sprite[i].x,sprite[i].y,sprite[i].z,sprite[i].sectnum,
					sintable[(sprite[i].ang+2560+1024)&2047],
					sintable[(sprite[i].ang+2048+1024)&2047],
					0,
					&hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);

				sprite[i].x = hitx;
				sprite[i].y = hity;
				sprite[i].z = hitz;
				changespritesect(i,hitsect);
				if (hitwall >= 0)
					sprite[i].ang = ((getangle(wall[wall[hitwall].point2].x-wall[hitwall].x,wall[wall[hitwall].point2].y-wall[hitwall].y)+512)&2047);

					//Make sure sprite's in right sector
				if (inside(sprite[i].x,sprite[i].y,sprite[i].sectnum) == 0)
				{
					j = wall[hitwall].point2;
					sprite[i].x -= ksgn(wall[j].y-wall[hitwall].y);
					sprite[i].y += ksgn(wall[j].x-wall[hitwall].x);
				}
			}
			keystatus[0x18] = 0;
		}
		if (keystatus[0x32] > 0)  // M (masking walls)
		{
			if (searchstat != 3)
			{
				i = wall[searchwall].nextwall;
				templong = (keystatus[0x2a]|keystatus[0x36]);
				if (i >= 0)
				{
					wall[searchwall].cstat ^= 16;
					if ((wall[searchwall].cstat&16) > 0)
					{
						wall[searchwall].cstat &= ~8;
						if (templong == 0)
						{
							wall[i].cstat |= 8;           //auto other-side flip
							wall[i].cstat |= 16;
							wall[i].overpicnum = wall[searchwall].overpicnum;
						}
					}
					else
					{
						wall[searchwall].cstat &= ~8;
						if (templong == 0)
						{
							wall[i].cstat &= ~8;         //auto other-side unflip
							wall[i].cstat &= ~16;
						}
					}
					wall[searchwall].cstat &= ~32;
					if (templong == 0) wall[i].cstat &= ~32;
					asksave = 1;
				}
			}
			keystatus[0x32] = 0;
		}
		if (keystatus[0x23] > 0)  // H (hitscan sensitivity)
		{
			if (searchstat == 3)
			{
				sprite[searchwall].cstat ^= 256;
				asksave = 1;
			}
			else
			{
				wall[searchwall].cstat ^= 64;
				if ((wall[searchwall].nextwall >= 0) && ((keystatus[0x2a]|keystatus[0x36]) == 0))
				{
					wall[wall[searchwall].nextwall].cstat &= ~64;
					wall[wall[searchwall].nextwall].cstat |= (wall[searchwall].cstat&64);
				}
				asksave = 1;
			}
			keystatus[0x23] = 0;
		}
		if (keystatus[0x12] > 0)  // E (expand)
		{
			if (searchstat == 1)
			{
				sector[searchsector].ceilingstat ^= 8;
				asksave = 1;
			}
			if (searchstat == 2)
			{
				sector[searchsector].floorstat ^= 8;
				asksave = 1;
			}
			keystatus[0x12] = 0;
		}
		if (keystatus[0x13] > 0)  // R (relative alignment, rotation)
		{
			if (searchstat == 1)
			{
				sector[searchsector].ceilingstat ^= 64;
				asksave = 1;
			}
			if (searchstat == 2)
			{
				sector[searchsector].floorstat ^= 64;
				asksave = 1;
			}
			if (searchstat == 3)
			{
				i = sprite[searchwall].cstat;
				if ((i&48) < 32) i += 16; else i &= ~48;
				sprite[searchwall].cstat = i;
				asksave = 1;
			}
			keystatus[0x13] = 0;
		}
		if (keystatus[0x21] > 0)  //F (Flip)
		{
			keystatus[0x21] = 0;
			if ((keystatus[0x38]|keystatus[0xb8]) > 0)  //ALT-F (relative alignmment flip)
			{
				if (searchstat != 3)
				{
					setfirstwall(searchsector,searchwall);
					asksave = 1;
				}
			}
			else
			{
				if ((searchstat == 0) || (searchstat == 4))
				{
					i = wall[searchwall].cstat;
					i = ((i>>3)&1)+((i>>7)&2);    //3-x,8-y
					switch(i)
					{
						case 0: i = 1; break;
						case 1: i = 3; break;
						case 2: i = 0; break;
						case 3: i = 2; break;
					}
					i = ((i&1)<<3)+((i&2)<<7);
					wall[searchwall].cstat &= ~0x0108;
					wall[searchwall].cstat |= i;
					asksave = 1;
				}
				if (searchstat == 1)         //8-way ceiling flipping (bits 2,4,5)
				{
					i = sector[searchsector].ceilingstat;
					i = (i&0x4)+((i>>4)&3);
					switch(i)
					{
						case 0: i = 6; break;
						case 6: i = 3; break;
						case 3: i = 5; break;
						case 5: i = 1; break;
						case 1: i = 7; break;
						case 7: i = 2; break;
						case 2: i = 4; break;
						case 4: i = 0; break;
					}
					i = (i&0x4)+((i&3)<<4);
					sector[searchsector].ceilingstat &= ~0x34;
					sector[searchsector].ceilingstat |= i;
					asksave = 1;
				}
				if (searchstat == 2)         //8-way floor flipping (bits 2,4,5)
				{
					i = sector[searchsector].floorstat;
					i = (i&0x4)+((i>>4)&3);
					switch(i)
					{
						case 0: i = 6; break;
						case 6: i = 3; break;
						case 3: i = 5; break;
						case 5: i = 1; break;
						case 1: i = 7; break;
						case 7: i = 2; break;
						case 2: i = 4; break;
						case 4: i = 0; break;
					}
					i = (i&0x4)+((i&3)<<4);
					sector[searchsector].floorstat &= ~0x34;
					sector[searchsector].floorstat |= i;
					asksave = 1;
				}
				if (searchstat == 3)
				{
					i = sprite[searchwall].cstat;
					if (((i&48) == 32) && ((i&64) == 0))
					{
						sprite[searchwall].cstat &= ~0xc;
						sprite[searchwall].cstat |= (i&4^4);
					}
					else
					{
						i = ((i>>2)&3);
						switch(i)
						{
							case 0: i = 1; break;
							case 1: i = 3; break;
							case 2: i = 0; break;
							case 3: i = 2; break;
						}
						i <<= 2;
						sprite[searchwall].cstat &= ~0xc;
						sprite[searchwall].cstat |= i;
					}
					asksave = 1;
				}
			}
		}
		if (keystatus[0x1f] > 0)  //S (insert sprite) (3D)
		{
			dax = 16384;
			day = divscale14(searchx-(xdim>>1),xdim>>1);
			rotatepoint(0,0,dax,day,ang,&dax,&day);

			hitscan(posx,posy,posz,cursectnum,               //Start position
				dax,day,(scale(searchy,200,ydim)-horiz)*2000, //vector of 3D ang
				&hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);

			if (hitsect >= 0)
			{
				dax = hitx;
				day = hity;
				if ((gridlock > 0) && (grid > 0))
				{
					if ((searchstat == 0) || (searchstat == 4))
					{
						hitz = (hitz&0xfffffc00);
					}
					else
					{
						dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
						day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
					}
				}

				i = insertsprite(hitsect,0);
				sprite[i].x = dax, sprite[i].y = day;
				sprite[i].cstat = defaultspritecstat;
				sprite[i].shade = 0;
				sprite[i].pal = 0;
				sprite[i].xrepeat = 64, sprite[i].yrepeat = 64;
				sprite[i].xoffset = 0, sprite[i].yoffset = 0;
				sprite[i].ang = 1536;
				sprite[i].xvel = 0; sprite[i].yvel = 0; sprite[i].zvel = 0;
				sprite[i].owner = -1;
				sprite[i].clipdist = 32;
				sprite[i].lotag = 0;
				sprite[i].hitag = 0;
				sprite[i].extra = -1;

				for(k=0;k<MAXTILES;k++)
					localartfreq[k] = 0;
				for(k=0;k<MAXSPRITES;k++)
					if (sprite[k].statnum < MAXSTATUS)
						localartfreq[sprite[k].picnum]++;
				j = 0;
				for(k=0;k<MAXTILES;k++)
					if (localartfreq[k] > localartfreq[j])
						j = k;
				if (localartfreq[j] > 0)
					sprite[i].picnum = j;
				else
					sprite[i].picnum = 0;

				if (somethingintab == 3)
				{
					sprite[i].picnum = temppicnum;
					if ((tilesizx[temppicnum] <= 0) || (tilesizy[temppicnum] <= 0))
					{
						j = 0;
						for(k=0;k<MAXTILES;k++)
							if ((tilesizx[k] > 0) && (tilesizy[k] > 0))
							{
								j = k;
								break;
							}
						sprite[i].picnum = j;
					}
					sprite[i].shade = tempshade;
					sprite[i].pal = temppal;
					sprite[i].xrepeat = tempxrepeat;
					sprite[i].yrepeat = tempyrepeat;
					if (sprite[i].xrepeat < 1) sprite[i].xrepeat = 1;
					if (sprite[i].yrepeat < 1) sprite[i].yrepeat = 1;
					sprite[i].cstat = tempcstat;
				}

				j = ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);
				if ((sprite[i].cstat&128) == 0)
					sprite[i].z = min(max(hitz,getceilzofslope(hitsect,hitx,hity)+(j<<1)),getflorzofslope(hitsect,hitx,hity));
				else
					sprite[i].z = min(max(hitz,getceilzofslope(hitsect,hitx,hity)+j),getflorzofslope(hitsect,hitx,hity)-j);

				if ((searchstat == 0) || (searchstat == 4))
				{
					sprite[i].cstat |= (16+64);
					if (hitwall >= 0)
						sprite[i].ang = ((getangle(wall[wall[hitwall].point2].x-wall[hitwall].x,wall[wall[hitwall].point2].y-wall[hitwall].y)+512)&2047);

						//Make sure sprite's in right sector
					if (inside(sprite[i].x,sprite[i].y,sprite[i].sectnum) == 0)
					{
						j = wall[hitwall].point2;
						sprite[i].x -= ksgn(wall[j].y-wall[hitwall].y);
						sprite[i].y += ksgn(wall[j].x-wall[hitwall].x);
					}
				}
				else
				{
					if (tilesizy[sprite[i].picnum] >= 32) sprite[i].cstat |= 1;
				}

				updatenumsprites();
				asksave = 1;
			}

			keystatus[0x1f] = 0;
		}
		if (keystatus[0xd3] > 0)
		{
			if (searchstat == 3)
			{
				deletesprite(searchwall);
				updatenumsprites();
				asksave = 1;
			}
			keystatus[0xd3] = 0;
		}

		if ((keystatus[0x3f]|keystatus[0x40]) > 0)  //F5,F6
		{
			switch(searchstat)
			{
				case 1: case 2: ExtShowSectorData(searchsector); break;
				case 0: case 4: ExtShowWallData(searchwall); break;
				case 3: ExtShowSpriteData(searchwall); break;
			}
			keystatus[0x3f] = 0, keystatus[0x40] = 0;
		}
		if ((keystatus[0x41]|keystatus[0x42]) > 0)  //F7,F8
		{
			switch(searchstat)
			{
				case 1: case 2: ExtEditSectorData(searchsector); break;
				case 0: case 4: ExtEditWallData(searchwall); break;
				case 3: ExtEditSpriteData(searchwall); break;
			}
			keystatus[0x41] = 0, keystatus[0x42] = 0;
		}

	}
	if (keystatus[buildkeys[14]] > 0)  // Enter
	{
		overheadeditor();
		keystatus[buildkeys[14]] = 0;
	}
}

changechar(char dachar, long dadir, char smooshyalign, char boundcheck)
{
	if (dadir < 0)
	{
		if ((dachar > 0) || (boundcheck == 0))
		{
			dachar--;
			if (smooshyalign > 0)
				dachar = (dachar&0xf8);
		}
	}
	else if (dadir > 0)
	{
		if ((dachar < 255) || (boundcheck == 0))
		{
			dachar++;
			if (smooshyalign > 0)
			{
				if (dachar >= 256-8) dachar = 255;
									 else dachar = ((dachar+7)&0xf8);
			}
		}
	}
	return(dachar);
}

gettile(long tilenum)
{
	char snotbuf[80];
	long i, j, k, otilenum, topleft, gap, temp, templong;
	long xtiles, ytiles, tottiles;

	xtiles = (xdim>>6); ytiles = (ydim>>6); tottiles = xtiles*ytiles;
	otilenum = tilenum;

	keystatus[0x2f] = 0;
	for(i=0;i<MAXTILES;i++)
	{
		localartfreq[i] = 0;
		localartlookup[i] = i;
	}
	if ((searchstat == 1) || (searchstat == 2))
		for(i=0;i<numsectors;i++)
		{
			localartfreq[sector[i].ceilingpicnum]++;
			localartfreq[sector[i].floorpicnum]++;
		}
	if (searchstat == 0)
		for(i=0;i<numwalls;i++)
			localartfreq[wall[i].picnum]++;
	if (searchstat == 4)
		for(i=0;i<numwalls;i++)
			localartfreq[wall[i].overpicnum]++;
	if (searchstat == 3)
		for(i=0;i<MAXSPRITES;i++)
			if (sprite[i].statnum < MAXSTATUS)
				localartfreq[sprite[i].picnum]++;
	gap = (MAXTILES>>1);
	do
	{
		for(i=0;i<MAXTILES-gap;i++)
		{
			temp = i;
			while ((localartfreq[temp] < localartfreq[temp+gap]) && (temp >= 0))
			{
				templong = localartfreq[temp];
				localartfreq[temp] = localartfreq[temp+gap];
				localartfreq[temp+gap] = templong;
				templong = localartlookup[temp];
				localartlookup[temp] = localartlookup[temp+gap];
				localartlookup[temp+gap] = templong;

				if (tilenum == temp)
					tilenum = temp+gap;
				else if (tilenum == temp+gap)
					tilenum = temp;

				temp -= gap;
			}
		}
		gap >>= 1;
	}
	while (gap > 0);
	localartlookupnum = 0;
	while (localartfreq[localartlookupnum] > 0)
		localartlookupnum++;

	if (localartfreq[0] == 0)
	{
		tilenum = otilenum;
		localartlookupnum = MAXTILES;
		for(i=0;i<MAXTILES;i++)
			localartlookup[i] = i;
	}

	topleft = ((tilenum/(xtiles<<gettilezoom))*(xtiles<<gettilezoom))-(xtiles<<gettilezoom);
	if (topleft < 0) topleft = 0;
	if (topleft > MAXTILES-(tottiles<<(gettilezoom<<1))) topleft = MAXTILES-(tottiles<<(gettilezoom<<1));
	while ((keystatus[0x1c]|keystatus[1]) == 0)
	{
		drawtilescreen(topleft,tilenum);
		if ((vidoption != 2) && ((vidoption != 1) || (vgacompatible == 1))) limitrate();

		nextpage();
		synctics = totalclock-lockclock;
		lockclock += synctics;

		if ((keystatus[0x37] > 0) && (gettilezoom < 2))
		{
			gettilezoom++;
			topleft = ((tilenum/(xtiles<<gettilezoom))*(xtiles<<gettilezoom))-(xtiles<<gettilezoom);
			if (topleft < 0) topleft = 0;
			if (topleft > MAXTILES-(tottiles<<(gettilezoom<<1))) topleft = MAXTILES-(tottiles<<(gettilezoom<<1));
			keystatus[0x37] = 0;
		}
		if ((keystatus[0xb5] > 0) && (gettilezoom > 0))
		{
			gettilezoom--;
			topleft = ((tilenum/(xtiles<<gettilezoom))*(xtiles<<gettilezoom))-(xtiles<<gettilezoom);
			if (topleft < 0) topleft = 0;
			if (topleft > MAXTILES-(tottiles<<(gettilezoom<<1))) topleft = MAXTILES-(tottiles<<(gettilezoom<<1));
			keystatus[0xb5] = 0;
		}
		if ((keystatus[0xcb] > 0) && (tilenum > 0))
			tilenum--, keystatus[0xcb] = 0;
		if ((keystatus[0xcd] > 0) && (tilenum < MAXTILES-1))
			tilenum++, keystatus[0xcd] = 0;
		if ((keystatus[0xc8] > 0) && (tilenum >= (xtiles<<gettilezoom)))
			tilenum-=(xtiles<<gettilezoom), keystatus[0xc8] = 0;
		if ((keystatus[0xd0] > 0) && (tilenum < MAXTILES-(xtiles<<gettilezoom)))
			tilenum+=(xtiles<<gettilezoom), keystatus[0xd0] = 0;
		if ((keystatus[0xc9] > 0) && (tilenum >= (xtiles<<gettilezoom)))
		{
			tilenum-=(tottiles<<(gettilezoom<<1));
			if (tilenum < 0) tilenum = 0;
			keystatus[0xc9] = 0;
		}
		if ((keystatus[0xd1] > 0) && (tilenum < MAXTILES-(xtiles<<gettilezoom)))
		{
			tilenum+=(tottiles<<(gettilezoom<<1));
			if (tilenum >= MAXTILES) tilenum = MAXTILES-1;
			keystatus[0xd1] = 0;
		}
		if (keystatus[0x2f] > 0)   //V
		{
			keystatus[0x2f] = 0;
			if (tilenum < localartlookupnum)
				tilenum = localartlookup[tilenum];
			else
				tilenum = 0;
			localartlookupnum = MAXTILES;
			for(i=0;i<MAXTILES;i++)
				localartlookup[i] = i;
		}
		if (keystatus[0x22] > 0)       //G (goto)
		{
			if (tilenum < localartlookupnum)         //Automatically press 'V'
				tilenum = localartlookup[tilenum];
			else
				tilenum = 0;
			localartlookupnum = MAXTILES;
			for(i=0;i<MAXTILES;i++)
				localartlookup[i] = i;

			keystatus[0x22] = 0;

			j = tilenum;
			while (keystatus[1] == 0)
			{
				drawtilescreen(topleft,tilenum);
				if ((vidoption != 2) && ((vidoption != 1) || (vgacompatible == 1))) limitrate();
				sprintf(&snotbuf,"Goto tile: %d_ ",j);
				printext256(0,0,whitecol,0,snotbuf,0);
				nextpage();

				for(k=0;k<10;k++)
					if (keystatus[((k+9)%10)+2] > 0)
					{
						keystatus[((k+9)%10)+2] = 0;
						i = (j*10)+k;
						if (i < MAXTILES) j = i;
					}
				if (keystatus[0xe] > 0)
				{
					keystatus[0xe] = 0;
					j /= 10;
				}
				if (keystatus[0x1c] > 0)
				{
					keystatus[0x1c] = 0;
					tilenum = j;
					break;
				}
			}
		}
		while (tilenum < topleft) topleft -= (xtiles<<gettilezoom);
		while (tilenum >= topleft+(tottiles<<(gettilezoom<<1))) topleft += (xtiles<<gettilezoom);
		if (topleft < 0) topleft = 0;
		if (topleft > MAXTILES-(tottiles<<(gettilezoom<<1))) topleft = MAXTILES-(tottiles<<(gettilezoom<<1));
	}

	if (keystatus[0x1c] == 0)
	{
		tilenum = otilenum;
	}
	else
	{
		if (tilenum < localartlookupnum)
		{
			tilenum = localartlookup[tilenum];
			if ((tilesizx[tilenum] == 0) || (tilesizy[tilenum] == 0))
				tilenum = otilenum;
		}
		else
			tilenum = otilenum;
	}
	keystatus[0x1] = 0;
	keystatus[0x1c] = 0;
	return(tilenum);
}

drawtilescreen(long pictopleft, long picbox)
{
	long i, j, vidpos, vidpos2, dat, wallnum, xdime, ydime, cnt, pinc;
	long dax, day, scaledown, xtiles, ytiles, tottiles;
	char *picptr, snotbuf[80];

	xtiles = (xdim>>6); ytiles = (ydim>>6); tottiles = xtiles*ytiles;

	pinc = ylookup[1];
	clearview(0L);
	for(cnt=0;cnt<(tottiles<<(gettilezoom<<1));cnt++)         //draw the 5*3 grid of tiles
	{
		wallnum = cnt+pictopleft;
		if (wallnum < localartlookupnum)
		{
			wallnum = localartlookup[wallnum];
			if ((tilesizx[wallnum] != 0) && (tilesizy[wallnum] != 0))
			{
				if (waloff[wallnum] == 0) loadtile(wallnum);
				picptr = (char *)(waloff[wallnum]);
				xdime = tilesizx[wallnum];
				ydime = tilesizy[wallnum];

				dax = ((cnt%(xtiles<<gettilezoom))<<(6-gettilezoom));
				day = ((cnt/(xtiles<<gettilezoom))<<(6-gettilezoom));
				vidpos = ylookup[day]+dax+frameplace;
				if ((xdime <= (64>>gettilezoom)) && (ydime <= (64>>gettilezoom)))
				{
					for(i=0;i<xdime;i++)
					{
						vidpos2 = vidpos+i;
						for(j=0;j<ydime;j++)
						{
							*(char *)vidpos2 = *picptr++;
							vidpos2 += pinc;
						}
					}
				}
				else                          //if 1 dimension > 64
				{
					if (xdime > ydime)
						scaledown = ((xdime+(63>>gettilezoom))>>(6-gettilezoom));
					else
						scaledown = ((ydime+(63>>gettilezoom))>>(6-gettilezoom));

					for(i=0;i<xdime;i+=scaledown)
					{
						if (waloff[wallnum] == 0) loadtile(wallnum);
						picptr = (char *)(waloff[wallnum]) + ydime*i;
						vidpos2 = vidpos;
						for(j=0;j<ydime;j+=scaledown)
						{
							*(char *)vidpos2 = *picptr;
							picptr += scaledown;
							vidpos2 += pinc;
						}
						vidpos++;
					}
				}
				if (localartlookupnum < MAXTILES)
				{
					dax = ((cnt%(xtiles<<gettilezoom))<<(6-gettilezoom));
					day = ((cnt/(xtiles<<gettilezoom))<<(6-gettilezoom));
					sprintf(snotbuf,"%ld",localartfreq[cnt+pictopleft]);
					printext256(dax,day,whitecol,-1,snotbuf,1);
				}
			}
		}
	}

	cnt = picbox-pictopleft;    //draw open white box
	dax = ((cnt%(xtiles<<gettilezoom))<<(6-gettilezoom));
	day = ((cnt/(xtiles<<gettilezoom))<<(6-gettilezoom));

	for(i=0;i<(64>>gettilezoom);i++)
	{
		plotpixel(dax+i,day,whitecol);
		plotpixel(dax+i,day+(63>>gettilezoom),whitecol);
		plotpixel(dax,day+i,whitecol);
		plotpixel(dax+(63>>gettilezoom),day+i,whitecol);
	}

	i = localartlookup[picbox];
	sprintf(snotbuf,"%ld",i);
	printext256(0L,ydim-8,whitecol,-1,snotbuf,0);
	printext256(xdim-(strlen(names[i])<<3),ydim-8,whitecol,-1,names[i],0);

	return(0);
}

overheadeditor()
{
	char buffer[80], *dabuffer;
	long i, j, k, m, mousxplc, mousyplc, firstx, firsty, oposz, col;
	long templong, templong1, templong2, opageoffset, doubvel;
	long startwall, endwall, dax, day, daz, x1, y1, x2, y2, x3, y3, x4, y4;
	long highlightx1, highlighty1, highlightx2, highlighty2, xvect, yvect;
	short pag, suckwall, sucksect, newnumwalls, newnumsectors, split, bad;
	short splitsect, danumwalls, secondstartwall, joinsector[2], joinsectnum;
	short splitstartwall, splitendwall, loopnum;
	short mousx, mousy, bstatus;
	long centerx, centery, circlerad;
	short circlewall, circlepoints, circleang1, circleang2, circleangdir;
	long sectorhighlightx, sectorhighlighty;
	short cursectorhighlight, sectorhighlightstat;
	short hitsect, hitwall, hitsprite;
	long hitx, hity, hitz;
	walltype *wal;

	searchx = scale(searchx,640,xdim);
	searchy = scale(searchy,480,ydim);
	oposz = posz;

	qsetmode640480();

	pageoffset = 0; ydim16 = 144;
	drawline16(0,0,639,0,7);
	drawline16(0,143,639,143,7);
	drawline16(0,0,0,143,7);
	drawline16(639,0,639,143,7);
	drawline16(0,24,639,24,7);
	drawline16(192,0,192,24,7);
	printext16(9L,9L,4,-1,kensig,0);
	printext16(8L,8L,12,-1,kensig,0);
	printmessage16("Version: 9/23/95");
	drawline16(0,143-24,639,143-24,7);
	drawline16(256,143-24,256,143,7);
	pageoffset = 92160; ydim16 = 336;

	outpw(0x3d4,0x000c);
	outpw(0x3d4,0x000d);
	pag = 0;
	highlightcnt = -1;
	cursectorhighlight = -1;

		//White out all bordering lines of grab that are
		//not highlighted on both sides
	for(i=highlightsectorcnt-1;i>=0;i--)
	{
		startwall = sector[highlightsector[i]].wallptr;
		endwall = startwall + sector[highlightsector[i]].wallnum;
		for(j=startwall;j<endwall;j++)
		{
			if (wall[j].nextwall >= 0)
			{
				for(k=highlightsectorcnt-1;k>=0;k--)
					if (highlightsector[k] == wall[j].nextsector)
						break;
				if (k < 0)
				{
					wall[wall[j].nextwall].nextwall = -1;
					wall[wall[j].nextwall].nextsector = -1;
					wall[j].nextwall = -1;
					wall[j].nextsector = -1;
				}
			}
		}
	}

	for(i=0;i<(MAXWALLS>>3);i++)   //Clear all highlights
		show2dwall[i] = 0;
	for(i=0;i<(MAXSPRITES>>3);i++)
		show2dsprite[i] = 0;

	sectorhighlightstat = -1;
	newnumwalls = -1;
	joinsector[0] = -1;
	circlewall = -1;
	circlepoints = 7;
	oldmousebstatus = 0;
	keystatus[buildkeys[14]] = 0;
	while ((keystatus[buildkeys[14]]>>1) == 0)
	{
		oldmousebstatus = bstatus;
		getmousevalues(&mousx,&mousy,&bstatus);
		searchx += (mousx>>1);
		searchy += (mousy>>1);
		if (searchx < 8) searchx = 8;
		if (searchx > 631) searchx = 631;
		if (searchy < 8) searchy = 8;
		if (searchy > 341) searchy = 341;

		if (keystatus[0x3b] > 0) posx--, keystatus[0x3b] = 0;
		if (keystatus[0x3c] > 0) posx++, keystatus[0x3c] = 0;
		if (keystatus[0x3d] > 0) posy--, keystatus[0x3d] = 0;
		if (keystatus[0x3e] > 0) posy++, keystatus[0x3e] = 0;
		if (keystatus[0x43] > 0) ang--, keystatus[0x43] = 0;
		if (keystatus[0x44] > 0) ang++, keystatus[0x44] = 0;
		if (angvel != 0)          //ang += angvel * constant
		{                         //ENGINE calculates angvel for you
			doubvel = synctics;
			if (keystatus[buildkeys[4]] > 0)  //Lt. shift makes turn velocity 50% faster
				doubvel += (synctics>>1);
			ang += ((angvel*doubvel)>>4);
			ang = (ang+2048)&2047;
		}
		if ((vel|svel) != 0)
		{
			doubvel = synctics;
			if (keystatus[buildkeys[4]] > 0)     //Lt. shift doubles forward velocity
				doubvel += synctics;
			xvect = 0, yvect = 0;
			if (vel != 0)
			{
				xvect += ((vel*doubvel*(long)sintable[(ang+2560)&2047])>>3);
				yvect += ((vel*doubvel*(long)sintable[(ang+2048)&2047])>>3);
			}
			if (svel != 0)
			{
				xvect += ((svel*doubvel*(long)sintable[(ang+2048)&2047])>>3);
				yvect += ((svel*doubvel*(long)sintable[(ang+1536)&2047])>>3);
			}
			clipmove(&posx,&posy,&posz,&cursectnum,xvect,yvect,128L,4L<<8,4L<<8,CLIPMASK0);
		}

		getpoint(searchx,searchy,&mousxplc,&mousyplc);
		linehighlight = getlinehighlight(mousxplc, mousyplc);

		if (newnumwalls >= numwalls)
		{
			dax = mousxplc;
			day = mousyplc;
			adjustmark(&dax,&day,newnumwalls);
			wall[newnumwalls].x = dax;
			wall[newnumwalls].y = day;
		}

		ydim16 = 336;

		templong = numwalls;
		numwalls = newnumwalls;
		if (numwalls < 0) numwalls = templong;

		clear2dscreen();
		draw2dgrid(posx,posy,ang,zoom,grid);

		x2 = mulscale14(startposx-posx,zoom);          //Draw brown arrow (start)
		y2 = mulscale14(startposy-posy,zoom);
		if (((320+x2) >= 2) && ((320+x2) <= 637))
			if (((200+y2) >= 2) && ((200+y2) <= ydim16-3))
			{
				x1 = mulscale11(sintable[(startang+2560)&2047],zoom) / 768;
				y1 = mulscale11(sintable[(startang+2048)&2047],zoom) / 768;
				drawline16((320+x2)+x1,(200+y2)+y1,(320+x2)-x1,(200+y2)-y1,6);
				drawline16((320+x2)+x1,(200+y2)+y1,(320+x2)+y1,(200+y2)-x1,6);
				drawline16((320+x2)+x1,(200+y2)+y1,(320+x2)-y1,(200+y2)+x1,6);
			}

		draw2dscreen(posx,posy,ang,zoom,grid);

		if ((showtags == 1) && (zoom >= 768))
		{
			for(i=0;i<numsectors;i++)
			{
				dabuffer = (char *)ExtGetSectorCaption(i);
				if (dabuffer[0] != 0)
				{
					dax = 0;   //Get average point of sector
					day = 0;
					startwall = sector[i].wallptr;
					endwall = startwall + sector[i].wallnum - 1;
					for(j=startwall;j<=endwall;j++)
					{
						dax += wall[j].x;
						day += wall[j].y;
					}
					if (endwall > startwall)
					{
						dax /= (endwall-startwall+1);
						day /= (endwall-startwall+1);
					}

					dax = mulscale14(dax-posx,zoom);
					day = mulscale14(day-posy,zoom);

					x1 = 320+dax-(strlen(dabuffer)<<1);
					y1 = 200+day-4;
					x2 = x1 + (strlen(dabuffer)<<2)+2;
					y2 = y1 + 7;
					if ((x1 >= 0) && (x2 < 640) && (y1 >= 0) && (y2 < ydim16))
						printext16(x1,y1+(pageoffset/640),0,7,dabuffer,1);
				}
			}

			x3 = divscale14(-320,zoom)+posx;
			y3 = divscale14(-196,zoom)+posy;
			x4 = divscale14(320,zoom)+posx;
			y4 = divscale14(ydim16-196,zoom)+posy;

			for(i=numwalls-1,wal=&wall[i];i>=0;i--,wal--)
			{
					//Get average point of wall
				dax = ((wal->x+wall[wal->point2].x)>>1);
				day = ((wal->y+wall[wal->point2].y)>>1);
				if ((dax > x3) && (dax < x4) && (day > y3) && (day < y4))
				{
					dabuffer = (char *)ExtGetWallCaption(i);
					if (dabuffer[0] != 0)
					{
						dax = mulscale14(dax-posx,zoom);
						day = mulscale14(day-posy,zoom);
						x1 = 320+dax-(strlen(dabuffer)<<1);
						y1 = 200+day-4;
						x2 = x1 + (strlen(dabuffer)<<2)+2;
						y2 = y1 + 7;
						if ((x1 >= 0) && (x2 < 640) && (y1 >= 0) && (y2 < ydim16))
							printext16(x1,y1+(pageoffset/640),0,4,dabuffer,1);
					}
				}
			}

			i = 0; j = numsprites;
			while ((j > 0) && (i < MAXSPRITES))
			{
				if (sprite[i].statnum < MAXSTATUS)
				{
					dabuffer = (char *)ExtGetSpriteCaption(i);
					if (dabuffer[0] != 0)
					{
							//Get average point of sprite
						dax = sprite[i].x;
						day = sprite[i].y;

						dax = mulscale14(dax-posx,zoom);
						day = mulscale14(day-posy,zoom);

						x1 = 320+dax-(strlen(dabuffer)<<1);
						y1 = 200+day-4;
						x2 = x1 + (strlen(dabuffer)<<2)+2;
						y2 = y1 + 7;
						if ((x1 >= 0) && (x2 < 640) && (y1 >= 0) && (y2 < ydim16))
						{
							if ((sprite[i].cstat&1) == 0) col = 3; else col = 5;
							printext16(x1,y1+(pageoffset/640),0,col,dabuffer,1);
						}
					}
					j--;
				}
				i++;
			}
		}

		printcoords16(posx,posy,ang);

		numwalls = templong;

		if (highlightsectorcnt > 0)
			for(i=0;i<highlightsectorcnt;i++)
				fillsector(highlightsector[i],2);

		col = 15-((gridlock<<1)+gridlock);
		drawline16(searchx,searchy-8,searchx,searchy-1,col);
		drawline16(searchx+1,searchy-8,searchx+1,searchy-1,col);
		drawline16(searchx,searchy+2,searchx,searchy+9,col);
		drawline16(searchx+1,searchy+2,searchx+1,searchy+9,col);
		drawline16(searchx-8,searchy,searchx-1,searchy,col);
		drawline16(searchx-8,searchy+1,searchx-1,searchy+1,col);
		drawline16(searchx+2,searchy,searchx+9,searchy,col);
		drawline16(searchx+2,searchy+1,searchx+9,searchy+1,col);

			//Draw the white pixel closest to mouse cursor on linehighlight
		getclosestpointonwall(mousxplc,mousyplc,(long)linehighlight,&dax,&day);
		x2 = mulscale14(dax-posx,zoom);
		y2 = mulscale14(day-posy,zoom);
		if (wall[linehighlight].nextsector >= 0)
			drawline16(320+x2,200+y2,320+x2,200+y2,15);
		else
			drawline16(320+x2,200+y2,320+x2,200+y2,5);

		if (keystatus[88] > 0)   //F12
		{
			keystatus[88] = 0;
			i = pageoffset; pageoffset = 92160;
			j = ydim16; ydim16 = 480;
			outpw(0x3d4,0x000c);
			outpw(0x3d4,0x000d);
			clear2dscreen();
			draw2dgrid(posx,posy,ang,zoom,grid);
			draw2dscreen(posx,posy,ang,zoom,grid);

			screencapture("captxxxx.pcx",keystatus[0x2a]|keystatus[0x36]);

			pageoffset = i;
			ydim16 = j;
		}
		if (keystatus[0x30] > 0)  // B (clip Blocking xor) (2D)
		{
			pointhighlight = getpointhighlight(mousxplc, mousyplc);
			linehighlight = getlinehighlight(mousxplc, mousyplc);

			if ((pointhighlight&0xc000) == 16384)
			{
				sprite[pointhighlight&16383].cstat ^= 1;
				sprite[pointhighlight&16383].cstat &= ~256;
				sprite[pointhighlight&16383].cstat |= ((sprite[pointhighlight&16383].cstat&1)<<8);
				asksave = 1;
			}
			else if (linehighlight >= 0)
			{
				wall[linehighlight].cstat ^= 1;
				wall[linehighlight].cstat &= ~64;
				if ((wall[linehighlight].nextwall >= 0) && ((keystatus[0x2a]|keystatus[0x36]) == 0))
				{
					wall[wall[linehighlight].nextwall].cstat &= ~(1+64);
					wall[wall[linehighlight].nextwall].cstat |= (wall[linehighlight].cstat&1);
				}
				asksave = 1;
			}
			keystatus[0x30] = 0;
		}
		if (keystatus[0x21] > 0)  //F (F alone does nothing in 2D right now)
		{
			keystatus[0x21] = 0;
			if ((keystatus[0x38]|keystatus[0xb8]) > 0)  //ALT-F (relative alignmment flip)
			{
				linehighlight = getlinehighlight(mousxplc, mousyplc);
				if (linehighlight >= 0)
				{
					setfirstwall(sectorofwall(linehighlight),linehighlight);
					asksave = 1;
					printmessage16("This wall now sector's first wall (sector[].wallptr)");
				}
			}
		}

		if (keystatus[0x18] > 0)  // O (ornament onto wall) (2D)
		{
			keystatus[0x18] = 0;
			if ((pointhighlight&0xc000) == 16384)
			{
				asksave = 1;
				i = (pointhighlight&16383);

				hitscan(sprite[i].x,sprite[i].y,sprite[i].z,sprite[i].sectnum,
					sintable[(sprite[i].ang+2560+1024)&2047],
					sintable[(sprite[i].ang+2048+1024)&2047],
					0,
					&hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);

				sprite[i].x = hitx;
				sprite[i].y = hity;
				sprite[i].z = hitz;
				changespritesect(i,hitsect);
				if (hitwall >= 0)
					sprite[i].ang = ((getangle(wall[wall[hitwall].point2].x-wall[hitwall].x,wall[wall[hitwall].point2].y-wall[hitwall].y)+512)&2047);

					//Make sure sprite's in right sector
				if (inside(sprite[i].x,sprite[i].y,sprite[i].sectnum) == 0)
				{
					j = wall[hitwall].point2;
					sprite[i].x -= ksgn(wall[j].y-wall[hitwall].y);
					sprite[i].y += ksgn(wall[j].x-wall[hitwall].x);
				}
			}
		}

		if (keystatus[0x33] > 0)  // , (2D)
		{
			if (highlightsectorcnt > 0)
			{
				k = 0;
				dax = 0;
				day = 0;
				for(i=0;i<highlightsectorcnt;i++)
				{
					startwall = sector[highlightsector[i]].wallptr;
					endwall = startwall+sector[highlightsector[i]].wallnum-1;
					for(j=startwall;j<=endwall;j++)
					{
						dax += wall[j].x;
						day += wall[j].y;
						k++;
					}
				}
				if (k > 0)
				{
					dax /= k;
					day /= k;
				}

				k = (keystatus[0x2a]|keystatus[0x36]);

				if (k == 0)
				{
					if ((gridlock > 0) && (grid > 0))
					{
						dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
						day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
					}
				}

				for(i=0;i<highlightsectorcnt;i++)
				{
					startwall = sector[highlightsector[i]].wallptr;
					endwall = startwall+sector[highlightsector[i]].wallnum-1;
					for(j=startwall;j<=endwall;j++)
					{
						if (k == 0)
						{
							x3 = wall[j].x;
							y3 = wall[j].y;
							wall[j].x = dax+day-y3;
							wall[j].y = day+x3-dax;
						}
						else
						{
							rotatepoint(dax,day,wall[j].x,wall[j].y,1,&wall[j].x,&wall[j].y);
						}
					}

					j = headspritesect[highlightsector[i]];
					while (j != -1)
					{
						if (k == 0)
						{
							x3 = sprite[j].x;
							y3 = sprite[j].y;
							sprite[j].x = dax+day-y3;
							sprite[j].y = day+x3-dax;
							sprite[j].ang = ((sprite[j].ang+512)&2047);
						}
						else
						{
							rotatepoint(dax,day,sprite[j].x,sprite[j].y,1,&sprite[j].x,&sprite[j].y);
							sprite[j].ang = ((sprite[j].ang+1)&2047);
						}

						j = nextspritesect[j];
					}
				}
				if (k == 0) keystatus[0x33] = 0;
				asksave = 1;
			}
			else
			{
				if (pointhighlight >= 16384)
				{
					i = pointhighlight-16384;
					if ((keystatus[0x2a]|keystatus[0x36]) > 0)
						sprite[i].ang = ((sprite[i].ang+2048-1)&2047);
					else
					{
						sprite[i].ang = ((sprite[i].ang+2048-128)&2047);
						keystatus[0x33] = 0;
					}

					clearmidstatbar16();
					showspritedata((short)pointhighlight-16384);
				}
			}
		}
		if (keystatus[0x34] > 0)  // .  (2D)
		{
			if (highlightsectorcnt > 0)
			{
				k = 0;
				dax = 0;
				day = 0;
				for(i=0;i<highlightsectorcnt;i++)
				{
					startwall = sector[highlightsector[i]].wallptr;
					endwall = startwall+sector[highlightsector[i]].wallnum-1;
					for(j=startwall;j<=endwall;j++)
					{
						dax += wall[j].x;
						day += wall[j].y;
						k++;
					}
				}
				if (k > 0)
				{
					dax /= k;
					day /= k;
				}

				k = (keystatus[0x2a]|keystatus[0x36]);

				if (k == 0)
				{
					if ((gridlock > 0) && (grid > 0))
					{
						dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
						day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
					}
				}

				for(i=0;i<highlightsectorcnt;i++)
				{
					startwall = sector[highlightsector[i]].wallptr;
					endwall = startwall+sector[highlightsector[i]].wallnum-1;
					for(j=startwall;j<=endwall;j++)
					{
						if (k == 0)
						{
							x3 = wall[j].x;
							y3 = wall[j].y;
							wall[j].x = dax+y3-day;
							wall[j].y = day+dax-x3;
						}
						else
						{
							rotatepoint(dax,day,wall[j].x,wall[j].y,2047,&wall[j].x,&wall[j].y);
						}
					}

					j = headspritesect[highlightsector[i]];
					while (j != -1)
					{
						if (k == 0)
						{
							x3 = sprite[j].x;
							y3 = sprite[j].y;
							sprite[j].x = dax+y3-day;
							sprite[j].y = day+dax-x3;
							sprite[j].ang = ((sprite[j].ang+1536)&2047);
						}
						else
						{
							rotatepoint(dax,day,sprite[j].x,sprite[j].y,2047,&sprite[j].x,&sprite[j].y);
							sprite[j].ang = ((sprite[j].ang+2047)&2047);
						}

						j = nextspritesect[j];
					}
				}
				if (k == 0) keystatus[0x34] = 0;
				asksave = 1;
			}
			else
			{
				if (pointhighlight >= 16384)
				{
					i = pointhighlight-16384;
					if ((keystatus[0x2a]|keystatus[0x36]) > 0)
						sprite[i].ang = ((sprite[i].ang+2048+1)&2047);
					else
					{
						sprite[i].ang = ((sprite[i].ang+2048+128)&2047);
						keystatus[0x34] = 0;
					}

					clearmidstatbar16();
					showspritedata((short)pointhighlight-16384);
				}
			}
		}
		if (keystatus[0x46] > 0)  //Scroll lock (set starting position)
		{
			startposx = posx;
			startposy = posy;
			startposz = posz;
			startang = ang;
			startsectnum = cursectnum;
			keystatus[0x46] = 0;
			asksave = 1;
		}

		if (keystatus[0x3f] > 0)  //F5
		{
			keystatus[0x3f] = 0;

			for (i=0;i<numsectors;i++)
				if (inside(mousxplc,mousyplc,i) == 1)
				{
					opageoffset = pageoffset; pageoffset = 0; ydim16 = 144;
					ExtShowSectorData((short)i);
					pageoffset = opageoffset; ydim16 = 336;
					break;
				}
		}
		if (keystatus[0x40] > 0)  //F6
		{
			keystatus[0x40] = 0;

			if (pointhighlight >= 16384)
			{
				i = pointhighlight-16384;

				opageoffset = pageoffset; pageoffset = 0; ydim16 = 144;
				ExtShowSpriteData((short)i);
				pageoffset = opageoffset; ydim16 = 336;
			}
			else if (linehighlight >= 0)
			{
				i = linehighlight;

				opageoffset = pageoffset; pageoffset = 0; ydim16 = 144;
				ExtShowWallData((short)i);
				pageoffset = opageoffset; ydim16 = 336;
			}
		}
		if (keystatus[0x41] > 0)  //F7
		{
			keystatus[0x41] = 0;

			for (i=0;i<numsectors;i++)
				if (inside(mousxplc,mousyplc,i) == 1)
				{
					opageoffset = pageoffset; pageoffset = 0; ydim16 = 144;
					ExtEditSectorData((short)i);
					pageoffset = opageoffset; ydim16 = 336;
					break;
				}
		}
		if (keystatus[0x42] > 0)  //F8
		{
			keystatus[0x42] = 0;

			if (pointhighlight >= 16384)
			{
				i = pointhighlight-16384;

				opageoffset = pageoffset; pageoffset = 0; ydim16 = 144;
				ExtEditSpriteData((short)i);
				pageoffset = opageoffset; ydim16 = 336;
			}
			else if (linehighlight >= 0)
			{
				i = linehighlight;

				opageoffset = pageoffset; pageoffset = 0; ydim16 = 144;
				ExtEditWallData((short)i);
				pageoffset = opageoffset; ydim16 = 336;
			}
		}

		if (keystatus[0x14] > 0)  // T (tag)
		{
			keystatus[0x14] = 0;
			if ((keystatus[0x1d]|keystatus[0x9d]) > 0)  //Ctrl-T
			{
				showtags ^= 1;
				if (showtags == 0)
					printmessage16("Show tags OFF");
				else
					printmessage16("Show tags ON");
			}
			else if ((keystatus[0x38]|keystatus[0xb8]) > 0)  //ALT
			{
				if (pointhighlight >= 16384)
				{
					i = pointhighlight-16384;
					sprintf(buffer,"Sprite (%ld) Lo-tag: ",i);
					sprite[i].lotag = getnumber16(buffer,sprite[i].lotag,65536L);
					clearmidstatbar16();
					showspritedata((short)i);
				}
				else if (linehighlight >= 0)
				{
					i = linehighlight;
					sprintf(buffer,"Wall (%ld) Lo-tag: ",i);
					wall[i].lotag = getnumber16(buffer,wall[i].lotag,65536L);
					clearmidstatbar16();
					showwalldata((short)i);
				}
				printmessage16("");
			}
			else
			{
				for (i=0;i<numsectors;i++)
					if (inside(mousxplc,mousyplc,i) == 1)
					{
						sprintf(buffer,"Sector (%ld) Lo-tag: ",i);
						sector[i].lotag = getnumber16(buffer,sector[i].lotag,65536L);
						clearmidstatbar16();
						showsectordata((short)i);
						break;
					}
				printmessage16("");
			}
		}
		if (keystatus[0x23] > 0)  //H (Hi 16 bits of tag)
		{
			keystatus[0x23] = 0;
			if ((keystatus[0x1d]|keystatus[0x9d]) > 0)  //Ctrl-H
			{
				pointhighlight = getpointhighlight(mousxplc, mousyplc);
				linehighlight = getlinehighlight(mousxplc, mousyplc);

				if ((pointhighlight&0xc000) == 16384)
				{
					sprite[pointhighlight&16383].cstat ^= 256;
					asksave = 1;
				}
				else if (linehighlight >= 0)
				{
					wall[linehighlight].cstat ^= 64;
					if ((wall[linehighlight].nextwall >= 0) && ((keystatus[0x2a]|keystatus[0x36]) == 0))
					{
						wall[wall[linehighlight].nextwall].cstat &= ~64;
						wall[wall[linehighlight].nextwall].cstat |= (wall[linehighlight].cstat&64);
					}
					asksave = 1;
				}
			}
			else if ((keystatus[0x38]|keystatus[0xb8]) > 0)  //ALT
			{
				if (pointhighlight >= 16384)
				{
					i = pointhighlight-16384;
					sprintf(buffer,"Sprite (%ld) Hi-tag: ",i);
					sprite[i].hitag = getnumber16(buffer,sprite[i].hitag,65536L);
					clearmidstatbar16();
					showspritedata((short)i);
				}
				else if (linehighlight >= 0)
				{
					i = linehighlight;
					sprintf(buffer,"Wall (%ld) Hi-tag: ",i);
					wall[i].hitag = getnumber16(buffer,wall[i].hitag,65536L);
					clearmidstatbar16();
					showwalldata((short)i);
				}
			}
			else
			{
				for (i=0;i<numsectors;i++)
					if (inside(mousxplc,mousyplc,i) == 1)
					{
						sprintf(buffer,"Sector (%ld) Hi-tag: ",i);
						sector[i].hitag = getnumber16(buffer,sector[i].hitag,65536L);
						clearmidstatbar16();
						showsectordata((short)i);
						break;
					}
			}
			printmessage16("");
		}
		if (keystatus[0x19] > 0)  // P (palookup #)
		{
			keystatus[0x19] = 0;

			for (i=0;i<numsectors;i++)
				if (inside(mousxplc,mousyplc,i) == 1)
				{
					sprintf(buffer,"Sector (%ld) Ceilingpal: ",i);
					sector[i].ceilingpal = getnumber16(buffer,sector[i].ceilingpal,256L);
					clearmidstatbar16();
					showsectordata((short)i);

					sprintf(buffer,"Sector (%ld) Floorpal: ",i);
					sector[i].floorpal = getnumber16(buffer,sector[i].floorpal,256L);
					clearmidstatbar16();
					showsectordata((short)i);

					printmessage16("");
					break;
				}
		}
		if (keystatus[0x12] > 0)  // E (status list)
		{
			if (pointhighlight >= 16384)
			{
				i = pointhighlight-16384;
				sprintf(buffer,"Sprite (%ld) Status list: ",i);
				changespritestat(i,getnumber16(buffer,sprite[i].statnum,65536L));
				clearmidstatbar16();
				showspritedata((short)i);
			}

			printmessage16("");

			keystatus[0x12] = 0;
		}

		if (keystatus[0x0f] > 0)  //TAB
		{
			clearmidstatbar16();

			if ((keystatus[0x38]|keystatus[0xb8]) > 0)  //ALT
			{
				if (pointhighlight >= 16384)
					showspritedata((short)pointhighlight-16384);
				else if (linehighlight >= 0)
					showwalldata((short)linehighlight);
			}
			else
			{
				for (i=0;i<numsectors;i++)
					if (inside(mousxplc,mousyplc,i) == 1)
					{
						showsectordata((short)i);
						break;
					}
			}
			keystatus[0x0f] = 0;
		}


		if (highlightsectorcnt < 0)
		{
			if (keystatus[0x36] > 0)  //Right shift (point highlighting)
			{
				if (highlightcnt == 0)
				{
					highlightx2 = searchx, highlighty2 = searchy;
					ydim16 = 336;
					drawline16(highlightx2,highlighty1,highlightx1,highlighty1,5);
					drawline16(highlightx2,highlighty2,highlightx1,highlighty2,5);
					drawline16(highlightx1,highlighty2,highlightx1,highlighty1,5);
					drawline16(highlightx2,highlighty2,highlightx2,highlighty1,5);
				}
				if (highlightcnt != 0)
				{
					highlightx1 = searchx;
					highlighty1 = searchy;
					highlightx2 = searchx;
					highlighty2 = searchx;
					highlightcnt = 0;

					for(i=0;i<(MAXWALLS>>3);i++)   //Clear all highlights
						show2dwall[i] = 0;
					for(i=0;i<(MAXSPRITES>>3);i++)
						show2dsprite[i] = 0;
				}
			}
			else
			{
				if (highlightcnt == 0)
				{
					getpoint(highlightx1,highlighty1,&highlightx1,&highlighty1);
					getpoint(highlightx2,highlighty2,&highlightx2,&highlighty2);
					if (highlightx1 > highlightx2)
					{
						templong = highlightx1; highlightx1 = highlightx2; highlightx2 = templong;
					}
					if (highlighty1 > highlighty2)
					{
						templong = highlighty1; highlighty1 = highlighty2; highlighty2 = templong;
					}

					if ((keystatus[0x1d]|keystatus[0x9d]) > 0)
					{
						if ((linehighlight >= 0) && (linehighlight < MAXWALLS))
						{
							i = linehighlight;
							do
							{
								highlight[highlightcnt++] = i;
								show2dwall[i>>3] |= (1<<(i&7));

								for(j=0;j<numwalls;j++)
									if (wall[j].x == wall[i].x)
										if (wall[j].y == wall[i].y)
											if (i != j)
											{
												highlight[highlightcnt++] = j;
												show2dwall[j>>3] |= (1<<(j&7));
											}

								i = wall[i].point2;
							}
							while (i != linehighlight);
						}
					}
					else
					{
						for(i=0;i<numwalls;i++)
							if ((wall[i].x >= highlightx1) && (wall[i].x <= highlightx2))
								if ((wall[i].y >= highlighty1) && (wall[i].y <= highlighty2))
								{
									highlight[highlightcnt++] = i;
									show2dwall[i>>3] |= (1<<(i&7));
								}
						for(i=0;i<MAXSPRITES;i++)
							if (sprite[i].statnum < MAXSTATUS)
								if ((sprite[i].x >= highlightx1) && (sprite[i].x <= highlightx2))
									if ((sprite[i].y >= highlighty1) && (sprite[i].y <= highlighty2))
									{
										highlight[highlightcnt++] = i+16384;
										show2dsprite[i>>3] |= (1<<(i&7));
									}
					}

					if (highlightcnt <= 0)
						highlightcnt = -1;
				}
			}
		}
		if (highlightcnt < 0)
		{
			if (keystatus[0xb8] > 0)  //Right alt (sector highlighting)
			{
				if (highlightsectorcnt == 0)
				{
					highlightx2 = searchx, highlighty2 = searchy;
					ydim16 = 336;
					drawline16(highlightx2,highlighty1,highlightx1,highlighty1,10);
					drawline16(highlightx2,highlighty2,highlightx1,highlighty2,10);
					drawline16(highlightx1,highlighty2,highlightx1,highlighty1,10);
					drawline16(highlightx2,highlighty2,highlightx2,highlighty1,10);
				}
				if (highlightsectorcnt != 0)
				{
					for(i=0;i<highlightsectorcnt;i++)
					{
						startwall = sector[highlightsector[i]].wallptr;
						endwall = startwall+sector[highlightsector[i]].wallnum-1;
						for(j=startwall;j<=endwall;j++)
						{
							if (wall[j].nextwall >= 0)
								checksectorpointer(wall[j].nextwall,wall[j].nextsector);
							checksectorpointer((short)j,highlightsector[i]);
						}
					}
					highlightx1 = searchx;
					highlighty1 = searchy;
					highlightx2 = searchx;
					highlighty2 = searchx;
					highlightsectorcnt = 0;
				}
			}
			else
			{
				if (highlightsectorcnt == 0)
				{
					getpoint(highlightx1,highlighty1,&highlightx1,&highlighty1);
					getpoint(highlightx2,highlighty2,&highlightx2,&highlighty2);
					if (highlightx1 > highlightx2)
					{
						templong = highlightx1; highlightx1 = highlightx2; highlightx2 = templong;
					}
					if (highlighty1 > highlighty2)
					{
						templong = highlighty1; highlighty1 = highlighty2; highlighty2 = templong;
					}

					for(i=0;i<numsectors;i++)
					{
						startwall = sector[i].wallptr;
						endwall = startwall + sector[i].wallnum;
						bad = 0;
						for(j=startwall;j<endwall;j++)
						{
							if (wall[j].x < highlightx1) bad = 1;
							if (wall[j].x > highlightx2) bad = 1;
							if (wall[j].y < highlighty1) bad = 1;
							if (wall[j].y > highlighty2) bad = 1;
							if (bad == 1) break;
						}
						if (bad == 0)
							highlightsector[highlightsectorcnt++] = i;
					}
					if (highlightsectorcnt <= 0)
						highlightsectorcnt = -1;

						//White out all bordering lines of grab that are
						//not highlighted on both sides
					for(i=highlightsectorcnt-1;i>=0;i--)
					{
						startwall = sector[highlightsector[i]].wallptr;
						endwall = startwall + sector[highlightsector[i]].wallnum;
						for(j=startwall;j<endwall;j++)
						{
							if (wall[j].nextwall >= 0)
							{
								for(k=highlightsectorcnt-1;k>=0;k--)
									if (highlightsector[k] == wall[j].nextsector)
										break;
								if (k < 0)
								{
									wall[wall[j].nextwall].nextwall = -1;
									wall[wall[j].nextwall].nextsector = -1;
									wall[j].nextwall = -1;
									wall[j].nextsector = -1;
								}
							}
						}
					}

				}
			}
		}

		if (((bstatus&1) < (oldmousebstatus&1)) && (highlightsectorcnt < 0))  //after dragging
		{
			j = 1;
			if (highlightcnt > 0)
				for (i=0;i<highlightcnt;i++)
					if (pointhighlight == highlight[i])
					{
						j = 0;
						break;
					}

			if (j == 0)
			{
				for(i=0;i<highlightcnt;i++)
				{
					if ((highlight[i]&0xc000) == 16384)
					{
						j = (highlight[i]&16383);

						setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);

						templong = ((tilesizy[sprite[j].picnum]*sprite[j].yrepeat)<<2);
						sprite[j].z = max(sprite[j].z,getceilzofslope(sprite[j].sectnum,sprite[j].x,sprite[j].y)+templong);
						sprite[j].z = min(sprite[j].z,getflorzofslope(sprite[j].sectnum,sprite[j].x,sprite[j].y));
					}
				}
			}
			else if ((pointhighlight&0xc000) == 16384)
			{
				j = (pointhighlight&16383);

				setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);

				templong = ((tilesizy[sprite[j].picnum]*sprite[j].yrepeat)<<2);

				sprite[j].z = max(sprite[j].z,getceilzofslope(sprite[j].sectnum,sprite[j].x,sprite[j].y)+templong);
				sprite[j].z = min(sprite[j].z,getflorzofslope(sprite[j].sectnum,sprite[j].x,sprite[j].y));
			}

			if ((pointhighlight&0xc000) == 0)
			{
				dax = wall[pointhighlight].x;
				day = wall[pointhighlight].y;
			}
			else
			{
				dax = sprite[pointhighlight&16383].x;
				day = sprite[pointhighlight&16383].y;
			}

			for(i=numwalls-1;i>=0;i--)     //delete points
			{
				if (wall[i].x == wall[wall[i].point2].x)
					if (wall[i].y == wall[wall[i].point2].y)
					{
						deletepoint((short)i);
						printmessage16("Point deleted.");
						asksave = 1;
					}
			}
			for(i=0;i<numwalls;i++)        //make new red lines?
			{
				if ((wall[i].x == dax) && (wall[i].y == day))
				{
					checksectorpointer((short)i,sectorofwall((short)i));
					fixrepeats((short)i);
					asksave = 1;
				}
				else if ((wall[wall[i].point2].x == dax) && (wall[wall[i].point2].y == day))
				{
					checksectorpointer((short)i,sectorofwall((short)i));
					fixrepeats((short)i);
					asksave = 1;
				}
			}

		}

		if ((bstatus&1) > 0)                //drag points
		{
			if (highlightsectorcnt > 0)
			{
				if ((bstatus&1) > (oldmousebstatus&1))
				{
					newnumwalls = -1;
					sectorhighlightstat = -1;
					updatesector(mousxplc,mousyplc,&cursectorhighlight);

					if ((cursectorhighlight >= 0) && (cursectorhighlight < numsectors))
					{
						for (i=0;i<highlightsectorcnt;i++)
							if (cursectorhighlight == highlightsector[i])
							{
									//You clicked inside one of the flashing sectors!
								sectorhighlightstat = 1;

								dax = mousxplc;
								day = mousyplc;
								if ((gridlock > 0) && (grid > 0))
								{
									dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
									day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
								}
								sectorhighlightx = dax;
								sectorhighlighty = day;
								break;
							}
					}
				}
				else if (sectorhighlightstat == 1)
				{
					dax = mousxplc;
					day = mousyplc;
					if ((gridlock > 0) && (grid > 0))
					{
						dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
						day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
					}

					dax -= sectorhighlightx;
					day -= sectorhighlighty;
					sectorhighlightx += dax;
					sectorhighlighty += day;

					for(i=0;i<highlightsectorcnt;i++)
					{
						startwall = sector[highlightsector[i]].wallptr;
						endwall = startwall+sector[highlightsector[i]].wallnum-1;
						for(j=startwall;j<=endwall;j++)
							{ wall[j].x += dax; wall[j].y += day; }

						for(j=headspritesect[highlightsector[i]];j>=0;j=nextspritesect[j])
							{ sprite[j].x += dax; sprite[j].y += day; }
					}

					//for(i=0;i<highlightsectorcnt;i++)
					//{
					//   startwall = sector[highlightsector[i]].wallptr;
					//   endwall = startwall+sector[highlightsector[i]].wallnum-1;
					//   for(j=startwall;j<=endwall;j++)
					//   {
					//      if (wall[j].nextwall >= 0)
					//         checksectorpointer(wall[j].nextwall,wall[j].nextsector);
					//     checksectorpointer((short)j,highlightsector[i]);
					//   }
					//}
					asksave = 1;
				}

			}
			else
			{
				if ((bstatus&1) > (oldmousebstatus&1))
					pointhighlight = getpointhighlight(mousxplc, mousyplc);

				if (pointhighlight >= 0)
				{
					dax = mousxplc;
					day = mousyplc;
					if ((gridlock > 0) && (grid > 0))
					{
						dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
						day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
					}

					j = 1;
					if (highlightcnt > 0)
						for (i=0;i<highlightcnt;i++)
							if (pointhighlight == highlight[i])
								{ j = 0; break; }

					if (j == 0)
					{
						if ((pointhighlight&0xc000) == 0)
						{
							dax -= wall[pointhighlight].x;
							day -= wall[pointhighlight].y;
						}
						else
						{
							dax -= sprite[pointhighlight&16383].x;
							day -= sprite[pointhighlight&16383].y;
						}
						for(i=0;i<highlightcnt;i++)
						{
							if ((highlight[i]&0xc000) == 0)
							{
								wall[highlight[i]].x += dax;
								wall[highlight[i]].y += day;
							}
							else
							{
								sprite[highlight[i]&16383].x += dax;
								sprite[highlight[i]&16383].y += day;
							}
						}
					}
					else
					{
						if ((pointhighlight&0xc000) == 0)
							dragpoint(pointhighlight,dax,day);
						else if ((pointhighlight&0xc000) == 16384)
						{
							daz = ((tilesizy[sprite[pointhighlight&16383].picnum]*sprite[pointhighlight&16383].yrepeat)<<2);

							for(i=0;i<numsectors;i++)
								if (inside(dax,day,i) == 1)
									if (sprite[pointhighlight&16383].z >= getceilzofslope(i,dax,day))
										if (sprite[pointhighlight&16383].z-daz <= getflorzofslope(i,dax,day))
										{
											sprite[pointhighlight&16383].x = dax;
											sprite[pointhighlight&16383].y = day;
											if (sprite[pointhighlight&16383].sectnum != i)
												changespritesect(pointhighlight&16383,(short)i);
											break;
										}
						}
					}
					asksave = 1;
				}
			}
		}
		else
		{
			pointhighlight = getpointhighlight(mousxplc, mousyplc);
			sectorhighlightstat = -1;
		}

		if ((bstatus&6) > 0)
		{
			searchx = 320;
			searchy = 200;
			posx = mousxplc;
			posy = mousyplc;
		}

		if ((keystatus[buildkeys[8]] > 0) && (zoom < 16384)) zoom += (zoom>>4);
		if ((keystatus[buildkeys[9]] > 0) && (zoom > 24)) zoom -= (zoom>>4);

		if (keystatus[0x22] > 0)  // G (grid on/off)
		{
			grid++;
			if (grid == 7) grid = 0;
			keystatus[0x22] = 0;
		}
		if (keystatus[0x26] > 0)  // L (grid lock)
		{
			gridlock = 1-gridlock, keystatus[0x26] = 0;
			if (gridlock == 0)
				printmessage16("Grid locking OFF");
			else
				printmessage16("Grid locking ON");
		}

		if (keystatus[0x24] > 0)  // J (join sectors)
		{
			if (joinsector[0] >= 0)
			{
				joinsector[1] = -1;
				for(i=0;i<numsectors;i++)
					if (inside(mousxplc,mousyplc,i) == 1)
					{
						joinsector[1] = i;
						break;
					}
				if ((joinsector[1] >= 0) && (joinsector[0] != joinsector[1]))
				{
					newnumwalls = numwalls;

					for(k=0;k<2;k++)
					{
						startwall = sector[joinsector[k]].wallptr;
						endwall = startwall + sector[joinsector[k]].wallnum - 1;
						for(j=startwall;j<=endwall;j++)
						{
							if (wall[j].cstat == 255)
								continue;
							joinsectnum = k;
							if (wall[j].nextsector == joinsector[1-joinsectnum])
							{
								wall[j].cstat = 255;
								continue;
							}

							i = j;
							m = newnumwalls;
							do
							{
								memcpy(&wall[newnumwalls],&wall[i],sizeof(walltype));
								wall[newnumwalls].point2 = newnumwalls+1;
								newnumwalls++;
								wall[i].cstat = 255;

								i = wall[i].point2;
								if (wall[i].nextsector == joinsector[1-joinsectnum])
								{
									i = wall[wall[i].nextwall].point2;
									joinsectnum = 1 - joinsectnum;
								}
							}
							while ((wall[i].cstat != 255) && (wall[i].nextsector != joinsector[1-joinsectnum]));
							wall[newnumwalls-1].point2 = m;
						}
					}

					if (newnumwalls > numwalls)
					{
						memcpy(&sector[numsectors],&sector[joinsector[0]],sizeof(sectortype));
						sector[numsectors].wallptr = numwalls;
						sector[numsectors].wallnum = newnumwalls-numwalls;

						//fix sprites
						for(i=0;i<2;i++)
						{
							j = headspritesect[joinsector[i]];
							while (j != -1)
							{
								k = nextspritesect[j];
								changespritesect(j,numsectors);
								j = k;
							}
						}

						numsectors++;

						for(i=numwalls;i<newnumwalls;i++)
						{
							if (wall[i].nextwall >= 0)
							{
								wall[wall[i].nextwall].nextwall = i;
								wall[wall[i].nextwall].nextsector = numsectors-1;
							}
						}

						numwalls = newnumwalls;
						newnumwalls = -1;

						for(k=0;k<2;k++)
						{
							startwall = sector[joinsector[k]].wallptr;
							endwall = startwall + sector[joinsector[k]].wallnum - 1;
							for(j=startwall;j<=endwall;j++)
							{
								wall[j].nextwall = -1;
								wall[j].nextsector = -1;
							}
						}

						deletesector((short)joinsector[0]);
						if (joinsector[0] < joinsector[1])
							joinsector[1]--;
						deletesector((short)joinsector[1]);
						printmessage16("Sectors joined.");
					}
				}
				joinsector[0] = -1;
			}
			else
			{
				joinsector[0] = -1;
				for(i=0;i<numsectors;i++)
					if (inside(mousxplc,mousyplc,i) == 1)
					{
						joinsector[0] = i;
						printmessage16("Join sector - press J again on sector to join with.");
						break;
					}
			}
			keystatus[0x24] = 0;
		}

		if (((keystatus[0x38]|keystatus[0xb8])&keystatus[0x1f]) > 0) //ALT-S
		{
			if ((linehighlight >= 0) && (wall[linehighlight].nextwall == -1))
			{
				if ((newnumwalls = whitelinescan(linehighlight)) < numwalls)
				{
					printmessage16("Can't make a sector out there.");
				}
				else
				{
					for(i=numwalls;i<newnumwalls;i++)
					{
						wall[wall[i].nextwall].nextwall = i;
						wall[wall[i].nextwall].nextsector = numsectors;
					}
					numwalls = newnumwalls;
					newnumwalls = -1;
					numsectors++;
					printmessage16("Inner loop made into new sector.");
				}
			}
			keystatus[0x1f] = 0;
		}
		else if (keystatus[0x1f] > 0)  //S
		{
			sucksect = -1;
			for(i=0;i<numsectors;i++)
				if (inside(mousxplc,mousyplc,i) == 1)
				{
					sucksect = i;
					break;
				}

			if (sucksect >= 0)
			{
				dax = mousxplc;
				day = mousyplc;
				if ((gridlock > 0) && (grid > 0))
				{
					dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
					day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
				}

				i = insertsprite(sucksect,0);
				sprite[i].x = dax, sprite[i].y = day;
				sprite[i].cstat = defaultspritecstat;
				sprite[i].shade = 0;
				sprite[i].pal = 0;
				sprite[i].xrepeat = 64, sprite[i].yrepeat = 64;
				sprite[i].xoffset = 0, sprite[i].yoffset = 0;
				sprite[i].ang = 1536;
				sprite[i].xvel = 0; sprite[i].yvel = 0; sprite[i].zvel = 0;
				sprite[i].owner = -1;
				sprite[i].clipdist = 32;
				sprite[i].lotag = 0;
				sprite[i].hitag = 0;
				sprite[i].extra = -1;

				sprite[i].z = getflorzofslope(sucksect,dax,day);
				if ((sprite[i].cstat&128) != 0)
					sprite[i].z -= ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<1);

				for(k=0;k<MAXTILES;k++)
					localartfreq[k] = 0;
				for(k=0;k<MAXSPRITES;k++)
					if (sprite[k].statnum < MAXSTATUS)
						localartfreq[sprite[k].picnum]++;
				j = 0;
				for(k=0;k<MAXTILES;k++)
					if (localartfreq[k] > localartfreq[j])
						j = k;
				if (localartfreq[j] > 0)
					sprite[i].picnum = j;
				else
					sprite[i].picnum = 0;

				if (somethingintab == 3)
				{
					sprite[i].picnum = temppicnum;
					if ((tilesizx[temppicnum] <= 0) || (tilesizy[temppicnum] <= 0))
					{
						j = 0;
						for(k=0;k<MAXTILES;k++)
							if ((tilesizx[k] > 0) && (tilesizy[k] > 0))
							{
								j = k;
								break;
							}
						sprite[i].picnum = j;
					}
					sprite[i].shade = tempshade;
					sprite[i].pal = temppal;
					sprite[i].xrepeat = tempxrepeat;
					sprite[i].yrepeat = tempyrepeat;
					if (sprite[i].xrepeat < 1) sprite[i].xrepeat = 1;
					if (sprite[i].yrepeat < 1) sprite[i].yrepeat = 1;
					sprite[i].cstat = tempcstat;
				}

				if (tilesizy[sprite[i].picnum] >= 32)
					sprite[i].cstat |= 1;

				printmessage16("Sprite inserted.");
				updatenumsprites();
				asksave = 1;
			}

			keystatus[0x1f] = 0;
		}

		if (keystatus[0x2e] > 0)  // C (make circle of points)
		{
			if (circlewall >= 0)
			{
				circlewall = -1;
			}
			else
			{
				if (linehighlight >= 0)
					circlewall = linehighlight;
			}
			keystatus[0x2e] = 0;
		}
		if (keystatus[0x4a] > 0)  // -
		{
			if (circlepoints > 1)
				circlepoints--;
			keystatus[0x4a] = 0;
		}
		if (keystatus[0x4e] > 0)  // +
		{
			if (circlepoints < 63)
				circlepoints++;
			keystatus[0x4e] = 0;
		}

		bad = (keystatus[0x39] > 0);  //Gotta do this to save lots of 3 spaces!

		if (circlewall >= 0)
		{
			x1 = wall[circlewall].x;
			y1 = wall[circlewall].y;
			x2 = wall[wall[circlewall].point2].x;
			y2 = wall[wall[circlewall].point2].y;
			x3 = mousxplc;
			y3 = mousyplc;
			adjustmark(&x3,&y3,newnumwalls);
			templong1 = dmulscale4(x3-x2,x1-x3,y1-y3,y3-y2);
			templong2 = dmulscale4(y1-y2,x1-x3,y1-y3,x2-x1);
			if (templong2 != 0)
			{
				centerx = (((x1+x2) + scale(y1-y2,templong1,templong2))>>1);
				centery = (((y1+y2) + scale(x2-x1,templong1,templong2))>>1);

				dax = mulscale14(centerx-posx,zoom);
				day = mulscale14(centery-posy,zoom);
				drawline16(320+dax-2,200+day-2,320+dax+2,200+day+2,14);
				drawline16(320+dax-2,200+day+2,320+dax+2,200+day-2,14);

				circleang1 = getangle(x1-centerx,y1-centery);
				circleang2 = getangle(x2-centerx,y2-centery);

				circleangdir = 1;
				k = ((circleang2-circleang1)&2047);
				if (mulscale4(x3-x1,y2-y1) < mulscale4(x2-x1,y3-y1))
				{
					circleangdir = -1;
					k = -((circleang1-circleang2)&2047);
				}

				circlerad = (ksqrt(dmulscale4(centerx-x1,centerx-x1,centery-y1,centery-y1))<<2);

				for(i=circlepoints;i>0;i--)
				{
					j = ((circleang1 + scale(i,k,circlepoints+1))&2047);
					dax = centerx+mulscale14(sintable[(j+512)&2047],circlerad);
					day = centery+mulscale14(sintable[j],circlerad);

					if (dax <= -131072) dax = -131072;
					if (dax >= 131072) dax = 131072;
					if (day <= -131072) day = -131072;
					if (day >= 131072) day = 131072;

					if (bad > 0)
					{
						m = 0;
						if (wall[circlewall].nextwall >= 0)
							if (wall[circlewall].nextwall < circlewall) m = 1;
						insertpoint(circlewall,dax,day);
						circlewall += m;
					}
					dax = mulscale14(dax-posx,zoom);
					day = mulscale14(day-posy,zoom);
					drawline16(320+dax-2,200+day-2,320+dax+2,200+day-2,14);
					drawline16(320+dax+2,200+day-2,320+dax+2,200+day+2,14);
					drawline16(320+dax+2,200+day+2,320+dax-2,200+day+2,14);
					drawline16(320+dax-2,200+day+2,320+dax-2,200+day-2,14);
				}
				if (bad > 0)
				{
					bad = 0;
					keystatus[0x39] = 0;
					asksave = 1;
					printmessage16("Circle points inserted.");
					circlewall = -1;
				}
			}
		}

		if (bad > 0)   //Space bar test
		{
			keystatus[0x39] = 0;
			adjustmark(&mousxplc,&mousyplc,newnumwalls);
			if (checkautoinsert(mousxplc,mousyplc,newnumwalls) == 1)
			{
				printmessage16("You must insert a point there first.");
				bad = 0;
			}
		}

		if (bad > 0)  //Space
		{
			if ((newnumwalls < numwalls) && (numwalls < MAXWALLS-1))
			{
				firstx = mousxplc, firsty = mousyplc;  //Make first point
				newnumwalls = numwalls;
				suckwall = -1;
				split = 0;

				clearbufbyte(FP_OFF(&wall[newnumwalls]),sizeof(walltype),0L);
				wall[newnumwalls].extra = -1;

				wall[newnumwalls].x = mousxplc;
				wall[newnumwalls].y = mousyplc;
				wall[newnumwalls].nextsector = -1;
				wall[newnumwalls].nextwall = -1;
				for(i=0;i<numwalls;i++)
					if ((wall[i].x == mousxplc) && (wall[i].y == mousyplc))
						suckwall = i;
				wall[newnumwalls].point2 = newnumwalls+1;
				printmessage16("Sector drawing started.");
				newnumwalls++;
			}
			else
			{  //if not back to first point
				if ((firstx != mousxplc) || (firsty != mousyplc))  //nextpoint
				{
					j = 0;
					for(i=numwalls;i<newnumwalls;i++)
						if ((mousxplc == wall[i].x) && (mousyplc == wall[i].y))
							j = 1;
					if (j == 0)
					{
							//check if starting to split a sector
						if (newnumwalls == numwalls+1)
						{
							dax = ((wall[numwalls].x+mousxplc)>>1);
							day = ((wall[numwalls].y+mousyplc)>>1);
							for(i=0;i<numsectors;i++)
								if (inside(dax,day,i) == 1)
								{    //check if first point at point of sector
									m = -1;
									startwall = sector[i].wallptr;
									endwall = startwall + sector[i].wallnum - 1;
									for(k=startwall;k<=endwall;k++)
										if (wall[k].x == wall[numwalls].x)
											if (wall[k].y == wall[numwalls].y)
											{
												m = k;
												break;
											}
									if (m >= 0)
										if ((wall[wall[k].point2].x != mousxplc) || (wall[wall[k].point2].y != mousyplc))
											if ((wall[lastwall((short)k)].x != mousxplc) || (wall[lastwall((short)k)].y != mousyplc))
											{
												split = 1;
												splitsect = i;
												splitstartwall = m;
												break;
											}
								}
						}

							//make new point

						//make sure not drawing over old red line
						bad = 0;
						for(i=0;i<numwalls;i++)
						{
							if (wall[i].nextwall >= 0)
							{
								if ((wall[i].x == mousxplc) && (wall[i].y == mousyplc))
									if ((wall[wall[i].point2].x == wall[newnumwalls-1].x) && (wall[wall[i].point2].y == wall[newnumwalls-1].y))
										bad = 1;
								if ((wall[i].x == wall[newnumwalls-1].x) && (wall[i].y == wall[newnumwalls-1].y))
									if ((wall[wall[i].point2].x == mousxplc) && (wall[wall[i].point2].y == mousyplc))
										bad = 1;
							}
						}

						if (bad == 0)
						{
							clearbufbyte(FP_OFF(&wall[newnumwalls]),sizeof(walltype),0L);
							wall[newnumwalls].extra = -1;

							wall[newnumwalls].x = mousxplc;
							wall[newnumwalls].y = mousyplc;
							wall[newnumwalls].nextsector = -1;
							wall[newnumwalls].nextwall = -1;
							for(i=0;i<numwalls;i++)
								if ((wall[i].x == mousxplc) && (wall[i].y == mousyplc))
									suckwall = i;
							wall[newnumwalls].point2 = newnumwalls+1;
							newnumwalls++;
						}
						else
						{
							printmessage16("You can't draw new lines over red lines.");
						}
					}
				}

					//if not split and back to first point
				if ((split == 0) && (firstx == mousxplc) && (firsty == mousyplc) && (newnumwalls >= numwalls+3))
				{
					wall[newnumwalls-1].point2 = numwalls;

					if (suckwall == -1)  //if no connections to other sectors
					{
						k = -1;
						for(i=0;i<numsectors;i++)
							if (inside(firstx,firsty,i) == 1)
								k = i;
						if (k == -1)   //if not inside another sector either
						{              //add island sector
							if (clockdir(numwalls) == 1)
								flipwalls(numwalls,newnumwalls);

							clearbufbyte(FP_OFF(&sector[numsectors]),sizeof(sectortype),0L);
							sector[numsectors].extra = -1;

							sector[numsectors].wallptr = numwalls;
							sector[numsectors].wallnum = newnumwalls-numwalls;
							sector[numsectors].ceilingz = (-32<<8);
							sector[numsectors].floorz = (32<<8);
							for(i=numwalls;i<newnumwalls;i++)
							{
								wall[i].cstat = 0;
								wall[i].shade = 0;
								wall[i].yrepeat = 8;
								fixrepeats((short)i);
								wall[i].picnum = 0;
								wall[i].overpicnum = 0;
								wall[i].nextsector = -1;
								wall[i].nextwall = -1;
							}
							headspritesect[numsectors] = -1;
							numsectors++;
						}
						else       //else add loop to sector
						{
							if (clockdir(numwalls) == 0)
								flipwalls(numwalls,newnumwalls);

							j = newnumwalls-numwalls;

							sector[k].wallnum += j;
							for(i=k+1;i<numsectors;i++)
								sector[i].wallptr += j;
							suckwall = sector[k].wallptr;

							for(i=0;i<numwalls;i++)
							{
								if (wall[i].nextwall >= suckwall)
									wall[i].nextwall += j;
								if (wall[i].point2 >= suckwall)
									wall[i].point2 += j;
							}

							for(i=newnumwalls-1;i>=suckwall;i--)
								memcpy(&wall[i+j],&wall[i],sizeof(walltype));
							for(i=0;i<j;i++)
								memcpy(&wall[i+suckwall],&wall[i+newnumwalls],sizeof(walltype));

							for(i=suckwall;i<suckwall+j;i++)
							{
								wall[i].point2 += (suckwall-numwalls);

								wall[i].cstat = wall[suckwall+j].cstat;
								wall[i].shade = wall[suckwall+j].shade;
								wall[i].yrepeat = wall[suckwall+j].yrepeat;
								fixrepeats((short)i);
								wall[i].picnum = wall[suckwall+j].picnum;
								wall[i].overpicnum = wall[suckwall+j].overpicnum;

								wall[i].nextsector = -1;
								wall[i].nextwall = -1;
							}
						}
					}
					else
					{
						  //add new sector with connections
						if (clockdir(numwalls) == 1)
							flipwalls(numwalls,newnumwalls);

						clearbufbyte(FP_OFF(&sector[numsectors]),sizeof(sectortype),0L);
						sector[numsectors].extra = -1;

						sector[numsectors].wallptr = numwalls;
						sector[numsectors].wallnum = newnumwalls-numwalls;
						sucksect = sectorofwall(suckwall);
						sector[numsectors].ceilingstat = sector[sucksect].ceilingstat;
						sector[numsectors].floorstat = sector[sucksect].floorstat;
						sector[numsectors].ceilingxpanning = sector[sucksect].ceilingxpanning;
						sector[numsectors].floorxpanning = sector[sucksect].floorxpanning;
						sector[numsectors].ceilingshade = sector[sucksect].ceilingshade;
						sector[numsectors].floorshade = sector[sucksect].floorshade;
						sector[numsectors].ceilingz = sector[sucksect].ceilingz;
						sector[numsectors].floorz = sector[sucksect].floorz;
						sector[numsectors].ceilingpicnum = sector[sucksect].ceilingpicnum;
						sector[numsectors].floorpicnum = sector[sucksect].floorpicnum;
						sector[numsectors].ceilingheinum = sector[sucksect].ceilingheinum;
						sector[numsectors].floorheinum = sector[sucksect].floorheinum;
						for(i=numwalls;i<newnumwalls;i++)
						{
							wall[i].cstat = wall[suckwall].cstat;
							wall[i].shade = wall[suckwall].shade;
							wall[i].yrepeat = wall[suckwall].yrepeat;
							fixrepeats((short)i);
							wall[i].picnum = wall[suckwall].picnum;
							wall[i].overpicnum = wall[suckwall].overpicnum;
							checksectorpointer((short)i,(short)numsectors);
						}
						headspritesect[numsectors] = -1;
						numsectors++;
					}
					numwalls = newnumwalls;
					newnumwalls = -1;
					asksave = 1;
				}
				if (split == 1)
				{
						 //split sector
					startwall = sector[splitsect].wallptr;
					endwall = startwall + sector[splitsect].wallnum - 1;
					for(k=startwall;k<=endwall;k++)
						if (wall[k].x == wall[newnumwalls-1].x)
							if (wall[k].y == wall[newnumwalls-1].y)
							{
								bad = 0;
								if (loopnumofsector(splitsect,splitstartwall) != loopnumofsector(splitsect,(short)k))
									bad = 1;

								if (bad == 0)
								{
									//SPLIT IT!
									//Split splitsect given: startwall,
									//   new points: numwalls to newnumwalls-2

									splitendwall = k;
									newnumwalls--;  //first fix up the new walls
									for(i=numwalls;i<newnumwalls;i++)
									{
										wall[i].cstat = wall[startwall].cstat;
										wall[i].shade = wall[startwall].shade;
										wall[i].yrepeat = wall[startwall].yrepeat;
										fixrepeats((short)i);
										wall[i].picnum = wall[startwall].picnum;
										wall[i].overpicnum = wall[startwall].overpicnum;

										wall[i].nextwall = -1;
										wall[i].nextsector = -1;
										wall[i].point2 = i+1;
									}

									danumwalls = newnumwalls;  //where to add more walls
									m = splitendwall;          //copy rest of loop next
									while (m != splitstartwall)
									{
										memcpy(&wall[danumwalls],&wall[m],sizeof(walltype));
										wall[danumwalls].point2 = danumwalls+1;
										danumwalls++;
										m = wall[m].point2;
									}
									wall[danumwalls-1].point2 = numwalls;

										//Add other loops for 1st sector
									loopnum = loopnumofsector(splitsect,splitstartwall);
									i = loopnum;
									for(j=startwall;j<=endwall;j++)
									{
										k = loopnumofsector(splitsect,(short)j);
										if ((k != i) && (k != loopnum))
										{
											i = k;
											if (loopinside(wall[j].x,wall[j].y,numwalls) == 1)
											{
												m = j;          //copy loop
												k = danumwalls;
												do
												{
													memcpy(&wall[danumwalls],&wall[m],sizeof(walltype));
													wall[danumwalls].point2 = danumwalls+1;
													danumwalls++;
													m = wall[m].point2;
												}
												while (m != j);
												wall[danumwalls-1].point2 = k;
											}
										}
									}

									secondstartwall = danumwalls;
										//copy split points for other sector backwards
									for(j=newnumwalls;j>numwalls;j--)
									{
										memcpy(&wall[danumwalls],&wall[j],sizeof(walltype));
										wall[danumwalls].nextwall = -1;
										wall[danumwalls].nextsector = -1;
										wall[danumwalls].point2 = danumwalls+1;
										danumwalls++;
									}
									m = splitstartwall;     //copy rest of loop next
									while (m != splitendwall)
									{
										memcpy(&wall[danumwalls],&wall[m],sizeof(walltype));
										wall[danumwalls].point2 = danumwalls+1;
										danumwalls++;
										m = wall[m].point2;
									}
									wall[danumwalls-1].point2 = secondstartwall;

										//Add other loops for 2nd sector
									loopnum = loopnumofsector(splitsect,splitstartwall);
									i = loopnum;
									for(j=startwall;j<=endwall;j++)
									{
										k = loopnumofsector(splitsect,(short)j);
										if ((k != i) && (k != loopnum))
										{
											i = k;
											if (loopinside(wall[j].x,wall[j].y,secondstartwall) == 1)
											{
												m = j;          //copy loop
												k = danumwalls;
												do
												{
													memcpy(&wall[danumwalls],&wall[m],sizeof(walltype));
													wall[danumwalls].point2 = danumwalls+1;
													danumwalls++;
													m = wall[m].point2;
												}
												while (m != j);
												wall[danumwalls-1].point2 = k;
											}
										}
									}

										//fix all next pointers on old sector line
									for(j=numwalls;j<danumwalls;j++)
									{
										if (wall[j].nextwall >= 0)
										{
											wall[wall[j].nextwall].nextwall = j;
											if (j < secondstartwall)
												wall[wall[j].nextwall].nextsector = numsectors;
											else
												wall[wall[j].nextwall].nextsector = numsectors+1;
										}
									}
										//set all next pointers on split
									for(j=numwalls;j<newnumwalls;j++)
									{
										m = secondstartwall+(newnumwalls-1-j);
										wall[j].nextwall = m;
										wall[j].nextsector = numsectors+1;
										wall[m].nextwall = j;
										wall[m].nextsector = numsectors;
									}
										//copy sector attributes & fix wall pointers
									memcpy(&sector[numsectors],&sector[splitsect],sizeof(sectortype));
									memcpy(&sector[numsectors+1],&sector[splitsect],sizeof(sectortype));
									sector[numsectors].wallptr = numwalls;
									sector[numsectors].wallnum = secondstartwall-numwalls;
									sector[numsectors+1].wallptr = secondstartwall;
									sector[numsectors+1].wallnum = danumwalls-secondstartwall;

										//fix sprites
									j = headspritesect[splitsect];
									while (j != -1)
									{
										k = nextspritesect[j];
										if (loopinside(sprite[j].x,sprite[j].y,numwalls) == 1)
											changespritesect(j,numsectors);
										//else if (loopinside(sprite[j].x,sprite[j].y,secondstartwall) == 1)
										else  //Make sure no sprites get left out & deleted!
											changespritesect(j,numsectors+1);
										j = k;
									}

									numsectors+=2;

										//Back of number of walls of new sector for later
									k = danumwalls-numwalls;

										//clear out old sector's next pointers for clean deletesector
									numwalls = danumwalls;
									for(j=startwall;j<=endwall;j++)
									{
										wall[j].nextwall = -1;
										wall[j].nextsector = -1;
									}
									deletesector(splitsect);

										//Check pointers
									for(j=numwalls-k;j<numwalls;j++)
									{
										if (wall[j].nextwall >= 0)
											checksectorpointer(wall[j].nextwall,wall[j].nextsector);
										checksectorpointer((short)j,sectorofwall((short)j));
									}

										//k now safe to use as temp

									for(m=numsectors-2;m<numsectors;m++)
									{
										j = headspritesect[m];
										while (j != -1)
										{
											k = nextspritesect[j];
											setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);
											j = k;
										}
									}

									newnumwalls = -1;
									printmessage16("Sector split.");
									break;
								}
								else
								{
										//Sector split - actually loop joining

									splitendwall = k;
									newnumwalls--;  //first fix up the new walls
									for(i=numwalls;i<newnumwalls;i++)
									{
										wall[i].cstat = wall[startwall].cstat;
										wall[i].shade = wall[startwall].shade;
										wall[i].yrepeat = wall[startwall].yrepeat;
										fixrepeats((short)i);
										wall[i].picnum = wall[startwall].picnum;
										wall[i].overpicnum = wall[startwall].overpicnum;

										wall[i].nextwall = -1;
										wall[i].nextsector = -1;
										wall[i].point2 = i+1;
									}

									danumwalls = newnumwalls;  //where to add more walls
									m = splitendwall;          //copy rest of loop next
									do
									{
										memcpy(&wall[danumwalls],&wall[m],sizeof(walltype));
										wall[danumwalls].point2 = danumwalls+1;
										danumwalls++;
										m = wall[m].point2;
									} while (m != splitendwall);

									//copy split points for other sector backwards
									for(j=newnumwalls;j>numwalls;j--)
									{
										memcpy(&wall[danumwalls],&wall[j],sizeof(walltype));
										wall[danumwalls].nextwall = -1;
										wall[danumwalls].nextsector = -1;
										wall[danumwalls].point2 = danumwalls+1;
										danumwalls++;
									}

									m = splitstartwall;     //copy rest of loop next
									do
									{
										memcpy(&wall[danumwalls],&wall[m],sizeof(walltype));
										wall[danumwalls].point2 = danumwalls+1;
										danumwalls++;
										m = wall[m].point2;
									} while (m != splitstartwall);
									wall[danumwalls-1].point2 = numwalls;

										//Add other loops to sector
									loopnum = loopnumofsector(splitsect,splitstartwall);
									i = loopnum;
									for(j=startwall;j<=endwall;j++)
									{
										k = loopnumofsector(splitsect,(short)j);
										if ((k != i) && (k != loopnumofsector(splitsect,splitstartwall)) && (k != loopnumofsector(splitsect,splitendwall)))
										{
											i = k;
											m = j; k = danumwalls;     //copy loop
											do
											{
												memcpy(&wall[danumwalls],&wall[m],sizeof(walltype));
												wall[danumwalls].point2 = danumwalls+1;
												danumwalls++;
												m = wall[m].point2;
											} while (m != j);
											wall[danumwalls-1].point2 = k;
										}
									}

										//fix all next pointers on old sector line
									for(j=numwalls;j<danumwalls;j++)
									{
										if (wall[j].nextwall >= 0)
										{
											wall[wall[j].nextwall].nextwall = j;
											wall[wall[j].nextwall].nextsector = numsectors;
										}
									}

										//copy sector attributes & fix wall pointers
									memcpy(&sector[numsectors],&sector[splitsect],sizeof(sectortype));
									sector[numsectors].wallptr = numwalls;
									sector[numsectors].wallnum = danumwalls-numwalls;

										//fix sprites
									j = headspritesect[splitsect];
									while (j != -1)
									{
										k = nextspritesect[j];
										changespritesect(j,numsectors);
										j = k;
									}

									numsectors++;

										//Back of number of walls of new sector for later
									k = danumwalls-numwalls;

										//clear out old sector's next pointers for clean deletesector
									numwalls = danumwalls;
									for(j=startwall;j<=endwall;j++)
									{
										wall[j].nextwall = -1;
										wall[j].nextsector = -1;
									}
									deletesector(splitsect);

										//Check pointers
									for(j=numwalls-k;j<numwalls;j++)
									{
										if (wall[j].nextwall >= 0)
											checksectorpointer(wall[j].nextwall,wall[j].nextsector);
										checksectorpointer((short)j,numsectors-1);
									}

									newnumwalls = -1;
									printmessage16("Loops joined.");
									break;
								}
							}
				}
			}
		}

		if (keystatus[0x1c] > 0) //Left Enter
		{
			keystatus[0x1c] = 0;
			if (keystatus[0x2a]&keystatus[0x1d])
			{
				printmessage16("CHECKING ALL POINTERS!");
				for(i=0;i<numsectors;i++)
				{
					startwall = sector[i].wallptr;
					for(j=startwall;j<numwalls;j++)
						if (wall[j].point2 < startwall) startwall = wall[j].point2;
					sector[i].wallptr = startwall;
				}
				for(i=numsectors-2;i>=0;i--)
					sector[i].wallnum = sector[i+1].wallptr-sector[i].wallptr;
				sector[numsectors-1].wallnum = numwalls-sector[numsectors-1].wallptr;

				for(i=0;i<numwalls;i++)
				{
					wall[i].nextsector = -1;
					wall[i].nextwall = -1;
				}
				for(i=0;i<numsectors;i++)
				{
					startwall = sector[i].wallptr;
					endwall = startwall + sector[i].wallnum;
					for(j=startwall;j<endwall;j++)
						checksectorpointer((short)j,(short)i);
				}
				printmessage16("ALL POINTERS CHECKED!");
				asksave = 1;
			}
			else
			{
				if (linehighlight >= 0)
				{
					checksectorpointer(linehighlight,sectorofwall(linehighlight));
					printmessage16("Highlighted line pointers checked.");
					asksave = 1;
				}
			}
		}

		if ((keystatus[0x0e] > 0) && (newnumwalls >= numwalls)) //Backspace
		{
			if (newnumwalls > numwalls)
			{
				newnumwalls--;
				asksave = 1;
				keystatus[0x0e] = 0;
			}
			if (newnumwalls == numwalls)
			{
				newnumwalls = -1;
				asksave = 1;
				keystatus[0x0e] = 0;
			}
		}

		if ((keystatus[0xd3] > 0) && (keystatus[0x9d] > 0) && (numwalls >= 0))
		{                                                      //sector delete
			keystatus[0xd3] = 0;

			sucksect = -1;
			for(i=0;i<numsectors;i++)
				if (inside(mousxplc,mousyplc,i) == 1)
				{
					k = 0;
					if (highlightsectorcnt >= 0)
						for(j=0;j<highlightsectorcnt;j++)
							if (highlightsector[j] == i)
							{
								for(j=highlightsectorcnt-1;j>=0;j--)
								{
									deletesector(highlightsector[j]);
									for(k=j-1;k>=0;k--)
										if (highlightsector[k] >= highlightsector[j])
											highlightsector[k]--;
								}
								printmessage16("Highlighted sectors deleted.");
								newnumwalls = -1;
								k = 1;
								highlightsectorcnt = -1;
								break;
							}
					if (k == 0)
					{
						deletesector((short)i);
						highlightsectorcnt = -1;
						printmessage16("Sector deleted.");
					}
					newnumwalls = -1;
					asksave = 1;
					break;
				}
		}

		if ((keystatus[0xd3] > 0) && (pointhighlight >= 0))
		{
			if ((pointhighlight&0xc000) == 16384)   //Sprite Delete
			{
				deletesprite(pointhighlight&16383);
				printmessage16("Sprite deleted.");
				updatenumsprites();
				asksave = 1;
			}
			keystatus[0xd3] = 0;
		}

		if (keystatus[0xd2] > 0)  //InsertPoint
		{
			if (highlightsectorcnt >= 0)
			{
				newnumsectors = numsectors;
				newnumwalls = numwalls;
				for(i=0;i<highlightsectorcnt;i++)
				{
					copysector(highlightsector[i],newnumsectors,newnumwalls,1);
					newnumsectors++;
					newnumwalls += sector[highlightsector[i]].wallnum;
				}

				for(i=0;i<highlightsectorcnt;i++)
				{
					startwall = sector[highlightsector[i]].wallptr;
					endwall = startwall+sector[highlightsector[i]].wallnum-1;
					for(j=startwall;j<=endwall;j++)
					{
						if (wall[j].nextwall >= 0)
							checksectorpointer(wall[j].nextwall,wall[j].nextsector);
						checksectorpointer((short)j,highlightsector[i]);
					}
					highlightsector[i] = numsectors+i;
				}
				numsectors = newnumsectors;
				numwalls = newnumwalls;

				newnumwalls = -1;
				newnumsectors = -1;

				updatenumsprites();
				printmessage16("Sectors duplicated and stamped.");
				asksave = 1;
			}
			else if (highlightcnt >= 0)
			{
				for(i=0;i<highlightcnt;i++)
					if ((highlight[i]&0xc000) == 16384)
					{
							//duplicate sprite
						k = (highlight[i]&16383);
						j = insertsprite(sprite[k].sectnum,sprite[k].statnum);
						memcpy(&sprite[j],&sprite[k],sizeof(spritetype));
						sprite[j].sectnum = sprite[k].sectnum;   //Don't let memcpy overwrite sector!
						setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);
					}
				updatenumsprites();
				printmessage16("Sprites duplicated and stamped.");
				asksave = 1;
			}
			else if (linehighlight >= 0)
			{
				getclosestpointonwall(mousxplc,mousyplc,(long)linehighlight,&dax,&day);
				adjustmark(&dax,&day,newnumwalls);
				insertpoint(linehighlight,dax,day);
				printmessage16("Point inserted.");

				j = 0;
					//Check to see if point was inserted over another point
				for(i=numwalls-1;i>=0;i--)     //delete points
					if (wall[i].x == wall[wall[i].point2].x)
						if (wall[i].y == wall[wall[i].point2].y)
						{
							deletepoint((short)i);
							j++;
						}
				for(i=0;i<numwalls;i++)        //make new red lines?
				{
					if ((wall[i].x == dax) && (wall[i].y == day))
					{
						checksectorpointer((short)i,sectorofwall((short)i));
						fixrepeats((short)i);
					}
					else if ((wall[wall[i].point2].x == dax) && (wall[wall[i].point2].y == day))
					{
						checksectorpointer((short)i,sectorofwall((short)i));
						fixrepeats((short)i);
					}
				}
				//if (j != 0)
				//{
				//   dax = ((wall[linehighlight].x + wall[wall[linehighlight].point2].x)>>1);
				//   day = ((wall[linehighlight].y + wall[wall[linehighlight].point2].y)>>1);
				//   if ((dax != wall[linehighlight].x) || (day != wall[linehighlight].y))
				//      if ((dax != wall[wall[linehighlight].point2].x) || (day != wall[wall[linehighlight].point2].y))
				//      {
				//         insertpoint(linehighlight,dax,day);
				//         printmessage16("Point inserted at midpoint.");
				//      }
				//}

				asksave = 1;
			}
			keystatus[0xd2] = 0;
		}

		ExtCheckKeys();

		j = 0;
		for(i=22-1;i>=0;i--) updatecrc16(j,kensig[i]);
		if ((j&0xffff) != 0xebf)
		{
			setvmode(0x3);
			printf("Don't screw with my name.\n");
			exit(0);
		}
		printext16(9L,9L,4,-1,kensig,0);
		printext16(8L,8L,12,-1,kensig,0);

		nextpage();
		synctics = totalclock-lockclock;
		lockclock += synctics;

		if (keystatus[buildkeys[14]] > 0)
		{
			updatesector(posx,posy,&cursectnum);
			if (cursectnum >= 0)
				keystatus[buildkeys[14]] = 2;
			else
				printmessage16("Arrow must be inside a sector before entering 3D mode.");
		}
		if (keystatus[1] > 0)
		{
			keystatus[1] = 0;
			printmessage16("(N)ew, (L)oad, (S)ave, save (A)s, (Q)uit");
			bad = 1;
			while (bad == 1)
			{
				if (keystatus[1] > 0)
				{
					keystatus[1] = 0;
					bad = 0;
					printmessage16("");
				}
				if (keystatus[0x31] > 0)  //N
				{
					bad = 0;
					keystatus[0x31] = 0;
					printmessage16("Are you sure you want to start a new board?");
					while ((keystatus[1]|keystatus[0x1c]|keystatus[0x39]|keystatus[0x31]) == 0)
					{
						if (keystatus[0x15] != 0)
						{
							keystatus[0x15] = 0;

							highlightsectorcnt = -1;
							highlightcnt = -1;

							for(i=0;i<(MAXWALLS>>3);i++)   //Clear all highlights
								show2dwall[i] = 0;
							for(i=0;i<(MAXSPRITES>>3);i++)
								show2dsprite[i] = 0;

							for(i=0;i<MAXSECTORS;i++) sector[i].extra = -1;
							for(i=0;i<MAXWALLS;i++) wall[i].extra = -1;
							for(i=0;i<MAXSPRITES;i++) sprite[i].extra = -1;

							sectorhighlightstat = -1;
							newnumwalls = -1;
							joinsector[0] = -1;
							circlewall = -1;
							circlepoints = 7;

							posx = 32768;          //new board!
							posy = 32768;
							posz = 0;
							ang = 1536;
							numsectors = 0;
							numwalls = 0;
							cursectnum = -1;
							initspritelists();
							strcpy(&boardfilename,"newboard.map");
							break;
						}
					}
					printmessage16("");
				}
				if (keystatus[0x26] > 0)  //L
				{
					bad = 0;
					keystatus[0x26] = 0;
					i = menuselect();
					if (i < 0)
					{
						if (i == -2)
							printmessage16("No .MAP files found.");
					}
					else
					{
						strcpy(&boardfilename,menuname[menuhighlight]);

						if (highlightsectorcnt >= 0)
						{
							j = 0; k = 0;
							for(i=0;i<highlightsectorcnt;i++)
							{
								j += sector[highlightsector[i]].wallnum;

								m = headspritesect[highlightsector[i]];
								while (m != -1)
								{
									k++;
									m = nextspritesect[m];
								}
							}

							updatenumsprites();
							if ((numsectors+highlightsectorcnt > MAXSECTORS) || (numwalls+j > MAXWALLS) || (numsprites+k > MAXSPRITES))
							{
								highlightsectorcnt = -1;
							}
							else
							{
									//Put sectors&walls to end of lists
								j = MAXWALLS;
								for(i=0;i<highlightsectorcnt;i++)
								{
									j -= sector[highlightsector[i]].wallnum;
									copysector(highlightsector[i],(short)(MAXSECTORS-highlightsectorcnt+i),(short)j,0);
								}

									//Put sprites to end of list
									//DONT USE m BETWEEN HERE AND SPRITE RE-ATTACHING!
								m = MAXSPRITES;
								for(i=MAXSPRITES-1;i>=0;i--)
									if (sprite[i].statnum < MAXSTATUS)
									{
										k = sprite[i].sectnum;
										for(j=0;j<highlightsectorcnt;j++)
											if (highlightsector[j] == k)
											{
												m--;
												if (i != m)
													memcpy(&sprite[m],&sprite[i],sizeof(spritetype));

													//HACK - THESE 2 buffers back up .sectnum and .statnum
													//for initspritelists() inside the loadboard call
												tsprite[m].picnum = MAXSECTORS-highlightsectorcnt+j;
												tsprite[m].owner = sprite[i].statnum;

												break;
											}
									}
							}
						}

						highlightcnt = -1;
						sectorhighlightstat = -1;
						newnumwalls = -1;
						joinsector[0] = -1;
						circlewall = -1;
						circlepoints = 7;

						for(i=0;i<MAXSECTORS;i++) sector[i].extra = -1;
						for(i=0;i<MAXWALLS;i++) wall[i].extra = -1;
						for(i=0;i<MAXSPRITES;i++) sprite[i].extra = -1;

						if (loadboard(boardfilename,&posx,&posy,&posz,&ang,&cursectnum) == -1)
						{
							printmessage16("Invalid map format.");
						}
						else
						{
							ExtLoadMap(boardfilename);

							if (highlightsectorcnt >= 0)
							{
								if ((numsectors+highlightsectorcnt > MAXSECTORS) || (sector[MAXSECTORS-highlightsectorcnt].wallptr < numwalls))
								{
									highlightsectorcnt = -1;
								}
								else
								{
										//Re-attach sectors&walls
									for(i=0;i<highlightsectorcnt;i++)
									{
										copysector((short)(MAXSECTORS-highlightsectorcnt+i),numsectors,numwalls,0);
										highlightsector[i] = numsectors;
										numwalls += sector[numsectors].wallnum;
										numsectors++;
									}
										//Re-attach sprites
									while (m < MAXSPRITES)
									{
										//HACK - THESE 2 buffers back up .sectnum and .statnum
										//for initspritelists() inside the loadboard call
										//tsprite[m].picnum = sprite[i].sectnum;
										//tsprite[m].owner = sprite[i].statnum;

										j = insertsprite(tsprite[m].picnum+(numsectors-MAXSECTORS),tsprite[m].owner);
										memcpy(&sprite[j],&sprite[m],sizeof(spritetype));
										sprite[j].sectnum = tsprite[m].picnum+(numsectors-MAXSECTORS);
										sprite[j].statnum = tsprite[m].owner;
										m++;
									}

									for(i=0;i<highlightsectorcnt;i++)
									{
										startwall = sector[highlightsector[i]].wallptr;
										endwall = startwall+sector[highlightsector[i]].wallnum-1;
										for(j=startwall;j<=endwall;j++)
										{
											if (wall[j].nextwall >= 0)
												checksectorpointer(wall[j].nextwall,wall[j].nextsector);
											checksectorpointer((short)j,highlightsector[i]);
										}
									}

								}
							}

							printmessage16("Map loaded successfully.");
						}
						updatenumsprites();
						startposx = posx;      //this is same
						startposy = posy;
						startposz = posz;
						startang = ang;
						startsectnum = cursectnum;
					}
					chdir(curpath);
					keystatus[0x1c] = 0;
				}
				if (keystatus[0x1e] > 0)  //A
				{
					bad = 0;
					keystatus[0x1e] = 0;

					strcpy(oboardfilename,boardfilename);

					i = 0;
					while ((boardfilename[i] != 0) && (i < 13))
						i++;
					if (boardfilename[i-4] == '.')
						i -= 4;
					boardfilename[i] = 0;

					while (bad == 0)
					{
						sprintf(buffer,"Save as: %s",boardfilename);
						printmessage16(buffer);

						if (keystatus[1] > 0) bad = 1;
						if (keystatus[0x1c] > 0) bad = 2;

						if (i > 0)
						{
							if (keystatus[0xe] > 0)
							{
								keystatus[0xe] = 0;
								i--;
								boardfilename[i] = 0;
							}
						}
						if (i < 8)
						{
							if ((keystatus[0x2a]|keystatus[0x36]) > 0)
							{
								for(j=0;j<128;j++)
									if (scantoascwithshift[j] > 0)
										if (keystatus[j] > 0)
										{
											keystatus[j] = 0;
											boardfilename[i++] = scantoascwithshift[j];
											boardfilename[i] = 0;
										}
							}
							else
							{
								for(j=0;j<128;j++)
									if (scantoasc[j] > 0)
										if (keystatus[j] > 0)
										{
											keystatus[j] = 0;
											boardfilename[i++] = scantoasc[j];
											boardfilename[i] = 0;
										}
							}
						}
					}
					if (bad == 1)
					{
						strcpy(boardfilename,oboardfilename);
						keystatus[1] = 0;
						printmessage16("Operation cancelled");
					}
					if (bad == 2)
					{
						keystatus[0x1c] = 0;

						boardfilename[i] = '.';
						boardfilename[i+1] = 'm';
						boardfilename[i+2] = 'a';
						boardfilename[i+3] = 'p';
						boardfilename[i+4] = 0;

						sprintf(buffer,"Saving to %s...",boardfilename);
						printmessage16(buffer);

						fixspritesectors();   //Do this before saving!
						updatesector(startposx,startposy,&startsectnum);
						saveboard(boardfilename,&startposx,&startposy,&startposz,&startang,&startsectnum);
						ExtSaveMap(boardfilename);
						printmessage16("Board saved.");
					}
					bad = 0;
				}
				if (keystatus[0x1f] > 0)  //S
				{
					bad = 0;
					keystatus[0x1f] = 0;
					printmessage16("Saving board...");
					fixspritesectors();   //Do this before saving!
					updatesector(startposx,startposy,&startsectnum);
					saveboard(boardfilename,&startposx,&startposy,&startposz,&startang,&startsectnum);
					ExtSaveMap(boardfilename);
					printmessage16("Board saved.");
				}
				if (keystatus[0x10] > 0)  //Q
				{
					bad = 0;
					keystatus[0x10] = 0;
					printmessage16("Are you sure you want to quit?");
					while ((keystatus[1]|keystatus[0x1c]|keystatus[0x39]|keystatus[0x31]) == 0)
					{
						if (keystatus[0x15] != 0)
						{
							keystatus[0x15] = 0;             //QUIT!

							printmessage16("Save changes?");
							while ((keystatus[1]|keystatus[0x1c]|keystatus[0x39]|keystatus[0x31]) == 0)
							{
								if (keystatus[0x15] > 0)
								{
									keystatus[0x15] = 0;

									fixspritesectors();   //Do this before saving!
									updatesector(startposx,startposy,&startsectnum);
									saveboard(boardfilename,&startposx,&startposy,&startposz,&startang,&startsectnum);
									ExtSaveMap(boardfilename);
									break;
								}
							}
							uninittimer();
							uninitkeys();
							ExtUnInit();
							uninitengine();
							setvmode(0x3);
							printf("Memory status: %ld(%ld) bytes\n",cachesize,artsize);
							printf("%s\n",kensig);
							exit(0);
						}
					}
					printmessage16("");
				}
			}
		}
	}

	for(i=0;i<highlightsectorcnt;i++)
	{
		startwall = sector[highlightsector[i]].wallptr;
		endwall = startwall+sector[highlightsector[i]].wallnum-1;
		for(j=startwall;j<=endwall;j++)
		{
			if (wall[j].nextwall >= 0)
				checksectorpointer(wall[j].nextwall,wall[j].nextsector);
			checksectorpointer((short)j,highlightsector[i]);
		}
	}

	fixspritesectors();

	if (setgamemode(vidoption,xdim,ydim) < 0)
	{
		ExtUnInit();
		uninitkeys();
		uninittimer();
		printf("%ld * %ld not supported in this graphics mode\n",xdim,ydim);
		exit(0);
	}

	posz = oposz;
	searchx = scale(searchx,xdim,640);
	searchy = scale(searchy,ydim,480);
}

getpoint(long searchxe, long searchye, long *x, long *y)
{
	if (posx <= -131072) posx = -131072;
	if (posx >= 131072) posx = 131072;
	if (posy <= -131072) posy = -131072;
	if (posy >= 131072) posy = 131072;

	*x = posx + divscale14(searchxe-320,zoom);
	*y = posy + divscale14(searchye-200,zoom);

	if (*x <= -131072) *x = -131072;
	if (*x >= 131072) *x = 131072;
	if (*y <= -131072) *y = -131072;
	if (*y >= 131072) *y = 131072;
}

getlinehighlight(long xplc, long yplc)
{
	long i, dst, dist, closest, x1, y1, x2, y2, nx, ny;

	if (numwalls == 0)
		return(-1);
	dist = 0x7fffffff;
	closest = numwalls-1;
	for(i=0;i<numwalls;i++)
	{
		getclosestpointonwall(xplc,yplc,i,&nx,&ny);
		dst = klabs(xplc-nx)+klabs(yplc-ny);
		if (dst <= dist)
			dist = dst, closest = i;
	}

	if (wall[closest].nextwall >= 0)
	{    //if red line, allow highlighting of both sides
		x1 = wall[closest].x;
		y1 = wall[closest].y;
		x2 = wall[wall[closest].point2].x;
		y2 = wall[wall[closest].point2].y;
		if (dmulscale32(xplc-x1,y2-y1,-(x2-x1),yplc-y1) >= 0)
			closest = wall[closest].nextwall;
	}

	return(closest);
}

getpointhighlight(long xplc, long yplc)
{
	long i, dst, dist, closest;

	if (numwalls == 0)
		return(-1);

	dist = 0;
	if (grid > 0)
		dist = 1024;

	closest = -1;
	for(i=0;i<numwalls;i++)
	{
		dst = klabs(xplc-wall[i].x) + klabs(yplc-wall[i].y);
		if (dst <= dist)
			dist = dst, closest = i;
	}
	for(i=0;i<MAXSPRITES;i++)
		if (sprite[i].statnum < MAXSTATUS)
		{
			dst = klabs(xplc-sprite[i].x) + klabs(yplc-sprite[i].y);
			if (dst <= dist)
				dist = dst, closest = i+16384;
		}
	return(closest);
}

adjustmark(long *xplc, long *yplc, short danumwalls)
{
	long i, dst, dist, dax, day, pointlockdist;

	if (danumwalls < 0)
		danumwalls = numwalls;

	pointlockdist = 0;
	if ((grid > 0) && (gridlock > 0))
		pointlockdist = (128>>grid);

	dist = pointlockdist;
	dax = *xplc;
	day = *yplc;
	for(i=0;i<danumwalls;i++)
	{
		dst = klabs((*xplc)-wall[i].x) + klabs((*yplc)-wall[i].y);
		if (dst < dist)
		{
			dist = dst;
			dax = wall[i].x;
			day = wall[i].y;
		}
	}
	if (dist == pointlockdist)
		if ((gridlock > 0) && (grid > 0))
		{
			dax = ((dax+(1024>>grid))&(0xffffffff<<(11-grid)));
			day = ((day+(1024>>grid))&(0xffffffff<<(11-grid)));
		}

	*xplc = dax;
	*yplc = day;
	return(0);
}

checkautoinsert(long dax, long day, short danumwalls)
{
	long i, x1, y1, x2, y2;

	if (danumwalls < 0)
		danumwalls = numwalls;
	for(i=0;i<danumwalls;i++)       // Check if a point should be inserted
	{
		x1 = wall[i].x;
		y1 = wall[i].y;
		x2 = wall[wall[i].point2].x;
		y2 = wall[wall[i].point2].y;

		if ((x1 != dax) || (y1 != day))
			if ((x2 != dax) || (y2 != day))
				if (((x1 <= dax) && (dax <= x2)) || ((x2 <= dax) && (dax <= x1)))
					if (((y1 <= day) && (day <= y2)) || ((y2 <= day) && (day <= y1)))
						if ((dax-x1)*(y2-y1) == (day-y1)*(x2-x1))
							return(1);          //insertpoint((short)i,dax,day);
	}
	return(0);
}

clockdir(short wallstart)   //Returns: 0 is CW, 1 is CCW
{
	short i, themin;
	long minx, templong, x0, x1, x2, y0, y1, y2;

	minx = 0x7fffffff;
	themin = -1;
	i = wallstart-1;
	do
	{
		i++;
		if (wall[wall[i].point2].x < minx)
		{
			minx = wall[wall[i].point2].x;
			themin = i;
		}
	}
	while ((wall[i].point2 != wallstart) && (i < MAXWALLS));

	x0 = wall[themin].x;
	y0 = wall[themin].y;
	x1 = wall[wall[themin].point2].x;
	y1 = wall[wall[themin].point2].y;
	x2 = wall[wall[wall[themin].point2].point2].x;
	y2 = wall[wall[wall[themin].point2].point2].y;

	if ((y1 >= y2) && (y1 <= y0)) return(0);
	if ((y1 >= y0) && (y1 <= y2)) return(1);

	templong = (x0-x1)*(y2-y1) - (x2-x1)*(y0-y1);
	if (templong < 0)
		return(0);
	else
		return(1);
}

flipwalls(short numwalls, short newnumwalls)
{
	long i, j, nume, templong;

	nume = newnumwalls-numwalls;

	for(i=numwalls;i<numwalls+(nume>>1);i++)
	{
		j = numwalls+newnumwalls-i-1;
		templong = wall[i].x; wall[i].x = wall[j].x; wall[j].x = templong;
		templong = wall[i].y; wall[i].y = wall[j].y; wall[j].y = templong;
	}
}

insertpoint(short linehighlight, long dax, long day)
{
	short sucksect;
	long i, j, k;

	j = linehighlight;
	sucksect = sectorofwall((short)j);

	sector[sucksect].wallnum++;
	for(i=sucksect+1;i<numsectors;i++)
		sector[i].wallptr++;

	movewalls((long)j+1,+1L);
	memcpy(&wall[j+1],&wall[j],sizeof(walltype));

	wall[j].point2 = j+1;
	wall[j+1].x = dax;
	wall[j+1].y = day;
	fixrepeats((short)j);
	fixrepeats((short)j+1);

	if (wall[j].nextwall >= 0)
	{
		k = wall[j].nextwall;

		sucksect = sectorofwall((short)k);

		sector[sucksect].wallnum++;
		for(i=sucksect+1;i<numsectors;i++)
			sector[i].wallptr++;

		movewalls((long)k+1,+1L);
		memcpy(&wall[k+1],&wall[k],sizeof(walltype));

		wall[k].point2 = k+1;
		wall[k+1].x = dax;
		wall[k+1].y = day;
		fixrepeats((short)k);
		fixrepeats((short)k+1);

		j = wall[k].nextwall;
		wall[j].nextwall = k+1;
		wall[j+1].nextwall = k;
		wall[k].nextwall = j+1;
		wall[k+1].nextwall = j;
	}
}

deletepoint(point)
{
	long i, j, k, sucksect;

	sucksect = sectorofwall(point);

	sector[sucksect].wallnum--;
	for(i=sucksect+1;i<numsectors;i++)
		sector[i].wallptr--;

	j = lastwall(point);
	k = wall[point].point2;
	wall[j].point2 = k;

	if (wall[j].nextwall >= 0)
	{
		wall[wall[j].nextwall].nextwall = -1;
		wall[wall[j].nextwall].nextsector = -1;
	}
	if (wall[point].nextwall >= 0)
	{
		wall[wall[point].nextwall].nextwall = -1;
		wall[wall[point].nextwall].nextsector = -1;
	}
	movewalls((long)point,-1L);

	checksectorpointer((short)j,(short)sucksect);
}

deletesector(short sucksect)
{
	long i, j, k, nextk, startwall, endwall;

	while (headspritesect[sucksect] >= 0)
		deletesprite(headspritesect[sucksect]);
	updatenumsprites();

	startwall = sector[sucksect].wallptr;
	endwall = startwall + sector[sucksect].wallnum - 1;
	j = sector[sucksect].wallnum;

	for(i=sucksect;i<numsectors-1;i++)
	{
		k = headspritesect[i+1];
		while (k != -1)
		{
			nextk = nextspritesect[k];
			changespritesect((short)k,(short)i);
			k = nextk;
		}

		memcpy(&sector[i],&sector[i+1],sizeof(sectortype));
		sector[i].wallptr -= j;
	}
	numsectors--;

	j = endwall-startwall+1;
	for (i=startwall;i<=endwall;i++)
		if (wall[i].nextwall != -1)
		{
			wall[wall[i].nextwall].nextwall = -1;
			wall[wall[i].nextwall].nextsector = -1;
		}
	movewalls(startwall,-j);
	for(i=0;i<numwalls;i++)
		if (wall[i].nextwall >= startwall)
			wall[i].nextsector--;
	return(0);
}

fixspritesectors()
{
	long i, j, dax, day, daz;

	for(i=numsectors-1;i>=0;i--)
		if ((sector[i].wallnum <= 0) || (sector[i].wallptr >= numwalls))
			deletesector((short)i);

	for(i=0;i<MAXSPRITES;i++)
		if (sprite[i].statnum < MAXSTATUS)
		{
			dax = sprite[i].x;
			day = sprite[i].y;
			if (inside(dax,day,sprite[i].sectnum) != 1)
			{
				daz = ((tilesizy[sprite[i].picnum]*sprite[i].yrepeat)<<2);

				for(j=0;j<numsectors;j++)
					if (inside(dax,day,(short)j) == 1)
						if (sprite[i].z >= getceilzofslope(j,dax,day))
							if (sprite[i].z-daz <= getflorzofslope(j,dax,day))
							{
								changespritesect((short)i,(short)j);
								break;
							}
			}
		}
}

movewalls(long start, long offs)
{
	long i;

	if (offs < 0)  //Delete
	{
		for(i=start;i<numwalls+offs;i++)
			memcpy(&wall[i],&wall[i-offs],sizeof(walltype));
	}
	else if (offs > 0)  //Insert
	{
		for(i=numwalls+offs-1;i>=start+offs;i--)
			memcpy(&wall[i],&wall[i-offs],sizeof(walltype));
	}
	numwalls += offs;
	for(i=0;i<numwalls;i++)
	{
		if (wall[i].nextwall >= start) wall[i].nextwall += offs;
		if (wall[i].point2 >= start) wall[i].point2 += offs;
	}
	return(0);
}

checksectorpointer(short i, short sectnum)
{
	long j, k, startwall, endwall, x1, y1, x2, y2;

	x1 = wall[i].x;
	y1 = wall[i].y;
	x2 = wall[wall[i].point2].x;
	y2 = wall[wall[i].point2].y;

	if (wall[i].nextwall >= 0)          //Check for early exit
	{
		k = wall[i].nextwall;
		if ((wall[k].x == x2) && (wall[k].y == y2))
			if ((wall[wall[k].point2].x == x1) && (wall[wall[k].point2].y == y1))
				return(0);
	}

	wall[i].nextsector = -1;
	wall[i].nextwall = -1;
	for(j=0;j<numsectors;j++)
	{
		startwall = sector[j].wallptr;
		endwall = startwall + sector[j].wallnum - 1;
		for(k=startwall;k<=endwall;k++)
		{
			if ((wall[k].x == x2) && (wall[k].y == y2))
				if ((wall[wall[k].point2].x == x1) && (wall[wall[k].point2].y == y1))
					if (j != sectnum)
					{
						wall[i].nextsector = j;
						wall[i].nextwall = k;
						wall[k].nextsector = sectnum;
						wall[k].nextwall = i;
					}
		}
	}
	return(0);
}

fixrepeats(short i)
{
	long dax, day, dist;

	dax = wall[wall[i].point2].x-wall[i].x;
	day = wall[wall[i].point2].y-wall[i].y;
	dist = ksqrt(dax*dax+day*day);
	dax = wall[i].xrepeat; day = wall[i].yrepeat;
	wall[i].xrepeat = (char)min(max(mulscale10(dist,day),1),255);
}

clearmidstatbar16()
{
	long opageoffset;

	opageoffset = pageoffset;    //clear bottom part of status bar
	pageoffset = 0;
	ydim16 = 144;
	fillscreen16((640L*25L)>>3,8L,640L*(143L-(25<<1)));
	drawline16(0,0,0,143,7);
	drawline16(639,0,639,143,7);
	pageoffset = opageoffset;
	ydim16 = 336;
}

loopinside(long x, long y, short startwall)
{
	long x1, y1, x2, y2, templong;
	short i, cnt;

	cnt = clockdir(startwall);
	i = startwall;
	do
	{
		x1 = wall[i].x; x2 = wall[wall[i].point2].x;
		if ((x1 >= x) || (x2 >= x))
		{
			y1 = wall[i].y; y2 = wall[wall[i].point2].y;
			if (y1 > y2)
			{
				templong = x1, x1 = x2, x2 = templong;
				templong = y1, y1 = y2, y2 = templong;
			}
			if ((y1 <= y) && (y2 > y))
				if (x1*(y-y2)+x2*(y1-y) <= x*(y1-y2))
					cnt ^= 1;
		}
		i = wall[i].point2;
	}
	while (i != startwall);
	return(cnt);
}

numloopsofsector(short sectnum)
{
	long i, numloops, startwall, endwall;

	numloops = 0;
	startwall = sector[sectnum].wallptr;
	endwall = startwall + sector[sectnum].wallnum;
	for(i=startwall;i<endwall;i++)
		if (wall[i].point2 < i) numloops++;
	return(numloops);
}

getnumber16(char namestart[80], short num, long maxnumber)
{
	char buffer[80];
	long j, k, n, danum, oldnum;

	danum = (long)num;
	oldnum = danum;
	while ((keystatus[0x1c] != 2) && (keystatus[0x1] == 0))
	{
		sprintf(&buffer,"%s%ld_ ",namestart,danum);
		printmessage16(buffer);

		for(j=2;j<=11;j++)
			if (keystatus[j] > 0)
			{
				keystatus[j] = 0;
				k = j-1;
				if (k == 10) k = 0;
				n = (danum*10)+k;
				if (n < maxnumber) danum = n;
			}
		if (keystatus[0xe] > 0)    // backspace
		{
			danum /= 10;
			keystatus[0xe] = 0;
		}
		if (keystatus[0x1c] == 1)
		{
			oldnum = danum;
			keystatus[0x1c] = 2;
			asksave = 1;
		}
	}
	keystatus[0x1c] = 0;
	keystatus[0x1] = 0;
	return((short)oldnum);
}

getnumber256(char namestart[80], short num, long maxnumber)
{
	char buffer[80];
	long j, k, n, danum, oldnum;

	danum = (long)num;
	oldnum = danum;
	while ((keystatus[0x1c] != 2) && (keystatus[0x1] == 0))
	{
		drawrooms(posx,posy,posz,ang,horiz,cursectnum);
		ExtAnalyzeSprites();
		drawmasks();

		sprintf(&buffer,"%s%ld_ ",namestart,danum);
		printmessage256(buffer);
		nextpage();

		for(j=2;j<=11;j++)
			if (keystatus[j] > 0)
			{
				keystatus[j] = 0;
				k = j-1;
				if (k == 10) k = 0;
				n = (danum*10)+k;
				if (n < maxnumber) danum = n;
			}
		if (keystatus[0xe] > 0)    // backspace
		{
			danum /= 10;
			keystatus[0xe] = 0;
		}
		if (keystatus[0x1c] == 1)
		{
			oldnum = danum;
			keystatus[0x1c] = 2;
			asksave = 1;
		}
	}
	keystatus[0x1c] = 0;
	keystatus[0x1] = 0;

	lockclock = totalclock;  //Reset timing

	return((short)oldnum);
}

initmenupaths(char *filename)
{
	long i;

	strcpy(&curpath,filename);
	i = 0;
	while ((i < 80) && (curpath[i] != 0)) i++;
	while ((i > 0) && (curpath[i] != 92)) i--;
	curpath[i] = 0;
	strcpy(&menupath,curpath);
}

getfilenames(char kind[6])
{
	short type;
	struct find_t fileinfo;

	if (strcmp(kind,"SUBD") == 0)
	{
		strcpy(kind,"*.*");
		if (_dos_findfirst(kind,_A_SUBDIR,&fileinfo) != 0)
			return(-1);
		type = 1;
	}
	else
	{
		if (_dos_findfirst(kind,_A_NORMAL,&fileinfo) != 0)
			return(-1);
		type = 0;
	}
	do
	{
		if ((type == 0) || ((fileinfo.attrib&16) > 0))
			if ((fileinfo.name[0] != '.') || (fileinfo.name[1] != 0))
			{
				strcpy(menuname[menunamecnt],fileinfo.name);
				menuname[menunamecnt][16] = type;
				menunamecnt++;
			}
	}
	while (_dos_findnext(&fileinfo) == 0);

	return(0);
}

sortfilenames()
{
	char sortbuffer[17];
	long i, j, k;

	for(i=1;i<menunamecnt;i++)
		for(j=0;j<i;j++)
		{
			 k = 0;
			 while ((menuname[i][k] == menuname[j][k]) && (menuname[i][k] != 0) && (menuname[j][k] != 0))
				 k++;
			if (menuname[i][k] < menuname[j][k])
			{
				memcpy(&sortbuffer[0],&menuname[i][0],sizeof(menuname[0]));
				memcpy(&menuname[i][0],&menuname[j][0],sizeof(menuname[0]));
				memcpy(&menuname[j][0],&sortbuffer[0],sizeof(menuname[0]));
			}
		}
}

menuselect()
{
	long newhighlight, i, j, topplc;
	char ch, buffer[42];

	chdir(menupath);
	menunamecnt = 0;
	getfilenames("SUBD");
	getfilenames("*.MAP");
	sortfilenames();
	if (menunamecnt == 0)
		return(-2);

	printmessage16("Select .MAP file with arrows&enter.");
	fillscreen16((399360-pageoffset)>>3,0L,640L*336L);
	if (menuhighlight < 0) menuhighlight = 0;
	if (menuhighlight >= menunamecnt) menuhighlight = menunamecnt-1;

	newhighlight = menuhighlight;
	do
	{
		if (menunamecnt <= 36)
		{
			topplc = 0;
		}
		else
		{
			topplc = newhighlight-11;
			if (topplc < 0) topplc = 0;
			if (topplc > menunamecnt-37) topplc = menunamecnt-37;
		}

		for(i=0;i<menunamecnt;i++)
		{
			for(j=0;j<38;j++)
				buffer[j] = 32;
			buffer[38] = 0;
			if (i == newhighlight)
			{
				buffer[0] = '-';
				buffer[1] = '-';
				buffer[2] = '>';
			}
			j = 0;
			while (menuname[i][j] != 0)
			{
				buffer[j+4] = menuname[i][j];
				j++;
			}
			if ((i-topplc >= 0) && (i-topplc <= 36))
			{
				if (menuname[i][16] == 1)
				{
					printext16(0L,((i-topplc)<<3)+16L+((399360-pageoffset)/640),4,0,buffer,0);
				}
				else
				{
					printext16(0L,((i-topplc)<<3)+16L+((399360-pageoffset)/640),7,0,buffer,0);
				}
			}
		}

		keystatus[0xcb] = 0;
		keystatus[0xcd] = 0;
		keystatus[0xc8] = 0;
		keystatus[0xd0] = 0;
		keystatus[0x1c] = 0;
		keystatus[1] = 0;
		ch = 0;                      //Interesting fakery of ch = getch()
		while (ch == 0)
		{
			if (keystatus[0xcb] > 0) ch = 75;
			if (keystatus[0xcd] > 0) ch = 77;
			if (keystatus[0xc8] > 0) ch = 72;
			if (keystatus[0xd0] > 0) ch = 80;
			if (keystatus[0x1c] > 0) ch = 13;
			if (keystatus[1] > 0) ch = 27;
		}

		if ((ch == 75) || (ch == 72))
		{
			newhighlight--;
			if (newhighlight < 0)
				newhighlight = menunamecnt-1;
		}
		if ((ch == 77) || (ch == 80))
		{
			newhighlight++;
			if (newhighlight >= menunamecnt)
				newhighlight = 0;
		}
		if ((ch == 13) && (menuname[newhighlight][16] == 1))
		{
			if ((menuname[newhighlight][0] == '.') && (menuname[newhighlight][1] == '.'))
			{
				i = 0;
				while ((i < 80) && (menupath[i] != 0)) i++;
				while ((i > 0) && (menupath[i] != 92)) i--;
				menupath[i] = 0;
			}
			else
			{
				strcat(&menupath,"\\");
				strcat(&menupath,menuname[newhighlight]);
			}
			chdir(menuname[newhighlight]);
			menunamecnt = 0;
			getfilenames("SUBD");
			getfilenames("*.MAP");
			sortfilenames();
			newhighlight = 0;
			ch = 0;
			fillscreen16((399360-pageoffset)>>3,0L,640L*336L);
		}
	}
	while ((ch != 13) && (ch != 27));
	if (ch == 13)
	{
		menuhighlight = newhighlight;
		return(newhighlight);
	}
	return(-1);
}

fillsector(short sectnum, char fillcolor)
{
	long x1, x2, y1, y2, sy, y, templong;
	long lborder, rborder, uborder, dborder, miny, maxy, dax;
	short z, zz, startwall, endwall, fillcnt;

	lborder = 0; rborder = 640;
	uborder = 0; dborder = ydim16;

	if (sectnum == -1)
		return(0);
	miny = dborder-1;
	maxy = uborder;
	startwall = sector[sectnum].wallptr;
	endwall = startwall + sector[sectnum].wallnum - 1;
	for(z=startwall;z<=endwall;z++)
	{
		y1 = (((wall[z].y-posy)*zoom)>>14)+200;
		y2 = (((wall[wall[z].point2].y-posy)*zoom)>>14)+200;
		if (y1 < miny) miny = y1;
		if (y2 < miny) miny = y2;
		if (y1 > maxy) maxy = y1;
		if (y2 > maxy) maxy = y2;
	}
	if (miny < uborder) miny = uborder;
	if (maxy >= dborder) maxy = dborder-1;

	for(sy=miny+(numframes%3);sy<=maxy;sy+=3)
	{
		y = posy+(((sy-200)<<14)/zoom);

		fillist[0] = lborder; fillcnt = 1;
		for(z=startwall;z<=endwall;z++)
		{
			x1 = wall[z].x; x2 = wall[wall[z].point2].x;
			y1 = wall[z].y; y2 = wall[wall[z].point2].y;
			if (y1 > y2)
			{
				templong = x1; x1 = x2; x2 = templong;
				templong = y1; y1 = y2; y2 = templong;
			}
			if ((y1 <= y) && (y2 > y))
				//if (x1*(y-y2) + x2*(y1-y) <= 0)
				{
					dax = x1+scale(y-y1,x2-x1,y2-y1);
					dax = (((dax-posx)*zoom)>>14)+320;
					if (dax >= lborder)
						fillist[fillcnt++] = dax;
				}
		}
		if (fillcnt > 0)
		{
			for(z=1;z<fillcnt;z++)
				for (zz=0;zz<z;zz++)
					if (fillist[z] < fillist[zz])
					{
						templong = fillist[z]; fillist[z] = fillist[zz]; fillist[zz] = templong;
					}

			for (z=(fillcnt&1);z<fillcnt-1;z+=2)
			{
				if (fillist[z] > rborder) break;
				if (fillist[z+1] > rborder)
					fillist[z+1] = rborder;
				drawline16(fillist[z],sy,fillist[z+1],sy,fillcolor);
			}
		}
	}
	return(0);
}

whitelinescan(short dalinehighlight)
{
	long i, j, k;
	short sucksect, newnumwalls;

	sucksect = sectorofwall(dalinehighlight);

	memcpy(&sector[numsectors],&sector[sucksect],sizeof(sectortype));
	sector[numsectors].wallptr = numwalls;
	sector[numsectors].wallnum = 0;
	i = dalinehighlight;
	newnumwalls = numwalls;
	do
	{
		j = lastwall((short)i);
		if (wall[j].nextwall >= 0)
		{
			j = wall[j].point2;
			for(k=0;k<numwalls;k++)
			{
				if (wall[wall[k].point2].x == wall[j].x)
					if (wall[wall[k].point2].y == wall[j].y)
						if (wall[k].nextwall == -1)
						{
							j = k;
							break;
						}
			}
		}

		memcpy(&wall[newnumwalls],&wall[i],sizeof(walltype));

		wall[newnumwalls].nextwall = j;
		wall[newnumwalls].nextsector = sectorofwall((short)j);

		newnumwalls++;
		sector[numsectors].wallnum++;

		i = j;
	}
	while (i != dalinehighlight);

	for(i=numwalls;i<newnumwalls-1;i++)
		wall[i].point2 = i+1;
	wall[newnumwalls-1].point2 = numwalls;

	if (clockdir(numwalls) == 1)
		return(-1);
	else
		return(newnumwalls);
}

#define loadbyte(fil,tempbuf,bufplc,dat)        \
{                                               \
	if (bufplc == 0)                             \
	{                                            \
		for(bufplc=0;bufplc<4096;bufplc++)        \
			tempbuf[bufplc] = 0;                   \
		bufplc = 0;                               \
		read(fil,tempbuf,4096);                   \
	}                                            \
	dat = tempbuf[bufplc];                       \
	bufplc = ((bufplc+1)&4095);                  \
}                                               \

loadnames()
{
	char buffer[80], firstch, ch;
	long fil, i, num, buffercnt, bufplc;

	if ((fil = open("names.h",O_BINARY|O_RDWR,S_IREAD)) == -1) return(-1);
	bufplc = 0;
	do { loadbyte(fil,tempbuf,bufplc,firstch); } while (firstch != '#');

	while ((firstch == '#') || (firstch == '/'))
	{
		do { loadbyte(fil,tempbuf,bufplc,ch); } while (ch > 32);

		buffercnt = 0;
		do
		{
			loadbyte(fil,tempbuf,bufplc,ch);
			if (ch > 32) buffer[buffercnt++] = ch;
		}
		while (ch > 32);

		num = 0;
		do
		{
			loadbyte(fil,tempbuf,bufplc,ch);
			if ((ch >= 48) && (ch <= 57)) num = num*10+(ch-48);
		}
		while (ch != 13);
		for(i=0;i<buffercnt;i++) names[num][i] = buffer[i];
		names[num][buffercnt] = 0;

		loadbyte(fil,tempbuf,bufplc,firstch);
		if (firstch == 10) loadbyte(fil,tempbuf,bufplc,firstch);
	}
	close(fil);
	return(0);
}

printcoords16(long posxe, long posye, short ange)
{
	char snotbuf[80];
	long i;

	sprintf(snotbuf,"x=%ld y=%ld ang=%ld",posxe,posye,ange);
	i = 0;
	while ((snotbuf[i] != 0) && (i < 30))
		i++;
	while (i < 30)
	{
		snotbuf[i] = 32;
		i++;
	}
	snotbuf[30] = 0;

	printext16(8, 128, 11, 6, snotbuf,0);

	sprintf(snotbuf,"%ld/%ld sect. %ld/%ld walls %ld/%ld spri.",numsectors,MAXSECTORS,numwalls,MAXWALLS,numsprites,MAXSPRITES);
	i = 0;
	while ((snotbuf[i] != 0) && (i < 46))
		i++;
	while (i < 46)
	{
		snotbuf[i] = 32;
		i++;
	}
	snotbuf[46] = 0;

	printext16(264, 128, 14, 6, snotbuf,0);
}

updatenumsprites()
{
	long i;

	numsprites = 0;
	for(i=0;i<MAXSPRITES;i++)
		if (sprite[i].statnum < MAXSTATUS)
			numsprites++;
}

copysector(short soursector, short destsector, short deststartwall, char copystat)
{
	short j, k, m, newnumwalls, startwall, endwall;

	newnumwalls = deststartwall;  //erase existing sector fragments

		//duplicate walls
	startwall = sector[soursector].wallptr;
	endwall = startwall + sector[soursector].wallnum;
	for(j=startwall;j<endwall;j++)
	{
		memcpy(&wall[newnumwalls],&wall[j],sizeof(walltype));
		wall[newnumwalls].point2 += deststartwall-startwall;
		if (wall[newnumwalls].nextwall >= 0)
		{
			wall[newnumwalls].nextwall += deststartwall-startwall;
			wall[newnumwalls].nextsector += destsector-soursector;
		}
		newnumwalls++;
	}

	//for(j=deststartwall;j<newnumwalls;j++)
	//{
	//   if (wall[j].nextwall >= 0)
	//      checksectorpointer(wall[j].nextwall,wall[j].nextsector);
	//   checksectorpointer((short)j,destsector);
	//}

	if (newnumwalls > deststartwall)
	{
			//duplicate sectors
		memcpy(&sector[destsector],&sector[soursector],sizeof(sectortype));
		sector[destsector].wallptr = deststartwall;
		sector[destsector].wallnum = newnumwalls-deststartwall;

		if (copystat == 1)
		{
				//duplicate sprites
			j = headspritesect[soursector];
			while (j >= 0)
			{
				k = nextspritesect[j];

				m = insertsprite(destsector,sprite[j].statnum);
				memcpy(&sprite[m],&sprite[j],sizeof(spritetype));
				sprite[m].sectnum = destsector;   //Don't let memcpy overwrite sector!

				j = k;
			}
		}

	}
}

showsectordata(short sectnum)
{
	char snotbuf[80];

	sprintf(snotbuf,"Sector %d",sectnum);
	printext16(8,32,11,-1,snotbuf,0);
	sprintf(snotbuf,"Firstwall: %d",sector[sectnum].wallptr);
	printext16(8,48,11,-1,snotbuf,0);
	sprintf(snotbuf,"Numberofwalls: %d",sector[sectnum].wallnum);
	printext16(8,56,11,-1,snotbuf,0);
	sprintf(snotbuf,"Firstsprite: %d",headspritesect[sectnum]);
	printext16(8,64,11,-1,snotbuf,0);
	sprintf(snotbuf,"Tags: %ld, %ld",sector[sectnum].hitag,sector[sectnum].lotag);
	printext16(8,72,11,-1,snotbuf,0);
	sprintf(snotbuf,"     (0x%x), (0x%x)",sector[sectnum].hitag,sector[sectnum].lotag);
	printext16(8,80,11,-1,snotbuf,0);
	sprintf(snotbuf,"Extra: %ld",sector[sectnum].extra);
	printext16(8,88,11,-1,snotbuf,0);
	sprintf(snotbuf,"Visibility: %ld",sector[sectnum].visibility);
	printext16(8,96,11,-1,snotbuf,0);
	sprintf(snotbuf,"Pixel height: %ld",(sector[sectnum].floorz-sector[sectnum].ceilingz)>>8);
	printext16(8,104,11,-1,snotbuf,0);

	printext16(200,32,11,-1,"CEILINGS:",0);
	sprintf(snotbuf,"Flags (hex): %x",sector[sectnum].ceilingstat);
	printext16(200,48,11,-1,snotbuf,0);
	sprintf(snotbuf,"(X,Y)pan: %d, %d",sector[sectnum].ceilingxpanning,sector[sectnum].ceilingypanning);
	printext16(200,56,11,-1,snotbuf,0);
	sprintf(snotbuf,"Shade byte: %d",sector[sectnum].ceilingshade);
	printext16(200,64,11,-1,snotbuf,0);
	sprintf(snotbuf,"Z-coordinate: %ld",sector[sectnum].ceilingz);
	printext16(200,72,11,-1,snotbuf,0);
	sprintf(snotbuf,"Tile number: %d",sector[sectnum].ceilingpicnum);
	printext16(200,80,11,-1,snotbuf,0);
	sprintf(snotbuf,"Ceiling heinum: %d",sector[sectnum].ceilingheinum);
	printext16(200,88,11,-1,snotbuf,0);
	sprintf(snotbuf,"Palookup number: %d",sector[sectnum].ceilingpal);
	printext16(200,96,11,-1,snotbuf,0);

	printext16(400,32,11,-1,"FLOORS:",0);
	sprintf(snotbuf,"Flags (hex): %x",sector[sectnum].floorstat);
	printext16(400,48,11,-1,snotbuf,0);
	sprintf(snotbuf,"(X,Y)pan: %d, %d",sector[sectnum].floorxpanning,sector[sectnum].floorypanning);
	printext16(400,56,11,-1,snotbuf,0);
	sprintf(snotbuf,"Shade byte: %d",sector[sectnum].floorshade);
	printext16(400,64,11,-1,snotbuf,0);
	sprintf(snotbuf,"Z-coordinate: %ld",sector[sectnum].floorz);
	printext16(400,72,11,-1,snotbuf,0);
	sprintf(snotbuf,"Tile number: %d",sector[sectnum].floorpicnum);
	printext16(400,80,11,-1,snotbuf,0);
	sprintf(snotbuf,"Floor heinum: %d",sector[sectnum].floorheinum);
	printext16(400,88,11,-1,snotbuf,0);
	sprintf(snotbuf,"Palookup number: %d",sector[sectnum].floorpal);
	printext16(400,96,11,-1,snotbuf,0);
}

showwalldata(short wallnum)
{
	long dax, day, dist;
	char snotbuf[80];

	sprintf(snotbuf,"Wall %d",wallnum);
	printext16(8,32,11,-1,snotbuf,0);
	sprintf(snotbuf,"X-coordinate: %ld",wall[wallnum].x);
	printext16(8,48,11,-1,snotbuf,0);
	sprintf(snotbuf,"Y-coordinate: %ld",wall[wallnum].y);
	printext16(8,56,11,-1,snotbuf,0);
	sprintf(snotbuf,"Point2: %d",wall[wallnum].point2);
	printext16(8,64,11,-1,snotbuf,0);
	sprintf(snotbuf,"Sector: %d",sectorofwall(wallnum));
	printext16(8,72,11,-1,snotbuf,0);

	sprintf(snotbuf,"Tags: %ld, %ld",wall[wallnum].hitag,wall[wallnum].lotag);
	printext16(8,88,11,-1,snotbuf,0);
	sprintf(snotbuf,"     (0x%x), (0x%x)",wall[wallnum].hitag,wall[wallnum].lotag);
	printext16(8,96,11,-1,snotbuf,0);

	printext16(200,32,11,-1,names[wall[wallnum].picnum],0);
	sprintf(snotbuf,"Flags (hex): %x",wall[wallnum].cstat);
	printext16(200,48,11,-1,snotbuf,0);
	sprintf(snotbuf,"Shade: %d",wall[wallnum].shade);
	printext16(200,56,11,-1,snotbuf,0);
	sprintf(snotbuf,"Pal: %d",wall[wallnum].pal);
	printext16(200,64,11,-1,snotbuf,0);
	sprintf(snotbuf,"(X,Y)repeat: %d, %d",wall[wallnum].xrepeat,wall[wallnum].yrepeat);
	printext16(200,72,11,-1,snotbuf,0);
	sprintf(snotbuf,"(X,Y)pan: %d, %d",wall[wallnum].xpanning,wall[wallnum].ypanning);
	printext16(200,80,11,-1,snotbuf,0);
	sprintf(snotbuf,"Tile number: %d",wall[wallnum].picnum);
	printext16(200,88,11,-1,snotbuf,0);
	sprintf(snotbuf,"OverTile number: %d",wall[wallnum].overpicnum);
	printext16(200,96,11,-1,snotbuf,0);

	sprintf(snotbuf,"nextsector: %d",wall[wallnum].nextsector);
	printext16(400,48,11,-1,snotbuf,0);
	sprintf(snotbuf,"nextwall: %d",wall[wallnum].nextwall);
	printext16(400,56,11,-1,snotbuf,0);

	sprintf(snotbuf,"Extra: %d",wall[wallnum].extra);
	printext16(400,72,11,-1,snotbuf,0);

	dax = wall[wallnum].x-wall[wall[wallnum].point2].x;
	day = wall[wallnum].y-wall[wall[wallnum].point2].y;
	dist = ksqrt(dax*dax+day*day);
	sprintf(snotbuf,"Wall length: %d",dist>>4);
	printext16(400,96,11,-1,snotbuf,0);

	dax = (long)sectorofwall(wallnum);
	sprintf(snotbuf,"Pixel height: %ld",(sector[dax].floorz-sector[dax].ceilingz)>>8);
	printext16(400,104,11,-1,snotbuf,0);
}

showspritedata(short spritenum)
{
	char snotbuf[80];

	sprintf(snotbuf,"Sprite %d",spritenum);
	printext16(8,32,11,-1,snotbuf,0);
	sprintf(snotbuf,"X-coordinate: %ld",sprite[spritenum].x);
	printext16(8,48,11,-1,snotbuf,0);
	sprintf(snotbuf,"Y-coordinate: %ld",sprite[spritenum].y);
	printext16(8,56,11,-1,snotbuf,0);
	sprintf(snotbuf,"Z-coordinate: %ld",sprite[spritenum].z);
	printext16(8,64,11,-1,snotbuf,0);

	sprintf(snotbuf,"Sectnum: %d",sprite[spritenum].sectnum);
	printext16(8,72,11,-1,snotbuf,0);
	sprintf(snotbuf,"Statnum: %d",sprite[spritenum].statnum);
	printext16(8,80,11,-1,snotbuf,0);

	sprintf(snotbuf,"Tags: %ld, %ld",sprite[spritenum].hitag,sprite[spritenum].lotag);
	printext16(8,96,11,-1,snotbuf,0);
	sprintf(snotbuf,"     (0x%x), (0x%x)",sprite[spritenum].hitag,sprite[spritenum].lotag);
	printext16(8,104,11,-1,snotbuf,0);

	printext16(200,32,11,-1,names[sprite[spritenum].picnum],0);
	sprintf(snotbuf,"Flags (hex): %x",sprite[spritenum].cstat);
	printext16(200,48,11,-1,snotbuf,0);
	sprintf(snotbuf,"Shade: %d",sprite[spritenum].shade);
	printext16(200,56,11,-1,snotbuf,0);
	sprintf(snotbuf,"Pal: %d",sprite[spritenum].pal);
	printext16(200,64,11,-1,snotbuf,0);
	sprintf(snotbuf,"(X,Y)repeat: %d, %d",sprite[spritenum].xrepeat,sprite[spritenum].yrepeat);
	printext16(200,72,11,-1,snotbuf,0);
	sprintf(snotbuf,"(X,Y)offset: %d, %d",sprite[spritenum].xoffset,sprite[spritenum].yoffset);
	printext16(200,80,11,-1,snotbuf,0);
	sprintf(snotbuf,"Tile number: %d",sprite[spritenum].picnum);
	printext16(200,88,11,-1,snotbuf,0);

	sprintf(snotbuf,"Angle (2048 degrees): %d",sprite[spritenum].ang);
	printext16(400,48,11,-1,snotbuf,0);
	sprintf(snotbuf,"X-Velocity: %d",sprite[spritenum].xvel);
	printext16(400,56,11,-1,snotbuf,0);
	sprintf(snotbuf,"Y-Velocity: %d",sprite[spritenum].yvel);
	printext16(400,64,11,-1,snotbuf,0);
	sprintf(snotbuf,"Z-Velocity: %d",sprite[spritenum].zvel);
	printext16(400,72,11,-1,snotbuf,0);
	sprintf(snotbuf,"Owner: %d",sprite[spritenum].owner);
	printext16(400,80,11,-1,snotbuf,0);
	sprintf(snotbuf,"Clipdist: %d",sprite[spritenum].clipdist);
	printext16(400,88,11,-1,snotbuf,0);
	sprintf(snotbuf,"Extra: %d",sprite[spritenum].extra);
	printext16(400,96,11,-1,snotbuf,0);
}

inittimer()
{
	outp(0x43,0x34); outp(0x40,(1193181/120)&255); outp(0x40,(1193181/120)>>8);
	oldtimerhandler = _dos_getvect(0x8);
	_disable(); _dos_setvect(0x8, timerhandler); _enable();
}

uninittimer()
{
	outp(0x43,0x34); outp(0x40,0); outp(0x40,0);           //18.2 times/sec
	_disable(); _dos_setvect(0x8, oldtimerhandler); _enable();
}

void __interrupt __far timerhandler()
{
	totalclock++;
	keytimerstuff();
	outp(0x20,0x20);
}

initkeys()
{
	long i;

	keyfifoplc = 0; keyfifoend = 0;
	for(i=0;i<256;i++) keystatus[i] = 0;
	oldkeyhandler = _dos_getvect(0x9);
	_disable(); _dos_setvect(0x9, keyhandler); _enable();
}

uninitkeys()
{
	short *ptr;

	_dos_setvect(0x9, oldkeyhandler);
		//Turn off shifts to prevent stucks with quitting
	ptr = (short *)0x417; *ptr &= ~0x030f;
}

void __interrupt __far keyhandler()
{
	oldreadch = readch; readch = kinp(0x60);
	keytemp = kinp(0x61); koutp(0x61,keytemp|128); koutp(0x61,keytemp&127);
	koutp(0x20,0x20);
	if ((readch|1) == 0xe1) { extended = 128; return; }
	if (oldreadch != readch)
	{
		if ((readch&128) == 0)
		{
			keytemp = readch+extended;
			if (keystatus[keytemp] == 0)
			{
				keystatus[keytemp] = 1;
				keyfifo[keyfifoend] = keytemp;
				keyfifo[(keyfifoend+1)&(KEYFIFOSIZ-1)] = 1;
				keyfifoend = ((keyfifoend+2)&(KEYFIFOSIZ-1));
			}
		}
		else
		{
			keytemp = (readch&127)+extended;
			keystatus[keytemp] = 0;
			keyfifo[keyfifoend] = keytemp;
			keyfifo[(keyfifoend+1)&(KEYFIFOSIZ-1)] = 0;
			keyfifoend = ((keyfifoend+2)&(KEYFIFOSIZ-1));
		}
	}
	extended = 0;
}

keytimerstuff()
{
	if (keystatus[buildkeys[5]] == 0)
	{
		if (keystatus[buildkeys[2]] > 0) angvel = max(angvel-16,-128);
		if (keystatus[buildkeys[3]] > 0) angvel = min(angvel+16,127);
	}
	else
	{
		if (keystatus[buildkeys[2]] > 0) svel = min(svel+8,127);
		if (keystatus[buildkeys[3]] > 0) svel = max(svel-8,-128);
	}
	if (keystatus[buildkeys[0]] > 0) vel = min(vel+8,127);
	if (keystatus[buildkeys[1]] > 0) vel = max(vel-8,-128);
	if (keystatus[buildkeys[12]] > 0) svel = min(svel+8,127);
	if (keystatus[buildkeys[13]] > 0) svel = max(svel-8,-128);

	if (angvel < 0) angvel = min(angvel+12,0);
	if (angvel > 0) angvel = max(angvel-12,0);
	if (svel < 0) svel = min(svel+2,0);
	if (svel > 0) svel = max(svel-2,0);
	if (vel < 0) vel = min(vel+2,0);
	if (vel > 0) vel = max(vel-2,0);
}

printmessage16(char name[82])
{
	char snotbuf[60];
	long i;

	i = 0;
	while ((name[i] != 0) && (i < 54))
	{
		snotbuf[i] = name[i];
		i++;
	}
	while (i < 54)
	{
		snotbuf[i] = 32;
		i++;
	}
	snotbuf[54] = 0;

	printext16(200L, 8L, 0, 6, snotbuf, 0);
}

printmessage256(char name[82])
{
	char snotbuf[40];
	long i;

	i = 0;
	while ((name[i] != 0) && (i < 38))
	{
		snotbuf[i] = name[i];
		i++;
	}
	while (i < 38)
	{
		snotbuf[i] = 32;
		i++;
	}
	snotbuf[38] = 0;

	printext256(0L,0L,whitecol,0,snotbuf,0);
}

	//Find closest point (*dax, *day) on wall (dawall) to (x, y)
getclosestpointonwall(long x, long y, long dawall, long *nx, long *ny)
{
	walltype *wal;
	long i, j, dx, dy;

	wal = &wall[dawall];
	dx = wall[wal->point2].x-wal->x;
	dy = wall[wal->point2].y-wal->y;
	i = dx*(x-wal->x) + dy*(y-wal->y);
	if (i <= 0) { *nx = wal->x; *ny = wal->y; return; }
	j = dx*dx+dy*dy;
	if (i >= j) { *nx = wal->x+dx; *ny = wal->y+dy; return; }
	i = divscale30(i,j);
	*nx = wal->x + mulscale30(dx,i);
	*ny = wal->y + mulscale30(dy,i);
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

static char visited[8192];

GetWallZPeg(long nWall)
{
	long z, nSector, nNextSector;

	nSector = sectorofwall((short)nWall);
	nNextSector = wall[nWall].nextsector;
	if (nNextSector == -1)
	{
			//1-sided wall
		if (wall[nWall].cstat&4) z = sector[nSector].floorz;
								  else z = sector[nSector].ceilingz;
	}
	else
	{
			//2-sided wall
		if (wall[nWall].cstat&4)
			z = sector[nSector].ceilingz;
		else
		{
			if (sector[nNextSector].ceilingz > sector[nSector].ceilingz)
				z = sector[nNextSector].ceilingz;   //top step
			if (sector[nNextSector].floorz < sector[nSector].floorz)
				z = sector[nNextSector].floorz;   //bottom step
		}
	}
	return(z);
}

AlignWalls(long nWall0, long z0, long nWall1, long z1, long nTile)
{
	long n;

		//do the x alignment
	wall[nWall1].cstat &= ~0x0108;    //Set to non-flip
	wall[nWall1].xpanning = (char)((wall[nWall0].xpanning+(wall[nWall0].xrepeat<<3))%tilesizx[nTile]);

	z1 = GetWallZPeg(nWall1);

	for(n=(picsiz[nTile]>>4);((1<<n)<tilesizy[nTile]);n++);

	wall[nWall1].yrepeat = wall[nWall0].yrepeat;
	wall[nWall1].ypanning = (char)(wall[nWall0].ypanning+(((z1-z0)*wall[nWall0].yrepeat)>>(n+3)));
}

AutoAlignWalls(long nWall0, long ply)
{
	long z0, z1, nTile, nWall1, branch, visible, nNextSector, nSector;

	nTile = wall[nWall0].picnum;
	branch = 0;
	if (ply == 0)
	{
			//clear visited bits
		memset(visited,0,sizeof(visited));
		visited[nWall0] = 1;
	}

	z0 = GetWallZPeg(nWall0);

	nWall1 = wall[nWall0].point2;

		//loop through walls at this vertex in CCW order
	while (1)
	{
			//break if this wall would connect us in a loop
		if (visited[nWall1]) break;

		visited[nWall1] = 1;

			//break if reached back of left wall
		if (wall[nWall1].nextwall == nWall0) break;

		if (wall[nWall1].picnum == nTile)
		{
			z1 = GetWallZPeg(nWall1);
			visible = 0;

			nNextSector = wall[nWall1].nextsector;
			if (nNextSector < 0)
				visible = 1;
			else
			{
					//ignore two sided walls that have no visible face
				nSector = wall[wall[nWall1].nextwall].nextsector;
				if (getceilzofslope((short)nSector,wall[nWall1].x,wall[nWall1].y) <
					getceilzofslope((short)nNextSector,wall[nWall1].x,wall[nWall1].y))
					visible = 1;

				if (getflorzofslope((short)nSector,wall[nWall1].x,wall[nWall1].y) >
					getflorzofslope((short)nNextSector,wall[nWall1].x,wall[nWall1].y))
					visible = 1;
			}

			if (visible)
			{
				branch++;
				AlignWalls(nWall0,z0,nWall1,z1,nTile);

					//if wall was 1-sided, no need to recurse
				if (wall[nWall1].nextwall < 0)
				{
					nWall0 = nWall1;
					z0 = GetWallZPeg(nWall0);
					nWall1 = wall[nWall0].point2;
					branch = 0;
					continue;
				}
				else
					AutoAlignWalls(nWall1,ply+1);
			}
		}

		if (wall[nWall1].nextwall < 0) break;
		nWall1 = wall[wall[nWall1].nextwall].point2;
	}
}
