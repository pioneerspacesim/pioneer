#include "TextureDescriptor.h"
#include "Texture.h"

namespace Graphics {

// RGBA and RGBpixel format for converting textures
// XXX little-endian. if we ever have a port to a big-endian arch, invert shift and mask
static SDL_PixelFormat pixfmtRGBA = {
	0,                                  // palette
	32,                                 // bits per pixel
	4,                                  // bytes per pixel
	0, 0, 0, 0,                         // RGBA loss
	24, 16, 8, 0,                       // RGBA shift
	0xff, 0xff00, 0xff0000, 0xff000000, // RGBA mask
	0,                                  // colour key
	0                                   // alpha
};

static SDL_PixelFormat pixfmtRGB = {
	0,                                  // palette
	24,                                 // bits per pixel
	3,                                  // bytes per pixel
	0, 0, 0, 0,                         // RGBA loss
	16, 8, 0, 0,                        // RGBA shift
	0xff, 0xff00, 0xff0000, 0,          // RGBA mask
	0,                                  // colour key
	0                                   // alpha
};

static inline bool GetTargetFormat(const SDL_PixelFormat *pixfmt, GLenum *targetGLformat, SDL_PixelFormat **targetPixfmt, bool forceRGBA)
{
	if (!forceRGBA && pixfmt->BytesPerPixel == pixfmtRGB.BytesPerPixel && pixfmt->Rmask == pixfmtRGB.Rmask && pixfmt->Bmask == pixfmtRGB.Bmask && pixfmt->Gmask == pixfmtRGB.Gmask) {
		*targetGLformat = GL_RGB;
		*targetPixfmt = &pixfmtRGB;
		return true;
	}

	if (pixfmt->BytesPerPixel == pixfmtRGBA.BytesPerPixel && pixfmt->Rmask == pixfmtRGBA.Rmask && pixfmt->Bmask == pixfmtRGBA.Bmask && pixfmt->Gmask == pixfmtRGBA.Gmask) {
		*targetGLformat = GL_RGBA;
		*targetPixfmt = &pixfmtRGBA;
		return true;
	}
	
	if (!forceRGBA && pixfmt->BytesPerPixel == 3) {
		*targetGLformat = GL_RGB;
		*targetPixfmt = &pixfmtRGB;
		return false;
	}

	*targetGLformat = GL_RGBA;
	*targetPixfmt = &pixfmtRGBA;
	return false;
}

static inline Uint32 ceil_pow2(Uint32 v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

const TextureDescriptor::Data *TextureDescriptor::GetDataFromSurface(SDL_Surface *s, bool potExtend, bool forceRGBA) const
{
	bool freeSurface = false;

	SDL_PixelFormat *pixfmt = s->format;

	GLenum targetGLformat;
	SDL_PixelFormat *targetPixfmt;
	bool needConvert = !GetTargetFormat(pixfmt, &targetGLformat, &targetPixfmt, forceRGBA);

	if (needConvert) {
		s = SDL_ConvertSurface(s, targetPixfmt, SDL_SWSURFACE);
		freeSurface = true;
	}

	/*
	// store incoming 24-bit as GL_RGB to save on texture memory
	if (targetGLformat == GL_RGB && m_format.internalFormat == Texture::Format::INTERNAL_RGBA) {
		m_format.internalFormat = Texture::Format::INTERNAL_RGB;
		m_format.dataFormat = Texture::Format::DATA_RGB;
	}
	*/

	unsigned int vwidth, width, vheight, height;
	vwidth = width = s->w;
	vheight = height = s->h;

	if (potExtend) {
		// extend to power-of-two if necessary
		width = ceil_pow2(s->w);
		height = ceil_pow2(s->h);
		if (width != vwidth || height != vheight) {
			SDL_Surface *s2 = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, targetPixfmt->BitsPerPixel, targetPixfmt->Rmask, targetPixfmt->Gmask, targetPixfmt->Bmask, targetPixfmt->Amask);

			SDL_SetAlpha(s, 0, 0);
			SDL_SetAlpha(s2, 0, 0);
			SDL_BlitSurface(s, 0, s2, 0);

			if (freeSurface)
				SDL_FreeSurface(s);

			s = s2;
			freeSurface = true;
		}
	}

	// XXX ick
	void *raw = malloc(s->w*s->h*4);
	SDL_LockSurface(s);
	memcpy(raw, s->pixels, s->w*s->h*4);
	SDL_UnlockSurface(s);

	TextureDescriptor::Data *data = new TextureDescriptor::Data(raw, vector2f(width,height), vector2f(float(vwidth)/float(width),float(vheight)/float(height)));

	if (freeSurface)
		SDL_FreeSurface(s);

	return data;
}

const TextureDescriptor::Data *TextureDescriptor::GetDataFromFile(const std::string &filename, bool potExtend, bool forceRGBA) const
{
	SDL_Surface *s = IMG_Load(filename.c_str());
	if (!s) {
		fprintf(stderr, "Texture::CreateFromFile: %s: %s\n", filename.c_str(), IMG_GetError());
		return false;
	}

	const TextureDescriptor::Data *data = GetDataFromSurface(s, potExtend, forceRGBA);

	SDL_FreeSurface(s);

	return data;
}

}
