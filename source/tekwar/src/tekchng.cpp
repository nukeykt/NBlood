/***************************************************************************
 *   TEKCHNG.C - changehealth, changescore, etc..                          *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#include "build.h"
#include "names.h"

#include "tekwar.h"

#define   MAXSTUNPOINTS       1000

int      stun[MAXPLAYERS],stuntics[MAXPLAYERS];                     
int      fallz[MAXPLAYERS];                                         


int
changehealth(short snum, short deltahealth)
{
     int       healthmax;

     if( !validplayer(snum) ) {
          crash("chnghlth bad plrnum");
     }
     if( toggles[TOGGLE_GODMODE] != 0 ) {
          return(0);
     }
     if( health[snum] <= 0 ) {
          return(1);
     }

     if( option[4] == 0 ) {
          if( deltahealth < 0 ) {
               switch( difficulty ) {
               case 0:
               case 1:
                    deltahealth>>=3;
                    deltahealth-=5;
                    break;
               case 2:
                    deltahealth>>=2;
                    break;
               case 3:
                    deltahealth>>=1;
                    break;
               }
          }
     }

     healthmax=MAXHEALTH;

     if( snum == screenpeek ) {
          if( deltahealth < 0 ) {
               if( deltahealth <= -90 ) {
                    criticalflash();
               }
               else {
                    woundflash();
               }
          }
          else {
               bonusflash();
          }
     }

     if (health[snum] > 0)
     {
          health[snum] += deltahealth;
          if( health[snum] > healthmax )
               health[snum]=healthmax;

          if (health[snum] <= 0)
          {
               health[snum] = -1;
               playsound(S_PLAYERDIE, posx[snum],posy[snum], 0,ST_NOUPDATE);
          }
     }

     if( (health[snum] <= 0) && (option[4] != 0) ) {
          playerdropitems(snum);
     }

     return(health[snum] <= 0);     
}

void
changescore(short snum, short deltascore)
{
     if( toggles[TOGGLE_GODMODE] != 0 ) {
          return;
     }

     score[snum] += deltascore;
     score[snum]&=0xFFFFFFFE;

     if( (score[snum] < 0) || (score[snum] > 99999999) ) 
          score[snum]=0;

}

void
tekchangestun(short snum,short deltastun)
{
     if( toggles[TOGGLE_GODMODE] != 0 ) {
          return;
     }

     switch (difficulty) {
     case 1:
          deltastun/=2;
          break;
     case 3:
          deltastun*=2;
          break;
     }
     stun[snum]+=deltastun;
     if (stun[snum] < 0) {
          stun[snum]=0;
     }
}

void
tekhealstun(short snum)
{
     if( toggles[TOGGLE_GODMODE] != 0 ) {
          return;
     }

     if (stun[snum] < MAXSTUNPOINTS) {
          if (stuntics[snum] >= CLKIPS) {
               stuntics[snum]-=CLKIPS;
               tekchangestun(snum,1);
          }
     }
}

void
tekchangefallz(short snum,int loz,int hiz)
{
     int  died=0;

     posz[snum]+=hvel[snum];

     fallz[snum]+=hvel[snum];
     if (posz[snum] > loz-(KENSPLAYERHEIGHT<<8)) {
          if (fallz[snum] < 0) {
               fallz[snum]=0;
          }
          if (posz[snum] > loz-(4<<8)) {
               posz[snum]=loz-(4<<8);
               hvel[snum]=0;
          }
          switch (fallz[snum]>>15) {
          case 0:
          case 1:
               break;
          case 2:
               tekchangestun(snum,-200);
               break;
          case 3:
               tekchangestun(snum,-400);
               died=changehealth(snum,-200);
               break;
          case 4:
               tekchangestun(snum,-800);
               died=changehealth(snum,-400);
               break;
          case 5:
               tekchangestun(snum,-1000);
               died=changehealth(snum,-1000);
               break;
          default:
               died=changehealth(snum,-9999);
               break;
          }
          fallz[snum]=0;
     }
     if (died && goreflag) {
          tekexplodebody(playersprite[snum]);
     }

     if (posz[snum] < hiz+(4<<8)) {
          posz[snum]=hiz+(4<<8);
          hvel[snum]=0;
     }
}

