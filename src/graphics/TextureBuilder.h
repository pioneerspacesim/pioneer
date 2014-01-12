// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXTUREBUILDER_H
#define _TEXTUREBUILDER_H

#include <SDL.h>
#include <string>
#include "Texture.h"
#include "Renderer.h"
#include "SDLWrappers.h"

#include "PicoDDS/PicoDDS.h"

namespace Graphics {

class TextureBuilder {
public:
	TextureBuilder(const SDLSurfacePtr &surface, TextureSampleMode sampleMode = LINEAR_CLAMP, bool generateMipmaps = false, bool potExtend = false, bool forceRGBA = true, bool compressTextures = true);
	TextureBuilder(const std::string &filename, TextureSampleMode sampleMode = LINEAR_CLAMP, bool generateMipmaps = false, bool potExtend = false, bool forceRGBA = true, bool compressTextures = true, TextureType textureType = TEXTURE_2D);
	~TextureBuilder();

	// convenience constructors for common texture types
	static TextureBuilder Model(const std::string &filename) {
		return TextureBuilder(filename, LINEAR_REPEAT, true, false, false, true);
	}
	static TextureBuilder Billboard(const std::string &filename) {
		return TextureBuilder(filename, LINEAR_CLAMP, true, false, false, true);
	}
	static TextureBuilder UI(const std::string &filename) {
		return TextureBuilder(filename, LINEAR_CLAMP, false, true, true, false);
	}
	static TextureBuilder Decal(const std::string &filename) {
		return TextureBuilder(filename, LINEAR_CLAMP, true, true, false, true);
	}
	static TextureBuilder Cube(const std::string &filename) {
		return TextureBuilder(filename, LINEAR_CLAMP, true, true, false, true, TEXTURE_CUBE_MAP);
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

	//commonly used dummy textures
	static Texture *GetWhiteTexture(Renderer *);
	static Texture *GetTransparentTexture(Renderer *);

private:
	SDLSurfacePtr m_surface;
	std::vector<SDLSurfacePtr> m_cubemap;
	PicoDDS::DDSImage m_dds;
	std::string m_filename;

	TextureSampleMode m_sampleMode;
	bool m_generateMipmaps;

	bool m_potExtend;
	bool m_forceRGBA;
	bool m_compressTextures;
	TextureType m_textureType;

	TextureDescriptor m_descriptor;

	void PrepareSurface();
	bool m_prepared;

	void LoadSurface();
	void LoadDDS();
};

}

#endif
