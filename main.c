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

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "lib/pff/pff.h"
#include "config.h"
#include "parser.h"

static FATFS fs;

/*
 * Memory buffer and routine for use with fdevopen(). This will cache a sector
 * in SRAM, returning appropriate values once the end of the file is reached
 * or an error occurs. Error states are indicated via mbuferr being != 0.
 */
static uint8_t mbuf[512];
static uint16_t mbufpos = 512;
static uint16_t mbufsize = 512;
static uint8_t mbuferr = 0;
static int mbufget(FILE* file)
{
	(void) file; // silence unused compiler warning
	if (mbuferr) return _FDEV_ERR;

	// if we are out of data, fetch more (if there is any)
	if (mbufpos >= mbufsize)
	{
		if (mbufsize != 512) return _FDEV_EOF;
		uint8_t res = pf_read(mbuf, 512, &mbufsize);
		if (res)
		{
			mbuferr = res;
			return _FDEV_ERR;
		}
		mbufpos = 0;
		if (mbufsize == 0) return _FDEV_EOF;
	}

	// provide next byte
	return mbuf[mbufpos++];
}

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
	res = pf_open(HEX_FILENAME);
	if (res)
	{
		// TODO implement
	}

	uint8_t data[255];
	uint8_t dsize;
	uint16_t addr;

	// setup stdio access for per-line reading
	FILE* file = fdevopen(NULL, mbufget);
	// read until EOF
	char line[270];
	while (fgets(line, sizeof(line), file))
	{
		PARSE_RES pres = parse_line(line, data, &dsize, &addr);
		if (pres == PARSE_EOF)
		{
			break;
		}
		else if (pres)
		{
			// TODO handle error condition
		}
	}
	// cleanup
	fclose(file);

	// TODO finish
	while (1) { };
	return 0;
}
