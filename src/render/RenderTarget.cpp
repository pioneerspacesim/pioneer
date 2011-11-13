#include "RenderTarget.h"
#include <stdexcept>
#include <sstream>

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
	glGenFramebuffersEXT(1, &m_fbo);
	glGenTextures(1, &m_texture);
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, m_fbo);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0,
		format, type, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
		GL_TEXTURE_2D, m_texture, 0);

	CheckCompleteness();

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

RenderTarget::~RenderTarget()
{
	glDeleteFramebuffersEXT(1, &m_fbo);
}

void RenderTarget::CheckCompleteness() const
{
	const GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		std::ostringstream ss;
		ss << "FBO error, ";
		switch (status) {
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			ss << "Unsupported formats";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			ss << "Incomplete attachment";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			ss << "Missing attachment";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			ss << "Incomplete dimensions";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			ss << "Incomplete formats";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			ss << "Incomplete draw buffer";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			ss << "Incomplete read buffer";
			break;
		default:
			ss << "Unknown status " << status;
			break;
		}
		throw std::runtime_error(ss.str());
	}
}

void RenderTarget::BeginRTT()
{
	//save current viewport and bind fbo
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, m_w, m_h);
}

void RenderTarget::EndRTT()
{
	//restore viewport and unbind fbo
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

}
