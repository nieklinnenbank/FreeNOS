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

#define RTC_PORT(x) (0x70 + (x))

#define RTC_SECONDS             0
#define RTC_MINUTES             2
#define RTC_HOURS               4
#define RTC_DAY_OF_WEEK         6
#define RTC_DAY_OF_MONTH        7
#define RTC_MONTH               8
#define RTC_YEAR                9

/* Assume that a two-digit year is after 2000 */
#define CMOS_YEARS_OFFS         2000

#define RTC_FREQ_SELECT         10
#define RTC_UIP                 0x80

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

    private:
    
        /**
         * Returns the value stored at the given address
         * from the CMOS.
         * I actually almost copied this code from the linux source
         * from the file arch/x86/kernel/rtc.h so you should also
         * take a look over there how they do it.
         * @param addr The address to read from the CMOS
         * @return The value at the given address.
         */
        unsigned char readCMOS(unsigned char addr);
        
        /**
         * Requests that the cpu does a nop.
         */
        static inline void cpuRelax();
        
        unsigned bcd2bin(unsigned char val);
};

#endif /* __TIME_TIME_H */
