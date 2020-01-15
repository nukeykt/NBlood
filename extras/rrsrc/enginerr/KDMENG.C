#include <malloc.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>

#define NUMCHANNELS 16
#define MAXWAVES 256
#define MAXTRACKS 256
#define MAXNOTES 8192
#define MAXEFFECTS 16

	//Actual sound parameters after initsb was called
long samplerate, numspeakers, bytespersample, intspersec, kdmqual;

	//KWV wave variables
long numwaves;
char instname[MAXWAVES][16];
long wavleng[MAXWAVES];
long repstart[MAXWAVES], repleng[MAXWAVES];
long finetune[MAXWAVES];

	//Other useful wave variables
long totsndbytes, totsndmem, wavoffs[MAXWAVES];

	//Effects array
long eff[MAXEFFECTS][256];

	//KDM song variables:
long kdmversionum, numnotes, numtracks;
char trinst[MAXTRACKS], trquant[MAXTRACKS];
char trvol1[MAXTRACKS], trvol2[MAXTRACKS];
long nttime[MAXNOTES];
char nttrack[MAXNOTES], ntfreq[MAXNOTES];
char ntvol1[MAXNOTES], ntvol2[MAXNOTES];
char ntfrqeff[MAXNOTES], ntvoleff[MAXNOTES], ntpaneff[MAXNOTES];

	//Other useful song variables:
long timecount, notecnt, musicstatus, musicrepeat;

long kdmasm1, kdmasm2, kdmasm3, kdmasm4;

static char digistat = 0, musistat = 0;

char *snd = NULL, kwvname[20] = {""};

#define MAXBYTESPERTIC 1024+128
static long stemp[MAXBYTESPERTIC];

	//Sound reading information
long splc[NUMCHANNELS], sinc[NUMCHANNELS], soff[NUMCHANNELS];
long svol1[NUMCHANNELS], svol2[NUMCHANNELS];
long volookup[NUMCHANNELS<<9];
long swavenum[NUMCHANNELS];
long frqeff[NUMCHANNELS], frqoff[NUMCHANNELS];
long voleff[NUMCHANNELS], voloff[NUMCHANNELS];
long paneff[NUMCHANNELS], panoff[NUMCHANNELS];

static long globposx, globposy, globxvect, globyvect;
static long xplc[NUMCHANNELS], yplc[NUMCHANNELS], vol[NUMCHANNELS];
static long vdist[NUMCHANNELS], sincoffs[NUMCHANNELS];
static char chanstat[NUMCHANNELS];

long frqtable[256];

static long mixerval = 0;

static long kdminprep = 0, kdmintinprep = 0;
static long dmacheckport, dmachecksiz;

void (__interrupt __far *oldsbhandler)();
void __interrupt __far sbhandler(void);

long samplediv, oldtimerfreq, chainbackcnt, chainbackstart;
char *pcsndptr, pcsndlookup[256], bufferside;
long samplecount, pcsndbufsiz, pcrealmodeint;
static short kdmvect = 0x8;
static unsigned short kdmpsel, kdmrseg, kdmroff;
static unsigned long kdmpoff;

void (__interrupt __far *oldpctimerhandler)();
#define KDMCODEBYTES 256
static char pcrealbuffer[KDMCODEBYTES] =        //See pckdm.asm
{
	0x50,0x53,0x51,0x52,0x32,0xC0,0xE6,0x42,0xB0,0x20,
	0xE6,0x20,0x5A,0x59,0x5B,0x58,0xCF,
};

	//Sound interrupt information
long sbport = 0x220, sbirq = 0x7, sbdma8 = 0x1, sbdma16 = 0x1;
char dmapagenum[8] = {0x87,0x83,0x81,0x82,0x8f,0x8b,0x89,0x8a};
long sbdma, sbver;
unsigned short sndselector;
volatile long sndoffs, sndoffsplc, sndoffsxor, sndplc, sndend;
static long bytespertic, sndbufsiz;

char qualookup[512*16];
long ramplookup[64];

unsigned short sndseg = 0;

extern long monolocomb(long,long,long,long,long,long);
#pragma aux monolocomb parm [eax][ebx][ecx][edx][esi][edi];
extern long monohicomb(long,long,long,long,long,long);
#pragma aux monohicomb parm [eax][ebx][ecx][edx][esi][edi];
extern long stereolocomb(long,long,long,long,long,long);
#pragma aux stereolocomb parm [eax][ebx][ecx][edx][esi][edi];
extern long stereohicomb(long,long,long,long,long,long);
#pragma aux stereohicomb parm [eax][ebx][ecx][edx][esi][edi];
extern long setuppctimerhandler(long,long,long,long,long,long);
#pragma aux setuppctimerhandler parm [eax][ebx][ecx][edx][esi][edi];
extern void interrupt pctimerhandler();
extern long pcbound2char(long,long,long);
#pragma aux pcbound2char parm [ecx][esi][edi];
extern long bound2char(long,long,long);
#pragma aux bound2char parm [ecx][esi][edi];
extern long bound2short(long,long,long);
#pragma aux bound2short parm [ecx][esi][edi];

static long oneshr10 = 0x3a800000, oneshl14 = 0x46800000;
#pragma aux fsin =\
	"fldpi",\
	"fimul dword ptr [eax]",\
	"fmul dword ptr [oneshr10]",\
	"fsin",\
	"fmul dword ptr [oneshl14]",\
	"fistp dword ptr [eax]",\
	parm [eax]\

#pragma aux convallocate =\
	"mov ax, 0x100",\
	"int 0x31",\
	"jnc nocarry",\
	"mov ax, 0",\
	"nocarry: mov sndselector, dx",\
	parm [bx]\
	modify [eax edx]\

#pragma aux convdeallocate =\
	"mov ax, 0x101",\
	"mov dx, sndselector",\
	"int 0x31",\
	parm [dx]\
	modify [eax edx]\

