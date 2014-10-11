// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "VtxColorMaterial.h"
#include "graphics/Material.h"
#include "graphics/TextureGL.h"
#include "graphics/Graphics.h"
#include "graphics/RendererGL2.h"
#include <sstream>
#include "StringF.h"

namespace Graphics {
namespace GL2 {

VtxColorProgram::VtxColorProgram(const MaterialDescriptor &desc)
{
	m_name = "vtxColor";
	CheckRenderErrors();

	LoadShaders(m_name, m_defines);
	InitUniforms();
}

Program *VtxColorMaterial::CreateProgram(const MaterialDescriptor &desc)
{
	return new VtxColorProgram(desc);
}

}
}
