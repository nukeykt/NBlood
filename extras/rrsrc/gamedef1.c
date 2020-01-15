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

#include "duke3d.h"

extern short otherp;

static short total_lines,line_number;
static char checking_ifelse,*last_used_text;
static short num_squigilly_brackets;
static long last_used_size;
static long parsing_state;

#ifdef RRRA
#define NUMKEYWORDS     147
#else
#define NUMKEYWORDS     131
#endif

char *keyw[NUMKEYWORDS] =
{
    "definelevelname",  // 0
    "actor",            // 1    [#]
    "addammo",   // 2    [#]
    "ifrnd",            // 3    [C]
    "enda",             // 4    [:]
    "ifcansee",         // 5    [C]
    "ifhitweapon",      // 6    [#]
    "action",           // 7    [#]
    "ifpdistl",         // 8    [#]
    "ifpdistg",         // 9    [#]
    "else",             // 10   [#]
    "strength",         // 11   [#]
    "break",            // 12   [#]
    "shoot",            // 13   [#]
    "palfrom",          // 14   [#]
    "sound",            // 15   [filename.voc]
    "fall",             // 16   []
    "state",            // 17
    "ends",             // 18
    "define",           // 19
    "//",               // 20
    "ifai",             // 21
    "killit",           // 22
    "addweapon",        // 23
    "ai",               // 24
    "addphealth",       // 25
    "ifdead",           // 26
    "ifsquished",       // 27
    "sizeto",           // 28
    "{",                // 29
    "}",                // 30
    "spawn",            // 31
    "move",             // 32
    "ifwasweapon",      // 33
    "ifaction",         // 34
    "ifactioncount",    // 35
    "resetactioncount", // 36
    "debris",           // 37
    "pstomp",           // 38
    "/*",               // 39
    "cstat",            // 40
    "ifmove",           // 41
    "resetplayer",      // 42
    "ifonwater",        // 43
    "ifinwater",        // 44
    "ifcanshoottarget", // 45
    "ifcount",          // 46
    "resetcount",       // 47
    "addinventory",     // 48
    "ifactornotstayput",// 49
    "hitradius",        // 50
    "ifp",              // 51
    "count",            // 52
    "ifactor",          // 53
    "music",            // 54
    "include",          // 55
    "ifstrength",       // 56
    "definesound",      // 57
    "guts",             // 58
    "ifspawnedby",      // 59
    "gamestartup",      // 60
    "wackplayer",       // 61
    "ifgapzl",          // 62
    "ifhitspace",       // 63
    "ifoutside",        // 64
    "ifmultiplayer",    // 65
    "operate",          // 66
    "ifinspace",        // 67
    "debug",            // 68
    "endofgame",        // 69
    "ifbulletnear",     // 70
    "ifrespawn",        // 71
    "iffloordistl",     // 72
    "ifceilingdistl",   // 73
    "spritepal",        // 74
    "ifpinventory",     // 75
    "betaname",         // 76
    "cactor",           // 77
    "ifphealthl",       // 78
    "definequote",      // 79
    "quote",            // 80
    "ifinouterspace",   // 81
    "ifnotmoving",      // 82
    "respawnhitag",        // 83
    "tip",             // 84
    "ifspritepal",      // 85
    "feathers",         // 86
    "soundonce",         // 87
    "addkills",         // 88
    "stopsound",        // 89
    "ifawayfromwall",       // 90
    "ifcanseetarget",   // 91
    "globalsound",  // 92
    "lotsofglass", // 93
    "ifgotweaponce", // 94
    "getlastpal", // 95
    "pkick",  // 96
    "mikesnd", // 97
    "useractor",  // 98
    "sizeat",  // 99
    "addstrength", // 100   [#]
    "cstator", // 101
    "mail", // 102
    "paper", // 103
    "tossweapon", // 104
    "sleeptime", // 105
    "nullop", // 106
    "definevolumename", // 107
    "defineskillname", // 108
    "ifnosounds", // 109
    "ifnocover", // 110
    "ifhittruck", // 111
    "iftipcow", // 112
    "isdrunk", // 113
    "iseat", // 114
    "destroyit", // 115
    "larrybird", // 116
    "strafeleft", // 117
    "straferight", // 118
    "ifactorhealthg", // 119
    "ifactorhealthl", // 120
    "slapplayer", // 121
    "ifpdrunk", // 122
    "tearitup", // 123
    "smackbubba", // 124
    "soundtagonce", // 125
    "soundtag", // 126
    "ifsoundid", // 127
    "ifsounddist", // 128
    "ifonmud", // 129
    "ifcoop", // 130
#ifdef RRRA
    "ifmotofast", // 131
    "ifwind", // 132
    "smacksprite", // 133
    "ifonmoto", // 134
    "ifonboat", // 135
    "fakebubba", // 136
    "mamatrigger", // 137
    "mamaspawn", // 138
    "mamaquake", // 139
    "clipdist", // 140
    "mamaend", // 141
    "newpic", // 142
    "garybanjo", // 143
    "motoloopsnd", // 144
    "ifsizedown", // 145
    "rndmove" // 146
#endif
};


short getincangle(short a,short na)
{
    a &= 2047;
    na &= 2047;

    if(klabs(a-na) < 1024)
        return (na-a);
    else
    {
        if(na > 1024) na -= 2048;
        if(a > 1024) a -= 2048;

        na -= 2048;
        a -= 2048;
        return (na-a);
    }
}

