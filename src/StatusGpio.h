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

#include "Thread.h"

namespace freelss
{

/**
 * Pi GPIO helpers for a physical scan trigger and status LED.
 *
 * wiringPi pin numbers (see wiringpi.com/pins):
 *   Scan button (input):  pin 2  -> BCM GPIO 27, header pin 13.
 *                         Wire a momentary switch between the pin and GND;
 *                         the internal pull-up makes a press read LOW.
 *   Status LED (output): pin 6  -> BCM GPIO 25, header pin 22.
 *                         Drive an LED (with series resistor) to GND.
 *
 * The LED is steady ON while the scanner is idle and blinks 600 ms ON /
 * 400 ms OFF while any scanner job is running.
 */
class StatusGpio : public Thread
{
public:
	static const int SCAN_BUTTON_PIN = 2;
	static const int STATUS_LED_PIN  = 6;

	static const int LED_ON_MS  = 600;
	static const int LED_OFF_MS = 400;

	/** Default scan range (degrees), matching the web UI form default. */
	static const double DEFAULT_SCAN_DEGREES;

	static StatusGpio * get();
	static void release();

	void run();

private:
	StatusGpio();
	~StatusGpio();

	bool initializePins();
	void updateStatusLed(bool scanning);

	static StatusGpio * m_instance;

	bool m_initialized;
	bool m_ledOn;
	double m_ledPhaseStartSec;
	bool m_wasScanning;
};

}
