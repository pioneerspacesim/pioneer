#ifndef _SDLTEXTUREBUILDER_H
#define _SDLTEXTUREBUILDER_H

#include <SDL/SDL.h>
#include <string>
#include "Texture.h"
#include "Renderer.h"

namespace Graphics {

class SDLTextureBuilder {
public:
	SDLTextureBuilder(SDL_Surface *surface, TextureSampleMode sampleMode = LINEAR_CLAMP, bool generateMipmaps = false, bool potExtend = false, bool forceRGBA = true);
	SDLTextureBuilder(const std::string &filename, TextureSampleMode sampleMode = LINEAR_CLAMP, bool generateMipmaps = false, bool potExtend = false, bool forceRGBA = true);
	~SDLTextureBuilder();

	// convenience constructors for common texture types
	static SDLTextureBuilder Model(const std::string &filename) {
		return SDLTextureBuilder(filename, LINEAR_REPEAT, true, false, false);
	}
	static SDLTextureBuilder Billboard(const std::string &filename) {
		return SDLTextureBuilder(filename, LINEAR_CLAMP, true, false, false);
	}
	static SDLTextureBuilder UI(const std::string &filename) {
		return SDLTextureBuilder(filename, LINEAR_CLAMP, false, true, true);
	}

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

	TextureSampleMode m_sampleMode;
	bool m_generateMipmaps;

	bool m_potExtend;
	bool m_forceRGBA;

	TextureDescriptor m_descriptor;

	void PrepareSurface();
	bool m_prepared;

	void LoadSurface();
};

}

#endif
