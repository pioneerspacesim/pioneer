// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_RINGMATERIAL_H
#define _OGL_RINGMATERIAL_H
/*
 * Planet ring material
 */
#include "OpenGLLibs.h"
#include "MaterialGL.h"
#include "Program.h"
namespace Graphics {

	namespace OGL {

		class RingMaterial : public Material {
		public:
			virtual Program *CreateProgram(const MaterialDescriptor &) override;
			virtual void Apply() override;
			virtual void Unapply() override;
		};
	}
}
#endif
