// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GeoSphereMaterial.h"

#include "Camera.h"
#include "GeoSphere.h"
#include "RendererGL.h"
#include "StringF.h"
#include "TextureGL.h"
#include "graphics/Graphics.h"
#include <sstream>

namespace Graphics {
	namespace OGL {

		void GeoSphereBaseMaterial::Apply()
		{
			PROFILE_SCOPED()
			SwitchShadowVariant();
			SetGSUniforms();
			Material::Apply();
		}

		struct GeoSphereDataBlock {
			vector3f geosphereCenter;
			float geosphereRadius;
			float geosphereInvRadius;
			float geosphereAtmosTopRad;
			float geosphereAtmosFogDensity;
			float geosphereAtmosInvScaleHeight;
			Color4f atmosColor;

			// Eclipse struct data
			alignas(16) vector3f shadowCentreX;
			alignas(16) vector3f shadowCentreY;
			alignas(16) vector3f shadowCentreZ;
			alignas(16) vector3f srad;
			alignas(16) vector3f lrad;
			alignas(16) vector3f sdivlrad;
			float __padding;

			// Texturing detail
			float detailScaleHi;
			float detailScaleLo;
		};
		static_assert(sizeof(GeoSphereDataBlock) == 160, "");
		static_assert(offsetof(GeoSphereDataBlock, detailScaleHi) == 144, "");

		static const float hiScale = 4.0f;
		static const float loScale = 0.5f;
		static const size_t s_texture0 = Renderer::GetName("texture0");
		static const size_t s_texture1 = Renderer::GetName("texture1");
		static const size_t s_geoSphereData = Renderer::GetName("GeoSphereData");
		void GeoSphereBaseMaterial::SetGSUniforms()
		{
			const GeoSphere::MaterialParameters params = *static_cast<GeoSphere::MaterialParameters *>(this->specialParameter0);
			const AtmosphereParameters ap = params.atmosphere;

			GeoSphereDataBlock dataBlock{};

			dataBlock.atmosColor = ap.atmosCol.ToColor4f();
			dataBlock.geosphereAtmosFogDensity = ap.atmosDensity;
			dataBlock.geosphereAtmosInvScaleHeight = ap.atmosInvScaleHeight;
			dataBlock.geosphereAtmosTopRad = ap.atmosRadius;
			dataBlock.geosphereCenter = vector3f(ap.center);
			dataBlock.geosphereRadius = ap.planetRadius;
			dataBlock.geosphereInvRadius = 1.0f / ap.planetRadius;

			if (this->texture0) {
				const float fDetailFrequency = pow(2.0f, float(params.maxPatchDepth) - float(params.patchDepth));

				dataBlock.detailScaleHi = hiScale * fDetailFrequency;
				dataBlock.detailScaleLo = loScale * fDetailFrequency;
			} else {
				dataBlock.detailScaleHi = 0.f;
				dataBlock.detailScaleLo = 0.f;
			}

			SetTexture(s_texture0, this->texture0);
			SetTexture(s_texture1, this->texture1);

			// we handle up to three shadows at a time
			std::vector<Camera::Shadow>::const_iterator it = params.shadows.begin(), itEnd = params.shadows.end();
			int j = 0;
			while (j < 3 && it != itEnd) {
				dataBlock.shadowCentreX[j] = it->centre[0];
				dataBlock.shadowCentreY[j] = it->centre[1];
				dataBlock.shadowCentreZ[j] = it->centre[2];
				dataBlock.srad[j] = it->srad;
				dataBlock.lrad[j] = it->lrad;
				dataBlock.sdivlrad[j] = it->srad / it->lrad;
				++it;
				++j;
			}

			Graphics::Material::SetBuffer(s_geoSphereData, &dataBlock, BufferUsage::BUFFER_USAGE_DYNAMIC);
		}

		void GeoSphereBaseMaterial::SwitchShadowVariant()
		{
			const GeoSphere::MaterialParameters params = *static_cast<GeoSphere::MaterialParameters *>(this->specialParameter0);
			//request a new shadow variation
			if (m_descriptor.numShadows != params.shadows.size()) {
				m_descriptor.numShadows = std::min(params.shadows.size(), size_t(4));
				m_activeVariant = m_shader->GetProgramForDesc(m_descriptor);
			}
		}

	} // namespace OGL
} // namespace Graphics
