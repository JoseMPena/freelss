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

#include "GCodeLighting.h"
#include "GCodeClient.h"
#include "Logger.h"

namespace freelss
{

GCodeLighting::GCodeLighting(GCodeClient * client) :
	m_client(client),
	m_intensity(0),
	m_on(false)
{
}

GCodeLighting::~GCodeLighting()
{
	if (m_on && m_client != NULL)
	{
		m_client->send("M6");
	}
}

void GCodeLighting::setIntensity(int intensity)
{
	if (intensity < 0)   intensity = 0;
	if (intensity > 100) intensity = 100;

	m_intensity = intensity;

	bool shouldBeOn = intensity > 0;
	if (shouldBeOn == m_on)
	{
		return;
	}

	if (m_client == NULL)
	{
		m_on = shouldBeOn;
		return;
	}

	GCodeClient::SendResult r = m_client->send(shouldBeOn ? "M5" : "M6");
	if (r != GCodeClient::SEND_OK)
	{
		ErrorLog << "GCodeLighting: " << (shouldBeOn ? "M5" : "M6") << " failed (result=" << (int) r << ")" << Logger::ENDL;
		return;
	}

	m_on = shouldBeOn;
}

int GCodeLighting::getIntensity() const
{
	return m_intensity;
}

}
