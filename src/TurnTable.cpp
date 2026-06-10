/*
 ****************************************************************************
 *  Copyright (c) 2014 Uriah Liggett <freelaserscanner@gmail.com>           *
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
#include "TurnTable.h"
#include "GCodeTurnTable.h"
#include "GCodeClient.h"
#include "Setup.h"

namespace freelss
{

TurnTable * TurnTable::m_instance = NULL;

TurnTable * TurnTable::getInstance()
{
	if (TurnTable::m_instance == NULL)
	{
		Setup * setup = Setup::get();
		int feed = setup->feedRate > 0
		         ? setup->feedRate
		         : (setup->motorStepDelay > 0 ? (1000000 / setup->motorStepDelay) : 1200);
		bool invert = setup->motorDirPinValue != 0;

		TurnTable::m_instance = new GCodeTurnTable(
			GCodeClient::get(),
			setup->stepsPerRevolution,
			feed,
			invert);
	}

	return TurnTable::m_instance;
}

void TurnTable::release()
{
	delete TurnTable::m_instance;
	TurnTable::m_instance = NULL;
}

} // ns scanner
