// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_SHIELDMATERIAL_H
#define _OGL_SHIELDMATERIAL_H
/*
 * A generic material & program for simple uses
 * textured/untextured, vertex colors or no...
 *
 */
#include "MaterialGL.h"
#include "Shader.h"

namespace Graphics {

	namespace OGL {

		class ShieldMaterial : public Material { //unlit
		public:
			virtual Shader *CreateShader(const MaterialDescriptor &desc) override
			{
				Shader *s = new Shader("shield", desc);
				s->AddBufferBinding("ShieldData");
				return s;
			}
		};
	} // namespace OGL
} // namespace Graphics

#endif
