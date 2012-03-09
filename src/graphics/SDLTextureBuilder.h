#ifndef _SDLTEXTUREBUILDER_H
#define _SDLTEXTUREBUILDER_H

#include <SDL/SDL.h>
#include <string>
#include "Texture.h"
#include "Renderer.h"

namespace Graphics {

class SDLTextureBuilder {
public:
	SDLTextureBuilder(SDL_Surface *surface, TextureSampler sampler = LINEAR_CLAMP, bool potExtend = false, bool forceRGBA = true);
	SDLTextureBuilder(const std::string &filename, TextureSampler sampler = LINEAR_CLAMP, bool potExtend = false, bool forceRGBA = true);
	~SDLTextureBuilder();

	const TextureDescriptor &GetDescriptor() { PrepareSurface(); return m_descriptor; }
	void UpdateTexture(Texture *texture); // XXX pass src/dest rectangles

	Texture *CreateTexture(Renderer *r) {
		Texture *t = r->CreateTexture(GetDescriptor());
		UpdateTexture(t);
		return t;
	}

	Texture *GetOrCreateTexture(Renderer *r, const std::string &type, const std::string &name = "") {
		const std::string &cacheName = name.length() > 0 ? name : m_filename;
		assert(cacheName.length() > 0);
		Texture *t = r->GetCachedTexture(type, cacheName);
		if (t) return t;
		t = CreateTexture(r);
		r->AddCachedTexture(type, cacheName, t);
		return t;
	}

private:
	SDL_Surface *m_surface;
	std::string m_filename;

	TextureSampler m_sampler;
	bool m_potExtend;
	bool m_forceRGBA;

	TextureDescriptor m_descriptor;

	void PrepareSurface();
	bool m_prepared;

	void LoadSurface();
};

}

#endif
