// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GL2VtxColorMaterial.h"
#include "graphics/Material.h"
#include "graphics/Graphics.h"
#include "GL2Texture.h"
#include "GL2Renderer.h"
#include <sstream>
#include "StringF.h"

namespace Graphics {
namespace GL2 {

VtxColorProgram::VtxColorProgram(const MaterialDescriptor &desc)
{
	m_name = "vtxColor";
	RENDERER_CHECK_ERRORS();

	LoadShaders(m_name, m_defines);
	InitUniforms();
}

Program *VtxColorMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	return new VtxColorProgram(desc);
}

}
}
