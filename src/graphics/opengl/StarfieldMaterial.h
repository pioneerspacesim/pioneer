// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_STARFIELD_MATERIAL_H
#define _OGL_STARFIELD_MATERIAL_H
/*
 * Starfield material.
 * This does nothing very special except toggle POINT_SIZE
 * The Program requires setting intensity using the generic emission parameter
 */
#include "MaterialGL.h"
#include "OpenGLLibs.h"
#include "Program.h"
#include "TextureGL.h"

namespace Graphics {
	namespace OGL {

		class StarfieldMaterial : public Material {
		public:
			virtual Shader *CreateShader(const MaterialDescriptor &desc) override
			{
				Shader *s = new Shader("starfield", desc);
				s->AddTextureBinding("texture0", TextureType::TEXTURE_2D);
				return s;
			}
		};

	} // namespace OGL
} // namespace Graphics

#endif
