// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include "build.h"
#include "pragmas.h"
#include "names.h"
#include <dos.h>

extern char keystatus[256];

static char tempbuf[256];
extern long qsetmode;
extern short searchsector, searchwall, searchstat;
extern long zmode, kensplayerheight;
extern short defaultspritecstat;

extern short temppicnum, tempcstat, templotag, temphitag, tempextra;
extern char tempshade, temppal, tempxrepeat, tempyrepeat;
extern char somethingintab;

#define NUMOPTIONS 8
#define NUMKEYS 19
static long vesares[13][2] = {320,200,360,200,320,240,360,240,320,400,
									360,400,640,350,640,400,640,480,800,600,
									1024,768,1280,1024,1600,1200};
static char option[NUMOPTIONS] = {0,0,0,0,0,0,1,0};
static char keys[NUMKEYS] =
{
	0xc8,0xd0,0xcb,0xcd,0x2a,0x9d,0x1d,0x39,
	0x1e,0x2c,0xd1,0xc9,0x33,0x34,
	0x9c,0x1c,0xd,0xc,0xf,
};

extern char buildkeys[NUMKEYS];

extern long frameplace, xdimenscale, ydimen;
extern long posx, posy, posz, horiz;
extern short ang, cursectnum;

static long hang = 0;
extern long stretchhline(long,long,long,long,long,long);
#pragma aux stretchhline parm [eax][ebx][ecx][edx][esi][edi];

//Detecting 2D / 3D mode:
//   qsetmode is 200 in 3D mode
//   qsetmode is 350/480 in 2D mode
//
//You can read these variables when F5-F8 is pressed in 3D mode only:
//
//   If (searchstat == 0)  WALL        searchsector=sector, searchwall=wall
//   If (searchstat == 1)  CEILING     searchsector=sector
//   If (searchstat == 2)  FLOOR       searchsector=sector
//   If (searchstat == 3)  SPRITE      searchsector=sector, searchwall=sprite
//   If (searchstat == 4)  MASKED WALL searchsector=sector, searchwall=wall
//
//   searchsector is the sector of the selected item for all 5 searchstat's
//
//   searchwall is undefined if searchstat is 1 or 2
//   searchwall is the wall if searchstat = 0 or 4
//   searchwall is the sprite if searchstat = 3 (Yeah, I know - it says wall,
//                                      but trust me, it's the sprite number)

long ofinetotalclock, ototalclock, averagefps;
#define AVERAGEFRAMES 32
static long frameval[AVERAGEFRAMES], framecnt = 0;

#pragma aux inittimer42 =\
	"in al, 0x61",\
	"or al, 1",\
	"out 0x61, al",\
	"mov al, 0xb4",\
	"out 0x43, al",\
	"xor al, al",\
	"out 0x42, al",\
	"out 0x42, al",\
	modify exact [eax]\

#pragma aux uninittimer42 =\
	"in al, 0x61",\
	"and al, 252",\
	"out 0x61, al",\
	modify exact [eax]\

#pragma aux gettimer42 =\
	"mov al, 0x84",\
	"out 0x43, al",\
	"in al, 0x42",\
	"shl eax, 24",\
	"in al, 0x42",\
	"rol eax, 8",\
	modify [eax]\

void ExtInit(void)
{
	long i, fil;

	/*printf("------------------------------------------------------------------------------\n");
	printf("   BUILD.EXE copyright(c) 1996 by Ken Silverman.  You are granted the\n");
	printf("   right to use this software for your personal use only.  This is a\n");
	printf("   special version to be used with \"Happy Fun KenBuild\" and may not work\n");
	printf("   properly with other Build engine games.  Please refer to license.doc\n");
	printf("   for distribution rights\n");
	printf("------------------------------------------------------------------------------\n");
	getch();
	*/

	initgroupfile("stuff.dat");
	if ((fil = open("setup.dat",O_BINARY|O_RDWR,S_IREAD)) != -1)
	{
		read(fil,&option[0],NUMOPTIONS);
		read(fil,&keys[0],NUMKEYS);
		memcpy((void *)buildkeys,(void *)keys,NUMKEYS);   //Trick to make build use setup.dat keys
		close(fil);
	}
	if (option[4] > 0) option[4] = 0;
	initmouse();
	initengine();
	vidoption = option[0]; xdim = vesares[option[6]&15][0]; ydim = vesares[option[6]&15][1];

		//You can load your own palette lookup tables here if you just
		//copy the right code!
	for(i=0;i<256;i++)
		tempbuf[i] = ((i+32)&255);  //remap colors for screwy palette sectors
	makepalookup(16,tempbuf,0,0,0,1);

	kensplayerheight = 32;
	zmode = 0;
	defaultspritecstat = 0;
	pskyoff[0] = 0; pskyoff[1] = 0; pskybits = 1;
}

