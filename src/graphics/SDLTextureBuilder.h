#ifndef _SDLTEXTUREBUILDER_H
#define _SDLTEXTUREBUILDER_H

#include <SDL/SDL.h>
#include <string>
#include "Texture.h"
#include "Renderer.h"

namespace Graphics {

class SDLTextureBuilder {
public:
	SDLTextureBuilder(SDL_Surface *surface, bool potExtend = false, bool forceRGBA = true);
	SDLTextureBuilder(const std::string &filename, bool potExtend = false, bool forceRGBA = true);
	~SDLTextureBuilder();

	const TextureDescriptor &GetDescriptor() const { return m_descriptor; }
	void UpdateTexture(Texture *texture); // XXX pass src/dest rectangles

	Texture *CreateTexture(Renderer *r) {
		Texture *t = r->CreateTexture(m_descriptor);
		UpdateTexture(t);
		return t;
	}

private:
	void PrepareSurface(bool potExtend, bool forceRGBA);

	TextureDescriptor m_descriptor;
	SDL_Surface *m_surface;
};

}

#endif