char ispecial(char c)
{
    if(c == 0x0a)
    {
        line_number++;
        return 1;
    }

    if(c == ' ' || c == 0x0d)
        return 1;

    return 0;
}

char isaltok(char c)
{
    return ( isalnum(c) || c == '{' || c == '}' || c == '/' || c == '*' || c == '-' || c == '_' || c == '.');
}

void getglobalz(short i)
{
    long hz,lz,zr;

    spritetype *s = &sprite[i];

    if( s->statnum == 10 || s->statnum == 6 || s->statnum == 2 || s->statnum == 1 || s->statnum == 4)
    {
        if(s->statnum == 4)
            zr = 4L;
        else zr = 127L;

        getzrange(s->x,s->y,s->z-(FOURSLEIGHT),s->sectnum,&hittype[i].ceilingz,&hz,&hittype[i].floorz,&lz,zr,CLIPMASK0);

        if( (lz&49152) == 49152 && (sprite[lz&(MAXSPRITES-1)].cstat&48) == 0 )
        {
            lz &= (MAXSPRITES-1);
            if( badguy(&sprite[lz]) && sprite[lz].pal != 1)
            {
                if( s->statnum != 4 )
                {
                    hittype[i].dispicnum = -4; // No shadows on actors
                    s->xvel = -256;
                    ssp(i,CLIPMASK0);
                }
            }
            else if(sprite[lz].picnum == APLAYER && badguy(s) )
            {
                hittype[i].dispicnum = -4; // No shadows on actors
                s->xvel = -256;
                ssp(i,CLIPMASK0);
            }
            else if(s->statnum == 4 && sprite[lz].picnum == APLAYER)
                if(s->owner == lz)
            {
                hittype[i].ceilingz = sector[s->sectnum].ceilingz;
                hittype[i].floorz   = sector[s->sectnum].floorz;
            }
        }
    }
    else
    {
        hittype[i].ceilingz = sector[s->sectnum].ceilingz;
        hittype[i].floorz   = sector[s->sectnum].floorz;
    }
}


void makeitfall(short i)
{
    spritetype *s = &sprite[i];
    long hz,lz,c;

    if( floorspace(s->sectnum) )
        c = 0;
    else
    {
        if( ceilingspace(s->sectnum) || sector[s->sectnum].lotag == 2)
            c = gc/6;
        else c = gc;
    }

#ifdef RRRA
    if ((s->picnum == BIKERB || s->picnum == CHEERB) && c == gc)
        c = gc>>2;
    else if (s->picnum == BIKERBV2 && c == gc)
        c = gc>>3;
#endif

    if( ( s->statnum == 1 || s->statnum == 10 || s->statnum == 2 || s->statnum == 6 ) )
        getzrange(s->x,s->y,s->z-(FOURSLEIGHT),s->sectnum,&hittype[i].ceilingz,&hz,&hittype[i].floorz,&lz,127L,CLIPMASK0);
    else
    {
        hittype[i].ceilingz = sector[s->sectnum].ceilingz;
        hittype[i].floorz   = sector[s->sectnum].floorz;
    }

    if( s->z < hittype[i].floorz-(FOURSLEIGHT) )
    {
        if( sector[s->sectnum].lotag == 2 && s->zvel > 3122 )
            s->zvel = 3144;
        if(s->zvel < 6144)
            s->zvel += c;
        else s->zvel = 6144;
        s->z += s->zvel;
    }
    if( s->z >= hittype[i].floorz-(FOURSLEIGHT) )
    {
        s->z = hittype[i].floorz - FOURSLEIGHT;
        s->zvel = 0;
    }
}


void getlabel(void)
{
    long i;

    while( isalnum(*textptr) == 0 )
    {
        if(*textptr == 0x0a) line_number++;
        textptr++;
        if( *textptr == 0)
            return;
    }

    i = 0;
    while( ispecial(*textptr) == 0 )
        label[(labelcnt<<6)+i++] = *(textptr++);

    label[(labelcnt<<6)+i] = 0;
}

long keyword(void)
{
    long i;
    char *temptextptr;

    temptextptr = textptr;

    while( isaltok(*temptextptr) == 0 )
    {
        temptextptr++;
        if( *temptextptr == 0 )
            return 0;
    }

    i = 0;
    while( isaltok(*temptextptr) )
    {
        tempbuf[i] = *(temptextptr++);
        i++;
    }
    tempbuf[i] = 0;

    for(i=0;i<NUMKEYWORDS;i++)
        if( strcmp( tempbuf,keyw[i]) == 0 )
            return i;

    return -1;
}

