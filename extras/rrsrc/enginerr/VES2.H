// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
// This file has been modified from Ken Silverman's original release

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>

#pragma pack(push,1);

typedef struct
{
	char VESASignature[4];
	short VESAVersion;
	long OemStringPtr, Capabilities, VideoModePtr;
	short TotalMemory, OemSoftwareRev;
	long OemVendorNamePtr, OemProductNamePtr, OemProductRevPtr;
	char reserved[222], OemDATA[256];
} VBE_vgaInfo;

typedef struct
{
	short ModeAttributes;
	char WinAAtributes, WinBAttributes;
	short WinGranularity, WinSize, WinASegment, WinBSegment;
	long WinFuncPtr;
	short BytesPerScanLine, XResolution, YResolution;
	char XCharSize, YCharSize, NumberOfPlanes, BitsPerPixel;
	char NumberOfBanks, MemoryModel, BankSize, NumberOfImagePages;
	char res1;
	char RedMaskSize, RedFieldPosition;
	char GreenMaskSize, GreenFieldPosition;
	char BlueMaskSize, BlueFieldPosition;
	char RsvdMaskSize, RsvdFieldPosition, DirectColorModeInfo;
	long PhysBasePtr, OffScreenMemOffset;
	short OffScreenMemSize;
	char res2[206];
} VBE_modeInfo;

struct _RMWORDREGS { unsigned short ax, bx, cx, dx, si, di, cflag; };
struct _RMBYTEREGS { unsigned char al, ah, bl, bh, cl, ch, dl, dh; };
typedef union { struct _RMWORDREGS x; struct _RMBYTEREGS h; } RMREGS;
typedef struct { unsigned short es, cs, ss, ds; } RMSREGS;

typedef struct
{
	long edi, esi, ebp, reserved, ebx, edx, ecx, eax;
	short flags, es, ds, fs, gs, ip, cs, sp, ss;
} _RMREGS;

#pragma pack(pop);

long VESABuf_len = 1024, VESABuf_sel = 0, VESABuf_rseg;
short modelist[256];
static char *screen = NULL, vesachecked = 0;
long xres, yres, bytesperline, frameplace, imageSize, maxpages;
long buffermode, origbuffermode, linearmode;
long setactiveentry = 0, setvisualentry = 0, setpaletteentry = 0;
char permanentupdate = 0, vgacompatible;
short visualpagelookup[64][2];
long activepagelookup[64];
static long ves2lastx[MAXYDIM];
static long davesapageshift;
static VBE_vgaInfo vgaInfo;
long globlinplace;

#pragma aux qlimitrate =\
	"mov dx, 0x3da",\
	"wait1: in al, dx",\
	"test al, 1",\
	"jnz wait1",\
	modify exact [eax edx]\

short ods, oes, oss;
#pragma aux backupsegs =\
	"mov ods, ds",\
	"mov oes, es",\
	"mov oss, ss",\
	modify [eax]\

#pragma aux restoresegs =\
	"mov ds, ods",\
	"mov es, oes",\
	"mov ss, oss",\
	modify [eax]\

#pragma aux setvmode =\
	"int 0x10",\
	parm [eax]\

#pragma aux copybufbyte =\
	"cmp ecx, 4",\
	"jae longcopy",\
	"test cl, 1",\
	"jz preskip",\
	"movsb",\
	"preskip: shr ecx, 1",\
	"rep movsw",\
	"jmp endit",\
	"longcopy: test edi, 1",\
	"jz skip1",\
	"movsb",\
	"dec ecx",\
	"skip1: test edi, 2",\
	"jz skip2",\
	"movsw",\
	"sub ecx, 2",\
	"skip2: mov ebx, ecx",\
	"shr ecx, 2",\
	"rep movsd",\
	"test bl, 2",\
	"jz skip3",\
	"movsw",\
	"skip3: test bl, 1",\
	"jz endit",\
	"movsb",\
	"endit:",\
	parm [esi][edi][ecx]\
	modify [ebx]\

