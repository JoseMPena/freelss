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

#include "Lighting.h"

namespace freelss
{

class GCodeClient;

/**
 * Lighting adapter for the Arduino G-code firmware.  The firmware
 * only exposes M5 (on) / M6 (off), so the 0..100 intensity input is
 * collapsed to a threshold (intensity > 0 -> on).  The last accepted
 * value is preserved for getIntensity() so the existing slider UI
 * still round-trips.
 */
class GCodeLighting : public Lighting
{
public:
	explicit GCodeLighting(GCodeClient * client);
	~GCodeLighting();

	void setIntensity(int intensity);
	int  getIntensity() const;

private:
	GCodeClient * m_client;
	int  m_intensity;
	bool m_on;
};

}
