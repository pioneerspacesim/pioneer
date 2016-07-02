// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _OGL_GENCLOUDSPHEREMATERIAL_H
#define _OGL_GENCLOUDSPHEREMATERIAL_H
/*
 * Material(s) used to generate 
 *
 */
#include "MaterialGL.h"
#include "Program.h"

namespace Graphics {

	struct GenCloudSphereMaterialParameters {
		const vector3d *v;
		float fracStep;
		float planetRadius;
		float time;
		float hueAdjust;
	};

	namespace OGL {
		class GenCloudSphereProgram : public Program {
		public:
			GenCloudSphereProgram(const MaterialDescriptor &);

			Uniform v0, v1, v2, v3;
			Uniform fracStep;
			Uniform permTexture;
			Uniform gradTexture;
			Uniform time;
			Uniform frequency;
			Uniform hueAdjust;

		protected:
			virtual void InitUniforms();
		};

		class GenCloudSphereMaterial : public Material { //unlit
		public:
			virtual Program *CreateProgram(const MaterialDescriptor &);
			virtual void Apply();
			virtual void Unapply();
		};
	}
}

#endif