long transword(void) //Returns its code #
{
    long i, l;

    while( isaltok(*textptr) == 0 )
    {
        if(*textptr == 0x0a) line_number++;
        if( *textptr == 0 )
            return -1;
        textptr++;
    }

    l = 0;
    while( isaltok(*(textptr+l)) )
    {
        tempbuf[l] = textptr[l];
        l++;
    }
    tempbuf[l] = 0;

    for(i=0;i<NUMKEYWORDS;i++)
    {
        if( strcmp( tempbuf,keyw[i]) == 0 )
        {
            *scriptptr = i;
            textptr += l;
            scriptptr++;
            return i;
        }
    }

    textptr += l;

    if( tempbuf[0] == '{' && tempbuf[1] != 0)
        printf("  * ERROR!(L%ld) Expecting a SPACE or CR between '{' and '%s'.\n",line_number,tempbuf+1);
    else if( tempbuf[0] == '}' && tempbuf[1] != 0)
        printf("  * ERROR!(L%ld) Expecting a SPACE or CR between '}' and '%s'.\n",line_number,tempbuf+1);
    else if( tempbuf[0] == '/' && tempbuf[1] == '/' && tempbuf[2] != 0 )
        printf("  * ERROR!(L%ld) Expecting a SPACE between '//' and '%s'.\n",line_number,tempbuf+2);
    else if( tempbuf[0] == '/' && tempbuf[1] == '*' && tempbuf[2] != 0 )
        printf("  * ERROR!(L%ld) Expecting a SPACE between '/*' and '%s'.\n",line_number,tempbuf+2);
    else if( tempbuf[0] == '*' && tempbuf[1] == '/' && tempbuf[2] != 0 )
        printf("  * ERROR!(L%ld) Expecting a SPACE between '*/' and '%s'.\n",line_number,tempbuf+2);
    else printf("  * ERROR!(L%ld) Expecting key word, but found '%s'.\n",line_number,tempbuf);

    error++;
    return -1;
}

void transnum(void)
{
    long i, l;

    while( isaltok(*textptr) == 0 )
    {
        if(*textptr == 0x0a) line_number++;
        textptr++;
        if( *textptr == 0 )
            return;
    }


    l = 0;
    while( isaltok(*(textptr+l)) )
    {
        tempbuf[l] = textptr[l];
        l++;
    }
    tempbuf[l] = 0;

    for(i=0;i<NUMKEYWORDS;i++)
        if( strcmp( label+(labelcnt<<6),keyw[i]) == 0 )
    {
        error++;
        printf("  * ERROR!(L%ld) Symbol '%s' is a key word.\n",line_number,label+(labelcnt<<6));
        textptr+=l;
    }


    for(i=0;i<labelcnt;i++)
    {
        if( strcmp(tempbuf,label+(i<<6)) == 0 )
        {
            *scriptptr = labelcode[i];
            scriptptr++;
            textptr += l;
            return;
        }
    }

    if( isdigit(*textptr) == 0 && *textptr != '-')
    {
        printf("  * ERROR!(L%ld) Parameter '%s' is undefined.\n",line_number,tempbuf);
        error++;
        textptr+=l;
        return;
    }

    *scriptptr = atol(textptr);
    scriptptr++;

    textptr += l;
}


