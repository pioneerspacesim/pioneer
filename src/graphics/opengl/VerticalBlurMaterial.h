// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Copyright © 2013-14 Meteoric Games Ltd
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_VERTICAL_BLUR_MATERIAL_H_
#define _OGL_VERTICAL_BLUR_MATERIAL_H_

/*
 * Performs a gaussian vertical blur on a fullscreen quad.
 */
#include "libs.h"
#include "MaterialGL.h"
#include "Program.h"

namespace Graphics {
	namespace OGL {
		class VerticalBlurMaterial : public Material {
		public:
			VerticalBlurMaterial() {
				texture0 = nullptr;
			}

			Program *CreateProgram(const MaterialDescriptor &) {
				return new Program("postprocessing", "vblur", "", false);
			}

			virtual void Apply() {
				m_program->Use();
				if(texture0) {
					m_program->texture0.Set(texture0, 0);
				}
			}

			virtual void Unapply() {
				m_program->Unuse();
			}
		};
	}
}

#endif