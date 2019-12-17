// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
// This file has been modified from Ken Silverman's original release

#include "compat.h"
#include "build.h"
#include "fx_man.h"
#include "renderlayer.h" // for win_gethwnd()

void initsb(char,char,int,char,char,char,char);
void uninitsb(void);
void setears(int,int,int,int);
void wsayfollow(char const *,int,int,int *,int *,char);
void wsay(char const *,int,int,int);
void loadwaves(char const *);
int loadsong(char const *);
void musicon(void);
void musicoff(void);
void refreshaudio(void) { };
void preparesndbuf();

static mutex_t m_callback;

#define NUMCHANNELS 16
#define MAXWAVES 256
#define MAXTRACKS 256
#define MAXNOTES 8192
#define MAXEFFECTS 16

    //Actual sound parameters after initsb was called
int samplerate, numspeakers, bytespersample, intspersec, kdmqual;

    //KWV wave variables
int numwaves;
char instname[MAXWAVES][16];
int wavleng[MAXWAVES];
int repstart[MAXWAVES], repleng[MAXWAVES];
int finetune[MAXWAVES];

    //Other useful wave variables
int totsndbytes, totsndmem;
intptr_t wavoffs[MAXWAVES];

    //Effects array
int eff[MAXEFFECTS][256];

    //KDM song variables:
int kdmversionum, numnotes, numtracks;
char trinst[MAXTRACKS], trquant[MAXTRACKS];
char trvol1[MAXTRACKS], trvol2[MAXTRACKS];
int nttime[MAXNOTES];
char nttrack[MAXNOTES], ntfreq[MAXNOTES];
char ntvol1[MAXNOTES], ntvol2[MAXNOTES];
char ntfrqeff[MAXNOTES], ntvoleff[MAXNOTES], ntpaneff[MAXNOTES];

    //Other useful song variables:
int timecount, notecnt, musicstatus, musicrepeat;

int kdmasm1, kdmasm3;
uint8_t *kdmasm2, *kdmasm4;

static char digistat = 0, musistat = 0;

char *snd = NULL, kwvname[20] = {""};

#define MAXBYTESPERTIC 1024+128
static int stemp[MAXBYTESPERTIC];

    //Sound reading information
int splc[NUMCHANNELS], sinc[NUMCHANNELS];
intptr_t soff[NUMCHANNELS];
int svol1[NUMCHANNELS], svol2[NUMCHANNELS];
int volookup[NUMCHANNELS<<9];
int swavenum[NUMCHANNELS];
int frqeff[NUMCHANNELS], frqoff[NUMCHANNELS];
int voleff[NUMCHANNELS], voloff[NUMCHANNELS];
int paneff[NUMCHANNELS], panoff[NUMCHANNELS];

static int globposx, globposy, globxvect, globyvect;
static intptr_t xplc[NUMCHANNELS], yplc[NUMCHANNELS];
static int vol[NUMCHANNELS];
static int vdist[NUMCHANNELS], sincoffs[NUMCHANNELS];
static char chanstat[NUMCHANNELS];

int frqtable[256];

int samplediv, oldtimerfreq, chainbackcnt, chainbackstart;
char *pcsndptr, pcsndlookup[256], bufferside;
int samplecount, pcsndbufsiz, pcrealmodeint;

#if 0
static short kdmvect = 0x8;
static unsigned short kdmpsel, kdmrseg, kdmroff;
static unsigned int kdmpoff;

#define KDMCODEBYTES 256
static char pcrealbuffer[KDMCODEBYTES] =        //See pckdm.asm
{
    0x50,0x53,0x51,0x52,0x32,0xC0,0xE6,0x42,0xB0,0x20,
    0xE6,0x20,0x5A,0x59,0x5B,0x58,0xCF,
};
#endif

static int bytespertic, sndsamplesleft;

char qualookup[512*16];
int ramplookup[64];

unsigned short sndseg = 0;

int monolocomb(int zero, int *vol, int cnt, int dat, int dasplc, int *stemp)
{
    UNREFERENCED_PARAMETER(zero);
    do
    {
        *stemp++ += vol[kdmasm4[dasplc>>12]];
        dasplc += dat;
        cnt--;
        if (dasplc >= 0 && cnt) // loop
        {
            if (!kdmasm1)
                break;
            kdmasm4 = kdmasm2;
            dasplc -= kdmasm3;
        }
    } while (cnt);
    return dasplc;
}

