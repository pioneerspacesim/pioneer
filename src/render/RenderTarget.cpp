#include "RenderTarget.h"
#include <stdexcept>

namespace Render {

RenderTarget::RenderTarget() :
	Texture(),
	m_fbo(0)
{
	//RT can be initialized later
}

RenderTarget::RenderTarget(int w, int h, GLint format,
	GLint internalFormat, GLenum type)
{
	m_w = w;
	m_h = h;
	glGenFramebuffers(1, &m_fbo);
	glGenTextures(1, &m_texture);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0,
		format, type, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, m_texture, 0);

	CheckCompleteness();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RenderTarget::~RenderTarget()
{
	glDeleteFramebuffers(1, &m_fbo);
}

void RenderTarget::CheckCompleteness() const
{
	const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Incomplete FBO.");
}

void RenderTarget::BeginRTT()
{
	//save current viewport and bind fbo
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	//glPushAttrib(GL_VIEWPORT_BIT);
	//glViewport(0, 0, m_w, m_h);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClearColor(0.f, 0.f, 0.f, 1.f);
}

void RenderTarget::EndRTT()
{
	//restore viewport and unbind fbo
	//glPopAttrib();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}
