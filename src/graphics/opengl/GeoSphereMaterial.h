// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_GEOSPHEREMATERIAL_H
#define _OGL_GEOSPHEREMATERIAL_H
/*
 * Programs & Materials used by terrain
 */
#include "OpenGLLibs.h"
#include "MaterialGL.h"
#include "Program.h"
#include "galaxy/StarSystem.h"

namespace Graphics {
	namespace OGL {
		class GeoSphereProgram : public Program {
		public:
			GeoSphereProgram(const std::string &filename, const std::string &defines);

			Uniform atmosColor;
			Uniform geosphereAtmosFogDensity;
			Uniform geosphereAtmosInvScaleHeight;
			Uniform geosphereAtmosTopRad; // in planet radii
			Uniform geosphereCenter;
			Uniform geosphereRadius; // (planet radius)
			Uniform geosphereInvRadius; // 1.0 / (planet radius)

			Uniform detailScaleHi;
			Uniform detailScaleLo;

			Uniform shadowCentreX;
			Uniform shadowCentreY;
			Uniform shadowCentreZ;
			Uniform srad;
			Uniform lrad;
			Uniform sdivlrad;

		protected:
			virtual void InitUniforms();
		};

		class GeoSphereSurfaceMaterial : public Material {
		public:
			GeoSphereSurfaceMaterial();
			virtual Program *CreateProgram(const MaterialDescriptor &) override;
			virtual void SetProgram(Program *p) override;
			virtual void Apply() override;
			virtual void Unapply() override;

		protected:
			void SetGSUniforms();
			// We actually have multiple programs at work here, one compiled for each of the number of shadows.
			// They are chosen/created based on what the current parameters passed in by the specialParameter0 are.
			void SwitchShadowVariant();
			Program* m_programs[4];	// 0 to 3 shadows
			Uint32 m_curNumShadows;
		};

		class GeoSphereSkyMaterial : public GeoSphereSurfaceMaterial {
		public:
			GeoSphereSkyMaterial();
			virtual Program *CreateProgram(const MaterialDescriptor &) override;
			virtual void Apply() override;
		};


		class GeoSphereStarMaterial : public Material {
		public:
			virtual Program *CreateProgram(const MaterialDescriptor &) override;
			virtual void Apply() override;
			virtual void Unapply() override;

		protected:
			void SetGSUniforms();
		};

	}
}
#endif
