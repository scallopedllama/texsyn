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

#include "hood.h"

hood::hood(gauss_pyramid *p, int curL, int x, int y, bool d)
{
    formatSurface = createSurface(1, 1);

	color = getPixel(p->getLevel(curL), x, y);

	//do this for all levels of the pyramid below this one if TEX_SYN_USE_MULTIRESOLUTION is defined
	int l = curL;
#ifdef TEX_SYN_USE_MULTIRESOLUTION
	for(l = p->getLevels() - 1; l >= curL; l--)
	{
#endif

		//get this level
		SDL_Surface *thisLevel = p->getLevel(l);

		//figure out what the scale of this layer is so that the
		//texton width can be be applied to it
		float scale = pow(0.5,  l);
		int useTextonDiameter = (int) ceil(scale * textonDiameter);

		//add this level
		addLevel(p, l, useTextonDiameter, int(scale * x), int(scale * y), l == curL);

#ifdef TEX_SYN_USE_MULTIRESOLUTION
	}
#endif

	if(d) dump(x, y);
}

void hood::addLevel(gauss_pyramid *p, int curL, int diameter, int x, int y, bool lowest)
{
	SDL_Surface *thisLevel = p->getLevel(curL);
	int halfWidth = (int)sqrt((float)diameter);
	if(halfWidth == 0) halfWidth++;

	//if this is the first layer (meaning everything after this pixel is garbage)
	//start at the current pixel
	int xOffset = 0, yOffset = 0, goal = diameter;
	//otherwise, get a square
	if(!lowest)
	{
		xOffset = halfWidth;
		yOffset = halfWidth;
		//for all levels that arn't the lowest, sample a whole square
		goal = (int)pow((float)halfWidth * 2 + 1, 2);
	}
	if(diameter == halfWidth == 1) goal = 1;

	for(int i = 0; i <= goal; i++)
	{

		n.push_back(getPixel(thisLevel, x + xOffset, y + yOffset));

		xOffset--;
		if(xOffset < -halfWidth)
		{
			yOffset--;
			xOffset = halfWidth;
		}
	}
}


void hood::dump(int ix, int iy)
{
	static int calls = 0;
	if(calls > 400 || ix < 20 || iy < 20)
		return;

	SDL_Surface *dbg = createSurface(textonDiameter, 1);
	SDL_LockSurface(dbg);
	for(int i = 0; i < textonDiameter; i++)
	{
		putPixel(dbg, i, 0, n[i]);
	}
	SDL_UnlockSurface(dbg);

	char fname[256];
	sprintf(fname, "debug/d%04d-%d,%d.bmp", calls, ix, iy);
	SDL_SaveBMP(dbg, fname);
	SDL_FreeSurface(dbg);
	calls++;
}

hood_pyramid::hood_pyramid(gauss_pyramid *p)
{
	//init values
	parent = p;
	int t = p->getLevels();

	//build hoods
	verboseDebug("\tAllocating and building neighborhoods\n");
	//allocate  (height) 2d arrays
	hoods = new hood***[t];
	//for each level
	for(int i = t - 1; i >= 0; i--)
	{
		//get the level
		SDL_Surface *thisLevel = p->getLevel(i);
		int lvlW = thisLevel->w, lvlH = thisLevel->h;

		//allocate a 1d array of arrays of pointers (x)
		hoods[i] = new hood**[lvlW];
		//for each x
		for(int j=0; j<lvlW; j++)
		{
			//allocate an array of pointers (y)
			hoods[i][j] = new hood*[lvlH];
			//for each y, allocate and generate the neighborhood
			for(int k=0; k <lvlH; k++)
				hoods[i][j][k] = new hood(p, i, j, k);
		}
	}
}

hood_pyramid::~hood_pyramid()
{
	//clean up the neighborhoods
	if(hoods)
	{
		int t = parent->getLevels();

		//for each level
		for(int i=0; i<t; i++)
		{
			//get the level
			SDL_Surface *thisLevel = parent->getLevel(i);
			int lvlW = thisLevel->w, lvlH = thisLevel->h;

			//for all the x values
			for(int j=0; j<lvlW; j++)
			{
				//for all the y values
				for(int k=0; k<lvlH; k++)
				{
					//delete this neighborhood
					delete hoods[i][j][k];
				}

				//delete this column of neighborhoods
				delete hoods[i][j];
			}

			//delete this row of neighborhoods
			delete hoods[i];
		}
		//delete this array of 2d arrays
		delete hoods;
	}
}
