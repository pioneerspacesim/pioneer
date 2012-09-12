// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See COPYING.txt for details

#include "Color.h"

const Color4f Color::BLACK = Color(0.0f,0.0f,0.0f,1.0f);
const Color4f Color::WHITE = Color(1.0f,1.0f,1.0f,1.0f);
const Color4f Color::RED   = Color(1.0f,0.0f,0.0f,1.0f);
const Color4f Color::BLUE  = Color(0.0f,0.0f,1.0f,1.0f);

const Color4ub Color4ub::BLACK = Color4ub(0, 0, 0, 255);
const Color4ub Color4ub::WHITE = Color4ub(255, 255, 255, 255);

float Color4f::GetLuminance() const
{
	return (0.299f * r) + (0.587f * g) + (0.114f * b);
}
