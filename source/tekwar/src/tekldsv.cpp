/***************************************************************************
 *   TEKLDSV.C - load and save game, perhaps will develop so can go back   *
 *               and forth beteen maps but I dont want to                  *
 *                                                                         *
 ***************************************************************************/

#include "build.h"
#include "names.h"
#include "mmulti.h"

#include "tekwar.h"


#ifdef COMPRESSION
void
writerle(int fil,void *data,int size)
{
     unsigned short n;
     int  i;
     char c,*p;

     if (size > 4) {
          p=(char *)data;
          i=0;
          while (i < size) {
               n=1;
               c=p[i++];
               while (c == p[i] && i < size) {
                    n++;
                    i++;
               }
               write(fil,&n,sizeof(short));
               write(fil,&c,sizeof(char));
          }
     }
     else {
          write(fil,data,size);
     }
}
#endif

#ifdef COMPRESSION
void
readrle(int fil,void *data,int size)
{
     unsigned short n;
     int  i;
     char c,*p;

     if (size > 4) {
          p=(char *)data;
          i=0;
          while (i < size) {
               read(fil,&n,sizeof(short));
               read(fil,&c,sizeof(char));
               while (n > 0) {
                    p[i++]=c;
                    n--;
               }
          }
     }
     else {
          read(fil,data,size);
     }
}
#endif

