#include "compat.h"

#if 0

/***************************************************************************
 *   TEKSND.C  - HMI library replaces Kens sound stuff                     *
 *               Also timer routine and keytimerstuff is here              *
 *                                                                         *
 ***************************************************************************/

#define  AMBUPDATEDIST  4000L

//defines for looping sound variables
extern    int       loopinsound;
extern    int       baydoorloop;
extern    int       ambsubloop;


// a 1-1 map from sounds in sound file to this array
struct    soundbuffertype {
     int       users;
     long      offset;
     long      cache_ptr;
     long      cache_length;
     char      cache_lock;
};
struct    soundbuffertype    sbuf[TOTALSOUNDS];
struct    soundbuffertype    *sbufptr[TOTALSOUNDS];
struct    soundbuffertype    loopbuf[MAXLOOPS];
struct    soundbuffertype    *loopbufptr[MAXLOOPS];

struct    soundtype {
     int       handle;
     int       sndnum;
     int       plevel;
     long      x,y;
     short     type;
};
struct    soundtype     dsound[MAXSOUNDS];
struct    soundtype     *dsoundptr[MAXSOUNDS];
struct    soundtype     lsound[MAXLOOPS];
struct    soundtype     *lsoundptr[MAXLOOPS];
DWORD    *LoopList;

#define   BASESONG            0
#define   MAXBASESONGLENGTH   44136
#define   AVAILMODES          3
#define   SONGSPERLEVEL       3 
#define   NUMLEVELS       7

int       totalsongsperlevel;
char      basesongdata[MAXBASESONGLENGTH];
char      secondsongdata[MAXBASESONGLENGTH];
char      thirdsongdata[MAXBASESONGLENGTH];

struct    songtype  {
     int       handle;
     int       offset;
     int       playing;
     int       pending;
     char     *buffer;
     long      length;
};
struct    songtype  song[SONGSPERLEVEL];
struct    songtype  *songptr[SONGSPERLEVEL];

int       songlist[4096];


initaudioptrs()
{
     int         i;

     for( i=0; i<TOTALSOUNDS; i++ ) {
          sbufptr[i]=&sbuf[i];
          sbufptr[i]->users=0;
          sbufptr[i]->offset=0;
          sbufptr[i]->cache_ptr=0L;
          sbufptr[i]->cache_length=0;
          sbufptr[i]->cache_lock=0x00;
     }
     for( i=0; i<MAXSOUNDS; i++ ) {
          dsoundptr[i]=&dsound[i];
          dsoundptr[i]->handle=NULL_HANDLE;
          dsoundptr[i]->sndnum=0;
          dsoundptr[i]->plevel=0;
          dsoundptr[i]->x=0L;
          dsoundptr[i]->y=0L;
          dsoundptr[i]->type=ST_UPDATE;
          dsoundptr[i]->sndnum=-1;
          sampleptr[i]=(_SOS_START_SAMPLE   _far *)&sample[i];
          sampleptr[i]->wSamplePitchFraction=0;
          sampleptr[i]->wSamplePanDirection=0;
          sampleptr[i]->wSamplePanStart=0;
          sampleptr[i]->wSamplePanEnd=0;
          sampleptr[i]->wSampleDelayBytes=0;
          sampleptr[i]->wSampleDelayRepeat=0;
          sampleptr[i]->dwSampleADPCMPredicted=0;
          sampleptr[i]->wSampleADPCMIndex=0;
          sampleptr[i]->wSampleRootNoteMIDI=0;
          sampleptr[i]->dwSampleTemp1=0;
          sampleptr[i]->dwSampleTemp2=0;
          sampleptr[i]->dwSampleTemp3=0;
     }
//jsa venom
     for( i=0; i<SONGSPERLEVEL; i++ ) {
          songptr[i]=&song[i];
          songptr[i]->handle=NULL_HANDLE;
          songptr[i]->offset=0;
          songptr[i]->playing= 0;
          songptr[i]->pending=0;
          songptr[i]->length=0L;
     }
     songptr[0]->buffer=&basesongdata;
     songptr[1]->buffer=&secondsongdata;
     songptr[2]->buffer=&thirdsongdata;


     for( i=0; i<MAXLOOPS; i++ ) {
          loopbufptr[i]=&loopbuf[i];
          loopbufptr[i]->users=0;
          loopbufptr[i]->offset=0;
          loopbufptr[i]->cache_ptr=0L;
          loopbufptr[i]->cache_length=0;
          loopbufptr[i]->cache_lock=0x00;
          lsoundptr[i]=&lsound[i];
          lsoundptr[i]->handle=NULL_HANDLE;
          lsoundptr[i]->sndnum=0;
          lsoundptr[i]->plevel=0;
          lsoundptr[i]->x=0L;
          lsoundptr[i]->y=0L;
          lsoundptr[i]->type=ST_UPDATE;
          lsoundptr[i]->sndnum=-1;
          loopsampleptr[i]=(_SOS_START_SAMPLE   _far *)&loopsampledata[i];
     }
}

