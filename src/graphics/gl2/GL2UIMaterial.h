// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GL_GL2UIMATERIAL_H
#define _GL_GL2UIMATERIAL_H
/*
 * A generic material & program for simple uses
 * textured/untextured, vertex colors or no...
 *
 */
#include "GL2Material.h"
#include "GL2Program.h"

namespace Graphics {

	namespace GL2 {
		///////////////////////////////////////////////////////////////////////
		class UIProgram : public Program {
		public:
			UIProgram(const MaterialDescriptor &);
		};

		class UIMaterial : public Material {
		public:
			virtual Program *CreateProgram(const MaterialDescriptor &) override final;
			virtual void Apply() override final;
			virtual void Unapply() override final;
		};
	}
}

#endif
