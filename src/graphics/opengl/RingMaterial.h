// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_RINGMATERIAL_H
#define _OGL_RINGMATERIAL_H
/*
 * Planet ring material
 */
#include "MaterialGL.h"
#include "OpenGLLibs.h"
#include "Shader.h"
namespace Graphics {

	namespace OGL {

		class RingMaterial : public Material {
		public:
			virtual Shader *CreateShader(const MaterialDescriptor &desc) override
			{
				assert(desc.textures == 1);
				Shader *s = new Shader("planetrings", desc);
				m_tex0name = s->AddTextureBinding("texture0", TextureType::TEXTURE_2D);
				return s;
			}

			virtual void Apply() override
			{
				SetTexture(m_tex0name, texture0);
				Material::Apply();
			}

		private:
			size_t m_tex0name;
		};
	} // namespace OGL
} // namespace Graphics
#endif
