// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PngWriter.h"
#include "FileSystem.h"
#include "graphics/Graphics.h"
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
	//SDL_Surface *surface = SDL_CreateRGBSurfaceFrom((void*)bytes, width, height, bytes_per_pixel * 8, width * bytes_per_pixel, rmask, gmask, bmask, amask);
	SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, bytes_per_pixel * 8, rmask, gmask, bmask, amask);

	// flip the image vertically
	int srcy = height - 1;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < stride; x++) {
			const int src_index = (srcy * stride) + x;
			const int dst_index = (y * stride) + x;
			for (int channel = 0; channel < bytes_per_pixel; channel++) {
				(static_cast<Uint8 *>(surface->pixels))[dst_index + channel] = bytes[src_index + channel];
			}
		}
		srcy--;
	}

	// do the actual saving
	const std::string fname = FileSystem::JoinPathBelow(fs.GetRoot(), path);
	IMG_SavePNG(surface, fname.c_str());

	//cleanup after ourselves
	SDL_FreeSurface(surface);
	surface = nullptr;
}

void write_screenshot(const Graphics::ScreendumpState &sd, const char *destFile)
{
	const std::string dir = "screenshots";
	FileSystem::userFiles.MakeDirectory(dir);
	const std::string fname = FileSystem::JoinPathBelow(dir, destFile);

	write_png(FileSystem::userFiles, fname, sd.pixels.get(), sd.width, sd.height, sd.stride, sd.bpp);

	Output("Screenshot %s saved\n", fname.c_str());
}
