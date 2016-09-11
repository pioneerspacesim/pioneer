// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "RenderTargetGL.h"
#include "TextureGL.h"

using namespace gl3x;

namespace Graphics { namespace OGL {

RenderBuffer::RenderBuffer()
{
	gl::glGenRenderbuffers(1, &buffer);
}

RenderBuffer::~RenderBuffer()
{
	gl::glDeleteRenderbuffers(1, &buffer);
}

void RenderBuffer::Bind()
{
	gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, buffer);
}

void RenderBuffer::Unbind()
{
	gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, 0);
}

void RenderBuffer::Attach(GLenum attachment)
{
	gl::glFramebufferRenderbuffer(gl::GL_FRAMEBUFFER, attachment, gl::GL_RENDERBUFFER, buffer);
}

RenderTarget::RenderTarget(const RenderTargetDesc &d)
: Graphics::RenderTarget(d)
, m_active(false)
{
	gl::glGenFramebuffers(1, &m_fbo);
}

RenderTarget::~RenderTarget()
{
	gl::glDeleteFramebuffers(1, &m_fbo);
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

void RenderTarget::SetCubeFaceTexture(const Uint32 face, Texture* t)
{
	const bool bound = m_active;
	if (!bound) Bind();
	//texture format should match the intended fbo format (aka. the one attached first)
	GLuint texId = 0;
	if (t) texId = static_cast<TextureGL*>(t)->GetTexture();
	gl::glFramebufferTexture2D(gl::GL_FRAMEBUFFER, gl::GL_COLOR_ATTACHMENT0, gl::GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texId, 0);
	m_colorTexture.Reset(t);
	if (!bound) Unbind();
}

void RenderTarget::SetColorTexture(Texture* t)
{
	const bool bound = m_active;
	if (!bound) Bind();
	//texture format should match the intended fbo format (aka. the one attached first)
	GLuint texId = 0;
	if (t) texId = static_cast<TextureGL*>(t)->GetTexture();
	gl::glFramebufferTexture2D(gl::GL_FRAMEBUFFER, gl::GL_COLOR_ATTACHMENT0, gl::GL_TEXTURE_2D, texId, 0);
	m_colorTexture.Reset(t);
	if (!bound) Unbind();
}

void RenderTarget::SetDepthTexture(Texture* t)
{
	assert(GetDesc().allowDepthTexture);
	const bool bound = m_active;
	if (!bound) Bind();
	if (!GetDesc().allowDepthTexture) return;
	GLuint texId = 0;
	if (t) texId = static_cast<TextureGL*>(t)->GetTexture();
	gl::glFramebufferTexture2D(gl::GL_FRAMEBUFFER, gl::GL_DEPTH_ATTACHMENT, gl::GL_TEXTURE_2D, texId, 0);
	m_depthTexture.Reset(t);
	if (!bound) Unbind();
}

void RenderTarget::Bind()
{
	gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, m_fbo);
	m_active = true;
}

void RenderTarget::Unbind()
{
	gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, 0);
	m_active = false;
}

bool RenderTarget::CheckStatus()
{
	return gl::glCheckFramebufferStatus(gl::GL_FRAMEBUFFER) == gl::GL_FRAMEBUFFER_COMPLETE;
}

void RenderTarget::CreateDepthRenderbuffer()
{
	assert(!GetDesc().allowDepthTexture);
	assert(m_active);
	m_depthRenderBuffer.Reset(new RenderBuffer());
	m_depthRenderBuffer->Bind();
	gl::glRenderbufferStorage(gl::GL_RENDERBUFFER, gl::GL_DEPTH_COMPONENT24, GetDesc().width, GetDesc().height);
	m_depthRenderBuffer->Attach(gl::GL_DEPTH_ATTACHMENT);
	m_depthRenderBuffer->Unbind();
}

} }
