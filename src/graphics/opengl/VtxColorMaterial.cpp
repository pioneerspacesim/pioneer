// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "VtxColorMaterial.h"
#include "RendererGL.h"
#include "StringF.h"
#include "TextureGL.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include <sstream>

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

	} // namespace OGL
} // namespace Graphics
