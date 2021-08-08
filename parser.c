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
#include <string.h>
#include "parser.h"

PARSER_RES parse_line(
	char* line,
	uint8_t* buffer,
	uint8_t* dsize,
	uint16_t* addr
)
{
	if (strlen(line) < 11) return PARSE_LENGTH;
	if (line[0] != ':') return PARSE_MALFORMED;

	char strb[3];
	strb[2] = '\0';
	uint8_t csum = 0;

	// get the number of data bytes
	strncpy(strb, line + 1, 2);
	*dsize = strtoul(strb, NULL, 16);
	csum += *dsize;

	// re-verify line length based on expected data
	if (strlen(line) != ((uint16_t) (*dsize)) * 2 + 11) return PARSE_LENGTH;

	// fetch address
	strncpy(strb, line + 3, 2);
	uint8_t haddr = strtoul(strb, NULL, 16);
	csum += haddr;
	strncpy(strb, line + 5, 2);
	uint8_t laddr = strtoul(strb, NULL, 16);
	csum += laddr;

	// fetch record type
	strncpy(strb, line + 7, 2);
	uint8_t rtype = strtoul(strb, NULL, 16);
	if (rtype > 1) return PARSE_UNKNOWN_RECORD;
	csum += rtype;

	// parse data
	uint16_t lof = 9;
	for (uint8_t i = 0; i < *dsize; i++)
	{
		strncpy(strb, line + lof, 2);
		buffer[i] = strtoul(strb, NULL, 16);
		csum += buffer[i];
		lof += 2;
	}

	// verify checksum
	strncpy(strb, line + 3, 2);
	uint8_t checksum = strtoul(strb, NULL, 16);
	if ((~csum) + 1 != checksum) return PARSE_CHECKSUM;

	// otherwise, data read was successful
	*addr = (haddr << 8) + laddr;
	return PARSE_OK;
}
