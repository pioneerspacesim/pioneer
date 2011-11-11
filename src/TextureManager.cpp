#include "TextureManager.h"
#include <map>

void Texture::Load()
{
	if (isLoaded) return;
	SDL_Surface *s = IMG_Load(filename.c_str());

	if (s) {
		width = s->w;
		height = s->h;

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
	typedef std::map<std::string, Texture*> TextureCacheMap;
	static TextureCacheMap s_textures;

	Texture *GetTexture(const std::string &filename, bool preload)
	{
		std::pair<TextureCacheMap::iterator, bool>
			ret = s_textures.insert(TextureCacheMap::value_type(filename, static_cast<Texture*>(0)));
				// cast required as work around broken msvc rvalue reference crap
		if (ret.second) {
			Texture *tex = new Texture(filename, preload);
			ret.first->second = tex;
			return tex;
		} else {
			return ret.first->second;
		}
	}

	void Clear()
	{
		TextureCacheMap::iterator i;
		for (i=s_textures.begin(); i!=s_textures.end(); ++i) delete (*i).second;
	}
}