/*   //64-bit copybufbyte
#pragma aux copybufbyte =\
	"cmp ecx, 8",\
	"jae longcopy",\
	"test cl, 1",\
	"jz shortskip1",\
	"movsb",\
	"shortskip1: shr ecx, 2",\
	"jnc shortskip2",\
	"movsw",\
	"shortskip2: rep movsd",\
	"jmp endit",\
	"longcopy: test edi, 1",\
	"jz skip1",\
	"movsb",\
	"dec ecx",\
	"skip1: test edi, 2",\
	"jz skip2",\
	"movsw",\
	"sub ecx, 2",\
	"skip2: test edi, 4",\
	"jz skip3",\
	"movsd",\
	"sub ecx, 4",\
	"skip3: mov ebx, ecx",\
	"shr ecx, 3",\
	"jz skip4",\
	"begloop: fld qword ptr [esi]",\
	"fstp qword ptr [edi]",\
	"add esi, 8",\
	"add edi, 8",\
	"dec ecx",\
	"jnz begloop",\
	"skip4: test bl, 4",\
	"jz skip5",\
	"movsd",\
	"skip5: test bl, 2",\
	"jz skip6",\
	"movsw",\
	"skip6: test bl, 1",\
	"jz endit",\
	"movsb",\
	"endit:",\
	parm [esi][edi][ecx]\
	modify [ebx]\
*/

#pragma aux vesasetactive =\
	"call dword ptr [setactiveentry]",\
	parm [eax][ebx][ecx][edx]\

#pragma aux vesasetvisual =\
	"call dword ptr [setvisualentry]",\
	parm [eax][ebx][ecx][edx]\

#pragma aux vesasetpalette =\
	"call dword ptr [setpaletteentry]",\
	parm [eax][ebx][ecx][edx][esi][edi]\

long DPMI_int86(long intno, RMREGS *in, RMREGS *out)
{
	_RMREGS rmregs;
	union REGS r;
	struct SREGS sr;

	memset(&rmregs,0,sizeof(rmregs));
	rmregs.eax = in->x.ax; rmregs.ebx = in->x.bx;
	rmregs.ecx = in->x.cx; rmregs.edx = in->x.dx;
	rmregs.esi = in->x.si; rmregs.edi = in->x.di;

	segread(&sr);
	r.w.ax = 0x300; r.h.bl = intno; r.h.bh = 0; r.w.cx = 0; sr.es = sr.ds;
	r.x.edi = (unsigned)&rmregs;
	backupsegs(); int386x(0x31,&r,&r,&sr); restoresegs();

	out->x.ax = rmregs.eax; out->x.bx = rmregs.ebx;
	out->x.cx = rmregs.ecx; out->x.dx = rmregs.edx;
	out->x.si = rmregs.esi; out->x.di = rmregs.edi;
	out->x.cflag = rmregs.flags&1;
	return(out->x.ax);
}

long DPMI_int86x(long intno, RMREGS *in, RMREGS *out, RMSREGS *sregs)
{
	_RMREGS rmregs;
	union REGS r;
	struct SREGS sr;

	memset(&rmregs, 0, sizeof(rmregs));
	rmregs.eax = in->x.ax; rmregs.ebx = in->x.bx;
	rmregs.ecx = in->x.cx; rmregs.edx = in->x.dx;
	rmregs.esi = in->x.si; rmregs.edi = in->x.di;
	rmregs.es = sregs->es;
	rmregs.ds = sregs->ds;

	segread(&sr);
	r.w.ax = 0x300; r.h.bl = intno; r.h.bh = 0; r.w.cx = 0; sr.es = sr.ds;
	r.x.edi = (unsigned)&rmregs;
	backupsegs(); int386x(0x31,&r,&r,&sr); restoresegs();

	out->x.ax = rmregs.eax; out->x.bx = rmregs.ebx;
	out->x.cx = rmregs.ecx; out->x.dx = rmregs.edx;
	out->x.si = rmregs.esi; out->x.di = rmregs.edi;
	sregs->es = rmregs.es; sregs->cs = rmregs.cs;
	sregs->ss = rmregs.ss; sregs->ds = rmregs.ds;
	out->x.cflag = rmregs.flags&1;
	return(out->x.ax);
}

static void ExitVBEBuf(void)
{
	union REGS r;
	r.w.ax = 0x101; r.w.dx = VESABuf_sel; //DPMI free real seg
	backupsegs(); int386(0x31,&r,&r); restoresegs();
}