#pragma aux resetsb =\
	"mov edx, sbport",\
	"add edx, 0x6",\
	"mov al, 1",\
	"out dx, al",\
	"xor al, al",\
	"delayreset: dec al",\
	"jnz delayreset",\
	"out dx, al",\
	"mov ecx, 65536",\
	"empty: mov edx, sbport",\
	"add edx, 0xe",\
	"in al, dx",\
	"test al, al",\
	"jns nextattempt",\
	"sub dl, 4",\
	"in al, dx",\
	"cmp al, 0aah",\
	"je resetok",\
	"dec ecx",\
	"nextattempt: jnz empty",\
	"mov eax, -1",\
	"jmp resetend",\
	"resetok: xor eax, eax",\
	"resetend:",\
	modify [ebx ecx edx]\

#pragma aux sbin =\
	"xor eax, eax",\
	"mov dx, word ptr sbport[0]",\
	"add dl, 0xe",\
	"busy: in al, dx",\
	"or al, al",\
	"jns busy",\
	"sub dl, 4",\
	"in al, dx",\
	modify [edx]\

#pragma aux sbout =\
	"mov dx, word ptr sbport[0]",\
	"add dl, 0xc",\
	"mov ah, al",\
	"busy: in al, dx",\
	"or al, al",\
	"js busy",\
	"mov al, ah",\
	"out dx, al",\
	parm [eax]\
	modify [edx]\

#pragma aux sbmixin =\
	"mov dx, word ptr sbport[0]",\
	"add dl, 0x4",\
	"out dx, al",\
	"inc dx",\
	"xor eax, eax",\
	"in al, dx",\
	parm [eax]\
	modify [edx]\

#pragma aux sbmixout =\
	"mov dx, word ptr sbport[0]",\
	"add dl, 0x4",\
	"out dx, al",\
	"inc dx",\
	"mov al, bl",\
	"out dx, al",\
	parm [eax][ebx]\
	modify [edx]\

#pragma aux findpas =\
	"mov eax, 0x0000bc00",\
	"mov ebx, 0x00003f3f",\
	"int 0x2f",\
	"xor bx, cx",\
	"xor bx, dx",\
	"cmp bx, 0x4d56",\
	"stc",\
	"jne nopasfound",\
	"mov eax, 0x0000bc04",\
	"int 0x2f",\
	"mov edx, 0x0000ffff",\
	"and ebx, edx",\
	"and ecx, edx",\
	"mov sbdma, ebx",\
	"mov sbirq, ecx",\
	"clc",\
	"nopasfound:",\
	"sbb eax, eax",\
	modify [eax ebx ecx edx]\

#pragma aux calcvolookupmono =\
	"mov ecx, 64",\
	"lea edx, [eax+ebx]",\
	"add ebx, ebx",\
	"begit: mov dword ptr [edi], eax",\
	"mov dword ptr [edi+4], edx",\
	"add eax, ebx",\
	"add edx, ebx",\
	"mov dword ptr [edi+8], eax",\
	"mov dword ptr [edi+12], edx",\
	"add eax, ebx",\
	"add edx, ebx",\
	"add edi, 16",\
	"dec ecx",\
	"jnz begit",\
	parm [edi][eax][ebx]\
	modify [ecx edx]\

#pragma aux calcvolookupstereo =\
	"mov esi, 128",\
	"begit: mov dword ptr [edi], eax",\
	"mov dword ptr [edi+4], ecx",\
	"add eax, ebx",\
	"add ecx, edx",\
	"mov dword ptr [edi+8], eax",\
	"mov dword ptr [edi+12], ecx",\
	"add eax, ebx",\
	"add ecx, edx",\
	"add edi, 16",\
	"dec esi",\
	"jnz begit",\
	parm [edi][eax][ebx][ecx][edx]\
	modify [esi]\

#pragma aux gettimerval =\
	"xor eax, eax",\
	"xor ebx, ebx",\
	"mov ecx, 65536",\
	"xor edx, edx",\
	"loopit: mov al, 0x4",\
	"out 0x43, al",\
	"in al, 0x40",\
	"mov dl, al",\
	"in al, 0x40",\
	"mov dh, al",\
	"cmp ebx, edx",\
	"dec ecx",\
	"ja loopit",\
	"jz endit",\
	"mov ebx, edx",\
	"jmp loopit",\
	"endit: mov eax, ebx",\
	modify [eax ebx ecx edx]\

#pragma aux klabs =\
	"test eax, eax",\
	"jns skipnegate",\
	"neg eax",\
	"skipnegate:",\
	parm [eax]\

#pragma aux mulscale16 =\
	"imul ebx",\
	"shrd eax, edx, 16",\
	parm nomemory [eax][ebx]\
	modify exact [eax edx]\

#pragma aux mulscale24 =\
	"imul ebx",\
	"shrd eax, edx, 24",\
	parm nomemory [eax][ebx]\
	modify exact [eax edx]\

#pragma aux mulscale30 =\
	"imul ebx",\
	"shrd eax, edx, 30",\
	parm nomemory [eax][ebx]\
	modify exact [eax edx]\

#pragma aux dmulscale28 =\
	"imul edx",\
	"mov ebx, eax",\
	"mov eax, esi",\
	"mov esi, edx",\
	"imul edi",\
	"add eax, ebx",\
	"adc edx, esi",\
	"shrd eax, edx, 28",\
	parm nomemory [eax][edx][esi][edi]\
	modify exact [eax ebx edx esi]\

#pragma aux clearbuf =\
	"snot: mov dword ptr [edi], eax",\
	"add edi, 4",\
	"loop snot",\
	parm [edi][ecx][eax]\

#pragma aux copybuf =\
	"snot: mov eax, dword ptr [esi]",\
	"mov dword ptr [edi], eax",\
	"add esi, 4",\
	"add edi, 4",\
	"loop snot",\
	parm [esi][edi][ecx]\
	modify [eax]\

