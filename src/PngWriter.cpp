// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PngWriter.h"
#include "FileSystem.h"
#include "graphics/Graphics.h"
#include "utils.h"

#include <SDL_image.h>

namespace {
void write_png(FileSystem::FileSourceFS &fs, const std::string &path, const Uint8 *bytes, int width, int height, int stride, int bytes_per_pixel, bool strip_alpha)
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
	int dest_bpp = bytes_per_pixel;
	if (strip_alpha)
	{
		if (amask == 0)
		{
			strip_alpha = false;
		}
		else
		{
			amask = 0;
			dest_bpp--;
		}
	}

	// create a surface
	SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, dest_bpp * 8, rmask, gmask, bmask, amask);

	// flip the image vertically and copy to destination surface
	if (!strip_alpha)
	{
		// optimized pass when not stripping alpha.
		for (int y = 0; y < height; ++y)
		{
			Uint8 *dest = static_cast<Uint8*>(surface->pixels) + (height - (y+1)) * surface->pitch;
			const Uint8 *source = bytes + y * stride;
			memcpy(dest, source, stride);
		}
	}
	else
	{
		for (int y = 0; y < height; ++y)
		{
			Uint8 *dest = static_cast<Uint8*>(surface->pixels) + (height - (y+1)) * surface->pitch;
			const Uint8  *source = bytes + y * stride;
			for (int x = 0; x < width; ++x)
			{
				// We could assume bpp is 4/3 here and just do:
				//*dest = *source++;
				//*dest = *source++;
				//*dest = *source++;
				//++source;

				for (int channel = 0; channel < dest_bpp; ++channel)
				{
					dest[channel] = source[channel];
				}
				source += bytes_per_pixel;
				dest += dest_bpp;
			}
		}
	}

	// do the actual saving
	const std::string fname = FileSystem::JoinPathBelow(fs.GetRoot(), path);
	IMG_SavePNG(surface, fname.c_str());

	//cleanup after ourselves
	SDL_FreeSurface(surface);
	surface = nullptr;
}
} //namespace

void write_screenshot(const Graphics::ScreendumpState &sd, const char *destFile)
{
	const std::string dir = "screenshots";
	FileSystem::userFiles.MakeDirectory(dir);
	const std::string fname = FileSystem::JoinPathBelow(dir, destFile);

	write_png(FileSystem::userFiles, fname, sd.pixels.get(), sd.width, sd.height, sd.stride, sd.bpp, true);

	Output("Screenshot %s saved\n", fname.c_str());
}