void VBE_callESDI(RMREGS *regs, void *buffer, long size)
{
	RMSREGS sregs;
	union REGS r;

	if (!VESABuf_sel)  //Init Real mode buffer
	{
		r.w.ax = 0x100; r.w.bx = (VESABuf_len+15)>>4;
		backupsegs(); int386(0x31,&r,&r); restoresegs();
		if (r.w.cflag) { printf("DPMI_allocRealSeg failed!\n"); exit(0); }
		VESABuf_sel = r.w.dx;
		VESABuf_rseg = r.w.ax;
		atexit(ExitVBEBuf);
	}
	sregs.es = VESABuf_rseg;
	regs->x.di = 0;
	_fmemcpy(MK_FP(VESABuf_sel,0),buffer,size);
	DPMI_int86x(0x10,regs,regs,&sregs);
	_fmemcpy(buffer,MK_FP(VESABuf_sel,0),size);
}

long VBE_getModeInfo(long mode, VBE_modeInfo *modeInfo)
{
	RMREGS regs;

	regs.x.ax = 0x4f01;
	regs.x.cx = mode;
	VBE_callESDI(&regs, modeInfo, sizeof(VBE_modeInfo));
	if (regs.x.ax != 0x004f) return(0);
	if ((modeInfo->ModeAttributes&1) == 0) return(0); //1 is vbeMdAvailable
	return(1);
}

GetPtrToLFB(long physAddr)
{
	#define LIMIT (4096*1024)-1
	long sel;
	union REGS r;

	r.w.ax = 0; r.w.cx = 1;
	backupsegs(); int386(0x31,&r,&r); restoresegs();
	if (r.x.cflag) { printf("DPMI_allocSelector() failed!\n"); exit(0); }
	sel = r.w.ax;
	r.w.ax = 9; r.w.bx = sel; r.w.cx = 0x8092;
	backupsegs(); int386(0x31,&r,&r); restoresegs();

	r.w.ax = 0x800; r.w.bx = physAddr >> 16; r.w.cx = physAddr & 0xffff;
	r.w.si = LIMIT>>16; r.w.di = LIMIT&0xffff;
	backupsegs(); int386(0x31,&r,&r); restoresegs();
	if (r.x.cflag) { printf("DPMI_mapPhysicalToLinear() failed!\n"); exit(0); }
	globlinplace = ((long)r.w.bx<<16)+r.w.cx;

	r.w.ax = 7; r.w.bx = sel;
	r.w.cx = globlinplace>>16; r.w.dx = globlinplace&0xffff;
	backupsegs(); int386(0x31,&r,&r); restoresegs();
	if (r.x.cflag) { printf("DPMI_setSelectorBase() failed!\n"); exit(0); }

	r.w.ax = 8; r.w.bx = sel;
	r.w.cx = LIMIT>>16; r.w.dx = LIMIT&0xffff;
	backupsegs(); int386(0x31,&r,&r); restoresegs();
	if (r.x.cflag) { printf("DPMI_setSelectorLimit() failed!\n"); exit(0); }
}

getvalidvesamodes()
{
	long i, j, k;
	short *p, *p2;
	VBE_modeInfo modeInfo;
	RMREGS regs;

	if (vesachecked) return;
	vesachecked = 1;

	validmodecnt = 0;
	modelist[0] = -1;

	strncpy(vgaInfo.VESASignature,"VBE2",4);
	regs.x.ax = 0x4f00;
	VBE_callESDI(&regs,&vgaInfo,sizeof(VBE_vgaInfo));
	if ((regs.x.ax != 0x004f) || (strncmp(vgaInfo.VESASignature,"VESA",4)))
		return;

	//if (vgaInfo.VESAVersion < 0x200) return;

		//LfbMapRealPointer
	p = (short *)(((vgaInfo.VideoModePtr&0xffff0000)>>12)+((vgaInfo.VideoModePtr)&0xffff));
	p2 = modelist;
	while (*p != -1) *p2++ = *p++;
	*p2 = -1;

	for(p=modelist;*p!=-1;p++)
	{
		regs.x.ax = 0x4f01; regs.x.cx = *p;
		VBE_callESDI(&regs,&modeInfo,sizeof(VBE_modeInfo));
		if (regs.x.ax != 0x004f) continue;
		if (!(modeInfo.ModeAttributes&1)) continue; //1 is vbeMdAvailable
		if (modeInfo.MemoryModel != 4) continue;    //4 is vbeMemPK
		if (modeInfo.BitsPerPixel != 8) continue;
		if (modeInfo.NumberOfPlanes != 1) continue;

		validmode[validmodecnt] = *p;
		validmodexdim[validmodecnt] = modeInfo.XResolution;
		validmodeydim[validmodecnt] = modeInfo.YResolution;
		validmodecnt++;
	}

	for(i=1;i<validmodecnt;i++)
		for(j=0;j<i;j++)
			if (validmodeydim[i] < validmodeydim[j])
			{
				k = validmode[i]; validmode[i] = validmode[j]; validmode[j] = k;
				k = validmodexdim[i]; validmodexdim[i] = validmodexdim[j]; validmodexdim[j] = k;
				k = validmodeydim[i]; validmodeydim[i] = validmodeydim[j]; validmodeydim[j] = k;
			}

	for(i=1;i<validmodecnt;i++)
		for(j=0;j<i;j++)
			if (validmodexdim[i] < validmodexdim[j])
			{
				k = validmode[i]; validmode[i] = validmode[j]; validmode[j] = k;
				k = validmodexdim[i]; validmodexdim[i] = validmodexdim[j]; validmodexdim[j] = k;
				k = validmodeydim[i]; validmodeydim[i] = validmodeydim[j]; validmodeydim[j] = k;
			}
}