#pragma aux koutp =\
	"out dx, al",\
	parm [edx][eax]\

#pragma aux koutpw =\
	"out dx, ax",\
	parm [edx][eax]\

#pragma aux kinp =\
	"xor eax, eax",\
	"in al, dx",\
	parm [edx]\

#pragma aux msqrtasm =\
	"mov eax, 0x40000000",\
	"mov ebx, 0x20000000",\
	"begit: cmp ecx, eax",\
	"jl skip",\
	"sub ecx, eax",\
	"lea eax, [eax+ebx*4]",\
	"skip: sub eax, ebx",\
	"shr eax, 1",\
	"shr ebx, 2",\
	"jnz begit",\
	"cmp ecx, eax",\
	"sbb eax, -1",\
	"shr eax, 1",\
	parm nomemory [ecx]\
	modify exact [eax ebx ecx]\

initsb(char dadigistat, char damusistat, long dasamplerate, char danumspeakers, char dabytespersample, char daintspersec, char daquality)
{
	long i, j, k;

	digistat = dadigistat;
	musistat = damusistat;

	if ((digistat == 0) && (musistat != 1))
		return;

	samplerate = dasamplerate;
	if (samplerate < 6000) samplerate = 6000;
	if (samplerate > 48000) samplerate = 48000;
	numspeakers = danumspeakers;
	if (numspeakers < 1) numspeakers = 1;
	if (numspeakers > 2) numspeakers = 2;
	bytespersample = dabytespersample;
	if (bytespersample < 1) bytespersample = 1;
	if (bytespersample > 2) bytespersample = 2;
	intspersec = daintspersec;
	if (intspersec < 1) intspersec = 1;
	if (intspersec > 120) intspersec = 120;
	kdmqual = daquality;
	if (kdmqual < 0) kdmqual = 0;
	if (kdmqual > 1) kdmqual = 1;

	switch(digistat)
	{
		case 1:
			getsbset();
			if (resetsb() != 0) { digistat = musistat = 0; break; }

			sbout(0xe1);
			sbver = (sbin()<<8);
			sbver += sbin();

			if (sbver < 0x0201) samplerate = min(samplerate,22050);
			if (sbver < 0x0300) numspeakers = 1;
			if (sbver < 0x0400)
			{
				samplerate = min(samplerate,44100>>(numspeakers-1));
				bytespersample = 1;
			}

			if (bytespersample == 2) sbdma = sbdma16; else sbdma = sbdma8;

			break;
		case 2:
			findpas();        // If == -1 then print not found & quit
			koutp(0xf8a,128);
			break;
		case 13:
			if (numspeakers == 2) numspeakers = 1;
			if (bytespersample == 2) bytespersample = 1;
			break;
	}

	bytespertic = (((samplerate/120)+1)&~1);
	sndbufsiz = bytespertic*(120/intspersec);

	if (sndseg == 0)    //Allocate DMA buffer in conventional memory
	{
		if ((sndseg = convallocate(((sndbufsiz<<(bytespersample+numspeakers))+15)>>4)) == 0)
		{
			printf("Could not allocation conventional memory for digitized music\n");
			exit(0);
		}
		sndoffs = (((long)sndseg)<<4);

		if ((sndoffs&65535)+(sndbufsiz<<(bytespersample+numspeakers-1)) >= 65536)   //64K DMA page check
			sndoffs += (sndbufsiz<<(bytespersample+numspeakers-1));

		sndoffsplc = sndoffs;
		sndoffsxor = sndoffsplc ^ (sndoffsplc+(sndbufsiz<<(bytespersample+numspeakers-2)));
	}
	
	j = (((11025L*2093)/samplerate)<<13);
	for(i=1;i<76;i++)
	{
		frqtable[i] = j;
		j = mulscale30(j,1137589835);  //(pow(2,1/12)<<30) = 1137589835
	}
	for(i=0;i>=-14;i--) frqtable[i&255] = (frqtable[(i+12)&255]>>1);

	loadwaves("WAVES.KWV");

	timecount = notecnt = musicstatus = musicrepeat = 0;

	clearbuf(FP_OFF(stemp),sizeof(stemp)>>2,32768L);
	for(i=0;i<256;i++)
		for(j=0;j<16;j++)
		{
			qualookup[(j<<9)+i] = (((-i)*j+8)>>4);
			qualookup[(j<<9)+i+256] = (((256-i)*j+8)>>4);
		}
	for(i=0;i<(samplerate>>11);i++)
	{
		j = 1536 - (i<<10)/(samplerate>>11);
		fsin(&j);
		ramplookup[i] = ((16384-j)<<1);
	}

	for(i=0;i<256;i++)
	{
		j = i*90; fsin(&j);
		eff[0][i] = 65536+j/9;
		eff[1][i] = min(58386+((i*(65536-58386))/30),65536);
		eff[2][i] = max(69433+((i*(65536-69433))/30),65536);
		j = (i*2048)/120; fsin(&j);
		eff[3][i] = 65536+(j<<2);
		j = (i*2048)/30; fsin(&j);
		eff[4][i] = 65536+j;
		switch((i>>1)%3)
		{
			case 0: eff[5][i] = 65536; break;
			case 1: eff[5][i] = 65536*330/262; break;
			case 2: eff[5][i] = 65536*392/262; break;
		}
		eff[6][i] = min((i<<16)/120,65536);
		eff[7][i] = max(65536-(i<<16)/120,0);
	}

	switch(digistat)
	{
		case 1: case 2:
			if (sbirq < 8)
			{
				oldsbhandler = _dos_getvect(sbirq+0x8);           //Set new IRQ7 vector
				_disable(); _dos_setvect(sbirq+0x8, sbhandler); _enable();
				koutp(0x21,kinp(0x21) & ~(1<<(sbirq&7)));
			}
			else
			{
				oldsbhandler = _dos_getvect(sbirq+0x68);    //Set new SB IRQ vector
				_disable(); _dos_setvect(sbirq+0x68, sbhandler); _enable();
				koutp(0xa1,kinp(0xa1) & ~(1<<(sbirq&7)));
			}
			break;
	}

	musicoff();

	if (digistat != 255)
	{
		preparesndbuf();
		if (digistat != 13) preparesndbuf();

		if ((digistat == 1) || (digistat == 2))
		{
			if (sbdma < 4)
			{
				dmacheckport = (sbdma<<1)+1;
				dmachecksiz = (sndbufsiz<<(bytespersample+numspeakers-1))-1;

				koutp(0xa,sbdma+4);             //Set up DMA REGISTERS
				koutp(0xc,0);
				koutp(0xb,0x58+sbdma);          //&H58 - auto-init, &H48 - 1 block only
				koutp(sbdma<<1,sndoffs&255);
				koutp(sbdma<<1,(sndoffs>>8)&255);
				koutp(dmacheckport,dmachecksiz&255);
				koutp(dmacheckport,(dmachecksiz>>8)&255);
				koutp(dmapagenum[sbdma],((sndoffs>>16)&255));
				koutp(0xa,sbdma);
			}
			else
			{
				dmacheckport = ((sbdma&3)<<2)+0xc2;
				dmachecksiz = ((sndbufsiz<<(bytespersample+numspeakers-1))>>1)-1;

				koutp(0xd4,sbdma);               //Set up DMA REGISTERS
				koutp(0xd8,0);
				koutp(0xd6,0x58+(sbdma&3));      //&H58 - auto-init, &H48 - 1 block only
				koutp(dmacheckport-2,(sndoffs>>1)&255);
				koutp(dmacheckport-2,((sndoffs>>1)>>8)&255);
				koutp(dmacheckport,dmachecksiz&255);
				koutp(dmacheckport,(dmachecksiz>>8)&255);
				koutp(dmapagenum[sbdma],((sndoffs>>16)&255));
				koutp(0xd4,sbdma&3);
			}
		}

		switch(digistat)
		{
			case 1:
				sbout(0xd1);                                    //SB Speaker on

				if (sbver < 0x0400)
				{
					sbout(0x40);                            //SB Set speed
					sbout(256-(1000000/(samplerate<<(numspeakers-1))));
				}

				if (sbver < 0x0200)
				{
					sbout(0x14);                                 //SB 1-shot mode
					i = sndbufsiz-1;
					sbout(i&255);
					sbout((i>>8)&255);
				}
				else
				{
					if (sbver < 0x0400)
					{
							//Set length for auto-init mode
						i = (sndbufsiz<<(numspeakers+bytespersample-2))-1;
						sbout(0x48);
						sbout(i&255);
						sbout((i>>8)&255);
						if (numspeakers == 2)                   //SB set stereo
						{
							sbmixout(0L,0L);
							sbmixout(0xe,sbmixin(0xe)|2);
						}
						if ((samplerate<<(numspeakers-1)) <= 22050)
							sbout(0x1c);                         //Start SB interrupts!
						else
							sbout(0x90);                         //High Speed DMA
					}
					else
					{
						sbout(0x41);
						sbout((samplerate>>8)&255);
						sbout(samplerate&255);

						sbout(0xc6-((bytespersample-1)<<4));
						sbout(((bytespersample-1)<<4)+((numspeakers-1)<<5));
						i = (sndbufsiz<<(numspeakers-1))-1;
						sbout(i&255);
						sbout((i>>8)&255);
					}
				}
				break;
			case 2:
				koutp(0xf88,128);
				koutp(0xb8a,0);

				i = (1193180L>>(numspeakers-1)) / samplerate;
				koutp(0x138b,0x36); koutp(0x1388,i&255); koutp(0x1388,i>>8);
				i = (sndbufsiz<<(bytespersample+numspeakers-2));
				koutp(0x138b,0x74); koutp(0x1389,i&255); koutp(0x1389,i>>8);

				koutp(0x8389,0x3+((bytespersample-1)<<2)); //0x3=8bit/0x7=16bit
				koutp(0xb89,0xdf);
				koutp(0xb8b,0x8);
				koutp(0xf8a,0xd9+((2-numspeakers)<<5));    //0xd9=play/0xc9=record
				koutp(0xb8a,0xe1);
				break;
			case 13:
				samplecount = sndbufsiz;
				pcsndptr = (char *)sndoffs;
				bufferside = 0;
				pcsndbufsiz = sndbufsiz;
				pcrealmodeint = 0;

				samplediv = 1193280L / samplerate;

				j = 0;
				for(i=0;i<256;i++)
				{
						 //Scale (65536 normal)
					k = mulscale24(j-(samplediv<<7),160000) + (samplediv>>1);
					if (k < 0) k = 0;
					if (k > samplerate) k = samplerate;
					pcsndlookup[i] = (char)(k+1);
					j += samplediv;
				}

				oldtimerfreq = gettimerval();
				chainbackstart = oldtimerfreq/samplediv;
				chainbackcnt = chainbackstart;
				setuppctimerhandler(sndoffs+sndbufsiz,oldtimerfreq,0L,0L,0L,0L);

				_disable();
				oldpctimerhandler = _dos_getvect(0x8);
				installbikdmhandlers();
				koutp(0x43,0x34); koutp(0x40,samplediv&255); koutp(0x40,(samplediv>>8)&255);
				koutp(0x43,0x90);
				koutp(97,kinp(97)|3);
				_enable();
				break;
		}
	}
}

