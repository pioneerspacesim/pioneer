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

	if (m_wrapMode == CLAMP) {
		glTexParameterf(m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(m_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else {
		glTexParameterf(m_target, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(m_target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	if (m_filterMode == NEAREST) {
		glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_hasMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
	}
	else {
		glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_hasMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	}

	// XXX use format options
	if (m_hasMipmaps)
		gluBuild2DMipmaps(m_target, GL_RGBA, s->w, s->h, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);
	else
		glTexImage2D(m_target, 0, GL_RGBA, s->w, s->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);

	glBindTexture(m_target, 0);
	glDisable(m_target);

	if (freeSurface)
		SDL_FreeSurface(s);

	return true;
}

bool Texture::CreateFromFile(const std::string &filename)
{
	SDL_Surface *s = IMG_Load(filename.c_str());
	if (!s) {
		fprintf(stderr, "Texture::CreateFromFile: %s: %s\n", filename.c_str(), IMG_GetError());
		return false;
	}

	if (!CreateFromSurface(s)) {
		fprintf(stderr, "Texture::CreateFromFile: %s: creating texture from surface failed\n", filename.c_str());
		SDL_FreeSurface(s);
		return false;
	}

	SDL_FreeSurface(s);

	return true;
}


ModelTexture::ModelTexture(const std::string &filename, bool preload) :
	Texture(GL_TEXTURE_2D, TextureFormat(GL_RGBA, GL_RGB, GL_UNSIGNED_BYTE), REPEAT, NEAREST, true),
	m_filename(filename),
	m_isLoaded(false)
{
	if (preload)
		Load();
}

void ModelTexture::Load()
{
	assert(!m_isLoaded);

	m_isLoaded = CreateFromFile(m_filename);
}


UITexture::UITexture(const std::string &filename) :
    Texture(GL_TEXTURE_2D, TextureFormat(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE), CLAMP, LINEAR, false)
{
	CreateFromFile(filename);
}
