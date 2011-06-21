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

#include "sdl.h"

SDL_Surface *screen;

void initSDL(int width, int height)
{
	//see if the passed width and height are within range
	int useWidth = MIN_WIDTH, useHeight = MIN_HEIGHT;
	if(width > MIN_WIDTH && width < MAX_WIDTH)
		useWidth = width;
	if(height > MIN_HEIGHT && height < MAX_HEIGHT)
		useHeight = height;

    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError() );
        exit(EXIT_FAILURE);
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create a new window
    screen = SDL_SetVideoMode(useWidth, useHeight, TEX_BPP, SDL_HWSURFACE|SDL_DOUBLEBUF);
    if ( !screen )
    {
        fprintf(stderr, "Unable to set 640x480 video: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

bool checkEvents()
{
    // message processing loop
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // check for messages
        switch (event.type)
        {
            // exit if the window is closed
        case SDL_QUIT:
            return true;

            // check for keypresses
        case SDL_KEYDOWN:
            {
                // exit if ESCAPE is pressed
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    return true;
            }
        } // end switch
    } // end of message processing
    return false;
}

void dispSurface(SDL_Surface *disp)
{
    if (checkEvents())
    {
    	debug("Interrupt quit requested. Terminating.\n");
    	exit(EXIT_SUCCESS);
    }

    // centre the bitmap on screen
    SDL_Rect dstrect;
    dstrect.x = (screen->w - disp->w) / 2;
    dstrect.y = (screen->h - disp->h) / 2;

	// clear screen
	SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 255, 255, 255));

	// draw bitmap
	SDL_BlitSurface(disp, 0, screen, &dstrect);

	// finally, update the screen :)
	SDL_Flip(screen);
}

SDL_Surface *createSurface(int width, int height)
{
	//make a surface
	SDL_Surface *aSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, TEX_BPP, 0, 0, 0, 0);

	//convert it to display type (just in case)
	SDL_Surface *toReturn = SDL_DisplayFormat(aSurface);
	SDL_FreeSurface(aSurface);

	//make sure everything went ok
	if( !toReturn )
	{
		fprintf(stderr, "ERROR creating surface with dimensions %d x %d: %s\n", width, height, SDL_GetError());
		exit( EXIT_FAILURE );
	}

	//fill it with black
	SDL_FillRect(toReturn, NULL, SDL_MapRGB(toReturn->format, 0, 0, 0));

	//return that surface
	return toReturn;
}

void noisify(SDL_Surface *input)
{
	//seed the random number generator
	srand( time( NULL ) );

	//the surface must be locked in order to access the pixels directly
	SDL_LockSurface(input);

	//the surface's pixels can be directly manipulated via input->pixels.
	//this should be in the format RGBA with 32 bits per pixel = 8 bits per componet
	//not going to do anything with the Alpha values for now. may add something later

	//loop through it all in scanline order
	for(int c = 0; c < input->w; c++)
	{
		for(int r = 0; r < input->h; r++)
		{
			Uint8 red = rand() % 255;
			Uint8 green = rand() % 255;
			Uint8 blue = rand() % 255;

			//and set it
			Uint32 *pixels = (Uint32 *) input->pixels;
			pixels[ (c * input->w) + r ] = SDL_MapRGB(input->format, red, green, blue);
		}
	}

	//the surface must be unlocked for it to be used elsewhere
	SDL_UnlockSurface(input);
}

Uint32 getPixel( SDL_Surface *surface, int x, int y )
{
	if(x < 0 || x > surface->w || y < 0 || y > surface->h)
	{
		//handle edge cases
		x = (x + surface->w) % (surface->w);
		y = (y + surface->h) % (surface->h);
	}

	//lock surface
	SDL_LockSurface(surface);

	//Convert the pixels to 32 bit
	Uint32 *pixels = (Uint32 *)surface->pixels;

	//Get the requested pixel
	Uint32 toReturn = pixels[ y * surface->w + x ];

	//unlock surface
	SDL_UnlockSurface(surface);

	//return requested pixel
	return toReturn;
}

void putPixel( SDL_Surface *surface, int x, int y, Uint32 pixel )
{
	if(x < 0 || x > surface->w || y < 0 || y > surface->h)
	{
		//handle edge cases
		x = (x + surface->w) % (surface->w);
		y = (y + surface->h) % (surface->h);
	}

	//make sure it's fully opaque
	pixel |= 0xff000000;

	//lock surface
	SDL_LockSurface(surface);

	//Convert the pixels to 32 bit
	Uint32 *pixels = (Uint32 *)surface->pixels;

	//Set the pixel
	pixels[ y * surface->w + x ] = pixel;

	//unlock surface
	SDL_UnlockSurface(surface);

	Uint32 check = getPixel(surface, x, y);
	if(check != pixel)
		debug("WARNING: putPixel didn't take at (%d, %d)!\n", x, y);
}
