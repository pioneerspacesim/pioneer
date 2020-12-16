// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GasGiantMaterial.h"
#include "Camera.h"
#include "GeoSphere.h"
#include "RendererGL.h"
#include "StringF.h"
#include "graphics/Graphics.h"
#include <sstream>
#include "galaxy/AtmosphereParameters.h"

namespace Graphics {
	namespace OGL {

		// GasGiantProgram -------------------------------------------
		GasGiantProgram::GasGiantProgram(const std::string &filename, const std::string &defines)
		{
			m_name = filename;
			m_defines = defines;

			LoadShaders(filename, defines);
			InitUniforms();
			materialDataBlock.InitBlock("GasSphereData", m_program, 2);
		}

		// GasGiantSurfaceMaterial -----------------------------------
		GasGiantSurfaceMaterial::GasGiantSurfaceMaterial() :
			m_curNumShadows(0)
		{
			for (int i = 0; i < 4; i++)
				m_programs[i] = nullptr;
		}

		Program *GasGiantSurfaceMaterial::CreateProgram(const MaterialDescriptor &desc)
		{
			assert(desc.effect == EFFECT_GASSPHERE_TERRAIN);
			assert(desc.dirLights < 5);
			std::stringstream ss;
			ss << stringf("#define NUM_LIGHTS %0{u}\n", desc.dirLights);
			if (desc.dirLights > 0) {
				const float invNumLights = 1.0f / float(desc.dirLights);
				ss << stringf("#define INV_NUM_LIGHTS %0{f}\n", invNumLights);
			}
			if (desc.textures > 0)
				ss << "#define TEXTURE0\n";
			if (desc.quality & HAS_ATMOSPHERE)
				ss << "#define ATMOSPHERE\n";
			if (desc.quality & HAS_ECLIPSES)
				ss << "#define ECLIPSE\n";

			ss << stringf("#define NUM_SHADOWS %0{u}\n", m_curNumShadows);

			return new Graphics::OGL::GasGiantProgram("gassphere_base", ss.str());
		}

		void GasGiantSurfaceMaterial::SetProgram(Program *p)
		{
			m_programs[m_curNumShadows] = p;
			m_program = p;
		}

		void GasGiantSurfaceMaterial::Apply()
		{
			SwitchShadowVariant();
			SetGSUniforms();
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
			OGL::Material::Apply();

			GasGiantProgram *p = static_cast<GasGiantProgram *>(m_program);
			const GeoSphere::MaterialParameters params = *static_cast<GeoSphere::MaterialParameters *>(this->specialParameter0);
			const AtmosphereParameters ap = params.atmosphere;

			auto dataBlock = m_renderer->GetDrawUniformBuffer(sizeof(GasSphereDataBlock))->Allocate<GasSphereDataBlock>(2);

			dataBlock->atmosColor = ap.atmosCol.ToColor4f();
			dataBlock->geosphereAtmosFogDensity = ap.atmosDensity;
			dataBlock->geosphereAtmosInvScaleHeight = ap.atmosInvScaleHeight;
			dataBlock->geosphereAtmosTopRad = ap.atmosRadius;
			dataBlock->geosphereCenter = vector3f(ap.center);
			dataBlock->geosphereRadius = ap.planetRadius;
			dataBlock->geosphereInvRadius = 1.0f / ap.planetRadius;

			p->texture0.Set(this->texture0, 0);

			// we handle up to three shadows at a time
			vector3f shadowCentreX;
			vector3f shadowCentreY;
			vector3f shadowCentreZ;
			vector3f srad;
			vector3f lrad;
			vector3f sdivlrad;
			std::vector<Camera::Shadow>::const_iterator it = params.shadows.begin(), itEnd = params.shadows.end();
			int j = 0;
			while (j < 3 && it != itEnd) {
				shadowCentreX[j] = it->centre[0];
				shadowCentreY[j] = it->centre[1];
				shadowCentreZ[j] = it->centre[2];
				srad[j] = it->srad;
				lrad[j] = it->lrad;
				sdivlrad[j] = it->srad / it->lrad;
				++it;
				++j;
			}

			dataBlock->shadowCentreX = shadowCentreX;
			dataBlock->shadowCentreY = shadowCentreY;
			dataBlock->shadowCentreZ = shadowCentreZ;
			dataBlock->srad = srad;
			dataBlock->lrad = lrad;
			dataBlock->sdivlrad = sdivlrad;
		}

		void GasGiantSurfaceMaterial::SwitchShadowVariant()
		{
			const GeoSphere::MaterialParameters params = *static_cast<GeoSphere::MaterialParameters *>(this->specialParameter0);
			// std::vector<Camera::Shadow>::const_iterator it = params.shadows.begin(), itEnd = params.shadows.end();
			//request a new shadow variation
			if (m_curNumShadows != params.shadows.size()) {
				m_curNumShadows = std::min(Uint32(params.shadows.size()), 4U);
				if (m_programs[m_curNumShadows] == nullptr) {
					m_descriptor.numShadows = m_curNumShadows; //hax - so that GetOrCreateProgram will create a NEW shader instead of reusing the existing one
					m_programs[m_curNumShadows] = m_renderer->GetOrCreateProgram(this);
				}
				m_program = m_programs[m_curNumShadows];
			}
		}

	} // namespace OGL
} // namespace Graphics
