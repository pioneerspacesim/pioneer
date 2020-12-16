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

		GeoSphereProgram::GeoSphereProgram(const std::string &filename, const std::string &defines)
		{
			m_name = filename;
			m_defines = defines;
			LoadShaders(filename, defines);
			InitUniforms();
			materialDataBlock.InitBlock("GeoSphereData", m_program, 2);
		}

		GeoSphereSurfaceMaterial::GeoSphereSurfaceMaterial() :
			m_curNumShadows(0)
		{
			for (int i = 0; i < 4; i++)
				m_programs[i] = nullptr;
		}

		Program *GeoSphereSurfaceMaterial::CreateProgram(const MaterialDescriptor &desc)
		{
			assert((desc.effect == EFFECT_GEOSPHERE_TERRAIN) ||
				(desc.effect == EFFECT_GEOSPHERE_TERRAIN_WITH_LAVA) ||
				(desc.effect == EFFECT_GEOSPHERE_TERRAIN_WITH_WATER));
			assert(desc.dirLights < 5);
			std::stringstream ss;
			ss << stringf("#define NUM_LIGHTS %0{u}\n", desc.dirLights);
			if (desc.dirLights > 0) {
				const float invNumLights = 1.0f / float(desc.dirLights);
				ss << stringf("#define INV_NUM_LIGHTS %0{f}\n", invNumLights);
			}
			if (desc.quality & HAS_ATMOSPHERE)
				ss << "#define ATMOSPHERE\n";
			if (desc.effect == EFFECT_GEOSPHERE_TERRAIN_WITH_LAVA)
				ss << "#define TERRAIN_WITH_LAVA\n";
			if (desc.effect == EFFECT_GEOSPHERE_TERRAIN_WITH_WATER)
				ss << "#define TERRAIN_WITH_WATER\n";
			if (desc.quality & HAS_ECLIPSES)
				ss << "#define ECLIPSE\n";

			ss << stringf("#define NUM_SHADOWS %0{u}\n", m_curNumShadows);

			return new Graphics::OGL::GeoSphereProgram("geosphere_terrain", ss.str());
		}

		void GeoSphereSurfaceMaterial::SetProgram(Program *p)
		{
			m_programs[m_curNumShadows] = p;
			m_program = p;
		}

		void GeoSphereSurfaceMaterial::Apply()
		{
			SwitchShadowVariant();
			SetGSUniforms();
		}

		void GeoSphereSurfaceMaterial::Unapply()
		{
			if (texture0) {
				static_cast<TextureGL *>(texture1)->Unbind();
				static_cast<TextureGL *>(texture0)->Unbind();
			}
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
		void GeoSphereSurfaceMaterial::SetGSUniforms()
		{
			OGL::Material::Apply();

			GeoSphereProgram *p = static_cast<GeoSphereProgram *>(m_program);
			const GeoSphere::MaterialParameters params = *static_cast<GeoSphere::MaterialParameters *>(this->specialParameter0);
			const AtmosphereParameters ap = params.atmosphere;

			auto dataBlock = m_renderer->GetDrawUniformBuffer(sizeof(GeoSphereDataBlock))->Allocate<GeoSphereDataBlock>(2);

			dataBlock->atmosColor = ap.atmosCol.ToColor4f();
			dataBlock->geosphereAtmosFogDensity = ap.atmosDensity;
			dataBlock->geosphereAtmosInvScaleHeight = ap.atmosInvScaleHeight;
			dataBlock->geosphereAtmosTopRad = ap.atmosRadius;
			dataBlock->geosphereCenter = vector3f(ap.center);
			dataBlock->geosphereRadius = ap.planetRadius;
			dataBlock->geosphereInvRadius = 1.0f / ap.planetRadius;

			if (this->texture0) {
				p->texture0.Set(this->texture0, 0);
				p->texture1.Set(this->texture1, 1);

				const float fDetailFrequency = pow(2.0f, float(params.maxPatchDepth) - float(params.patchDepth));

				dataBlock->detailScaleHi = hiScale * fDetailFrequency;
				dataBlock->detailScaleLo = loScale * fDetailFrequency;
			} else {
				dataBlock->detailScaleHi = 0.f;
				dataBlock->detailScaleLo = 0.f;
			}

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

		void GeoSphereSurfaceMaterial::SwitchShadowVariant()
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

		GeoSphereSkyMaterial::GeoSphereSkyMaterial() :
			GeoSphereSurfaceMaterial()
		{
		}

		Program *GeoSphereSkyMaterial::CreateProgram(const MaterialDescriptor &desc)
		{
			assert(desc.effect == EFFECT_GEOSPHERE_SKY);
			assert(desc.dirLights > 0 && desc.dirLights < 5);
			std::stringstream ss;
			ss << stringf("#define NUM_LIGHTS %0{u}\n", desc.dirLights);
			if (desc.dirLights > 0) {
				const float invNumLights = 1.0f / float(desc.dirLights);
				ss << stringf("#define INV_NUM_LIGHTS %0{f}\n", invNumLights);
			}
			ss << "#define ATMOSPHERE\n";
			if (desc.quality & HAS_ECLIPSES)
				ss << "#define ECLIPSE\n";

			ss << stringf("#define NUM_SHADOWS %0{u}\n", m_curNumShadows);

			return new Graphics::OGL::GeoSphereProgram("geosphere_sky", ss.str());
		}

		Program *GeoSphereStarMaterial::CreateProgram(const MaterialDescriptor &desc)
		{
			assert((desc.effect == EFFECT_GEOSPHERE_STAR));
			assert(desc.dirLights < 5);
			std::stringstream ss;
			ss << stringf("#define NUM_LIGHTS %0{u}\n", desc.dirLights);

			return new Graphics::OGL::Program("geosphere_star", ss.str());
		}

		void GeoSphereStarMaterial::Apply()
		{
			OGL::Material::Apply();
		}

		void GeoSphereStarMaterial::Unapply()
		{
			if (texture0) {
				static_cast<TextureGL *>(texture1)->Unbind();
				static_cast<TextureGL *>(texture0)->Unbind();
			}
		}

	} // namespace OGL
} // namespace Graphics