setupdigi()
{
     int       i;
     DWORD     *digilist;

     if( soundmode == SM_NOHARDWARE ) {
          return;
     }

     digilist=( DWORD *)malloc(( size_t)4096);
     if( digilist == ( DWORD *)NULL ) {
          crash("setupdigi: digilist malloc failed");
     }

     fhsounds=open("sounds",O_RDONLY|O_BINARY);
     if( fhsounds == -1 ) {
          crash("setupdigi: cant open sounds");
     }
     memset(digilist,0, 4096);
     lseek(fhsounds,-4096L,SEEK_END);
     i=read(fhsounds,( void *)digilist, 4096);
     if( i != 4096 ) {
          crash("setupdigi: bad read of digilist");
     }

     for( i=0; i<TOTALSOUNDS; i++ ) {
          sbufptr[i]=&sbuf[i];
          sbufptr[i]->users=0;
          sbufptr[i]->offset=(digilist[i*3]*4096L);
          sbufptr[i]->cache_ptr=0L;
          sbufptr[i]->cache_length=( WORD)(digilist[i*3+1]);
          sbufptr[i]->cache_lock=0x00;
     }
     for( i=0; i<MAXSOUNDS; i++ ) {
          dsoundptr[i]=&dsound[i];
          dsoundptr[i]->handle=NULL_HANDLE;
          dsoundptr[i]->sndnum=0;
          dsoundptr[i]->plevel=0;
          dsoundptr[i]->x=0L;
          dsoundptr[i]->y=0L;
          dsoundptr[i]->type=ST_UPDATE;
          dsoundptr[i]->sndnum=-1;
          sampleptr[i]=(_SOS_START_SAMPLE   _far *)&sample[i];
          sampleptr[i]->wSamplePitchFraction=0;
          sampleptr[i]->wSamplePanDirection=0;
          sampleptr[i]->wSamplePanStart=0;
          sampleptr[i]->wSamplePanEnd=0;
          sampleptr[i]->wSampleDelayBytes=0;
          sampleptr[i]->wSampleDelayRepeat=0;
          sampleptr[i]->dwSampleADPCMPredicted=0;
          sampleptr[i]->wSampleADPCMIndex=0;
          sampleptr[i]->wSampleRootNoteMIDI=0;
          sampleptr[i]->dwSampleTemp1=0;
          sampleptr[i]->dwSampleTemp2=0;
          sampleptr[i]->dwSampleTemp3=0;
     }

     free(digilist);
}

setupmidi()
{
     int       fh,dl,rv,i;

     if( musicmode == MM_NOHARDWARE ) {
          return;
     }

     melodicbankptr=( LPSTR)0;
     drumbankptr=( LPSTR)0;
     digitalbankptr=( LPSTR)0;

     if( (musicmode != MM_MIDIFM) && (musicmode != MM_MIDIDIGI) )
          goto nobanks;

     melodicbankptr=( LPSTR)malloc(( size_t)MELODICBANKLENGTH);
     drumbankptr=( LPSTR)malloc(( size_t)DRUMBANKLENGTH);
     if( (melodicbankptr == ( LPSTR)NULL) || (drumbankptr == ( LPSTR)NULL) ) {
          crash("setupmidi: failed malloc");
     }
     if( (fh=open("melodic.bnk",O_RDONLY)) == -1 ) {
          crash("setupmidi: cant open melodic.bnk");
     }
     read(fh, ( void * )melodicbankptr, MELODICBANKLENGTH);
     close(fh);
     rv=sosMIDISetInsData(*fhmididriverptr, melodicbankptr, 1);
     if( rv != _ERR_NO_ERROR ) {
          crash("setupmidi: bad SetInsData");
     }
     if( (fh=open("drum.bnk",O_RDONLY)) == -1 ) {
          crash("setupmidi: cant open drum.bnk");
     }
     read(fh, ( void * )drumbankptr, DRUMBANKLENGTH);
     close(fh);
     rv=sosMIDISetInsData(*fhmididriverptr, drumbankptr, 1);
     if( rv != _ERR_NO_ERROR ) {
          crash("setupmidi: bad SetInsData");
     }

     if( (musicmode == MM_MIDIDIGI) && (midihardwareptr->wPort == 0x388) ) {
          if( (fh=open("test.dig",O_BINARY|O_RDWR)) == -1 ) {
               crash("setupmidi: cant open test.dig");
          }
          dl=lseek(fh, 0L, SEEK_END);
          lseek(fh, 0L, SEEK_SET);
          digitalbankptr=( LPSTR)malloc(( size_t)dl);
          if( digitalbankptr == ( LPSTR)NULL ) {
               crash("setupmidi: failed malloc digbnkptr");
          }
          rv=read(fh, ( void * )digitalbankptr, dl);
          if( rv != dl ) {
               crash("setupmidi: bad .dig read");
          }
          close(fh);
          rv=sosMIDISetInsData(*fhmididigidriverptr, digitalbankptr, 1);
          if( rv != _ERR_NO_ERROR ) {
               crash("setupmidi: bad SetInsData");
          }
     }    

nobanks:

     if( musicmode != MM_NOHARDWARE ) {
          if( (fhsongs=open("SONGS",O_RDONLY | O_BINARY)) == -1 ) {
               crash("setupmidi: cant open songs");
          }
          lseek(fhsongs, 0, SEEK_SET);
          lseek(fhsongs, -4096, SEEK_END);
          read(fhsongs, ( void *)songlist, 4096);
     }

//jsa venom
     for( i=0; i<SONGSPERLEVEL; i++ ) {
          songptr[i]=&song[i];
          songptr[i]->handle=NULL_HANDLE;
          songptr[i]->offset=0;
          songptr[i]->playing= 0;
          songptr[i]->pending=0;
          songptr[i]->length=0L;
     }
     songptr[0]->buffer=&basesongdata;
     songptr[1]->buffer=&secondsongdata;
     songptr[2]->buffer=&thirdsongdata;

      totalsongsperlevel=SONGSPERLEVEL*AVAILMODES;


}


