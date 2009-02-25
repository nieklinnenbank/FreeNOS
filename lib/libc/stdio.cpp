/**
 * Copyright (C) 2009 Niek Linnenbank
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

#include <api/IPCMessage.h>
#include <TerminalServer.h>
#include <Config.h>
#include <stdarg.h>
#include "stdio.h"

void itoa(char *buffer, int divisor, int number)
{
    char *p = buffer, *p1, *p2, tmp;
    unsigned long ud = number;
    int remainder;
        
    /* Negative decimal. */
    if (divisor == 10 && number < 0)
    {
	*p++ = '-';
	buffer++;
	ud = -number;
    }
    /* Hexadecimal. */
    else if (divisor == 16)
    {
	*p++ = '0';
	*p++ = 'x';
	buffer += 2;
    }
    /* Divide ud by the divisor, until ud == 0. */
    do
    {
	remainder = ud % divisor;
	*p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    }
    while (ud /= divisor);
    
    /* Terminate buffer. */
    *p = 0;
    
    /* Initialize pointers. */
    p1 = buffer;
    p2 = p - 1;
    
    /* Reverse buf. */
    while (p1 < p2)
    {
	tmp = *p1;
	*p1 = *p2;
	*p2 = tmp;
	p1++;
	p2--;
    }
}

int snprintf(char *buffer, unsigned int size, const char *fmt, ...)
{
    va_list args;
    int ret;

    va_start(args, fmt);
    ret = vsnprintf(buffer, size, fmt, args);
    va_end(args);
    
    return (ret);
}

int vsnprintf(char *buffer, unsigned int size, const char *fmt, va_list args)
{
    char buf[20], *ptr;
    int ch;
    unsigned int written = 0;
    
    /* Loop formatted message. */
    while ((ch = *fmt++) && written < size)
    {
	if (ch != '%')
	{
	    *buffer++ = ch;
	    written++;
	}
	else
	{
	    switch (*fmt)
	    {
	    case 'd':
	    case 'u':
	    	itoa(buf, 10, va_arg(args, int));
		ptr = buf;
		goto string;
	
	    case 'l':
		itoa(buf, 10, va_arg(args, long));
		ptr = buf;
		goto string;
	
	    case 'x':
		itoa(buf, 16, va_arg(args, int));
		ptr = buf;
		goto string;
	
	    case 'c':
		buf[0] = va_arg(args, int);
		buf[1] = ZERO;
		ptr    = buf;
		goto string;
	
	    case 's':
		ptr = va_arg(args, char *);
	
	    string:
		while(*ptr && written++ < size)
		{
		    *buffer++ = *ptr++;
		}
		break;
	
	    default:
		*buffer++ = ch;
		written++;
		break;
	    }
	    fmt++;
	}
    }
    /* Null terminate. */
    if (written < size)
	*buffer = 0;
    return (written);
}

int printf(char *format, ...)
{
    va_list args;
    int ret;
    
    va_start(args, format);
    ret = vprintf(format, args);
    va_end(args);
    
    return ret;
}

int vprintf(char *format, va_list args)
{
    TerminalMessage msg;
    char buf[1024];
    
    /* Clear buffer first. */
    memset(&buf, 0, sizeof(buf));
    
    /* Write formatted string. */
    msg.action = TerminalWrite;
    msg.buffer = buf;
    msg.size   = vsnprintf(buf, sizeof(buf), format, args);

    /* Send message to the terminal server. */
    return IPCMessage(TERMINAL_PID, SendReceive, &msg);
}

char * gets(char *buffer, Size size)
{
    Size total = 0;
    
    /* Read a line. */
    while (total < size)
    {
	*buffer = getc(buffer + total);
	 total++;
	 
	/* EOL reached? */
	if (buffer[total-1] == '\n' || buffer[total-1] == '\r')
	{
	    buffer[total-1] = ZERO;
	    break;
	}
    }
    return buffer;
}

char getc(char *buffer)
{
    TerminalMessage msg;
    
    /* Read a character. */
    while (msg.action != TerminalOK)
    {
	/* Fill in message. */
        msg.action = TerminalRead;
	msg.buffer = buffer;
        msg.size   = 1;
    
	/* Send message to terminal. */
	IPCMessage(TERMINAL_PID, SendReceive, &msg);
    }
    return *buffer;
}
