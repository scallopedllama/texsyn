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

#ifndef HOOD_H_INCLUDED
#define HOOD_H_INCLUDED

/*
 * This file contains the class representing a neighborhood
 */

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "util.h"	//debug() textonDiameter
#ifdef __APPLE__
    #include <SDL/SDL.h>
#else
    #include <SDL.h>
#endif
#include "sdl.h"		//getPixel()
#include "gauss_pyramid.h" //gauss pyramid class

using namespace std;

class hood
{
	public:
		hood(gauss_pyramid *p, int curL, int x, int y, bool dump = false);
		~hood()
		{
		    SDL_FreeSurface(formatSurface);
		}

		inline Uint32 getColor(int i)
		{
			return n[i];
		}
		inline SDL_Surface *getFormatSurface()
		{
			return formatSurface;
		}
		inline int getColors()
		{
			return n.size();
		}

	private:
		//dumps this hood to a file in the debug folder
		void dump(int ix, int iy);

		//adds a neighborhood
		void addLevel(gauss_pyramid *p, int curL, int diameter, int x, int y, bool lowest);

		SDL_Surface *formatSurface;
		vector<Uint32> n;
		Uint32 color;
};

class hood_pyramid
{
	public:
		hood_pyramid(gauss_pyramid *pyramid);
		~hood_pyramid();

		inline hood *getHood(int i, int x, int y)
		{
			if(i < 0 || i >= parent->getLevels())
			{
				debug("HOOD PYRAMID WARNING: hood request level index out of bounds: lvl %d\n", i);
				i = parent->getLevels() - 1;
			}
			int w = parent->getLevel(i)->w;
			int h = parent->getLevel(i)->h;

			if( x < 0 || y < 0 || x >= w || y >= h )
			{
				debug("HOOD PYRAMID WARNING: hood request out of bounds: lvl %d at (%d, %d)\n", i, x, y);
				return NULL;
			}


			return hoods[i][x][y];
		}

	private:
		//3d array of pointers :-S
		hood ****hoods;

		gauss_pyramid *parent;
};

#endif // HOOD_H_INCLUDED