int
loadgame(int loadno)                                             
{
     int      i, fil;
     short     dummyscreensize;
     int       rv;
     intptr_t tmpanimates[MAXANIMATES];

     sprintf((char *)tempbuf,"savegam%d.tek",loadno);
     if( (fil=open((char *)tempbuf,O_BINARY|O_RDWR,S_IREAD)) == -1 ) {
          return(-1);
     }

     lseek(fil, 0, SEEK_SET);

     read(fil,loadsavenames[loadno-1],MAXLOADSAVESIZE);

     read(fil,boardfilename,80);
     // setup new map
     newgame(boardfilename);

     read(fil,&numplayers,2);
     read(fil,&myconnectindex,2);
     read(fil,&connecthead,2);
     read(fil,connectpoint2,MAXPLAYERS<<1);

     // Make sure palookups get set, sprites will get overwritten later
     for(i=connecthead;i>=0;i=connectpoint2[i]) initplayersprite((short)i);

     read(fil,posx,MAXPLAYERS<<2);
     read(fil,posy,MAXPLAYERS<<2);
     read(fil,posz,MAXPLAYERS<<2);
     read(fil,horiz,MAXPLAYERS<<2);
     read(fil,zoom,MAXPLAYERS<<2);
     read(fil,hvel,MAXPLAYERS<<2);
     read(fil,ang,MAXPLAYERS<<1);
     read(fil,cursectnum,MAXPLAYERS<<1);
     read(fil,ocursectnum,MAXPLAYERS<<1);
     read(fil,playersprite,MAXPLAYERS<<1);
     read(fil,deaths,MAXPLAYERS<<1);
     read(fil,lastchaingun,MAXPLAYERS<<2);
     read(fil,health,MAXPLAYERS<<2);
     read(fil,score,MAXPLAYERS<<2);
     read(fil,saywatchit,MAXPLAYERS<<2);
     read(fil,numbombs,MAXPLAYERS<<1);
     read(fil,&dummyscreensize,2);
     read(fil,oflags,MAXPLAYERS<<1);
     read(fil,dimensionmode,MAXPLAYERS);
     read(fil,revolvedoorstat,MAXPLAYERS);
     read(fil,revolvedoorang,MAXPLAYERS<<1);
     read(fil,revolvedoorrotang,MAXPLAYERS<<1);
     read(fil,revolvedoorx,MAXPLAYERS<<2);
     read(fil,revolvedoory,MAXPLAYERS<<2);

     read(fil,&numsectors,2);
     read(fil,sector,sizeof(sectortype)*numsectors);

     read(fil,&numwalls,2);
     read(fil,wall,sizeof(walltype)*numwalls);

     // Store all sprites (even holes) to preserve indeces
     read(fil,sprite,sizeof(spritetype)*MAXSPRITES);
     read(fil,headspritesect,(MAXSECTORS+1)<<1);
     read(fil,prevspritesect,MAXSPRITES<<1);
     read(fil,nextspritesect,MAXSPRITES<<1);
     read(fil,headspritestat,(MAXSTATUS+1)<<1);
     read(fil,prevspritestat,MAXSPRITES<<1);
     read(fil,nextspritestat,MAXSPRITES<<1);

     read(fil,&vel,4);
     read(fil,&svel,4);
     read(fil,&angvel,4);

     read(fil,&locselectedgun,4);
     read(fil,&locvel,1);
     read(fil,&olocvel,1);
     read(fil,&locsvel,2);                                   // Les 09/28/95
     read(fil,&olocsvel,2);                                  // Les 09/28/95
     read(fil,&locangvel,2);                                 // Les 09/28/95
     read(fil,&olocangvel,2);                                // Les 09/28/95
     read(fil,&locbits,2);
     read(fil,&olocbits,2);

     read(fil,&locselectedgun2,4);
     read(fil,&locvel2,1);
     read(fil,&olocvel2,1);
     read(fil,&locsvel2,2);                                  // Les 09/28/95
     read(fil,&olocsvel2,2);                                 // Les 09/28/95
     read(fil,&locangvel2,2);                                // Les 09/28/95
     read(fil,&olocangvel2,2);                               // Les 09/28/95
     read(fil,&locbits2,2);
     read(fil,&olocbits2,2);

     read(fil,syncvel,MAXPLAYERS);
     read(fil,osyncvel,MAXPLAYERS);
     read(fil,syncsvel,MAXPLAYERS<<1);                       // Les 09/28/95
     read(fil,osyncsvel,MAXPLAYERS<<1);                      // Les 09/28/95
     read(fil,syncangvel,MAXPLAYERS<<1);                     // Les 09/28/95
     read(fil,osyncangvel,MAXPLAYERS<<1);                    // Les 09/28/95
     read(fil,syncbits,MAXPLAYERS<<1);
     read(fil,osyncbits,MAXPLAYERS<<1);

     read(fil,&screenpeek,2);
     read(fil,&oldmousebstatus,2);
     read(fil,&brightness,2);
     read(fil,&neartagsector,2);
     read(fil,&neartagwall,2);
     read(fil,&neartagsprite,2);
     read(fil,&lockclock,4);
     read(fil,&neartagdist,4);
     read(fil,&neartaghitdist,4);
     read(fil,&masterslavetexttime,4);

     read(fil,rotatespritelist,16<<1);
     read(fil,&rotatespritecnt,2);
     read(fil,warpsectorlist,16<<1);
     read(fil,&warpsectorcnt,2);
     read(fil,xpanningsectorlist,16<<1);
     read(fil,&xpanningsectorcnt,2);
     read(fil,ypanningwalllist,64<<1);
     read(fil,&ypanningwallcnt,2);
     read(fil,floorpanninglist,64<<1);
     read(fil,&floorpanningcnt,2);
     read(fil,dragsectorlist,16<<1);
     read(fil,dragxdir,16<<1);
     read(fil,dragydir,16<<1);
     read(fil,&dragsectorcnt,2);
     read(fil,dragx1,16<<2);
     read(fil,dragy1,16<<2);
     read(fil,dragx2,16<<2);
     read(fil,dragy2,16<<2);
     read(fil,dragfloorz,16<<2);
     read(fil,&swingcnt,2);
     read(fil,swingwall,(32*5)<<1);
     read(fil,swingsector,32<<1);
     read(fil,swingangopen,32<<1);
     read(fil,swingangclosed,32<<1);
     read(fil,swingangopendir,32<<1);
     read(fil,swingang,32<<1);
     read(fil,swinganginc,32<<1);
     read(fil,swingx,(32*8)<<2);
     read(fil,swingy,(32*8)<<2);
     read(fil,revolvesector,4<<1);
     read(fil,revolveang,4<<1);
     read(fil,&revolvecnt,2);
     read(fil,revolvex,(4*16)<<2);
     read(fil,revolvey,(4*16)<<2);
     read(fil,revolvepivotx,4<<2);
     read(fil,revolvepivoty,4<<2);
     read(fil,subwaytracksector,(4*128)<<1);
     read(fil,subwaynumsectors,4<<1);
     read(fil,&subwaytrackcnt,2);
     read(fil,subwaystop,(4*8)<<2);
     read(fil,subwaystopcnt,4<<2);
     read(fil,subwaytrackx1,4<<2);
     read(fil,subwaytracky1,4<<2);
     read(fil,subwaytrackx2,4<<2);
     read(fil,subwaytracky2,4<<2);
     read(fil,subwayx,4<<2);
     read(fil,subwaygoalstop,4<<2);
     read(fil,subwayvel,4<<2);
     read(fil,subwaypausetime,4<<2);
     read(fil,waterfountainwall,MAXPLAYERS<<1);
     read(fil,waterfountaincnt,MAXPLAYERS<<1);
     read(fil,slimesoundcnt,MAXPLAYERS<<1);

     // Warning: only works if all pointers are in sector structures!
     read(fil,tmpanimates,sizeof(intptr_t) * MAXANIMATES);
     for(i=MAXANIMATES-1;i>=0;i--)
          animateptr[i] = (int *)(tmpanimates[i] + (intptr_t)sector);
     read(fil,animategoal,MAXANIMATES<<2);
     read(fil,animatevel,MAXANIMATES<<2);
     read(fil,animateacc,MAXANIMATES<<2);
     read(fil,&animatecnt,4);

     read(fil,&totalclock,4);
     read(fil,&numframes,4);
     read(fil,&randomseed,4);

    #ifdef UNINITENGINECRASH
     read(fil,startumost,MAXXDIM<<1);
     read(fil,startdmost,MAXXDIM<<1);
    #endif

     read(fil,&numpalookups,2);

     read(fil,&visibility,4);
     read(fil,&parallaxtype,1);
     read(fil,&parallaxyoffs,4);
     read(fil,pskyoff,MAXPSKYTILES<<1);
     read(fil,&pskybits,2);

     read(fil,show2dsector,MAXSECTORS>>3);
     read(fil,show2dwall,MAXWALLS>>3);
     read(fil,show2dsprite,MAXSPRITES>>3);
     read(fil,&automapping,1);

     read(fil,invredcards,MAXPLAYERS<<2);   //  sizeof(invredcards));
     read(fil,invbluecards,MAXPLAYERS<<2);  //  sizeof(invbluecards));
     read(fil,invaccutrak,MAXPLAYERS<<2);   //  sizeof(invaccutrak));

     tekgunload(fil);
     tektagload(fil);
     tekstatload(fil);

     tekloadmissioninfo(fil);
 
     rv=close(fil);

     for(i=connecthead;i>=0;i=connectpoint2[i]) initplayersprite((short)i);

     totalclock = lockclock;
     ototalclock = lockclock;

     showmessage("GAME LOADED");

     return(0);
}

