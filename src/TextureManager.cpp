#include "TextureManager.h"
#include <map>

void Texture::Load()
{
	if (isLoaded) return;
	SDL_Surface *s = IMG_Load(filename.c_str());

	if (s) {
		glGenTextures (1, &tex);
		glBindTexture (GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		switch ( s->format->BitsPerPixel )
		{
		case 32:
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, s->w, s->h, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);
			break;
		case 24:
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, s->w, s->h, GL_RGB, GL_UNSIGNED_BYTE, s->pixels);
			break;
		default:
			Error("Texture '%s' needs to be 24 or 32 bit.", filename.c_str());
			exit(0);
		}
	
		SDL_FreeSurface(s);
		isLoaded = true;
		glBindTexture(GL_TEXTURE_2D, 0);
	} else {
		Error("IMG_Load: %s\n", IMG_GetError());
	}
}

namespace TextureManager {
	static std::map<std::string, Texture*> s_textures;

	Texture *GetTexture(const char *filename, bool preload)
	{
		std::map<std::string, Texture*>::iterator i = s_textures.find(filename);

		if (i != s_textures.end()) return (*i).second;

		Texture *tex = new Texture(filename, preload);
		s_textures[filename] = tex;
		return tex;
	}

	void Clear()
	{
		std::map<std::string, Texture*>::iterator i;
		for (i=s_textures.begin(); i!=s_textures.end(); ++i) delete (*i).second;		
	}
}

