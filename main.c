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
#include "sp_driver.h"
#include "config.h"

// flash-code error conditions
#define ERR_MEM_CARD_READ        2
#define ERR_MEM_CARD_OPEN        3
#define ERR_FLASH_VERIFY         4

#ifndef FLASH_PAGE_SIZE
	#error "FLASH_PAGE_SIZE must be defined"
#elif (512 % FLASH_PAGE_SIZE != 0)
	#error "FLASH_PAGE_SIZE must be a multiple of 512"
#endif

#define led_on()  LED_PORT.DIR |= LED_PIN;
#define led_off() LED_PORT.DIR &= ~LED_PIN;

static FATFS fs;

/*
 * Executes a software reboot.
 * 
 * The bootloader checks the reset status flags when starting, and will jump
 * to the application section immediately if not a power-on / PDI reset.
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
	uint8_t rst_stat = RST.STATUS & (RST_PORF_bm | RST_PDIRF_bm);
	if (! (rst_stat))
	{
		// jump to the application at address 0x0
		EIND = 0;
		__asm__ __volatile__(
			"clr ZL"            "\n\t"
			"clr ZH"            "\n\t"
			"ijmp"              "\n\t"
			);
	}
	// clear flags to prevent a bootloader boot-loop
	RST.STATUS = rst_stat;

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

	// erase the application
	SP_EraseApplicationSection();
	SP_WaitForSPM();

	// write the file to flash, starting at 0x0
	uint32_t addr = 0;
	uint8_t end = 0;
	uint8_t buf[FLASH_PAGE_SIZE];
	uint16_t br;
	do
	{
		// LED on during mem card reads, off during flash programming
		led_on();

		// read memory card contents
		res = pf_read(buf, FLASH_PAGE_SIZE, &br);
		if (res)
		{
			end = ERR_MEM_CARD_READ;
			break;
		}
		// note end if encountered
		if (br != FLASH_PAGE_SIZE) end = 255;

		// write data to flash
		led_off();
		SP_LoadFlashPage(buf);
		SP_WriteApplicationPage(addr);
		SP_WaitForSPM();
		addr += FLASH_PAGE_SIZE;
	}
	while (! end);

	// verify contents
	if (end == 255)
	{
		addr = 0;
		uint8_t flash[FLASH_PAGE_SIZE];

		// re-open file
		res = pf_open(FLASH_FILENAME);
		if (res)
		{
			end = 4;
		}

		// read from card again, comparing as we go
		while (end == 255)
		{
			led_on();

			// read memory card contents
			res = pf_read(buf, FLASH_PAGE_SIZE, &br);
			if (res)
			{
				end = 5;
				break;
			}
			if (br != FLASH_PAGE_SIZE) end = 254;

			// read flash page
			SP_ReadFlashPage(flash, addr);
			addr += FLASH_PAGE_SIZE;

			// compare
			for (uint16_t i = 0; i < br; i++)
			{
				if (buf[i] != flash[i]) end = 3;
			}
		}
	}

	// programming complete
	while (1)
	{
		for (uint8_t i = 0; i < end; i++)
		{
			led_on();
			_delay_ms(500);
			led_off();
			_delay_ms(500);
		}
		_delay_ms(1000);
	}
	return 0;
}
