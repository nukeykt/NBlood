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
#include "common_game.h"
#include "pqueue.h"

PriorityQueue::PriorityQueue()
{
    fNodeCount = 0;
}

void PriorityQueue::Upheap(void)
{
    queueItem item = queueItems[fNodeCount];
    queueItems[0].at0 = 0;
    unsigned int x = fNodeCount;
    while (queueItems[x>>1].at0 > item.at0)
    {
        queueItems[x] = queueItems[x>>1];
        x >>= 1;
    }
    queueItems[x] = item;
}

void PriorityQueue::Downheap(unsigned int n)
{
    queueItem item = queueItems[n];
    while ((unsigned int)(fNodeCount/2) >= n)
    {
        unsigned int t = n*2;
        if (t < (unsigned int)fNodeCount && queueItems[t].at0 > queueItems[t+1].at0)
            t++;
        if (item.at0 <= queueItems[t].at0)
            break;
        queueItems[n] = queueItems[t];
        n = t;
    }
    queueItems[n] = item;
}

void PriorityQueue::Delete(unsigned int k)
{
    dassert(k <= fNodeCount);
    queueItems[k] = queueItems[fNodeCount--];
    Downheap(k);
}

void PriorityQueue::Insert(unsigned long a1, unsigned long a2)
{
    dassert(fNodeCount < kPQueueSize);
    fNodeCount++;
    queueItems[fNodeCount].at0 = a1;
    queueItems[fNodeCount].at4 = a2;
    Upheap();
}

unsigned long PriorityQueue::Remove(void)
{
    unsigned long data = queueItems[1].at4;
    queueItems[1] = queueItems[fNodeCount--];
    Downheap(1);
    return data;
}