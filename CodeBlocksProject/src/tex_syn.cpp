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

#include "tex_syn.h"

int TEX_SYN_THREADS = 22;
float TEX_SYN_RED_WEIGHT = 0.85;
float TEX_SYN_GREEN_WEIGHT = 1.0;
float TEX_SYN_BLUE_WEIGHT = 0.6;


//this function determines how similar the two passed neighborhoods are by using
//a sum squared of difference
double match(hood *one, hood * two)
{
	if(one->getColors() != two->getColors())
		verboseDebug("WARNING! these two neighborhoods don't have the same number of colors!\n");

	double sum = 0.0;
	for(int i=0; i < MIN( one->getColors(), two->getColors() ); i++)
	{
	    /* bad version
	    double oneC = (double)one->getColor(i);
	    double twoC = (double)two->getColor(i);
	    sum += pow( oneC - twoC, 2.0);
	    */

	    SDL_Surface *oneFormat = one->getFormatSurface();
	    SDL_Surface *twoFormat = two->getFormatSurface();
		Uint8 red1 = 0;
		Uint8 green1 = 0;
		Uint8 blue1 = 0;
		Uint8 red2 = 0;
		Uint8 green2 = 0;
		Uint8 blue2 = 0;
		SDL_LockSurface(oneFormat);
		SDL_GetRGB(one->getColor(i), oneFormat->format, &red1, &green1, &blue1);
		SDL_UnlockSurface(oneFormat);
		SDL_LockSurface(twoFormat);
		SDL_GetRGB(two->getColor(i), twoFormat->format, &red2, &green2, &blue2);
		SDL_UnlockSurface(twoFormat);

#ifdef TEX_SYN_WEIGHTED_COLORS
		sum += (pow( double(red1) - double(red2), 2.0) * TEX_SYN_RED_WEIGHT);
		sum += (pow( double(green1) - double(green2), 2.0) * TEX_SYN_GREEN_WEIGHT);
		sum += (pow( double(blue1) - double(blue2), 2.0) * TEX_SYN_BLUE_WEIGHT);
#else
		sum += pow( double(red1) - double(red2), 2.0);
		sum += pow( double(green1) - double(green2), 2.0);
		sum += pow( double(blue1) - double(blue2), 2.0);
#endif
	}
	return sum;
}

struct threadData
{
    threadData(int by, int ey, int w, int curLevel, SDL_mutex *mut, hood *outHood, hood_pyramid *inHoodPyramid, gauss_pyramid *inPyramid, hood *bestHood, double *lastMatch, Uint32 *color, int *bestX, int *bestY)
    {
        this->by = by;
        this->ey = ey;
        this->w = w;
        this->curLevel = curLevel;
        this->mut = mut;
        this->outHood = outHood;
        this->inHoodPyramid = inHoodPyramid;
        this->inPyramid = inPyramid;
        this->bestHood = bestHood;
        this->lastMatch = lastMatch;
        this->color = color;
        this->bestX = bestX;
        this->bestY = bestY;
    }
    int by, ey, w, curLevel;
    SDL_mutex *mut;
    hood *outHood;
    hood_pyramid *inHoodPyramid;
    gauss_pyramid *inPyramid;
    hood *bestHood;
    double *lastMatch;
    Uint32 *color;
    int *bestX, *bestY;
};

