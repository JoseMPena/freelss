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

#include "Laser.h"

namespace freelss
{

class GCodeClient;

/**
 * Laser port adapter that drives the Arduino Mega via the M19..M22
 * G-code commands.  ALL_LASERS is implemented as two sequential
 * single-side commands so the firmware contract stays simple.
 */
class GCodeLaser : public Laser
{
public:
	explicit GCodeLaser(GCodeClient * client);
	~GCodeLaser();

	void turnOn(Laser::LaserSide laser);
	void turnOff(Laser::LaserSide laser);
	bool isOn(Laser::LaserSide laser);

private:
	void send(const char * gcode);

	GCodeClient * m_client;
	bool m_rightOn;
	bool m_leftOn;
};

}
