#include "RenderTarget.h"
#include <stdexcept>
#include <sstream>

namespace Graphics {

const char *RenderTarget::fbo_incomplete::what() const throw() {
	switch (m_errcode) {
	case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
		return "Unsupported formats";
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
		return "Incomplete attachment";
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
		return "Missing attachment";
	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
		return "Incomplete dimensions";
	case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
		return "Incomplete formats";
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
		return "Incomplete draw buffer";
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
		return "Incomplete read buffer";
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT:
		return "Number of samples does not match for all buffers";
	default: return "Unknown status";
	}
}

RenderTarget::RenderTarget(unsigned int w, unsigned int h, GLenum target, const Texture::Format &format, bool wantMipmaps) :
	Texture(target, format, CLAMP, LINEAR, wantMipmaps),
    m_fbo(0)
{
	glGenFramebuffersEXT(1, &m_fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

	CreateFromArray(0, w, h);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GetTarget(), GetGLTexture(), 0);

	CheckCompleteness();

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

RenderTarget::~RenderTarget()
{
	if (m_fbo)
		glDeleteFramebuffersEXT(1, &m_fbo);
}

void RenderTarget::CheckCompleteness() const
{
	const GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		throw fbo_incomplete(status);
	}
}

void RenderTarget::BeginRTT()
{
	//save current viewport and bind fbo
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, GetWidth(), GetHeight());
}

void RenderTarget::EndRTT()
{
	//restore viewport and unbind fbo
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

}
