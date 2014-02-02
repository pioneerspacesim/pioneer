// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GL2_SKYBOX_MATERIAL_H_
#define _GL2_SKYBOX_MATERIAL_H_

/*
 * Renders a cube map as a skybox.
 */
#include "libs.h"
#include "Program.h"

namespace Graphics {
	namespace GL2 {
		class SkyboxMaterial : public Material {
		private:

		public:
			SkyboxMaterial() {
				texture0 = nullptr;
				fSkyboxFactor = 0.8f;
			}

			Program *CreateProgram(const MaterialDescriptor &) {
				return new Program("skybox", "");
			}

			virtual void Apply() {
				m_program->Use();
				if(texture0) {
					m_program->texture0.Set(texture0, 0);
				}
				const float em = (float(emissive.r) * 0.003921568627451f);
				m_program->shininess.Set(fSkyboxFactor * em);
			}

			// Skybox multiplier
			float fSkyboxFactor;
		};
	}
}

#endif
