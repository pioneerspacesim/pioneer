// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_SKYBOX_MATERIAL_H_
#define _OGL_SKYBOX_MATERIAL_H_

/*
 * Renders a cube map as a skybox.
 */
#include "MaterialGL.h"
#include "OpenGLLibs.h"
#include "Shader.h"

namespace Graphics {
	namespace OGL {

		class SkyboxMaterial : public Material {
		public:
			virtual Shader *CreateShader(const MaterialDescriptor &desc) override
			{
				Shader *s = new Shader("skybox", desc);
				s->AddTextureBinding("texture0", TextureType::TEXTURE_CUBE_MAP);
				return s;
			}

			virtual void Apply() override
			{
				const float em = (float(emissive.r) * 0.003921568627451f);
				// Skybox multiplier
				shininess = 0.8 * em;

				OGL::Material::Apply();
			}
		};

	} // namespace OGL
} // namespace Graphics

#endif
