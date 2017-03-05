// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _OGL_GENGASGIANTCOLOURMATERIAL_H
#define _OGL_GENGASGIANTCOLOURMATERIAL_H
/*
 * Material(s) used to generate
 *
 */
#include "MaterialGL.h"
#include "Program.h"

namespace Graphics {

	struct GenGasGiantColourMaterialParameters {
		const vector3d *v;
		float fracStep;
		float planetRadius;
		float time;
		float hueAdjust;
		vector3f frequency;
	};

	namespace OGL {

		enum GasGiantQuality {
			GEN_JUPITER_TEXTURE = 0,
			GEN_SATURN_TEXTURE,
			GEN_SATURN2_TEXTURE,
			// technically Ice Giants not Gas Giants...
			GEN_NEPTUNE_TEXTURE,
			GEN_NEPTUNE2_TEXTURE,
			GEN_URANUS_TEXTURE
		};

		class GenGasGiantColourProgram : public Program {
		public:
			GenGasGiantColourProgram(const MaterialDescriptor &);

			Uniform v0, v1, v2, v3;
			Uniform fracStep;
			Uniform permTexture;
			Uniform gradTexture;
			Uniform time;
			Uniform frequency;
			Uniform hueAdjust;

		protected:
			virtual void InitUniforms() override final;
		};

		class GenGasGiantColourMaterial : public Material { //unlit
		public:
			virtual Program *CreateProgram(const MaterialDescriptor &) override final;
			virtual void Apply() override final;
			virtual void Unapply() override final;
		};
	}
}

#endif
