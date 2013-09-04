// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FaceGenManager.h"
#include "Lang.h"
#include "Pi.h"
#include "FileSystem.h"
#include "SDLWrappers.h"
#include "StringF.h"
#include "json/json.h"

namespace
{
	void LoadImage(std::string &filename, std::vector<SDLSurfacePtr> &vec) {
		SDLSurfacePtr pSurf = LoadSurfaceFromFile(filename);
		if(!pSurf)
			printf("Failed to load image %s\n", filename.c_str());
		vec.push_back(pSurf);
	}

	static const Uint32 FACE_WIDTH = 295;
	static const Uint32 FACE_HEIGHT = 285;

	enum Flags {
		GENDER_RAND   = 0,
		GENDER_MALE   = (1<<0),
		GENDER_FEMALE = (1<<1),
		GENDER_MASK   = 0x03,   // <enum skip>

		ARMOUR = (1<<2),
	};

	static void _blit_image(const SDLSurfacePtr &s, SDLSurfacePtr &is, int xoff, int yoff)
	{
		// XXX what should this do if the image couldn't be loaded?
		if (! is) { return; }

		SDL_Rect destrec = { 0, 0, 0, 0 };
		destrec.x = ((FACE_WIDTH-is->w-1)/2)+xoff;
		destrec.y = yoff;
		SDL_BlitSurface(is.Get(), 0, s.Get(), &destrec);
	}
}

class Race
{
public:
	Race(const Json::Value &data, const Sint32 speciesIdx, const Sint32 race, const Sint8 maxGenders) : m_numGenders(maxGenders) 
	{
		m_numHeads = data.get("Heads", 0).asInt();
		m_numEyes = data.get("Eyes", 0).asInt();
		m_numNoses = data.get("Noses", 0).asInt();
		m_numMouths = data.get("Mouths", 0).asInt();
		m_numHairstyles = data.get("Hairs", 0).asInt();

		// reserve space for them all
		m_heads.reserve(m_numHeads);
		m_eyes.reserve(m_numEyes);
		m_noses.reserve(m_numNoses);
		m_mouths.reserve(m_numMouths);
		m_hairstyles.reserve(m_numHairstyles);

		char filename[256];
		// load the images
		for(Sint32 gender = 0; gender < m_numGenders; ++gender) {
			for(Sint32 head = 0; head < m_numHeads; ++head) {
				snprintf(filename, sizeof(filename), "facegen/species_%d/race_%d/head/head_%d_%d.png", speciesIdx, race, gender, head);
				LoadImage(std::string(filename), m_heads);
			}

			for(Sint32 eyes = 0; eyes < m_numEyes; ++eyes) {
				snprintf(filename, sizeof(filename), "facegen/species_%d/race_%d/eyes/eyes_%d_%d.png", speciesIdx, race, gender, eyes);
				LoadImage(std::string(filename), m_eyes);
			}

			for(Sint32 nose = 0; nose < m_numNoses; ++nose) {
				snprintf(filename, sizeof(filename), "facegen/species_%d/race_%d/nose/nose_%d_%d.png", speciesIdx, race, gender, nose);
				LoadImage(std::string(filename), m_noses);
			}

			for(Sint32 mouth = 0; mouth < m_numMouths; ++mouth) {
				snprintf(filename, sizeof(filename), "facegen/species_%d/race_%d/mouth/mouth_%d_%d.png", speciesIdx, race, gender, mouth);
				LoadImage(std::string(filename), m_mouths);
			}

			for(Sint32 hair = 0; hair < m_numHairstyles; ++hair) {
				snprintf(filename, sizeof(filename), "facegen/species_%d/race_%d/hair/hair_%d_%d.png", speciesIdx, race, gender, hair);
				LoadImage(std::string(filename), m_hairstyles);
			}
		}
	}

	Sint8 NumHeads() const { return m_numHeads; }
	Sint8 NumEyes() const { return m_numEyes; }
	Sint8 NumNoses() const { return m_numNoses; }
	Sint8 NumMouths() const { return m_numMouths; }
	Sint8 NumHairstyles() const { return m_numHairstyles; }

