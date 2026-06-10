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

#include "GCodeClient.h"
#include "IByteStream.h"
#include "SerialPort.h"
#include "Logger.h"
#include "Thread.h"

#include <dirent.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>

namespace freelss
{

const char GCodeClient::READY_SENTINEL = '>';
const int  GCodeClient::DEFAULT_TIMEOUT_MS = 30000;

GCodeClient * GCodeClient::m_instance = NULL;

namespace
{

/**
 * The Arduino auto-resets when the port opens.  Give it time to boot
 * and ignore the banner; the protocol-level handshake (ping) is what
 * actually confirms liveness.
 */
const int ARDUINO_BOOT_WAIT_MS = 2000;

bool currentTimeMs(uint64_t& out)
{
	struct timeval tv;
	if (gettimeofday(&tv, NULL) != 0)
	{
		return false;
	}
	out = (uint64_t) tv.tv_sec * 1000ULL + (uint64_t) (tv.tv_usec / 1000);
	return true;
}

/** Pop the first complete line (without trailing \r\n) from `buffer`. */
bool popLine(std::string& buffer, std::string& line)
{
	size_t pos = buffer.find('\n');
	if (pos == std::string::npos)
	{
		return false;
	}
	line = buffer.substr(0, pos);
	buffer.erase(0, pos + 1);
	if (!line.empty() && line[line.size() - 1] == '\r')
	{
		line.erase(line.size() - 1);
	}
	return true;
}

/** Trim leading/trailing whitespace from `s` in place. */
void trim(std::string& s)
{
	size_t start = 0;
	while (start < s.size() && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n'))
	{
		start++;
	}
	size_t end = s.size();
	while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r' || s[end - 1] == '\n'))
	{
		end--;
	}
	s = s.substr(start, end - start);
}

std::vector<std::string> candidatePaths()
{
	std::vector<std::string> out;

	const char * dirs[] = { "/dev", NULL };
	const char * prefixes[] = { "ttyACM", "ttyUSB", NULL };

	for (size_t iDir = 0; dirs[iDir] != NULL; iDir++)
	{
		DIR * dir = opendir(dirs[iDir]);
		if (dir == NULL)
		{
			continue;
		}

		struct dirent * entry;
		while ((entry = readdir(dir)) != NULL)
		{
			std::string name = entry->d_name;
			for (size_t iPx = 0; prefixes[iPx] != NULL; iPx++)
			{
				if (name.compare(0, strlen(prefixes[iPx]), prefixes[iPx]) == 0)
				{
					std::string full = std::string(dirs[iDir]) + "/" + name;
					out.push_back(full);
					break;
				}
			}
		}

		closedir(dir);
	}

	return out;
}

}

GCodeClient::GCodeClient(IByteStream * stream, const std::string& devicePath) :
	m_stream(stream),
	m_devicePath(devicePath),
	m_rxBuffer(""),
	m_connected(stream != NULL && stream->isOpen())
{
}

GCodeClient::~GCodeClient()
{
	delete m_stream;
}

GCodeClient * GCodeClient::get()
{
	return m_instance;
}

void GCodeClient::install(IByteStream * stream)
{
	release();
	std::string path = "";
	m_instance = new GCodeClient(stream, path);
}

void GCodeClient::release()
{
	delete m_instance;
	m_instance = NULL;
}

bool GCodeClient::isOpen() const
{
	return m_stream != NULL && m_stream->isOpen();
}

bool GCodeClient::isConnected() const
{
	return m_connected && isOpen();
}

GCodeClient::SendResult GCodeClient::send(const std::string& line, int timeoutMs)
{
	if (m_stream == NULL || !m_stream->isOpen())
	{
		return SEND_DISCONNECTED;
	}

	InfoLog << "GCode >> " << line << Logger::ENDL;

	m_sentLines.push_back(line);

	std::string framed = line;
	if (framed.empty() || framed[framed.size() - 1] != '\n')
	{
		framed += '\n';
	}

	if (m_stream->write(framed.data(), framed.size()) < 0)
	{
		m_connected = false;
		return SEND_IO_ERROR;
	}

	uint64_t startMs = 0;
	if (!currentTimeMs(startMs))
	{
		startMs = 0;
	}

	char chunk[128];

	while (true)
	{
		// Drain anything buffered from a previous read first.  Multi-ack
		// responses (or stub byte streams that hand back several ">" at
		// once) must be consumed sequentially, one per send() call.
		std::string finished;
		while (popLine(m_rxBuffer, finished))
		{
			trim(finished);
			if (!finished.empty() && finished[0] == READY_SENTINEL)
			{
				return SEND_OK;
			}
		}

		uint64_t nowMs = startMs;
		currentTimeMs(nowMs);
		int elapsed = (int) (nowMs - startMs);
		int remaining = timeoutMs - elapsed;
		if (remaining <= 0)
		{
			return SEND_TIMEOUT;
		}

		int n = m_stream->read(chunk, sizeof(chunk), remaining);
		if (n < 0)
		{
			m_connected = false;
			return SEND_IO_ERROR;
		}
		if (n == 0)
		{
			// The underlying stream returned without data; either it was
			// a true select() timeout (then `remaining` has fully elapsed
			// and the next iteration will return SEND_TIMEOUT) or the
			// stub returned eagerly.  Yield to avoid burning CPU.
			Thread::usleep(1000);
			continue;
		}

		m_rxBuffer.append(chunk, n);
	}
}

bool GCodeClient::ping(int timeoutMs)
{
	if (!isOpen())
	{
		return false;
	}
	return send("M100", timeoutMs) == SEND_OK;
}

bool GCodeClient::initialize(const std::string& preferredPath, int baudRate)
{
	release();

	std::vector<std::string> candidates;
	if (!preferredPath.empty())
	{
		candidates.push_back(preferredPath);
	}

	std::vector<std::string> scanned = candidatePaths();
	for (size_t i = 0; i < scanned.size(); i++)
	{
		if (scanned[i] != preferredPath)
		{
			candidates.push_back(scanned[i]);
		}
	}

	for (size_t i = 0; i < candidates.size(); i++)
	{
		SerialPort * port = new SerialPort();
		if (!port->open(candidates[i], baudRate))
		{
			delete port;
			continue;
		}

		Thread::usleep((unsigned long) ARDUINO_BOOT_WAIT_MS * 1000UL);

		m_instance = new GCodeClient(port, candidates[i]);

		if (m_instance->ping(2000))
		{
			InfoLog << "GCodeClient: connected to Arduino on " << candidates[i] << Logger::ENDL;
			return true;
		}

		ErrorLog << "GCodeClient: " << candidates[i] << " did not answer M100; trying next candidate" << Logger::ENDL;
		release();
	}

	ErrorLog << "GCodeClient: no Arduino device responded; hardware control is disabled" << Logger::ENDL;
	return false;
}

}