int monohicomb(int zero, int *vol, int cnt, int dat, int dasplc, int *stemp)
{
    UNREFERENCED_PARAMETER(zero);
    do
    {
        uint8_t s0 = kdmasm4[dasplc>>12];
        uint8_t s1 = kdmasm4[(dasplc>>12)+1];
        uint8_t si = qualookup[((dasplc<<1)&0x1e00)+((s0-s1)&0x1ff)]+s0;
        *stemp++ += vol[si];
        dasplc += dat;
        cnt--;
        if (dasplc >= 0 && cnt) // loop
        {
            if (!kdmasm1)
                break;
            kdmasm4 = kdmasm2;
            dasplc -= kdmasm3;
        }
    } while (cnt);
    return dasplc;
}

int stereolocomb(int zero, int *vol, int cnt, int dat, int dasplc, int *stemp)
{
    UNREFERENCED_PARAMETER(zero);
    do
    {
        *stemp++ += vol[kdmasm4[dasplc>>12]*2+0];
        *stemp++ += vol[kdmasm4[dasplc>>12]*2+1];
        dasplc += dat;
        cnt--;
        if (dasplc >= 0 && cnt) // loop
        {
            if (!kdmasm1)
                break;
            kdmasm4 = kdmasm2;
            dasplc -= kdmasm3;
        }
    } while (cnt);
    return dasplc;
}

int stereohicomb(int zero, int *vol, int cnt, int dat, int dasplc, int *stemp)
{
    UNREFERENCED_PARAMETER(zero);
    do
    {
        uint8_t s0 = kdmasm4[dasplc>>12];
        uint8_t s1 = kdmasm4[(dasplc>>12)+1];
        uint8_t si = qualookup[((dasplc<<1)&0x1e00)+((s0-s1)&0x1ff)]+s0;
        *stemp++ += vol[si*2+0];
        *stemp++ += vol[si*2+1];
        dasplc += dat;
        cnt--;
        if (dasplc >= 0 && cnt) // loop
        {
            if (!kdmasm1)
                break;
            kdmasm4 = kdmasm2;
            dasplc -= kdmasm3;
        }
    } while (cnt);
    return dasplc;
}

void bound2short(int bytespertic, int *stemp, short *shortptr)
{
    for (int i = 0; i < (bytespertic<<1); i++)
    {
        int j = stemp[i];
        stemp[i] = 32768;
        if (j < 0) j = 0;
        if (j > 65535) j = 65535;
        *shortptr++ = (short)(j^0x8000);
    }
}

void fsin(int *eax)
{
    *eax = (int)sin((*eax)*M_PI*(1.0/1024.0))*16384.0;
}

#if 0

#pragma aux fsin =\
	"fldpi",\
	"fimul dword ptr [eax]",\
	"fmul dword ptr [oneshr10]",\
	"fsin",\
	"fmul dword ptr [oneshl14]",\
	"fistp dword ptr [eax]",\
	parm [eax]\

#endif

void calcvolookupmono(int *edi, int eax, int ebx)
{
    for (int i = 0; i < 256; i++)
    {
        edi[i] = eax;
        eax += ebx;
    }
}

void calcvolookupstereo(int *edi, int eax, int ebx, int ecx, int edx)
{
    for (int i = 0; i < 256; i++)
    {
        edi[i*2+0] = eax;
        edi[i*2+1] = ecx;
        eax += ebx;
        ecx += edx;
    }
}

#if 0

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

#endif

void initsb(char dadigistat, char damusistat, int dasamplerate, char danumspeakers, char dabytespersample, char daintspersec, char daquality)
{
    int i, j;

    digistat = dadigistat;
    musistat = damusistat;

    if ((digistat == 0) && (musistat != 1))
        return;

#ifdef MIXERTYPEWIN
    void *initdata = (void *) win_gethwnd(); // used for DirectSound
#else
    void *initdata = NULL;
#endif

    if (FX_Init(1, danumspeakers, dasamplerate, initdata))
        return;

    MV_HookMusicRoutine(preparesndbuf);

    mutex_init(&m_callback);

    samplerate = dasamplerate;
    //if (samplerate < 6000) samplerate = 6000;
    //if (samplerate > 48000) samplerate = 48000;
    numspeakers = 2;//  danumspeakers;
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

    bytespertic = (((samplerate/120)+1)&~1);
    sndsamplesleft = 0;

    j = (((11025L*2093)/samplerate)<<13);
    for(i=1;i<76;i++)
    {
        frqtable[i] = j;
        j = mulscale30(j,1137589835);  //(pow(2,1/12)<<30) = 1137589835
    }
    for(i=0;i>=-14;i--) frqtable[i&255] = (frqtable[(i+12)&255]>>1);

    loadwaves("WAVES.KWV");

    timecount = notecnt = musicstatus = musicrepeat = 0;

    clearbuf(stemp,sizeof(stemp)>>2,32768L);
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

    musicoff();
}

