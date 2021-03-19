// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_MULTIMATERIAL_H
#define _OGL_MULTIMATERIAL_H
/*
 * A generic material & program for simple uses
 * textured/untextured, vertex colors or no...
 *
 */
#include "MaterialGL.h"
#include "Shader.h"
#include "Uniform.h"

namespace Graphics {

	namespace OGL {

		class MultiMaterial : public Material { //unlit
		public:
			virtual Shader *CreateShader(const MaterialDescriptor &) override;
			virtual void Apply() override;
		};

		/*
		 * This material uses up to four variations of the Program,
		 * one for each directional light
		 */
		class LitMultiMaterial : public MultiMaterial {
		public:
			LitMultiMaterial();
			virtual void Apply() override;

		private:
			Uint32 m_curNumLights;
		};
	} // namespace OGL
} // namespace Graphics

#endif
