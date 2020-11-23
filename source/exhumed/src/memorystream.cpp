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

#include "MemoryStream.h"
#include <stdlib.h>
#include <cstring>
#include <algorithm>

MemoryReadStream::MemoryReadStream(uint8_t *buffer, size_t size)
{
	_size  = size;
	_start = buffer;
	_end   = buffer + size;
	_currentOffset = _start;
}

MemoryReadStream::~MemoryReadStream()
{
}

void MemoryReadStream::SkipBytes(size_t nBytes)
{
	_currentOffset += nBytes;
}

size_t MemoryReadStream::GetBytesRead()
{
	return _currentOffset - _start;
}

size_t MemoryReadStream::GetBytesLeft()
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
	return ret;
}

uint16_t MemoryReadStream::GetUint16BE()
{
	uint16_t ret;
	memcpy(&ret, _currentOffset, sizeof(ret));
	_currentOffset += sizeof(ret);
	return _byteswap_ushort(ret);
}

uint32_t MemoryReadStream::GetUint32LE()
{
	uint32_t ret;
	memcpy(&ret, _currentOffset, sizeof(ret));
	_currentOffset += sizeof(ret);
	return ret;
}

uint32_t MemoryReadStream::GetUint32BE()
{
	uint32_t ret;
	memcpy(&ret, _currentOffset, sizeof(ret));
	_currentOffset += sizeof(ret);
	return _byteswap_ulong(ret);
}

uint64_t MemoryReadStream::GetUint64LE()
{
	uint64_t ret;
	memcpy(&ret, _currentOffset, sizeof(ret));
	_currentOffset += sizeof(ret);
	return ret;
}

uint64_t MemoryReadStream::GetUint64BE()
{
	uint64_t ret;
	memcpy(&ret, _currentOffset, sizeof(ret));
	_currentOffset += sizeof(ret);
	return _byteswap_uint64(ret);
}

uint8_t MemoryReadStream::PeekByte()
{
	return *_currentOffset;
}

size_t MemoryReadStream::GetBytes(void *buffer, size_t nBytes)
{
	if (!buffer) {
		return 0;
	}

	size_t count = std::min((size_t)(_end - _currentOffset), nBytes);

	memcpy(buffer, _currentOffset, count);
	_currentOffset += count;
	return count;
}

// Write stream implementation
MemoryWriteStream::MemoryWriteStream(uint8_t *buffer, size_t size)
{
	_size  = size;
	_start = buffer;
	_end   = buffer + size;
	_currentOffset = _start;
}

MemoryWriteStream::~MemoryWriteStream()
{
}

void MemoryWriteStream::PutByte(uint8_t v)
{
	memcpy(_currentOffset, &v, sizeof(v));
	_currentOffset += sizeof(v);
}

void MemoryWriteStream::PutUint16LE(uint16_t v)
{
	memcpy(_currentOffset, &v, sizeof(v));
	_currentOffset += sizeof(v);
}

void MemoryWriteStream::PutUint16BE(uint16_t v)
{
	uint16_t _v = _byteswap_ushort(v);
	memcpy(_currentOffset, &_v, sizeof(_v));
	_currentOffset += sizeof(_v);
}

void MemoryWriteStream::PutUint32LE(uint32_t v)
{
	memcpy(_currentOffset, &v, sizeof(v));
	_currentOffset += sizeof(v);
}

void MemoryWriteStream::PutUint32BE(uint32_t v)
{
	uint32_t _v = _byteswap_ulong(v);
	memcpy(_currentOffset, &_v, sizeof(_v));
	_currentOffset += sizeof(_v);
}

void MemoryWriteStream::PutUint64LE(uint64_t v)
{
	memcpy(_currentOffset, &v, sizeof(v));
	_currentOffset += sizeof(v);
}

void MemoryWriteStream::PutUint64BE(uint64_t v)
{
	uint64_t _v = _byteswap_uint64(v);
	memcpy(_currentOffset, &_v, sizeof(_v));
	_currentOffset += sizeof(_v);
}

size_t MemoryWriteStream::GetBytesWritten()
{
	return _currentOffset - _start;
}

size_t MemoryWriteStream::PutBytes(const void *buffer, size_t nBytes)
{
	if (!buffer) {
		return 0;
	}

	size_t count = std::min((size_t)(_end - _currentOffset), nBytes);

	memcpy(_currentOffset, buffer, count);
	_currentOffset += count;
	return count;
}