VOID _far cdecl 
soundcallback(WORD fhdriver, WORD action, WORD fhsample)
{
     int  i;

     switch( action ) {
     case _SAMPLE_PROCESSED:
          return;
     case _SAMPLE_LOOPING:
          return;
     case _SAMPLE_DONE:
          for( i=0; i<MAXSOUNDS; i++ ) {
               if( dsoundptr[i]->handle == fhsample ) {
                    sbufptr[dsoundptr[i]->sndnum]->users--;
                    if( sbufptr[dsoundptr[i]->sndnum]->users == 0 ) {
                         sbufptr[dsoundptr[i]->sndnum]->cache_lock=0x00;                              
                    }
                    dsoundptr[i]->handle=NULL_HANDLE;
                    dsoundptr[i]->plevel=0;
                    dsoundptr[i]->sndnum=-1;
                    break;
               }
          }
          break;
     }
     return;
}

VOID _far cdecl 
digiloopcallback(WORD fhdriver, WORD action, WORD fhsample)
{
     if ( action == _SAMPLE_LOOPING ) {
               if(LoopPending) {
                    SND_SwapLoops();
                    LoopPending = 0;
               }
     }
} 

int
playsound(int sn, long sndx,long sndy, int loop, short type)
{
     int       i,nr=0;
     long      dist=0L,vol=0L,pan=0L;

     if( (toggles[TOGGLE_SOUND] == 0) || (soundmode == SM_NOHARDWARE) || (sn < 0) || (sn >= TOTALSOUNDS) )
          return(-1);

     if( type&(ST_UNIQUE|ST_AMBUPDATE|ST_TOGGLE) ) {
          for( i=0; i<MAXSOUNDS; i++ ) {
               if( dsoundptr[i]->handle == NULL_HANDLE ) {
                    continue;
               }
               else if( dsoundptr[i]->sndnum == sn ) {
                    if( (type&ST_TOGGLE) != 0 ) {
                         stopsound(i);
                    }
                    return(-1);
               }
          }
     }

     for( i=0; i<MAXSOUNDS; i++ ) {
          if( dsoundptr[i]->handle == NULL_HANDLE ) 
               break;
     }
     if( i == MAXSOUNDS ) {
          // add plevel and multiple occurrence replacement
          return(-1);
     }

     dsoundptr[i]->type=type;
     dsoundptr[i]->x=sndx; dsoundptr[i]->y=sndy;

     sbufptr[sn]->cache_lock=1;

     if( sbufptr[sn]->cache_ptr == 0L ) {   // no longer in cache
          allocache(&(sbufptr[sn]->cache_ptr), sbufptr[sn]->cache_length, &(sbufptr[sn]->cache_lock));
          if( sbufptr[sn]->cache_ptr == 0L ) {
               sbufptr[sn]->cache_lock=0x00;
               return(-1);
          }
          lseek(fhsounds, sbufptr[sn]->offset, SEEK_SET);
          nr=read(fhsounds,( void *)(sbufptr[sn]->cache_ptr),sbufptr[sn]->cache_length);
          if( nr != sbufptr[sn]->cache_length ) {
               sbufptr[sn]->cache_ptr=0L;
               sbufptr[sn]->cache_lock=0x00;
               return(-1);
          }
     }
     else {
     }

     if( (type&ST_IMMEDIATE) ) {
          vol=0x7fff;
          pan=13;
     }
     else {
          dist=labs(posx[screenpeek]-sndx)+labs(posy[screenpeek]-sndy);
           if( (type&ST_AMBUPDATE) || (type&ST_VEHUPDATE) ) {
               if( dist < AMBUPDATEDIST ) {
                   vol = (AMBUPDATEDIST<<3)-(dist<<3);
               }
               else {
                   vol=0;                   
               }
           }
           else {   
               if(dist < 1500L)
                    vol = 0x7fff;
               else if(dist > 8500L) {
                    if(sn >= S_MALE_COMEONYOU)
                         vol = 0x0000;
                    else
                         vol = 0x1f00;
               }
               else
                    vol = 39000L-(dist<<2);
           }
           pan=((getangle(posx[screenpeek]-dsoundptr[i]->x,posy[screenpeek]-dsoundptr[i]->y)+(2047-ang[screenpeek]))&2047) >> 6;
           if( (pan < 0) || (pan > 35) ) 
               pan=13;
     }
     if( (vol < 0) )
         vol=0;
     if( (vol > 0x7fff) )
         vol=0x7fff;

     sampleptr[i]->lpSamplePtr=( LPSTR)sbufptr[sn]->cache_ptr;
     sampleptr[i]->dwSampleSize=sbufptr[sn]->cache_length;
     sampleptr[i]->wLoopCount=loop;
     sampleptr[i]->wChannel=_CENTER_CHANNEL;
     sampleptr[i]->wVolume=vol;
     sampleptr[i]->wSampleID=sn;
     sampleptr[i]->lpCallback=soundcallback;
     sampleptr[i]->wSamplePort=0;
     sampleptr[i]->wSampleFlags=_LOOPING|_VOLUME|_PANNING;
     sampleptr[i]->dwSampleLoopPoint=0;
     sampleptr[i]->dwSampleLoopLength=0;
     sampleptr[i]->dwSamplePitchAdd=0;
     sampleptr[i]->dwSampleByteLength=sbufptr[sn]->cache_length;
     sampleptr[i]->wSamplePanLocation=PanArray[pan];
     if( sampleptr[i]->wSamplePanLocation > 0xffff ) 
          sampleptr[i]->wSamplePanLocation=0x8000;
     sampleptr[i]->wSamplePanSpeed=0;

     dsoundptr[i]->handle=sosDIGIStartSample(*fhdigidriverptr,sampleptr[i]);
     if( dsoundptr[i]->handle == _ERR_NO_SLOTS ) {
          dsoundptr[i]->handle=NULL_HANDLE;
          dsoundptr[i]->plevel=0;
          dsoundptr[i]->sndnum=-1;
          if( sbufptr[sn]->users == 0 ) {
               sbufptr[sn]->cache_lock=0;
          }
          return(-1);
     }
     else {
          sbufptr[sn]->users++;
         #ifdef SNDDEBUG
          showmessage("SND %03d ADDR %08ld USRS %02d", sn, sbufptr[sn]->cache_ptr, sbufptr[sn]->users);
         #endif
         dsoundptr[i]->sndnum=sn;
     }

     return(i);    
}

