// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
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
		class GasGiantProgram : public Program {
		public:
			GasGiantProgram(const std::string &filename, const std::string &defines);

			Uniform atmosColor;
			Uniform geosphereAtmosFogDensity;
			Uniform geosphereAtmosInvScaleHeight;
			Uniform geosphereAtmosTopRad; // in planet radii
			Uniform geosphereCenter;
			Uniform geosphereRadius; // planet radius
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

		class GasGiantSurfaceMaterial : public Material {
		public:
			GasGiantSurfaceMaterial();
			virtual Program *CreateProgram(const MaterialDescriptor &) override;
			virtual void SetProgram(Program *p) override;
			virtual void Apply() override;

		protected:
			void SetGSUniforms();
			// We actually have multiple programs at work here, one compiled for each of the number of shadows.
			// They are chosen/created based on what the current parameters passed in by the specialParameter0 are.
			void SwitchShadowVariant();
			Program *m_programs[4]; // 0 to 3 shadows
			Uint32 m_curNumShadows;
		};
	} // namespace OGL
} // namespace Graphics
#endif