setvesa(long x, long y)
{
	div_t dived;
	long i, j, k;
	short *p, *p1, *p2;
	VBE_modeInfo modeInfo;
	RMREGS regs;
	RMSREGS sregs;

	getvalidvesamodes();

	xres = x; yres = y;
	for(k=0;k<validmodecnt;k++)
	{
		regs.x.ax = 0x4f01; regs.x.cx = validmode[k];
		VBE_callESDI(&regs,&modeInfo,sizeof(VBE_modeInfo));
		if (regs.x.ax != 0x004f) continue;
		if (!(modeInfo.ModeAttributes&1)) continue; //1 is vbeMdAvailable

		if (modeInfo.MemoryModel != 4) continue;    //4 is vbeMemPK
		if (modeInfo.BitsPerPixel != 8) continue;
		if (modeInfo.NumberOfPlanes != 1) continue;
		if (modeInfo.XResolution != x) continue;
		if (modeInfo.YResolution != y) continue;

		bytesperline = modeInfo.BytesPerScanLine;
		maxpages = min(modeInfo.NumberOfImagePages+1,64);

		regs.x.ax = 0x4f02;
		regs.x.bx = validmode[k] | ((modeInfo.ModeAttributes&128)<<7);
		DPMI_int86(0x10,&regs,&regs);

		if (modeInfo.ModeAttributes&32) vgacompatible = 0;
											else vgacompatible = 1;

		regs.x.ax = 0x4f0a; regs.x.bx = 0;
		DPMI_int86x(0x10,&regs,&regs,&sregs);
		if (regs.x.ax == 0x004f)   //cx is length of protected mode code
		{
			i = (((long)sregs.es)<<4)+((long)regs.x.di);
			p1 = (short *)i;
			setactiveentry = ((long)p1[0]) + i;
			setvisualentry = ((long)p1[1]) + i;
			setpaletteentry = ((long)p1[2]) + i;
				//p1[2] is useless palette function - see vesprot.asm for code
		}

			//Linear mode
		if (modeInfo.ModeAttributes&128) //128 is vbeMdLinear
		{
			GetPtrToLFB(modeInfo.PhysBasePtr);

			linearmode = 1;
			buffermode = (maxpages<=1);
			imageSize = bytesperline*yres;
			frameplace = globlinplace;
			j = 0;
			for(i=0;i<maxpages;i++)
			{
				activepagelookup[i] = globlinplace+j;
				j += imageSize;
			}
		}
		else  //Banked mode
		{
				//Get granularity
			switch(modeInfo.WinGranularity)
			{
				case 64: davesapageshift = 0; break;
				case 32: davesapageshift = 1; break;
				case 16: davesapageshift = 2; break;
				case 8: davesapageshift = 3; break;
				case 4: davesapageshift = 4; break;
				case 2: davesapageshift = 5; break;
				case 1: davesapageshift = 6; break;
			}

			linearmode = 0;
			if ((x == 320) && (y == 200) && (maxpages >= 2))
			{
				buffermode = 0;
				imageSize = 65536;
				frameplace = 0xa0000;
			}
			else
			{
				buffermode = 1;
				imageSize = bytesperline*yres;
				frameplace = FP_OFF(screen);
				maxpages = 1;
			}
		}

		origbuffermode = buffermode;

		j = 0;
		for(i=0;i<maxpages;i++)
		{
			dived = div(j,bytesperline);
			visualpagelookup[i][0] = (short)dived.rem;
			visualpagelookup[i][1] = (short)dived.quot;
			j += imageSize;
		}
		return(0);
	}
	return(-1);
}

