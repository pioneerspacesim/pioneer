// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _DUMMY_MATERIAL_H
#define _DUMMY_MATERIAL_H

#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"

namespace Graphics {

	class RendererDummy;

	namespace Dummy {

		class Program;

		class Material : public Graphics::Material {
		public:
			Material(RenderStateDesc rsd) :
				rsd(rsd) {}
			// Create an appropriate program for this material.
			virtual Program *CreateProgram(const MaterialDescriptor &) { return nullptr; }
			virtual bool IsProgramLoaded() const override final { return false; }
			virtual void SetProgram(Program *p) {}

			virtual bool SetTexture(size_t name, Texture *tex) override { return false; }
			virtual bool SetBuffer(size_t name, BufferBinding<UniformBuffer>) override { return false; }
			virtual bool SetBufferDynamic(size_t name, void *data, size_t size) override { return false; }

			virtual bool SetPushConstant(size_t name, int i) override { return false; }
			virtual bool SetPushConstant(size_t name, float f) override { return false; }
			virtual bool SetPushConstant(size_t name, vector3f v3) override { return false; }
			virtual bool SetPushConstant(size_t name, vector3f v4, float f4) override { return false; }
			virtual bool SetPushConstant(size_t name, Color c) override { return false; }
			virtual bool SetPushConstant(size_t name, matrix3x3f mat3) override { return false; }
			virtual bool SetPushConstant(size_t name, matrix4x4f mat4) override { return false; }

			RenderStateDesc rsd; // here to ensure validation works correctly
		};
	} // namespace Dummy
} // namespace Graphics

#endif
