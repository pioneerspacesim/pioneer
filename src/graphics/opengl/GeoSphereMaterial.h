// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_GEOSPHEREMATERIAL_H
#define _OGL_GEOSPHEREMATERIAL_H
/*
 * Programs & Materials used by terrain
 */
#include "MaterialGL.h"
#include "OpenGLLibs.h"
#include "Program.h"

namespace Graphics {
	namespace OGL {
		class GeoSphereProgram : public Program {
		public:
			GeoSphereProgram(const std::string &filename, const std::string &defines);

			Uniform materialDataBlock;
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
			Program *m_programs[4]; // 0 to 3 shadows
			Uint32 m_curNumShadows;
		};

		class GeoSphereSkyMaterial : public GeoSphereSurfaceMaterial {
		public:
			GeoSphereSkyMaterial();
			virtual Program *CreateProgram(const MaterialDescriptor &) override;
		};

		class GeoSphereStarMaterial : public Material {
		public:
			virtual Program *CreateProgram(const MaterialDescriptor &) override;
			virtual void Apply() override;
			virtual void Unapply() override;
		};

	} // namespace OGL
} // namespace Graphics
#endif
