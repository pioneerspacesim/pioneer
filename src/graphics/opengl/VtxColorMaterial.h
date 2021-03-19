// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GL_VTXCOLORMATERIAL_H
#define _GL_VTXCOLORMATERIAL_H
/*
 * A generic material & program for simple uses
 * textured/untextured, vertex colors or no...
 *
 */
#include "MaterialGL.h"
#include "Shader.h"

namespace Graphics {

	namespace OGL {

		class VtxColorMaterial : public Material {
		public:
			virtual Shader *CreateShader(const MaterialDescriptor &desc) override final
			{
				return new Shader("vtxColor", desc);
			}
		};

	} // namespace OGL
} // namespace Graphics

#endif
