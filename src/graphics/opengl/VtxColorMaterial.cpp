// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "VtxColorMaterial.h"
#include "graphics/Material.h"
#include "graphics/Graphics.h"
#include "TextureGL.h"
#include "RendererGL.h"
#include <sstream>
#include "StringF.h"

namespace Graphics {
namespace OGL {

VtxColorProgram::VtxColorProgram(const MaterialDescriptor &desc)
{
	m_name = "vtxColor";
	CHECKERRORS();

	LoadShaders(m_name, m_defines);
	InitUniforms();
}

Program *VtxColorMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	return new VtxColorProgram(desc);
}

}
}