char parsecommand(void)
{
    long i, j, k, *tempscrptr;
    char done, *origtptr, temp_ifelse_check, tw;
    short temp_line_number;
    int fp;

    if( error > 12 || ( *textptr == '\0' ) || ( *(textptr+1) == '\0' ) ) return 1;

    tw = transword();

    switch(tw)
    {
        default:
        case -1:
            return 0; //End
        case 39:      //Rem endrem
            scriptptr--;
            j = line_number;
            do
            {
                textptr++;
                if(*textptr == 0x0a) line_number++;
                if( *textptr == 0 )
                {
                    printf("  * ERROR!(L%ld) Found '/*' with no '*/'.\n",j,label+(labelcnt<<6));
                    error++;
                    return 0;
                }
            }
            while( *textptr != '*' || *(textptr+1) != '/' );
            textptr+=2;
            return 0;
        case 17:
            if( parsing_actor == 0 && parsing_state == 0 )
            {
                getlabel();
                scriptptr--;
                labelcode[labelcnt] = (long) scriptptr;
                labelcnt++;

                parsing_state = 1;

                return 0;
            }

            getlabel();

            for(i=0;i<NUMKEYWORDS;i++)
                if( strcmp( label+(labelcnt<<6),keyw[i]) == 0 )
                {
                    error++;
                    printf("  * ERROR!(L%ld) Symbol '%s' is a key word.\n",line_number,label+(labelcnt<<6));
                    return 0;
                }

            for(j=0;j<labelcnt;j++)
            {
                if( strcmp(label+(j<<6),label+(labelcnt<<6)) == 0 )
                {
                    *scriptptr = labelcode[j];
                    break;
                }
            }

            if(j==labelcnt)
            {
                printf("  * ERROR!(L%ld) State '%s' not found.\n",line_number,label+(labelcnt<<6));
                error++;
            }
            scriptptr++;
            return 0;

        case 15:
        case 92:
        case 87:
        case 89:
        case 93:
            transnum();
            return 0;

        case 18:
            if( parsing_state == 0 )
            {
                printf("  * ERROR!(L%ld) Found 'ends' with no 'state'.\n",line_number);
                error++;
            }
//            else
            {
                if( num_squigilly_brackets > 0 )
                {
                    printf("  * ERROR!(L%ld) Found more '{' than '}' before 'ends'.\n",line_number);
                    error++;
                }
                if( num_squigilly_brackets < 0 )
                {
                    printf("  * ERROR!(L%ld) Found more '}' than '{' before 'ends'.\n",line_number);
                    error++;
                }
                parsing_state = 0;
            }
            return 0;
        case 19:
            getlabel();
            // Check to see it's already defined

            for(i=0;i<NUMKEYWORDS;i++)
                if( strcmp( label+(labelcnt<<6),keyw[i]) == 0 )
                {
                    error++;
                    printf("  * ERROR!(L%ld) Symbol '%s' is a key word.\n",line_number,label+(labelcnt<<6));
                    return 0;
                }

            for(i=0;i<labelcnt;i++)
            {
                if( strcmp(label+(labelcnt<<6),label+(i<<6)) == 0 )
                {
                    warning++;
                    printf("  * WARNING.(L%ld) Duplicate definition '%s' ignored.\n",line_number,label+(labelcnt<<6));
                    break;
                }
            }

            transnum();
            if(i == labelcnt)
                labelcode[labelcnt++] = *(scriptptr-1);
            scriptptr -= 2;
            return 0;
        case 14:

            for(j = 0;j < 4;j++)
            {
                if( keyword() == -1 )
                    transnum();
                else break;
            }

            while(j < 4)
            {
                *scriptptr = 0;
                scriptptr++;
                j++;
            }
            return 0;

        case 32:
            if( parsing_actor || parsing_state )
            {
                transnum();

                j = 0;
                while(keyword() == -1)
                {
                    transnum();
                    scriptptr--;
                    j |= *scriptptr;
                }
                *scriptptr = j;
                scriptptr++;
            }
            else
            {
                scriptptr--;
                getlabel();
                // Check to see it's already defined

                for(i=0;i<NUMKEYWORDS;i++)
                    if( strcmp( label+(labelcnt<<6),keyw[i]) == 0 )
                {
                    error++;
                    printf("  * ERROR!(L%ld) Symbol '%s' is a key word.\n",line_number,label+(labelcnt<<6));
                    return 0;
                }

                for(i=0;i<labelcnt;i++)
                    if( strcmp(label+(labelcnt<<6),label+(i<<6)) == 0 )
                    {
                        warning++;
                        printf("  * WARNING.(L%ld) Duplicate move '%s' ignored.\n",line_number,label+(labelcnt<<6));
                        break;
                    }
                if(i == labelcnt)
                    labelcode[labelcnt++] = (long) scriptptr;
                for(j=0;j<2;j++)
                {
                    if(keyword() >= 0) break;
                    transnum();
                }
                for(k=j;k<2;k++)
                {
                    *scriptptr = 0;
                    scriptptr++;
                }
            }
            return 0;

        case 54:
            {
                scriptptr--;
                transnum(); // Volume Number (0/4)
                scriptptr--;

                k = *scriptptr-1;

                if(k >= 0) // if it's background music
                {
                    i = 0;
                    while(keyword() == -1)
                    {
                        while( isaltok(*textptr) == 0 )
                        {
                            if(*textptr == 0x0a) line_number++;
                            textptr++;
                            if( *textptr == 0 ) break;
                        }
                        j = 0;
                        while( isaltok(*(textptr+j)) )
                        {
                            music_fn[k][i][j] = textptr[j];
                            j++;
                        }
                        music_fn[k][i][j] = '\0';
                        textptr += j;
                        if(i > 9) break;
                        i++;
                    }
                }
                else
                {
                    i = 0;
                    while(keyword() == -1)
                    {
                        while( isaltok(*textptr) == 0 )
                        {
                            if(*textptr == 0x0a) line_number++;
                            textptr++;
                            if( *textptr == 0 ) break;
                        }
                        j = 0;
                        while( isaltok(*(textptr+j)) )
                        {
                            env_music_fn[i][j] = textptr[j];
                            j++;
                        }
                        env_music_fn[i][j] = '\0';

                        textptr += j;
                        if(i > 9) break;
                        i++;
                    }
                }
            }
            return 0;
        case 55:
            scriptptr--;
            while( isaltok(*textptr) == 0 )
            {
                if(*textptr == 0x0a) line_number++;
                textptr++;
                if( *textptr == 0 ) break;
            }
            j = 0;
            while( isaltok(*textptr) )
            {
                tempbuf[j] = *(textptr++);
                j++;
            }
            tempbuf[j] = '\0';

            fp = kopen4load(tempbuf,loadfromgrouponly);
            if(fp == 0)
            {
                error++;
                printf("  * ERROR!(L%ld) Could not find '%s'.\n",line_number,label+(labelcnt<<6));
                return 0;
            }

            j = kfilelength(fp);

            printf("Including: '%s'.\n",tempbuf);

            temp_line_number = line_number;
            line_number = 1;
            temp_ifelse_check = checking_ifelse;
            checking_ifelse = 0;
            origtptr = textptr;
            textptr = last_used_text+last_used_size;

            *(textptr+j) = 0;

            kread(fp,(char *)textptr,j);
            kclose(fp);

            do
                done = parsecommand();
            while( done == 0 );

            textptr = origtptr;
            total_lines += line_number;
            line_number = temp_line_number;
            checking_ifelse = temp_ifelse_check;

            return 0;
        case 24:
            if( parsing_actor || parsing_state )
                transnum();
            else
            {
                scriptptr--;
                getlabel();

                for(i=0;i<NUMKEYWORDS;i++)
                    if( strcmp( label+(labelcnt<<6),keyw[i]) == 0 )
                    {
                        error++;
                        printf("  * ERROR!(L%ld) Symbol '%s' is a key word.\n",line_number,label+(labelcnt<<6));
                        return 0;
                    }

                for(i=0;i<labelcnt;i++)
                    if( strcmp(label+(labelcnt<<6),label+(i<<6)) == 0 )
                    {
                        warning++;
                        printf("  * WARNING.(L%ld) Duplicate ai '%s' ignored.\n",line_number,label+(labelcnt<<6));
                        break;
                    }

                if(i == labelcnt)
                    labelcode[labelcnt++] = (long) scriptptr;

                for(j=0;j<3;j++)
                {
                    if(keyword() >= 0) break;
                    if(j == 2)
                    {
                        k = 0;
                        while(keyword() == -1)
                        {
                            transnum();
                            scriptptr--;
                            k |= *scriptptr;
                        }
                        *scriptptr = k;
                        scriptptr++;
                        return 0;
                    }
                    else transnum();
                }
                for(k=j;k<3;k++)
                {
                    *scriptptr = 0;
                    scriptptr++;
                }
            }
            return 0;

        case 7:
            if( parsing_actor || parsing_state )
                transnum();
            else
            {
                scriptptr--;
                getlabel();
                // Check to see it's already defined

                for(i=0;i<NUMKEYWORDS;i++)
                    if( strcmp( label+(labelcnt<<6),keyw[i]) == 0 )
                    {
                        error++;
                        printf("  * ERROR!(L%ld) Symbol '%s' is a key word.\n",line_number,label+(labelcnt<<6));
                        return 0;
                    }

                for(i=0;i<labelcnt;i++)
                    if( strcmp(label+(labelcnt<<6),label+(i<<6)) == 0 )
                    {
                        warning++;
                        printf("  * WARNING.(L%ld) Duplicate action '%s' ignored.\n",line_number,label+(labelcnt<<6));
                        break;
                    }

                if(i == labelcnt)
                    labelcode[labelcnt++] = (long) scriptptr;

                for(j=0;j<5;j++)
                {
                    if(keyword() >= 0) break;
                    transnum();
                }
                for(k=j;k<5;k++)
                {
                    *scriptptr = 0;
                    scriptptr++;
                }
            }
            return 0;

        case 1:
            if( parsing_state )
            {
                printf("  * ERROR!(L%ld) Found 'actor' within 'state'.\n",line_number);
                error++;
            }

            if( parsing_actor )
            {
                printf("  * ERROR!(L%ld) Found 'actor' within 'actor'.\n",line_number);
                error++;
            }

            num_squigilly_brackets = 0;
            scriptptr--;
            parsing_actor = scriptptr;

            transnum();
            scriptptr--;
            actorscrptr[*scriptptr] = parsing_actor;

            for(j=0;j<4;j++)
            {
                *(parsing_actor+j) = 0;
                if(j == 3)
                {
                    j = 0;
                    while(keyword() == -1)
                    {
                        transnum();
                        scriptptr--;
                        j |= *scriptptr;
                    }
                    *scriptptr = j;
                    scriptptr++;
                    break;
                }
                else
                {
                    if(keyword() >= 0)
                    {
                        scriptptr += (4-j);
                        break;
                    }
                    transnum();

                    *(parsing_actor+j) = *(scriptptr-1);
                }
            }

            checking_ifelse = 0;

            return 0;

        case 98:

            if( parsing_state )
            {
                printf("  * ERROR!(L%ld) Found 'useritem' within 'state'.\n",line_number);
                error++;
            }

            if( parsing_actor )
            {
                printf("  * ERROR!(L%ld) Found 'useritem' within 'actor'.\n",line_number);
                error++;
            }

            num_squigilly_brackets = 0;
            scriptptr--;
            parsing_actor = scriptptr;

            transnum();
            scriptptr--;
            j = *scriptptr;

            transnum();
            scriptptr--;
            actorscrptr[*scriptptr] = parsing_actor;
            actortype[*scriptptr] = j;

            for(j=0;j<4;j++)
            {
                *(parsing_actor+j) = 0;
                if(j == 3)
                {
                    j = 0;
                    while(keyword() == -1)
                    {
                        transnum();
                        scriptptr--;
                        j |= *scriptptr;
                    }
                    *scriptptr = j;
                    scriptptr++;
                    break;
                }
                else
                {
                    if(keyword() >= 0)
                    {
                        scriptptr += (4-j);
                        break;
                    }
                    transnum();

                    *(parsing_actor+j) = *(scriptptr-1);
                }
            }

            checking_ifelse = 0;

            return 0;



        case 11:
        case 13:
        case 25:
        case 31:
        case 40:
        case 52:
        case 69:
        case 74:
        case 77:
        case 80:
        case 86:
        case 88:
        case 68:
        case 100:
        case 101:
        case 102:
        case 103:
        case 105:
        case 113:
        case 114:
#ifdef RRRA
        case 140:
        case 142:
#endif
            transnum();
            return 0;

        case 2:
        case 23:
        case 28:
        case 99:
        case 37:
        case 48:
        case 58:
            transnum();
            transnum();
            break;
        case 50:
            transnum();
            transnum();
            transnum();
            transnum();
            transnum();
            break;
        case 10:
            if( checking_ifelse )
            {
                checking_ifelse--;
                tempscrptr = scriptptr;
                scriptptr++; //Leave a spot for the fail location
                parsecommand();
                *tempscrptr = (long) scriptptr;
            }
            else
            {
                scriptptr--;
                warning++;
                printf("  * WARNING.(L%ld) Found 'else' with no 'if', ignored.\n",line_number);
            }

            return 0;

        case 75:
            transnum();
        case 3:
        case 8:
        case 9:
        case 21:
        case 33:
        case 34:
        case 35:
        case 41:
        case 46:
        case 53:
        case 56:
        case 59:
        case 62:
        case 72:
        case 73:
//        case 74:
        case 78:
        case 85:
        case 94:
        case 119:
        case 120:
        case 127:
        case 128:
            transnum();
        case 43:
        case 44:
        case 49:
        case 5:
        case 6:
        case 27:
        case 26:
        case 45:
        case 51:
        case 63:
        case 64:
        case 65:
        case 67:
        case 70:
        case 71:
        case 81:
        case 82:
        case 90:
        case 91:
        case 109:
        case 110:
        case 111:
        case 112:
        case 129:
        case 130:
#ifdef RRRA
        case 131:
        case 132:
        case 134:
        case 135:
        case 145:
#endif

            if(tw == 51)
            {
                j = 0;
                do
                {
                    transnum();
                    scriptptr--;
                    j |= *scriptptr;
                }
                while(keyword() == -1);
                *scriptptr = j;
                scriptptr++;
            }

            tempscrptr = scriptptr;
            scriptptr++; //Leave a spot for the fail location

            do
            {
                j = keyword();
                if(j == 20 || j == 39)
                    parsecommand();
            } while(j == 20 || j == 39);

            parsecommand();

            *tempscrptr = (long) scriptptr;

            checking_ifelse++;
            return 0;
        case 29:
            num_squigilly_brackets++;
            do
                done = parsecommand();
            while( done == 0 );
            return 0;
        case 30:
            num_squigilly_brackets--;
            if( num_squigilly_brackets < 0 )
            {
                printf("  * ERROR!(L%ld) Found more '}' than '{'.\n",line_number);
                error++;
            }
            return 1;
        case 76:
            scriptptr--;
            j = 0;
            while( *textptr != 0x0a )
            {
                betaname[j] = *textptr;
                j++; textptr++;
            }
            betaname[j] = 0;
            return 0;
        case 20:
            scriptptr--; //Negate the rem
            while( *textptr != 0x0a )
                textptr++;

            // line_number++;
            return 0;

        case 107:
            scriptptr--;
            transnum();
            scriptptr--;
            j = *scriptptr;
            while( *textptr == ' ' ) textptr++;

            i = 0;

            while( *textptr != 0x0a )
            {
                volume_names[j][i] = toupper(*textptr);
                textptr++,i++;
                if(i >= 32)
                {
                    printf("  * ERROR!(L%ld) Volume name exceeds character size limit of 32.\n",line_number);
                    error++;
                    while( *textptr != 0x0a ) textptr++;
                    break;
                }
            }
            volume_names[j][i-1] = '\0';
            return 0;
        case 108:
            scriptptr--;
            transnum();
            scriptptr--;
            j = *scriptptr;
            while( *textptr == ' ' ) textptr++;

            i = 0;

            while( *textptr != 0x0a )
            {
                skill_names[j][i] = toupper(*textptr);
                textptr++,i++;
                if(i >= 32)
                {
                    printf("  * ERROR!(L%ld) Skill name exceeds character size limit of 32.\n",line_number);
                    error++;
                    while( *textptr != 0x0a ) textptr++;
                    break;
                }
            }
            skill_names[j][i-1] = '\0';
            return 0;

        case 0:
            scriptptr--;
            transnum();
            scriptptr--;
            j = *scriptptr;
            transnum();
            scriptptr--;
            k = *scriptptr;
            while( *textptr == ' ' ) textptr++;

            i = 0;
            while( *textptr != ' ' && *textptr != 0x0a )
            {
                level_file_names[j*7+k][i] = *textptr;
                textptr++,i++;
                if(i > 127)
                {
                    printf("  * ERROR!(L%ld) Level file name exceeds character size limit of 128.\n",line_number);
                    error++;
                    while( *textptr != ' ') textptr++;
                    break;
                }
            }
            level_names[j*7+k][i-1] = '\0';

            while( *textptr == ' ' ) textptr++;

            partime[j*7+k] =
                (((*(textptr+0)-'0')*10+(*(textptr+1)-'0'))*26*60)+
                (((*(textptr+3)-'0')*10+(*(textptr+4)-'0'))*26);

            textptr += 5;
            while( *textptr == ' ' ) textptr++;

            designertime[j*7+k] =
                (((*(textptr+0)-'0')*10+(*(textptr+1)-'0'))*26*60)+
                (((*(textptr+3)-'0')*10+(*(textptr+4)-'0'))*26);

            textptr += 5;
            while( *textptr == ' ' ) textptr++;

            i = 0;

            while( *textptr != 0x0a )
            {
                level_names[j*7+k][i] = toupper(*textptr);
                textptr++,i++;
                if(i >= 32)
                {
                    printf("  * ERROR!(L%ld) Level name exceeds character size limit of 32.\n",line_number);
                    error++;
                    while( *textptr != 0x0a ) textptr++;
                    break;
                }
            }
            level_names[j*7+k][i-1] = '\0';
            return 0;

        case 79:
            scriptptr--;
            transnum();
            k = *(scriptptr-1);
            if(k >= NUMOFFIRSTTIMEACTIVE)
            {
                printf("  * ERROR!(L%ld) Quote amount exceeds limit of %ld characters.\n",line_number,NUMOFFIRSTTIMEACTIVE);
                error++;
            }
            scriptptr--;
            i = 0;
            while( *textptr == ' ' )
                textptr++;

            while( *textptr != 0x0a )
            {
                fta_quotes[k][i] = *textptr;
                textptr++,i++;
                if(i >= 64)
                {
                    printf("  * ERROR!(L%ld) Quote exceeds character size limit of 64.\n",line_number);
                    error++;
                    while( *textptr != 0x0a ) textptr++;
                    break;
                }
            }
            fta_quotes[k][i] = '\0';
            return 0;
        case 57:
            scriptptr--;
            transnum();
            k = *(scriptptr-1);
            if(k >= NUM_SOUNDS)
            {
                printf("  * ERROR!(L%ld) Exceeded sound limit of %ld.\n",line_number,NUM_SOUNDS);
                error++;
            }
            scriptptr--;
            i = 0;
            while( *textptr == ' ')
                textptr++;

            while( *textptr != ' ' )
            {
                sounds[k][i] = *textptr;
                textptr++,i++;
                if(i >= 13)
                {
                    puts(sounds[k]);
                    printf("  * ERROR!(L%ld) Sound filename exceeded limit of 13 characters.\n",line_number);
                    error++;
                    while( *textptr != ' ' ) textptr++;
                    break;
                }
            }
            sounds[k][i] = '\0';

            transnum();
            soundps[k] = *(scriptptr-1);
            scriptptr--;
            transnum();
            soundpe[k] = *(scriptptr-1);
            scriptptr--;
            transnum();
            soundpr[k] = *(scriptptr-1);
            scriptptr--;
            transnum();
            soundm[k] = *(scriptptr-1);
            scriptptr--;
            transnum();
            soundvo[k] = *(scriptptr-1);
            scriptptr--;
            return 0;
        
        case 4:
            if( parsing_actor == 0 )
            {
                printf("  * ERROR!(L%ld) Found 'enda' without defining 'actor'.\n",line_number);
                error++;
            }
//            else
            {
                if( num_squigilly_brackets > 0 )
                {
                    printf("  * ERROR!(L%ld) Found more '{' than '}' before 'enda'.\n",line_number);
                    error++;
                }
                parsing_actor = 0;
            }

            return 0;
        case 12:
        case 16:
        case 84:
//        case 21:
        case 22:    //KILLIT
        case 36:
        case 38:
        case 42:
        case 47:
        case 61:
        case 66:
        case 83:
        case 95:
        case 96:
        case 97:
        case 104:
        case 106:
        case 115:
        case 116:
        case 117:
        case 118:
        case 121:
        //case 122:
        case 123:
        case 124:
        case 125:
        case 126:
#ifdef RRRA
        case 133:
        case 136:
        case 137:
        case 138:
        case 139:
        case 141:
        case 143:
        case 144:
        case 146:
#endif
            return 0;
        case 60:
            j = 0;
#ifdef RRRA
            while(j < 34)
#else
            while(j < 31)
#endif
            {
                transnum();
                scriptptr--;

                switch(j)
                {
                    case 0:
                        ud.const_visibility = *scriptptr;
                        break;
                    case 1:
                        impact_damage = *scriptptr;
                        break;
                    case 2:
                        max_player_health = *scriptptr;
                        break;
                    case 3:
                        max_armour_amount = *scriptptr;
                        break;
                    case 4:
                        respawnactortime = *scriptptr;break;
                    case 5:
                        respawnitemtime = *scriptptr;break;
                    case 6:
                        rdnkfriction = *scriptptr;break;
                    case 7:
                        gc = *scriptptr;break;
                    case 8:rpgblastradius = *scriptptr;break;
                    case 9:pipebombblastradius = *scriptptr;break;
                    case 10:shrinkerblastradius = *scriptptr; break;
                    case 11:powderkegblastradius = *scriptptr; break;
                    case 12:morterblastradius = *scriptptr;break;
                    case 13:bouncemineblastradius = *scriptptr;break;
                    case 14:seenineblastradius = *scriptptr;break;

                    case 15:
                    case 16:
                    case 17:
                    case 18:
                    case 19:
                    case 20:
                    case 21:
                    case 22:
                    case 23:
                    case 24:
                    case 25:
                        if(j == 24)
                            max_ammo_amount[11] = *scriptptr;
                        else if(j == 25)
                            max_ammo_amount[12] = *scriptptr;
                        else max_ammo_amount[j-14] = *scriptptr;
                        break;
                    case 26:
                        camerashitable = *scriptptr;
                        break;
                    case 27:
                        numfreezebounces = *scriptptr;
                        break;
                    case 28:
                        freezerhurtowner = *scriptptr;
                        break;
                    case 29:
                        spriteqamount = *scriptptr;
                        if(spriteqamount > 1024) spriteqamount = 1024;
                        else if(spriteqamount < 0) spriteqamount = 0;
                        break;
                    case 30:
                        lasermode = *scriptptr;
                        break;
#ifdef RRRA
                    case 31:
                        max_ammo_amount[RA13_WEAPON] = *scriptptr;
                        break;
                    case 32:
                        max_ammo_amount[RA14_WEAPON] = *scriptptr;
                        break;
                    case 33:
                        max_ammo_amount[RA16_WEAPON] = *scriptptr;
                        break;
#endif
                }
                j++;
            }
            scriptptr++;
            return 0;
    }
    return 0;
}


