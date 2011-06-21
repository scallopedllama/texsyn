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

#include "gauss_pyramid.h"

void gaussianBlur(SDL_Surface *input, int filterRadius)
{
	if(filterRadius <= 0)	return;
 	int gaussWidth = filterRadius * 2 + 1;

	verboseDebug("gaussianBlur()\n");
	SDL_LockSurface(input);
	Uint32 *inPixels = (Uint32*)input->pixels;
	int w = input->w, h = input->h;
	int size = w * h;

	//data for each pass of the gaussian blur
	Uint32 *xPass = new Uint32[size];
	Uint32 *yPass = new Uint32[size];
	for(int i=0; i < size; i++)
		xPass[i] = yPass[i] = 0;

	//generate the kernel
	verboseDebug("\tGenerating Kernel\n");
	float *K = new float[gaussWidth];
	float mean = (float)gaussWidth / (float)GAUSS_SD;
	for(int i=0; i < filterRadius + 1; i++)
	{
		K[i] = (float) pow( sin((((i + 1) * M_PI_2 ) - mean) / gaussWidth) ,2.0) * GAUSS_SD;
		//mirror that value
		K[gaussWidth - 1 - i] = K[i];
	}
	float gaussSum = 0.0;
	for(int i=0; i < gaussWidth; i++) gaussSum += K[i];

	//frequently used variables
	Uint8 sourceR, sourceG, sourceB;
	Uint8 destR, destG, destB;
	int shift, dest, source;

	//let's do it!
	verboseDebug("\tRunning horizontal pass\n");
	for(int x = 0; x < w; x++)
	{
		for( int y = 0; y < h; y++ )
		{
			dest = y * w + x;

			//go through the kernel
			for(int k=0; k < gaussWidth; k++)
			{
				// Get pixel-shift (pixel dist between dest and source)
				shift = k - filterRadius;

				// Basic edge clamp
				source = y * w + (x + shift);
				if(x + shift <= 0 || x + shift >= w)
					source = dest;

				// Read source pixel through kernel matrix from pixels[array]
				SDL_GetRGB(inPixels[source], input->format, &sourceR, &sourceG, &sourceB);

				//combine source and dest pixels with gaussian weight
				SDL_GetRGB(xPass[dest], input->format, &destR, &destG, &destB);
				destR += Uint8( (sourceR * K[k]) / gaussSum);
				destG += Uint8( (sourceG * K[k]) / gaussSum);
				destB += Uint8( (sourceB * K[k]) / gaussSum);

				//store that value in the output
				xPass[dest] = SDL_MapRGB(input->format, destR, destG, destB);
			}
		}
	}
	verboseDebug("\tRunning vertical pass\n");
	for(int x = 0; x < w; x++)
	{
		for(int y = 0; y < h; y++)
		{
			dest = y * w + x;

			//go through kernel
			for(int k=0; k < gaussWidth; k++)
			{
				shift = k - filterRadius;

				//edge clamp
				source = (y + shift) * w + x;
				if( y + shift <= 0 || y + shift >= h)
					source = dest;

				// Read source pixel through kernel matrix from pixels[array]
				SDL_GetRGB(xPass[source], input->format, &sourceR, &sourceG, &sourceB);

				//combine source and dest pixels with gaussian weight
				SDL_GetRGB(yPass[dest], input->format, &destR, &destG, &destB);
				destR += Uint8( (sourceR * K[k]) / gaussSum);
				destG += Uint8( (sourceG * K[k]) / gaussSum);
				destB += Uint8( (sourceB * K[k]) / gaussSum);

				//store that value in the output
				yPass[dest] = SDL_MapRGB(input->format, destR, destG, destB);
			}
		}
	}

	verboseDebug("\tSaving data\n");
	//copy into output data
	for(int i=0; i < size; i++)
		inPixels[i] = yPass[i];

	//unlock the surfaces
	SDL_UnlockSurface(input);
	//SDL_UnlockSurface(output);

	verboseDebug("\tCleaning up\n");

	//delete those heap-allocated variables
	delete K;
	delete xPass;
	delete yPass;
	verboseDebug("done.\n");
}

gauss_pyramid::gauss_pyramid(SDL_Surface *source, int levels, bool blur)
{
	//get some parameters
	w = source->w;
	h = source->h;

	//how tall the pyramid is
	t = levels;
	if(t < 0)
	{
		t = 0;
		for(int n = MIN(w, h); n > 1; n /= 2)
			t++;
	}
	debug("Creating a %d x %d x %d Gaussian Pyramid\n", w, h, t);

	//set level 0
	pyramid.push_back(source);

	//generate the rest of the surfaces
	verboseDebug("\tGenerating pyramid levels\n");
	for(int i=1; i<t; i++)
	{
		verboseDebug("\tShrinking previous level\n");
		SDL_Surface *thisOne = zoomSurface(pyramid[i-1], 0.5, 0.5, SMOOTHING_ON);

		if(blur)
		{
			verboseDebug("\tApplying Gaussian Blur\n");
			gaussianBlur(thisOne);
		}

		pyramid.push_back(thisOne);
	}

	verboseDebug("done.\n");
}

gauss_pyramid::~gauss_pyramid()
{
	//free all the surfaces used in the pyramid except for the one passed
	//as source (with index 0)
	for(int i=1; i < pyramid.size(); i++)
		SDL_FreeSurface(pyramid[i]);
}

void gauss_pyramid::dumpData()
{
	debug("Dumping pyramid data\n");
	char output[25];
	for(int i=0; i < pyramid.size(); i++)
	{
		sprintf(output, "debug/pyr-%d.bmp", i);
		verboseDebug("\tSaving file #%d -- %s\n", i, output);
		SDL_SaveBMP(pyramid[i], output);
	}
	verboseDebug("done.\n");
}
