/*
 ****************************************************************************
 *  Copyright (c) 2026 FreeLSS contributors                                 *
 *	This file is part of FreeLSS.                                           *
 *                                                                          *
 *  FreeLSS is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, either version 3 of the License, or       *
 *  (at your option) any later version.                                     *
 *                                                                          *
 *  FreeLSS is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with FreeLSS.  If not, see <http://www.gnu.org/licenses/>.       *
 ****************************************************************************
*/

#pragma once

#include <stddef.h>

namespace freelss
{

/**
 * Narrow byte-stream port used by the G-code client so the transport
 * (POSIX serial, in-memory mock, ...) can be swapped without touching
 * protocol logic.  All methods are blocking up to the given timeout.
 */
class IByteStream
{
public:
	virtual ~IByteStream() {}

	/**
	 * Read up to `len` bytes into `buf`, blocking up to `timeoutMs` ms.
	 * @return number of bytes read (>=0); 0 on timeout; -1 on I/O error.
	 */
	virtual int read(void * buf, size_t len, int timeoutMs) = 0;

	/**
	 * Write `len` bytes from `buf`.  Blocking until written or error.
	 * @return number of bytes written; -1 on I/O error.
	 */
	virtual int write(const void * buf, size_t len) = 0;

	/** True when the underlying device is open and usable. */
	virtual bool isOpen() const = 0;
};

}
