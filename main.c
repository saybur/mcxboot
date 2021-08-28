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

int main(void)
{
	// the bootloader is only invoked on a power-on reset
	uint8_t rst_stat = RST.STATUS;
	if (! (rst_stat & RST_PORF_bm))
	{
		// TODO implement
	}

	// initialize the memory card interface
	MEM_PORT.OUTCLR = MEM_PIN_XCK;
	MEM_PORT.OUTSET = MEM_PIN_TX | MEM_PIN_CS;
	MEM_PORT.DIRSET = MEM_PIN_XCK | MEM_PIN_TX | MEM_PIN_CS;
	MEM_PINCTRL_RX |= PORT_OPC_PULLUP_gc;

	// mount the memory card
	uint8_t res = pf_mount(&fs);
	if (res)
	{
		// TODO implement
	}
	// open the programming file
	res = pf_open(FLASH_FILENAME);
	if (res)
	{
		// TODO implement
	}

	// TODO finish
	while (1) { };
	return 0;
}
