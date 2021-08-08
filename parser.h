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

#ifndef PARSER_H
#define PARSER_H

#include <avr/io.h>

/*
 * Simplistic parser for a Intel hex format file, as described here:
 * 
 * https://en.wikipedia.org/wiki/Intel_HEX
 * 
 * This has many limitations, including only really handling the I8HEX format.
 */

typedef enum {
	PARSE_OK = 0,
	PARSE_EOF,                  // the 0x01 end-of-fine line was found
	PARSE_LENGTH,               // line length was invalid
	PARSE_MALFORMED,            // missing ':', or otherwise illegal
	PARSE_UNKNOWN_RECORD,       // record type was unknown
	PARSE_CHECKSUM,             // checksum was invalid
} PARSE_RES;

/*
 * Parses a line of input from a Intel hex format file, from a fgets() type
 * call, to get the data block contined within.
 * 
 * The given line of characters will be parsed to generate a data block,
 * returned in the data pointer, which must be at least 255 bytes in size.
 * The size of the data is given in the 8-bit data pointer. The address is
 * given in the address pointer.
 * 
 * If the result is not PARSE_OK, the data/address returned is invalid and
 * should not be used.
 */
PARSE_RES parse_line(
	char* line,
	uint8_t* data,
	uint8_t* dsize,
	uint16_t* addr
);

#endif /* PARSER_H */
