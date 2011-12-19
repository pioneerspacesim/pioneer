#include "Texture.h"

Texture::Texture(const std::string &filename, bool preload, bool clamp) :
	m_filename(filename),
	m_clamp(clamp),
	m_isLoaded(false),
	m_width(-1),
	m_height(-1),
	m_tex(0)
{
	if (preload)
		Load();
}

Texture::Texture(SDL_Surface *s, bool clamp) :
	m_filename(),
	m_clamp(clamp),
	m_isLoaded(false),
	m_width(-1),
	m_height(-1),
	m_tex(0)
{
	if (!CreateFromSurface(s))
		fprintf(stderr, "Texture::Texture: creating texture from surface failed\n");
	else
		m_isLoaded = true;
}

Texture::~Texture()
{
	if (m_tex)
		glDeleteTextures(1, &m_tex);
}

void Texture::BindTexture()
{
	if (!IsLoaded())
		Load();
	glBindTexture(GL_TEXTURE_2D, m_tex);
}

void Texture::Load()
{
	if (m_isLoaded) return;

	SDL_Surface *s = IMG_Load(m_filename.c_str());
	if (!s) {
		fprintf(stderr, "Texture::Load: %s: %s\n", m_filename.c_str(), IMG_GetError());
		return;
	}

	if (!CreateFromSurface(s)) {
		fprintf(stderr, "Texture::Load: %s: creating texture from surface failed\n", m_filename.c_str());
		SDL_FreeSurface(s);
		return;
	}

	m_isLoaded = true;
}

bool Texture::CreateFromSurface(SDL_Surface *s)
{
	if (s->format->BitsPerPixel != 24 && s->format->BitsPerPixel != 32) {
		fprintf(stderr, "Texture::CreateFromSurface: cannot handle image with %d bits per pixel\n", s->format->BitsPerPixel);
		return false;
	}

	m_width = s->w;
	m_height = s->h;

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &m_tex);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	if (m_clamp) {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	switch (s->format->BitsPerPixel) {
		case 32:
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, s->w, s->h, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);
			break;

		case 24:
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, s->w, s->h, GL_RGB, GL_UNSIGNED_BYTE, s->pixels);
			break;

		default:
			assert(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	return true;
}
