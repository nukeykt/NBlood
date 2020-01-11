//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment
Copyright (C) 2017-2019 Nuke.YKT

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <dos.h>
#include "duke3d.h"
#include "scriplib.h"

// we load this in to get default button and key assignments
// as well as setting up function mappings

#include "_functio.h"

//
// Sound variables
//
int32 FXDevice;
int32 MusicDevice;
int32 FXVolume;
int32 MusicVolume;
int32 SoundToggle;
int32 MusicToggle;
int32 VoiceToggle;
int32 AmbienceToggle;
fx_blaster_config BlasterConfig;
int32 NumVoices;
int32 NumChannels;
int32 NumBits;
int32 MixRate;
int32 MidiPort;
int32 ReverseStereo;

int32 ControllerType;
int32 MouseAiming;

//
// Screen variables
//

int32 ScreenMode;
int32 ScreenWidth;
int32 ScreenHeight;

static char setupfilename[128]={SETUPFILENAME};
static int32 scripthandle;
static int32 setupread=0;
/*
===================
=
= CONFIG_GetSetupFilename
=
===================
*/
#define MAXSETUPFILES 20
void CONFIG_GetSetupFilename( void )
   {
   struct find_t fblock;
   char extension[10];
   char * src;
   char * filenames[MAXSETUPFILES];
   int32 numfiles;
   int32 i;

   strcpy(setupfilename,SETUPFILENAME);

   // determine extension

   src = setupfilename + strlen(setupfilename) - 1;

   while (*src != '.')
      {
      src--;
      }
   strcpy (&extension[1],src);
   extension[0] = '*';

   numfiles=0;
   if (_dos_findfirst(extension,0,&fblock)==0)
      {
      do
         {
         filenames[numfiles]=SafeMalloc(128);
         strcpy(filenames[numfiles],fblock.name);
         numfiles++;
         if (numfiles == MAXSETUPFILES)
            break;
         }
      while(!_dos_findnext(&fblock));
      }
   i = CheckParm (SETUPNAMEPARM);
   if (i!=0)
      {
      numfiles = 0;
      strcpy(setupfilename,_argv[i+1]);
      }
   if (numfiles>1)
      {
      int32 time;
      int32 oldtime;
      int32 count;

      printf("\nMultiple Configuration Files Encountered\n");
      printf("========================================\n");
      printf("Please choose a configuration file from the following list by pressing its\n");
      printf("corresponding letter:\n");
      for (i=0;i<numfiles;i++)
         {
         if (strcmpi(filenames[i],SETUPFILENAME))
            {
            printf("%c. %s\n",'a'+(char)i,filenames[i]);
            }
         else
            {
            printf("%c. %s <DEFAULT>\n",'a'+(char)i,filenames[i]);
            }
         }
      printf("\n");
      printf("(%s will be used if no selection is made within 10 seconds.)\n\n",SETUPFILENAME);
      KB_FlushKeyboardQueue();
      KB_ClearKeysDown();
      count = 9;
      oldtime = clock();
      time=clock()+(10*CLOCKS_PER_SEC);
      while (clock()<time)
         {
         if (clock()>oldtime)
            {
            printf("%ld seconds left. \r",count);
            fflush(stdout);
            oldtime = clock()+CLOCKS_PER_SEC;
            count--;
            }
         if (KB_KeyWaiting())
            {
            int32 ch = KB_Getch();
            ch -='a';
            if (ch>=0 && ch<numfiles)
               {
               strcpy (setupfilename, filenames[ch]);
               break;
               }
            }
         }
      printf("\n\n");
      }
   if (numfiles==1)
      strcpy (setupfilename, filenames[0]);
   printf("Using Setup file: '%s'\n",setupfilename);
   i=clock()+(3*CLOCKS_PER_SEC/4);
   while (clock()<i)
      {
      ;
      }
   for (i=0;i<numfiles;i++)
      {
      SafeFree(filenames[i]);
      }
   }

/*
===================
=
= CONFIG_FunctionNameToNum
=
===================
*/

int32 CONFIG_FunctionNameToNum( char * func )
   {
   int32 i;

   for (i=0;i<NUMGAMEFUNCTIONS;i++)
      {
      if (!stricmp(func,gamefunctions[i]))
         {
         return i;
         }
      }
   return -1;
   }

/*
===================
=
= CONFIG_FunctionNumToName
=
===================
*/

char * CONFIG_FunctionNumToName( int32 func )
   {
   if (func < NUMGAMEFUNCTIONS)
      {
      return gamefunctions[func];
      }
   else
      {
      return NULL;
      }
   }

