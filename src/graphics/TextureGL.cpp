// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TextureGL.h"
#include <cassert>
#include "utils.h"

static const unsigned int MIN_COMPRESSED_TEXTURE_DIMENSION = 16;

namespace Graphics {

inline GLint GLInternalFormat(TextureFormat format) {
	switch (format) {
		case TEXTURE_RGB_888: return GL_RGB;
		case TEXTURE_RGBA_8888: return GL_RGBA;
		case TEXTURE_LUMINANCE_ALPHA_88: return GL_LUMINANCE_ALPHA;
		case TEXTURE_INTENSITY_8:  return GL_INTENSITY;
		case TEXTURE_DXT5: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		case TEXTURE_DXT1:  return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		default: assert(0); return 0;
	}
}

//for on the fly compression of textures
//luminance/intensity is used for fonts, so we prefer not to compress them
inline GLint GLCompressedInternalFormat(TextureFormat format) {
	switch (format) {
		case TEXTURE_RGBA_8888: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		case TEXTURE_RGB_888:  return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		case TEXTURE_LUMINANCE_ALPHA_88: return GL_LUMINANCE_ALPHA;
		case TEXTURE_INTENSITY_8:  return GL_INTENSITY;
		case TEXTURE_DXT5: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		case TEXTURE_DXT1:  return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		default: assert(0); return 0;
	}
}

inline GLint GLImageFormat(TextureFormat format) {
	switch (format) {
		case TEXTURE_RGBA_8888: return GL_RGBA;
		case TEXTURE_RGB_888:  return GL_RGB;
		case TEXTURE_LUMINANCE_ALPHA_88: return GL_LUMINANCE_ALPHA;
		case TEXTURE_INTENSITY_8:  return GL_LUMINANCE;
		case TEXTURE_DXT5: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		case TEXTURE_DXT1:  return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		default: assert(0); return 0;
	}
}

inline GLint GLImageType(TextureFormat format) {
	return GL_UNSIGNED_BYTE;
}

inline int GetMinSize(TextureFormat flag) {
	switch(flag) {
	case TEXTURE_DXT1: return 8;
	case TEXTURE_DXT5: return 16;
	default: return 1;
	}
}

inline bool IsCompressed(TextureFormat format) {
	return (format == TEXTURE_DXT1 || format == TEXTURE_DXT5);
}

TextureGL::TextureGL(const TextureDescriptor &descriptor, const bool useCompressed) :
	Texture(descriptor), m_target(GL_TEXTURE_2D) // XXX don't force target
{
	glGenTextures(1, &m_texture);
	glBindTexture(m_target, m_texture);

	glEnable(m_target); //XXX legacy only

	// useCompressed is the global scope flag whereas descriptor.allowCompression is the local texture mode flag
	// either both or neither might be true however only compress the texture when both are true.
	const bool compressTexture = useCompressed && descriptor.allowCompression;

	switch (m_target) {
		case GL_TEXTURE_2D:
			if (!IsCompressed(descriptor.format)) {
				if (descriptor.generateMipmaps)
					glTexParameteri(m_target, GL_GENERATE_MIPMAP, GL_TRUE);
				else
					glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, 0);

				glTexImage2D(
					m_target, 0, compressTexture ? GLCompressedInternalFormat(descriptor.format) : GLInternalFormat(descriptor.format),
					descriptor.dataSize.x, descriptor.dataSize.y, 0,
					GLImageFormat(descriptor.format),
					GLImageType(descriptor.format), 0);
			} else {
				const GLint oglFormatMinSize = GetMinSize(descriptor.format);
				size_t Width = descriptor.dataSize.x;
				size_t Height = descriptor.dataSize.y;
				size_t bufSize = ((Width + 3) / 4) * ((Height + 3) / 4) * oglFormatMinSize;

				GLint maxMip = 0;
				for( unsigned int i=0; i < descriptor.numberOfMipMaps; ++i ) {
					maxMip = i;
					glCompressedTexImage2D(GL_TEXTURE_2D, i, GLInternalFormat(descriptor.format), Width, Height, 0, bufSize, 0);
					if( Width<=MIN_COMPRESSED_TEXTURE_DIMENSION || Height<=MIN_COMPRESSED_TEXTURE_DIMENSION ) {
						break;
					}
					bufSize /= 4;
					Width /= 2;
					Height /= 2;
				}
				glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, maxMip);
			}
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

	glDisable(m_target);  //XXX legacy only
}

TextureGL::~TextureGL()
{
	glDeleteTextures(1, &m_texture);
}

void TextureGL::Update(const void *data, const vector2f &dataSize, TextureFormat format, const unsigned int numMips)
{
	glEnable(m_target);  //XXX legacy only
	glBindTexture(m_target, m_texture);

	switch (m_target) {
		case GL_TEXTURE_2D:
			if (!IsCompressed(format)) {
				glTexSubImage2D(m_target, 0, 0, 0, dataSize.x, dataSize.y, GLImageFormat(format), GLImageType(format), data);
			} else {
				const GLint oglInternalFormat = GLImageFormat(format);
				size_t Offset = 0;
				size_t Width = dataSize.x;
				size_t Height = dataSize.y;
				size_t bufSize = ((Width + 3) / 4) * ((Height + 3) / 4) * GetMinSize(format);

				const unsigned char *pData = static_cast<const unsigned char*>(data);
				for( unsigned int i = 0; i < numMips; ++i ) {
					glCompressedTexSubImage2D(m_target, i, 0, 0, Width, Height, oglInternalFormat, bufSize, &pData[Offset]);
					if( Width<=MIN_COMPRESSED_TEXTURE_DIMENSION || Height<=MIN_COMPRESSED_TEXTURE_DIMENSION ) {
						break;
					}
					Offset += bufSize;
					bufSize /= 4;
					Width /= 2;
					Height /= 2;
				}
			}
			break;

		default:
			assert(0);
	}

	glBindTexture(m_target, 0);
	glDisable(m_target);  //XXX legacy only
}

void TextureGL::Bind()
{
	glEnable(m_target);  //XXX legacy only
	glBindTexture(m_target, m_texture);
}

void TextureGL::Unbind()
{
	glBindTexture(m_target, 0);
	glDisable(m_target);  //XXX legacy only
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
