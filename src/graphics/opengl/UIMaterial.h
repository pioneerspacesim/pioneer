// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
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

		class UIMaterial : public Material {
		public:
			virtual Shader *CreateShader(const MaterialDescriptor &desc) override final
			{
				Shader *s = new Shader("ui", desc);
				s->AddTextureBinding("texture0", TextureType::TEXTURE_2D);
				return s;
			}
		};

	} // namespace OGL
} // namespace Graphics

#endif
