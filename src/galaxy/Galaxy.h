#ifndef _GALAXY_H
#define _GALAXY_H

/* Sector density lookup */
namespace Galaxy {
	// lightyears
	extern const float GALAXY_RADIUS;
	extern const float SOL_OFFSET_X;
	extern const float SOL_OFFSET_Y;

	void Init();
	void Uninit();
	SDL_Surface *GetGalaxyBitmap();
	/* 0 - 255 */
	Uint8 GetSectorDensity(int sx, int sy, int sz);
}

#endif /* _GALAXY_H */