	SDLSurfacePtr Head(const Sint32 index, const Sint32 gender) const		{ 
		assert(index<m_numHeads); 
		return m_heads[(index+(gender*m_numHeads))]; 
	}
	SDLSurfacePtr Eyes(const Sint32 index, const Sint32 gender) const		{ 
		assert(index<m_numEyes); 
		return m_eyes[(index+(gender*m_numEyes))]; 
	}
	SDLSurfacePtr Nose(const Sint32 index, const Sint32 gender) const		{ 
		assert(index<m_numNoses); 
		return m_noses[(index+(gender*m_numNoses))]; 
	}
	SDLSurfacePtr Mouth(const Sint32 index, const Sint32 gender) const		{ 
		assert(index<m_numMouths); 
		return m_mouths[(index+(gender*m_numMouths))]; 
	}
	SDLSurfacePtr Hairstyle(const Sint32 index, const Sint32 gender) const	{ 
		assert(index<m_numHairstyles); 
		return m_hairstyles[(index+(gender*m_numHairstyles))]; 
	}

private:
	// private methods
	
private:
	// private members
	const Sint8 m_numGenders;
	Sint8 m_numHeads;
	Sint8 m_numEyes;
	Sint8 m_numNoses;
	Sint8 m_numMouths;
	Sint8 m_numHairstyles;

	std::vector<SDLSurfacePtr> m_heads;
	std::vector<SDLSurfacePtr> m_eyes;
	std::vector<SDLSurfacePtr> m_noses;
	std::vector<SDLSurfacePtr> m_mouths;
	std::vector<SDLSurfacePtr> m_hairstyles;
};

class Species
{
public:
	Species(const Json::Value &data, const Sint32 speciesIdx) 
	{
		m_numGenders = data.get("num_genders", 0).asInt();
		const Json::Value::UInt num_races = data.get("num_races", 0).asInt();
		m_races.reserve(num_races);
		char tempRace[32];
		for (Json::Value::ArrayIndex index = 0; index < num_races; ++index) {
			snprintf(tempRace, 32, "race_%d", index);
			m_races.push_back(new Race(data.get(tempRace, 0), speciesIdx, index, m_numGenders));
		}

		m_numClothes = data.get("Clothes", 0).asInt();
		m_numArmour = data.get("Armours", 0).asInt();
		m_numAccessories = data.get("Accessories", 0).asInt();
		m_numBackground = data.get("Backgrounds", 0).asInt();

		m_clothes.reserve(m_numClothes);
		m_armour.reserve(m_numArmour);
		m_accessories.reserve(m_numAccessories);
		m_background.reserve(m_numBackground);

		char filename[256];
		// load the images
		for(Sint32 gender = 0; gender < m_numGenders; ++gender) {
			for(Sint32 cloth = 0; cloth < m_numClothes; ++cloth) {
				snprintf(filename, sizeof(filename), "facegen/species_%d/clothes/cloth_%d_%d.png", speciesIdx, gender, cloth);
				LoadImage(std::string(filename), m_clothes);
			}
		}
		for(Sint32 armour = 0; armour < m_numArmour; ++armour) {
			snprintf(filename, sizeof(filename), "facegen/species_%d/clothes/armour_%d.png", speciesIdx, armour);
			LoadImage(std::string(filename), m_armour);
		}
		for(Sint32 accessories = 0; accessories < m_numAccessories; ++accessories) {
			snprintf(filename, sizeof(filename), "facegen/species_%d/accessories/acc_%d.png", speciesIdx, accessories);
			LoadImage(std::string(filename), m_accessories);
		}
		for(Sint32 background = 0; background < m_numBackground; ++background) {
			snprintf(filename, sizeof(filename), "facegen/species_%d/backgrounds/background_%d.png", speciesIdx, background);
			LoadImage(std::string(filename), m_background);
		}
	}
	~Species() {
		for (std::vector<Race*>::iterator it = m_races.begin(), itEnd = m_races.end(); it != itEnd; ++it) {
			delete (*it);
		}
		m_races.clear();
	}

