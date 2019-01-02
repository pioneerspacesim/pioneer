// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_MULTIMATERIAL_H
#define _OGL_MULTIMATERIAL_H
/*
 * A generic material & program for simple uses
 * textured/untextured, vertex colors or no...
 *
 */
#include "MaterialGL.h"
#include "Program.h"

namespace Graphics {

	namespace OGL {
		class MultiProgram : public Program {
		public:
			MultiProgram(const MaterialDescriptor &, int numLights = 0);
		};

		class MultiMaterial : public Material { //unlit
		public:
			virtual Program *CreateProgram(const MaterialDescriptor &) override;
			virtual void Apply() override;
			virtual void Unapply() override;
		};

		/*
		 * This material uses up to four variations of the Program,
		 * one for each directional light
		 */
		class LitMultiMaterial : public MultiMaterial {
		public:
			LitMultiMaterial();
			virtual Program *CreateProgram(const MaterialDescriptor &) override;
			virtual void SetProgram(Program *p) override;
			virtual void Apply() override;

		private:
			Program *m_programs[5];
			Uint32 m_curNumLights;
		};
	} // namespace OGL
} // namespace Graphics

#endif
