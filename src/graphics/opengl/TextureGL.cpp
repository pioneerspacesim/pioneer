// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TextureGL.h"
#include "RendererGL.h"
#include "graphics/Renderer.h"
#include "utils.h"
#include <cassert>

namespace Graphics {
	namespace OGL {

		inline GLint GLInternalFormat(TextureFormat format)
		{
			switch (format) {
			case TEXTURE_RGB_888: return GL_RGB;
			case TEXTURE_RGBA_8888: return GL_RGBA;
			case TEXTURE_RG_88: return GL_RG;
			case TEXTURE_R8: return GL_RED;
			case TEXTURE_DXT5: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			case TEXTURE_DXT1: return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			case TEXTURE_DEPTH: return GL_DEPTH_COMPONENT32F;
			default: assert(0); return 0;
			}
		}

		//for on the fly compression of textures
		//luminance/intensity is used for fonts, so we prefer not to compress them
		inline GLint GLCompressedInternalFormat(TextureFormat format)
		{
			switch (format) {
			case TEXTURE_RGBA_8888: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			case TEXTURE_RGB_888: return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			case TEXTURE_RG_88: return GL_RG;
			case TEXTURE_R8: return GL_RED;
			case TEXTURE_DXT5: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			case TEXTURE_DXT1: return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			default: assert(0); return 0;
			}
		}

		inline GLint GLImageFormat(TextureFormat format)
		{
			switch (format) {
			case TEXTURE_RGBA_8888: return GL_RGBA;
			case TEXTURE_RGB_888: return GL_RGB;
			case TEXTURE_RG_88: return GL_RG;
			case TEXTURE_R8: return GL_RED;
			case TEXTURE_DXT5: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			case TEXTURE_DXT1: return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			case TEXTURE_DEPTH: return GL_DEPTH_COMPONENT;
			default: assert(0); return 0;
			}
		}

		inline GLint GLTextureType(TextureType type)
		{
			switch (type) {
			case TEXTURE_2D: return GL_TEXTURE_2D;
			case TEXTURE_CUBE_MAP: return GL_TEXTURE_CUBE_MAP;
			case TEXTURE_2D_ARRAY: return GL_TEXTURE_2D_ARRAY;
			default: assert(0); return 0;
			}
		}

		inline GLint GLImageType(TextureFormat format)
		{
			return GL_UNSIGNED_BYTE;
		}

		inline int GetMinSize(TextureFormat flag)
		{
			switch (flag) {
			case TEXTURE_DXT1: return 8;
			case TEXTURE_DXT5: return 16;
			default: return 1;
			}
		}

		inline bool IsCompressed(TextureFormat format)
		{
			return (format == TEXTURE_DXT1 || format == TEXTURE_DXT5);
		}

		TextureGL::TextureGL(const TextureDescriptor &descriptor, const bool useCompressed, const bool useAnisoFiltering, const Uint16 numSamples) :
			Texture(descriptor),
			m_allocSize(0),
			m_useAnisoFiltering(useAnisoFiltering && descriptor.useAnisotropicFiltering)
		{
			PROFILE_SCOPED()
			// this is kind of a hack, but it limits the amount of things that need to care about multisample textures.
			m_target = numSamples ? GL_TEXTURE_2D_MULTISAMPLE : GLTextureType(descriptor.type);

			glGenTextures(1, &m_texture);
			glBindTexture(m_target, m_texture);
			CHECKERRORS();

			// useCompressed is the global scope flag whereas descriptor.allowCompression is the local texture mode flag
			// either both or neither might be true however only compress the texture when both are true.
			const bool compressTexture = useCompressed && descriptor.allowCompression;

			switch (m_target) {
			// XXX(sturnclaw): multisample assumes an uncompressed, un-mipmapped 2d texture descriptor.
			case GL_TEXTURE_2D_MULTISAMPLE: {
				glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, 0);
				glTexImage2DMultisample(
					m_target, numSamples, GLInternalFormat(descriptor.format),
					descriptor.dataSize.x, descriptor.dataSize.y, true); // must use fixedsamplelocations when mixed with renderbuffer
				CHECKERRORS();
			} break;
			case GL_TEXTURE_2D:
				if (!IsCompressed(descriptor.format)) {
					if (!descriptor.generateMipmaps)
						glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, 0);
					CHECKERRORS();

					glTexImage2D(
						m_target, 0, compressTexture ? GLCompressedInternalFormat(descriptor.format) : GLInternalFormat(descriptor.format),
						descriptor.dataSize.x, descriptor.dataSize.y, 0,
						GLImageFormat(descriptor.format),
						GLImageType(descriptor.format), 0);
					CHECKERRORS();
				} else {
					size_t Width = descriptor.dataSize.x;
					size_t Height = descriptor.dataSize.y;

					GLint maxMip = 0;
					for (unsigned int i = 0; i < descriptor.numberOfMipMaps; ++i) {
						maxMip = i;
						size_t bufSize = ((Width + 3) / 4) * ((Height + 3) / 4) * GetMinSize(descriptor.format);
						glCompressedTexImage2D(GL_TEXTURE_2D, i, GLInternalFormat(descriptor.format), Width, Height, 0, bufSize, 0);
						if (!Width || !Height)
							break;

						Width = std::max<size_t>(Width / 2, 1ul);
						Height = std::max<size_t>(Height / 2, 1ul);
						m_allocSize += bufSize;
					}
					glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, maxMip);
					CHECKERRORS();
				}
				break;

