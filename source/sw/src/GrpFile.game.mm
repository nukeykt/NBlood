//-------------------------------------------------------------------------
/*
 Copyright (C) 2013 Jonathon Fowler <jf@jonof.id.au>

 This file is part of JFShadowWarrior

 Shadow Warrior is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
//-------------------------------------------------------------------------

#include "GrpFile.game.h"

@implementation GrpFile
- (id)initWithGrpfile:(grpfile_t const *)grpfile
{
    self = [super init];
    if (self) {
        fg = grpfile;
        namestring = [NSString stringWithCString:fg->type->name encoding:NSUTF8StringEncoding];
        [namestring retain];
        grpnamestring = [NSString stringWithCString:fg->filename encoding:NSUTF8StringEncoding];
        [grpnamestring retain];
    }
    return self;
}
- (void)dealloc
{
    [namestring release];
    [grpnamestring release];
    [super dealloc];
}
- (NSString *)name
{
    return namestring;
}
- (NSString *)grpname
{
    return grpnamestring;
}
- (grpfile_t const *)entryptr
{
    return fg;
}
@end