stopsound(int i)
{
     if( soundmode == SM_NOHARDWARE )
          return;
     if( (i < 0) || (i >= MAXSOUNDS) ) {
          return;
     }
     if( dsoundptr[i]->handle == NULL_HANDLE )
          return;
     
     sosDIGIStopSample(*fhdigidriverptr, dsoundptr[i]->handle);
     sbufptr[dsoundptr[i]->sndnum]->users--;
     if( sbufptr[dsoundptr[i]->sndnum]->users < 0 )
              sbufptr[dsoundptr[i]->sndnum]->users=0;
     if( sbufptr[dsoundptr[i]->sndnum]->users == 0 ) {
         sbufptr[dsoundptr[i]->sndnum]->cache_lock=0x00;                              
     }
     dsoundptr[i]->handle=NULL_HANDLE;
     dsoundptr[i]->plevel=0;
     dsoundptr[i]->sndnum=-1;
}

void
updatesounds(int    snum)
{
     long      dist=0L,vol=0L,pan=0L;
     int       i,bufnum,panindx;

     if( (toggles[TOGGLE_SOUND] == 0) || (soundmode == SM_NOHARDWARE) ) 
          return;

     for( i=0; i<MAXSOUNDS; i++ ) {
          if( dsoundptr[i]->handle == NULL_HANDLE ) {
               continue;
          }
          if( (dsoundptr[i]->type&(ST_IMMEDIATE|ST_NOUPDATE|ST_VEHUPDATE)) != 0 ) {
               continue;
          }
          dist=labs(posx[snum]-dsoundptr[i]->x)+labs(posy[snum]-dsoundptr[i]->y);

          if(dsoundptr[i]->type==ST_AMBUPDATE) {
               if( dist < AMBUPDATEDIST ) {
                    vol = (AMBUPDATEDIST<<3)-(dist<<3);
               }
               else {
                    vol=0;                   
               }
          }
          else {
               if(dist < 1500L)
                    vol = 0x7fff;
               else if(dist > 8500L)
                    vol = 0x1f00;
               else
                    vol = 39000L-(dist<<2);
          }

          if( (vol < 0) )
              vol=0;
          if( (vol > 0x7fff) )
              vol=0x7fff;

          if( dsoundptr[i]->handle != NULL_HANDLE ) {   // safeguard on int level
               sosDIGISetSampleVolume(*fhdigidriverptr, dsoundptr[i]->handle, vol);
          }
         #ifdef DYNAMICPANPERFECT
          panindx=((getangle(posx[snum]-dsoundptr[i]->x,posy[snum]-dsoundptr[i]->y)+(2047-ang[snum]))&2047) >> 6;
          if( (panindx < 0) || (panindx > 35) ) 
               panindx=13;
          pan=PanArray[panindx];
          if( pan > 0xffff )
               pan=0xffff;
          if( dsoundptr[i]->handle != NULL_HANDLE ) {   // safeguard on int level
               sosDIGISetPanLocation(*fhdigidriverptr, dsoundptr[i]->handle, pan);
          }
         #endif
     }
}

