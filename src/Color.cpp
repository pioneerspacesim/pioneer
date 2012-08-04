#include "Color.h"

const Color4f Color::BLACK = Color(0.0f,0.0f,0.0f,1.0f);
const Color4f Color::WHITE = Color(1.0f,1.0f,1.0f,1.0f);
const Color4f Color::RED   = Color(1.f,0.f,0.f,1.f);
const Color4f Color::BLUE  = Color(0.f,0.f,1.f,1.f);

const Color4ub Color4ub::BLACK = Color4ub(0, 0, 0, 255);
const Color4ub Color4ub::WHITE = Color4ub(255, 255, 255, 255);

float Color4f::GetLuminance() const
{
	return (0.299f * r) + (0.587f * g) + (0.114f * b);
}
