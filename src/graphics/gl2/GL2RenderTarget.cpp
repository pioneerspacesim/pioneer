// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "graphics/gl2/GL2RenderTarget.h"
#include "graphics/TextureGL.h"

namespace Graphics { namespace GL2 {

RenderBuffer::RenderBuffer()
{
	glGenRenderbuffersEXT(1, &buffer);
}

RenderBuffer::~RenderBuffer()
{
	glDeleteRenderbuffersEXT(1, &buffer);
}

void RenderBuffer::Bind()
{
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, buffer);
}

void RenderBuffer::Unbind()
{
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
}

void RenderBuffer::Attach(GLenum attachment)
{
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, attachment, GL_RENDERBUFFER_EXT, buffer);
}

RenderTarget::RenderTarget(const RenderTargetDesc &d)
: Graphics::RenderTarget(d)
, m_active(false)
{
	glGenFramebuffersEXT(1, &m_fbo);
}

RenderTarget::~RenderTarget()
{
	glDeleteFramebuffersEXT(1, &m_fbo);
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

void RenderTarget::SetColorTexture(Texture* t)
{
	const bool bound = m_active;
	if (!bound) Bind();
	//texture format should match the intended fbo format (aka. the one attached first)
	GLuint texId = 0;
	if (t) texId = static_cast<TextureGL*>(t)->GetTexture();
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texId, 0);
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
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, texId, 0);
	m_depthTexture.Reset(t);
	if (!bound) Unbind();
}

void RenderTarget::Bind()
{
	assert(!m_active);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
	m_active = true;
}

void RenderTarget::Unbind()
{
	assert(m_active);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	m_active = false;
}

bool RenderTarget::CheckStatus()
{
	return glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT;
}

void RenderTarget::CreateDepthRenderbuffer()
{
	assert(!GetDesc().allowDepthTexture);
	assert(m_active);
	m_depthRenderBuffer.Reset(new RenderBuffer());
	m_depthRenderBuffer->Bind();
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, GetDesc().width, GetDesc().height);
	m_depthRenderBuffer->Attach(GL_DEPTH_ATTACHMENT_EXT);
	m_depthRenderBuffer->Unbind();
}

} }
