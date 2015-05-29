/*
 * Copyright (C) 2015 Niek Linnenbank
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

#include <FreeNOS/System.h>
#include "RaspiSerial.h"

RaspiSerial::RaspiSerial()
{
    init();
}

void RaspiSerial::init(void)
{
    // Disable UART0.
    IO::write(UART0_CR, 0x00000000);
    
    // Setup the GPIO pin 14 && 15.
    // Disable pull up/down for all GPIO pins & delay for 150 cycles.
    IO::write(GPPUD, 0x00000000);
    delay(150);

    // Disable pull up/down for pin 14,15 & delay for 150 cycles.
    IO::write(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);

    // Write 0 to GPPUDCLK0 to make it take effect.
    IO::write(GPPUDCLK0, 0x00000000);
    
    // Clear pending interrupts.
    IO::write(UART0_ICR, 0x7FF);

    // Set integer & fractional part of baud rate.
    // Divider = UART_CLOCK/(16 * Baud)
    // Fraction part register = (Fractional part * 64) + 0.5
    // UART_CLOCK = 3000000; Baud = 115200.

    // Divider = 3000000/(16 * 115200) = 1.627 = ~1.
    // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
    IO::write(UART0_IBRD, 1);
    IO::write(UART0_FBRD, 40);

    // Enable FIFO & 8 bit data transmissio (1 stop bit, no parity).
    IO::write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

    // Mask all interrupts.
    IO::write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) |
                          (1 << 6) | (1 << 7) | (1 << 8) |
                          (1 << 9) | (1 << 10));

    // Enable UART0, receive & transfer part of UART.
    IO::write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void RaspiSerial::put(u8 byte)
{
    // wait for UART to become ready to transmit
    while(true)
    {
        if (!(IO::read(UART0_FR) & (1 << 5)))
        {
            break;
        }
    }
    IO::write(UART0_DR, byte);
}

u8 RaspiSerial::get(void)
{
    // wait for UART to have recieved something
    while(true)
    {
        if (!(IO::read(UART0_FR) & (1 << 4)))
        {
            break;
        }
    }
    return IO::read(UART0_DR);
}

void RaspiSerial::write(const char *str)
{
    while(*str)
    {
        RaspiSerial::put(*str++);
    }
}

void RaspiSerial::delay(s32 count)
{
    asm volatile("1: subs %[count], %[count], #1; bne 1b"
         : : [count]"r"(count));
}
