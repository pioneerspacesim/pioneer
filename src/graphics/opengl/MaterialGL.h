// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_MATERIAL_H
#define _OGL_MATERIAL_H
/*
 * Multi-purpose OGL material.
 *
 * Generally, the idea is that a Program contains uniforms but
 * a material sets them, using the standard parameters of Graphics::Material
 * or whatever necessary to achieve an effect
 *
 * Programs are owned by the renderer, since they are shared between materials.
 */
#include "OpenGLLibs.h"
#include "graphics/Material.h"
#include "graphics/Types.h"
#include "graphics/opengl/UniformBuffer.h"

#include <memory>
#include <string>
#include <vector>

namespace Graphics {

	class RendererOGL;

	namespace OGL {

		class CommandList;
		class Shader;
		class Program;

		class Material : public Graphics::Material {
		public:
			Material() {}

			virtual bool IsProgramLoaded() const override final;
			virtual void SetShader(Shader *p);
			virtual const Shader *GetShader() const { return m_shader; }

			virtual bool SetTexture(size_t name, Texture *tex) override;
			virtual bool SetBuffer(size_t name, void *buffer, size_t size, BufferUsage usage) override;
			virtual bool SetBuffer(size_t name, UniformBuffer *buffer, uint32_t offset, uint32_t size);

			virtual bool SetPushConstant(size_t name, int i) override;
			virtual bool SetPushConstant(size_t name, float f) override;
			virtual bool SetPushConstant(size_t name, vector3f v3) override;
			virtual bool SetPushConstant(size_t name, vector3f v4, float f4) override;
			virtual bool SetPushConstant(size_t name, Color c) override;
			virtual bool SetPushConstant(size_t name, matrix3x3f mat3) override;
			virtual bool SetPushConstant(size_t name, matrix4x4f mat4) override;

		protected:
			friend class Graphics::RendererOGL;
			friend class OGL::CommandList;
			void Copy(OGL::Material *to) const;
			Program *EvaluateVariant();
			void UpdateDrawData();

			size_t m_renderStateHash;
			Shader *m_shader;
			Program *m_activeVariant;
			RendererOGL *m_renderer;

			uint32_t m_perDrawBinding;

			// TODO: not happy with this structure - makes it impossible to track
			// per-frame buffers vs occasionally-updated (set-once?) buffers
			// Interface needs some way to promise to update static buffers manually,
			// and needs automatic end-of-frame invalidation of per-frame buffer bindings
			struct BufferBinding {
				RefCountedPtr<UniformBuffer> buffer;
				GLuint offset;
				GLuint size;
			};

			std::unique_ptr<char[]> m_pushConstants;
			std::unique_ptr<Texture *[]> m_textureBindings;
			std::unique_ptr<BufferBinding[]> m_bufferBindings;
		};
	} // namespace OGL
} // namespace Graphics
#endif
