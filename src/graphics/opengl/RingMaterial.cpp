// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "RingMaterial.h"
#include "RendererGL.h"
#include "StringF.h"
#include "TextureGL.h"
#include "graphics/Graphics.h"

namespace Graphics {
	namespace OGL {

		Program *RingMaterial::CreateProgram(const MaterialDescriptor &desc)
		{
			assert(desc.textures == 1);
			//pick light count and some defines
			unsigned int numLights = Clamp(desc.dirLights, 1u, 4u);
			std::string defines = stringf("#define NUM_LIGHTS %0{u}\n", numLights);
			return new Program("planetrings", defines);
		}

		void RingMaterial::Apply()
		{
			OGL::Material::Apply();

			assert(this->texture0);
			m_program->texture0.Set(this->texture0, 0);
		}

		void RingMaterial::Unapply()
		{
			static_cast<TextureGL *>(texture0)->Unbind();
		}

	} // namespace OGL
} // namespace Graphics
