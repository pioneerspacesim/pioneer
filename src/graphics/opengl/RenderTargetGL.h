// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_RENDERTARGET_H
#define _OGL_RENDERTARGET_H
/*
 * Framebuffer object with switchable target textures.
 * In theory you should use one texture format and size per FBO
 * 2013-May-05 left out stencil buffer because we don't need it now
 */
#include "OpenGLLibs.h"
#include "graphics/RenderTarget.h"

namespace Graphics {

	class RendererOGL;

	namespace OGL {
		class RenderStateCache;
		class CommandList;

		class RenderTarget : public Graphics::RenderTarget {
		public:
			enum Binding {
				READ = 1,
				DRAW = 2,
				BOTH = 3
			};

			~RenderTarget();
			virtual Texture *GetColorTexture() const override final;
			virtual Texture *GetDepthTexture() const override final;
			virtual void SetCubeFaceTexture(const Uint32 face, Texture *t) override final;
			virtual void SetColorTexture(Texture *) override final;
			virtual void SetDepthTexture(Texture *) override final;

		protected:
			friend class Graphics::RendererOGL;
			friend class RenderStateCache;
			friend class CommandList;

			RenderTarget(RendererOGL *, const RenderTargetDesc &);
			void Bind(Binding bind = BOTH);
			void Unbind(Binding bind = BOTH);
			void CreateDepthRenderbuffer();
			bool CheckStatus();

			RendererOGL *m_renderer;

			bool m_active;
			GLuint m_fbo;
			GLuint m_depthRenderBuffer;

			RefCountedPtr<Texture> m_colorTexture;
			RefCountedPtr<Texture> m_depthTexture;
		};

	} // namespace OGL

} // namespace Graphics

#endif
