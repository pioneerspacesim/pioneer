#ifndef _INTRO_H
#define _INTRO_H
/*
 * Game intro
 */
#include "libs.h"
#include "Light.h"

namespace Graphics {
	class Renderer;
}
namespace Background {
	class Container;
}
class Model;

class Intro {
public:
	Intro(Graphics::Renderer*, const vector2f &size);
	~Intro();
	void Render(float time);

private:
	Graphics::Renderer *m_renderer;
	Background::Container *m_background;
	float m_aspectRatio;
	Model *m_model;

	//scene properties
	std::vector<Light> m_lights;
	Color4f m_ambientColor;
};
#endif