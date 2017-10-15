// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_CLOUDSPHERETERIAL_H
#define _OGL_CLOUDSPHERETERIAL_H
/*
 * Programs & Materials used by cloud sphere
 */
#include "libs.h"
#include "MaterialGL.h"
#include "Program.h"
#include "galaxy/StarSystem.h"

namespace Graphics {
	namespace OGL {
		class CloudSphereProgram : public Program {
		public:
			CloudSphereProgram(const std::string &filename, const std::string &defines);

			Uniform atmosColor;
			Uniform geosphereAtmosFogDensity;
			Uniform geosphereAtmosInvScaleHeight;
			Uniform geosphereAtmosTopRad; // in planet radii
			Uniform geosphereCenter;
			Uniform geosphereRadius; // (planet radius)
			Uniform geosphereInvRadius; // 1.0 / (planet radius)

			Uniform shadowCentreX;
			Uniform shadowCentreY;
			Uniform shadowCentreZ;
			Uniform srad;
			Uniform lrad;
			Uniform sdivlrad;
			
			Uniform time;

		protected:
			virtual void InitUniforms() override final;
		};

		class CloudSphereMaterial : public Material {
		public:
			CloudSphereMaterial();
			virtual Program *CreateProgram(const MaterialDescriptor &) override final;
			virtual void SetProgram(Program *p) override final;
			virtual void Apply() override final;

		protected:
			void SetGSUniforms();
			// We actually have multiple programs at work here, one compiled for each of the number of shadows.
			// They are chosen/created based on what the current parameters passed in by the specialParameter0 are.
			void SwitchShadowAndLightingVariant();
			Program* m_programs[4][5];	// 0 to 3 shadows, 0 to 4 lights
			Uint32 m_curNumShadows;
			Uint32 m_curNumLights;
		};
	}
}
#endif