void uninitsb()
{
    if ((digistat == 0) && (musistat != 1))
        return;

    if (snd != 0) free(snd), snd = 0;
}

static void startwave_internal(int wavnum, int dafreq, int davolume1, int davolume2, int dafrqeff, int davoleff, int dapaneff)
{
    int i, chanum;

    chanum = 0;
    for(i=NUMCHANNELS-1;i>0;i--)
        if (splc[i] > splc[chanum])
            chanum = i;

    splc[chanum] = 0;     //Disable channel temporarily for clean switch

    if (numspeakers == 1)
        calcvolookupmono(&volookup[chanum<<9],-(davolume1+davolume2)<<6,(davolume1+davolume2)>>1);
    else
        calcvolookupstereo(&volookup[chanum<<9],-(davolume1<<7),davolume1,-(davolume2<<7),davolume2);

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

static inline void startwave(int wavnum, int dafreq, int davolume1, int davolume2, int dafrqeff, int davoleff, int dapaneff)
{
    if ((davolume1|davolume2) == 0) return;

    mutex_lock(&m_callback);
    startwave_internal(wavnum, dafreq, davolume1, davolume2, dafrqeff, davoleff, dapaneff);
    mutex_unlock(&m_callback);
}

void setears(int daposx, int daposy, int daxvect, int dayvect)
{
    mutex_lock(&m_callback);
    globposx = daposx;
    globposy = daposy;
    globxvect = daxvect;
    globyvect = dayvect;
    mutex_unlock(&m_callback);
}

void wsayfollow(const char *dafilename, int dafreq, int davol, int *daxplc, int *dayplc, char followstat)
{
    char ch1, ch2, bad;
    int i, wavnum, chanum;

    if (digistat == 0) return;
    if (davol <= 0) return;

    mutex_lock(&m_callback);
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
            xplc[chanum] = ((intptr_t)daxplc);
            yplc[chanum] = ((intptr_t)dayplc);
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
        break;
    }
    mutex_unlock(&m_callback);
}

