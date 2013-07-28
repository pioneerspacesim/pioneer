// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TextureBuilder.h"
#include "FileSystem.h"
#include "utils.h"
#include <SDL_image.h>
#include <SDL_rwops.h>

#include <algorithm>

namespace Graphics {

TextureBuilder::TextureBuilder(const SDLSurfacePtr &surface, TextureSampleMode sampleMode, bool generateMipmaps, bool potExtend, bool forceRGBA, bool compressTextures) :
    m_surface(surface), m_sampleMode(sampleMode), m_generateMipmaps(generateMipmaps), m_potExtend(potExtend), m_forceRGBA(forceRGBA), m_compressTextures(compressTextures), m_prepared(false)
{
}

TextureBuilder::TextureBuilder(const std::string &filename, TextureSampleMode sampleMode, bool generateMipmaps, bool potExtend, bool forceRGBA, bool compressTextures) :
    m_filename(filename), m_sampleMode(sampleMode), m_generateMipmaps(generateMipmaps), m_potExtend(potExtend), m_forceRGBA(forceRGBA), m_compressTextures(compressTextures), m_prepared(false)
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
	0,                                  // palette
	32,                                 // bits per pixel
	4,                                  // bytes per pixel
	0, 0, 0, 0,                         // RGBA loss
	24, 16, 8, 0,                       // RGBA shift
	0xff, 0xff00, 0xff0000, 0xff000000, // RGBA mask
	0,                                  // colour key
	0                                   // alpha
};

static SDL_PixelFormat pixelFormatRGB = {
	0,                                  // palette
	24,                                 // bits per pixel
	3,                                  // bytes per pixel
	0, 0, 0, 0,                         // RGBA loss
	16, 8, 0, 0,                        // RGBA shift
	0xff, 0xff00, 0xff0000, 0,          // RGBA mask
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
		if (ends_with(filename, ".dds")) {
			LoadDDS();
		} else {
			LoadSurface();
		}
	}

	TextureFormat targetTextureFormat;
	unsigned int virtualWidth, actualWidth, virtualHeight, actualHeight, numberOfMipMaps = 0;
	if( m_surface ) {
		SDL_PixelFormat *targetPixelFormat;
		bool needConvert = !GetTargetFormat(m_surface->format, &targetTextureFormat, &targetPixelFormat, m_forceRGBA);

		if (needConvert) {
			SDL_Surface *s = SDL_ConvertSurface(m_surface.Get(), targetPixelFormat, SDL_SWSURFACE);
			m_surface = SDLSurfacePtr::WrapNew(s);
		}

		virtualWidth = actualWidth = m_surface->w;
		virtualHeight = actualHeight = m_surface->h;

		if (m_potExtend) {
			// extend to power-of-two if necessary
			actualWidth = ceil_pow2(m_surface->w);
			actualHeight = ceil_pow2(m_surface->h);
			if (actualWidth != virtualWidth || actualHeight != virtualHeight) {
				SDL_Surface *s = SDL_CreateRGBSurface(SDL_SWSURFACE, actualWidth, actualHeight, targetPixelFormat->BitsPerPixel,
					targetPixelFormat->Rmask, targetPixelFormat->Gmask, targetPixelFormat->Bmask, targetPixelFormat->Amask);

				SDL_SetAlpha(m_surface.Get(), 0, 0);
				SDL_SetAlpha(s, 0, 0);
				SDL_BlitSurface(m_surface.Get(), 0, s, 0);

				m_surface = SDLSurfacePtr::WrapNew(s);
			}
		}
		else if (! m_filename.empty()) {
			// power-of-two check
			unsigned long width = ceil_pow2(m_surface->w);
			unsigned long height = ceil_pow2(m_surface->h);

			if (width != virtualWidth || height != virtualHeight)
				fprintf(stderr, "WARNING: texture '%s' is not power-of-two and may not display correctly\n", m_filename.c_str());
		}
	} else {
		switch(m_dds.GetTextureFormat()) {
		case PicoDDS::FORMAT_DXT1: targetTextureFormat = TEXTURE_DXT1; break;
		case PicoDDS::FORMAT_DXT5: targetTextureFormat = TEXTURE_DXT5; break;
		default:
			fprintf(stderr, "ERROR: DDS texture with invalid format '%s' (only DXT1 and DXT5 are supported)\n", m_filename.c_str());
			assert(false);
			return;
		}

		virtualWidth = actualWidth = m_dds.imgdata_.width;
		virtualHeight = actualHeight = m_dds.imgdata_.height;
		numberOfMipMaps = m_dds.imgdata_.numMipMaps;
	}

	m_descriptor = TextureDescriptor(
		targetTextureFormat,
		vector2f(actualWidth,actualHeight),
		vector2f(float(virtualWidth)/float(actualWidth),float(virtualHeight)/float(actualHeight)),
		m_sampleMode, m_generateMipmaps, m_compressTextures, numberOfMipMaps);

	m_prepared = true;
}

static size_t LoadDDSFromFile(const std::string &filename, PicoDDS::DDSImage& dds)
{
	RefCountedPtr<FileSystem::FileData> filedata = FileSystem::gameDataFiles.ReadFile(filename);
	if (!filedata) {
		fprintf(stderr, "LoadDDSFromFile: %s: could not read file\n", filename.c_str());
		return 0;
	}

	// read the dds file
	const size_t sizeRead = dds.Read( filedata->GetData(), filedata->GetSize() );
	return sizeRead;
}

void TextureBuilder::LoadSurface()
{
	assert(!m_surface);

	SDLSurfacePtr s = LoadSurfaceFromFile(m_filename);
	if (! s) { s = LoadSurfaceFromFile("textures/unknown.png"); }

	// XXX if we can't load the fallback texture, then what?
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
		texture->Update(m_surface->pixels, vector2f(m_surface->w,m_surface->h), m_descriptor.format, 0);
	} else {
		assert(m_dds.headerdone_);
		assert(m_descriptor.format == TEXTURE_DXT1 || m_descriptor.format == TEXTURE_DXT5);
		texture->Update(m_dds.imgdata_.imgData, vector2f(m_dds.imgdata_.width,m_dds.imgdata_.height), m_descriptor.format, m_dds.imgdata_.numMipMaps);
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