void ExtUnInit(void)
{
	uninittimer42();
	uninitgroupfile();
}

static long daviewingrange, daaspect, horizval1, horizval2;
void ExtPreCheckKeys(void)
{
	long cosang, sinang, dx, dy, mindx, i;

	if (keystatus[0x3e])  //F4 - screen re-size
	{
		keystatus[0x3e] = 0;

			//cycle through all vesa modes, then screen-buffer mode
		getvalidvesamodes();
		if (vidoption == 1)
		{
			for(i=0;i<validmodecnt;i++)
				if ((validmodexdim[i] == xdim) && (validmodeydim[i] == ydim))
				{
					if (i == validmodecnt-1)
						setgamemode(2,320L,200L);
					else
						setgamemode(1,validmodexdim[i+1],validmodeydim[i+1]);
					break;
				}
		}
		else if (validmodecnt > 0)
			setgamemode(1,validmodexdim[0],validmodeydim[0]);

		inittimer42();  //Must init here because VESA 0x4F02 messes timer 2
	}

	if (keystatus[0x2a]|keystatus[0x36])
	{
		if (keystatus[0xcf]) hang = max(hang-1,-182);
		if (keystatus[0xc7]) hang = min(hang+1,182);
	}
	else
	{
		if (keystatus[0xcf]) hang = max(hang-8,-182);
		if (keystatus[0xc7]) hang = min(hang+8,182);
	}
	if (keystatus[0x4c]) { hang = 0; horiz = 100; }
	if (hang != 0)
	{
		walock[4094] = 255;
		if (waloff[4094] == 0) allocache(&waloff[4094],240L*384L,&walock[4094]);
		setviewtotile(4094,240L,384L);

		cosang = sintable[(hang+512)&2047];
		sinang = sintable[hang&2047];

		dx = dmulscale1(320,cosang,200,sinang); mindx = dx;
		dy = dmulscale1(-200,cosang,320,sinang);
		horizval1 = dy*(320>>1)/dx-1;

		dx = dmulscale1(320,cosang,-200,sinang); mindx = min(dx,mindx);
		dy = dmulscale1(200,cosang,320,sinang);
		horizval2 = dy*(320>>1)/dx+1;

		daviewingrange = scale(65536,16384*(xdim>>1),mindx-16);
		daaspect = scale(daviewingrange,scale(320,tilesizx[4094],tilesizy[4094]),horizval2+6-horizval1);
		setaspect(daviewingrange,scale(daaspect,ydim*320,xdim*200));
		horiz = 100-divscale15(horizval1+horizval2,daviewingrange);
	}
}

#define MAXVOXMIPS 5
extern char *voxoff[][MAXVOXMIPS];
void ExtAnalyzeSprites(void)
{
	long i, *longptr;
	spritetype *tspr;

	for(i=0,tspr=&tsprite[0];i<spritesortcnt;i++,tspr++)
	{
		switch(tspr->picnum)
		{
			case PLAYER:
				if (!voxoff[0][0]) qloadkvx(0L,"voxel000.kvx");
				tspr->cstat |= 48;
				longptr = (long *)voxoff[0][0];
				tspr->xrepeat = scale(tspr->xrepeat,56,longptr[2]);
				tspr->yrepeat = scale(tspr->yrepeat,56,longptr[2]);
				tspr->picnum = 0;
				tspr->shade -= 6;
				break;
			case BROWNMONSTER:
				if (!voxoff[1][0]) qloadkvx(1L,"voxel001.kvx");
				tspr->cstat |= 48;
				tspr->picnum = 1;
				break;
		}

		tspr->shade += 6;
		if (sector[tspr->sectnum].ceilingstat&1)
			tspr->shade += sector[tspr->sectnum].ceilingshade;
		else
			tspr->shade += sector[tspr->sectnum].floorshade;
	}
}

