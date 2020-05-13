// Evil and Nasty Configuration File Reader for KenBuild
// Repurposes for JFTekWar
// by Jonathon Fowler

#include "compat.h"
#include "build.h"
#include "osd.h"
#include "tekwar.h"

#ifdef RENDERTYPEWIN
#include "winlayer.h"
#endif
#include "baselayer.h"

static int readconfig(BFILE *fp, const char *key, char *value, unsigned len)
{
	char buf[1000], *k, *v, *eq;
	int x=0;

	if (len < 1) return 0;

	Brewind(fp);

	while (1) {
		if (!Bfgets(buf, 1000, fp)) return 0;

		if (buf[0] == ';') continue;

		eq = Bstrchr(buf, '=');
		if (!eq) continue;

		k = buf;
		v = eq+1;

		while (*k == ' ' || *k == '\t') k++;
		*(eq--) = 0;
		while ((*eq == ' ' || *eq == '\t') && eq>=k) *(eq--) = 0;

		if (Bstrcasecmp(k, key)) continue;
		
		while (*v == ' ' || *k == '\t') v++;
		eq = v + Bstrlen(v)-1;

		while ((*eq == ' ' || *eq == '\t' || *eq == '\r' || *eq == '\n') && eq>=v) *(eq--) = 0;

		value[--len] = 0;
		do value[x] = v[x]; while (v[x++] != 0 && len-- > 0);

		return x-1;
	}
}


int loadsetup(const char *fn)
{
	BFILE *fp;
#define VL 32
	char val[VL];
	int i;

	if ((fp = Bfopen(fn, "rt")) == NULL) return -1;

	if (readconfig(fp, "forcesetup", val, VL) > 0) { forcesetup = (Batoi(val) != 0); }
	if (readconfig(fp, "fullscreen", val, VL) > 0) { fullscreen = (Batoi(val) != 0); }
	if (readconfig(fp, "xdim", val, VL) > 0) xdimgame = Batoi(val);
	if (readconfig(fp, "ydim", val, VL) > 0) ydimgame = Batoi(val);
	if (readconfig(fp, "bpp", val, VL) > 0) bppgame = Batoi(val);
	if (readconfig(fp, "mouse", val, VL) > 0) { option[3] = (Batoi(val) != 0); }
	if (readconfig(fp, "brightness", val, VL) > 0) brightness = min(max(Batoi(val),0),15);

#ifdef RENDERTYPEWIN
	if (readconfig(fp, "maxrefreshfreq", val, VL) > 0) maxrefreshfreq = Batoi(val);
#endif

	if (readconfig(fp, "keyforward", val, VL) > 0) keys[0] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keybackward", val, VL) > 0) keys[1] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyturnleft", val, VL) > 0) keys[2] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyturnright", val, VL) > 0) keys[3] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyrun", val, VL) > 0) keys[4] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keystrafe", val, VL) > 0) keys[5] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyfire", val, VL) > 0) keys[6] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyuse", val, VL) > 0) keys[7] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyjump", val, VL) > 0) keys[8] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keycrouch", val, VL) > 0) keys[9] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keylookup", val, VL) > 0) keys[10] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keylookdown", val, VL) > 0) keys[11] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keystrafeleft", val, VL) > 0) keys[12] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keystraferight", val, VL) > 0) keys[13] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keymapmode", val, VL) > 0) keys[14] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyviewcycle", val, VL) > 0) keys[15] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyexpandview", val, VL) > 0) keys[16] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyshrinkview", val, VL) > 0) keys[17] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keychat", val, VL) > 0) keys[18] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyautocenter", val, VL) > 0) keys[19] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyshowrearview", val, VL) > 0) keys[20] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyshowprepitem", val, VL) > 0) keys[21] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyshowhealth", val, VL) > 0) keys[22] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyshowcrosshair", val, VL) > 0) keys[23] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyshowtime", val, VL) > 0) keys[24] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyshowscore", val, VL) > 0) keys[25] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyshowinventory", val, VL) > 0) keys[26] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyconcealgun", val, VL) > 0) keys[27] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keymouselook", val, VL) > 0) keys[28] = Bstrtol(val, NULL, 16);
	if (readconfig(fp, "keyconsole", val, VL) > 0) { keys[29] = Bstrtol(val, NULL, 16); OSD_CaptureKey(keys[29]); }

	Bfclose(fp);

	return 0;
}

