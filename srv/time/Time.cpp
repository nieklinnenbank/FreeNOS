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

#include <API/ProcessCtl.h>
#include <FreeNOS/CPU.h>
#include <Macros.h>
#include <Types.h>
#include <Config.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "Time.h"

Time::Time()
{
}

Error Time::initialize()
{
    ProcessCtl(SELF, AllowIO, RTC_PORT(0));
    ProcessCtl(SELF, AllowIO, RTC_PORT(1));
    /* Done! */
    return ESUCCESS;
}

Error Time::read(s8 *buffer, Size size, Size offset)
{
    /*
     * NOTICE:
     * A lot of this code comes from the linux kernel source,
     * particularly from the file arch/x86/kernel/rtc.c.
     * As a result, the code (from mainly Time::readCMOS()) may
     * have to be replaced to something like kernel/X86/RTC.cpp
    */
    
    /* PHONY read */
    if( offset >= 10 )
    {
        return 0;
    }
    
    //unsigned int status;
    //unsigned int century;
    unsigned int year, month, day, hour, min, sec = 0;
    
    /*
     * This code comes from arch/x86/kernel/rtc.c from the linux source
     */
    while( (readCMOS(RTC_FREQ_SELECT) & RTC_UIP ))
    {
        Time::cpuRelax();
    }
    
    sec = bcd2bin(readCMOS(RTC_SECONDS));
    min = bcd2bin(readCMOS(RTC_MINUTES));
    hour = bcd2bin(readCMOS(RTC_HOURS));
    day = bcd2bin(readCMOS(RTC_DAY_OF_MONTH));
    month = bcd2bin(readCMOS(RTC_MONTH));
    year = bcd2bin(readCMOS(RTC_YEAR));
    
    if( year < 100 )
    {
        year += CMOS_YEARS_OFFS;
    }
    
    unsigned long time = mktime(year, month, day, hour, min, sec);
    
    snprintf((char*)buffer, size, "%u", time);
    return (Error) size;
}

unsigned char Time::readCMOS(unsigned char addr)
{
    unsigned char val;
    
    //lock_cmos_prefix(addr);
    outb(RTC_PORT(0), addr);
    val = inb(RTC_PORT(1));
    // lock_cmos_suffix(addr);
    return val;
}

void Time::cpuRelax()
{
    asm volatile("rep; nop" ::: "memory");
}

unsigned Time::bcd2bin(unsigned char val)
{
    return (val & 0x0f) + (val >> 4) * 10;
}