void
updatevehiclesnds(int i, long sndx, long sndy)
{
     long      dist=0L,vol=0L,pan=0L;

     if( soundmode == SM_NOHARDWARE ) {
          return;
     }
     if( (i < 0) || (i > MAXSOUNDS) ) {
          return;
     }

     dsoundptr[i]->x=sndx;
     dsoundptr[i]->y=sndy;

     dist=labs(posx[screenpeek]-sndx)+labs(posy[screenpeek]-sndy);


     if( dist < 1000L ) {
          vol = 0x7fff;
     }
     else if( dist > 9000L ) {
          vol = 0x0000;
     }
     else {
          vol = 36000L-(dist<<2);
     }
     if( (vol < 0) || (vol > 0x7FFF) ) {
          vol=0x7fff;
     }

     if( dsoundptr[i]->handle != NULL_HANDLE ) 
          sosDIGISetSampleVolume(*fhdigidriverptr, dsoundptr[i]->handle, vol);

     pan=((getangle(posx[screenpeek]-dsoundptr[i]->x,posy[screenpeek]-dsoundptr[i]->y)+(2047-ang[screenpeek]))&2047) >> 6;
     if( (pan < 0) || (pan > 35) ) 
          pan=13;

     if( dsoundptr[i]->handle != NULL_HANDLE ) 
          sosDIGISetPanLocation(*fhdigidriverptr, dsoundptr[i]->handle, PanArray[pan]);
}


VOID _far cdecl 
songcallback(WORD shandle)
{
}

VOID _far cdecl 
triggercallback(WORD shandle, BYTE track, BYTE id)
{
}

stopsong(int sn)
{
     if( musicmode == MM_NOHARDWARE )
          return;

     if( songptr[sn]->playing == 0 ) {
          return;     
     }
     if( songptr[sn]->pending != 0 ) {    // cant stop a pending song
          return;                         // since may be interrupted
     }                                    // by trigger function

     sosMIDIStopSong(songptr[sn]->handle);
     songptr[sn]->playing=0;
}

removesong(int sn)
{
     if( musicmode == MM_NOHARDWARE )
          return;

     if( songptr[sn]->handle != NULL_HANDLE ) {
          songptr[sn]->pending=0;
          sosMIDIStopSong(songptr[sn]->handle);
          sosMIDIUnInitSong(songptr[sn]->handle);
          songptr[sn]->handle=NULL_HANDLE;
          songptr[sn]->playing=0;
     }
}

