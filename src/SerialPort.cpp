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

#include "SerialPort.h"
#include "Logger.h"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>

namespace freelss
{

namespace
{

/** Translate an integer baud rate to a termios B<rate> constant. */
bool toSpeed(int baud, speed_t& out)
{
	switch (baud)
	{
	case 9600:   out = B9600;   return true;
	case 19200:  out = B19200;  return true;
	case 38400:  out = B38400;  return true;
	case 57600:  out = B57600;  return true;
	case 115200: out = B115200; return true;
#ifdef B230400
	case 230400: out = B230400; return true;
#endif
#ifdef B250000
	case 250000: out = B250000; return true;
#endif
#ifdef B500000
	case 500000: out = B500000; return true;
#endif
	default: return false;
	}
}

}

SerialPort::SerialPort() :
	m_fd(-1),
	m_devicePath("")
{
}

SerialPort::~SerialPort()
{
	close();
}

bool SerialPort::open(const std::string& devicePath, int baudRate)
{
	close();

	speed_t speed;
	if (!toSpeed(baudRate, speed))
	{
		ErrorLog << "SerialPort: unsupported baud rate " << baudRate << Logger::ENDL;
		return false;
	}

	int fd = ::open(devicePath.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd < 0)
	{
		ErrorLog << "SerialPort: failed to open " << devicePath << " (errno=" << errno << ")" << Logger::ENDL;
		return false;
	}

	struct termios tio;
	memset(&tio, 0, sizeof(tio));
	if (tcgetattr(fd, &tio) != 0)
	{
		ErrorLog << "SerialPort: tcgetattr failed on " << devicePath << " (errno=" << errno << ")" << Logger::ENDL;
		::close(fd);
		return false;
	}

	cfmakeraw(&tio);

	tio.c_cflag |= (CLOCAL | CREAD);
	tio.c_cflag &= ~CRTSCTS;
	tio.c_cflag &= ~CSIZE;
	tio.c_cflag |= CS8;
	tio.c_cflag &= ~PARENB;
	tio.c_cflag &= ~CSTOPB;

	tio.c_iflag &= ~(IXON | IXOFF | IXANY);
	tio.c_iflag &= ~(INLCR | ICRNL);

	tio.c_oflag &= ~OPOST;

	tio.c_cc[VMIN]  = 0;
	tio.c_cc[VTIME] = 0;

	if (cfsetispeed(&tio, speed) != 0 || cfsetospeed(&tio, speed) != 0)
	{
		ErrorLog << "SerialPort: cfsetispeed/ospeed failed (errno=" << errno << ")" << Logger::ENDL;
		::close(fd);
		return false;
	}

	if (tcsetattr(fd, TCSANOW, &tio) != 0)
	{
		ErrorLog << "SerialPort: tcsetattr failed on " << devicePath << " (errno=" << errno << ")" << Logger::ENDL;
		::close(fd);
		return false;
	}

	tcflush(fd, TCIOFLUSH);

	m_fd = fd;
	m_devicePath = devicePath;
	return true;
}

void SerialPort::close()
{
	if (m_fd >= 0)
	{
		::close(m_fd);
		m_fd = -1;
	}
	m_devicePath.clear();
}

bool SerialPort::isOpen() const
{
	return m_fd >= 0;
}

int SerialPort::read(void * buf, size_t len, int timeoutMs)
{
	if (m_fd < 0)
	{
		return -1;
	}

	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(m_fd, &rfds);

	struct timeval tv;
	tv.tv_sec  = timeoutMs / 1000;
	tv.tv_usec = (timeoutMs % 1000) * 1000;

	int sel = ::select(m_fd + 1, &rfds, NULL, NULL, &tv);
	if (sel < 0)
	{
		if (errno == EINTR)
		{
			return 0;
		}
		ErrorLog << "SerialPort: select() failed (errno=" << errno << ")" << Logger::ENDL;
		return -1;
	}
	if (sel == 0)
	{
		return 0;
	}

	ssize_t n = ::read(m_fd, buf, len);
	if (n < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
		{
			return 0;
		}
		ErrorLog << "SerialPort: read() failed (errno=" << errno << ")" << Logger::ENDL;
		return -1;
	}

	return (int) n;
}

int SerialPort::write(const void * buf, size_t len)
{
	if (m_fd < 0)
	{
		return -1;
	}

	const unsigned char * p = (const unsigned char *) buf;
	size_t remaining = len;

	while (remaining > 0)
	{
		ssize_t n = ::write(m_fd, p, remaining);
		if (n < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			{
				continue;
			}
			ErrorLog << "SerialPort: write() failed (errno=" << errno << ")" << Logger::ENDL;
			return -1;
		}

		p += n;
		remaining -= (size_t) n;
	}

	return (int) len;
}

}
