#ifndef _TEXTUREBUILDER_H
#define _TEXTUREBUILDER_H

#include <SDL.h>
#include <string>
#include "Texture.h"
#include "Renderer.h"

namespace Graphics {

class TextureBuilder {
public:
	TextureBuilder(SDL_Surface *surface, TextureSampleMode sampleMode = LINEAR_CLAMP, bool generateMipmaps = false, bool potExtend = false, bool forceRGBA = true);
	TextureBuilder(const std::string &filename, TextureSampleMode sampleMode = LINEAR_CLAMP, bool generateMipmaps = false, bool potExtend = false, bool forceRGBA = true, const std::string &cacheType = "");
	~TextureBuilder();

	// convenience constructors for common texture types
	static TextureBuilder Model(const std::string &filename) {
		return TextureBuilder(filename, LINEAR_REPEAT, true, false, false, "model");
	}
	static TextureBuilder Billboard(const std::string &filename) {
		return TextureBuilder(filename, LINEAR_CLAMP, true, false, false, "billboard");
	}
	static TextureBuilder UI(const std::string &filename) {
		return TextureBuilder(filename, LINEAR_CLAMP, false, true, true, "ui");
	}

	const TextureDescriptor &GetDescriptor() { PrepareSurface(); return m_descriptor; }
	void UpdateTexture(Texture *texture); // XXX pass src/dest rectangles

	Texture *CreateTexture(Renderer *r) {
		Texture *t = r->CreateTexture(GetDescriptor());
		UpdateTexture(t);
		return t;
	}

	Texture *GetOrCreateTexture(Renderer *r, const std::string &name = "") {
		const std::string &cacheName = name.length() > 0 ? name : m_filename;
		assert(cacheName.length() > 0);
		Texture *t = r->GetCachedTexture(m_cacheType, cacheName);
		if (t) return t;
		t = CreateTexture(r);
		r->AddCachedTexture(m_cacheType, cacheName, t);
		return t;
	}

private:
	SDL_Surface *m_surface;
	std::string m_filename;

	TextureSampleMode m_sampleMode;
	bool m_generateMipmaps;

	bool m_potExtend;
	bool m_forceRGBA;

	std::string m_cacheType;

	TextureDescriptor m_descriptor;

	void PrepareSurface();
	bool m_prepared;

	void LoadSurface();
};

}

#endif