//compares the neighborhoods of the input pyramid for rows [by, ey)
void checkRows(int by, int ey, int w, int curLevel, SDL_mutex *mut, hood *outHood, hood_pyramid *inHoodPyramid, gauss_pyramid *inPyramid, hood *bestHood, double *bestMatch, Uint32 *color, int *bestX, int *bestY)
{
	hood *threadBestHood = NULL;
	double threadBestMatch = 99999999.9;
	Uint32 threadColor = 0;
	int threadBestX = 0, threadBestY = 0;
    for(int sy = by; sy < ey; sy++)
    {
        //loop through the columns
        for(int sx = 0; sx < w; sx++)
        {
            hood *thisHood = inHoodPyramid->getHood(curLevel, sx, sy);
            //if there is not best hood
            if(!threadBestHood)
            {
            	//assign a default value to the best hood
            	threadBestHood = thisHood;
            	//and the last match
            	threadBestMatch = match(threadBestHood, outHood);
				threadColor = getPixel(inPyramid->getLevel(curLevel), 0, 0);
            	//skip the rest
            	continue;
            }
            double thisMatch = match(thisHood, outHood);
            if( thisMatch < threadBestMatch )
            {
            	//found a better match, reset those values
            	threadBestHood = thisHood;
            	threadBestMatch = thisMatch;
            	threadBestX = sx;
            	threadBestY = sy;
            	threadColor = getPixel(inPyramid->getLevel(curLevel), sx, sy);
            }
        }
    }

    //now that this thread is done with its calculations, reset the master bestHood if it's better than this one
    //do all this down here to cut down mutex lockings. like this, the worst case is that there will be
    //TEX_SYN_THREADS locks to the mutex to write, old version was possible for it to be the area of the input
    //image :-/
    //use a while here in case there is a different thread with the mutex locked
    while (threadBestMatch < *bestMatch)
	{
		if (TEX_SYN_THREADS > 0)
		{
			//try to lock the mutex
			if(SDL_mutexP(mut) != -1)
			{
				//success! reset all the master variables
				bestHood = threadBestHood;
				*bestMatch = threadBestMatch;
				*bestX =  threadBestX;
				*bestY = threadBestY;
				*color = threadColor;
				//unlock the mutex
				SDL_mutexV(mut);
			}
		}
		else
		{
			//reset all the master variables
			bestHood = threadBestHood;
			*bestMatch = threadBestMatch;
			*bestX =  threadBestX;
			*bestY = threadBestY;
			*color = threadColor;
		}
		//if the mutex lock failed, the values aren't set and it returns to the
		//while() which will make the thread try to lock the mutex again.
		//this will all continue until it gets the lock or another thread
		//sets bestMatch to a value that is better than this thread's.
	}
}

//all threads are initialized to run this function, it just takes the void* data type and breaks
//everything out to run the standard version above.
int threadCheckRows(void *data)
{
    //get it in a usable type
    threadData *dat = (threadData*) data;
    checkRows(dat->by, dat->ey, dat->w, dat->curLevel, dat->mut, dat->outHood, dat->inHoodPyramid, dat->inPyramid, dat->bestHood, dat->lastMatch, dat->color, dat->bestX, dat->bestY);

    //everything a ok
    return 0;
}

//a searching function used by textureSynthesis() to determine output pixel values
//returns the color to assign that pixel
Uint32 findBestMatch(hood_pyramid *inHoodPyramid, gauss_pyramid *inPyramid, gauss_pyramid *outPyramid, int curLevel, int x, int y)
{
	verboseDebug("\t\t\tBuilding output position neighborhood\n");
	hood *outHood = new hood(outPyramid, curLevel, x, y);

	//best match stuff
	hood *bestHood = NULL;
	double lastMatch = 999999999.9;
	Uint32 color = 0;
	int bestX = 0, bestY = 0;

	//others
	int w = inPyramid->getLevel(curLevel)->w, h = inPyramid->getLevel(curLevel)->h;

	verboseDebug("\t\t\tComparing neighborhoods\n");
    //Start with the multithreading stuff
	if (TEX_SYN_THREADS > 0)
	{
		SDL_mutex *mut = SDL_CreateMutex();

		//this is to make sure that no more than height threads are used
		int numThreads = (TEX_SYN_THREADS <= h) ? TEX_SYN_THREADS : h;

		//calculate how many rows to give to each thread. the first threads get the floor of the division,
		//the last one gets the remaining rows
		int firstThreadsRows = floor(float(h) / numThreads);

		//create the threads
		SDL_Thread **threads = new SDL_Thread*[TEX_SYN_THREADS];
		threadData **datas = new threadData*[TEX_SYN_THREADS];
		for( int t = 0; t < numThreads; t++)
		{
			//figure out what rows it needs to generate
			int from = firstThreadsRows * t;
			int to = firstThreadsRows * (t + 1);
			if(t == numThreads)
			{
				from = firstThreadsRows * t;
				to = h;
			}
			verboseDebug("\t\t\tthread #%d will check rows [%d, %d)\n", t, from, to);

			//create the data structure
			datas[t] = new threadData(from, to, w, curLevel, mut, outHood, inHoodPyramid, inPyramid, bestHood, &lastMatch, &color, &bestX, &bestY);
			//make the thread
			threads[t] = SDL_CreateThread(threadCheckRows, (void*)datas[t]);
		}

		//wait for them to finish
		for(int t = 0; t<numThreads; t++)
		{
			int rs = 0;
			SDL_WaitThread(threads[t], &rs);

			//my thread function will never return anything but 0 so this will never happen... I don't think anyway
			if(rs != 0)
				debug("WARNING: thread #%d returned status %d!\n", t, rs);

			//delete the data it used
			delete datas[t];
		}

		//delete the threads and datas arrays
		delete threads;
		delete datas;

		//clean it up
		SDL_DestroyMutex(mut);
	}
	else
		checkRows(0, h, w, curLevel, NULL, outHood, inHoodPyramid, inPyramid, bestHood, &lastMatch, &color, &bestX, &bestY);

	delete outHood;
	verboseDebug("\t\t\t\tBest match was %x at (%d, %d)\n", color, bestX, bestY);
	verboseDebug("\t\t\tDone\n");
	return color;
}

