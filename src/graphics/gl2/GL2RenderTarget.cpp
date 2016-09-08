// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GL2RenderTarget.h"
#include "GL2Texture.h"

using namespace gl21;

namespace Graphics { namespace GL2 {

RenderBuffer::RenderBuffer()
{
	gl::GenRenderbuffersEXT(1, &buffer);
}

RenderBuffer::~RenderBuffer()
{
	gl::DeleteRenderbuffersEXT(1, &buffer);
}

void RenderBuffer::Bind()
{
	gl::BindRenderbufferEXT(gl::RENDERBUFFER_EXT, buffer);
}

void RenderBuffer::Unbind()
{
	gl::BindRenderbufferEXT(gl::RENDERBUFFER_EXT, 0);
}

void RenderBuffer::Attach(GLenum attachment)
{
	gl::FramebufferRenderbufferEXT(gl::FRAMEBUFFER_EXT, attachment, gl::RENDERBUFFER_EXT, buffer);
}

RenderTarget::RenderTarget(const RenderTargetDesc &d)
: Graphics::RenderTarget(d)
, m_active(false)
{
	gl::GenFramebuffersEXT(1, &m_fbo);
}

RenderTarget::~RenderTarget()
{
	gl::DeleteFramebuffersEXT(1, &m_fbo);
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
	if (t) texId = static_cast<GL2Texture*>(t)->GetTexture();
	gl::FramebufferTexture2DEXT(gl::FRAMEBUFFER_EXT, gl::COLOR_ATTACHMENT0_EXT, gl::TEXTURE_CUBE_MAP_POSITIVE_X + face, texId, 0);
	m_colorTexture.Reset(t);
	if (!bound) Unbind();
}

void RenderTarget::SetColorTexture(Texture* t)
{
	const bool bound = m_active;
	if (!bound) Bind();
	//texture format should match the intended fbo format (aka. the one attached first)
	GLuint texId = 0;
	if (t) texId = static_cast<GL2Texture*>(t)->GetTexture();
	gl::FramebufferTexture2DEXT(gl::FRAMEBUFFER_EXT, gl::COLOR_ATTACHMENT0_EXT, gl::TEXTURE_2D, texId, 0);
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
	if (t) texId = static_cast<GL2Texture*>(t)->GetTexture();
	gl::FramebufferTexture2DEXT(gl::FRAMEBUFFER_EXT, gl::DEPTH_ATTACHMENT_EXT, gl::TEXTURE_2D, texId, 0);
	m_depthTexture.Reset(t);
	if (!bound) Unbind();
}

void RenderTarget::Bind()
{
	assert(!m_active);
	gl::BindFramebufferEXT(gl::FRAMEBUFFER_EXT, m_fbo);
	m_active = true;
}

void RenderTarget::Unbind()
{
	assert(m_active);
	gl::BindFramebufferEXT(gl::FRAMEBUFFER_EXT, 0);
	m_active = false;
}

bool RenderTarget::CheckStatus()
{
	return gl::CheckFramebufferStatusEXT(gl::FRAMEBUFFER_EXT) == gl::FRAMEBUFFER_COMPLETE_EXT;
}

void RenderTarget::CreateDepthRenderbuffer()
{
	assert(!GetDesc().allowDepthTexture);
	assert(m_active);
	m_depthRenderBuffer.Reset(new RenderBuffer());
	m_depthRenderBuffer->Bind();
	gl::RenderbufferStorageEXT(gl::RENDERBUFFER_EXT, gl::DEPTH_COMPONENT24, GetDesc().width, GetDesc().height);
	m_depthRenderBuffer->Attach(gl::DEPTH_ATTACHMENT_EXT);
	m_depthRenderBuffer->Unbind();
}

} }