/*
===================
=
= CONFIG_AnalogNameToNum
=
===================
*/


int32 CONFIG_AnalogNameToNum( char * func )
   {

   if (!stricmp(func,"analog_turning"))
      {
      return analog_turning;
      }
   if (!stricmp(func,"analog_strafing"))
      {
      return analog_strafing;
      }
   if (!stricmp(func,"analog_moving"))
      {
      return analog_moving;
      }
   if (!stricmp(func,"analog_lookingupanddown"))
      {
      return analog_lookingupanddown;
      }

   return -1;
   }

/*
===================
=
= CONFIG_SetDefaults
=
===================
*/

void CONFIG_SetDefaults( void )
   {
   SoundToggle = 1;
   MusicToggle = 1;
   VoiceToggle = 1;
   AmbienceToggle = 1;
   FXVolume = 192;
   MusicVolume = 128;
   ReverseStereo = 0;
   ps[0].aim_mode = 0;
   ud.screen_size = 8;
   ud.screen_tilting = 1;
   ud.shadows = 1;
   ud.detail = 1;
   ud.lockout = 0;
   ud.pwlockout[0] = '\0';
   ud.crosshair = 0;
   ud.m_marker = 1;
   ud.m_ffire = 1;
}
/*
===================
=
= CONFIG_ReadKeys
=
===================
*/

void CONFIG_ReadKeys( void )
   {
   int32 i;
   int32 numkeyentries;
   int32 function;
   char keyname1[80];
   char keyname2[80];
   kb_scancode key1,key2;

   numkeyentries = SCRIPT_NumberEntries( scripthandle, "KeyDefinitions" );

   for (i=0;i<numkeyentries;i++)
      {
      function = CONFIG_FunctionNameToNum(SCRIPT_Entry( scripthandle, "KeyDefinitions", i ));
      if (function != -1)
         {
         memset(keyname1,0,sizeof(keyname1));
         memset(keyname2,0,sizeof(keyname2));
         SCRIPT_GetDoubleString
            (
            scripthandle,
            "KeyDefinitions",
            SCRIPT_Entry( scripthandle,"KeyDefinitions", i ),
            keyname1,
            keyname2
            );
         key1 = 0;
         key2 = 0;
         if (keyname1[0])
            {
            key1 = (byte) KB_StringToScanCode( keyname1 );
            }
         if (keyname2[0])
            {
            key2 = (byte) KB_StringToScanCode( keyname2 );
            }
         CONTROL_MapKey( function, key1, key2 );
         }
      }
   }


/*
===================
=
= CONFIG_SetupMouse
=
===================
*/

void CONFIG_SetupMouse( int32 scripthandle )
   {
   int32 i;
   char str[80];
   char temp[80];
   int32 function, scale;

   for (i=0;i<MAXMOUSEBUTTONS;i++)
      {
      sprintf(str,"MouseButton%ld",i);
      memset(temp,0,sizeof(temp));
      SCRIPT_GetString( scripthandle,"Controls", str,temp);
      function = CONFIG_FunctionNameToNum(temp);
      if (function != -1)
         CONTROL_MapButton( function, i, false );
      sprintf(str,"MouseButtonClicked%ld",i);
      memset(temp,0,sizeof(temp));
      SCRIPT_GetString( scripthandle,"Controls", str,temp);
      function = CONFIG_FunctionNameToNum(temp);
      if (function != -1)
         CONTROL_MapButton( function, i, true );
      }
   // map over the axes
   for (i=0;i<MAXMOUSEAXES;i++)
      {
      sprintf(str,"MouseAnalogAxes%ld",i);
      memset(temp,0,sizeof(temp));
      SCRIPT_GetString(scripthandle, "Controls", str,temp);
      function = CONFIG_AnalogNameToNum(temp);
      if (function != -1)
         {
         CONTROL_MapAnalogAxis(i,function);
         }
      sprintf(str,"MouseDigitalAxes%ld_0",i);
      memset(temp,0,sizeof(temp));
      SCRIPT_GetString(scripthandle, "Controls", str,temp);
      function = CONFIG_FunctionNameToNum(temp);
      if (function != -1)
         CONTROL_MapDigitalAxis( i, function, 0 );
      sprintf(str,"MouseDigitalAxes%ld_1",i);
      memset(temp,0,sizeof(temp));
      SCRIPT_GetString(scripthandle, "Controls", str,temp);
      function = CONFIG_FunctionNameToNum(temp);
      if (function != -1)
         CONTROL_MapDigitalAxis( i, function, 1 );
      sprintf(str,"MouseAnalogScale%ld",i);
      SCRIPT_GetNumber(scripthandle, "Controls", str,&scale);
      CONTROL_SetAnalogAxisScale( i, scale );
      }

   SCRIPT_GetNumber( scripthandle, "Controls","MouseSensitivity",&function);
   CONTROL_SetMouseSensitivity(function);
   }