static char timerinited = 0;
void ExtCheckKeys(void)
{
	long i, j, p, y, dx, dy, cosang, sinang, bufplc, tsizy, tsizyup15;

	if (qsetmode == 200)    //In 3D mode
	{
		if (hang != 0)
		{
			bufplc = waloff[4094]+(mulscale16(horiz-100,xdimenscale)+(tilesizx[4094]>>1))*tilesizy[4094];
			setviewback();
			cosang = sintable[(hang+512)&2047];
			sinang = sintable[hang&2047];
			dx = dmulscale1(xdim,cosang,ydim,sinang);
			dy = dmulscale1(-ydim,cosang,xdim,sinang);

			tsizy = tilesizy[4094];
			tsizyup15 = (tsizy<<15);
			dx = mulscale14(dx,daviewingrange);
			dy = mulscale14(dy,daaspect);
			sinang = mulscale14(sinang,daviewingrange);
			cosang = mulscale14(cosang,daaspect);
			p = ylookup[windowy1]+frameplace+windowx2+1;
			for(y=windowy1;y<=windowy2;y++)
			{
				i = divscale16(tsizyup15,dx);
				stretchhline(0,(xdim>>1)*i+tsizyup15,xdim>>2,i,mulscale32(i,dy)*tsizy+bufplc,p);
				dx -= sinang; dy += cosang; p += ylookup[1];
			}
			walock[4094] = 1;

			sprintf(tempbuf,"%ld",(hang*180)>>10);
			printext256(0L,8L,31,-1,tempbuf,1);
		}

		if (keystatus[0xa]) setaspect(viewingrange+(viewingrange>>8),yxaspect+(yxaspect>>8));
		if (keystatus[0xb]) setaspect(viewingrange-(viewingrange>>8),yxaspect-(yxaspect>>8));
		if (keystatus[0xc]) setaspect(viewingrange,yxaspect-(yxaspect>>8));
		if (keystatus[0xd]) setaspect(viewingrange,yxaspect+(yxaspect>>8));

		if (!timerinited)
		{
			timerinited = 1;
			inittimer42();  //Must init here because VESA 0x4F02 messes timer 2
		}
		i = totalclock-ototalclock; ototalclock += i;
		j = ofinetotalclock-gettimer42(); ofinetotalclock -= j;
		i = ((i*(1193181/120)-(j&65535)+32768)&0xffff0000)+(j&65535);
		if (i) { frameval[framecnt&(AVERAGEFRAMES-1)] = 11931810/i; framecnt++; }
			//Print MAX FRAME RATE
		i = frameval[(framecnt-1)&(AVERAGEFRAMES-1)];
		for(j=AVERAGEFRAMES-1;j>0;j--) i = max(i,frameval[j]);
		averagefps = ((averagefps*3+i)>>2);
		sprintf(tempbuf,"%ld.%ld",averagefps/10,averagefps%10);
		printext256(0L,0L,31,-1,tempbuf,1);

		editinput();
	}
	else
	{
		timerinited = 0;
	}
}

void ExtCleanUp(void)
{
}

void ExtLoadMap(const char *mapname)
{
}

void ExtSaveMap(const char *mapname)
{
}

const char *ExtGetSectorCaption(short sectnum)
{
	if ((sector[sectnum].lotag|sector[sectnum].hitag) == 0)
	{
		tempbuf[0] = 0;
	}
	else
	{
		sprintf(tempbuf,"%hu,%hu",(unsigned short)sector[sectnum].hitag,
										  (unsigned short)sector[sectnum].lotag);
	}
	return(tempbuf);
}

const char *ExtGetWallCaption(short wallnum)
{
	if ((wall[wallnum].lotag|wall[wallnum].hitag) == 0)
	{
		tempbuf[0] = 0;
	}
	else
	{
		sprintf(tempbuf,"%hu,%hu",(unsigned short)wall[wallnum].hitag,
										  (unsigned short)wall[wallnum].lotag);
	}
	return(tempbuf);
}

const char *ExtGetSpriteCaption(short spritenum)
{
	if ((sprite[spritenum].lotag|sprite[spritenum].hitag) == 0)
	{
		tempbuf[0] = 0;
	}
	else
	{
		sprintf(tempbuf,"%hu,%hu",(unsigned short)sprite[spritenum].hitag,
										  (unsigned short)sprite[spritenum].lotag);
	}
	return(tempbuf);
}

