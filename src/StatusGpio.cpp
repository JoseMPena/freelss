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

#include "Main.h"
#include "StatusGpio.h"
#include "HttpServer.h"
#include "Scanner.h"
#include "Logger.h"

#ifndef MOCK
#include <wiringPi.h>
#endif

namespace freelss
{

StatusGpio * StatusGpio::m_instance = NULL;

const double StatusGpio::DEFAULT_SCAN_DEGREES = 360.0;

StatusGpio * StatusGpio::get()
{
	if (StatusGpio::m_instance == NULL)
	{
		StatusGpio::m_instance = new StatusGpio();
	}
	return StatusGpio::m_instance;
}

void StatusGpio::release()
{
	if (StatusGpio::m_instance != NULL)
	{
		StatusGpio::m_instance->stop();
		StatusGpio::m_instance->join();
		delete StatusGpio::m_instance;
		StatusGpio::m_instance = NULL;
	}
}

StatusGpio::StatusGpio() :
	m_initialized(false),
	m_ledOn(false),
	m_ledPhaseStartSec(0),
	m_wasScanning(false)
{
#ifndef MOCK
	m_initialized = initializePins();
	if (!m_initialized)
	{
		ErrorLog << "StatusGpio: wiringPi unavailable; button/LED disabled." << Logger::ENDL;
	}
#else
	m_initialized = false;
#endif
}

StatusGpio::~StatusGpio()
{
#ifndef MOCK
	if (m_initialized)
	{
		digitalWrite(STATUS_LED_PIN, LOW);
	}
#endif
}

bool StatusGpio::initializePins()
{
#ifndef MOCK
	if (wiringPiSetup() == -1)
	{
		return false;
	}

	pinMode(SCAN_BUTTON_PIN, INPUT);
	pullUpDnControl(SCAN_BUTTON_PIN, PUD_UP);

	pinMode(STATUS_LED_PIN, OUTPUT);
	digitalWrite(STATUS_LED_PIN, HIGH);
	m_ledOn = true;
	m_ledPhaseStartSec = GetTimeInSeconds();
	return true;
#else
	return false;
#endif
}

void StatusGpio::updateStatusLed(bool scanning)
{
#ifndef MOCK
	if (!m_initialized)
	{
		return;
	}

	if (scanning != m_wasScanning)
	{
		m_wasScanning = scanning;
		m_ledPhaseStartSec = GetTimeInSeconds();
		m_ledOn = true;
	}

	if (!scanning)
	{
		if (!m_ledOn)
		{
			digitalWrite(STATUS_LED_PIN, HIGH);
			m_ledOn = true;
		}
		return;
	}

	double now = GetTimeInSeconds();
	double elapsedMs = (now - m_ledPhaseStartSec) * 1000.0;
	int phaseMs = m_ledOn ? LED_ON_MS : LED_OFF_MS;

	if (elapsedMs >= phaseMs)
	{
		m_ledOn = !m_ledOn;
		m_ledPhaseStartSec = now;
		digitalWrite(STATUS_LED_PIN, m_ledOn ? HIGH : LOW);
	}
#else
	(void) scanning;
#endif
}

void StatusGpio::run()
{
#ifndef MOCK
	if (!m_initialized)
	{
		while (!m_stopRequested)
		{
			Thread::usleep(500000);
		}
		return;
	}

	int stableLowSamples = 0;
	const int debounceSamples = 3;

	while (!m_stopRequested)
	{
		Scanner * scanner = HttpServer::get()->getScanner();
		bool scanning = scanner != NULL && scanner->isRunning();
		updateStatusLed(scanning);

		if (digitalRead(SCAN_BUTTON_PIN) == LOW)
		{
			stableLowSamples++;
			if (stableLowSamples == debounceSamples)
			{
				InfoLog << "StatusGpio: scan button pressed." << Logger::ENDL;
				HttpServer::get()->startScan((real) DEFAULT_SCAN_DEGREES);
			}
		}
		else
		{
			stableLowSamples = 0;
		}

		Thread::usleep(20000);
	}

	digitalWrite(STATUS_LED_PIN, LOW);
#else
	while (!m_stopRequested)
	{
		Thread::usleep(500000);
	}
#endif
}

}
