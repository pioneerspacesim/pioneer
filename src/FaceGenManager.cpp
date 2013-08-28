// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FaceGenManager.h"
#include "Lang.h"
#include "Pi.h"
#include "FileSystem.h"
#include "SDLWrappers.h"
#include "json/json.h"

//static
void FaceGenManager::Init()
{
	RefCountedPtr<FileSystem::FileData> fd = FileSystem::gameDataFiles.ReadFile("facegen/facegen.json");

	Json::Value data;
	if (!Json::Reader().parse(fd->GetData(), fd->GetData() + fd->GetSize(), data))
		throw SavedGameCorruptException();

	m_numHead			= data.get("Heads", 0).asInt();
	m_numEyes			= data.get("Eyes", 0).asInt();
	m_numNose			= data.get("Noses", 0).asInt();
	m_numMouth			= data.get("Mouths", 0).asInt();
	m_numHair			= data.get("Hairs", 0).asInt();
	m_numClothes		= data.get("Clothes", 0).asInt();
	m_numArmour			= data.get("Armours", 0).asInt();
	m_numAccessories	= data.get("Accessories", 0).asInt();
	m_numBackground		= data.get("Backgrounds", 0).asInt();
}

//static
void FaceGenManager::Destroy()
{
}