getsbset()
{
	char *sbset;
	long i;

	sbset = getenv("BLASTER");

	i = 0;
	while (sbset[i] > 0)
	{
		switch(sbset[i])
		{
			case 'A': case 'a':
				i++;
				sbport = 0;
				while (((sbset[i] >= 48) && (sbset[i] <= 57)) ||
						 ((sbset[i] >= 'A') && (sbset[i] <= 'F')) ||
						 ((sbset[i] >= 'a') && (sbset[i] <= 'f')))
				{
					sbport *= 16;
					if ((sbset[i] >= 48) && (sbset[i] <= 57)) sbport += sbset[i]-48;
					if ((sbset[i] >= 'A') && (sbset[i] <= 'F')) sbport += sbset[i]-55;
					if ((sbset[i] >= 'a') && (sbset[i] <= 'f')) sbport += sbset[i]-55-32;
					i++;
				}
				break;
			case 'I': case 'i':
				i++;
				sbirq = 0;
				while ((sbset[i] >= 48) && (sbset[i] <= 57))
				{
					sbirq *= 10;
					sbirq += sbset[i]-48;
					i++;
				}
				break;
			case 'D': case 'd':
				i++;
				sbdma8 = 0;
				while ((sbset[i] >= 48) && (sbset[i] <= 57))
				{
					sbdma8 *= 10;
					sbdma8 += sbset[i]-48;
					i++;
				}
				break;
			case 'H': case 'h':
				i++;
				sbdma16 = 0;
				while ((sbset[i] >= 48) && (sbset[i] <= 57))
				{
					sbdma16 *= 10;
					sbdma16 += sbset[i]-48;
					i++;
				}
				break;
			default:
				i++;
				break;
		}
	}
}

