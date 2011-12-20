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

bool Texture::CreateFromSurface(SDL_Surface *s)
{
	if (s->format->BitsPerPixel != 24 && s->format->BitsPerPixel != 32) {
		fprintf(stderr, "Texture::CreateFromSurface: cannot handle image with %d bits per pixel\n", s->format->BitsPerPixel);
		return false;
	}

	m_width = s->w;
	m_height = s->h;

	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &m_glTexture);
	glBindTexture(GL_TEXTURE_2D, m_glTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

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


ModelTexture::ModelTexture(const std::string &filename, bool preload) :
	Texture(GL_TEXTURE_2D, TextureFormat(GL_RGBA, GL_RGB, GL_UNSIGNED_BYTE), REPEAT, NEAREST, true),
	m_filename(filename)
{
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
}
