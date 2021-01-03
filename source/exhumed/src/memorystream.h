//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2020 EDuke32 developers and contributors
Copyright (C) 2020 sirlemonhead, Nuke.YKT

This file is part of PCExhumed.

PCExhumed is free software; you can redistribute it and/or
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

#ifndef __memorystream_h__
#define __memorystream_h__

#include "compat.h"

class MemoryReadStream
{
    public:
        MemoryReadStream(uint8_t *buffer, bsize_t size);
        ~MemoryReadStream();

        uint8_t  GetByte();
        uint16_t GetUint16LE();
        uint32_t GetUint32LE();
        uint64_t GetUint64LE();
        uint8_t  PeekByte();
        bsize_t GetBytesRead();
        bsize_t GetBytesLeft();
        bsize_t GetBytes(void *buffer, bsize_t nBytes);
        void    SkipBytes(bsize_t nBytes);

    private:
        bsize_t  _size;
        uint8_t *_start;
        uint8_t *_currentOffset;
        uint8_t *_end;
};

#endif
