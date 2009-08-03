/*
 * Copyright (C) 2009 Coen Bijlsma
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TIME_TIME_H
#define __TIME_TIME_H

#include <Macros.h>
#include <Types.h>
#include <Device.h>

class Time : public Device
{
    public:

	/**
	 * Constructor function.
	 */
        Time();

	/**
	 * @brief Initializes the time class.
	 * @return Error status code.
	 */
	Error initialize();

	/** 
	 * Read the time
	 * @param buffer Buffer to save the read bytes.
	 * @param size Number of bytes to read.
	 * @param offset Unused.
	 * @return Number of bytes on success and ZERO on failure. 
	 */
	Error read(s8 *buffer, Size size, Size offset);

	/** 
	 * Write the time
	 * @param buffer Buffer containing bytes to write. 
	 * @param size Number of bytes to write.
	 * @param offset Unused.
	 * @return Number of bytes on success and ZERO on failure. 
	 */
	/* Error write(s8 *buffer, Size size, Size offset); */

};

#endif /* __TIME_TIME_H */
