/*
 * Copyright (C) 2019-2021 saybur
 * 
 * This file is part of mcxboot.
 * 
 * mcxboot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mcxboot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with mcxboot.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <avr/io.h>
#include <util/delay.h>
#include "lib/pff/pff.h"
#include "config.h"

static FATFS fs;

/*
 * Executes a software reboot. Entry into the bootloader is controlled by reset
 * status flags, so as long as those are appropriately cleared the bootloader
 * will jump to the application section immediately after starting.
 */
static void sw_rst(void)
{
	while (1)
	{
		__asm__ __volatile__(
			"ldi r24, %0"       "\n\t"
			"out %1, r24"       "\n\t"
			"ldi r24, %2"       "\n\t"
			"sts %3, r24"       "\n\t"
			:
			: "M" (CCP_IOREG_gc), "i" (&CCP),
			  "M" (RST_SWRST_bm), "i" (&(RST.CTRL))
			: "r24"
			);
	};
}

int main(void)
{
	// skip bootloader if not a power-on reset
	uint8_t rst_stat = RST.STATUS;
	if (! (rst_stat & RST_PORF_bm))
	{
		// jump to the application (at address 0x0)
		EIND = 0;
		__asm__ __volatile__(
			"clr ZL"            "\n\t"
			"clr ZH"            "\n\t"
			"ijmp"              "\n\t"
			);
	}
	// clear RST_PORF_bm to prevent a bootloader boot-loop
	RST.STATUS = RST_PORF_bm;

	// initialize the memory card interface
	MEM_PORT.OUTCLR = MEM_PIN_XCK;
	MEM_PORT.OUTSET = MEM_PIN_TX | MEM_PIN_CS;
	MEM_PORT.DIRSET = MEM_PIN_XCK | MEM_PIN_TX | MEM_PIN_CS;
	MEM_PINCTRL_RX |= PORT_OPC_PULLUP_gc;

	// mount the memory card
	uint8_t res = pf_mount(&fs);
	if (res)
	{
		// no SD card, allow the application to handle
		sw_rst();
	}
	// open the programming file
	res = pf_open(FLASH_FILENAME);
	if (res)
	{
		// not unexpected, the file will frequently be missing
		sw_rst();
	}



	// TODO implement
	while (1) { }
	return 0;
}
