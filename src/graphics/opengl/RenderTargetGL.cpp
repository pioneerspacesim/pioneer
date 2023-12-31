// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "RenderTargetGL.h"
#include "RenderStateCache.h"
#include "RendererGL.h"
#include "TextureGL.h"

namespace Graphics {
	namespace OGL {

		static GLuint get_binding(RenderTarget::Binding bind)
		{
			switch (bind) {
				case RenderTarget::READ: return GL_READ_FRAMEBUFFER;
				case RenderTarget::DRAW: return GL_DRAW_FRAMEBUFFER;
				case RenderTarget::BOTH: return GL_FRAMEBUFFER;
				// default value is never reached, calm down -Werror=return-type
				default: return 42;
			}
		}

		// RAII helper to push/pop a framebuffer for temporary modification
		struct ScopedActive {
			ScopedActive(RenderStateCache *c, RenderTarget *t) :
				m_cache(c)
			{
				m_last = m_cache->GetActiveRenderTarget();
				m_cache->SetRenderTarget(t);
			}
			~ScopedActive() { m_cache->SetRenderTarget(m_last); }

			RenderStateCache *m_cache;
			RenderTarget *m_last;
		};

		RenderTarget::RenderTarget(Graphics::RendererOGL *r, const RenderTargetDesc &d) :
			Graphics::RenderTarget(d),
			m_renderer(r),
			m_active(false),
			m_depthRenderBuffer(0)
		{
			glGenFramebuffers(1, &m_fbo);
		}

		RenderTarget::~RenderTarget()
		{
			glDeleteFramebuffers(1, &m_fbo);
			if (m_depthRenderBuffer)
				glDeleteRenderbuffers(1, &m_depthRenderBuffer);
		}

		Texture *RenderTarget::GetColorTexture() const
		{
			return m_colorTexture.Get();
		}

		Texture *RenderTarget::GetDepthTexture() const
		{
			return m_depthTexture.Get();
		}

		void RenderTarget::SetCubeFaceTexture(const Uint32 face, Texture *t)
		{
			ScopedActive binding(m_renderer->GetStateCache(), this);

			//texture format should match the intended fbo format (aka. the one attached first)
			GLuint texId = t ? static_cast<TextureGL *>(t)->GetTextureID() : 0;
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texId, 0);
			m_colorTexture.Reset(t);
		}

		void RenderTarget::SetColorTexture(Texture *t)
		{
			ScopedActive binding(m_renderer->GetStateCache(), this);

			//texture format should match the intended fbo format (aka. the one attached first)
			GLuint texId = t ? static_cast<TextureGL *>(t)->GetTextureID() : 0;
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GetDesc().numSamples ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, texId, 0);
			m_colorTexture.Reset(t);
		}

		void RenderTarget::SetDepthTexture(Texture *t)
		{
			assert(GetDesc().allowDepthTexture);
			if (!GetDesc().allowDepthTexture) return;
			ScopedActive binding(m_renderer->GetStateCache(), this);

			GLuint texId = t ? static_cast<TextureGL *>(t)->GetTextureID() : 0;
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				GetDesc().numSamples ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, texId, 0);
			m_depthTexture.Reset(t);
		}

		void RenderTarget::Bind(Binding bind)
		{
			glBindFramebuffer(get_binding(bind), m_fbo);
			m_active = true;
		}

		void RenderTarget::Unbind(Binding bind)
		{
			glBindFramebuffer(get_binding(bind), 0);
			m_active = false;
		}

		bool RenderTarget::CheckStatus()
		{
			return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		}

		void RenderTarget::CreateDepthRenderbuffer()
		{
			assert(!GetDesc().allowDepthTexture);
			assert(m_active);
			assert(m_depthRenderBuffer == 0);

			glGenRenderbuffers(1, &m_depthRenderBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, GetDesc().numSamples, GL_DEPTH_COMPONENT32F, GetDesc().width, GetDesc().height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);
		}

	} // namespace OGL
} // namespace Graphics