void __interrupt __far sbhandler()
{
	switch(digistat)
	{
		case 1:
			if (sbver < 0x0200)
			{
				sbout(0x14);                           //SB 1-shot mode
				sbout((sndbufsiz-1)&255);
				sbout(((sndbufsiz-1)>>8)&255);
				kinp(sbport+0xe);                      //Acknowledge SB
			}
			else
			{
				mixerval = sbmixin(0x82);
				if (mixerval&1) kinp(sbport+0xe);      //Acknowledge 8-bit DMA
				if (mixerval&2) kinp(sbport+0xf);      //Acknowledge 16-bit DMA
			}
			break;
		case 2:
			if ((kinp(0xb89)&8) > 0) koutp(0xb89,0);
			break;
	}
	if (sbirq >= 8) koutp(0xa0,0x20);
	koutp(0x20,0x20);
	_enable(); preparesndbuf();
}

uninitsb()
{
	if ((digistat == 0) && (musistat != 1))
		return;

	if (digistat != 255)
	{
		if ((digistat == 1) || (digistat == 2))  //Mask off DMA
		{
			if (sbdma < 4) koutp(0xa,sbdma+4); else koutp(0xd4,sbdma);
		}

		switch(digistat)
		{
			case 1:
				if (sbver >= 0x0400) sbout(0xda-(bytespersample-1));
				resetsb();
				sbout(0xd3);                           //Turn speaker off
				break;
			case 2:
				koutp(0xb8a,32);                       //Stop interrupts
				koutp(0xf8a,0x9);                      //DMA stop
				break;
			case 13:
				koutp(97,kinp(97)&252);
				koutp(0x43,0x34); koutp(0x40,0); koutp(0x40,0);
				koutp(0x43,0xbc);
				uninstallbikdmhandlers();
				break;
		}
	}

	if (snd != 0) free(snd), snd = 0;
	if (sndseg != 0) convdeallocate(sndseg), sndseg = 0;

	switch(digistat)
	{
		case 1: case 2:
			if (sbirq < 8)
			{
				koutp(0x21,kinp(0x21) | (1<<(sbirq&7)));
				_disable(); _dos_setvect(sbirq+0x8, oldsbhandler); _enable();
			}
			else
			{
				koutp(0xa1,kinp(0xa1) | (1<<(sbirq&7)));
				_disable(); _dos_setvect(sbirq+0x68, oldsbhandler); _enable();
			}
			break;
	}
}

startwave(long wavnum, long dafreq, long davolume1, long davolume2, long dafrqeff, long davoleff, long dapaneff)
{
	long i, j, chanum;

	if ((davolume1|davolume2) == 0) return;

	chanum = 0;
	for(i=NUMCHANNELS-1;i>0;i--)
		if (splc[i] > splc[chanum])
			chanum = i;

	splc[chanum] = 0;     //Disable channel temporarily for clean switch

	if (numspeakers == 1)
		calcvolookupmono(FP_OFF(volookup)+(chanum<<(9+2)),-(davolume1+davolume2)<<6,(davolume1+davolume2)>>1);
	else
		calcvolookupstereo(FP_OFF(volookup)+(chanum<<(9+2)),-(davolume1<<7),davolume1,-(davolume2<<7),davolume2);

	sinc[chanum] = dafreq;
	svol1[chanum] = davolume1;
	svol2[chanum] = davolume2;
	soff[chanum] = wavoffs[wavnum]+wavleng[wavnum];
	splc[chanum] = -(wavleng[wavnum]<<12);              //splc's modified last
	swavenum[chanum] = wavnum;
	frqeff[chanum] = dafrqeff; frqoff[chanum] = 0;
	voleff[chanum] = davoleff; voloff[chanum] = 0;
	paneff[chanum] = dapaneff; panoff[chanum] = 0;
	chanstat[chanum] = 0; sincoffs[chanum] = 0;
}

setears(long daposx, long daposy, long daxvect, long dayvect)
{
	globposx = daposx;
	globposy = daposy;
	globxvect = daxvect;
	globyvect = dayvect;
}

