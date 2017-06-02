// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GL_UIMATERIAL_H
#define _GL_UIMATERIAL_H
/*
 * A generic material & program for simple uses
 * textured/untextured, vertex colors or no...
 *
 */
#include "MaterialGL.h"
#include "Program.h"

namespace Graphics {

	namespace OGL {
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