SDL_Surface *textureSynthesis(SDL_Surface *inputTexture, int w, int h)
{
	debug("Making output texture\n");						//I_s
	SDL_Surface *outputTexture = createSurface(w, h);

	debug("Generating noise on output texture\n");
	noisify(outputTexture);

	debug("Making output texture Gaussian Pyramid\n");				//G_s
	gauss_pyramid *outPyramid = new gauss_pyramid(outputTexture, -1, false);

	debug("Making input texture Gaussian Pyramid\n");				//G_a
	gauss_pyramid *inPyramid = new gauss_pyramid(inputTexture, outPyramid->getLevels());
	hood_pyramid *inHoodPyramid = new hood_pyramid(inPyramid);

	debug("Beginning texture synthesis...\n");
	int l = 0;
	double totTime = 0;
#ifdef TEX_SYN_USE_MULTIRESOLUTION
	for(l = outPyramid->getLevels() - 1; l >= 0; l--)
	{
#endif
		SDL_Surface *curLevel = outPyramid->getLevel(l);
		int lvlW = curLevel->w, lvlH = curLevel->h;
		debug("\tBeginning work on %d x %d level %d of the output pyramid..\n", lvlW, lvlH, l);

		//do this in scanline order
		for(int y = 0; y < lvlH; y++)
		{
            //for timing the operation
            clock_t start = clock();
			for(int x = 0; x < lvlW; x++)
			{
				verboseDebug("\t\tCalculating color for level %d at (%d, %d)\n", l, x, y);

				//update the display
				dispSurface(curLevel);

				//calculate the color to put here
				Uint32 color = findBestMatch(inHoodPyramid, inPyramid, outPyramid, l, x, y);

				//put that color on the pyramid level
				putPixel(curLevel, x, y, color);
			}
			totTime += ((double)clock() - start) / CLOCKS_PER_SEC;
			if(y % 20 == 0)
            	debug("\t\tTwenty rows done. Average time per row: %f s*\n", totTime / (y + 1));
            	//about the seconds* there, that value is not quite seconds, but it should be close and
            	//consistant in time.
		}

#ifdef TEX_SYN_USE_MULTIRESOLUTION
		//reset the timer every level
		totTime = 0.0;

		//blur the curent level (if it isn't the last)
		if(l > 0)
			gaussianBlur(curLevel);
	}
#endif

	//reconstruct the pyramid
	SDL_Surface *toReturn = outPyramid->reconstructPyramid();

	//free output bitmap
	debug("Cleaning up\n");
	delete outPyramid;
	delete inHoodPyramid;
	delete inPyramid;

	//return it up.
	return toReturn;
}
