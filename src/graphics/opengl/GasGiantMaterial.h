// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_GASGIANTMATERIAL_H
#define _OGL_GASGIANTMATERIAL_H
/*
 * Programs & Materials used by terrain
 */
#include "MaterialGL.h"
#include "OpenGLLibs.h"
#include "Program.h"

namespace Graphics {
	namespace OGL {

		class GasGiantSurfaceMaterial : public Material {
		public:
			virtual Shader *CreateShader(const MaterialDescriptor &desc) override
			{
				Shader *s = new Shader("gassphere_base", desc);
				s->AddBufferBinding("GasSphereData");
				s->AddTextureBinding("texture0", TextureType::TEXTURE_CUBE_MAP);
				return s;
			}

			virtual void Apply() override;

		protected:
			void SetGSUniforms();
			// We actually have multiple programs at work here, one compiled for each of the number of shadows.
			// They are chosen/created based on what the current parameters passed in by the specialParameter0 are.
			void SwitchShadowVariant();
		};

	} // namespace OGL
} // namespace Graphics
#endif
