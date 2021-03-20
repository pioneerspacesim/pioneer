// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
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
			Material() {}
			// Create an appropriate program for this material.
			virtual Program *CreateProgram(const MaterialDescriptor &) { return nullptr; }
			// bind textures, set uniforms
			virtual void Apply() override {}
			virtual void Unapply() override {}
			virtual bool IsProgramLoaded() const override final { return false; }
			virtual void SetProgram(Program *p) {}

			virtual bool SetTexture(size_t name, Texture *tex) override { return false; }
			virtual bool SetBuffer(size_t name, void *buffer, size_t size, BufferUsage usage) override { return false; }

			virtual bool SetPushConstant(size_t name, int i) override { return false; }
			virtual bool SetPushConstant(size_t name, float f) override { return false; }
			virtual bool SetPushConstant(size_t name, vector3f v3) override { return false; }
			virtual bool SetPushConstant(size_t name, vector3f v4, float f4) override { return false; }
			virtual bool SetPushConstant(size_t name, Color c) override { return false; }
			virtual bool SetPushConstant(size_t name, matrix3x3f mat3) override { return false; }
			virtual bool SetPushConstant(size_t name, matrix4x4f mat4) override { return false; }
		};
	} // namespace Dummy
} // namespace Graphics

#endif
