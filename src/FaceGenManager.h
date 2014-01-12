// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// DEPRECATED due to new ui system

#ifndef _FACEGENMANAGER
#define _FACEGENMANAGER

#include "SDLWrappers.h"

// fwd decl'
class Species;
class Race;

class FaceGenManager
{
public:
	struct TQueryResult {
		SDLSurfacePtr mHead;
		SDLSurfacePtr mEyes;
		SDLSurfacePtr mNose;
		SDLSurfacePtr mMouth;
		SDLSurfacePtr mHairstyle;
		SDLSurfacePtr mClothes;
		SDLSurfacePtr mArmour;
		SDLSurfacePtr mAccessories;
		SDLSurfacePtr mBackground;
	};

	static void Init();
	static void Destroy();

	// species & race dependent attributes
	static Sint32 NumSpecies();
	static Sint32 NumGenders(const Sint32 speciesIdx);
	static Sint32 NumRaces(const Sint32 speciesIdx);
	static Sint32 NumHeads(const Sint32 speciesIdx, const Sint32 raceIdx);
	static Sint32 NumEyes(const Sint32 speciesIdx, const Sint32 raceIdx);
	static Sint32 NumNoses(const Sint32 speciesIdx, const Sint32 raceIdx);
	static Sint32 NumMouths(const Sint32 speciesIdx, const Sint32 raceIdx);
	static Sint32 NumHairstyles(const Sint32 speciesIdx, const Sint32 raceIdx);

	// species generic attributes
	static Sint8 NumClothes(const Sint32 speciesIdx);
	static Sint8 NumArmour(const Sint32 speciesIdx);
	static Sint8 NumAccessories(const Sint32 speciesIdx);
	static Sint8 NumBackground(const Sint32 speciesIdx);

	static void GetImagesForCharacter( TQueryResult& out, const Sint32 speciesIdx, const int race, const int gender, const int head, const int eyes,
		const int nose, const int mouth, const int hair, const int clothes, const int armour,
		const int accessories, const int background );

	static void BlitFaceIm( SDLSurfacePtr &faceim, Sint8 &genderOut, const Uint32 flags, const Uint32 seed );
private:
	static std::vector<Species*> m_species;
};

#endif