wsayfollow(char *dafilename, long dafreq, long davol, long *daxplc, long *dayplc, char followstat)
{
	char ch1, ch2, bad;
	long i, wavnum, chanum;

	if (digistat == 0) return;
	if (davol <= 0) return;

	for(wavnum=numwaves-1;wavnum>=0;wavnum--)
	{
		bad = 0;

		i = 0;
		while ((dafilename[i] > 0) && (i < 16))
		{
			ch1 = dafilename[i]; if ((ch1 >= 97) && (ch1 <= 123)) ch1 -= 32;
			ch2 = instname[wavnum][i]; if ((ch2 >= 97) && (ch2 <= 123)) ch2 -= 32;
			if (ch1 != ch2) {bad = 1; break;}
			i++;
		}
		if (bad != 0) continue;

		chanum = 0;
		for(i=NUMCHANNELS-1;i>0;i--) if (splc[i] > splc[chanum]) chanum = i;

		splc[chanum] = 0;     //Disable channel temporarily for clean switch

		if (followstat == 0)
		{
			xplc[chanum] = *daxplc;
			yplc[chanum] = *dayplc;
		}
		else
		{
			xplc[chanum] = ((long)daxplc);
			yplc[chanum] = ((long)dayplc);
		}
		vol[chanum] = davol;
		vdist[chanum] = 0;
		sinc[chanum] = (dafreq*11025)/samplerate;
		svol1[chanum] = davol;
		svol2[chanum] = davol;
		sincoffs[chanum] = 0;
		soff[chanum] = wavoffs[wavnum]+wavleng[wavnum];
		splc[chanum] = -(wavleng[wavnum]<<12);              //splc's modified last
		swavenum[chanum] = wavnum;
		chanstat[chanum] = followstat+1;
		frqeff[chanum] = 0; frqoff[chanum] = 0;
		voleff[chanum] = 0; voloff[chanum] = 0;
		paneff[chanum] = 0; panoff[chanum] = 0;
		return;
	}
}

getsndbufinfo(long *dasndoffsplc, long *dasndbufsiz)
{
	*dasndoffsplc = sndoffsplc;
	*dasndbufsiz = (sndbufsiz<<(bytespersample+numspeakers-2));
}

