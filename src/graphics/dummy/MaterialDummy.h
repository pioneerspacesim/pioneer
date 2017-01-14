// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _DUMMY_MATERIAL_H
#define _DUMMY_MATERIAL_H

#include "graphics/Material.h"

namespace Graphics {

	class RendererDummy;

	namespace Dummy {

		class Program;

		class Material : public Graphics::Material {
		public:
			Material() { }
			// Create an appropriate program for this material.
			virtual Program *CreateProgram(const MaterialDescriptor &) { return nullptr; }
			// bind textures, set uniforms
			virtual void Apply() override {}
			virtual void Unapply() override {}
			virtual bool IsProgramLoaded() const override final { return false; }
			virtual void SetProgram(Program *p) { }
			virtual void SetCommonUniforms(const matrix4x4f& mv, const matrix4x4f& proj) override {}
		};
	}
}

#endif
