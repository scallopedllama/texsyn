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

#ifndef SDL_H_INCLUDED
#define SDL_H_INCLUDED

/*
 * This file contains the functions that initialize and generally run SDL.
 * This includes SDL_Surface utility functions when they don't fit better
 * elsewhere ( like noisify() ).
 */

#include <stdio.h>
#include <stdlib.h>
#include "util.h"		//debug()
#include <time.h>	//time(NULL) used to seed random number generator

#ifdef __APPLE__
    #include <SDL/SDL.h>
#else
    #include <SDL.h>
#endif

//how many bits per pixel to use with sdl surfaces
#define TEX_BPP 32
//minimum / maximum windows sizes
#define MAX_WIDTH 1270
#define MIN_WIDTH 640
#define MAX_HEIGHT 900
#define MIN_HEIGHT 480

extern SDL_Surface *screen;

//initializes sdl with window width and height passed unless smaller
//than the minimum or greater than the maximum
void initSDL(int width = MIN_WIDTH, int height = MIN_HEIGHT);

//checks for events that should end the program like pressing esc
//or telling it to close
bool checkEvents();

//draws the passed surface to the screen, flips it, then returns.
//this does not loop to keep the window open or anything
//NOTE: this will checks the event queue and forces an exit if an exit was requested.
void dispSurface(SDL_Surface *disp);

//This is a wrapper function that creates an SDL_Surface with specified
//dimensions that is TEX_BPP bits per pixel laid out in RGBA format.
SDL_Surface *createSurface(int width, int height);

//takes the given surface and generates random noise for all pixels
void noisify(SDL_Surface *input);

//gets the pixel at (x, y) in the passed surface
Uint32 getPixel( SDL_Surface *surface, int x, int y );

//sets the pixel at (x, y) on the passed surface to be the passed pixel value
void putPixel( SDL_Surface *surface, int x, int y, Uint32 pixel );

#endif // SDL_H_INCLUDED