void passone(void)
{

    while( parsecommand() == 0 );

    if( (error+warning) > 12)
        puts(  "  * ERROR! Too many warnings or errors.");

}

char *defaultcons[3] =
{
     {"GAME.CON"},
     {"USER.CON"},
     {"DEFS.CON"}
};

void copydefaultcons(void)
{
    long i, fs, fpi;
    FILE *fpo;

    for(i=0;i<3;i++)
    {
        fpi = kopen4load( defaultcons[i] , 1 );
        fpo = fopen( defaultcons[i],"wb");

        if(fpi == 0)
        {
            if(fpo == -1) fclose(fpo);
            continue;
        }
        if(fpo == -1)
        {
            if(fpi == 0) kclose(fpi);
            continue;
        }

        fs = kfilelength(fpi);

        kread(fpi,&hittype[0],fs);
        fwrite(&hittype[0],fs,1,fpo);

        kclose(fpi);
        fclose(fpo);
    }
}

void loadefs(char *filenam,char *mptr)
{
    int i;
    long fs,fp;

    if(!SafeFileExists(filenam) && loadfromgrouponly == 0)
    {
        puts("Missing external con file(s).");
        puts("COPY INTERNAL DEFAULTS TO DIRECTORY(Y/n)?");

        KB_FlushKeyboardQueue();
        while( KB_KeyWaiting() );

        i = KB_Getch();
        if(i == 'y' || i == 'Y' )
        {
            puts(" Yes");
            copydefaultcons();
        }
    }

    fp = kopen4load(filenam,loadfromgrouponly);
    if( fp == 0 )
    {
        if( loadfromgrouponly == 1 )
            gameexit("\nMissing con file(s).");

        loadfromgrouponly = 1;
        return; //Not there
    }
    else
    {
        printf("Compiling: '%s'.\n",filenam);

        fs = kfilelength(fp);

        last_used_text = textptr = (char *) mptr;
        last_used_size = fs;

        kread(fp,(char *)textptr,fs);
        kclose(fp);
    }

    textptr[fs - 2] = 0;

    clearbuf(actorscrptr,MAXSPRITES,0L);
    clearbufbyte(actortype,MAXSPRITES,0L);

    labelcnt = 0;
    scriptptr = script+1;
    warning = 0;
    error = 0;
    line_number = 1;
    total_lines = 0;

    passone(); //Tokenize
    *script = (long) scriptptr;

    if(warning|error)
        printf("Found %ld warning(s), %ld error(s).\n",warning,error);

    if( error == 0 && warning != 0)
    {
        if( groupfile != -1 && loadfromgrouponly == 0 )
        {
            printf("\nWarnings found in %s file.  You should backup the original copies before\n",filenam);
            puts("before attempting to modify them.  Do you want to use the");
            puts("INTERNAL DEFAULTS (y/N)?");

            KB_FlushKeyboardQueue();
            while( KB_KeyWaiting() );
            i = KB_Getch();
            if(i == 'y' || i == 'Y' )
            {
                loadfromgrouponly = 1;
                puts(" Yes");
                return;
            }
        }
    }

    if(error)
    {
        if( loadfromgrouponly )
        {
            sprintf(buf,"\nError in %s.",filenam);
            gameexit(buf);
        }
        else
        {
            if( groupfile != -1 && loadfromgrouponly == 0 )
            {
                printf("\nErrors found in %s file.  You should backup the original copies\n",filenam);
                puts("before attempting to modify them.  Do you want to use the");
                puts("internal defaults (Y/N)?");

                KB_FlushKeyboardQueue();
                while( !KB_KeyWaiting() );

                i = KB_Getch();
                if( i == 'y' || i == 'Y' )
                {
                    puts(" Yes");
                    loadfromgrouponly = 1;
                    return;
                }
                else gameexit("");
            }
        }
    }
    else
    {
        total_lines += line_number;
        printf("Code Size:%ld bytes(%ld labels).\n",(long)((scriptptr-script)<<2)-4,labelcnt);
    }
}

