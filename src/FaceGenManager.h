// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// DEPRECATED due to new ui system

#ifndef _FACEGENMANAGER
#define _FACEGENMANAGER

class FaceGenManager
{
public:
	static void Init();
	static void Destroy();
private:
	static unsigned int m_numHead;
	static unsigned int m_numEyes;
	static unsigned int m_numNose;
	static unsigned int m_numMouth;
	static unsigned int m_numHair;

	static unsigned int m_numClothes;
	static unsigned int m_numArmour;
	static unsigned int m_numAccessories;

	static unsigned int m_numBackground;
};

#endif
