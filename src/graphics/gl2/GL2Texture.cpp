// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GL2Texture.h"
#include <cassert>
#include "utils.h"

using namespace gl21;

static const unsigned int MIN_COMPRESSED_TEXTURE_DIMENSION = 16;

namespace Graphics {
namespace GL2 {

inline GLint GLInternalFormat(TextureFormat format) {
	switch (format) {
		case TEXTURE_RGB_888: return gl::RGB;
		case TEXTURE_RGBA_8888: return gl::RGBA;
		case TEXTURE_LUMINANCE_ALPHA_88: return gl::LUMINANCE_ALPHA;
		case TEXTURE_INTENSITY_8:  return gl::INTENSITY;
		case TEXTURE_DXT5: return gl::COMPRESSED_RGBA_S3TC_DXT5_EXT;
		case TEXTURE_DXT1:  return gl::COMPRESSED_RGB_S3TC_DXT1_EXT;
		case TEXTURE_DEPTH: return gl::DEPTH_COMPONENT;
		default: assert(0); return 0;
	}
}

//for on the fly compression of textures
//luminance/intensity is used for fonts, so we prefer not to compress them
inline GLint GLCompressedInternalFormat(TextureFormat format) {
	switch (format) {
		case TEXTURE_RGBA_8888: return gl::COMPRESSED_RGBA_S3TC_DXT5_EXT;
		case TEXTURE_RGB_888:  return gl::COMPRESSED_RGB_S3TC_DXT1_EXT;
		case TEXTURE_LUMINANCE_ALPHA_88: return gl::LUMINANCE_ALPHA;
		case TEXTURE_INTENSITY_8:  return gl::INTENSITY;
		case TEXTURE_DXT5: return gl::COMPRESSED_RGBA_S3TC_DXT5_EXT;
		case TEXTURE_DXT1:  return gl::COMPRESSED_RGB_S3TC_DXT1_EXT;
		default: assert(0); return 0;
	}
}

inline GLint GLImageFormat(TextureFormat format) {
	switch (format) {
		case TEXTURE_RGBA_8888: return gl::RGBA;
		case TEXTURE_RGB_888:  return gl::RGB;
		case TEXTURE_LUMINANCE_ALPHA_88: return gl::LUMINANCE_ALPHA;
		case TEXTURE_INTENSITY_8:  return gl::LUMINANCE;
		case TEXTURE_DXT5: return gl::COMPRESSED_RGBA_S3TC_DXT5_EXT;
		case TEXTURE_DXT1:  return gl::COMPRESSED_RGB_S3TC_DXT1_EXT;
		case TEXTURE_DEPTH: return gl::DEPTH_COMPONENT;
		default: assert(0); return 0;
	}
}

inline GLint GLTextureType(TextureType type) {
	switch (type) {
		case TEXTURE_2D: return gl::TEXTURE_2D;
		case TEXTURE_CUBE_MAP: return gl::TEXTURE_CUBE_MAP;
		default: assert(0); return 0;
	}
}

inline GLint GLImageType(TextureFormat format) {
	return gl::UNSIGNED_BYTE;
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

GL2Texture::GL2Texture(const TextureDescriptor &descriptor, const bool useCompressed) :
	Texture(descriptor)
{
	m_target = GLTextureType(descriptor.type);

	gl::GenTextures(1, &m_texture);
	gl::BindTexture(m_target, m_texture);


	// useCompressed is the global scope flag whereas descriptor.allowCompression is the local texture mode flag
	// either both or neither might be true however only compress the texture when both are true.
	const bool compressTexture = useCompressed && descriptor.allowCompression;

	switch (m_target) {
		case gl::TEXTURE_2D:
			if (!IsCompressed(descriptor.format)) {
				if (descriptor.generateMipmaps)
					gl::TexParameteri(m_target, gl::GENERATE_MIPMAP, gl::TRUE_);
				else
					gl::TexParameteri(m_target, gl::TEXTURE_MAX_LEVEL, 0);

				gl::TexImage2D(
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
					gl::CompressedTexImage2D(gl::TEXTURE_2D, i, GLInternalFormat(descriptor.format), Width, Height, 0, bufSize, 0);
					if( Width<=MIN_COMPRESSED_TEXTURE_DIMENSION || Height<=MIN_COMPRESSED_TEXTURE_DIMENSION ) {
						break;
					}
					bufSize /= 4;
					Width /= 2;
					Height /= 2;
				}
				gl::TexParameteri(m_target, gl::TEXTURE_MAX_LEVEL, maxMip);
			}
			break;

		case gl::TEXTURE_CUBE_MAP:
			if(!IsCompressed(descriptor.format)) {
				if(descriptor.generateMipmaps)
					gl::TexParameteri(m_target, gl::GENERATE_MIPMAP, gl::TRUE_);
				else
					gl::TexParameteri(m_target, gl::TEXTURE_MAX_LEVEL, 0);

				gl::TexImage2D(
					gl::TEXTURE_CUBE_MAP_POSITIVE_X, 0, compressTexture ? GLCompressedInternalFormat(descriptor.format) : GLInternalFormat(descriptor.format),
					descriptor.dataSize.x, descriptor.dataSize.y, 0,
					GLImageFormat(descriptor.format),
					GLImageType(descriptor.format), 0);
				gl::TexImage2D(
					gl::TEXTURE_CUBE_MAP_NEGATIVE_X, 0, compressTexture ? GLCompressedInternalFormat(descriptor.format) : GLInternalFormat(descriptor.format),
					descriptor.dataSize.x, descriptor.dataSize.y, 0,
					GLImageFormat(descriptor.format),
					GLImageType(descriptor.format), 0);
				gl::TexImage2D(
					gl::TEXTURE_CUBE_MAP_POSITIVE_Y, 0, compressTexture ? GLCompressedInternalFormat(descriptor.format) : GLInternalFormat(descriptor.format),
					descriptor.dataSize.x, descriptor.dataSize.y, 0,
					GLImageFormat(descriptor.format),
					GLImageType(descriptor.format), 0);
				gl::TexImage2D(
					gl::TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, compressTexture ? GLCompressedInternalFormat(descriptor.format) : GLInternalFormat(descriptor.format),
					descriptor.dataSize.x, descriptor.dataSize.y, 0,
					GLImageFormat(descriptor.format),
					GLImageType(descriptor.format), 0);
				gl::TexImage2D(
					gl::TEXTURE_CUBE_MAP_POSITIVE_Z, 0, compressTexture ? GLCompressedInternalFormat(descriptor.format) : GLInternalFormat(descriptor.format),
					descriptor.dataSize.x, descriptor.dataSize.y, 0,
					GLImageFormat(descriptor.format),
					GLImageType(descriptor.format), 0);
				gl::TexImage2D(
					gl::TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, compressTexture ? GLCompressedInternalFormat(descriptor.format) : GLInternalFormat(descriptor.format),
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
					gl::CompressedTexImage2D(gl::TEXTURE_CUBE_MAP_POSITIVE_X, i, GLInternalFormat(descriptor.format), Width, Height, 0, bufSize, 0);
					gl::CompressedTexImage2D(gl::TEXTURE_CUBE_MAP_NEGATIVE_X, i, GLInternalFormat(descriptor.format), Width, Height, 0, bufSize, 0);
					gl::CompressedTexImage2D(gl::TEXTURE_CUBE_MAP_POSITIVE_Y, i, GLInternalFormat(descriptor.format), Width, Height, 0, bufSize, 0);
					gl::CompressedTexImage2D(gl::TEXTURE_CUBE_MAP_NEGATIVE_Y, i, GLInternalFormat(descriptor.format), Width, Height, 0, bufSize, 0);
					gl::CompressedTexImage2D(gl::TEXTURE_CUBE_MAP_POSITIVE_Z, i, GLInternalFormat(descriptor.format), Width, Height, 0, bufSize, 0);
					gl::CompressedTexImage2D(gl::TEXTURE_CUBE_MAP_NEGATIVE_Z, i, GLInternalFormat(descriptor.format), Width, Height, 0, bufSize, 0);
					if( Width<=MIN_COMPRESSED_TEXTURE_DIMENSION || Height<=MIN_COMPRESSED_TEXTURE_DIMENSION ) {
						break;
					}
					bufSize /= 4;
					Width /= 2;
					Height /= 2;
				}
				gl::TexParameteri(m_target, gl::TEXTURE_MAX_LEVEL, maxMip);
			}
			break;

		default:
			assert(0);
	}

	GLenum magFilter, minFilter, wrapS, wrapT;
	switch (descriptor.sampleMode) {
		default:	// safe default will fall through to LINEAR_CLAMP when run in release builds without assert
			assert(0);
		case LINEAR_CLAMP:
			magFilter = gl::LINEAR;
			minFilter = descriptor.generateMipmaps ? gl::LINEAR_MIPMAP_LINEAR : gl::LINEAR;
			wrapS = wrapT = gl::CLAMP_TO_EDGE;
			break;

		case NEAREST_CLAMP:
			magFilter = gl::NEAREST;
			minFilter = descriptor.generateMipmaps ? gl::NEAREST_MIPMAP_NEAREST : gl::NEAREST;
			wrapS = wrapT = gl::CLAMP_TO_EDGE;
			break;

		case LINEAR_REPEAT:
			magFilter = gl::LINEAR;
			minFilter = descriptor.generateMipmaps ? gl::LINEAR_MIPMAP_LINEAR : gl::LINEAR;
			wrapS = wrapT = gl::REPEAT;
			break;

		case NEAREST_REPEAT:
			magFilter = gl::NEAREST;
			minFilter = descriptor.generateMipmaps ? gl::NEAREST_MIPMAP_NEAREST : gl::NEAREST;
			wrapS = wrapT = gl::REPEAT;
			break;
	}

	gl::TexParameteri(m_target, gl::TEXTURE_WRAP_S, wrapS);
	gl::TexParameteri(m_target, gl::TEXTURE_WRAP_T, wrapS);
	gl::TexParameteri(m_target, gl::TEXTURE_MAG_FILTER, magFilter);
	gl::TexParameteri(m_target, gl::TEXTURE_MIN_FILTER, minFilter);

}

GL2Texture::~GL2Texture()
{
	gl::DeleteTextures(1, &m_texture);
}

void GL2Texture::Update(const void *data, const vector2f &pos, const vector2f &dataSize, TextureFormat format, const unsigned int numMips)
{
	assert(m_target == gl::TEXTURE_2D);
	gl::BindTexture(m_target, m_texture);

	switch (m_target) {
		case gl::TEXTURE_2D:
			if (!IsCompressed(format)) {
				gl::TexSubImage2D(m_target, 0, pos.x, pos.y, dataSize.x, dataSize.y, GLImageFormat(format), GLImageType(format), data);
			} else {
				const GLint oglInternalFormat = GLImageFormat(format);
				size_t Offset = 0;
				size_t Width = dataSize.x;
				size_t Height = dataSize.y;
				size_t bufSize = ((Width + 3) / 4) * ((Height + 3) / 4) * GetMinSize(format);

				const unsigned char *pData = static_cast<const unsigned char*>(data);
				for( unsigned int i = 0; i < numMips; ++i ) {
					gl::CompressedTexSubImage2D(m_target, i, pos.x, pos.y, Width, Height, oglInternalFormat, bufSize, &pData[Offset]);
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

	gl::BindTexture(m_target, 0);
}

void GL2Texture::Update(const TextureCubeData &data, const vector2f &dataSize, TextureFormat format, const unsigned int numMips)
{
	assert(m_target == gl::TEXTURE_CUBE_MAP);

	gl::BindTexture(m_target, m_texture);

	switch (m_target) {
		case gl::TEXTURE_CUBE_MAP:
			if (!IsCompressed(format)) {
				gl::TexSubImage2D(gl::TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0, dataSize.x, dataSize.y, GLImageFormat(format), GLImageType(format), data.posX);
				gl::TexSubImage2D(gl::TEXTURE_CUBE_MAP_NEGATIVE_X, 0, 0, 0, dataSize.x, dataSize.y, GLImageFormat(format), GLImageType(format), data.negX);
				gl::TexSubImage2D(gl::TEXTURE_CUBE_MAP_POSITIVE_Y, 0, 0, 0, dataSize.x, dataSize.y, GLImageFormat(format), GLImageType(format), data.posY);
				gl::TexSubImage2D(gl::TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, 0, 0, dataSize.x, dataSize.y, GLImageFormat(format), GLImageType(format), data.negY);
				gl::TexSubImage2D(gl::TEXTURE_CUBE_MAP_POSITIVE_Z, 0, 0, 0, dataSize.x, dataSize.y, GLImageFormat(format), GLImageType(format), data.posZ);
				gl::TexSubImage2D(gl::TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, 0, 0, dataSize.x, dataSize.y, GLImageFormat(format), GLImageType(format), data.negZ);
			} else {
				const GLint oglInternalFormat = GLImageFormat(format);
				size_t Offset = 0;
				size_t Width = dataSize.x;
				size_t Height = dataSize.y;
				size_t bufSize = ((Width + 3) / 4) * ((Height + 3) / 4) * GetMinSize(format);

				const unsigned char *pData_px = static_cast<const unsigned char*>(data.posX);
				const unsigned char *pData_nx = static_cast<const unsigned char*>(data.negX);
				const unsigned char *pData_py = static_cast<const unsigned char*>(data.posY);
				const unsigned char *pData_ny = static_cast<const unsigned char*>(data.negY);
				const unsigned char *pData_pz = static_cast<const unsigned char*>(data.posZ);
				const unsigned char *pData_nz = static_cast<const unsigned char*>(data.negZ);
				for( unsigned int i = 0; i < numMips; ++i ) {
					gl::CompressedTexSubImage2D(gl::TEXTURE_CUBE_MAP_POSITIVE_X, i, 0, 0, Width, Height, oglInternalFormat, bufSize, &pData_px[Offset]);
					gl::CompressedTexSubImage2D(gl::TEXTURE_CUBE_MAP_NEGATIVE_X, i, 0, 0, Width, Height, oglInternalFormat, bufSize, &pData_nx[Offset]);
					gl::CompressedTexSubImage2D(gl::TEXTURE_CUBE_MAP_POSITIVE_Y, i, 0, 0, Width, Height, oglInternalFormat, bufSize, &pData_py[Offset]);
					gl::CompressedTexSubImage2D(gl::TEXTURE_CUBE_MAP_NEGATIVE_Y, i, 0, 0, Width, Height, oglInternalFormat, bufSize, &pData_ny[Offset]);
					gl::CompressedTexSubImage2D(gl::TEXTURE_CUBE_MAP_POSITIVE_Z, i, 0, 0, Width, Height, oglInternalFormat, bufSize, &pData_pz[Offset]);
					gl::CompressedTexSubImage2D(gl::TEXTURE_CUBE_MAP_NEGATIVE_Z, i, 0, 0, Width, Height, oglInternalFormat, bufSize, &pData_nz[Offset]);
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

	gl::BindTexture(m_target, 0);
}

void GL2Texture::Bind()
{
	gl::BindTexture(m_target, m_texture);
}

void GL2Texture::Unbind()
{
	gl::BindTexture(m_target, 0);
}

void GL2Texture::SetSampleMode(TextureSampleMode mode)
{
	GLenum magFilter, minFilter;
	const bool mipmaps = GetDescriptor().generateMipmaps;
	switch (mode) {
		default:	// safe default will fall through to LINEAR_CLAMP when run in release builds without assert
			assert(0);
		case LINEAR_CLAMP:
			magFilter = gl::LINEAR;
			minFilter = mipmaps ? gl::LINEAR_MIPMAP_LINEAR : gl::LINEAR;
			break;

		case NEAREST_CLAMP:
			magFilter = gl::NEAREST;
			minFilter = mipmaps ? gl::NEAREST_MIPMAP_NEAREST : gl::NEAREST;
			break;

		case LINEAR_REPEAT:
			magFilter = gl::LINEAR;
			minFilter = mipmaps ? gl::LINEAR_MIPMAP_LINEAR : gl::LINEAR;
			break;

		case NEAREST_REPEAT:
			magFilter = gl::NEAREST;
			minFilter =mipmaps ? gl::NEAREST_MIPMAP_NEAREST : gl::NEAREST;
			break;
	}
	gl::BindTexture(m_target, m_texture);
	gl::TexParameteri(m_target, gl::TEXTURE_MAG_FILTER, magFilter);
	gl::TexParameteri(m_target, gl::TEXTURE_MIN_FILTER, minFilter);
	gl::BindTexture(m_target, 0);
}

void GL2Texture::BuildMipmaps()
{
	const TextureDescriptor& descriptor = GetDescriptor();
	const bool mipmaps = descriptor.generateMipmaps;
	if (mipmaps)
	{
		gl::BindTexture(m_target, m_texture);
		gl::TexParameteri(m_target, gl::GENERATE_MIPMAP, gl::TRUE_);
		gl::BindTexture(m_target, 0);
	}
}

}
}
