#include "StarfieldMaterial.h"
#include "Background.h"
#include "StringF.h"
#include "graphics/Graphics.h"
#include "graphics/RendererGL2.h"
#include <sstream>

namespace Graphics {
namespace GL2 {
StarfieldProgram::StarfieldProgram(const std::string &filename, const std::string &defines)
{
	m_name = filename;
	m_defines = defines;
	LoadShaders(filename, defines);
	InitUniforms();
}

void StarfieldProgram::InitUniforms()
{
	//Program::InitUniforms(); //not needed except for emission
	twinkling.Init("twinkling", m_program);
	brightness.Init("brightness", m_program);
	time.Init("time", m_program);
	effect.Init("effect", m_program);
	starScaling.Init("starScaling", m_program);
}

void StarfieldMaterial::SetStarfieldUniforms()
{
	StarfieldProgram *p = static_cast<StarfieldProgram *>(m_program);
	const Background::Starfield::StarfieldParameters *sp = static_cast<Background::Starfield::StarfieldParameters *>(this->specialParameter0);

	p->twinkling.Set(sp->twinkling);
	p->brightness.Set(sp->brightness);
	p->time.Set(sp->time);
	p->effect.Set(sp->effect);
	p->starScaling.Set(sp->starScaling);
}

}
}