int
playsong(int sn)
{
     int       rv;
     int       fpos;

     if( (musicmode == MM_NOHARDWARE) || (toggles[TOGGLE_MUSIC] == 0) ) {
          return(0);
     }
     if( (sn < 0) || (sn >= SONGSPERLEVEL) || (songptr[sn]->playing != 0) || (songptr[sn]->pending != 0) ) {
          return(0);
     }
     
     if( songptr[sn]->handle != NULL_HANDLE ) {
          removesong(sn);
     }
     if( songptr[sn]->length == 0 )
         return(0);

     songdataptr->lpSongData=( LPSTR)songptr[sn]->buffer;
     songdataptr->lpSongCallback=( VOID _far *)NULL; //songcallback;

     fpos=flushall();
     if( songptr[sn]->handle == NULL_HANDLE ) {
          lseek(fhsongs,0,SEEK_SET);
          fpos=filelength(fhsongs);
          lseek(fhsongs, songptr[sn]->offset, SEEK_SET);
          fpos=tell(fhsongs);
          rv=read(fhsongs, ( void *)songptr[sn]->buffer, songptr[sn]->length);
          if( rv != songptr[sn]->length ) {
              crash("playsong: bad read");
          }
          rv=sosMIDIInitSong(songdataptr, trackmapptr, ( WORD _far *)&(songptr[sn]->handle));
          if( rv != _ERR_NO_ERROR ) {
               songptr[sn]->handle=NULL_HANDLE;
               return(0);
          }
     }
     else {
          rv=sosMIDIResetSong(songptr[sn]->handle, songdataptr); 
          if( rv != _ERR_NO_ERROR ) {
               songptr[sn]->handle=NULL_HANDLE;
              #ifdef MUSICDEBUG
               showmessage("CANT RESET SONG %2d", sn);
              #endif
          }
     }

     rv=sosMIDIStartSong(songptr[sn]->handle);
     if( rv != _ERR_NO_ERROR ) {
          songptr[sn]->handle=NULL_HANDLE;
          return(0);
     }

     if( (musicv<<3) > 0 ) {
          sosMIDIFadeSong(songptr[sn]->handle,_SOS_MIDI_FADE_IN,250,
                          0,(musicv<<3), 50);
     }

    #ifdef MUSICDEBUG
     showmessage("PLAYING SONG %2d", sn);
    #endif
     songptr[sn]->playing=1;
     songptr[sn]->pending=0;

     return(1);
}


void menusong(int insubway)
{
int i,index;

     if( musicmode == MM_NOHARDWARE )
     return;
     
    for( i=0; i<SONGSPERLEVEL; i++ ) {
         removesong(i);
    }

     if(insubway)
          index=(NUMLEVELS*(AVAILMODES*SONGSPERLEVEL)+3);

     else                
          index=NUMLEVELS*(AVAILMODES*SONGSPERLEVEL);

     switch( musicmode ) {
     case MM_MIDIFM:
          break;
     case MM_MIDIAWE32:
           index++;        
          break;
     case MM_MIDIGEN:
           index+=2;
          break;
     }

     for( i=0; i<SONGSPERLEVEL; i++ ) {
          songptr[0]->handle=NULL_HANDLE;
          songptr[0]->offset=songlist[index*3]*4096;
          songptr[0]->playing=0;
          songptr[0]->pending=0;
          songptr[0]->length=( WORD)songlist[(index*3)+1];
          if( songptr[0]->length >= MAXBASESONGLENGTH ) {
               crash("prepsongs: basesong exceeded max length");
          }
     }
     songptr[0]->buffer=&basesongdata;

     playsong(BASESONG);
     

}

startmusic(int level)
{
     int       i,index;

     if( musicmode == MM_NOHARDWARE ) {
          return;
     }

     if( level > 6 ) {
          return;
     }
     
     for( i=0; i<SONGSPERLEVEL; i++ ) {
          removesong(i);
     }

     index=totalsongsperlevel*(level);                 

     switch( musicmode ) {
     case MM_MIDIFM:
          break;
     case MM_MIDIAWE32:
           index+=SONGSPERLEVEL;        
          break;
     case MM_MIDIGEN:
           index+=SONGSPERLEVEL*2;
          break;
     }

     for( i=0; i<SONGSPERLEVEL; i++ ) {
          songptr[i]->handle=NULL_HANDLE;
          songptr[i]->offset=songlist[(index*3)+(i*3)]*4096;
          songptr[i]->playing=0;
          songptr[i]->pending=0;
          songptr[i]->length=( WORD)songlist[((index*3)+(i*3))+1];
          if( songptr[i]->length >= MAXBASESONGLENGTH ) {
               crash("prepsongs: basesong exceeded max length");
          }
     }
     songptr[0]->buffer=&basesongdata;
     songptr[1]->buffer=&secondsongdata;
     songptr[2]->buffer=&thirdsongdata;

     playsong(BASESONG);
}