#ifdef RRRA

char byte_11A3E8 = 12;
char byte_11A3E9 = 12;
char byte_11A3EA = 12;
char byte_11A3FA = 0;
char *dword_1DC1C8, *dword_1DC1CC, *dword_1DC1D0, *dword_1DC1D4, *dword_1DC1D8;

void sub_86730(short unk)
{
    char table[768];
    short i;
    if (!byte_11A3FA)
    {
        byte_11A3FA = 1;
        dword_1DC1C8 = palookup[0];
        dword_1DC1CC = palookup[30];
        dword_1DC1D0 = palookup[33];
        dword_1DC1D4 = palookup[23];
        dword_1DC1D8 = palookup[8];
        for (i = 0; i < 256; i++)
        {
            table[i] = i;
        }
        makepalookup(50,table,byte_11A3E8,byte_11A3E9,byte_11A3EA,1);
        makepalookup(51,table,byte_11A3E8,byte_11A3E9,byte_11A3EA,1);
    }
    if (!unk)
    {
        palookup[0] = dword_1DC1C8;
        palookup[30] = dword_1DC1CC;
        palookup[33] = dword_1DC1D0;
        palookup[23] = dword_1DC1D4;
        palookup[8] = dword_1DC1D8;
    }
    else if (unk == 2)
    {
        palookup[0] = palookup[50];
        palookup[30] = palookup[51];
        palookup[33] = palookup[51];
        palookup[23] = palookup[51];
        palookup[8] = palookup[54];
    }
}
#endif