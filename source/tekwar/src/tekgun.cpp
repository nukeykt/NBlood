
/***************************************************************************
 *   TEKGUN.C  - Tekwar specific code for shooting                         *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#include "build.h"
#include "names.h"
#include "pragmas.h"
#include "mmulti.h"

#include "tekwar.h"

#define   NUMWEAPONS               8
                                   
#define   FORCEPROJECTILESTAT      710
#define   DARTPROJECTILESTAT       712
#define   BOMBPROJECTILESTAT       714
#define   BOMBPROJECTILESTAT2      716
#define   ROCKETPROJECTILESTAT     718
#define   MATRIXPROJECTILESTAT     720
#define   MOVEBODYPARTSSTAT        900
#define   VANISH                   999

#define   DRAWWEAPSPEED       4

int       goreflag;
int       fireseq[MAXPLAYERS];
int       oneshot[MAXPLAYERS];
short     ammo0[MAXPLAYERS];
short     ammo1[MAXPLAYERS];
short     ammo2[MAXPLAYERS];
short     ammo3[MAXPLAYERS];
short     ammo4[MAXPLAYERS];
short     ammo5[MAXPLAYERS];
short     ammo6[MAXPLAYERS];
short     ammo7[MAXPLAYERS];
short     ammo8[MAXPLAYERS];
int      weapons[MAXPLAYERS];          // flags for weapons onself   
int      firedonetics[MAXPLAYERS];
int      lastgun[MAXPLAYERS];
int       drawweap[MAXPLAYERS];

struct guntype {
     short pic;               // gun frame when carrying weapon
     short firepic;           // 1st gun frame when firing weapon
     short endfirepic;        // last gun frame when firing weapon
     char rep;                // 1=automatic weapon, 0=semi-automatic
     char action[8];          // 8 frame action bytes - 1=shootgun()
     char pos;                // position on screen 0=center, 1=left, 2=right
     short tics;              // tics to delay for each frame
} guntype[NUMWEAPONS]={
     //   pic         firepic    endfirepic rep /             \ pos  tics/frame
     {GUN07READY,GUN07FIRESTART,GUN07FIREEND,0,{0,1,0,0,0,0,0,0},2,TICSPERFRAME*8}, 
     {GUN04READY,GUN04FIRESTART,GUN04FIREEND,1,{0,1,0,0,0,0,0,0},2,TICSPERFRAME*4}, 
     {GUN01READY,GUN01FIRESTART,GUN01FIREEND,1,{0,1,0,0,0,0,0,0},2,TICSPERFRAME*4},
     {GUN03READY,GUN03FIRESTART,GUN03FIREEND,1,{0,1,0,0,0,0,0,0},2,TICSPERFRAME*2},
     {GUN02READY,GUN02FIRESTART,GUN02FIREEND,1,{0,1,0,0,0,0,0,0},2,TICSPERFRAME*2},
     {GUN08READY,GUN08FIRESTART,GUN08FIREEND,1,{0,1,0,0,0,0,0,0},2,TICSPERFRAME*2},
     {GUN05READY,GUN05FIRESTART,GUN05FIREEND,0,{0,1,1,0,0,0,0,0},2,TICSPERFRAME*8},
     // matrix hand
     {      3980,          3981,        3984,1,{0,1,0,0,0,0,0,0},2,TICSPERFRAME*3}
};

int  gunbobx[10]={0,2,4,6,8,8,8,6,4,2};
int  gunboby[10]={0,2,4,6,8,10,8,6,4,2};
int  gunbob,bobtics;

short
maxdamage[NUMWEAPONS]={ 10, 100, 500, 500, 20, 500, 500, 1000 };
short
mindamage[NUMWEAPONS]={ 1,   1,  10,  50,   1,  10,  50,  10,  };


int
validplayer(int snum)
{
     int       j;

     for( j=connecthead ; j >= 0 ; j=connectpoint2[j] ) {
          if( j == snum ) {
               return(1);
          }
     }
     return(0);
}

void
restockammo(int snum)
{
     ammo1[snum]=MAXAMMO;     
     ammo2[snum]=MAXAMMO>>1;     
     ammo3[snum]=20;
     ammo4[snum]=0;
     ammo5[snum]=MAXAMMO;
     ammo6[snum]=0;
     ammo7[snum]=0;
     ammo8[snum]=0;
}

int
tekgunrep(int gun)                // is "gun" an automatic weapon?
{
     return(guntype[gun+1].rep);
}

int
tekgundamage(int gun,int UNUSED(x),int UNUSED(y),int UNUSED(z),int UNUSED(hitsprite))
{
     int       damage;

     switch( gun ) {
     case 1:  damage= 2; break;
     case 4:  damage=20; break;
     default: damage= 2; break;
     }
     switch( difficulty ) {
     case 0:  
     case 1:  damage+=4; break;
     case 2:  damage+=4; break;
     case 3:
     default:  damage+=10; break;    
     }

     return(damage);
}

int
hasammo(int gun,short p)     // does player[p] have ammo for "gun"?
{
     switch (gun+1) {
     case GUN1FLAG:
          if( ammo1[p] > 0 ) {
               return(1);
          }
          break;
     case GUN2FLAG:
          if( ammo2[p] > 0 ) {
               return(1);
          }
          break;
     case GUN3FLAG:
          if( ammo3[p] > 0 ) {
               return(1);
          }
          break;
     case GUN4FLAG:
          if( ammo4[p] > 0 ) {
               return(1);
          }
          break;
     case GUN5FLAG:
          if( ammo5[p] > 0 ) {
               return(1);
          }
          break;
     case GUN6FLAG:
          if( ammo6[p] > 0 ) {
               return(1);
          }
          break;
     case GUN7FLAG:
          if( ammo7[p] > 0 ) {
               return(1);
          }
          break;
     case GUN8FLAG:
          if( ammo8[p] > 0 ) {
               return(1);
          }
          break;
     }
     return(0);
}

void
tekfiregun(int gun,short p)  // this kicks off an animation sequence
{
     if( fireseq[p] != 0 ) {
          return;
     }
     if( tekgunrep(gun) || ((oflags[p]&2048) == 0) ) {
//          if( hasammo(gun,p) ) {
               fireseq[p]=1;
               oneshot[p]=0;
//          }
     }
}

void
playerwoundplayer(short plrhit, short plr, char guntype)
{
     int       killed=0,damage=0,score=0;

     if( !validplayer(plrhit) || !validplayer(plr) ) {
          return;
     }
                               
     switch( guntype ) {
     case 2:  score=5;  damage=48;   break;
     case 3:  score=10; damage=192;  break;
     case 4:  score=10; damage=512;  break;
     case 5:  score=15; damage=1024; break;
     case 6:  score=10; damage=24;   break;
     case 7:  score=10; damage=512;  break;
     default: score=5;  damage=32;   break;
     }

     killed=changehealth(plrhit,-damage);

     if( killed ) {
          changescore(plr, (score<<1));
     }
     else {
          changescore(plr, score);
     }
}

void
killscore(short hs, short snum, char guntype)
{
     int       ext=sprptr[hs]->extra;
     short     score;

     if( !validplayer(snum) ) {
          return;
     }
     if( !validext(ext) ) {
          return;
     }

     if( isanandroid(hs) ) {
          score=200;
     }
     else if( isahologram(hs) ) {
          score=100;
     }
     else {
          switch( sprXTptr[ext]->class ) {
          case CLASS_NULL:
               return;       
          case CLASS_FCIAGENT:
               score=-500;
               break;
          case CLASS_CIVILLIAN:       
               score=-500;
               break;
          case CLASS_SPIDERDROID:
               score=50;
               break;
          case CLASS_COP:
               score=-500;
               break;   
          case CLASS_MECHCOP:
               score=-50;
               break;    
          case CLASS_TEKBURNOUT:
               score=100;
               break; 
          case CLASS_TEKGOON:
               score=200;
               break;    
          case CLASS_ASSASSINDROID:
               score=300;
               break;
          case CLASS_SECURITYDROID:
               score=100;
               break;
          case CLASS_TEKBOSS:
               score=300;
               break;     
          case CLASS_TEKLORD:
               score=500;
               break;     
          }
          switch( guntype ) {
          case 0:
          case 1:
               if( score < 0 ) {
                    score=0;
               }
               else {
                    score+=(score<<1);
               }
               break;
          }
     }

     if( option[4] != 0 ) {
          score>>=4;
     }
     changescore(snum, score);
}

void
playerpainsound(int p)
{
     if( !validplayer(p) ) {
          crash("playerpainsnd: bad plr num");
     }

     if( krand_intercept(" GUN 341") < 1024 ) {
          playsound(S_PAIN1+(krand_intercept(" GUN 342")&1),posx[p],posy[p],0,ST_NOUPDATE);
     }
}

void                          
shootgun(short snum,int x,int y,int z,short daang,int dahoriz,
     short dasectnum,char guntype)
{
     short hitsect,hitwall,hitsprite,daang2;
     short bloodhitsect,bloodhitwall,bloodhitsprite;
     int  bloodhitx,bloodhity,bloodhitz;
     int  cx,cy,i,j,daz2,hitx,hity,hitz,xydist,zdist;
     int   rv,pnum;

     if( health[snum] <= 0 ) {     
          return;                  
     }                             

     guntype+=1;
     switch (guntype) {
     case GUN1FLAG:
          cx=x+(sintable[(daang+2560+128)&2047]>>6);
          cy=y+(sintable[(daang+2048+128)&2047]>>6);
          j=jsinsertsprite(dasectnum, FORCEPROJECTILESTAT);
          if( j != -1 ) {
               fillsprite(j,cx,cy,z,128+2,-16,0,12,16,16,0,0,FORCEBALLPIC,daang,
                          sintable[(daang+2560-11)&2047]>>5, // -17 = travel 3 degrees left
                          sintable[(daang+2048-11)&2047]>>5, // -17 = travel 3 degrees left
                          100-dahoriz,snum+MAXSPRITES,dasectnum,FORCEPROJECTILESTAT,0,0,-1);
          }
          break;
     case GUN2FLAG:
          ammo2[snum]--;
          if( ammo2[snum] < 0 ) {
               ammo2[snum]=0;
               break;
          }
          daang2=daang;
          daz2=(100-dahoriz)*2000;
          hitscan(x,y,z,dasectnum,sintable[(daang2+2560)&2047],
                  sintable[(daang2+2048)&2047],daz2,
                  &hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);
          if( (hitsprite >= 0) && (sprptr[hitsprite]->statnum < MAXSTATUS)) {
               if( playerhit(hitsprite, &pnum) ) {
                    playerpainsound(pnum);
                    playerwoundplayer(pnum,snum,2);
               }
               else {
                    switch( sprptr[hitsprite]->picnum ) {
                    default:
                         rv=damagesprite(hitsprite,tekgundamage(guntype,x,y,z,hitsprite));
                         if( (rv == 1) && (goreflag) ) {
                              if( spewblood(hitsprite, hitz, daang) != 0 ) {
                                   sprptr[hitsprite]->cstat&=0xFEFE;  // non hitscan and non block
                                   // must preserve values from previous hitscan call, 
                                   // thus the bloodxhitx, bloodwall, etc...
                                   hitscan(x,y,z,dasectnum,sintable[(daang2+2560)&2047],
                                           sintable[(daang2+2048)&2047],daz2,
                                           &bloodhitsect,&bloodhitwall,&bloodhitsprite,&bloodhitx,&bloodhity,&bloodhitz,CLIPMASK1);
                                   if( bloodhitwall != -1 ) {
                                        bloodonwall(bloodhitwall,sprptr[hitsprite]->x,sprptr[hitsprite]->y,sprptr[hitsprite]->z,
                                                    sprptr[hitsprite]->sectnum,daang2,bloodhitx,bloodhity,bloodhitz);
                                   }
                              }
                         }
                         if( rv == 1 ) {
                              killscore(hitsprite,snum,guntype);
                         }
                    break;
               }
               }
          }
          if( hitwall >= 0 ) {
               j=jsinsertsprite(hitsect, 3);
               if( j != -1 ) {
                    fillsprite(j,hitx,hity,hitz+(8<<8),2,0,0,32,22,22,0,0,
                               EXPLOSION,daang,0,0,0,snum+MAXSPRITES,hitsect,3,63,0,-1);
                    movesprite((short)j,
                               -(((int)sintable[(512+daang)&2047]*TICSPERFRAME)<<4),
                               -(((int)sintable[daang]*TICSPERFRAME)<<4),0L,4L<<8,4L<<8,1);
                    playsound(S_RIC1,hitx,hity,0,ST_NOUPDATE);
               }
          }
          break;
     case GUN3FLAG:
          ammo3[snum]--;
          if( ammo3[snum] < 0 ) {
               ammo3[snum]=0;
               break;
          }
          cx=x+(sintable[(daang+2560+256)&2047]>>6);
          cy=y+(sintable[(daang+2048+256)&2047]>>6);
          if( invaccutrak[snum] > 0 ) {
               j=jsinsertsprite(dasectnum, BOMBPROJECTILESTAT);
               if( j != -1 ) {
                    fillsprite(j,cx,cy,z+(4<<8),128,-16,0,32,16,16,0,0,BOMBPIC,daang,
                         sintable[(daang+2560-11)&2047]>>5, // -17 = travel 3 degrees left
                         sintable[(daang+2048-11)&2047]>>5, // -17 = travel 3 degrees left
                         100-dahoriz,snum,dasectnum,BOMBPROJECTILESTAT,0,0,-1);
                    sprptr[j]->owner=snum;
               }
          }
          else {
               j=jsinsertsprite(dasectnum, BOMBPROJECTILESTAT2);
               if( j != -1 ) {
                    fillsprite(j,cx,cy,z+(4<<8),128,-16,0,32,16,16,0,0,BOMBPIC,daang,
                         sintable[(daang+2560-11)&2047]>>5, // -17 = travel 3 degrees left
                         sintable[(daang+2048-11)&2047]>>5, // -17 = travel 3 degrees left
                         100-dahoriz,snum,dasectnum,BOMBPROJECTILESTAT2,0,0,-1);
                    sprptr[j]->owner=snum;
               }
          }
          break;
     case GUN4FLAG:                 
          ammo4[snum]--;
          if( ammo4[snum] < 0 ) {
               ammo4[snum]=0;
               break;
          }
          cx=x+(sintable[(daang+2560+128)&2047]>>6);
          cy=y+(sintable[(daang+2048+128)&2047]>>6);
          j=jsinsertsprite(dasectnum, DARTPROJECTILESTAT);
          if( j != -1 ) {
               fillsprite(j,cx,cy,z+(5<<8),128,-35,0,12,16,16,0,0, 338 ,daang,
                    sintable[(daang+2560-6)&2047]>>5, // -17 = travel 3 degrees left
                    sintable[(daang+2048-6)&2047]>>5, // -17 = travel 3 degrees left
                    100-dahoriz,snum,dasectnum,DARTPROJECTILESTAT,0,0,-1);
               sprptr[j]->owner=snum;
               playsound(S_RIC1,cx,cy,0,ST_NOUPDATE);

          }
          break;
     case GUN5FLAG:
          daang2=daang;
          daz2=(100-dahoriz)*2000;
          hitscan(x,y,z,dasectnum,sintable[(daang2+2560)&2047],
                  sintable[(daang2+2048)&2047],daz2,
                  &hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);
          if( (hitsprite >= 0) && (sprptr[hitsprite]->statnum < MAXSTATUS)) {
               xydist=klabs(posx[snum]-sprptr[hitsprite]->x)+klabs(posy[snum]-sprptr[hitsprite]->y);
               zdist=klabs( (posz[snum]>>8)-((sprptr[hitsprite]->z>>8)-(tilesizy[sprptr[hitsprite]->picnum]>>1)) ); 
               if( (xydist > 768) || (zdist > 50) ) {
                    break;
               }
               if( playerhit(hitsprite, &pnum) ) {
                    playerpainsound(pnum);
                    playerwoundplayer(pnum,snum,5);
               }
               else {
                    rv=damagesprite(hitsprite,tekgundamage(guntype,x,y,z,hitsprite));
                    if( rv == 1 ) {
                         killscore(hitsprite, snum, guntype);
                    }
               }
          }
          break;
     case GUN6FLAG:
          ammo6[snum]--;
          if( ammo6[snum] < 0 ) {
               ammo6[snum]=0;
               break;
          }
          daang2=daang;
          daz2=(100-dahoriz)*2000;
          hitscan(x,y,z,dasectnum,sintable[(daang2+2560)&2047],
                  sintable[(daang2+2048)&2047],daz2,
                  &hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);
          if( (hitsprite >= 0) && (sprptr[hitsprite]->statnum < MAXSTATUS)) {
               xydist=klabs(posx[snum]-sprptr[hitsprite]->x)+klabs(posy[snum]-sprptr[hitsprite]->y);
               zdist=klabs( (posz[snum]>>8)-((sprptr[hitsprite]->z>>8)-(tilesizy[sprptr[hitsprite]->picnum]>>1)) ); 
               if( (xydist > 2560) || (zdist > 576) ) {
                    break;
               }
               if( playerhit(hitsprite, &pnum) ) {
                    if( playervirus(pnum, FIREPIC) ) {
                         playerwoundplayer(pnum,snum,6);
                    }
               }
               else {
                    attachvirus(hitsprite, FIREPIC);
               }
          }
          break;
     case GUN7FLAG:
          ammo7[snum]--;
          if( ammo7[snum] < 0 ) {
               ammo7[snum]=0;
               break;
          }
          for (i=0 ; i < 3 ; i++) {
               cx=x+(sintable[(daang+2560+256)&2047]>>6);
               cy=y+(sintable[(daang+2048+256)&2047]>>6);
               j=jsinsertsprite(dasectnum,ROCKETPROJECTILESTAT);
               if (j != -1) {
                    fillsprite(j,cx,cy,z+(4<<8),128,-24,0,12,16,16,0,0,335,daang,
                         sintable[(daang+2560-11)&2047]>>(2+i),
                         sintable[(daang+2048-11)&2047]>>(2+i),
                         (100-dahoriz)-(i<<2),
                         snum,dasectnum,ROCKETPROJECTILESTAT,0,0,-1);
               sprptr[j]->owner=snum;
               }
          }
          break;
     case GUN8FLAG:
          ammo8[snum]--;
          if( ammo8[snum] < 0 ) {
               ammo8[snum]=0;
               break;
          }
          cx=x+(sintable[(daang+2560)&2047]>>6);
          cy=y+(sintable[(daang+2048)&2047]>>6);
          j=jsinsertsprite(dasectnum, MATRIXPROJECTILESTAT);
          if( j != -1 ) {
               fillsprite(j,cx,cy,z+(5<<8),128,-35,0,12,16,16,0,0,3765,daang,
                    sintable[(daang+2560)&2047]>>5,
                    sintable[(daang+2048)&2047]>>5,
                    100-dahoriz,snum,dasectnum,MATRIXPROJECTILESTAT,0,0,-1);
               sprptr[j]->owner=snum;
          }
          break;
     default:
          break;
    }

    if( guntype != GUN1FLAG )
          playergunshot(snum); 
}

#define   DIEFRAMETIME   (160/(JAKETWITCHPIC-JAKEDEATHPIC))

short dieframe[MAXPLAYERS],
     firepicdelay[MAXPLAYERS];

void
tekanimweap(int gun,short p)
{
     int  ammo,firekey,fseq,seq,tics;
     int usegun;
     struct guntype *gunptr;

     if (gun < 0 || gun >= NUMWEAPONS) {
          crash("gun589: Invalid gun number (%d,p=%d)",gun,p);
     }
     if (option[4] != 0) {
          if (health[p] < 0 && dieframe[p] == 0) {
               dieframe[p]=TICSPERFRAME;
          }
          else if (dieframe[p] > 0) {
               sprptr[playersprite[p]]->picnum=
                    JAKEDEATHPIC+(dieframe[p]/DIEFRAMETIME);
               dieframe[p]+=TICSPERFRAME;
               return;
          }
          else {
               if ((syncbits[p]&2048) && firepicdelay[p] == 0) {
                    sprptr[playersprite[p]]->picnum=JAKEATTACKPIC;
                    firepicdelay[p]=16;
               }
               else if (firepicdelay[p] > 0) {
                    firepicdelay[p]--;
                    if (firepicdelay[p] <= 0) {
                         firepicdelay[p]=0;
                         sprptr[playersprite[p]]->picnum=JAKEWALKPIC;
                    }
               }
               if (firepicdelay[p] == 0 && syncvel[p] == 0 && syncsvel[p] == 0) {
                    sprptr[playersprite[p]]->picnum=JAKESTANDPIC;
               }
          }
     }
     if ((seq=fireseq[p]) == 0) {
          lastgun[p]=gun;
          return;
     }
     usegun=lastgun[p];
     if (usegun != gun) {
          gunptr=&guntype[usegun];
          fseq=gunptr->firepic-gunptr->pic;
          if (firedonetics[p] >= 0) {
               firedonetics[p]=2;
          }
          firekey=0;
     }
     else {
          firekey=(syncbits[p]&2048);
          if (firekey != 0) {
               drawweap[p]=1;
          }
     }
     if( (firekey == 0) && ((syncbits[p]&128) != 0) && firedonetics[p] == 1) {
//         ((firedonetics[p] == 1) || !hasammo(usegun,p)) ) {
          firedonetics[p]=2;
     }
     if (firedonetics[p] == 2) {
          firedonetics[p]=-1;
          drawweap[p]=0;
          if (firedonetics[p] <= 0) {
               gunptr=&guntype[usegun];
               fseq=gunptr->firepic-gunptr->pic;
               fireseq[p]=fseq;
          }
     }
     gunptr=&guntype[usegun];
     fseq=gunptr->firepic-gunptr->pic;
     if (seq-1 >= fseq) {
          tics=gunptr->tics;
     }
     else {
          tics=DRAWWEAPSPEED;
     }
     if (lockclock >= lastchaingun[p]+tics) {
          lastchaingun[p]=lockclock;
          ammo=hasammo(usegun,p);
          if (seq-1 >= fseq && ammo) {
               if (gunptr->action[seq-fseq-1]) {
                    switch( gun+1 ) {
//jsa friday
                         case 1:
                              if(option[4] != 0)
                                   playsound(S_WEAPON1 ,posx[p],posy[p],0,ST_NOUPDATE);
                              else
                                   playsound(S_WEAPON1 ,0,0,0,ST_IMMEDIATE);
                              break;
                         case 2:
                              if(option[4] != 0)
                                   playsound(S_WEAPON2 ,posx[p],posy[p],0,ST_NOUPDATE);
                              else
                                   playsound(S_WEAPON2 ,0,0,0,ST_IMMEDIATE);
                              break;
                         case 3:
                              if(option[4] != 0)
                                   playsound(S_WEAPON3 ,posx[p],posy[p],0,ST_NOUPDATE);
                              else
                                   playsound(S_WEAPON3 ,0,0,0,ST_IMMEDIATE);
                              break;
                         case 4:
                              if(option[4] != 0)
                                   playsound(S_WEAPON4 ,posx[p],posy[p],0,ST_NOUPDATE);
                              else
                                   playsound(S_WEAPON4 ,0,0,0,ST_IMMEDIATE);
                              break;
                         case 5:
                              if(option[4] != 0)
                                   playsound(S_WEAPON5 ,posx[p],posy[p],0,ST_NOUPDATE);
                              else
                                   playsound(S_WEAPON5 ,0,0,0,ST_IMMEDIATE);
                              break;
                         case 6:
                              if(option[4] != 0)
                                   playsound(S_WEAPON6 ,posx[p],posy[p],0,ST_NOUPDATE);
                              else
                                   playsound(S_WEAPON6 ,0,0,0,ST_IMMEDIATE);
                              break;
                         case 7: 
                              if(option[4] != 0)
                                   playsound(S_WEAPON7 ,posx[p],posy[p],0,ST_NOUPDATE);
                              else
                                   playsound(S_WEAPON7 ,0,0,0,ST_IMMEDIATE);
                              break;
                         case 8:
                              playsound(S_WEAPON8 ,0,0,0,ST_IMMEDIATE);
                              break;
                         default:
                              break;

                    }
                    shootgun(p,posx[p],posy[p],posz[p],ang[p],horiz[p],
                              cursectnum[p],usegun);
                    if (option[4] != 0) {
                         sprptr[playersprite[p]]->picnum=JAKEATTACKPIC+1;
                         firepicdelay[p]=8;
                    }
               }
               if (seq-1 >= gunptr->endfirepic-gunptr->pic) {
                    fireseq[p]=fseq+1;
                    return;
               }
               if (firekey == 0) {
                    firedonetics[p]=1;
                    fireseq[p]=fseq;
                    return;
               }
               else {
                    firedonetics[p]=0;
               }
          }
          if (drawweap[p] != 0) {
               if (ammo || seq < fseq) {
                    seq++;
               }
               else {
                    if (firekey == 0) {
                         firedonetics[p]=1;
                    }
               }
          }
          else if (firedonetics[p] < 0) {
               seq--;
          }
          fireseq[p]=seq;
     }
}

int       
tekexplodebody(int i)
{
     int  j,k,r,ext;

     ext=sprptr[i]->extra;
     if( (!validext(ext)) || (goreflag == 0) ) {
          return(0);
     }
     switch( sprXTptr[ext]->basepic ) {
     case RUBWALKPIC:
     case JAKEWALKPIC:
     case COP1WALKPIC:
     case ANTWALKPIC:
     case SARAHWALKPIC:
     case MAWALKPIC:
     case DIWALKPIC:
     case ERWALKPIC:
     case SAMWALKPIC:
     case FRGWALKPIC:
     case SUNGWALKPIC:
     case COWWALKPIC:
     case COPBWALKPIC:
     case NIKAWALKPIC:
     case REBRWALKPIC:
     case TRENWALKPIC:
     case WINGWALKPIC:
     case HALTWALKPIC:
     case REDHWALKPIC:
     case ORANWALKPIC:
     case BLKSWALKPIC:
     case SFROWALKPIC:
     case SSALWALKPIC:
     case SGOLWALKPIC:
     case SWATWALKPIC:
          break;
     default:
          return(0);
     }          

     r=(krand_intercept(" GUN 787")%72)+8;
     for (k=0 ; k < r ; k++) {
          j=jsinsertsprite(sprptr[i]->sectnum,MOVEBODYPARTSSTAT);
          sprptr[j]->x=sprptr[i]->x;
          sprptr[j]->y=sprptr[i]->y;
          sprptr[j]->z=sprptr[i]->z+(8<<8);
          sprptr[j]->cstat=0;
          sprptr[j]->shade=0;
          sprptr[j]->pal=0;
          sprptr[j]->xrepeat=24;
          sprptr[j]->yrepeat=24;
          sprptr[j]->ang=sprptr[i]->ang;
          sprptr[j]->xvel=(krand_intercept(" GUN 799")&511)-256;
          sprptr[j]->yvel=(krand_intercept(" GUN 800")&511)-256;
          sprptr[j]->zvel=-((krand_intercept(" GUN 801")&8191)+4096);
          sprptr[j]->owner=sprptr[i]->owner;
          sprptr[j]->clipdist=32;
          sprptr[j]->lotag=360;
          sprptr[j]->hitag=0;
          switch (k) {
          case 0:
               sprptr[j]->picnum=GOREHEAD;
               break;
          case 1:
          case 10:
               sprptr[j]->picnum=GOREARM;
               break;
          case 5:
          case 15:
               sprptr[j]->picnum=GORELEG;
               break;
          default:
               sprptr[j]->picnum=GOREBLOOD;
               break;
          }
     }
     playsound(S_GORE1+(krand_intercept(" GUN 823")%2), sprptr[i]->x,sprptr[i]->y, 0, ST_NOUPDATE);

     return(1);
}

void
gunstatuslistcode(void)
{
     short     hitobject,hitsprite,ext;
     int      i,nexti,dax,day,daz,j;
     int       pnum,rv;

     i=headspritestat[FORCEPROJECTILESTAT];   //moving force ball
     while (i >= 0) {
          nexti=nextspritestat[i];
          dax=((((int)sprptr[i]->xvel)*TICSPERFRAME)<<11);
          day=((((int)sprptr[i]->yvel)*TICSPERFRAME)<<11);
          daz=((((int)sprptr[i]->zvel)*TICSPERFRAME)<<3);
          hitobject=movesprite((short)i,dax,day,daz,4<<8,4<<8,1);
          if( (hitobject&0xC000) == 49152 ) {  // hit a sprite
               hitsprite=hitobject&0x0FFF;
               ext=sprptr[hitsprite]->extra;
               if( validext(ext) ) {
                    switch( sprXTptr[ext]->walkpic ) {
                    case RUBWALKPIC:
                    case FRGWALKPIC:
                    case COP1WALKPIC:
                    case ANTWALKPIC:
                    case SARAHWALKPIC:
                    case MAWALKPIC:
                    case DIWALKPIC:
                    case ERWALKPIC:
                    case SAMWALKPIC:
                    case SUNGWALKPIC:
                    case COWWALKPIC:
                    case COPBWALKPIC:
                    case NIKAWALKPIC:
                    case REBRWALKPIC:
                    case TRENWALKPIC:
                    case WINGWALKPIC:
                    case HALTWALKPIC:
                    case REDHWALKPIC:
                    case ORANWALKPIC:
                    case BLKSWALKPIC:
                    case SFROWALKPIC:
                    case SSALWALKPIC:
                    case SGOLWALKPIC:
                    case SWATWALKPIC:
                         attachvirus(hitsprite, FORCEBALLPIC);
                         break;
                    default:
                         forceexplosion(i);
                         break;
                    }
                    jsdeletesprite(( short)i);
               }
               else {
                    forceexplosion(i);
                    jsdeletesprite((short)i);
               }
          }
          else if( hitobject != 0 ) {
               forceexplosion(i);
               jsdeletesprite((short)i);
          }

          i=nexti;
     }

     i=headspritestat[ROCKETPROJECTILESTAT];  
     while (i >= 0) {
          nexti=nextspritestat[i];

          dax=((((int)sprptr[i]->xvel)*TICSPERFRAME)<<10);
          day=((((int)sprptr[i]->yvel)*TICSPERFRAME)<<10);
          daz=((((int)sprptr[i]->zvel)*TICSPERFRAME)<<3);

          hitobject=movesprite((short)i,dax,day,daz,4<<8,4<<8,1);
          if( (hitobject&0xC000) == 49152 ) {  // hit a sprite
               hitsprite=hitobject&0x0FFF;
               if( playerhit(hitsprite, &pnum) ) {
                    playerpainsound(pnum);
                    playerwoundplayer(pnum,sprptr[i]->owner,7);
               }
               else if( sprptr[i]->picnum != sprptr[hitsprite]->picnum ) {
                    if( isahologram(hitsprite) ) {
                         showmessage("WAS A HOLOGRAM");
                         killscore(hitsprite, sprptr[i]->owner, 3);
                         changespritestat(hitsprite, VANISH);
                         //jsdeletesprite(hitsprite);
                    }
                    else if( isanandroid(hitsprite) ) {
                         showmessage("WAS AN ANDROID");
                         killscore(hitsprite, sprptr[i]->owner, 3);
                         androidexplosion(hitsprite);
                         changespritestat(hitsprite, VANISH);
                         //jsdeletesprite(hitsprite);
                    }
                    else {
                         blastmark(hitsprite);
                         rv=damagesprite(hitsprite, 500);
                         if( rv == 1 ) {
                              killscore(hitsprite,sprptr[i]->owner,3);
                         }
                    }
               }
          }
          if( hitobject != 0 ) {
               bombexplosion(i);   
               jsdeletesprite((short)i);
          }

          i=nexti;
     }

     i=headspritestat[MATRIXPROJECTILESTAT];  
     while (i >= 0) {
          nexti=nextspritestat[i];

          dax=((((int)sprptr[i]->xvel)*TICSPERFRAME)<<12);
          day=((((int)sprptr[i]->yvel)*TICSPERFRAME)<<12);
          daz=((((int)sprptr[i]->zvel)*TICSPERFRAME)<<4);

          hitobject=movesprite((short)i,dax,day,daz,4<<8,4<<8,1);
          if( (hitobject&0xC000) == 49152 ) {  // hit a sprite
               hitsprite=hitobject&0x0FFF;
               if( !playerhit(hitsprite, &pnum) ) {
                    rv=damagesprite(hitsprite, 500);
                    if( rv == 1 ) {
                         killscore(hitsprite,sprptr[i]->owner,8);
                    }
               }
          }
          if( hitobject != 0 ) {
               jsdeletesprite((short)i);
          }

          i=nexti;
     }

     i=headspritestat[BOMBPROJECTILESTAT];  
     while (i >= 0) {
          nexti=nextspritestat[i];

          dax=((((int)sprptr[i]->xvel)*TICSPERFRAME)<<12);
          day=((((int)sprptr[i]->yvel)*TICSPERFRAME)<<12);
          daz=((((int)sprptr[i]->zvel)*TICSPERFRAME)<<4);

          hitobject=movesprite((short)i,dax,day,daz,4<<8,4<<8,1);
          if( (hitobject&0xC000) == 49152 ) {  // hit a sprite
               hitsprite=hitobject&0x0FFF;
               if( playerhit(hitsprite, &pnum) ) {
                    playerpainsound(pnum);
                    playerwoundplayer(pnum,sprptr[i]->owner,3);
               }
               else if( sprptr[i]->picnum != sprptr[hitsprite]->picnum ) {
                    if( isahologram(hitsprite) ) {
                         showmessage("WAS A HOLOGRAM");
                         killscore(hitsprite, sprptr[i]->owner, 3);
                         changespritestat(hitsprite, VANISH);
                         //jsdeletesprite(hitsprite);
                    }
                    else if( isanandroid(hitsprite) ) {
                         showmessage("WAS AN ANDROID");
                         killscore(hitsprite, sprptr[i]->owner, 3);
                         androidexplosion(hitsprite);
                         changespritestat(hitsprite, VANISH);
                         //jsdeletesprite(hitsprite);
                    }
                    else {
                         blastmark(hitsprite);
                         rv=damagesprite(hitsprite, 500);
                         if( rv == 1 ) {
                              killscore(hitsprite,sprptr[i]->owner,3);
                         }
                    }
               }
          }
          if( hitobject != 0 ) {
               bombexplosion(i);   
               jsdeletesprite((short)i);
          }

          i=nexti;
     }
 
     i=headspritestat[BOMBPROJECTILESTAT2];  
     while (i >= 0) {
          nexti=nextspritestat[i];

          sprptr[i]->xvel+=( ((krand_intercept(" GUN1025")&64)-32)>>1 );
          sprptr[i]->yvel+=( ((krand_intercept(" GUN1026")&64)-32)>>1 );
          sprptr[i]->zvel+=( ((krand_intercept(" GUN1027")&31)-16)>>1 );

          dax=((((int)sprptr[i]->xvel)*TICSPERFRAME)<<12);
          day=((((int)sprptr[i]->yvel)*TICSPERFRAME)<<12);
          daz=((((int)sprptr[i]->zvel)*TICSPERFRAME)<<4);

          hitobject=movesprite((short)i,dax,day,daz,4<<8,4<<8,1);
          if( (hitobject&0xC000) == 49152 ) {  // hit a sprite
               hitsprite=hitobject&0x0FFF;
               if( playerhit(hitsprite, &pnum) ) {
                    playerpainsound(pnum);
                    playerwoundplayer(pnum,sprptr[i]->owner,3);
               }
               else if( sprptr[i]->picnum != sprptr[hitsprite]->picnum ) {
                    if( isahologram(hitsprite) ) {
                         showmessage("WAS A HOLOGRAM");
                         killscore(hitsprite, sprptr[i]->owner, 3);
                         changespritestat(hitsprite, VANISH);
                         //jsdeletesprite(hitsprite);
                    }
                    else if( isanandroid(hitsprite) ) {
                         showmessage("WAS AN ANDROID");
                         killscore(hitsprite, sprptr[i]->owner, 3);
                         androidexplosion(hitsprite);
                         changespritestat(hitsprite, VANISH);
                         //jsdeletesprite(hitsprite);
                    }
                    else {
                         blastmark(hitsprite);
                         rv=damagesprite(hitsprite, 500);
                         if( rv == 1 ) {
                              killscore(hitsprite,sprptr[i]->owner,3);
                         }
                    }
               }
          }
          if( hitobject != 0 ) {
               bombexplosion(i);   
               jsdeletesprite((short)i);
          }

          i=nexti;
     }

     i=headspritestat[DARTPROJECTILESTAT];  
     while (i >= 0) {
          nexti=nextspritestat[i];
          dax=((((int)sprptr[i]->xvel)*TICSPERFRAME)<<13);
          day=((((int)sprptr[i]->yvel)*TICSPERFRAME)<<13);
          daz=((((int)sprptr[i]->zvel)*TICSPERFRAME)<<5);

          hitobject=movesprite((short)i,dax,day,daz,4<<8,4<<8,1);
          if( (hitobject&0xC000) == 49152 ) {  // hit a sprite
               hitsprite=hitobject&0x0FFF;
               if( playerhit(hitsprite, &pnum) ) {
                    playerpainsound(pnum);
                    playerwoundplayer(pnum,sprptr[i]->owner,4);
               }
               else if( sprptr[i]->picnum != sprptr[hitsprite]->picnum ) {
                    if( isahologram(hitsprite) ) {
                         showmessage("WAS A HOLOGRAM");
                         killscore(hitsprite, sprptr[i]->owner, 4);
                         changespritestat(hitsprite, VANISH);
                         //jsdeletesprite(hitsprite);
                    }
                    else if( isanandroid(hitsprite) ) {
                         showmessage("WAS AN ANDROID");
                         killscore(hitsprite, sprptr[i]->owner, 4);
                         androidexplosion(hitsprite);
                         changespritestat(hitsprite, VANISH);
                         //jsdeletesprite(hitsprite);
                    }
                    else if( tekexplodebody(hitsprite) ) {
                         missionaccomplished(hitsprite);
                         killscore(hitsprite, sprptr[i]->owner, 4);
                         changespritestat(hitsprite, VANISH);
                         //jsdeletesprite(hitsprite);
                    } 
                    else {
                         rv=damagesprite(hitsprite, 500);
                         if( rv == 1 ) {
                              killscore(hitsprite,sprptr[i]->owner,3);
                         }
                    }
               }
          }
          if( hitobject != 0 ) {
               j=jsinsertsprite(sprite[i].sectnum, 3);
               if( j != -1 ) {
                    fillsprite(j,sprptr[i]->x,sprptr[i]->y,sprptr[i]->z+(8<<8),0,-4,0,32,
                         64,64,0,0,
                         EXPLOSION,sprptr[i]->ang,0,0,0,i,sprptr[i]->sectnum,3,63,0,0);
               }
               jsdeletesprite((short)i);
          }

          i=nexti;
     }

     i=headspritestat[MOVEBODYPARTSSTAT];    //flying body parts (gore option)
     while (i >= 0) {
          nexti=nextspritestat[i];
          sprptr[i]->x+=((sprptr[i]->xvel*TICSPERFRAME)>>5);
          sprptr[i]->y+=((sprptr[i]->yvel*TICSPERFRAME)>>5);
          sprptr[i]->z+=((sprptr[i]->zvel*TICSPERFRAME)>>5);
          sprptr[i]->zvel+=(TICSPERFRAME<<7);
          if (sprptr[i]->z < sectptr[sprptr[i]->sectnum]->ceilingz+(4<<8)) {
               sprptr[i]->z=sectptr[sprptr[i]->sectnum]->ceilingz+(4<<8);
               sprptr[i]->zvel=-(sprptr[i]->zvel>>1);
          }
          if (sprptr[i]->z > sectptr[sprptr[i]->sectnum]->floorz-(4<<8)) {
               sprptr[i]->z=sectptr[sprptr[i]->sectnum]->floorz-(4<<8);
               if (sprptr[i]->picnum == GOREBLOOD) {
                    sprptr[i]->xvel=0;
                    sprptr[i]->yvel=0;
                    sprptr[i]->zvel=0;
                    sprptr[i]->cstat|=0x20;
               }
               else {
                    sprptr[i]->zvel=-(sprptr[i]->zvel>>1);
               }
          }
          sprptr[i]->lotag-=TICSPERFRAME;
          if (sprptr[i]->lotag < 0) {
               jsdeletesprite(i);
          }
          i=nexti;
     }
}


int       matgunpic;

void
tekdrawgun(int gun,short p)
{
     int  pic,x,apic;

     if (fireseq[p] == 0) {
          pic=guntype[gun].pic;
          return;
     }
     apic=AIMPIC;
     if( difficulty <= 1 )
          apic=BIGAIMPIC;
     if( (toggles[TOGGLE_RETICULE]) && (dimensionmode[myconnectindex] == 3) ) {
          overwritesprite(windowx1+((windowx2-windowx1)>>1),
                          windowy1+((windowy2-windowy1)>>1),apic,16,0x01,0);
     }
     gun=lastgun[p];
     if( firedonetics[p] > 0 ) {
          pic=guntype[gun].firepic;
     }
     else {
          pic=guntype[gun].pic+fireseq[p]-1;
     }
     x=160L;

     if( pic == 3981 ) {
          if( syncsvel[p] < 0 ) {
               overwritesprite(x,100L,3990,sectptr[cursectnum[p]]->floorshade,1|2,0);
               return;
          }
          else if( syncsvel[p] > 0 ) {
               overwritesprite(x,100L,3986,sectptr[cursectnum[p]]->floorshade,1|2,0);
               return;
          }
          else if( syncangvel[p] < 0 ) {
               overwritesprite(x,100L,3985,sectptr[cursectnum[p]]->floorshade,1|2,0);
               return;
          }
          else if( syncangvel[p] > 0 ) {
               overwritesprite(x,100L,3989,sectptr[cursectnum[p]]->floorshade,1|2,0);
               return;
          }
          else if( (syncbits[p]&1) != 0 ) {
               overwritesprite(x,100L,3994+((syncbits[p]&256)==0),sectptr[cursectnum[p]]->floorshade,1|2,0);
               return;
          }
          else if( (syncbits[p]&2) != 0 ) {
               overwritesprite(x,100L,3998+((syncbits[p]&256)==0),sectptr[cursectnum[p]]->floorshade,1|2,0);
               return;
          }
     }

     if( (syncvel[p]|syncsvel[p]) != 0 ) {
          switch( pic ) {
          case GUN01FIRESTART:
          case GUN02FIRESTART:
          case GUN03FIRESTART:
          case GUN04FIRESTART:
          case GUN05FIRESTART:
          case GUN06FIRESTART:
          case GUN07FIRESTART:
          case GUN08FIRESTART:
               bobtics+=( 1 + (((syncbits[p]&256)>0)<<2) );
               if( bobtics > TICSPERFRAME ) {
                    bobtics=0;
                    gunbob++;
                    if( gunbob > 9 )
                         gunbob=0;
               }
               break;
          default:
               gunbob=0;
               break;
          }
     }
     overwritesprite(x+gunbobx[gunbob],100L+gunboby[gunbob],pic,sectptr[cursectnum[p]]->floorshade,1|2,0);
}

int
tekhasweapon(int gun,short snum)
{  
     int       hasit=0;
     
     if( mission == 7 ) {
          if( gun != 7 ) {
               if( snum == screenpeek ) {
                    notininventory=1;
               }
               return(0);
          }
          else {
               return(1);
          }
     }
     else {
          if( gun == 7 ) {
               if( snum == screenpeek ) {
                    showmessage("ONLY IN MATRIX");
               }
               return(0);
          }
     }

     hasit=(weapons[snum]&flags32[gun+1]);
     if( hasit )
          return(1);

     notininventory=1;
     return(0);
}

void
tekgunsave(int fil)
{
     write(fil,fireseq,MAXPLAYERS*sizeof(int));
     write(fil,ammo1,MAXPLAYERS*sizeof(short));
     write(fil,ammo2,MAXPLAYERS*sizeof(short));
     write(fil,ammo3,MAXPLAYERS*sizeof(short));
     write(fil,ammo4,MAXPLAYERS*sizeof(short));
     write(fil,ammo5,MAXPLAYERS*sizeof(short));
     write(fil,ammo6,MAXPLAYERS*sizeof(short));
     write(fil,ammo7,MAXPLAYERS*sizeof(short));
     write(fil,ammo8,MAXPLAYERS*sizeof(short));
     write(fil,weapons,MAXPLAYERS*sizeof(int));
     write(fil,firedonetics,MAXPLAYERS*sizeof(int));
     write(fil,lastgun,MAXPLAYERS*sizeof(int));
     write(fil,&goreflag,sizeof(int));
}

void
tekgunload(int fil)
{
     read(fil,fireseq,MAXPLAYERS*sizeof(int));
     read(fil,ammo1,MAXPLAYERS*sizeof(short));
     read(fil,ammo2,MAXPLAYERS*sizeof(short));
     read(fil,ammo3,MAXPLAYERS*sizeof(short));
     read(fil,ammo4,MAXPLAYERS*sizeof(short));
     read(fil,ammo5,MAXPLAYERS*sizeof(short));
     read(fil,ammo6,MAXPLAYERS*sizeof(short));
     read(fil,ammo7,MAXPLAYERS*sizeof(short));
     read(fil,ammo8,MAXPLAYERS*sizeof(short));
     read(fil,weapons,MAXPLAYERS*sizeof(int));
     read(fil,firedonetics,MAXPLAYERS*sizeof(int));
     read(fil,lastgun,MAXPLAYERS*sizeof(int));
     read(fil,&goreflag,sizeof(int));
}