preparesndbuf()
{
	long i, j, k, voloffs1, voloffs2, *stempptr;
	long daswave, dasinc, dacnt;
	long ox, oy, x, y;
	char *sndptr, v1, v2;

	kdmintinprep++;
	if (kdminprep != 0) return;

	if ((digistat == 1) || (digistat == 2))
	{
		i = kinp(dmacheckport); i += (kinp(dmacheckport)<<8);
		if (i <= dmachecksiz)
		{
			i = ((i > 32) && (i <= (dmachecksiz>>1)+32));
			if ((sndoffsplc<(sndoffsplc^sndoffsxor)) == i) kdmintinprep++;
		}
	}

	kdminprep = 1;
	while (kdmintinprep > 0)
	{
		sndoffsplc ^= sndoffsxor;

		for (i=NUMCHANNELS-1;i>=0;i--)
			if ((splc[i] < 0) && (chanstat[i] > 0))
			{
				if (chanstat[i] == 1)
				{
					ox = xplc[i];
					oy = yplc[i];
				}
				else
				{
					stempptr = (long *)xplc[i]; ox = *stempptr;
					stempptr = (long *)yplc[i]; oy = *stempptr;
				}
				ox -= globposx; oy -= globposy;
				x = dmulscale28(oy,globxvect,-ox,globyvect);
				y = dmulscale28(ox,globxvect,oy,globyvect);

				if ((klabs(x) >= 32768) || (klabs(y) >= 32768))
					{ splc[i] = 0; continue; }

				j = vdist[i];
				vdist[i] = msqrtasm(x*x+y*y);
				if (j)
				{
					j = (sinc[i]<<10)/(min(max(vdist[i]-j,-768),768)+1024)-sinc[i];
					sincoffs[i] = ((sincoffs[i]*7+j)>>3);
				}

				voloffs1 = min((vol[i]<<22)/(((x+1536)*(x+1536)+y*y)+1),255);
				voloffs2 = min((vol[i]<<22)/(((x-1536)*(x-1536)+y*y)+1),255);

				if (numspeakers == 1)
					calcvolookupmono(FP_OFF(volookup)+(i<<(9+2)),-(voloffs1+voloffs2)<<6,(voloffs1+voloffs2)>>1);
				else
					calcvolookupstereo(FP_OFF(volookup)+(i<<(9+2)),-(voloffs1<<7),voloffs1,-(voloffs2<<7),voloffs2);
			}

		for(dacnt=0;dacnt<sndbufsiz;dacnt+=bytespertic)
		{
			if (musicstatus > 0)    //Gets here 120 times/second
			{
				while ((notecnt < numnotes) && (timecount >= nttime[notecnt]))
				{
					j = trinst[nttrack[notecnt]];
					k = mulscale24(frqtable[ntfreq[notecnt]],finetune[j]+748);

					if (ntvol1[notecnt] == 0)   //Note off
					{
						for(i=NUMCHANNELS-1;i>=0;i--)
							if (splc[i] < 0)
								if (swavenum[i] == j)
									if (sinc[i] == k)
										splc[i] = 0;
					}
					else                        //Note on
						startwave(j,k,ntvol1[notecnt],ntvol2[notecnt],ntfrqeff[notecnt],ntvoleff[notecnt],ntpaneff[notecnt]);

					notecnt++;
					if (notecnt >= numnotes)
						if (musicrepeat > 0)
							notecnt = 0, timecount = nttime[0];
				}
				timecount++;
			}

			for(i=NUMCHANNELS-1;i>=0;i--)
			{
				if (splc[i] >= 0) continue;

				dasinc = sinc[i]+sincoffs[i];

				if (frqeff[i] != 0)
				{
					dasinc = mulscale16(dasinc,eff[frqeff[i]-1][frqoff[i]]);
					frqoff[i]++; if (frqoff[i] >= 256) frqeff[i] = 0;
				}
				if ((voleff[i]) || (paneff[i]))
				{
					voloffs1 = svol1[i];
					voloffs2 = svol2[i];
					if (voleff[i])
					{
						voloffs1 = mulscale16(voloffs1,eff[voleff[i]-1][voloff[i]]);
						voloffs2 = mulscale16(voloffs2,eff[voleff[i]-1][voloff[i]]);
						voloff[i]++; if (voloff[i] >= 256) voleff[i] = 0;
					}

					if (numspeakers == 1)
						calcvolookupmono(FP_OFF(volookup)+(i<<(9+2)),-(voloffs1+voloffs2)<<6,(voloffs1+voloffs2)>>1);
					else
					{
						if (paneff[i])
						{
							voloffs1 = mulscale16(voloffs1,131072-eff[paneff[i]-1][panoff[i]]);
							voloffs2 = mulscale16(voloffs2,eff[paneff[i]-1][panoff[i]]);
							panoff[i]++; if (panoff[i] >= 256) paneff[i] = 0;
						}
						calcvolookupstereo(FP_OFF(volookup)+(i<<(9+2)),-(voloffs1<<7),voloffs1,-(voloffs2<<7),voloffs2);
					}
				}

				daswave = swavenum[i];
				voloffs1 = FP_OFF(volookup)+(i<<(9+2));

				kdmasm1 = repleng[daswave];
				kdmasm2 = wavoffs[daswave]+repstart[daswave]+repleng[daswave];
				kdmasm3 = (repleng[daswave]<<12); //repsplcoff
				kdmasm4 = soff[i];
				if (numspeakers == 1)
				{
					if (kdmqual == 0) splc[i] = monolocomb(0L,voloffs1,bytespertic,dasinc,splc[i],FP_OFF(stemp));
									 else splc[i] = monohicomb(0L,voloffs1,bytespertic,dasinc,splc[i],FP_OFF(stemp));
				}
				else
				{
					if (kdmqual == 0) splc[i] = stereolocomb(0L,voloffs1,bytespertic,dasinc,splc[i],FP_OFF(stemp));
									 else splc[i] = stereohicomb(0L,voloffs1,bytespertic,dasinc,splc[i],FP_OFF(stemp));
				}
				soff[i] = kdmasm4;

				if ((kdmqual == 0) || (splc[i] >= 0)) continue;
				if (numspeakers == 1)
				{
					if (kdmqual == 0) monolocomb(0L,voloffs1,samplerate>>11,dasinc,splc[i],FP_OFF(stemp)+(bytespertic<<2));
									 else monohicomb(0L,voloffs1,samplerate>>11,dasinc,splc[i],FP_OFF(stemp)+(bytespertic<<2));
				}
				else
				{
					if (kdmqual == 0) stereolocomb(0L,voloffs1,samplerate>>11,dasinc,splc[i],FP_OFF(stemp)+(bytespertic<<3));
									 else stereohicomb(0L,voloffs1,samplerate>>11,dasinc,splc[i],FP_OFF(stemp)+(bytespertic<<3));
				}
			}

			if (kdmqual)
			{
				if (numspeakers == 1)
				{
					for(i=(samplerate>>11)-1;i>=0;i--)
						stemp[i] += mulscale16(stemp[i+1024]-stemp[i],ramplookup[i]);
					j = bytespertic; k = (samplerate>>11);
					copybuf(&stemp[j],&stemp[1024],k);
					clearbuf(&stemp[j],k,32768);
				}
				else
				{
					for(i=(samplerate>>11)-1;i>=0;i--)
					{
						j = (i<<1);
						stemp[j+0] += mulscale16(stemp[j+1024]-stemp[j+0],ramplookup[i]);
						stemp[j+1] += mulscale16(stemp[j+1025]-stemp[j+1],ramplookup[i]);
					}
					j = (bytespertic<<1); k = ((samplerate>>11)<<1);
					copybuf(&stemp[j],&stemp[1024],k);
					clearbuf(&stemp[j],k,32768);
				}
			}

			if (numspeakers == 1)
			{
				if (bytespersample == 1)
				{
					if (digistat == 13) pcbound2char(bytespertic>>1,FP_OFF(stemp),sndoffsplc+dacnt);
										  else bound2char(bytespertic>>1,FP_OFF(stemp),sndoffsplc+dacnt);
				} else bound2short(bytespertic>>1,FP_OFF(stemp),sndoffsplc+(dacnt<<1));
			}
			else
			{
				if (bytespersample == 1) bound2char(bytespertic,FP_OFF(stemp),sndoffsplc+(dacnt<<1));
										 else bound2short(bytespertic,FP_OFF(stemp),sndoffsplc+(dacnt<<2));
			}
		}
		kdmintinprep--;
	}
	kdminprep = 0;
}

wsay(char *dafilename, long dafreq, long volume1, long volume2)
{
	unsigned char ch1, ch2;
	long i, j, bad;

	if (digistat == 0) return;

	i = numwaves-1;
	do
	{
		bad = 0;

		j = 0;
		while ((dafilename[j] > 0) && (j < 16))
		{
			ch1 = dafilename[j]; if ((ch1 >= 97) && (ch1 <= 123)) ch1 -= 32;
			ch2 = instname[i][j]; if ((ch2 >= 97) && (ch2 <= 123)) ch2 -= 32;
			if (ch1 != ch2) {bad = 1; break;}
			j++;
		}
		if (bad == 0)
		{
			startwave(i,(dafreq*11025)/samplerate,volume1,volume2,0L,0L,0L);
			return;
		}

		i--;
	} while (i >= 0);
}

