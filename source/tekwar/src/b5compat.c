#include "build.h"

void permanentwritesprite(int thex, int they, short tilenum, signed char shade,
        int cx1, int cy1, int cx2, int cy2, unsigned char dapalnum) {
    rotatesprite(thex<<16,they<<16,65536L,0,tilenum,shade,
                 dapalnum,8+16,cx1,cy1,cx2,cy2);
}

void permanentwritespritetile(int UNUSED(thex), int UNUSED(they), short tilenum, signed char shade,
        int cx1, int cy1, int cx2, int cy2, unsigned char dapalnum) {
    int x, y, xsiz, ysiz, tx1, ty1, tx2, ty2;
    
    xsiz = tilesizx[tilenum]; tx1 = cx1/xsiz; tx2 = cx2/xsiz;
    ysiz = tilesizy[tilenum]; ty1 = cy1/ysiz; ty2 = cy2/ysiz;
    
    for (x=tx1;x<=tx2;x++) {
        for (y=ty1;y<=ty2;y++) {
            rotatesprite(x*xsiz<<16,y*ysiz<<16,65536L,0,tilenum,shade,
                         dapalnum,8+16+64+128,cx1,cy1,cx2,cy2);
        }
    }
}

void overwritesprite(int thex, int they, short tilenum, signed char shade,
        char stat, unsigned char dapalnum) {
    rotatesprite(thex<<16,they<<16,65536L,(stat&8)<<7,tilenum,shade,dapalnum,
                 (((stat&1)^1)<<4)+(stat&2)+((stat&4)>>2)+(((stat&16)>>2)^((stat&8)>>1)),
                 windowx1,windowy1,windowx2,windowy2);
}

void printext(int x, int y, char *buffer, short tilenum, char UNUSED(invisiblecol))
{
    int i;
    unsigned char ch;
    
    for(i=0;buffer[i]!=0;i++) {
        ch = (unsigned char)buffer[i];
        rotatesprite((x-((ch&15)<<3))<<16,(y-((ch>>4)<<3))<<16,65536L,0,tilenum,0,0,8+16+128,x,y,x+7,y+7);
        x += 8;
    }
}

void resettiming()
{
    totalclock = 0;
}

void precache()
{
}