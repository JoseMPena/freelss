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

#include <string>
#include "IByteStream.h"

namespace freelss
{

/**
 * POSIX termios-backed serial port adapter (the only IByteStream
 * implementation linked into the freelss binary).  Uses select() so
 * reads can honour a timeout without blocking forever.
 */
class SerialPort : public IByteStream
{
public:
	SerialPort();
	~SerialPort();

	/**
	 * Open `devicePath` (e.g. "/dev/ttyACM0") at `baudRate` in raw 8N1
	 * mode.  Returns true on success.  Errors are logged via ErrorLog
	 * so callers only need the boolean for control-flow.
	 */
	bool open(const std::string& devicePath, int baudRate);

	void close();

	bool isOpen() const;

	int read(void * buf, size_t len, int timeoutMs);
	int write(const void * buf, size_t len);

	const std::string& getDevicePath() const { return m_devicePath; }

private:
	SerialPort(const SerialPort&);
	SerialPort& operator=(const SerialPort&);

	int m_fd;
	std::string m_devicePath;
};

}
