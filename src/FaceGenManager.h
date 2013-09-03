// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// DEPRECATED due to new ui system

#ifndef _FACEGENMANAGER
#define _FACEGENMANAGER

#include "SDLWrappers.h"

// fwd decl'
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

	// race dependent attributes
	static Sint32 NumRaces() { return m_races.size(); }
	static Sint32 NumHeads(const Sint32 raceIdx);
	static Sint32 NumEyes(const Sint32 raceIdx);
	static Sint32 NumNoses(const Sint32 raceIdx);
	static Sint32 NumMouths(const Sint32 raceIdx);
	static Sint32 NumHairstyles(const Sint32 raceIdx);

	// generic attributes
	static Sint8 NumClothes() { return m_numClothes; }
	static Sint8 NumArmour() { return m_numArmour; }
	static Sint8 NumAccessories() { return m_numAccessories; }
	static Sint8 NumBackground() { return m_numBackground; }

	static void GetImagesForCharacter( TQueryResult& out, const int race, const int gender, const int head, const int eyes,
		const int nose, const int mouth, const int hair, const int clothes, const int armour,
		const int accessories, const int background );
private:

	static Sint8 m_numClothes;
	static Sint8 m_numArmour;
	static Sint8 m_numAccessories;
	static Sint8 m_numBackground;

	static std::vector<Race*> m_races;

	static std::vector<SDLSurfacePtr> m_clothes;
	static std::vector<SDLSurfacePtr> m_armour;
	static std::vector<SDLSurfacePtr> m_accessories;
	static std::vector<SDLSurfacePtr> m_background;
};

#endif
