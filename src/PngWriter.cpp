// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PngWriter.h"
#include "FileSystem.h"
#include "utils.h"

void write_png(FileSystem::FileSourceFS &fs, const std::string &path, const Uint8 *bytes, int width, int height, int stride, int bytes_per_pixel)
{
	// Set up the pixel format color masks for RGB(A) byte arrays.
	Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	int shift = ((sd.bpp == 3) ? 8 : 0;
	rmask = 0xff000000 >> shift;
	gmask = 0x00ff0000 >> shift;
	bmask = 0x0000ff00 >> shift;
	amask = 0x000000ff >> shift;
#else // little endian, like x86
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = (bytes_per_pixel == 3) ? 0 : 0xff000000;
#endif
	// create a surface
	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom((void*)bytes, width, height, bytes_per_pixel * 8, width * bytes_per_pixel, rmask, gmask, bmask, amask);
	const std::string fname = FileSystem::JoinPathBelow(fs.GetRoot(), path);
	// do the actual saving
	IMG_SavePNG(surface, fname.c_str());
	//cleanup after ourselves
	SDL_FreeSurface(surface);
	surface = nullptr;
}
