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
#include "Lighting.h"
#include "GCodeLighting.h"
#include "GCodeClient.h"
#include <stddef.h>

namespace freelss
{

Lighting * Lighting::m_instance = NULL;

Lighting::Lighting()  {}
Lighting::~Lighting() {}

Lighting * Lighting::get()
{
	if (Lighting::m_instance == NULL)
	{
		Lighting::m_instance = new GCodeLighting(GCodeClient::get());
	}
	return Lighting::m_instance;
}

void Lighting::release()
{
	delete Lighting::m_instance;
	Lighting::m_instance = NULL;
}

}