			case GL_TEXTURE_CUBE_MAP:
				if (!IsCompressed(descriptor.format)) {
					if (!descriptor.generateMipmaps)
						glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, 0);
					CHECKERRORS();

					glTexImage2D(
						GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, compressTexture ? GLCompressedInternalFormat(descriptor.format) : GLInternalFormat(descriptor.format),
						descriptor.dataSize.x, descriptor.dataSize.y, 0,
						GLImageFormat(descriptor.format),
						GLImageType(descriptor.format), 0);
					glTexImage2D(
						GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, compressTexture ? GLCompressedInternalFormat(descriptor.format) : GLInternalFormat(descriptor.format),
						descriptor.dataSize.x, descriptor.dataSize.y, 0,
						GLImageFormat(descriptor.format),
						GLImageType(descriptor.format), 0);
					glTexImage2D(
						GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, compressTexture ? GLCompressedInternalFormat(descriptor.format) : GLInternalFormat(descriptor.format),
						descriptor.dataSize.x, descriptor.dataSize.y, 0,
						GLImageFormat(descriptor.format),
						GLImageType(descriptor.format), 0);
					glTexImage2D(
						GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, compressTexture ? GLCompressedInternalFormat(descriptor.format) : GLInternalFormat(descriptor.format),
						descriptor.dataSize.x, descriptor.dataSize.y, 0,
						GLImageFormat(descriptor.format),
						GLImageType(descriptor.format), 0);
					glTexImage2D(
						GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, compressTexture ? GLCompressedInternalFormat(descriptor.format) : GLInternalFormat(descriptor.format),
						descriptor.dataSize.x, descriptor.dataSize.y, 0,
						GLImageFormat(descriptor.format),
						GLImageType(descriptor.format), 0);
					glTexImage2D(
						GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, compressTexture ? GLCompressedInternalFormat(descriptor.format) : GLInternalFormat(descriptor.format),
						descriptor.dataSize.x, descriptor.dataSize.y, 0,
						GLImageFormat(descriptor.format),
						GLImageType(descriptor.format), 0);
					CHECKERRORS();
				} else {
					size_t Width = descriptor.dataSize.x;
					size_t Height = descriptor.dataSize.y;

					GLint maxMip = 0;
					for (unsigned int i = 0; i < descriptor.numberOfMipMaps; ++i) {
						maxMip = i;
						size_t bufSize = ((Width + 3) / 4) * ((Height + 3) / 4) * GetMinSize(descriptor.format);
						glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, i, GLInternalFormat(descriptor.format), Width, Height, 0, bufSize, 0);
						glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, i, GLInternalFormat(descriptor.format), Width, Height, 0, bufSize, 0);
						glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, i, GLInternalFormat(descriptor.format), Width, Height, 0, bufSize, 0);
						glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, i, GLInternalFormat(descriptor.format), Width, Height, 0, bufSize, 0);
						glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, i, GLInternalFormat(descriptor.format), Width, Height, 0, bufSize, 0);
						glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, i, GLInternalFormat(descriptor.format), Width, Height, 0, bufSize, 0);
						Width = std::max<size_t>(Width / 2, 1ul);
						Height = std::max<size_t>(Height / 2, 1ul);
						m_allocSize += bufSize * 6;

						if (!Width || !Height)
							break;
					}
					glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, maxMip);
					CHECKERRORS();
				}
				break;

			case GL_TEXTURE_2D_ARRAY:
				if (!IsCompressed(descriptor.format)) {
					if (descriptor.generateMipmaps) {
						glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, 0);
					}
					RendererOGL::CheckErrors();

					glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, compressTexture ? GLCompressedInternalFormat(descriptor.format) : GLInternalFormat(descriptor.format),
						descriptor.dataSize.x, descriptor.dataSize.y, descriptor.dataSize.z, 0,
						GLImageFormat(descriptor.format),
						GLImageType(descriptor.format), nullptr);
					RendererOGL::CheckErrors();

					if (descriptor.generateMipmaps) {
						glGenerateMipmap(m_target);
					}
				} else {
					size_t Width = descriptor.dataSize.x;
					size_t Height = descriptor.dataSize.y;
					const size_t Layers = descriptor.dataSize.z;

					GLint maxMip = 0;
					for (unsigned int i = 0; i < descriptor.numberOfMipMaps; ++i) {
						maxMip = i;
						size_t bufSize = ((Width + 3) / 4) * ((Height + 3) / 4) * GetMinSize(descriptor.format);
						glCompressedTexImage3D(GL_TEXTURE_2D_ARRAY, i, GLInternalFormat(descriptor.format), Width, Height, Layers, 0, bufSize * Layers, nullptr);
						RendererOGL::CheckErrors();

						Width = std::max<size_t>(Width / 2, 1ul);
						Height = std::max<size_t>(Height / 2, 1ul);
						m_allocSize += bufSize * Layers;

						if (!Width || !Height)
							break;
					}
					glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, maxMip);
				}
				RendererOGL::CheckErrors();
				break;

			default:
				assert(0);
			}
			CHECKERRORS();

			GLenum magFilter, minFilter, wrapS, wrapT;
			switch (descriptor.sampleMode) {
			default: // safe default will fall through to LINEAR_CLAMP when run in release builds without assert
				assert(0);
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
			}

			// multisample textures don't support wrap or filter operations.
			if (!numSamples) {
				glTexParameteri(m_target, GL_TEXTURE_WRAP_S, wrapS);
				glTexParameteri(m_target, GL_TEXTURE_WRAP_T, wrapS);
				glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, magFilter);
				glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, minFilter);
			}

			// Anisotropic texture filtering
			if (m_useAnisoFiltering) {
				GLfloat maxAniso = 0.0f;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
				glTexParameterf(m_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
			}

			CHECKERRORS();
		}

		TextureGL::~TextureGL()
		{
			glDeleteTextures(1, &m_texture);
		}

		void TextureGL::Update(const void *data, const vector2f &pos, const vector3f &dataSize, TextureFormat format, const unsigned int numMips)
		{
			PROFILE_SCOPED()
			assert(m_target == GL_TEXTURE_2D);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(m_target, m_texture);

			switch (m_target) {
			case GL_TEXTURE_2D:
				if (!IsCompressed(format)) {
					glTexSubImage2D(m_target, 0, pos.x, pos.y, dataSize.x, dataSize.y, GLImageFormat(format), GLImageType(format), data);
				} else {
					const GLint oglInternalFormat = GLImageFormat(format);
					size_t Offset = 0;
					size_t Width = dataSize.x;
					size_t Height = dataSize.y;

					const unsigned char *pData = static_cast<const unsigned char *>(data);
					for (unsigned int i = 0; i < numMips; ++i) {
						size_t bufSize = ((Width + 3) / 4) * ((Height + 3) / 4) * GetMinSize(format);
						glCompressedTexSubImage2D(m_target, i, pos.x, pos.y, Width, Height, oglInternalFormat, bufSize, &pData[Offset]);

						Offset += bufSize;
						Width = std::max<size_t>(Width / 2, 1ul);
						Height = std::max<size_t>(Height / 2, 1ul);

						if (!Width || !Height)
							break;
					}
				}
				break;

			default:
				assert(0);
			}

			BuildMipmaps(numMips);

			glBindTexture(m_target, 0);
			CHECKERRORS();
		}

		void TextureGL::Update(const TextureCubeData &data, const vector3f &dataSize, TextureFormat format, const unsigned int numMips)
		{
			PROFILE_SCOPED()
			assert(m_target == GL_TEXTURE_CUBE_MAP);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(m_target, m_texture);

			switch (m_target) {
			case GL_TEXTURE_CUBE_MAP:
				if (!IsCompressed(format)) {
					glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0, dataSize.x, dataSize.y, GLImageFormat(format), GLImageType(format), data.posX);
					glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, 0, 0, dataSize.x, dataSize.y, GLImageFormat(format), GLImageType(format), data.negX);
					glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, 0, 0, dataSize.x, dataSize.y, GLImageFormat(format), GLImageType(format), data.posY);
					glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, 0, 0, dataSize.x, dataSize.y, GLImageFormat(format), GLImageType(format), data.negY);
					glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, 0, 0, dataSize.x, dataSize.y, GLImageFormat(format), GLImageType(format), data.posZ);
					glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, 0, 0, dataSize.x, dataSize.y, GLImageFormat(format), GLImageType(format), data.negZ);
				} else {
					const GLint oglInternalFormat = GLImageFormat(format);
					size_t Offset = 0;
					size_t Width = dataSize.x;
					size_t Height = dataSize.y;

					const unsigned char *pData_px = static_cast<const unsigned char *>(data.posX);
					const unsigned char *pData_nx = static_cast<const unsigned char *>(data.negX);
					const unsigned char *pData_py = static_cast<const unsigned char *>(data.posY);
					const unsigned char *pData_ny = static_cast<const unsigned char *>(data.negY);
					const unsigned char *pData_pz = static_cast<const unsigned char *>(data.posZ);
					const unsigned char *pData_nz = static_cast<const unsigned char *>(data.negZ);
					for (unsigned int i = 0; i < numMips; ++i) {
						size_t bufSize = ((Width + 3) / 4) * ((Height + 3) / 4) * GetMinSize(format);
						glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, i, 0, 0, Width, Height, oglInternalFormat, bufSize, &pData_px[Offset]);
						glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, i, 0, 0, Width, Height, oglInternalFormat, bufSize, &pData_nx[Offset]);
						glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, i, 0, 0, Width, Height, oglInternalFormat, bufSize, &pData_py[Offset]);
						glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, i, 0, 0, Width, Height, oglInternalFormat, bufSize, &pData_ny[Offset]);
						glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, i, 0, 0, Width, Height, oglInternalFormat, bufSize, &pData_pz[Offset]);
						glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, i, 0, 0, Width, Height, oglInternalFormat, bufSize, &pData_nz[Offset]);

						Offset += bufSize;
						Width = std::max<size_t>(Width / 2, 1ul);
						Height = std::max<size_t>(Height / 2, 1ul);
						if (!Width || !Height)
							break;
					}
				}
				break;

			default:
				assert(0);
			}

			BuildMipmaps(numMips);

			glBindTexture(m_target, 0);
			CHECKERRORS();
		}

		void TextureGL::Update(const vecDataPtr &data, const vector3f &dataSize, const TextureFormat format, const unsigned int numMips)
		{
			assert(m_target == GL_TEXTURE_2D_ARRAY);

			glBindTexture(m_target, m_texture);
			RendererOGL::CheckErrors();

			const size_t Layers = dataSize.z;
			assert(Layers == data.size());

			if (!IsCompressed(format)) {
				for (size_t i = 0; i < Layers; i++) {
					glTexSubImage3D(
						GL_TEXTURE_2D_ARRAY,   //	GLenum target,
						0,					   //	GLint level, // mip
						0,					   //	GLint xoffset,
						0,					   //	GLint yoffset,
						i,					   //	GLint zoffset,
						dataSize.x,			   //	GLsizei width,
						dataSize.y,			   //	GLsizei height,
						1,					   //	GLsizei depth,
						GLImageFormat(format), //	GLenum format,
						GLImageType(format),   //	GLenum type,
						data[i]);			   //	const GLvoid * data);
				}
				RendererOGL::CheckErrors();
			} else {
				const GLint oglInternalFormat = GLImageFormat(format);
				for (size_t ilayer = 0; ilayer < Layers; ilayer++) {
					size_t Offset = 0;
					size_t Width = dataSize.x;
					size_t Height = dataSize.y;

					const unsigned char *pData = static_cast<const unsigned char *>(data[ilayer]);
					for (unsigned int i = 0; i < numMips; ++i) {
						size_t bufSize = ((Width + 3) / 4) * ((Height + 3) / 4) * GetMinSize(format);
						glCompressedTexSubImage3D(
							m_target,		   //	GLenum  		target,
							i,				   //	GLint			level,
							0,				   //	GLint			xoffset,
							0,				   //	GLint			yoffset,
							ilayer,			   //	GLint			zoffset,
							Width,			   //	GLsizei  		width,
							Height,			   //	GLsizei  		height,
							1,				   //	GLsizei  		depth,
							oglInternalFormat, //	GLenum  		format,
							bufSize,		   //	GLsizei  		imageSize,
							&pData[Offset]);   //	const GLvoid *  data);
						Offset += bufSize;
						Width = std::max<size_t>(Width / 2, 1ul);
						Height = std::max<size_t>(Height / 2, 1ul);

						if (!Width || !Height)
							break;
					}
					RendererOGL::CheckErrors();
				}
			}

			// Generate mipmaps for all layers that we were not passed data for
			BuildMipmaps(numMips);

			RendererOGL::CheckErrors();
			glBindTexture(m_target, 0);
		}

		void TextureGL::Bind()
		{
			glBindTexture(m_target, m_texture);
		}

		void TextureGL::Unbind()
		{
			glBindTexture(m_target, 0);
		}

		void TextureGL::SetSampleMode(TextureSampleMode mode)
		{
			GLenum magFilter, minFilter;
			const bool mipmaps = GetDescriptor().generateMipmaps;
			switch (mode) {
			default: // safe default will fall through to LINEAR_CLAMP when run in release builds without assert
				assert(0);
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
				minFilter = mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
				break;
			}
			glBindTexture(m_target, m_texture);
			glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, magFilter);
			glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, minFilter);

			// Anisotropic texture filtering
			if (m_useAnisoFiltering) {
				GLfloat maxAniso = 0.0f;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
				glTexParameterf(m_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
			}

			glBindTexture(m_target, 0);
			CHECKERRORS();
		}

		void TextureGL::BuildMipmaps(const uint32_t validMips)
		{
			const TextureDescriptor &descriptor = GetDescriptor();
			const bool mipmaps = descriptor.generateMipmaps;
			// HACK: uncompressed textures come in with numberOfMipMaps == 0 but generateMipmaps == true
			// Solution: refactor TextureGL::TextureGL() to calculate the required number of mip levels
			// and store it in the texture's descriptor.
			if (mipmaps && (!descriptor.numberOfMipMaps || validMips < descriptor.numberOfMipMaps)) {
				glBindTexture(m_target, m_texture);
				// set TEXTURE_BASE_LEVEL to the last valid image in the mip chain
				glTexParameteri(m_target, GL_TEXTURE_BASE_LEVEL, std::max(validMips, 1U) - 1);
				// thus only generating the mip levels which have not been filled in with valid mip images
				glGenerateMipmap(m_target);
				// reset back to the highest mip level
				glTexParameteri(m_target, GL_TEXTURE_BASE_LEVEL, 0);
				glBindTexture(m_target, 0);
			}
		}

	} // namespace OGL
} // namespace Graphics
