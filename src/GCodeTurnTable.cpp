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

#include "GCodeTurnTable.h"
#include "GCodeClient.h"
#include "Logger.h"

#include <math.h>
#include <sstream>

namespace freelss
{

namespace
{

// Local 2*PI to avoid pulling Main.h.  Matches the project-wide PI.
const real TWO_PI = (real) (2.0 * 3.14159265359);

}

GCodeTurnTable::GCodeTurnTable(GCodeClient * client,
                               int stepsPerRevolution,
                               int feedRate,
                               bool invertDirection) :
	m_client(client),
	m_stepsPerRevolution(stepsPerRevolution),
	m_feedRate(feedRate > 0 ? feedRate : 1),
	m_invertDirection(invertDirection)
{
}

GCodeTurnTable::~GCodeTurnTable()
{
	if (m_client != NULL)
	{
		m_client->send("M18");
	}
}

int GCodeTurnTable::rotate(real theta)
{
	// Preserve historical semantics: truncation toward zero; negative theta
	// silently does nothing (existing A4988TurnTable::rotate did the same).
	int numSteps = (int) ((theta / TWO_PI) * (real) m_stepsPerRevolution);
	if (numSteps <= 0)
	{
		return 0;
	}

	int signedSteps = m_invertDirection ? -numSteps : numSteps;

	std::ostringstream cmd;
	cmd << "G2 T" << signedSteps << " F" << m_feedRate;

	if (m_client != NULL)
	{
		GCodeClient::SendResult r = m_client->send(cmd.str());
		if (r != GCodeClient::SEND_OK)
		{
			ErrorLog << "GCodeTurnTable: rotate failed (result=" << (int) r << ")" << Logger::ENDL;
		}
	}

	return numSteps;
}

void GCodeTurnTable::setMotorEnabled(bool enabled)
{
	if (m_client == NULL)
	{
		return;
	}

	GCodeClient::SendResult r = m_client->send(enabled ? "M17" : "M18");
	if (r != GCodeClient::SEND_OK)
	{
		ErrorLog << "GCodeTurnTable: setMotorEnabled(" << (enabled ? 1 : 0) << ") failed (result=" << (int) r << ")" << Logger::ENDL;
	}
}

}
