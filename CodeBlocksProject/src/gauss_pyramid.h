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

#ifndef GAUSSPYRAMID_H_INCLUDED
#define GAUSSPYRAMID_H_INCLUDED

/*
 * This file contains a class description of a Gaussian pyramid as required by the
 * texture synthesis routines.
 */

#include <stdio.h>
#include <stdlib.h>
#include "util.h"	//debug()
#include <math.h>	//sin() and M_PI_2 on non-windows
#include <vector>	//vector
#ifndef M_PI_2
	//stupid windows is stupid and doesn't doesn't provide M_PI_2 in math
	#define M_PI_2 1.57079632679489661923
#endif

#ifdef __APPLE__
    #include <SDL/SDL.h>
#else
    #include <SDL.h>
#endif

//for zoom functions
#include <SDL_rotozoom.h>

using namespace std;

#define GAUSS_SD 255;

//Takes an input SDL_Surface and applies a gaussian blur to it with the passed parameters
//Based off of the javascript code here: http://hyper-metrix.com/processing-js/docs/?page=Gaussian%20Blur
void gaussianBlur(SDL_Surface *input, int radius = 4);

class gauss_pyramid
{
public:
	//takes the input surface and generates a gaussian pyramid from it
	//the provided surface becomes the first level of the pyramid (it is not copied)
	//levels defines how many gaussian levels to use to generate the texture.
	//if levels is left set to -1, it will automatically decide how many levels to use.
	//if blur is true, it'll apply a gaussian blur to all levels except for the bottom
	gauss_pyramid(SDL_Surface *source, int levels = -1, bool blur = false);

	//frees all the surfaces used in the pyramid EXCEPT for the surface
	//passed for initialization.
	~gauss_pyramid();

	//returns the SDL_Surface for the indicated level.
	//note that index 0 is the full sized, unfilterd input texture.
	inline SDL_Surface *getLevel(int i)
	{
		//bounds checking
		if(i < 0 || i >= pyramid.size())
		{
			debug("PYRAMID WARNING: level request out of bounds: lvl %d (max is %d)\n", i, pyramid.size() - 1);
			i = pyramid.size() - 1;
		}

		return pyramid[i];
	}

	//returns the number of levels in the gaussian pyramid
	inline int getLevels()
	{
		return pyramid.size();
	}

	//reconstructs the pyramid. this is trivial since we're working with a gaussian pyramid
	//just return the bottom layer of the pyramid.
	inline SDL_Surface *reconstructPyramid()
	{
		return pyramid[0];
	}

	//dumps all the pyramid levels to files in the format
	// pyr[i].bmp where i is the pyramid level in the "debug" folder
	void dumpData();

private:
	vector<SDL_Surface*> pyramid;

	int w, h, t;
};



#endif
