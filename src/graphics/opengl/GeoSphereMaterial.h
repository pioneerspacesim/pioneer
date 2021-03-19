// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_GEOSPHEREMATERIAL_H
#define _OGL_GEOSPHEREMATERIAL_H
/*
 * Programs & Materials used by terrain
 */
#include "MaterialGL.h"
#include "OpenGLLibs.h"
#include "Shader.h"

namespace Graphics {
	namespace OGL {

		class GeoSphereBaseMaterial : public Material {
		public:
			virtual void Apply() override;

		protected:
			void SetGSUniforms();
			// We actually have multiple programs at work here, one compiled for each of the number of shadows.
			// They are chosen/created based on what the current parameters passed in by the specialParameter0 are.
			void SwitchShadowVariant();
		};

		class GeoSphereSurfaceMaterial : public GeoSphereBaseMaterial {
		public:
			virtual Shader *CreateShader(const MaterialDescriptor &desc) override
			{
				Shader *s = new Shader("geosphere_terrain", desc);
				s->AddTextureBinding("texture0", TextureType::TEXTURE_2D);
				s->AddTextureBinding("texture1", TextureType::TEXTURE_2D);
				s->AddBufferBinding("GeoSphereData");
				return s;
			}
		};

		class GeoSphereSkyMaterial : public GeoSphereBaseMaterial {
		public:
			virtual Shader *CreateShader(const MaterialDescriptor &desc) override
			{
				Shader *s = new Shader("geosphere_sky", desc);
				s->AddBufferBinding("GeoSphereData");
				return s;
			}
		};

		class GeoSphereStarMaterial : public GeoSphereBaseMaterial {
		public:
			virtual Shader *CreateShader(const MaterialDescriptor &desc) override
			{
				Shader *s = new Shader("geosphere_star", desc);
				s->AddBufferBinding("GeoSphereData");
				return s;
			}
		};

	} // namespace OGL
} // namespace Graphics
#endif
