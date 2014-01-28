// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Galaxy.h"
#include "Sector.h"
#include "Pi.h"
#include "FileSystem.h"

namespace Galaxy {

// lightyears
const float GALAXY_RADIUS = 50000.0;
const float SOL_OFFSET_X = 25000.0;
const float SOL_OFFSET_Y = 0.0;

static SDL_Surface *s_galaxybmp;

void Init()
{
	static const std::string filename("galaxy.bmp");

	RefCountedPtr<FileSystem::FileData> filedata = FileSystem::gameDataFiles.ReadFile(filename);
	if (!filedata) {
		Output("Galaxy: couldn't load '%s'\n", filename.c_str());
		Pi::Quit();
	}

	SDL_RWops *datastream = SDL_RWFromConstMem(filedata->GetData(), filedata->GetSize());
	s_galaxybmp = SDL_LoadBMP_RW(datastream, 1);
	if (!s_galaxybmp) {
		Output("Galaxy: couldn't load: %s (%s)\n", filename.c_str(), SDL_GetError());
		Pi::Quit();
	}
}

void Uninit()
{
	if(s_galaxybmp) SDL_FreeSurface(s_galaxybmp);
}

SDL_Surface *GetGalaxyBitmap()
{
	return s_galaxybmp;
}

Uint8 GetSectorDensity(int sx, int sy, int sz)
{
	// -1.0 to 1.0
	float offset_x = (sx*Sector::SIZE + SOL_OFFSET_X)/GALAXY_RADIUS;
	float offset_y = (-sy*Sector::SIZE + SOL_OFFSET_Y)/GALAXY_RADIUS;
	// 0.0 to 1.0
	offset_x = Clamp((offset_x + 1.0)*0.5, 0.0, 1.0);
	offset_y = Clamp((offset_y + 1.0)*0.5, 0.0, 1.0);

	int x = int(floor(offset_x * (s_galaxybmp->w - 1)));
	int y = int(floor(offset_y * (s_galaxybmp->h - 1)));

	SDL_LockSurface(s_galaxybmp);
	int val = static_cast<unsigned char*>(s_galaxybmp->pixels)[x + y*s_galaxybmp->pitch];
	SDL_UnlockSurface(s_galaxybmp);
	// crappy unrealistic but currently adequate density dropoff with sector z
	val = val * (256 - std::min(abs(sz),256)) / 256;
	// reduce density somewhat to match real (gliese) density
	val /= 2;
	return Uint8(val);
}

} /* namespace Galaxy */
