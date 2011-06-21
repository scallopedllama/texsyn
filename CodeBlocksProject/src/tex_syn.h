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

#ifndef TEX_SYN_H_INCLUDED
#define TEX_SYN_H_INCLUDED

/*
 * This file contains the functions that will actually perform the texture synthesis.
 * This should be the only file including gauss_pyramid.h.
 */

//NOTE: if you have TEX_SYN_THREADS set to a value > 0, that number of threads
//		will be used when comparing neighborhoods generating the texture.
//		This does improve performance a TINY bit but you can't use a seemingly
//		logical number of threads, like 2 for your dual core. I found that my dual core
//		processor did best with 22 threads generating a 64 x 64 image with texton size 23
//		also, you can turn this up higher than the height of your render but it won't use
//		any more than (height) threads.
//		the default value can be changed in tex_syn.c or defined on the command line.
extern int TEX_SYN_THREADS;

//the following preprocessor instructinos will affect the way the texture is synthesized
//if you want to turn of them off, just comment them out.

//NOTE: if you have TEX_SYN_USE_MULTIRESOLUTION defined, the program will use a
//		multiresolution synthesis algorthim. If this is not defined, it'll just
//		generate the texture using only one layer.
#define TEX_SYN_USE_MULTIRESOLUTION



//NOTE: if you have TEX_SYN_WEIGHTED_COLORS defined, the similarity function will weight
//		the color differences. The weight is determined by the TEX_SYN_[color]_WEIGHT
//		defines. The function used is a sum of squared differences. These values factor
//		in at the sum level. It takes the square of the difference for the channel then
//		multiplies it by the color weight
#define TEX_SYN_WEIGHTED_COLORS
extern float TEX_SYN_RED_WEIGHT;
extern float TEX_SYN_GREEN_WEIGHT;
extern float TEX_SYN_BLUE_WEIGHT;



#ifdef __APPLE__
    #include <SDL/SDL.h>
    #include <SDL/SDL_thread.h>
#else
    #include <SDL.h>
    #include <SDL_thread.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "util.h"			//debug()
#include "sdl.h"			//noisify(), getPixel()
#include "gauss_pyramid.h"	//gauss_pyramid class
#include "hood.h"			//hood class


//Takes input surface and output size and returns an SDL_Surface of the specified
//size that contains a synthesized texture based off the input SDL_Surface
SDL_Surface *textureSynthesis(SDL_Surface *inputTexture, int w, int h);




#endif // TEX_SYN_H_INCLUDED
