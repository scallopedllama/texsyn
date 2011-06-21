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

#include <stdlib.h>
#include "util.h"

#ifdef __APPLE__
    #include <SDL/SDL.h>
    #include <SDL/SDL_image.h>
#else
    #include <SDL.h>
    #include <SDL_image.h>
#endif

//for initSDL(), checkEvents(), and the screen surface
#include "sdl.h"
#include "tex_syn.h"

SDL_Surface *inputTexture;
int outputSize;

int main ( int argc, char** argv )
{
    //make sure we have all the necessary arguments
    if( argc < 4 )
    {
        fprintf(stderr, "Usage: %s (input texture filename) (texton neighborhood diameter) \n", argv[0]);
        fprintf(stderr, "                (output size) [number threads] [r weight] [g weight] [b weight]\n");
        fprintf(stderr, "   Options in () are required, options in [] are optional.\n");
		fprintf(stderr, "        Note that if you want to set r weight, number threads must be set too.\n");
        fprintf(stderr, "   Input texture reading is handled by SDL_Image so the file can be tga, bmp, \n");
        fprintf(stderr, "         pnm, xpm, xcf, pcx, gif, jpg, lbm, or png.\n");
        fprintf(stderr, "   If [number threads] is set to 0, no threads will be generated. If it is set\n");
        fprintf(stderr, "         to 1, one thread will be generated to do all the work. Default is %d.\n", TEX_SYN_THREADS);
        fprintf(stderr, "   [rgb weight] defines how much weight to give to the r, g, and b channels\n");
        fprintf(stderr, "         when calculating the similarity between two neighborhoods.\n");
        fprintf(stderr, "         These values are only used if rgb weighting is enabled in the code.\n");
        fprintf(stderr, "         Default Values are %f, %f, and %f respectively.\n", TEX_SYN_RED_WEIGHT, TEX_SYN_GREEN_WEIGHT, TEX_SYN_BLUE_WEIGHT);
        exit(EXIT_FAILURE);
    }
    //looks good, start loading values:

    //diameter
    textonDiameter = atoi(argv[2]);
    //make sure textonDiameter is odd, add one to it if it is even.
    if(textonDiameter % 2 == 0)
        textonDiameter++;

	//output size
    outputSize = atoi(argv[3]);

    //# threads
    if(argc >= 5)
    	TEX_SYN_THREADS = atoi(argv[4]);

    //r weight
    if(argc >= 6)
    	TEX_SYN_RED_WEIGHT = atof(argv[5]);

    //g weight
    if(argc >= 7)
    	TEX_SYN_GREEN_WEIGHT = atof(argv[6]);

    //b weight
    if(argc >= 8)
    	TEX_SYN_BLUE_WEIGHT = atof(argv[7]);

    debug("Will generate texture using file %s as a kernel and\n", argv[1]);
    debug("\tneighborhood size %d to generate unique %d x %d texture\n", textonDiameter, outputSize, outputSize);
#ifdef TEX_SYN_USE_MULTIRESOLUTION
	debug("\twith a multi-resolution synthesis algorithm.\n");
#else
	debug("\twith a single-resolution synthesis algorithm.\n");
#endif
	if(TEX_SYN_THREADS == 0)
		debug("No threads will be generated to compare neighborhoods.\n");
	else
		debug("%d threads will be generated to compare neighborhoods.\n", TEX_SYN_THREADS);
#ifdef TEX_SYN_WEIGHTED_COLORS
	debug("When comparing neighborhoods, red, green, and blue will be weighted\n");
	debug("\twith the values %f, %f, and %f respectively\n", TEX_SYN_RED_WEIGHT, TEX_SYN_GREEN_WEIGHT, TEX_SYN_BLUE_WEIGHT);
#else
	debug("When comparing neighborhoods, red, green, and blue will not be weighted\n");
#endif

    //initialize SDL
    debug("Initializing SDL\n");
    initSDL(outputSize, outputSize);

    // load an image
    debug("Loading Image %s\n", argv[1]);
    SDL_Surface *loadedTexture = IMG_Load(argv[1]);
    if (!loadedTexture)
    {
        printf("Unable to load image %s: %s\n", argv[1], SDL_GetError());
        return 1;
    }

    //convert to be the same format as the display (32 bit)
    debug("Convert input texture to useable format\n");
    inputTexture = SDL_DisplayFormat(loadedTexture);
    SDL_FreeSurface(loadedTexture);

    //run the texture synthesis
	SDL_Surface *outputTexture = textureSynthesis(inputTexture, outputSize, outputSize);

	//this is the texture that will be rendered on screen:
	SDL_Surface *renderTexture = outputTexture;
    // centre the bitmap on screen
    SDL_Rect dstrect;
    dstrect.x = (screen->w - renderTexture->w) / 2;
    dstrect.y = (screen->h - renderTexture->h) / 2;

    // program main loop
    debug("Entering display loop...\n");
    while (!checkEvents())
    {
        // DRAWING STARTS HERE

        // clear screen
        SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 255, 255, 255));

        // draw bitmap
        SDL_BlitSurface(renderTexture, 0, screen, &dstrect);

        // DRAWING ENDS HERE

        // finally, update the screen :)
        SDL_Flip(screen);
    } // end main loop

	//save output texture
	char stripped[256];
	char outName[256];
	int start = 0, end = 0;
	for(int i = strlen(argv[1]) - 1; i>=0; i--)
	{
		if( argv[1][i] == '/' && i > start )
			start = i + 1;
		if( argv[1][i] == '.' && i > end )
			end = i;
	}
	int i = 0;
	for( int n = start; n < end; n++, i++)
		stripped[i] = argv[1][n];
	stripped[i] = '\0';

	sprintf(outName, "synthesizedTextures/%s-%dx%d,%d.bmp", stripped, outputSize, outputSize, textonDiameter);
	debug("Saving the output image to %s\n", outName);
	if(SDL_SaveBMP(outputTexture, outName) < 0)
	{
		fprintf(stderr, "ERROR saving output texture to file %s: %s\n", outName, SDL_GetError());
		exit(EXIT_FAILURE);
	}

	debug("Cleaning up\n");
    // free loaded bitmap
    SDL_FreeSurface(inputTexture);
    SDL_FreeSurface(outputTexture);

	//note, sdl_quit doesn't need to be here because it's told to run
	//on quit in the init function.
    return 0;
}
