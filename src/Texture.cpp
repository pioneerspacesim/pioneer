#include "Texture.h"

Texture::~Texture()
{
	if (m_glTexture)
		glDeleteTextures(1, &m_glTexture);
}

void Texture::Bind()
{
	assert(m_glTexture);
	glBindTexture(m_target, m_glTexture);
}

void Texture::Unbind()
{
	glBindTexture(m_target, 0);
}

// RGBA pixel format for converting textures
// XXX little-endian. if we ever have a port to a big-endian arch, invert shift and mask
static SDL_PixelFormat rgba_pixfmt = {
	0,                                  // palette
	32,                                 // bits per pixel
	4,                                  // bytes per pixel
	0, 0, 0, 0,                         // RGBA loss
	24, 16, 8, 0,                       // RGBA shift
	0xff, 0xff00, 0xff0000, 0xff000000, // RGBA mask
	0,                                  // colour key
	0                                   // alpha
};

bool Texture::CreateFromSurface(SDL_Surface *s)
{
	bool freeSurface = false;

	SDL_PixelFormat *pixfmt = s->format;
	if (pixfmt->BytesPerPixel != rgba_pixfmt.BytesPerPixel || pixfmt->Rmask != rgba_pixfmt.Rmask || pixfmt->Gmask != rgba_pixfmt.Gmask || pixfmt->Bmask != rgba_pixfmt.Bmask)
	{
		s = SDL_ConvertSurface(s, &rgba_pixfmt, SDL_SWSURFACE);
		freeSurface = true;
	}

	m_width = s->w;
	m_height = s->h;

	glEnable(m_target);

	glGenTextures(1, &m_glTexture);
	glBindTexture(m_target, m_glTexture);
	glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	gluBuild2DMipmaps(m_target, GL_RGBA, s->w, s->h, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);

	glBindTexture(m_target, 0);
	glDisable(m_target);

	if (freeSurface)
		SDL_FreeSurface(s);

	return true;
}


ModelTexture::ModelTexture(const std::string &filename, bool preload) :
	Texture(GL_TEXTURE_2D, TextureFormat(GL_RGBA, GL_RGB, GL_UNSIGNED_BYTE), REPEAT, NEAREST, true),
	m_filename(filename)
{
	SDL_Surface *s = IMG_Load(m_filename.c_str());
	if (!s) {
		fprintf(stderr, "ModelTexture::Load: %s: %s\n", m_filename.c_str(), IMG_GetError());
		return;
	}

	if (!CreateFromSurface(s)) {
		fprintf(stderr, "ModelTexture::Load: %s: creating texture from surface failed\n", m_filename.c_str());
		SDL_FreeSurface(s);
		return;
	}
}
