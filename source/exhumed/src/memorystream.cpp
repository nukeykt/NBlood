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

#include "memorystream.h"

MemoryReadStream::MemoryReadStream(uint8_t *buffer, bsize_t size)
{
    _size  = size;
    _start = buffer;
    _end   = buffer + size;
    _currentOffset = _start;
}

MemoryReadStream::~MemoryReadStream()
{
}

void MemoryReadStream::SkipBytes(bsize_t nBytes)
{
    _currentOffset += nBytes;
}

bsize_t MemoryReadStream::GetBytesRead()
{
    return _currentOffset - _start;
}

bsize_t MemoryReadStream::GetBytesLeft()
{
    return _end - _currentOffset;
}

uint8_t MemoryReadStream::GetByte()
{
    uint8_t ret;
    memcpy(&ret, _currentOffset, sizeof(ret));
    _currentOffset += sizeof(ret);
    return ret;
}

uint16_t MemoryReadStream::GetUint16LE()
{
    uint16_t ret;
    memcpy(&ret, _currentOffset, sizeof(ret));
    _currentOffset += sizeof(ret);
    return B_LITTLE16(ret);
}

uint32_t MemoryReadStream::GetUint32LE()
{
    uint32_t ret;
    memcpy(&ret, _currentOffset, sizeof(ret));
    _currentOffset += sizeof(ret);
    return B_LITTLE32(ret);
}

uint64_t MemoryReadStream::GetUint64LE()
{
    uint64_t ret;
    memcpy(&ret, _currentOffset, sizeof(ret));
    _currentOffset += sizeof(ret);
    return B_LITTLE64(ret);
}

uint8_t MemoryReadStream::PeekByte()
{
    return *_currentOffset;
}

bsize_t MemoryReadStream::GetBytes(void *buffer, bsize_t nBytes)
{
    if (!buffer) {
        return 0;
    }

    bsize_t count = std::min((bsize_t)(_end - _currentOffset), nBytes);

    memcpy(buffer, _currentOffset, count);
    _currentOffset += count;
    return count;
}
