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

#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

/*
 * This file contains several useful utility functions and a couple defines / macros
 * and global variables
 */

//NOTE: VERBOSITY indicates how verbose to be with the debug output
//can be 0 = no output, 1 = a bit of output, 2 = tons of output
#define VERBOSITY 1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//this is how many colors to sample when generating a neighborhood.
//it is always set in the command line options.
extern int textonDiameter;

//got a little help with the printf wrapper from the following website
// http://bytes.com/topic/c/answers/220856-printf-wrapper

//some quick wrappers for debug printing
//prints if verbosity is 1 or 2
int debug(char *format, ...);
//only prints if verbosity is 2
//note: all "done" messages should be printed using this function.
int verboseDebug(char *format, ...);

#ifndef MAX
#define MAX(x,y) ((x) < (y) ? (y) : (x))
#endif
#ifndef MIN
#define MIN(x,y) ((x) > (y) ? (y) : (x))
#endif


#endif // UTIL_H_INCLUDED