songmastervolume(int vol)
{
     if( musicmode == MM_NOHARDWARE )
          return;

     if( (vol < 0) || (vol > 127) )
          vol=127;
     sosMIDISetMasterVolume(vol);
}

soundmastervolume(int vol)
{
     if( soundmode == SM_NOHARDWARE )
          return;

     if( (vol < 0) || (vol > 0x7FFF) )
          vol=0x7fff;
     sosDIGISetMasterVolume(*fhdigidriverptr, vol);
}

musicfade(int  dir)
{
     int i;

     if( musicmode == MM_NOHARDWARE ) {
          return;
     }
     for( i=0; i<SONGSPERLEVEL; i++ ) {
          if( (songptr[i]->handle != NULL_HANDLE) ) {
               if( ((musicv<<3) > 0) && (sosMIDISongDone(songptr[i]->handle) == _FALSE) ) {
                    sosMIDIFadeSong(songptr[i]->handle,_SOS_MIDI_FADE_OUT_STOP, 700,
                                    (musicv<<3),0,50);
                    while( (sosMIDISongDone(songptr[i]->handle)==_FALSE) ) {
                    }
               }
               removesong(i);
          }
     }
}

musicoff(void)
{
     int  i;

     if( musicmode != MM_NOHARDWARE )  {
          for( i=0; i<SONGSPERLEVEL; i++ ) {
               if( songptr[i]->handle == NULL_HANDLE )
                    continue;
               sosMIDIStopSong(songptr[i]->handle);
               sosMIDIUnInitSong(songptr[i]->handle);
          }
    }
}

stopallsounds()
{
     int       i;

     if( soundmode == SM_NOHARDWARE )
          return;

     for( i=0; i< MAXSOUNDS; i++ ) {
          if( dsoundptr[i]->handle == NULL_HANDLE )
               continue;
          sosDIGIStopSample(*fhdigidriverptr, dsoundptr[i]->handle);
          sbufptr[dsoundptr[i]->sndnum]->users--;
          if( sbufptr[dsoundptr[i]->sndnum]->users < 0 )
              sbufptr[dsoundptr[i]->sndnum]->users=0;
          if( sbufptr[dsoundptr[i]->sndnum]->users == 0 ) {
               sbufptr[dsoundptr[i]->sndnum]->cache_lock=0x00;                              
          }
          dsoundptr[i]->handle=NULL_HANDLE;
          dsoundptr[i]->plevel=0;
          dsoundptr[i]->sndnum=-1;
     }

//clear variables that track looping sounds
     loopinsound=-1;
     baydoorloop=-1;
     ambsubloop=-1;

}

uninitsb(void)
{
     int       i;

     if( musicmode != MM_NOHARDWARE )  {
          for( i=0; i<SONGSPERLEVEL; i++ ) {
               if( songptr[i]->handle == NULL_HANDLE )
                    continue;
               sosMIDIStopSong(songptr[i]->handle);
               sosMIDIUnInitSong(songptr[i]->handle);
          }
          sosMIDIUnInitDriver(*fhmididriverptr, _TRUE );
         sosMIDIUnInitSystem();
    }

    if( digiloopflag != 0 ) {
          for( i=0; i<MAXLOOPS; i++ ) {
               if( lsoundptr[i]->handle == NULL_HANDLE )
                    continue;
               sosDIGIStopSample(*fhdigidriverptr, lsoundptr[i]->handle);
          }
    }

     if( soundmode != SM_NOHARDWARE ) {
          for( i=0; i<MAXSOUNDS; i++ ) {
               if( dsoundptr[i]->handle == NULL_HANDLE )
                    continue;
               sosDIGIStopSample(*fhdigidriverptr, dsoundptr[i]->handle);
          }
          if( soundmode != SM_NOHARDWARE ) {
               sosTIMERRemoveEvent(*fhdigifillptr);
               sosDIGIUnInitDriver(*fhdigidriverptr, _TRUE,_TRUE);
               sosDIGIUnInitSystem();
          }
     }

     if( fhsounds >= 0 )
          close(fhsounds);
     if( fhsongs >= 0 )
          close(fhsongs);

     if( hLoopFile != -1 )
           close( hLoopFile );
     if( LoopList != ( DWORD *)NULL )
           free( LoopList );

     if( melodicbankptr )
          free( ( void *)melodicbankptr);
     if( drumbankptr )
          free( ( void *)drumbankptr);
     if( digitalbankptr )
          free( ( void *)digitalbankptr);

     smkuninit(*fhdigidriverptr);
}