loadwaves(char *wavename)
{
	long fil, i, j, dawaversionum;
	char filename[80];

	strcpy(filename,wavename);
	if (strstr(filename,".KWV") == 0) strcat(filename,".KWV");
	if ((fil = kopen4load(filename,0)) == -1)
		if (strcmp(filename,"WAVES.KWV") != 0)
		{
			strcpy(filename,"WAVES.KWV");
			fil = kopen4load(filename,0);
		}

	totsndbytes = 0;

	if (fil != -1)
	{
		if (strcmp(kwvname,filename) == 0) { kclose(fil); return; }
		strcpy(kwvname,filename);

		kread(fil,&dawaversionum,4);
		if (dawaversionum != 0) { kclose(fil); return; }

		kread(fil,&numwaves,4);
		for(i=0;i<numwaves;i++)
		{
			kread(fil,&instname[i][0],16);
			kread(fil,&wavleng[i],4);
			kread(fil,&repstart[i],4);
			kread(fil,&repleng[i],4);
			kread(fil,&finetune[i],4);
			wavoffs[i] = totsndbytes;
			totsndbytes += wavleng[i];
		}
	}
	else
	{
		dawaversionum = 0;
		numwaves = 0;
	}

	for(i=numwaves;i<MAXWAVES;i++)
	{
		for(j=0;j<16;j++) instname[i][j] = 0;
		wavoffs[i] = totsndbytes;
		wavleng[i] = 0L;
		repstart[i] = 0L;
		repleng[i] = 0L;
		finetune[i] = 0L;
	}

	if (snd == 0)
	{
		if ((snd = (char *)malloc(totsndbytes+2)) == 0)
			{ printf("Not enough memory for digital music!\n"); exit(0); }
	}
	for(i=0;i<MAXWAVES;i++) wavoffs[i] += FP_OFF(snd);
	if (fil != -1)
	{
		kread(fil,snd,totsndbytes);
		kclose(fil);
	}
	snd[totsndbytes] = snd[totsndbytes+1] = 128;
}

loadsong(char *filename)
{
	long i, fil;

	if (musistat != 1) return(0);
	musicoff();
	filename = strupr(filename);
	if (strstr(filename,".KDM") == 0) strcat(filename,".KDM");
	if ((fil = kopen4load(filename,0)) == -1)
	{
		printf("I cannot load %s.\n",filename);
		uninitsb();
		return(-1);
	}
	kread(fil,&kdmversionum,4);
	if (kdmversionum != 0) return(-2);
	kread(fil,&numnotes,4);
	kread(fil,&numtracks,4);
	kread(fil,trinst,numtracks);
	kread(fil,trquant,numtracks);
	kread(fil,trvol1,numtracks);
	kread(fil,trvol2,numtracks);
	kread(fil,nttime,numnotes<<2);
	kread(fil,nttrack,numnotes);
	kread(fil,ntfreq,numnotes);
	kread(fil,ntvol1,numnotes);
	kread(fil,ntvol2,numnotes);
	kread(fil,ntfrqeff,numnotes);
	kread(fil,ntvoleff,numnotes);
	kread(fil,ntpaneff,numnotes);
	kclose(fil);
	return(0);
}

musicon()
{
	if (musistat != 1)
		return;

	notecnt = 0;
	timecount = nttime[notecnt];
	musicrepeat = 1;
	musicstatus = 1;
}

musicoff()
{
	long i;

	musicstatus = 0;
	for(i=0;i<NUMCHANNELS;i++)
		splc[i] = 0;
	musicrepeat = 0;
	timecount = 0;
	notecnt = 0;
}

kdmconvalloc32 (long size)
{
	union REGS r;

	r.x.eax = 0x0100;           //DPMI allocate DOS memory
	r.x.ebx = ((size+15)>>4);   //Number of paragraphs requested
	int386(0x31,&r,&r);

	if (r.x.cflag != 0)         //Failed
		return ((long)0);
	return ((long)((r.x.eax&0xffff)<<4));   //Returns full 32-bit offset
}

installbikdmhandlers()
{
	union REGS r;
	struct SREGS sr;
	long lowp;
	void far *fh;

		//Get old protected mode handler
	r.x.eax = 0x3500+kdmvect;   /* DOS get vector (INT 0Ch) */
	sr.ds = sr.es = 0;
	int386x(0x21,&r,&r,&sr);
	kdmpsel = (unsigned short)sr.es;
	kdmpoff = r.x.ebx;

		//Get old real mode handler
	r.x.eax = 0x0200;   /* DPMI get real mode vector */
	r.h.bl = kdmvect;
	int386(0x31,&r,&r);
	kdmrseg = (unsigned short)r.x.ecx;
	kdmroff = (unsigned short)r.x.edx;


		//Allocate memory in low memory to store real mode handler
	if ((lowp = kdmconvalloc32(KDMCODEBYTES)) == 0)
	{
		printf("Can't allocate conventional memory.\n");
		exit;
	}
	memcpy((void *)lowp,(void *)pcrealbuffer,KDMCODEBYTES);

		//Set new protected mode handler
	r.x.eax = 0x2500+kdmvect;   /* DOS set vector (INT 0Ch) */
	fh = (void far *)pctimerhandler;
	r.x.edx = FP_OFF(fh);
	sr.ds = FP_SEG(fh);      //DS:EDX == &handler
	sr.es = 0;
	int386x(0x21,&r,&r,&sr);

		//Set new real mode handler (must be after setting protected mode)
	r.x.eax = 0x0201;
	r.h.bl = kdmvect;              //CX:DX == real mode &handler
	r.x.ecx = ((lowp>>4)&0xffff);  //D32realseg
	r.x.edx = 0L;                  //D32realoff
	int386(0x31,&r,&r);
}

uninstallbikdmhandlers()
{
	union REGS r;
	struct SREGS sr;

		//restore old protected mode handler
	r.x.eax = 0x2500+kdmvect;   /* DOS set vector (INT 0Ch) */
	r.x.edx = kdmpoff;
	sr.ds = kdmpsel;    /* DS:EDX == &handler */
	sr.es = 0;
	int386x(0x21,&r,&r,&sr);

		//restore old real mode handler
	r.x.eax = 0x0201;   /* DPMI set real mode vector */
	r.h.bl = kdmvect;
	r.x.ecx = (unsigned long)kdmrseg;     //CX:DX == real mode &handler
	r.x.edx = (unsigned long)kdmroff;
	int386(0x31,&r,&r);
}