/*
===================
=
= CONFIG_SetupGamePad
=
===================
*/

void CONFIG_SetupGamePad( int32 scripthandle )
   {
   int32 i;
   char str[80];
   char temp[80];
   int32 function;


   for (i=0;i<MAXJOYBUTTONS;i++)
      {
      sprintf(str,"JoystickButton%ld",i);
      memset(temp,0,sizeof(temp));
      SCRIPT_GetString( scripthandle,"Controls", str,temp);
      function = CONFIG_FunctionNameToNum(temp);
      if (function != -1)
         CONTROL_MapButton( function, i, false );
      sprintf(str,"JoystickButtonClicked%ld",i);
      memset(temp,0,sizeof(temp));
      SCRIPT_GetString( scripthandle,"Controls", str,temp);
      function = CONFIG_FunctionNameToNum(temp);
      if (function != -1)
         CONTROL_MapButton( function, i, true );
      }
   // map over the axes
   for (i=0;i<MAXGAMEPADAXES;i++)
      {
      sprintf(str,"GamePadDigitalAxes%ld_0",i);
      memset(temp,0,sizeof(temp));
      SCRIPT_GetString(scripthandle, "Controls", str,temp);
      function = CONFIG_FunctionNameToNum(temp);
      if (function != -1)
         CONTROL_MapDigitalAxis( i, function, 0 );
      sprintf(str,"GamePadDigitalAxes%ld_1",i);
      memset(temp,0,sizeof(temp));
      SCRIPT_GetString(scripthandle, "Controls", str,temp);
      function = CONFIG_FunctionNameToNum(temp);
      if (function != -1)
         CONTROL_MapDigitalAxis( i, function, 1 );
      }
   SCRIPT_GetNumber( scripthandle, "Controls","JoystickPort",&function);
   CONTROL_JoystickPort = function;
   }

/*
===================
=
= CONFIG_SetupJoystick
=
===================
*/

void CONFIG_SetupJoystick( int32 scripthandle )
   {
   int32 i;
   char str[80];
   char temp[80];
   int32 function, scale;

   for (i=0;i<MAXJOYBUTTONS;i++)
      {
      sprintf(str,"JoystickButton%ld",i);
      memset(temp,0,sizeof(temp));
      SCRIPT_GetString( scripthandle,"Controls", str,temp);
      function = CONFIG_FunctionNameToNum(temp);
      if (function != -1)
         CONTROL_MapButton( function, i, false );
      sprintf(str,"JoystickButtonClicked%ld",i);
      memset(temp,0,sizeof(temp));
      SCRIPT_GetString( scripthandle,"Controls", str,temp);
      function = CONFIG_FunctionNameToNum(temp);
      if (function != -1)
         CONTROL_MapButton( function, i, true );
      }
   // map over the axes
   for (i=0;i<MAXJOYAXES;i++)
      {
      sprintf(str,"JoystickAnalogAxes%ld",i);
      memset(temp,0,sizeof(temp));
      SCRIPT_GetString(scripthandle, "Controls", str,temp);
      function = CONFIG_AnalogNameToNum(temp);
      if (function != -1)
         {
         CONTROL_MapAnalogAxis(i,function);
         }
      sprintf(str,"JoystickDigitalAxes%ld_0",i);
      memset(temp,0,sizeof(temp));
      SCRIPT_GetString(scripthandle, "Controls", str,temp);
      function = CONFIG_FunctionNameToNum(temp);
      if (function != -1)
         CONTROL_MapDigitalAxis( i, function, 0 );
      sprintf(str,"JoystickDigitalAxes%ld_1",i);
      memset(temp,0,sizeof(temp));
      SCRIPT_GetString(scripthandle, "Controls", str,temp);
      function = CONFIG_FunctionNameToNum(temp);
      if (function != -1)
         CONTROL_MapDigitalAxis( i, function, 1 );
      sprintf(str,"JoystickAnalogScale%ld",i);
      SCRIPT_GetNumber(scripthandle, "Controls", str,&scale);
      CONTROL_SetAnalogAxisScale( i, scale );
      }
   // read in JoystickPort
   SCRIPT_GetNumber( scripthandle, "Controls","JoystickPort",&function);
   CONTROL_JoystickPort = function;
   // read in rudder state
   SCRIPT_GetNumber( scripthandle, "Controls","EnableRudder",&CONTROL_RudderEnabled);
   }

