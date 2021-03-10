// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GasGiantMaterial.h"
#include "Camera.h"
#include "GeoSphere.h"
#include "RendererGL.h"
#include "StringF.h"
#include "galaxy/AtmosphereParameters.h"
#include "graphics/Graphics.h"
#include <sstream>

namespace Graphics {
	namespace OGL {

		void GasGiantSurfaceMaterial::Apply()
		{
			SwitchShadowVariant();
			SetGSUniforms();
			Material::Apply();
		}

		struct GasSphereDataBlock {
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
		};
		static_assert(sizeof(GasSphereDataBlock) == 144, "");

		void GasGiantSurfaceMaterial::SetGSUniforms()
		{
			const GeoSphere::MaterialParameters params = *static_cast<GeoSphere::MaterialParameters *>(this->specialParameter0);
			const AtmosphereParameters ap = params.atmosphere;

			GasSphereDataBlock dataBlock{};

			dataBlock.atmosColor = ap.atmosCol.ToColor4f();
			dataBlock.geosphereAtmosFogDensity = ap.atmosDensity;
			dataBlock.geosphereAtmosInvScaleHeight = ap.atmosInvScaleHeight;
			dataBlock.geosphereAtmosTopRad = ap.atmosRadius;
			dataBlock.geosphereCenter = vector3f(ap.center);
			dataBlock.geosphereRadius = ap.planetRadius;
			dataBlock.geosphereInvRadius = 1.0f / ap.planetRadius;

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

			static size_t s_texture0Name = Renderer::GetName("texture0");
			static size_t s_gasSphereDataName = Renderer::GetName("GasSphereData");
			SetTexture(s_texture0Name, this->texture0);
			Graphics::Material::SetBuffer(s_gasSphereDataName, &dataBlock, BufferUsage::BUFFER_USAGE_DYNAMIC);
		}

		void GasGiantSurfaceMaterial::SwitchShadowVariant()
		{
			const GeoSphere::MaterialParameters params = *static_cast<GeoSphere::MaterialParameters *>(this->specialParameter0);
			//request a new shadow variation
			// TODO: automatically select variants based on runtime state
			if (m_descriptor.numShadows != params.shadows.size()) {
				m_descriptor.numShadows = std::min(params.shadows.size(), size_t(4));
				m_activeVariant = m_shader->GetProgramForDesc(m_descriptor);
			}
		}

	} // namespace OGL
} // namespace Graphics
