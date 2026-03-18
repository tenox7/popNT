/* SDL_image.h - minimal stub for MSVC 4.0 / NT 4.0 port */
#ifndef _SDL_IMAGE_H
#define _SDL_IMAGE_H

#include <SDL.h>

#define IMG_GetError SDL_GetError

SDL_Surface* IMG_Load(const char* file);
SDL_Surface* IMG_Load_RW(SDL_RWops* src, int freesrc);
int IMG_SavePNG(SDL_Surface* surface, const char* file);

#endif

