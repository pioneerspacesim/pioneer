// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_FresnelColourMaterial_H
#define _OGL_FresnelColourMaterial_H
/*
 * A generic material & program for simple uses
 * textured/untextured, vertex colors or no...
 *
 */
#include "MaterialGL.h"
#include "Program.h"

namespace Graphics {

	namespace OGL {
		class FresnelColourMaterial : public Material { //unlit
		public:
			virtual Shader *CreateShader(const MaterialDescriptor &desc) override
			{
				return new Shader("FresnelColour", desc);
			}
		};
	} // namespace OGL
} // namespace Graphics

#endif
