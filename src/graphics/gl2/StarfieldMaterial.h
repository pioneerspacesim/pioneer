// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GL2_STARFIELD_MATERIAL_H
#define _GL2_STARFIELD_MATERIAL_H
/*
 * Starfield material.
 * This does nothing very special except toggle POINT_SIZE
 * The Program requires setting intensity using the generic emission parameter
 */
#include "libs.h"
#include "Material.h"
#include "Program.h"

namespace Graphics {
	namespace GL2 {
		class StarfieldMaterial : public Material {
		public:
			Program *CreateProgram(const MaterialDescriptor &) {
				return new Program("starfield", "");
			}

			virtual void Apply() {
				glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
				m_program->Use();
				m_program->emission.Set(this->emissive);
			}

			virtual void Unapply() {
				glDisable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
			}
		};
	}
}

#endif