void readsavenames(void)
{
    long dummy;
    short i;
    char *fn = {"game_.sav"};
    FILE *fil;

    for (i=0;i<10;i++)
    {
        fn[4] = i+'0';
        if ((fil = fopen(fn,"rb")) == NULL ) continue;
        dfread(&dummy,4,1,fil);

        if(dummy != BYTEVERSION) return;
        dfread(&dummy,4,1,fil);
        dfread(&ud.savegame[i][0],19,1,fil);
        fclose(fil);
    }
}

/*
===================
=
= CONFIG_ReadSetup
=
===================
*/

void CONFIG_ReadSetup( void )
{
   int32 dummy;
   char commmacro[] = "RedneckMacro# ";

   if (!SafeFileExists(setupfilename))
      {
      Error("ReadSetup: %s does not exist\n"
            "           Please run SETUP.EXE\n",setupfilename);
      }

   CONFIG_SetDefaults();
   scripthandle = SCRIPT_Load( setupfilename );

   for(dummy = 0;dummy < 10;dummy++)
   {
       commmacro[13] = dummy+'0';
       SCRIPT_GetString( scripthandle, "Comm Setup",commmacro,&ud.ridecule[dummy][0]);
   }

   SCRIPT_GetString( scripthandle, "Comm Setup","PlayerName",&myname[0]);

   dummy = CheckParm("NAME");
   if( dummy ) strcpy(myname,_argv[dummy+1]);
   dummy = CheckParm("MAP");
#ifndef VOLUMEONE
   if( dummy )
   {
       strcpy(boardfilename,_argv[dummy+1]);
       if( strchr(boardfilename,'.') == 0)
           strcat(boardfilename,".map");
       printf("Using level: '%s'.\n",boardfilename);
   }
   else
#else
    if( dummy )
        puts("The -map option is available in the registered version only!");
#endif
       boardfilename[0] = 0;

   SCRIPT_GetString( scripthandle, "Comm Setup","RTSName",&ud.rtsname[0]);

   SCRIPT_GetNumber( scripthandle, "Screen Setup", "Shadows",&ud.shadows);
   SCRIPT_GetString( scripthandle, "Screen Setup","Password",&ud.pwlockout[0]);
   SCRIPT_GetNumber( scripthandle, "Screen Setup", "Detail",&ud.detail);
   SCRIPT_GetNumber( scripthandle, "Screen Setup", "Tilt",&ud.screen_tilting);
   SCRIPT_GetNumber( scripthandle, "Screen Setup", "Messages",&ud.fta_on);
   SCRIPT_GetNumber( scripthandle, "Screen Setup", "ScreenWidth",&ScreenWidth);
   SCRIPT_GetNumber( scripthandle, "Screen Setup", "ScreenHeight",&ScreenHeight);
   SCRIPT_GetNumber( scripthandle, "Screen Setup", "ScreenMode",&ScreenMode);
   SCRIPT_GetNumber( scripthandle, "Screen Setup", "ScreenGamma",&ud.brightness);
   SCRIPT_GetNumber( scripthandle, "Screen Setup", "ScreenSize",&ud.screen_size);
   SCRIPT_GetNumber( scripthandle, "Screen Setup", "Out",&ud.lockout);

   SCRIPT_GetNumber( scripthandle, "Misc", "Executions",&ud.executions);
   ud.executions++;
   SCRIPT_GetNumber( scripthandle, "Misc", "RunMode",&ud.auto_run);
   SCRIPT_GetNumber( scripthandle, "Misc", "Crosshairs",&ud.crosshair);
   if(ud.wchoice[0][0] == 0 && ud.wchoice[0][1] == 0)
   {
       ud.wchoice[0][0] = 3;
       ud.wchoice[0][1] = 4;
       ud.wchoice[0][2] = 5;
       ud.wchoice[0][3] = 7;
       ud.wchoice[0][4] = 8;
       ud.wchoice[0][5] = 6;
       ud.wchoice[0][6] = 0;
       ud.wchoice[0][7] = 2;
       ud.wchoice[0][8] = 9;
       ud.wchoice[0][9] = 1;

       for(dummy=0;dummy<10;dummy++)
       {
           sprintf(buf,"WeaponChoice%ld",dummy);
           SCRIPT_GetNumber( scripthandle, "Misc", buf, &ud.wchoice[0][dummy]);
       }
    }

   SCRIPT_GetNumber( scripthandle, "Sound Setup", "FXDevice",&FXDevice);
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "MusicDevice",&MusicDevice);
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "FXVolume",&FXVolume);
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "MusicVolume",&MusicVolume);
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "SoundToggle",&SoundToggle);
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "MusicToggle",&MusicToggle);
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "VoiceToggle",&VoiceToggle);
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "AmbienceToggle",&AmbienceToggle);
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "NumVoices",&NumVoices);
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "NumChannels",&NumChannels);
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "NumBits",&NumBits);
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "MixRate",&MixRate);
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "MidiPort",&MidiPort);
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "BlasterAddress",&dummy);
   BlasterConfig.Address = dummy;
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "BlasterType",&dummy);
   BlasterConfig.Type = dummy;
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "BlasterInterrupt",&dummy);
   BlasterConfig.Interrupt = dummy;
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "BlasterDma8",&dummy);
   BlasterConfig.Dma8 = dummy;
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "BlasterDma16",&dummy);
   BlasterConfig.Dma16 = dummy;
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "BlasterEmu",&dummy);
   BlasterConfig.Emu = dummy;
   SCRIPT_GetNumber( scripthandle, "Sound Setup", "ReverseStereo",&dummy);
   ReverseStereo = dummy;

   SCRIPT_GetNumber( scripthandle, "Controls","ControllerType",&ControllerType);
   SCRIPT_GetNumber( scripthandle, "Controls","MouseFlip",&ud.mouseflip);
   SCRIPT_GetNumber( scripthandle, "Controls","MouseAiming",&MouseAiming);
   SCRIPT_GetNumber( scripthandle, "Controls","GameMouseAiming",(int32 *)&ps[0].aim_mode);

   CONTROL_ClearAssignments();

   CONFIG_ReadKeys();

   switch (ControllerType)
      {
      case controltype_keyboardandmouse:
         CONFIG_SetupMouse(scripthandle);
         break;
      default:
         CONFIG_SetupMouse(scripthandle);
      case controltype_keyboardandjoystick:
      case controltype_keyboardandflightstick:
      case controltype_keyboardandthrustmaster:
         CONFIG_SetupJoystick(scripthandle);
         break;
      case controltype_keyboardandgamepad:
         CONFIG_SetupGamePad(scripthandle);
         break;

      }
   setupread = 1;
   }