setdacbits(long newdacbits)
{
	RMREGS regs;

	if ((vgaInfo.Capabilities&1) == 0) return(6L);
	regs.x.ax = 0x4f08;
	regs.x.bx = (((long)newdacbits)<<8);
	DPMI_int86(0x10,&regs,&regs);
	return((long)regs.h.bh);
}

#define setvesapage(i)                                      \
{                                                           \
	if (setactiveentry)                                      \
		vesasetactive(0x4f05,0L,0L,i);                        \
	else                                                     \
	{                                                        \
		regs.x.ax = 0x4f05; regs.x.bx = 0; regs.x.dx = i;     \
		DPMI_int86(0x10,&regs,&regs);                         \
	}                                                        \
}

void setactivepage(long dapagenum)
{
	RMREGS regs;

	if ((origbuffermode == 0) && (linearmode == 0))
	{
		frameplace = 0xa0000;
		setvesapage(dapagenum<<davesapageshift);
	}
	if (buffermode != 0) { frameplace = FP_OFF(screen); return; }
	if (linearmode != 0) { frameplace = activepagelookup[dapagenum]; return; }
}

static long curpag = 0;
void setvisualpage(long dapagenum)
{
	RMREGS regs;
	long i, j, p, dx, cx1, cy1, cx2, cy2, delta, x, y, y1, y2, ny1, ny2;

	if (buffermode != 0)
	{
		cx1 = windowx1; cy1 = windowy1; cx2 = windowx2; cy2 = windowy2;
		if (permanentupdate) { cx1 = 0; cy1 = 0; cx2 = xdim-1; cy2 = ydim-1; }
		dx = cx2-cx1+1;
		if (linearmode)
		{
			p = FP_OFF(screen);
			delta = activepagelookup[dapagenum&0x7fffffff]-FP_OFF(screen);
			if (permanentupdate == 0)
			{
				y1 = startumost[cx1]; y2 = y1;
				for(x=cx1;x<=cx2;x++)
				{
					ny1 = startumost[x]-1; ny2 = startdmost[x];
					if (ny1 < ny2-1)
					{
						if (ny1 >= y2)
						{
							while (y1 < y2-1)
							{
								y1++; if ((y1&31) == 0) faketimerhandler();
									//x,y1
								i = p+ylookup[y1]+ves2lastx[y1];
								copybufbyte(i,i+delta,x-ves2lastx[y1]);
							}
							y1 = ny1;
						}
						else
						{
							while (y1 < ny1)
							{
								y1++; if ((y1&31) == 0) faketimerhandler();
									//x-1,y1
								i = p+ylookup[y1]+ves2lastx[y1];
								copybufbyte(i,i+delta,x-ves2lastx[y1]);
							}
							while (y1 > ny1) ves2lastx[y1--] = x;
						}
						while (y2 > ny2)
						{
							y2--; if ((y2&31) == 0) faketimerhandler();
								//x-1,y2
							i = p+ylookup[y2]+ves2lastx[y2];
							copybufbyte(i,i+delta,x-ves2lastx[y2]);
						}
						while (y2 < ny2) ves2lastx[y2++] = x;
					}
					else
					{
						while (y1 < y2-1)
						{
							y1++; if ((y1&31) == 0) faketimerhandler();
								//x-1,y1
							i = p+ylookup[y1]+ves2lastx[y1];
							copybufbyte(i,i+delta,x-ves2lastx[y1]);
						}
						if (x == cx2) break;
						y1 = startumost[x+1]; y2 = y1;
					}
				}
				while (y1 < y2-1)
				{
					y1++; if ((y1&31) == 0) faketimerhandler();
						//cx2+1,y1
					i = p+ylookup[y1]+ves2lastx[y1];
					copybufbyte(i,i+delta,cx2+1-ves2lastx[y1]);
				}
			}
			else
			{
				p += ylookup[cy1]+cx1;
				delta = activepagelookup[dapagenum&0x7fffffff]-FP_OFF(screen);
				for(y=cy1;y<=cy2;y++)
				{
					copybufbyte(p,p+delta,dx);
					p += ylookup[1];
					if ((y&31) == 0) faketimerhandler();
				}
			}
		}
		else
		{
			p = ylookup[cy1]+cx1;
			for(y=cy1;y<=cy2;y++)
			{
				if ((p>>16) != curpag)
				{
					curpag = (p>>16);

					setvesapage(curpag<<davesapageshift);
					faketimerhandler();
				}

				i = (p&65535)+dx-65536;
				if (i <= 0)
					copybufbyte(p+FP_OFF(screen),(p&65535)+0xa0000,dx);
				else
				{
					copybufbyte(p+FP_OFF(screen),0xb0000-(dx-i),dx-i);

					curpag = ((p+dx-1)>>16);
					setvesapage(curpag<<davesapageshift);
					faketimerhandler();

					copybufbyte(p+(dx-i)+FP_OFF(screen),0xa0000,i);
				}
				p += ylookup[1];
			}
		}
		permanentupdate = 0;
	}
	if (origbuffermode == 0)
	{
		if (setvisualentry)
		{
			i = imageSize*(dapagenum&0x7fffffff);
			if (vgacompatible)
			{
				if (dapagenum >= 0) qlimitrate();
				vesasetvisual(0x4f07,0L,i>>2,i>>18);
			}
			else
				{ vesasetvisual(0x4f07,0x80,i>>2,i>>18); }

		}
		else
		{
			regs.x.ax = 0x4f07;
			regs.x.cx = visualpagelookup[dapagenum&0x7fffffff][0]; //X-coordinate
			regs.x.dx = visualpagelookup[dapagenum&0x7fffffff][1]; //Y-coordinate
			if (vgacompatible)
			{
				regs.x.bx = 0;
				if (dapagenum >= 0) qlimitrate();
				DPMI_int86(0x10,&regs,&regs);
			}
			else
				{ regs.x.bx = 0x80; DPMI_int86(0x10,&regs,&regs); }
		}
		if (dapagenum >= 0) faketimerhandler();
	}
}

