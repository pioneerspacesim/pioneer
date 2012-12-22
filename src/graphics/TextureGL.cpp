// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TextureGL.h"
#include <cassert>
#include "utils.h"
//warning C4715: 'Graphics::GLImageTypeForTextureFormat' : not all control paths return a value
namespace Graphics {

inline GLint GLCompressedTextureFormat(TextureFormat format) {
	switch (format) {
		case TEXTURE_RGBA: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		case TEXTURE_RGB:  return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		default: assert(0); return 0;
	}
}

inline GLint GLTextureFormat(TextureFormat format) {
	switch (format) {
		case TEXTURE_RGBA: return GL_RGBA;
		case TEXTURE_RGB:  return GL_RGB;
		default: assert(0); return 0;
	}
}

inline GLint GLImageFormat(ImageFormat format) {
	switch (format) {
		case IMAGE_RGBA: return GL_RGBA;
		case IMAGE_RGB:  return GL_RGB;
		default: assert(0); return 0;
	}
}

inline GLint GLImageType(ImageType type) {
	switch (type) {
		case IMAGE_UNSIGNED_BYTE: return GL_UNSIGNED_BYTE;
		default: assert(0); return 0;
	}
}

inline GLint GLImageFormatForTextureFormat(TextureFormat format) {
	switch (format) {
		case TEXTURE_RGBA: return GL_RGBA;
		case TEXTURE_RGB:  return GL_RGB;
		default: assert(0); return 0;
	}
}

inline GLint GLImageTypeForTextureFormat(TextureFormat format) {
	switch (format) {
		case TEXTURE_RGBA: return GL_UNSIGNED_BYTE;
		case TEXTURE_RGB:  return GL_UNSIGNED_BYTE;
		default: assert(0); return 0;
	}
}

TextureGL::TextureGL(const TextureDescriptor &descriptor, const bool useCompressed) :
	Texture(descriptor), m_target(GL_TEXTURE_2D) // XXX don't force target
{
	glGenTextures(1, &m_texture);
	glBindTexture(m_target, m_texture);

	glEnable(m_target);

	// useCompressed is the global scope flag whereas descriptor.allowCompression is the local texture mode flag
	// either both or neither might be true however only compress the texture when both are true.
	const bool compressTexture = useCompressed && descriptor.allowCompression;

	switch (m_target) {
		case GL_TEXTURE_2D:
			if (descriptor.generateMipmaps)
				glTexParameteri(m_target, GL_GENERATE_MIPMAP, GL_TRUE);

			glTexImage2D(
				m_target, 0, compressTexture ? GLCompressedTextureFormat(descriptor.format) : GLTextureFormat(descriptor.format),
				descriptor.dataSize.x, descriptor.dataSize.y, 0,
				GLImageFormatForTextureFormat(descriptor.format),
				GLImageTypeForTextureFormat(descriptor.format), 0);
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

void TextureGL::SetSampleMode(TextureSampleMode mode)
{
	GLenum magFilter, minFilter;
	const bool mipmaps = GetDescriptor().generateMipmaps;
	switch (mode) {
		case LINEAR_CLAMP:
			magFilter = GL_LINEAR;
			minFilter = mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
			break;

		case NEAREST_CLAMP:
			magFilter = GL_NEAREST;
			minFilter = mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
			break;

		case LINEAR_REPEAT:
			magFilter = GL_LINEAR;
			minFilter = mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
			break;

		case NEAREST_REPEAT:
			magFilter = GL_NEAREST;
			minFilter =mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
			break;

		default:
			assert(0);
	}
	glBindTexture(m_target, m_texture);
	glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, minFilter);
	glBindTexture(m_target, 0);
}

}
