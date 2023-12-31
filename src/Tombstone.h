// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TOMBSTONE_H
#define _TOMBSTONE_H

#include "Cutscene.h"

class Tombstone : public Cutscene {
public:
	Tombstone(Graphics::Renderer *r, int width, int height);
	virtual void Draw(float time);
};

#endif