	Sint32 NumGenders() const {
		return m_numGenders;
	}
	Sint32 NumRaces() const {
		return m_races.size();
	}
	Sint32 NumHeads(const Sint32 raceIdx) const
	{
		assert(Uint32(raceIdx) < m_races.size());
		return m_races[raceIdx]->NumHeads();
	}
	Sint32 NumEyes(const Sint32 raceIdx) const
	{
		assert(Uint32(raceIdx) < m_races.size());
		return m_races[raceIdx]->NumEyes();
	}
	Sint32 NumNoses(const Sint32 raceIdx) const
	{
		assert(Uint32(raceIdx) < m_races.size());
		return m_races[raceIdx]->NumNoses();
	}
	Sint32 NumMouths(const Sint32 raceIdx) const
	{
		assert(Uint32(raceIdx) < m_races.size());
		return m_races[raceIdx]->NumMouths();
	}
	Sint32 NumHairstyles(const Sint32 raceIdx) const
	{
		assert(Uint32(raceIdx) < m_races.size());
		return m_races[raceIdx]->NumHairstyles();
	}

	// generic attributes
	Sint8 NumClothes()		const { return m_numClothes; }
	Sint8 NumArmour()		const { return m_numArmour; }
	Sint8 NumAccessories()	const { return m_numAccessories; }
	Sint8 NumBackground()	const { return m_numBackground; }

	void GetImagesForCharacter(FaceGenManager::TQueryResult& res, const int race, const int gender, const int head, const int eyes,
		const int nose, const int mouth, const int hair, const int clothes, const int armour,
		const int accessories, const int background) const
	{
		res.mHead = m_races[race]->Head(head, gender);
		res.mEyes = m_races[race]->Eyes(eyes, gender);
		res.mNose = m_races[race]->Nose(nose, gender);
		res.mMouth = m_races[race]->Mouth(mouth, gender);
		res.mHairstyle = m_races[race]->Hairstyle(hair, gender);
		res.mClothes = m_clothes[clothes];
		res.mArmour = m_armour[armour];
		res.mAccessories = m_accessories[accessories];
		res.mBackground = m_background[background];
	}

private:
	Sint8 m_numGenders;
	Sint8 m_numClothes;
	Sint8 m_numArmour;
	Sint8 m_numAccessories;
	Sint8 m_numBackground;

	std::vector<Race*> m_races;

	std::vector<SDLSurfacePtr> m_clothes;
	std::vector<SDLSurfacePtr> m_armour;
	std::vector<SDLSurfacePtr> m_accessories;
	std::vector<SDLSurfacePtr> m_background;
};



//static
std::vector<Species*> FaceGenManager::m_species;

//static
void FaceGenManager::Init()
{
	RefCountedPtr<FileSystem::FileData> fd = FileSystem::gameDataFiles.ReadFile("facegen/facegen.json");

	Json::Value data;
	if (!Json::Reader().parse(fd->GetData(), fd->GetData() + fd->GetSize(), data))
		throw SavedGameCorruptException();

	const Json::Value::UInt num_species = data.get("num_species", 0).asInt();
	char tempSpecies[32];
	for (Json::Value::ArrayIndex index = 0; index < num_species; ++index) {
		snprintf(tempSpecies, 32, "species_%d", index);
		m_species.push_back(new Species(data.get(tempSpecies, 0), index));
	}

	printf("Face Generation source images loaded.\n");
}

//static
void FaceGenManager::Destroy()
{
	for (std::vector<Species*>::iterator it = m_species.begin(), itEnd = m_species.end(); it != itEnd; ++it) {
		delete (*it);
	}
	m_species.clear();
}

//static 
Sint32 FaceGenManager::NumGenders(const Sint32 speciesIdx) { 
	return m_species[speciesIdx]->NumGenders(); 
}
//static
Sint32 FaceGenManager::NumRaces(const Sint32 speciesIdx) { 
	return m_species[speciesIdx]->NumRaces(); 
}
//static 
Sint32 FaceGenManager::NumHeads(const Sint32 speciesIdx, const Sint32 raceIdx)
{
	assert(Uint32(speciesIdx) < m_species.size());
	return m_species[speciesIdx]->NumHeads(raceIdx);
}
//static 
Sint32 FaceGenManager::NumEyes(const Sint32 speciesIdx, const Sint32 raceIdx)
{
	assert(Uint32(speciesIdx) < m_species.size());
	return m_species[speciesIdx]->NumEyes(raceIdx);
}
//static 
Sint32 FaceGenManager::NumNoses(const Sint32 speciesIdx, const Sint32 raceIdx)
{
	assert(Uint32(speciesIdx) < m_species.size());
	return m_species[speciesIdx]->NumNoses(raceIdx);
}
//static 
Sint32 FaceGenManager::NumMouths(const Sint32 speciesIdx, const Sint32 raceIdx)
{
	assert(Uint32(speciesIdx) < m_species.size());
	return m_species[speciesIdx]->NumMouths(raceIdx);
}
//static 
Sint32 FaceGenManager::NumHairstyles(const Sint32 speciesIdx, const Sint32 raceIdx)
{
	assert(Uint32(speciesIdx) < m_species.size());
	return m_species[speciesIdx]->NumHairstyles(raceIdx);
}

