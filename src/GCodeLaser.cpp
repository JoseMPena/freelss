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

#include "GCodeLaser.h"
#include "GCodeClient.h"
#include "Logger.h"

namespace freelss
{

GCodeLaser::GCodeLaser(GCodeClient * client) :
	m_client(client),
	m_rightOn(false),
	m_leftOn(false)
{
}

GCodeLaser::~GCodeLaser()
{
	turnOff(Laser::ALL_LASERS);
}

void GCodeLaser::send(const char * gcode)
{
	if (m_client == NULL)
	{
		return;
	}

	GCodeClient::SendResult r = m_client->send(gcode);
	if (r != GCodeClient::SEND_OK)
	{
		ErrorLog << "GCodeLaser: " << gcode << " failed (result=" << (int) r << ")" << Logger::ENDL;
	}
}

void GCodeLaser::turnOn(Laser::LaserSide laser)
{
	if (laser == Laser::LEFT_LASER || laser == Laser::ALL_LASERS)
	{
		send("M19");
		m_leftOn = true;
	}
	if (laser == Laser::RIGHT_LASER || laser == Laser::ALL_LASERS)
	{
		send("M21");
		m_rightOn = true;
	}
}

void GCodeLaser::turnOff(Laser::LaserSide laser)
{
	if (laser == Laser::LEFT_LASER || laser == Laser::ALL_LASERS)
	{
		send("M20");
		m_leftOn = false;
	}
	if (laser == Laser::RIGHT_LASER || laser == Laser::ALL_LASERS)
	{
		send("M22");
		m_rightOn = false;
	}
}

bool GCodeLaser::isOn(Laser::LaserSide laser)
{
	if (laser == Laser::ALL_LASERS) return m_leftOn && m_rightOn;
	if (laser == Laser::LEFT_LASER)  return m_leftOn;
	if (laser == Laser::RIGHT_LASER) return m_rightOn;
	return false;
}

}
