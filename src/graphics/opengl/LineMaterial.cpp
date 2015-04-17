// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LineMaterial.h"
#include "GeoSphere.h"
#include "Camera.h"
#include "StringF.h"
#include "graphics/Graphics.h"
#include "RendererGL.h"
#include <sstream>

namespace Graphics {
namespace OGL {

// LineProgram -------------------------------------------
LineProgram::LineProgram(const std::string &filename, const std::string &defines) : Program(filename, defines, true)
{
}

void LineProgram::InitUniforms()
{
	Program::InitUniforms();
	lineWidth.Init("lineWidth", m_program);
}

// LineMaterial -----------------------------------
Program *LineMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	assert(desc.effect == EFFECT_LINE);
	return new Graphics::OGL::LineProgram("line", "");
}

void LineMaterial::Apply()
{
	SetGSUniforms();
}

void LineMaterial::SetGSUniforms()
{
	OGL::Material::Apply();

	LineProgram *p = static_cast<LineProgram*>(m_program);
	if(this->specialParameter0) {
		const LineMaterialParams *params = static_cast<const LineMaterialParams*>(this->specialParameter0);
		p->lineWidth.Set(params->lineWidth);
	} else {
		p->lineWidth.Set(0.0f);
	}
}

}
}
