// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_RINGMATERIAL_H
#define _OGL_RINGMATERIAL_H
/*
 * Planet ring material
 */
#include "libs.h"
#include "MaterialGL.h"
#include "Program.h"
namespace Graphics {

	namespace OGL {

		class RingMaterial : public Material {
		public:
			Program *CreateProgram(const MaterialDescriptor &);
			void Apply();
			void Unapply();
		};
	}
}
#endif
