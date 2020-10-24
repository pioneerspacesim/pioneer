// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "RenderTargetGL.h"
#include "TextureGL.h"

namespace Graphics {
	namespace OGL {

		RenderTarget::RenderTarget(const RenderTargetDesc &d) :
			Graphics::RenderTarget(d),
			m_active(false),
			m_depthRenderBuffer(0)
		{
			glGenFramebuffers(1, &m_fbo);
		}

		RenderTarget::~RenderTarget()
		{
			glDeleteFramebuffers(1, &m_fbo);
			glDeleteRenderbuffers(1, &m_depthRenderBuffer);
		}

		Texture *RenderTarget::GetColorTexture() const
		{
			return m_colorTexture.Get();
		}

		Texture *RenderTarget::GetDepthTexture() const
		{
			assert(GetDesc().allowDepthTexture);
			return m_depthTexture.Get();
		}

		void RenderTarget::SetCubeFaceTexture(const Uint32 face, Texture *t)
		{
			const bool bound = m_active;
			if (!bound) Bind();
			//texture format should match the intended fbo format (aka. the one attached first)
			GLuint texId = 0;
			if (t) texId = static_cast<TextureGL *>(t)->GetTextureID();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texId, 0);
			m_colorTexture.Reset(t);
			if (!bound) Unbind();
		}

		void RenderTarget::SetColorTexture(Texture *t)
		{
			const bool bound = m_active;
			if (!bound) Bind();
			//texture format should match the intended fbo format (aka. the one attached first)
			GLuint texId = 0;
			if (t) texId = static_cast<TextureGL *>(t)->GetTextureID();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GetDesc().numSamples ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, texId, 0);

			m_colorTexture.Reset(t);
			if (!bound) Unbind();
		}

		void RenderTarget::SetDepthTexture(Texture *t)
		{
			assert(GetDesc().allowDepthTexture);
			const bool bound = m_active;
			if (!bound) Bind();
			if (!GetDesc().allowDepthTexture) return;
			GLuint texId = 0;
			if (t) texId = static_cast<TextureGL *>(t)->GetTextureID();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				GetDesc().numSamples ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, texId, 0);
			m_depthTexture.Reset(t);
			if (!bound) Unbind();
		}

		void RenderTarget::Bind()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
			m_active = true;
		}

		void RenderTarget::Unbind()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
