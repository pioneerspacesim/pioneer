// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "OpenGLLibs.h"
#include "graphics/RenderState.h"

#include <vector>

namespace Graphics {
	class RendererOGL;

	namespace OGL {
		class TextureGL;
		class Program;
		struct UniformBufferBinding;

		class RenderStateCache {
		public:
			size_t GetActiveRenderStateHash() const { return m_activeRenderStateHash; }
			const RenderStateDesc &GetActiveRenderState() const { return m_activeRenderState; }

			void SetRenderState(size_t hash);
			void SetTexture(uint32_t index, TextureGL *texture);
			void SetBufferBinding(uint32_t index, UniformBufferBinding &binding);
			void SetProgram(Program *program);

		private:
			friend class Graphics::RendererOGL;
			RenderStateCache() = default;

			const RenderStateDesc &GetRenderState(size_t hash) const;
			size_t InternRenderState(const RenderStateDesc &rsd);
			void InvalidateState() { m_activeRenderStateHash = 0; }

			std::vector<TextureGL *> m_textureCache;
			std::vector<UniformBufferBinding> m_bufferCache;
			std::vector<std::pair<size_t, RenderStateDesc>> m_stateDescCache;
			GLuint m_activeProgram = 0;
			size_t m_activeRenderStateHash = 0;
			RenderStateDesc m_activeRenderState;
		};

	} // namespace OGL
} // namespace Graphics