void preparesndbuf()
{
    int i, j, k, voloffs1, voloffs2, *stempptr;
    int *volptr;
    int daswave, dasinc;
    int ox, oy, x, y;

    auto routinebuf = MV_GetMusicRoutineBuffer();
    auto sndbuf = (short *)routinebuf.buffer;
    int32_t sndsamples = routinebuf.size / 4;

    mutex_lock(&m_callback);
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
                stempptr = (int *)xplc[i]; ox = *stempptr;
                stempptr = (int *)yplc[i]; oy = *stempptr;
            }
            ox -= globposx; oy -= globposy;
            x = dmulscale28(oy,globxvect,-ox,globyvect);
            y = dmulscale28(ox,globxvect,oy,globyvect);

            if ((klabs(x) >= 32768) || (klabs(y) >= 32768))
                { splc[i] = 0; continue; }

            j = vdist[i];
            vdist[i] = ksqrt(x*x+y*y);
            if (j)
            {
                j = (sinc[i]<<10)/(min(max(vdist[i]-j,-768),768)+1024)-sinc[i];
                sincoffs[i] = ((sincoffs[i]*7+j)>>3);
            }

            voloffs1 = min((vol[i]<<22)/(((x+1536)*(x+1536)+y*y)+1),255);
            voloffs2 = min((vol[i]<<22)/(((x-1536)*(x-1536)+y*y)+1),255);

            if (numspeakers == 1)
                calcvolookupmono(&volookup[i<<9],-(voloffs1+voloffs2)<<6,(voloffs1+voloffs2)>>1);
            else
                calcvolookupstereo(&volookup[i<<9],-(voloffs1<<7),voloffs1,-(voloffs2<<7),voloffs2);
        }


    int left = min(sndsamplesleft, sndsamples);
    if (left)
    {
        bound2short(left, &stemp[(bytespertic-sndsamplesleft)<<1], sndbuf);
        sndbuf += left<<1;
        sndsamples -= left;
        sndsamplesleft -= left;
    }
    while (sndsamples > 0)
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
                    startwave_internal(j,k,ntvol1[notecnt],ntvol2[notecnt],ntfrqeff[notecnt],ntvoleff[notecnt],ntpaneff[notecnt]);

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
                    calcvolookupmono(&volookup[i<<9],-(voloffs1+voloffs2)<<6,(voloffs1+voloffs2)>>1);
                else
                {
                    if (paneff[i])
                    {
                        voloffs1 = mulscale16(voloffs1,131072-eff[paneff[i]-1][panoff[i]]);
                        voloffs2 = mulscale16(voloffs2,eff[paneff[i]-1][panoff[i]]);
                        panoff[i]++; if (panoff[i] >= 256) paneff[i] = 0;
                    }
                    calcvolookupstereo(&volookup[i<<9],-(voloffs1<<7),voloffs1,-(voloffs2<<7),voloffs2);
                }
            }

            daswave = swavenum[i];
            volptr = &volookup[i<<9];

            kdmasm1 = repleng[daswave];
            kdmasm2 = (uint8_t*)wavoffs[daswave]+repstart[daswave]+repleng[daswave];
            kdmasm3 = (repleng[daswave]<<12); //repsplcoff
            kdmasm4 = (uint8_t*)soff[i];
            if (numspeakers == 1)
            {
                if (kdmqual == 0) splc[i] = monolocomb(0L,volptr,bytespertic,dasinc,splc[i],stemp);
                                    else splc[i] = monohicomb(0L,volptr,bytespertic,dasinc,splc[i],stemp);
            }
            else
            {
                if (kdmqual == 0) splc[i] = stereolocomb(0L,volptr,bytespertic,dasinc,splc[i],stemp);
                                    else splc[i] = stereohicomb(0L,volptr,bytespertic,dasinc,splc[i],stemp);
            }
            soff[i] = (intptr_t)kdmasm4;

            if ((kdmqual == 0) || (splc[i] >= 0)) continue;
            if (numspeakers == 1)
            {
                if (kdmqual == 0) monolocomb(0L,volptr,samplerate>>11,dasinc,splc[i],&stemp[bytespertic]);
                                    else monohicomb(0L,volptr,samplerate>>11,dasinc,splc[i],&stemp[bytespertic]);
            }
            else
            {
                if (kdmqual == 0) stereolocomb(0L,volptr,samplerate>>11,dasinc,splc[i],&stemp[bytespertic<<1]);
                                    else stereohicomb(0L,volptr,samplerate>>11,dasinc,splc[i],&stemp[bytespertic<<1]);
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

        sndsamplesleft = bytespertic;
        left = min(sndsamplesleft, sndsamples);
        bound2short(left, stemp, sndbuf);
        sndbuf += left<<1;
        sndsamples -= left;
        sndsamplesleft -= left;
    }
    mutex_unlock(&m_callback);
}

void wsay(const char *dafilename, int dafreq, int volume1, int volume2)
{
    unsigned char ch1, ch2;
    int i, j, bad;

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

void loadwaves(const char *wavename)
{
    int fil, i, j, dawaversionum;
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
    for(i=0;i<MAXWAVES;i++) wavoffs[i] += (intptr_t)snd;
    if (fil != -1)
    {
        kread(fil,snd,totsndbytes);
        kclose(fil);
    }
    snd[totsndbytes] = snd[totsndbytes+1] = 128;
}

int loadsong(const char *_filename)
{
    int fil;
    char filename[256];

    Bstrcpy(filename, _filename);

    if (musistat != 1) return(0);
    musicoff();
    Bstrupr(filename);
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

void musicon()
{
    if (musistat != 1)
        return;

    mutex_lock(&m_callback);
    notecnt = 0;
    timecount = nttime[notecnt];
    musicrepeat = 1;
    musicstatus = 1;
    mutex_unlock(&m_callback);
}

void musicoff()
{
    int i;

    mutex_lock(&m_callback);
    musicstatus = 0;
    for(i=0;i<NUMCHANNELS;i++)
        splc[i] = 0;
    musicrepeat = 0;
    timecount = 0;
    notecnt = 0;
    mutex_unlock(&m_callback);
}
