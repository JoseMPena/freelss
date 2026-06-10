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
#include <vector>

namespace freelss
{

class IByteStream;

/**
 * Synchronous request/response client for the Arduino Mega G-code
 * firmware: writes a single command line, then blocks until the
 * firmware emits its ready sentinel (a line containing only ">").
 *
 * The client owns its IByteStream so callers can swap a SerialPort for
 * an in-memory stub in tests.
 */
class GCodeClient
{
public:
	enum SendResult
	{
		SEND_OK = 0,
		SEND_TIMEOUT,
		SEND_IO_ERROR,
		SEND_DISCONNECTED
	};

	/** The Arduino firmware ready sentinel character. */
	static const char READY_SENTINEL;

	/** Default per-command timeout when none is provided. */
	static const int DEFAULT_TIMEOUT_MS;

	/** Process-wide accessor (created lazily; see initialize()). */
	static GCodeClient * get();

	/**
	 * Replace the global instance with one wrapping `stream` (ownership
	 * transferred).  Used by Main during boot and by tests; releases any
	 * previously-installed instance.
	 */
	static void install(IByteStream * stream);

	/** Release the global instance, if any. */
	static void release();

	/**
	 * Try the configured device path first (if non-empty), then scan
	 * common Arduino USB paths and pick the first that answers the
	 * liveness probe.  Stores the resulting SerialPort as the global
	 * instance.  Returns true if a working device was found.
	 */
	static bool initialize(const std::string& preferredPath, int baudRate);

	~GCodeClient();

	/** True if the transport is open AND the last operation didn't fail. */
	bool isConnected() const;

	/** The device path that was opened (empty when the stream is a stub). */
	const std::string& getDevicePath() const { return m_devicePath; }

	/** Send a single G-code line and wait for the ">" ack. */
	SendResult send(const std::string& line, int timeoutMs = DEFAULT_TIMEOUT_MS);

	/** True if the stream is currently considered usable. */
	bool isOpen() const;

	/** Convenience used by initialize() and the http "Test" endpoint. */
	bool ping(int timeoutMs = 2000);

	/** Test seam: read the bytes already written by send() in order. */
	const std::vector<std::string>& getSentLines() const { return m_sentLines; }

private:
	GCodeClient(IByteStream * stream, const std::string& devicePath);

	GCodeClient(const GCodeClient&);
	GCodeClient& operator=(const GCodeClient&);

	IByteStream * m_stream;
	std::string m_devicePath;
	std::string m_rxBuffer;
	std::vector<std::string> m_sentLines;
	bool m_connected;

	static GCodeClient * m_instance;
};

}
