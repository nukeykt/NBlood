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

#ifndef _MemoryStream_h_
#define _MemoryStream_h_

#include <stdint.h>

class MemoryReadStream
{
	public:
		MemoryReadStream(uint8_t *buffer, size_t size);
		~MemoryReadStream();

		uint8_t  GetByte();
		uint16_t GetUint16LE();
		uint16_t GetUint16BE();
		uint32_t GetUint32LE();
		uint32_t GetUint32BE();
		uint64_t GetUint64LE();
		uint64_t GetUint64BE();
		uint8_t  PeekByte();
		size_t   GetBytesRead();
        size_t   GetBytesLeft();
		size_t   GetBytes(void *buffer, size_t nBytes);
		void     SkipBytes(size_t nBytes);

	private:
		size_t   _size;
		uint8_t *_start;
		uint8_t *_currentOffset;
		uint8_t *_end;
};

class MemoryWriteStream
{
	public:
		MemoryWriteStream(uint8_t *buffer, size_t size);
		~MemoryWriteStream();

		void  PutByte(uint8_t v);
		void PutUint16LE(uint16_t v);
		void PutUint16BE(uint16_t v);
		void PutUint32LE(uint32_t v);
		void PutUint32BE(uint32_t v);
		void PutUint64LE(uint64_t v);
		void PutUint64BE(uint64_t v);
		size_t GetBytesWritten();
		size_t PutBytes(const void *buffer, size_t nBytes);

	private:
		size_t   _size;
		uint8_t *_start;
		uint8_t *_currentOffset;
		uint8_t *_end;
};

#endif
