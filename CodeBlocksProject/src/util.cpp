/*
 * Copyright (C) (2009) (Joseph Balough) <jbb5044@psu.edu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "util.h"

int textonDiameter = 0;

int callNo = 0;
int debug(char *format, ...)
{
	if(VERBOSITY < 1)
		return 0;

	int toReturn = 0;
	printf("%06d: ", callNo);
	callNo++;

	//... option stuff
	va_list ap;
	va_start(ap, format);

	//do the special printf stuff
	toReturn = vprintf(format, ap);

	fflush(stdout);

	//clean up the ... stuff
	va_end(ap);

	return toReturn;
}

int verboseDebug(char *format, ...)
{
	if(VERBOSITY < 2)
		return 0;

	int toReturn = 0;
	printf("%06d: ", callNo);
	callNo++;

	va_list ap;
	va_start(ap, format);

	//do the special printf stuff
	toReturn = vprintf(format, ap);

	fflush(stdout);

	//clean up the ... stuff
	va_end(ap);

	return toReturn;
}