/*
===================
=
= CONFIG_WriteSetup
=
===================
*/

void CONFIG_WriteSetup( void )
   {
   int32 dummy;

   if (!setupread) return;

   SCRIPT_PutNumber( scripthandle, "Screen Setup", "Shadows",ud.shadows,false,false);
   SCRIPT_PutString( scripthandle, "Screen Setup", "Password",ud.pwlockout);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "Detail",ud.detail,false,false);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "Tilt",ud.screen_tilting,false,false);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "Messages",ud.fta_on,false,false);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "Out",ud.lockout,false,false);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "FXVolume",FXVolume,false,false);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "MusicVolume",MusicVolume,false,false);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "SoundToggle",SoundToggle,false,false);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "VoiceToggle",VoiceToggle,false,false);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "AmbienceToggle",AmbienceToggle,false,false);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "MusicToggle",MusicToggle,false,false);
   SCRIPT_PutNumber( scripthandle, "Sound Setup", "ReverseStereo",ReverseStereo,false,false);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "ScreenSize",ud.screen_size,false,false);
   SCRIPT_PutNumber( scripthandle, "Screen Setup", "ScreenGamma",ud.brightness,false,false);
   SCRIPT_PutNumber( scripthandle, "Misc", "Executions",ud.executions,false,false);
   SCRIPT_PutNumber( scripthandle, "Misc", "RunMode",ud.auto_run,false,false);
   SCRIPT_PutNumber( scripthandle, "Misc", "Crosshairs",ud.crosshair,false,false);
   SCRIPT_PutNumber( scripthandle, "Controls","MouseFlip",ud.mouseflip,false,false);
   SCRIPT_PutNumber( scripthandle, "Controls","GameMouseAiming",(int32) ps[myconnectindex].aim_mode,false,false);

   for(dummy=0;dummy<10;dummy++)
   {
       sprintf(buf,"WeaponChoice%ld",dummy);
       SCRIPT_PutNumber( scripthandle, "Misc",buf,ud.wchoice[myconnectindex][dummy],false,false);
   }

   switch (ControllerType)
      {
      case controltype_keyboardandmouse:
         dummy = CONTROL_GetMouseSensitivity();
         SCRIPT_PutNumber( scripthandle, "Controls","MouseSensitivity",dummy,false,false);
         break;
      }
   SCRIPT_Save (scripthandle, setupfilename);
   SCRIPT_Free (scripthandle);
   }

