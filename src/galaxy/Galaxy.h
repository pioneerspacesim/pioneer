// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GALAXY_H
#define _GALAXY_H

#include <cstdio>

class SDL_Surface;

class Galaxy {
public:
	// lightyears
	const float GALAXY_RADIUS;
	const float SOL_OFFSET_X;
	const float SOL_OFFSET_Y;

	Galaxy();
	~Galaxy();

	SDL_Surface *GetGalaxyBitmap();
	/* 0 - 255 */
	Uint8 GetSectorDensity(int sx, int sy, int sz);

	void Dump(FILE* file, Sint32 centerX, Sint32 centerY, Sint32 centerZ, Sint32 radius);

private:
	SDL_Surface *m_galaxybmp;
};

#endif /* _GALAXY_H */
