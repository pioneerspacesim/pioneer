// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TextureBuilder.h"
#include "FileSystem.h"
#include "utils.h"
#include <SDL_image.h>
#include <SDL_rwops.h>
#include <algorithm>

// XXX SDL2 can all this be replaced with SDL_GL_BindTexture?

namespace Graphics {

TextureBuilder::TextureBuilder(const SDLSurfacePtr &surface, TextureSampleMode sampleMode, bool generateMipmaps, bool potExtend, bool forceRGBA, bool compressTextures) :
    m_surface(surface), m_sampleMode(sampleMode), m_generateMipmaps(generateMipmaps), m_potExtend(potExtend), m_forceRGBA(forceRGBA), m_compressTextures(compressTextures), m_textureType(TEXTURE_2D), m_prepared(false)
{
}

TextureBuilder::TextureBuilder(const std::string &filename, TextureSampleMode sampleMode, bool generateMipmaps, bool potExtend, bool forceRGBA, bool compressTextures, TextureType textureType) :
    m_filename(filename), m_sampleMode(sampleMode), m_generateMipmaps(generateMipmaps), m_potExtend(potExtend), m_forceRGBA(forceRGBA), m_compressTextures(compressTextures), m_textureType(textureType), m_prepared(false)
{
}

TextureBuilder::~TextureBuilder()
{
}

// RGBA and RGBpixel format for converting textures
// XXX little-endian. if we ever have a port to a big-endian arch, invert shift and mask
#if SDL_BYTEORDER != SDL_LIL_ENDIAN
#error "SDL surface pixel formats are endian-specific"
#endif
static SDL_PixelFormat pixelFormatRGBA = {
	0,                                  // format#
	0,                                  // palette
	32,                                 // bits per pixel
	4,                                  // bytes per pixel
	{ 0, 0 },                           // padding
	0xff, 0xff00, 0xff0000, 0xff000000, // RGBA mask
	0, 0, 0, 0,                         // RGBA loss
	24, 16, 8, 0,                       // RGBA shift
	0,                                  // colour key
	0                                   // alpha
};

static SDL_PixelFormat pixelFormatRGB = {
	0,                                  // format#
	0,                                  // palette
	24,                                 // bits per pixel
	3,                                  // bytes per pixel
	{ 0, 0 },                           // padding
	0xff, 0xff00, 0xff0000, 0,          // RGBA mask
	0, 0, 0, 0,                         // RGBA loss
	16, 8, 0, 0,                        // RGBA shift
	0,                                  // colour key
	0                                   // alpha
};

static inline bool GetTargetFormat(const SDL_PixelFormat *sourcePixelFormat, TextureFormat *targetTextureFormat, SDL_PixelFormat **targetPixelFormat, bool forceRGBA)
{
	if (!forceRGBA && sourcePixelFormat->BytesPerPixel == pixelFormatRGB.BytesPerPixel &&
			sourcePixelFormat->Rmask == pixelFormatRGB.Rmask && sourcePixelFormat->Bmask == pixelFormatRGB.Bmask && sourcePixelFormat->Gmask == pixelFormatRGB.Gmask) {
		*targetTextureFormat = TEXTURE_RGB_888;
		*targetPixelFormat = &pixelFormatRGB;
		return true;
	}

	if (sourcePixelFormat->BytesPerPixel == pixelFormatRGBA.BytesPerPixel &&
			sourcePixelFormat->Rmask == pixelFormatRGBA.Rmask && sourcePixelFormat->Bmask == pixelFormatRGBA.Bmask && sourcePixelFormat->Gmask == pixelFormatRGBA.Gmask) {
		*targetTextureFormat = TEXTURE_RGBA_8888;
		*targetPixelFormat = &pixelFormatRGBA;
		return true;
	}

	if (!forceRGBA && sourcePixelFormat->BytesPerPixel == 3) {
		*targetTextureFormat = TEXTURE_RGB_888;
		*targetPixelFormat = &pixelFormatRGB;
		return false;
	}

	*targetTextureFormat = TEXTURE_RGBA_8888;
	*targetPixelFormat = &pixelFormatRGBA;
	return false;
}

void TextureBuilder::PrepareSurface()
{
	if (m_prepared) return;

	if (!m_surface && !m_filename.empty()) {
		std::string filename = m_filename;
		std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
		if (ends_with_ci(filename, ".dds")) {
			LoadDDS();
		} else {
			LoadSurface();
		}
	}

	TextureFormat targetTextureFormat;
	unsigned int virtualWidth, actualWidth, virtualHeight, actualHeight, numberOfMipMaps = 0, numberOfImages = 1;
	if( m_surface ) {
		SDL_PixelFormat *targetPixelFormat;
		bool needConvert = !GetTargetFormat(m_surface->format, &targetTextureFormat, &targetPixelFormat, m_forceRGBA);

		if (needConvert) {
			if(m_textureType == TEXTURE_2D) {
				SDL_Surface *s = SDL_ConvertSurface(m_surface.Get(), targetPixelFormat, SDL_SWSURFACE);
				m_surface = SDLSurfacePtr::WrapNew(s);
			} else if(m_textureType == TEXTURE_CUBE_MAP) {
				assert(m_cubemap.size() == 6);
				for(unsigned int i = 0; i < 6; ++i) {
					SDL_Surface *s = SDL_ConvertSurface(m_cubemap[i].Get(), targetPixelFormat, SDL_SWSURFACE);
					m_cubemap[i] = SDLSurfacePtr::WrapNew(s);
				}
			} else {
				// Unknown texture type
				assert(0);
			}
		}

		virtualWidth = actualWidth = m_surface->w;
		virtualHeight = actualHeight = m_surface->h;

		if (m_potExtend) {
			// extend to power-of-two if necessary
			actualWidth = ceil_pow2(m_surface->w);
			actualHeight = ceil_pow2(m_surface->h);
			if (actualWidth != virtualWidth || actualHeight != virtualHeight) {
				if(m_textureType == TEXTURE_2D) {
					SDL_Surface *s = SDL_CreateRGBSurface(SDL_SWSURFACE, actualWidth, actualHeight, targetPixelFormat->BitsPerPixel,
						targetPixelFormat->Rmask, targetPixelFormat->Gmask, targetPixelFormat->Bmask, targetPixelFormat->Amask);
					SDL_SetSurfaceBlendMode(m_surface.Get(), SDL_BLENDMODE_NONE);
					SDL_BlitSurface(m_surface.Get(), 0, s, 0);

					m_surface = SDLSurfacePtr::WrapNew(s);
				} else if(m_textureType == TEXTURE_CUBE_MAP) {
					assert(m_cubemap.size() == 6);
					for(unsigned int i = 0; i < 6; ++i) {
						SDL_Surface *s = SDL_CreateRGBSurface(SDL_SWSURFACE, actualWidth, actualHeight, targetPixelFormat->BitsPerPixel,
							targetPixelFormat->Rmask, targetPixelFormat->Gmask, targetPixelFormat->Bmask, targetPixelFormat->Amask);
						SDL_SetSurfaceBlendMode(m_cubemap[i].Get(), SDL_BLENDMODE_NONE);
						SDL_BlitSurface(m_cubemap[i].Get(), 0, s, 0);
						m_cubemap[i] = SDLSurfacePtr::WrapNew(s);
					}
				} else {
					assert(0);
				}
			}
		}
		else if (! m_filename.empty()) {
			// power-of-two check
			unsigned long width = ceil_pow2(m_surface->w);
			unsigned long height = ceil_pow2(m_surface->h);

			if (width != virtualWidth || height != virtualHeight)
				Output("WARNING: texture '%s' is not power-of-two and may not display correctly\n", m_filename.c_str());
		}
	} else {
		switch(m_dds.GetTextureFormat()) {
		case PicoDDS::FORMAT_DXT1: targetTextureFormat = TEXTURE_DXT1; break;
		case PicoDDS::FORMAT_DXT5: targetTextureFormat = TEXTURE_DXT5; break;
		default:
			Output("ERROR: DDS texture with invalid format '%s' (only DXT1 and DXT5 are supported)\n", m_filename.c_str());
			assert(false);
			return;
		}

		virtualWidth = actualWidth = m_dds.imgdata_.width;
		virtualHeight = actualHeight = m_dds.imgdata_.height;
		numberOfMipMaps = m_dds.imgdata_.numMipMaps;
		numberOfImages = m_dds.imgdata_.numImages;
		if(m_textureType == TEXTURE_CUBE_MAP) {
			// Cube map must be fully defined (6 images) to be used correctly
			assert(numberOfImages == 6);
		}
	}

	m_descriptor = TextureDescriptor(
		targetTextureFormat,
		vector2f(actualWidth,actualHeight),
		vector2f(float(virtualWidth)/float(actualWidth),float(virtualHeight)/float(actualHeight)),
		m_sampleMode, m_generateMipmaps, m_compressTextures, numberOfMipMaps, m_textureType);

	m_prepared = true;
}

static size_t LoadDDSFromFile(const std::string &filename, PicoDDS::DDSImage& dds)
{
	RefCountedPtr<FileSystem::FileData> filedata = FileSystem::gameDataFiles.ReadFile(filename);
	if (!filedata) {
		Output("LoadDDSFromFile: %s: could not read file\n", filename.c_str());
		return 0;
	}

	// read the dds file
	const size_t sizeRead = dds.Read( filedata->GetData(), filedata->GetSize() );
	return sizeRead;
}

void TextureBuilder::LoadSurface()
{
	assert(!m_surface);

	SDLSurfacePtr s;
	if(m_textureType == TEXTURE_2D) {
		s = LoadSurfaceFromFile(m_filename);
		if (! s) { 
			s = LoadSurfaceFromFile("textures/unknown.png"); 
		}
	} else if(m_textureType == TEXTURE_CUBE_MAP) {
		Output("LoadSurface: %s: cannot load non-DDS cubemaps\n", m_filename.c_str());
	}

	// XXX if we can't load the fallback texture, then what?
	assert(s);
	m_surface = s;
}

void TextureBuilder::LoadDDS()
{
	assert(!m_dds.headerdone_);
	LoadDDSFromFile(m_filename, m_dds);

	if (!m_dds.headerdone_) {
		m_surface = LoadSurfaceFromFile("textures/unknown.png");
	}
	// XXX if we can't load the fallback texture, then what?
}

void TextureBuilder::UpdateTexture(Texture *texture)
{
	if( m_surface ) {
		if(texture->GetDescriptor().type == TEXTURE_2D && m_textureType == TEXTURE_2D) {
			texture->Update(m_surface->pixels, vector2f(m_surface->w,m_surface->h), m_descriptor.format, 0);
		} else if(texture->GetDescriptor().type == TEXTURE_CUBE_MAP && m_textureType == TEXTURE_CUBE_MAP) {
			assert(m_cubemap.size() == 6);
			TextureCubeData tcd;
			// Sequence of cube map face storage: +X -X +Y -Y -Z +Z
			tcd.posX = m_cubemap[0]->pixels;
			tcd.negX = m_cubemap[1]->pixels;
			tcd.posY = m_cubemap[2]->pixels;
			tcd.negY = m_cubemap[3]->pixels;
			tcd.posZ = m_cubemap[4]->pixels;
			tcd.negZ = m_cubemap[5]->pixels;
			texture->Update(tcd, vector2f(m_cubemap[0]->w, m_cubemap[0]->h), m_descriptor.format, 0);
		} else {
			// Given texture and current texture don't have the same type!
			assert(0);
		}
	} else {
		assert(m_dds.headerdone_);
		assert(m_descriptor.format == TEXTURE_DXT1 || m_descriptor.format == TEXTURE_DXT5);
		if(texture->GetDescriptor().type == TEXTURE_2D && m_textureType == TEXTURE_2D) {
			texture->Update(m_dds.imgdata_.imgData, vector2f(m_dds.imgdata_.width,m_dds.imgdata_.height), m_descriptor.format, m_dds.imgdata_.numMipMaps);
		} else if(texture->GetDescriptor().type == TEXTURE_CUBE_MAP && m_textureType == TEXTURE_CUBE_MAP) {
			TextureCubeData tcd;
			// Size in bytes of each cube map face
			size_t face_size = m_dds.imgdata_.size / m_dds.imgdata_.numImages;
			// Sequence of cube map face storage: +X -X +Y -Y +Z -Z
			tcd.posX = static_cast<void*>(m_dds.imgdata_.imgData + (0 * face_size));
			tcd.negX = static_cast<void*>(m_dds.imgdata_.imgData + (1 * face_size));
			tcd.posY = static_cast<void*>(m_dds.imgdata_.imgData + (2 * face_size));
			tcd.negY = static_cast<void*>(m_dds.imgdata_.imgData + (3 * face_size));
			tcd.posZ = static_cast<void*>(m_dds.imgdata_.imgData + (4 * face_size));
			tcd.negZ = static_cast<void*>(m_dds.imgdata_.imgData + (5 * face_size));
			texture->Update(tcd, vector2f(m_dds.imgdata_.width, m_dds.imgdata_.height), m_descriptor.format, m_dds.imgdata_.numMipMaps);
		} else {
			// Given texture and current texture don't have the same type!
			assert(0);
		}
	}
}

Texture *TextureBuilder::GetWhiteTexture(Renderer *r)
{
	return Model("textures/white.png").GetOrCreateTexture(r, "model");
}

Texture *TextureBuilder::GetTransparentTexture(Renderer *r)
{
	return Model("textures/transparent.png").GetOrCreateTexture(r, "model");
}

}