//printext16 parameters:
//printext16(long xpos, long ypos, short col, short backcol,
//           char name[82], char fontsize)
//  xpos 0-639   (top left)
//  ypos 0-479   (top left)
//  col 0-15
//  backcol 0-15, -1 is transparent background
//  name
//  fontsize 0=8*8, 1=3*5

//drawline16 parameters:
// drawline16(long x1, long y1, long x2, long y2, char col)
//  x1, x2  0-639
//  y1, y2  0-143  (status bar is 144 high, origin is top-left of STATUS BAR)
//  col     0-15

void ExtShowSectorData(short sectnum)   //F5
{
	if (qsetmode == 200)    //In 3D mode
	{
	}
	else
	{
		clearmidstatbar16();             //Clear middle of status bar

		sprintf(tempbuf,"Sector %d",sectnum);
		printext16(8,32,11,-1,tempbuf,0);

		printext16(8,48,11,-1,"8*8 font: ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 0123456789",0);
		printext16(8,56,11,-1,"3*5 font: ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 0123456789",1);

		drawline16(320,68,344,80,4);       //Draw house
		drawline16(344,80,344,116,4);
		drawline16(344,116,296,116,4);
		drawline16(296,116,296,80,4);
		drawline16(296,80,320,68,4);
	}
}

void ExtShowWallData(short wallnum)       //F6
{
	if (qsetmode == 200)    //In 3D mode
	{
	}
	else
	{
		clearmidstatbar16();             //Clear middle of status bar

		sprintf(tempbuf,"Wall %d",wallnum);
		printext16(8,32,11,-1,tempbuf,0);
	}
}

void ExtShowSpriteData(short spritenum)   //F6
{
	if (qsetmode == 200)    //In 3D mode
	{
	}
	else
	{
		clearmidstatbar16();             //Clear middle of status bar

		sprintf(tempbuf,"Sprite %d",spritenum);
		printext16(8,32,11,-1,tempbuf,0);
	}
}

void ExtEditSectorData(short sectnum)    //F7
{
	short nickdata;

	if (qsetmode == 200)    //In 3D mode
	{
			//Ceiling
		if (searchstat == 1)
			sector[searchsector].ceilingpicnum++;   //Just a stupid example

			//Floor
		if (searchstat == 2)
			sector[searchsector].floorshade++;      //Just a stupid example
	}
	else                    //In 2D mode
	{
		sprintf(tempbuf,"Sector (%ld) Nick's variable: ",sectnum);
		nickdata = 0;
		nickdata = getnumber16(tempbuf,nickdata,65536L);

		printmessage16("");              //Clear message box (top right of status bar)
		ExtShowSectorData(sectnum);
	}
}

void ExtEditWallData(short wallnum)       //F8
{
	short nickdata;

	if (qsetmode == 200)    //In 3D mode
	{
	}
	else
	{
		sprintf(tempbuf,"Wall (%ld) Nick's variable: ",wallnum);
		nickdata = 0;
		nickdata = getnumber16(tempbuf,nickdata,65536L);

		printmessage16("");              //Clear message box (top right of status bar)
		ExtShowWallData(wallnum);
	}
}

void ExtEditSpriteData(short spritenum)   //F8
{
	short nickdata;

	if (qsetmode == 200)    //In 3D mode
	{
	}
	else
	{
		sprintf(tempbuf,"Sprite (%ld) Nick's variable: ",spritenum);
		nickdata = 0;
		nickdata = getnumber16(tempbuf,nickdata,65536L);
		printmessage16("");

		printmessage16("");              //Clear message box (top right of status bar)
		ExtShowSpriteData(spritenum);
	}
}

faketimerhandler()
{
}

	//Just thought you might want my getnumber16 code
/*
getnumber16(char namestart[80], short num, long maxnumber)
{
	char buffer[80];
	long j, k, n, danum, oldnum;

	danum = (long)num;
	oldnum = danum;
	while ((keystatus[0x1c] != 2) && (keystatus[0x1] == 0))  //Enter, ESC
	{
		sprintf(&buffer,"%s%ld_ ",namestart,danum);
		printmessage16(buffer);

		for(j=2;j<=11;j++)                //Scan numbers 0-9
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
		if (keystatus[0x1c] == 1)   //L. enter
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
*/
