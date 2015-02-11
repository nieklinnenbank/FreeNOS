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

#ifndef __ARM_UART_H
#define __ARM_UART_H

#include <Types.h>

class UART
{
public:

    /**
     * Constructor.
     */
    UART();

    /*
     * Initialize UART0.
     */
    static void init(void);

    /*
     * Receive a byte via UART0.
     *
     * Returns:
     * uint8_t: byte received.
     */
    u8 get(void);

    /*
     * Transmit a byte via UART0.
     * uint8_t Byte: byte to send.
     */
    void put(u8 byte);

    /*
     * print a string to the UART one character at a time
     * const char *str: 0-terminated string
     */
    void put(const char *str);

private:

    enum {
	// The GPIO registers base address.
	GPIO_BASE = 0x20200000,

	// The offsets for reach register.

	// Controls actuation of pull up/down to ALL GPIO pins.
	GPPUD = (GPIO_BASE + 0x94),

	// Controls actuation of pull up/down for specific GPIO pin.
	GPPUDCLK0 = (GPIO_BASE + 0x98),

	// The base address for UART.
	UART0_BASE = 0x20201000,

	// The offsets for reach register for the UART.
	UART0_DR     = (UART0_BASE + 0x00),
	UART0_RSRECR = (UART0_BASE + 0x04),
	UART0_FR     = (UART0_BASE + 0x18),
	UART0_ILPR   = (UART0_BASE + 0x20),
	UART0_IBRD   = (UART0_BASE + 0x24),
	UART0_FBRD   = (UART0_BASE + 0x28),
	UART0_LCRH   = (UART0_BASE + 0x2C),
	UART0_CR     = (UART0_BASE + 0x30),
	UART0_IFLS   = (UART0_BASE + 0x34),
	UART0_IMSC   = (UART0_BASE + 0x38),
	UART0_RIS    = (UART0_BASE + 0x3C),
	UART0_MIS    = (UART0_BASE + 0x40),
	UART0_ICR    = (UART0_BASE + 0x44),
	UART0_DMACR  = (UART0_BASE + 0x48),
	UART0_ITCR   = (UART0_BASE + 0x80),
	UART0_ITIP   = (UART0_BASE + 0x84),
	UART0_ITOP   = (UART0_BASE + 0x88),
	UART0_TDR    = (UART0_BASE + 0x8C),
    };

    /*
     * delay function
     * int32_t delay: number of cycles to delay
     *
     * This just loops <delay> times in a way that the compiler
     * wont optimize away.
     */
    static void delay(s32 count);
};

#endif /* __ARM_UART_H */
