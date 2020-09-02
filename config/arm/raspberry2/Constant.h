/*
 * Copyright (C) 2020 Niek Linnenbank
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

#ifndef __CONFIG_ARM_RASPBERRY2_CONSTANT_H
#define __CONFIG_ARM_RASPBERRY2_CONSTANT_H

/** Raspberry Pi 2 have a Broadcom 2836 SoC */
#define ARM
#define ARMV7
#define BCM2835
#define BCM2836
#define SMP
#define SMP_WAIT

/** Default IO base on the Broadcom 2836 */
#define IO_BASE 0x3F000000
#define IO_SIZE (1024*1024*128)

#define GPIO_BASE (IO_BASE + 0x200000)
#define UART_BASE (GPIO_BASE + 0x1000)

/** Triggered when the PL011 asserts IRQ status */
#define UART0_IRQ 57

/** PhysicalTimer1, IRQ number. */
#define ARMTIMER_IRQ 3

#define RAM_ADDR (0x0)

/* Unfortunately, libarch does not support RAM sizes of 1GiB and higher.
 * The reason is that the kernel maps only 1GiB minus 128MiB of RAM,
 * where the upper 128MiB is needed for the KernelPrivate section.
 * Mapping more than that will not work with SplitAllocator::toVirtual().
 * Therefore, use 128MiB less than the actual device. */
#define RAM_SIZE (1024 * 1024 * (1024-128))
#define TMPSTACKOFF  (1024 * 1024 * 32)
#define TMPSTACKADDR (RAM_ADDR + TMPSTACKOFF)

#include <arm/ARMConstant.h>

#endif /* __CONFIG_ARM_RASPBERRY2_CONSTANT_H */
