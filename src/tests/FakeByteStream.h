/*
 ****************************************************************************
 *  Copyright (c) 2026 FreeLSS contributors                                 *
 *	This file is part of FreeLSS.                                           *
 *                                                                          *
 *  FreeLSS is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, either version 3 of the License, or       *
 *  (at your option) any later version.                                     *
 ****************************************************************************
*/

#pragma once

#include "../IByteStream.h"

#include <string>
#include <vector>
#include <cstring>
#include <algorithm>

namespace freelss
{

/**
 * In-memory IByteStream for unit tests.  Reads return bytes queued via
 * `queueResponse`; writes are accumulated in `written` so tests can
 * assert the exact wire format.  When no queued data is available a
 * read() returns 0 (timeout) and the timeout argument is otherwise
 * ignored so tests stay fast.
 */
class FakeByteStream : public IByteStream
{
public:
	FakeByteStream() : m_open(true), m_failNextWrite(false), m_failNextRead(false) {}

	void queueResponse(const std::string& bytes) { m_rxQueue += bytes; }

	void setOpen(bool open)               { m_open = open; }
	void setFailNextWrite(bool fail)      { m_failNextWrite = fail; }
	void setFailNextRead(bool fail)       { m_failNextRead = fail; }

	const std::string& written() const    { return m_written; }
	std::vector<std::string> writtenLines() const
	{
		std::vector<std::string> out;
		std::string cur;
		for (size_t i = 0; i < m_written.size(); i++)
		{
			char c = m_written[i];
			if (c == '\n')
			{
				out.push_back(cur);
				cur.clear();
			}
			else if (c != '\r')
			{
				cur.push_back(c);
			}
		}
		if (!cur.empty())
		{
			out.push_back(cur);
		}
		return out;
	}

	bool isOpen() const { return m_open; }

	int read(void * buf, size_t len, int /*timeoutMs*/)
	{
		if (!m_open) return -1;
		if (m_failNextRead)
		{
			m_failNextRead = false;
			return -1;
		}
		if (m_rxQueue.empty()) return 0;

		size_t n = std::min(len, m_rxQueue.size());
		std::memcpy(buf, m_rxQueue.data(), n);
		m_rxQueue.erase(0, n);
		return (int) n;
	}

	int write(const void * buf, size_t len)
	{
		if (!m_open) return -1;
		if (m_failNextWrite)
		{
			m_failNextWrite = false;
			return -1;
		}
		m_written.append(reinterpret_cast<const char *>(buf), len);
		return (int) len;
	}

private:
	std::string m_rxQueue;
	std::string m_written;
	bool m_open;
	bool m_failNextWrite;
	bool m_failNextRead;
};

}