void
initlooptable(void)
{
     if(!digiloopflag) 
          return;
     
     hLoopFile = open("LOOPS",O_RDONLY | O_BINARY);
     if( hLoopFile == -1 ) {
          crash("initlooptable: cant open loops");
     }
     LoopList = ( DWORD    *)malloc(0x1000); 
    if( LoopList == ( DWORD *)NULL )
         crash("initlooptable: cant get mem for LoopList");
     lseek(hLoopFile,-4096L,SEEK_END);
     read(hLoopFile,(void *)FP_OFF(LoopList),4096);
}

void
tekprepdigiloops(void)
{
     if( !digiloopflag )
          return;

     loopbufptr[0]->cache_lock=1;
     allocache(&(loopbufptr[0]->cache_ptr), MAX_LOOP_LENGTH, &(loopbufptr[0]->cache_lock));
     if( loopbufptr[0]->cache_ptr == 0L ) {
          loopbufptr[0]->cache_lock=0x00;
          digiloopflag=0;
     }

     loopbufptr[1]->cache_lock=1;
     allocache(&(loopbufptr[1]->cache_ptr), MAX_LOOP_LENGTH, &(loopbufptr[1]->cache_lock));
     if( loopbufptr[1]->cache_ptr == 0L ) {
          loopbufptr[1]->cache_lock=0x00;
          digiloopflag=0;
     }
}

void 
SND_LoadLoop(int load_start)
{
     int  nr=0;
     SeekIndex = ( LoopList[(LoopIndex * 3)+0] * 4096 );
     SampleSize= (WORD)LoopList[(LoopIndex * 3) + 1];
     lseek(hLoopFile, SeekIndex, SEEK_SET);

     if(!load_start) {
          nr=read(hLoopFile,( void *)(loopbufptr[looptoggle]->cache_ptr),SampleSize);
          if( nr != SampleSize ) {
               loopbufptr[looptoggle]->cache_ptr=0L;
               loopbufptr[looptoggle]->cache_lock=0x00;
               crash("read problem with loops");
          }
          loopsampleptr[looptoggle]->lpSamplePtr=( LPSTR)loopbufptr[looptoggle]->cache_ptr;
          loopsampleptr[looptoggle]->dwSampleSize= SampleSize;
          loopsampleptr[looptoggle]->dwSampleByteLength= SampleSize;
     }
     else {
          nr=read(hLoopFile,( void *)(loopbufptr[looptoggle]->cache_ptr),SampleSize);
          if( nr != SampleSize ) {
               loopbufptr[looptoggle]->cache_ptr=0L;
               loopbufptr[looptoggle]->cache_lock=0x00;
               crash("read problem with loops");
          }
         loopsampleptr[looptoggle]->lpSamplePtr=( LPSTR)loopbufptr[looptoggle]->cache_ptr;
          loopsampleptr[looptoggle]->dwSampleSize= SampleSize;
          loopsampleptr[looptoggle]->dwSampleByteLength= SampleSize;
          lsoundptr[looptoggle]->handle=sosDIGIStartSample(*fhdigidriverptr,loopsampleptr[looptoggle]);
          looptoggle^=1;
     }

     LoopIndex++;
     if(LoopIndex>MAX_SND_LOOPS-1)
          LoopIndex=0;
}

VOID 
SND_SwapLoops(VOID)
{
     int temp,i;

     temp=looptoggle^1;

     if( !sosDIGISampleDone(*fhdigidriverptr,lsoundptr[temp]->handle) )
     {
          sosDIGIStopSample(*fhdigidriverptr,lsoundptr[temp]->handle);
          lsoundptr[looptoggle]->handle = sosDIGIStartSample(*fhdigidriverptr,loopsampleptr[looptoggle] );
     }
       
     looptoggle^=1;

}

#endif

int       digiloopflag=0;

void
initsb(char UNUSED(option1),char UNUSED(option2),int UNUSED(digihz),
    char UNUSED(option7a),char UNUSED(option7b),int UNUSED(val),char UNUSED(option7c))
{
}

void
uninitsb(void)
{
}

void
musicoff(void)
{
}

int
playsound(int UNUSED(sn), int UNUSED(sndx), int UNUSED(sndy), int UNUSED(loop), short UNUSED(type))
{
    return -1;
}

void
updatevehiclesnds(int UNUSED(i), int UNUSED(sndx), int UNUSED(sndy))
{
}

void
stopsound(int UNUSED(i))
{
}

void
songmastervolume(int UNUSED(vol))
{
}

void
soundmastervolume(int UNUSED(vol))
{
}

void
updatesounds(int UNUSED(snum))
{
}

void
stopallsounds()
{
}

void
musicfade()
{
}

void
menusong(int UNUSED(insubway))
{
}

void
startmusic(int UNUSED(level))
{
}