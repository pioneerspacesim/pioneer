// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_STARFIELD_MATERIAL_H
#define _OGL_STARFIELD_MATERIAL_H
/*
 * Starfield material.
 * This does nothing very special except toggle POINT_SIZE
 * The Program requires setting intensity using the generic emission parameter
 */
#include "OpenGLLibs.h"
#include "graphics/Material.h"
#include "Program.h"

namespace Graphics {
	namespace OGL {
		class StarfieldMaterial : public Material {
		public:
			virtual Program *CreateProgram(const MaterialDescriptor &) override {
				return new Program("starfield", "");
			}

			virtual void Apply() override {
				glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
				m_program->Use();
				m_program->emission.Set(this->emissive);
			}

			virtual void Unapply() override {
				glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
			}
		};
	}
}

#endif