int
savegame(int saveno)
{
     int      i, fil;
     int       rv;
     intptr_t tmpanimates[MAXANIMATES];

     sprintf((char *)tempbuf,"savegam%d.tek",saveno);
   
     if( (fil = open((char *)tempbuf,O_BINARY|O_TRUNC|O_CREAT|O_WRONLY,S_IWRITE)) == -1 ) {
          return(-1);
     }

     lseek(fil, 0, SEEK_SET);

     rv=write(fil,loadsavenames[saveno-1],MAXLOADSAVESIZE);    

     rv=write(fil,boardfilename,80);

     rv=write(fil,&numplayers,2);
     rv=write(fil,&myconnectindex,2);
     rv=write(fil,&connecthead,2);
     rv=write(fil,connectpoint2,MAXPLAYERS<<1);

     rv=write(fil,posx,MAXPLAYERS<<2);
     rv=write(fil,posy,MAXPLAYERS<<2);
     rv=write(fil,posz,MAXPLAYERS<<2);
     rv=write(fil,horiz,MAXPLAYERS<<2);
     rv=write(fil,zoom,MAXPLAYERS<<2);
     rv=write(fil,hvel,MAXPLAYERS<<2);
     rv=write(fil,ang,MAXPLAYERS<<1);
     rv=write(fil,cursectnum,MAXPLAYERS<<1);
     rv=write(fil,ocursectnum,MAXPLAYERS<<1);
     rv=write(fil,playersprite,MAXPLAYERS<<1);
     rv=write(fil,deaths,MAXPLAYERS<<1);
     rv=write(fil,lastchaingun,MAXPLAYERS<<2);
     rv=write(fil,health,MAXPLAYERS<<2);
     rv=write(fil,score,MAXPLAYERS<<2);
     rv=write(fil,saywatchit,MAXPLAYERS<<2);
     rv=write(fil,numbombs,MAXPLAYERS<<1);
     rv=write(fil,&screensize,2);
     rv=write(fil,oflags,MAXPLAYERS<<1);
     rv=write(fil,dimensionmode,MAXPLAYERS);
     rv=write(fil,revolvedoorstat,MAXPLAYERS);
     rv=write(fil,revolvedoorang,MAXPLAYERS<<1);
     rv=write(fil,revolvedoorrotang,MAXPLAYERS<<1);
     rv=write(fil,revolvedoorx,MAXPLAYERS<<2);
     rv=write(fil,revolvedoory,MAXPLAYERS<<2);

     rv=write(fil,&numsectors,2);
     rv=write(fil,sector,sizeof(sectortype)*numsectors);

     rv=write(fil,&numwalls,2);
     rv=write(fil,wall,sizeof(walltype)*numwalls);

     // Store all sprites (even holes) to preserve indeces
     rv=write(fil,sprite,sizeof(spritetype)*MAXSPRITES);
     rv=write(fil,headspritesect,(MAXSECTORS+1)<<1);
     rv=write(fil,prevspritesect,MAXSPRITES<<1);
     rv=write(fil,nextspritesect,MAXSPRITES<<1);
     rv=write(fil,headspritestat,(MAXSTATUS+1)<<1);
     rv=write(fil,prevspritestat,MAXSPRITES<<1);
     rv=write(fil,nextspritestat,MAXSPRITES<<1);

     rv=write(fil,&vel,4);
     rv=write(fil,&svel,4);
     rv=write(fil,&angvel,4);

     rv=write(fil,&locselectedgun,4);
     rv=write(fil,&locvel,1);
     rv=write(fil,&olocvel,1);
     rv=write(fil,&locsvel,2);                                  // Les 09/28/95
     rv=write(fil,&olocsvel,2);                                 // Les 09/28/95
     rv=write(fil,&locangvel,2);                                // Les 09/28/95
     rv=write(fil,&olocangvel,2);                               // Les 09/28/95
     rv=write(fil,&locbits,2);
     rv=write(fil,&olocbits,2);

     rv=write(fil,&locselectedgun2,4);
     rv=write(fil,&locvel2,1);
     rv=write(fil,&olocvel2,1);
     rv=write(fil,&locsvel2,2);                                 // Les 09/28/95
     rv=write(fil,&olocsvel2,2);                                // Les 09/28/95
     rv=write(fil,&locangvel2,2);                               // Les 09/28/95
     rv=write(fil,&olocangvel2,2);                              // Les 09/28/95
     rv=write(fil,&locbits2,2);
     rv=write(fil,&olocbits2,2);

     rv=write(fil,syncvel,MAXPLAYERS);
     rv=write(fil,osyncvel,MAXPLAYERS);
     rv=write(fil,syncsvel,MAXPLAYERS<<1);                      // Les 09/28/95
     rv=write(fil,osyncsvel,MAXPLAYERS<<1);                     // Les 09/28/95
     rv=write(fil,syncangvel,MAXPLAYERS<<1);                    // Les 09/28/95
     rv=write(fil,osyncangvel,MAXPLAYERS<<1);                   // Les 09/28/95
     rv=write(fil,syncbits,MAXPLAYERS<<1);
     rv=write(fil,osyncbits,MAXPLAYERS<<1);

     rv=write(fil,&screenpeek,2);
     rv=write(fil,&oldmousebstatus,2);
     rv=write(fil,&brightness,2);
     rv=write(fil,&neartagsector,2);
     rv=write(fil,&neartagwall,2);
     rv=write(fil,&neartagsprite,2);
     rv=write(fil,&lockclock,4);
     rv=write(fil,&neartagdist,4);
     rv=write(fil,&neartaghitdist,4);
     rv=write(fil,&masterslavetexttime,4);

     rv=write(fil,rotatespritelist,16<<1);
     rv=write(fil,&rotatespritecnt,2);
     rv=write(fil,warpsectorlist,16<<1);
     rv=write(fil,&warpsectorcnt,2);
     rv=write(fil,xpanningsectorlist,16<<1);
     rv=write(fil,&xpanningsectorcnt,2);
     rv=write(fil,ypanningwalllist,64<<1);
     rv=write(fil,&ypanningwallcnt,2);
     rv=write(fil,floorpanninglist,64<<1);
     rv=write(fil,&floorpanningcnt,2);
     rv=write(fil,dragsectorlist,16<<1);
     rv=write(fil,dragxdir,16<<1);
     rv=write(fil,dragydir,16<<1);
     rv=write(fil,&dragsectorcnt,2);
     rv=write(fil,dragx1,16<<2);
     rv=write(fil,dragy1,16<<2);
     rv=write(fil,dragx2,16<<2);
     rv=write(fil,dragy2,16<<2);
     rv=write(fil,dragfloorz,16<<2);
     rv=write(fil,&swingcnt,2);
     rv=write(fil,swingwall,(32*5)<<1);
     rv=write(fil,swingsector,32<<1);
     rv=write(fil,swingangopen,32<<1);
     rv=write(fil,swingangclosed,32<<1);
     rv=write(fil,swingangopendir,32<<1);
     rv=write(fil,swingang,32<<1);
     rv=write(fil,swinganginc,32<<1);
     rv=write(fil,swingx,(32*8)<<2);
     rv=write(fil,swingy,(32*8)<<2);
     rv=write(fil,revolvesector,4<<1);
     rv=write(fil,revolveang,4<<1);
     rv=write(fil,&revolvecnt,2);
     rv=write(fil,revolvex,(4*16)<<2);
     rv=write(fil,revolvey,(4*16)<<2);
     rv=write(fil,revolvepivotx,4<<2);
     rv=write(fil,revolvepivoty,4<<2);
     rv=write(fil,subwaytracksector,(4*128)<<1);
     rv=write(fil,subwaynumsectors,4<<1);
     rv=write(fil,&subwaytrackcnt,2);
     rv=write(fil,subwaystop,(4*8)<<2);
     rv=write(fil,subwaystopcnt,4<<2);
     rv=write(fil,subwaytrackx1,4<<2);
     rv=write(fil,subwaytracky1,4<<2);
     rv=write(fil,subwaytrackx2,4<<2);
     rv=write(fil,subwaytracky2,4<<2);
     rv=write(fil,subwayx,4<<2);
     rv=write(fil,subwaygoalstop,4<<2);
     rv=write(fil,subwayvel,4<<2);
     rv=write(fil,subwaypausetime,4<<2);
     rv=write(fil,waterfountainwall,MAXPLAYERS<<1);
     rv=write(fil,waterfountaincnt,MAXPLAYERS<<1);
     rv=write(fil,slimesoundcnt,MAXPLAYERS<<1);

     // Warning: only works if all pointers are in sector structures!
     for(i=MAXANIMATES-1;i>=0;i--)
          tmpanimates[i] = (intptr_t)animateptr[i] - (intptr_t)sector;
     rv=write(fil,tmpanimates, sizeof(intptr_t)*MAXANIMATES);
     rv=write(fil,animategoal,MAXANIMATES<<2);
     rv=write(fil,animatevel,MAXANIMATES<<2);
     rv=write(fil,animateacc,MAXANIMATES<<2);
     rv=write(fil,&animatecnt,4);

     rv=write(fil,&totalclock,4);
     rv=write(fil,&numframes,4);
     rv=write(fil,&randomseed,4);

    #ifdef UNINITENGINECRASH
     rv=write(fil,startumost,MAXXDIM<<1);
     rv=write(fil,startdmost,MAXXDIM<<1);
    #endif
 
     rv=write(fil,&numpalookups,2);

     rv=write(fil,&visibility,4);
     rv=write(fil,&parallaxtype,1);
     rv=write(fil,&parallaxyoffs,4);
     rv=write(fil,pskyoff,MAXPSKYTILES<<1);
     rv=write(fil,&pskybits,2);

     rv=write(fil,show2dsector,MAXSECTORS>>3);
     rv=write(fil,show2dwall,MAXWALLS>>3);
     rv=write(fil,show2dsprite,MAXSPRITES>>3);
     rv=write(fil,&automapping,1);

     rv=write(fil,invredcards,MAXPLAYERS<<2);  // sizeof(invredcards));
     rv=write(fil,invbluecards,MAXPLAYERS<<2); // sizeof(invbluecards));
     rv=write(fil,invaccutrak,MAXPLAYERS<<2);  // sizeof(invaccutrak));

     tekgunsave(fil);
     tektagsave(fil);
     tekstatsave(fil);

     teksavemissioninfo(fil);
 
     rv=close(fil);

     showmessage("GAME SAVED");

     return(0);
}
