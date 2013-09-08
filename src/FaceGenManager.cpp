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

	static const Sint32 kMAX_GENDERS = 2;
}

class Race
{
public:
	Race(const Json::Value &data, const Sint32 race) {
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
		for(Sint32 gender = 0; gender < kMAX_GENDERS; ++gender) {
			for(Sint32 head = 0; head < m_numHeads; ++head) {
				snprintf(filename, sizeof(filename), "facegen/race_%d/head/head_%d_%d.png", race, gender, head);
				LoadImage(std::string(filename), m_heads);
			}

			for(Sint32 eyes = 0; eyes < m_numEyes; ++eyes) {
				snprintf(filename, sizeof(filename), "facegen/race_%d/eyes/eyes_%d_%d.png", race, gender, eyes);
				LoadImage(std::string(filename), m_eyes);
			}

			for(Sint32 nose = 0; nose < m_numNoses; ++nose) {
				snprintf(filename, sizeof(filename), "facegen/race_%d/nose/nose_%d_%d.png", race, gender, nose);
				LoadImage(std::string(filename), m_noses);
			}

			for(Sint32 mouth = 0; mouth < m_numMouths; ++mouth) {
				snprintf(filename, sizeof(filename), "facegen/race_%d/mouth/mouth_%d_%d.png", race, gender, mouth);
				LoadImage(std::string(filename), m_mouths);
			}

			for(Sint32 hair = 0; hair < m_numHairstyles; ++hair) {
				snprintf(filename, sizeof(filename), "facegen/race_%d/hair/hair_%d_%d.png", race, gender, hair);
				LoadImage(std::string(filename), m_hairstyles);
			}
		}
	}

	Sint8 NumHeads() const { return m_numHeads; }
	Sint8 NumEyes() const { return m_numEyes; }
	Sint8 NumNoses() const { return m_numNoses; }
	Sint8 NumMouths() const { return m_numMouths; }
	Sint8 NumHairstyles() const { return m_numHairstyles; }

	SDLSurfacePtr Head(const Sint32 index, const Sint32 gender)			{ assert(index<m_numHeads); return m_heads[(gender==0) ? index : (index+m_numHeads)]; }
	SDLSurfacePtr Eyes(const Sint32 index, const Sint32 gender)			{ assert(index<m_numEyes); return m_eyes[(gender==0) ? index : (index+m_numEyes)]; }
	SDLSurfacePtr Nose(const Sint32 index, const Sint32 gender)			{ assert(index<m_numNoses); return m_noses[(gender==0) ? index : (index+m_numNoses)]; }
	SDLSurfacePtr Mouth(const Sint32 index, const Sint32 gender)		{ assert(index<m_numMouths); return m_mouths[(gender==0) ? index : (index+m_numMouths)]; }
	SDLSurfacePtr Hairstyle(const Sint32 index, const Sint32 gender)	{ assert(index<m_numHairstyles); return m_hairstyles[(gender==0) ? index : (index+m_numHairstyles)]; }

private:
	// private methods
	
private:
	// private members
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

//static
Sint8 FaceGenManager::m_numClothes;
Sint8 FaceGenManager::m_numArmour;
Sint8 FaceGenManager::m_numAccessories;
Sint8 FaceGenManager::m_numBackground;
std::vector<Race*> FaceGenManager::m_races;
std::vector<SDLSurfacePtr> FaceGenManager::m_clothes;
std::vector<SDLSurfacePtr> FaceGenManager::m_armour;
std::vector<SDLSurfacePtr> FaceGenManager::m_accessories;
std::vector<SDLSurfacePtr> FaceGenManager::m_background;

//static
void FaceGenManager::Init()
{
	RefCountedPtr<FileSystem::FileData> fd = FileSystem::gameDataFiles.ReadFile("facegen/facegen.json");

	Json::Value data;
	if (!Json::Reader().parse(fd->GetData(), fd->GetData() + fd->GetSize(), data))
		throw SavedGameCorruptException();

	const Json::Value::UInt num_races = data.get("num_races", 0).asInt();
	m_races.reserve(num_races);
	char tempRace[32];
	for (Json::Value::ArrayIndex index = 0; index < num_races; ++index) {
		snprintf(tempRace, 32, "race_%d", index);
		m_races.push_back(new Race(data.get(tempRace, 0), index));
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
	for(Sint32 gender = 0; gender < kMAX_GENDERS; ++gender) {
		for(Sint32 cloth = 0; cloth < m_numClothes; ++cloth) {
			snprintf(filename, sizeof(filename), "facegen/clothes/cloth_%d_%d.png", gender, cloth);
			LoadImage(std::string(filename), m_clothes);
		}
	}
	for(Sint32 armour = 0; armour < m_numArmour; ++armour) {
		snprintf(filename, sizeof(filename), "facegen/clothes/armour_%d.png", armour);
		LoadImage(std::string(filename), m_armour);
	}
	for(Sint32 accessories = 0; accessories < m_numAccessories; ++accessories) {
		snprintf(filename, sizeof(filename), "facegen/accessories/acc_%d.png", accessories);
		LoadImage(std::string(filename), m_accessories);
	}
	for(Sint32 background = 0; background < m_numBackground; ++background) {
		snprintf(filename, sizeof(filename), "facegen/backgrounds/background_%d.png", background);
		LoadImage(std::string(filename), m_background);
	}

	printf("Face Generation source images loaded.\n");
}

//static
void FaceGenManager::Destroy()
{
	for (std::vector<Race*>::iterator it = m_races.begin(), itEnd = m_races.end(); it != itEnd; ++it) {
		delete (*it);
	}
	m_races.clear();
}

//static 
Sint32 FaceGenManager::NumHeads(const Sint32 raceIdx)
{
	assert(Uint32(raceIdx) < m_races.size());
	return m_races[raceIdx]->NumHeads();
}
//static 
Sint32 FaceGenManager::NumEyes(const Sint32 raceIdx)
{
	assert(Uint32(raceIdx) < m_races.size());
	return m_races[raceIdx]->NumEyes();
}
//static 
Sint32 FaceGenManager::NumNoses(const Sint32 raceIdx)
{
	assert(Uint32(raceIdx) < m_races.size());
	return m_races[raceIdx]->NumNoses();
}
//static 
Sint32 FaceGenManager::NumMouths(const Sint32 raceIdx)
{
	assert(Uint32(raceIdx) < m_races.size());
	return m_races[raceIdx]->NumMouths();
}
//static 
Sint32 FaceGenManager::NumHairstyles(const Sint32 raceIdx)
{
	assert(Uint32(raceIdx) < m_races.size());
	return m_races[raceIdx]->NumHairstyles();
}

//static 
void FaceGenManager::GetImagesForCharacter(TQueryResult& res, const int race, const int gender, const int head, const int eyes,
	const int nose, const int mouth, const int hair, const int clothes, const int armour,
	const int accessories, const int background)
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