int writesetup(const char *fn)
{
	BFILE *fp;

	fp = Bfopen(fn,"wt");
	if (!fp) return -1;
	
	Bfprintf(fp,
	"; Always show configuration options on startup\n"
	";   0 - No\n"
	";   1 - Yes\n"
	"forcesetup = %d\n"
	"\n"
	"; Video mode selection\n"
	";   0 - Windowed\n"
	";   1 - Fullscreen\n"
	"fullscreen = %d\n"
	"\n"
	"; Video resolution\n"
	"xdim = %d\n"
	"ydim = %d\n"
	"\n"
	"; Colour depth\n"
	"bpp = %d\n"
	"\n"
#ifdef RENDERTYPEWIN
	"; Maximum OpenGL mode refresh rate (Windows only, in Hertz)\n"
	"maxrefreshfreq = %d\n"
	"\n"
#endif
	"; Brightness setting\n"
	";   0  - lowest\n"
	";   15 - highest\n"
	"brightness = %d\n"
	"\n"
	"; Enable mouse\n"
	";   0 - No\n"
	";   1 - Yes\n"
	"mouse = %d\n"
	"\n"
	"; Key Settings\n"
	";  Here's a map of all the keyboard scan codes: NOTE: values are listed in hex!\n"
	"; +---------------------------------------------------------------------------------------------+\n"
	"; | 01   3B  3C  3D  3E   3F  40  41  42   43  44  57  58          46                           |\n"
	"; |ESC   F1  F2  F3  F4   F5  F6  F7  F8   F9 F10 F11 F12        SCROLL                         |\n"
	"; |                                                                                             |\n"
	"; |29  02  03  04  05  06  07  08  09  0A  0B  0C  0D   0E     D2  C7  C9      45  B5  37  4A   |\n"
	"; | ` '1' '2' '3' '4' '5' '6' '7' '8' '9' '0'  -   =  BACK    INS HOME PGUP  NUMLK KP/ KP* KP-  |\n"
	"; |                                                                                             |\n"
	"; | 0F  10  11  12  13  14  15  16  17  18  19  1A  1B  2B     D3  CF  D1      47  48  49  4E   |\n"
	"; |TAB  Q   W   E   R   T   Y   U   I   O   P   [   ]    \\    DEL END PGDN    KP7 KP8 KP9 KP+   |\n"
	"; |                                                                                             |\n"
	"; | 3A   1E  1F  20  21  22  23  24  25  26  27  28     1C                     4B  4C  4D       |\n"
	"; |CAPS  A   S   D   F   G   H   J   K   L   ;   '   ENTER                    KP4 KP5 KP6    9C |\n"
	"; |                                                                                      KPENTER|\n"
	"; |  2A    2C  2D  2E  2F  30  31  32  33  34  35    36            C8          4F  50  51       |\n"
	"; |LSHIFT  Z   X   C   V   B   N   M   ,   .   /   RSHIFT          UP         KP1 KP2 KP3       |\n"
	"; |                                                                                             |\n"
	"; | 1D     38              39                  B8     9D       CB  D0   CD      52    53        |\n"
	"; |LCTRL  LALT           SPACE                RALT   RCTRL   LEFT DOWN RIGHT    KP0    KP.      |\n"
	"; +---------------------------------------------------------------------------------------------+\n"
	"\n"
	"keyforward = %X\n"
	"keybackward = %X\n"
	"keyturnleft = %X\n"
	"keyturnright = %X\n"
	"keyrun = %X\n"
	"keystrafe = %X\n"
	"keyfire = %X\n"
	"keyuse = %X\n"
	"keyjump = %X\n"
	"keycrouch = %X\n"
	"keylookup = %X\n"
	"keylookdown = %X\n"
	"keystrafeleft = %X\n"
	"keystraferight = %X\n"
	"keymapmode = %X\n"
	"keyviewcycle = %X\n"
	"keyexpandview = %X\n"
	"keyshrinkview = %X\n"
	"keychat = %X\n"
	"keyautocenter = %X\n"
	"keyshowrearview = %X\n"
	"keyshowprepitem = %X\n"
	"keyshowhealth = %X\n"
	"keyshowcrosshair = %X\n"
	"keyshowtime = %X\n"
	"keyshowscore = %X\n"
	"keyshowinventory = %X\n"
	"keyconcealgun = %X\n"
	"keymouselook = %X\n"
	"keyconsole = %X\n"
	"\n",
	
	forcesetup, fullscreen, xdimgame, ydimgame, bppgame,
#ifdef RENDERTYPEWIN
	maxrefreshfreq,
#endif
	brightness,
	option[3],
	keys[0],  keys[1],  keys[2],  keys[3],  keys[4],  keys[5],
	keys[6],  keys[7],  keys[8],  keys[9],  keys[10], keys[11],
	keys[12], keys[13], keys[14], keys[15], keys[16], keys[17],
	keys[18], keys[19], keys[20], keys[21], keys[22], keys[23],
	keys[24], keys[25], keys[26], keys[27], keys[28], keys[29]
		);

	Bfclose(fp);

	return 0;
}
