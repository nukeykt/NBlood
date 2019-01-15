//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT

This file is part of NBlood.

NBlood is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------
#pragma once
#define kPQueueSize 1024

class PriorityQueue
{
public:
    PriorityQueue();
    void Upheap(void);
    void Downheap(unsigned int);
    void Delete(unsigned int);
    void Insert(unsigned long, unsigned long);
    unsigned long Remove(void);

    struct queueItem
    {
        unsigned long at0; // priority
        unsigned long at4; // data
    } queueItems[kPQueueSize+1];

    int fNodeCount; // at2008
};