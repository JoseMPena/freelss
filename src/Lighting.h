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

#pragma once

#include "Types.h"

namespace freelss
{

/**
 * Lighting port.  The concrete adapter is built lazily by `get()`.
 * Intensity is accepted on the original 0..100 scale; adapters that
 * can't honour PWM (e.g. the Arduino firmware) collapse it to on/off.
 */
class Lighting
{
public:
	/** Returns the singleton instance. */
	static Lighting * get();

	/** Releases the singleton instance. */
	static void release();

	virtual ~Lighting();

	/** 0 (off) to 100 (full intensity). */
	virtual void setIntensity(int intensity) = 0;

	virtual int getIntensity() const = 0;

protected:
	Lighting();

private:
	static Lighting * m_instance;
};

}
