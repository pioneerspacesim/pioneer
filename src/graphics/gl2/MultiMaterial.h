// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See COPYING.txt for details

#ifndef _GL2_MULTIMATERIAL_H
#define _GL2_MULTIMATERIAL_H
/*
 * A generic material & program for simple uses
 * textured/untextured, vertex colors or no...
 */
#include "GL2Material.h"
#include "Program.h"

namespace Graphics {

	namespace GL2 {
		class MultiProgram : public Program {
		public:
			MultiProgram(const MaterialDescriptor &);
		};

		class MultiMaterial : public Material {
		public:
			Program *CreateProgram(const MaterialDescriptor &);
			virtual void Apply();
			virtual void Unapply();
		};
	}
}

#endif
