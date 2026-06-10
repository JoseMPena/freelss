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

#include "TurnTable.h"

namespace freelss
{

class GCodeClient;

/**
 * TurnTable port adapter that drives the Arduino Mega via G-code
 * (G2/M17/M18).  All real-time step pacing happens on the firmware;
 * the host only sends one blocking command per rotation.
 */
class GCodeTurnTable : public TurnTable
{
public:
	GCodeTurnTable(GCodeClient * client,
	               int stepsPerRevolution,
	               int feedRate,
	               bool invertDirection);

	~GCodeTurnTable();

	int rotate(real theta);
	void setMotorEnabled(bool enabled);

private:
	GCodeClient * m_client;
	int m_stepsPerRevolution;
	int m_feedRate;
	bool m_invertDirection;
};

}
