// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GL_GL2VTXCOLORMATERIAL_H
#define _GL_GL2VTXCOLORMATERIAL_H
/*
 * A generic material & program for simple uses
 * textured/untextured, vertex colors or no...
 *
 */
#include "GL2Material.h"
#include "GL2Program.h"

namespace Graphics {

	namespace GL2 {
		class VtxColorProgram : public Program {
		public:
			VtxColorProgram(const MaterialDescriptor &);
		};

		class VtxColorMaterial : public Material {
		public:
			virtual Program *CreateProgram(const MaterialDescriptor &);
		};
	}
}

#endif
