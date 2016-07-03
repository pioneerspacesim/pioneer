// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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

		protected:
			virtual void InitUniforms();
		};

		class CloudSphereMaterial : public Material {
			virtual Program *CreateProgram(const MaterialDescriptor &);
			virtual void Apply();

		protected:
			void SetGSUniforms();
		};
	}
}
#endif