// generic attributes
//static
Sint8 FaceGenManager::NumClothes(const Sint32 speciesIdx) { 
	return m_species[speciesIdx]->NumClothes(); 
}
//static 
Sint8 FaceGenManager::NumArmour(const Sint32 speciesIdx) { 
	return m_species[speciesIdx]->NumArmour(); 
}
//static 
Sint8 FaceGenManager::NumAccessories(const Sint32 speciesIdx) { 
	return m_species[speciesIdx]->NumAccessories(); 
}
//static 
Sint8 FaceGenManager::NumBackground(const Sint32 speciesIdx) { 
	return m_species[speciesIdx]->NumBackground(); 
}

//static 
void FaceGenManager::GetImagesForCharacter(TQueryResult& res, const Sint32 speciesIdx, const int race, const int gender, 
	const int head, const int eyes, const int nose, const int mouth, const int hair, const int clothes, const int armour,
	const int accessories, const int background)
{
	m_species[speciesIdx]->GetImagesForCharacter(res, race, gender, head, eyes, 
		nose, mouth, hair, clothes, armour, accessories, background);
}

//static 
void FaceGenManager::BlitFaceIm(SDLSurfacePtr &faceim, Sint8 &genderOut, const Uint32 flags, const Uint32 seed)
{
	Random rand(seed);

	const int species = (m_species.size()==1) ? 0 : rand.Int32(0,m_species.size()-1);
	const int race = rand.Int32(0,NumRaces(species)-1);

	int gender;
	switch (flags & GENDER_MASK) {
		case GENDER_MALE:
			gender = 0;
			break;
		case GENDER_FEMALE:
			gender = 1;
			break;
		case GENDER_RAND:
		default:
			gender = rand.Int32(0,NumGenders(species)-1);
			break;
	}
	genderOut = gender;

	const int head  = rand.Int32(0,NumHeads(species,race)-1);
	const int eyes  = rand.Int32(0,NumEyes(species,race)-1);
	const int nose  = rand.Int32(0,NumNoses(species,race)-1);
	const int mouth = rand.Int32(0,NumMouths(species,race)-1);
	const int hair  = rand.Int32(0,NumHairstyles(species,race)-1);

	const int clothes		= rand.Int32(0,NumClothes(species)-1);
	const int armour		= rand.Int32(0,NumArmour(species)-1);
	const int accessories	= rand.Int32(0,NumAccessories(species)-1);
	const int background	= rand.Int32(0,NumBackground(species)-1);
	
	FaceGenManager::TQueryResult res;
	FaceGenManager::GetImagesForCharacter(res, 0, race, gender, head, eyes, 
		nose, mouth, hair, clothes, armour, accessories, background);

	_blit_image(faceim, res.mBackground, 0, 0);
	_blit_image(faceim, res.mHead, 0, 0);

	if (!(flags & ARMOUR)) {
		_blit_image(faceim, res.mClothes, 0, 135);
	}

	_blit_image(faceim, res.mEyes, 0, 41);
	_blit_image(faceim, res.mNose, 1, 89);
	_blit_image(faceim, res.mMouth, 0, 155);

	if (!(flags & ARMOUR)) {
		if (rand.Int32(0,1)>0)
			_blit_image(faceim, res.mAccessories, 0, 0);

		_blit_image(faceim, res.mHairstyle, 0, 0);
	}
	else {
		_blit_image(faceim, res.mArmour, 0, 0);
	}
}
