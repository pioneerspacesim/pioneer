#include "Textures.h"
#include "libs.h"

std::map<std::string, GLuint> Textures::m_textures;

void Textures::Init()
{

}

void Textures::FreeAll()
{
	unsigned int num = 0;
	for (std::map<std::string, GLuint>::const_iterator it = m_textures.begin();
		it != m_textures.end();
		it++)
	{
		glDeleteTextures(1, &it->second);
		num++;
	}
	printf("Freed %d textures\n", num);
	m_textures.clear();
}

GLuint Textures::Load(const std::string& filename)
{
	GLuint tex = -1;
	std::map<std::string, GLuint>::iterator t = m_textures.find(filename);
	if (t != m_textures.end()) return (*t).second;

	SDL_Surface *s = IMG_Load(filename.c_str());

	if (s) {
		glGenTextures (1, &tex);
		glBindTexture (GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		switch ( s->format->BitsPerPixel ) {
		case 32:
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, s->w, s->h, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);
			break;
		case 24:
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, s->w, s->h, GL_RGB, GL_UNSIGNED_BYTE, s->pixels);
			break;
		default:
			printf("Texture '%s' needs to be 24 or 32 bit but it is %d.\n",
				filename.c_str(), s->format->BitsPerPixel);
			exit(0);
		}

		SDL_FreeSurface(s);
		m_textures[filename] = tex;

	} else {
		Error("IMG_Load: %s\n", IMG_GetError());
	}

	return tex;
}
