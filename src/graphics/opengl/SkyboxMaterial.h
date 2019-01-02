// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_SKYBOX_MATERIAL_H_
#define _OGL_SKYBOX_MATERIAL_H_

/*
 * Renders a cube map as a skybox.
 */
#include "OpenGLLibs.h"
#include "Program.h"

namespace Graphics {
	namespace OGL {
		class SkyboxMaterial : public Material {
		private:
		public:
			SkyboxMaterial()
			{
				texture0 = nullptr;
				fSkyboxFactor = 0.8f;
			}

			virtual Program *CreateProgram(const MaterialDescriptor &) override
			{
				return new Program("skybox", "");
			}

			virtual void Apply() override
			{
				m_program->Use();
				if (texture0) {
					m_program->texture0.Set(texture0, 0);
				}
				const float em = (float(emissive.r) * 0.003921568627451f);
				m_program->shininess.Set(fSkyboxFactor * em);
			}

			// Skybox multiplier
			float fSkyboxFactor;
		};
	} // namespace OGL
} // namespace Graphics

#endif
