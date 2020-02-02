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

#import <Foundation/Foundation.h>

#include "compat.h"

#import "GrpFile.game.h"
#import "GameListSource.game.h"

@implementation GameListSource
- (id)init
{
    self = [super init];
    if (self) {
        list = [[NSMutableArray alloc] init];

        for (grpfile_t const * p = foundgrps; p; p=p->next) {
            [list addObject:[[GrpFile alloc] initWithGrpfile:p]];
        }
    }

    return self;
}

- (void)dealloc
{
    [list release];
    [super dealloc];
}

- (GrpFile*)grpAtIndex:(int)index
{
    return [list objectAtIndex:index];
}

- (int)findIndexForGrpname:(NSString*)grpname
{
    NSUInteger i, listcount = [list count];
    for (i=0; i<listcount; i++) {
        if ([[[list objectAtIndex:i] grpname] isEqual:grpname]) return i;
    }
    return -1;
}

- (id)tableView:(NSTableView *)aTableView
        objectValueForTableColumn:(NSTableColumn *)aTableColumn
            row:(NSInteger)rowIndex
{
    UNREFERENCED_PARAMETER(aTableView);

    NSParameterAssert((NSUInteger)rowIndex < [list count]);
    switch ([[aTableColumn identifier] intValue]) {
        case 0:	// name column
            return [[list objectAtIndex:rowIndex] name];
        case 1:	// grp column
            return [[list objectAtIndex:rowIndex] grpname];
        default: return nil;
    }
}

- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
    UNREFERENCED_PARAMETER(aTableView);

    return [list count];
}
@end