uninitvesa()
{
	VESABuf_len = 1024;
	VESABuf_sel = 0;
	vesachecked = 0;
}

#pragma aux setpalettequick =\
	"mov edx, 0x3c8",\
	"out dx, al",\
	"inc edx",\
	"lea ecx, [ecx+ecx*2]",\
	"cld",\
	"rep outsb",\
	parm [eax][ecx][esi]\
	modify exact [ecx edx esi]\

VBE_setPalette(long start, long num, char *dapal)
{
	RMREGS regs;
	long i, j, k;
	char palquick[768];

	if ((vgacompatible) || (vgaInfo.VESAVersion < 0x200) || (vidoption != 1))
	{
		j = 0; k = (start<<2);
		for(i=0;i<num;i++)
		{
			palquick[j] = dapal[k+2];
			palquick[j+1] = dapal[k+1];
			palquick[j+2] = dapal[k];
			j += 3; k += 4;
		}
		setpalettequick(start,num,palquick);
		return(1);
	}

	if (setpaletteentry)
	{
		i = (vesasetpalette(0x4f09,(vgaInfo.Capabilities&4)<<5,
								  num,start,0L,dapal)&65535);
	}
	else
	{
		regs.x.ax = 0x4f09; regs.h.bl = ((vgaInfo.Capabilities&4)<<5);
		regs.x.cx = num; regs.x.dx = start;
		VBE_callESDI(&regs,dapal,sizeof(dapal)*num);
		i = regs.x.ax;
	}
	if (i != 0x004f) return(0);
	return(1);
}

VBE_getPalette(long start, long num, char *dapal)
{
	RMREGS regs;
	long i;

	if ((vgacompatible) || (vgaInfo.VESAVersion < 0x200) || (vidoption != 1))
	{
		koutp(0x3c7,start);
		for(i=num;i>0;i--)
		{
			dapal[2] = kinp(0x3c9);
			dapal[1] = kinp(0x3c9);
			dapal[0] = kinp(0x3c9);
			dapal += 4;
		}
		return(1);
	}

	regs.x.ax = 0x4f09; regs.h.bl = 1;
	regs.x.cx = num; regs.x.dx = start;
	VBE_callESDI(&regs,dapal,sizeof(dapal)*num);
	i = regs.x.ax;
	if (i != 0x004f) return(0);
	return(1);
}
