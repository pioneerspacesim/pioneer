// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
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
	TextureBuilder(const SDLSurfacePtr &surface, TextureSampleMode sampleMode = LINEAR_CLAMP, bool generateMipmaps = false, bool potExtend = false, bool forceRGBA = true, bool compressTextures = true, bool anisoFiltering = true);
	TextureBuilder(const std::string &filename, TextureSampleMode sampleMode = LINEAR_CLAMP, bool generateMipmaps = false, bool potExtend = false, bool forceRGBA = true, bool compressTextures = true, bool anisoFiltering = true, TextureType textureType = TEXTURE_2D);
	~TextureBuilder();

	static void Init();

	// convenience constructors for common texture types
	static TextureBuilder Model(const std::string &filename) {
		return TextureBuilder(filename, LINEAR_REPEAT, true, false, false, true, true);
	}
	static TextureBuilder Normal(const std::string &filename) {
		return TextureBuilder(filename, LINEAR_REPEAT, true, false, false, false, true);
	}
	static TextureBuilder Billboard(const std::string &filename) {
		return TextureBuilder(filename, LINEAR_CLAMP, true, false, false, true, false);
	}
	static TextureBuilder UI(const std::string &filename) {
		return TextureBuilder(filename, LINEAR_CLAMP, false, true, true, false, false);
	}
	static TextureBuilder Decal(const std::string &filename) {
		return TextureBuilder(filename, LINEAR_CLAMP, true, true, false, true, true);
	}
	static TextureBuilder Raw(const std::string &filename) {
		return TextureBuilder(filename, NEAREST_REPEAT, false, false, false, false, false);
	}
	static TextureBuilder Cube(const std::string &filename) {
		return TextureBuilder(filename, LINEAR_CLAMP, true, true, false, true, false, TEXTURE_CUBE_MAP);
	}

	const TextureDescriptor &GetDescriptor() { PrepareSurface(); return m_descriptor; }

	Texture *GetOrCreateTexture(Renderer *r, const std::string &type) {
		if(m_filename.empty()) {
			return CreateTexture(r);
		}
		SDL_LockMutex(m_textureLock);
		Texture *t = r->GetCachedTexture(type, m_filename);
		if (t) { SDL_UnlockMutex(m_textureLock); return t; }
		t = CreateTexture(r);
		r->AddCachedTexture(type, m_filename, t);
		SDL_UnlockMutex(m_textureLock);
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
	bool m_anisotropicFiltering;
	TextureType m_textureType;

	TextureDescriptor m_descriptor;

	Texture *CreateTexture(Renderer *r) {
		Texture *t = r->CreateTexture(GetDescriptor());
		UpdateTexture(t);
		return t;
	}
	void UpdateTexture(Texture *texture); // XXX pass src/dest rectangles
	void PrepareSurface();
	bool m_prepared;

	void LoadSurface();
	void LoadDDS();

	static SDL_mutex *m_textureLock;
};

}

#endif
