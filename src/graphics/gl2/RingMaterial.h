// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GL2_RINGMATERIAL_H
#define _GL2_RINGMATERIAL_H
/*
 * Planet ring material
 */
#include "libs.h"
#include "GL2Material.h"
#include "Program.h"
namespace Graphics {

	namespace GL2 {

		class RingMaterial : public Material {
		public:
			Program *CreateProgram(const MaterialDescriptor &);
			void Apply();
			void Unapply();
		};
	}
}
#endif
