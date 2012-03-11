#include "TextureGL.h"
#include <cassert>
#include "utils.h"

namespace Graphics {

inline GLint GLTextureFormat(TextureFormat format) {
	switch (format) {
		case TEXTURE_RGBA: return GL_RGBA;
		case TEXTURE_RGB:  return GL_RGB;
		default: assert(0);
	}
}

inline GLint GLImageFormat(ImageFormat format) {
	switch (format) {
		case IMAGE_RGBA: return GL_RGBA;
		case IMAGE_RGB:  return GL_RGB;
		default: assert(0);
	}
}

inline GLint GLImageType(ImageType type) {
	switch (type) {
		case IMAGE_UNSIGNED_BYTE: return GL_UNSIGNED_BYTE;
		default: assert(0);
	}
}

inline GLint GLImageFormatForTextureFormat(TextureFormat format) {
	switch (format) {
		case TEXTURE_RGBA: return GL_RGBA;
		case TEXTURE_RGB:  return GL_RGB;
		default: assert(0);
	}
}

inline GLint GLImageTypeForTextureFormat(TextureFormat format) {
	switch (format) {
		case TEXTURE_RGBA: return GL_UNSIGNED_BYTE;
		case TEXTURE_RGB:  return GL_UNSIGNED_BYTE;
		default: assert(0);
	}
}

TextureGL::TextureGL(const TextureDescriptor &descriptor) : Texture(descriptor), m_target(GL_TEXTURE_2D) // XXX don't force target
{
	glGenTextures(1, &m_texture);
	glBindTexture(m_target, m_texture);

	glEnable(m_target);

	switch (m_target) {
		case GL_TEXTURE_2D:
			if (descriptor.generateMipmaps)
				glTexParameteri(m_target, GL_GENERATE_MIPMAP, GL_TRUE);
			glTexImage2D(
				m_target, 0, GLTextureFormat(descriptor.format),
				descriptor.dataSize.x, descriptor.dataSize.y, 0,
				GLImageFormatForTextureFormat(descriptor.format), GLImageTypeForTextureFormat(descriptor.format), 0);
			break;

		default:
			assert(0);
	}

	GLenum magFilter, minFilter, wrapS, wrapT;
	switch (descriptor.sampleMode) {
		case LINEAR_CLAMP:
			magFilter = GL_LINEAR;
			minFilter = descriptor.generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
			wrapS = wrapT = GL_CLAMP_TO_EDGE;
			break;

		case NEAREST_CLAMP:
			magFilter = GL_NEAREST;
			minFilter = descriptor.generateMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
			wrapS = wrapT = GL_CLAMP_TO_EDGE;
			break;

		case LINEAR_REPEAT:
			magFilter = GL_LINEAR;
			minFilter = descriptor.generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
			wrapS = wrapT = GL_REPEAT;
			break;

		case NEAREST_REPEAT:
			magFilter = GL_NEAREST;
			minFilter = descriptor.generateMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
			wrapS = wrapT = GL_REPEAT;
			break;

		default:
			assert(0);
	}

	glTexParameteri(m_target, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(m_target, GL_TEXTURE_WRAP_T, wrapS);
	glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, minFilter);

	glDisable(m_target);
}

TextureGL::~TextureGL()
{
	glDeleteTextures(1, &m_texture);
}

void TextureGL::Update(const void *data, const vector2f &dataSize, ImageFormat format, ImageType type)
{
	glEnable(m_target);
	glBindTexture(m_target, m_texture);

	switch (m_target) {
		case GL_TEXTURE_2D:
			glTexSubImage2D(m_target, 0, 0, 0, dataSize.x, dataSize.y, GLImageFormat(format), GLImageType(type), data);
			break;

		default:
			assert(0);
	}

	glBindTexture(m_target, 0);
	glDisable(m_target);
}

void TextureGL::Bind()
{
	glEnable(m_target);
	glBindTexture(m_target, m_texture);
}

void TextureGL::Unbind()
{
	glBindTexture(m_target, 0);
	glDisable(m_target);
}

}
