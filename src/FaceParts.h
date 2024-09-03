// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef FACE_PARTS_H
#define FACE_PARTS_H

#include <SDL_stdinc.h>

// FaceParts deals with:
//   - Scanning the data/facegen/ directory and loading all the face part images
//   - Generating random faces from a particular seed and constraints
//   - Building a combined face image from a face descriptor

struct SDL_Surface;

namespace FaceParts {
	extern const int FACE_WIDTH;
	extern const int FACE_HEIGHT;

	// describes a face
	// components can be set to -1 to indicate that the attribute should be chosen randomly,
	// or set to a non-negative integer to specify a particular part
	struct FaceDescriptor {
		// selectors
		int species = -1;
		int race = -1;
		int gender = -1;

		// parts
		int head = -1;
		int eyes = -1;
		int nose = -1;
		int mouth = -1;
		int hairstyle = -1;
		int accessories = -1;
		int clothes = -1;
		int armour = -1;
	};

	void Init();
	void Uninit();

	int NumSpecies();
	int NumGenders(const int speciesIdx);
	int NumRaces(const int speciesIdx);

	int NumHeads(const int speciesIdx, const int raceIdx, const int genderIdx);
	int NumEyes(const int speciesIdx, const int raceIdx, const int genderIdx);
	int NumNoses(const int speciesIdx, const int raceIdx, const int genderIdx);
	int NumMouths(const int speciesIdx, const int raceIdx, const int genderIdx);
	int NumHairstyles(const int speciesIdx, const int raceIdx, const int genderIdx);
	int NumClothes(const int speciesIdx, const int raceIdx, const int genderIdx);
	int NumAccessories(const int speciesIdx, const int raceIdx, const int genderIdx);
	int NumArmour(const int speciesIdx, const int raceIdx, const int genderIdx);

	void PickFaceParts(FaceDescriptor &inout_face, const Uint32 seed);
	void BuildFaceImage(SDL_Surface *faceIm, const FaceDescriptor &face);
} // namespace FaceParts

#endif
