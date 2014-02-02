// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GL2_FresnelColourMaterial_H
#define _GL2_FresnelColourMaterial_H
/*
 * A generic material & program for simple uses
 * textured/untextured, vertex colors or no...
 *
 */
#include "GL2Material.h"
#include "Program.h"

namespace Graphics {

	namespace GL2 {
		class FresnelColourProgram : public Program {
		public:
			FresnelColourProgram(const MaterialDescriptor &, int lights=0);
		};

		class FresnelColourMaterial : public Material { //unlit
		public:
			virtual Program *CreateProgram(const MaterialDescriptor &);
			virtual void Apply();
		};
	}
}

#endif
