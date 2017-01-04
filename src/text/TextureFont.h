// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXT_TEXTUREFONT_H
#define _TEXT_TEXTUREFONT_H

#include "libs.h"
#include "FontConfig.h"
#include "RefCounted.h"
#include "graphics/Texture.h"
#include "graphics/Material.h"
#include "graphics/VertexBuffer.h"
#include "graphics/RenderState.h"

#include <unordered_map>

namespace FileSystem { class FileData; }

// forward declarations for FreeType types
struct FT_FaceRec_;
struct FT_LibraryRec_;
struct FT_StrokerRec_;
typedef struct FT_FaceRec_ *FT_Face;
typedef struct FT_LibraryRec_ *FT_Library;
typedef struct FT_StrokerRec_ *FT_Stroker;

namespace Text {

class TextureFont : public RefCounted {

public:
	TextureFont(const FontConfig &config, Graphics::Renderer *renderer, float scale = 1.0f);
	~TextureFont();

	void RenderBuffer(Graphics::VertexBuffer *vb, const Color &color = Color::WHITE);
	void MeasureString(const std::string &str, float &w, float &h);
	void MeasureCharacterPos(const std::string &str, int charIndex, float &x, float &y);
	int PickCharacter(const std::string &str, float mouseX, float mouseY);

	void PopulateString(Graphics::VertexArray &va, const std::string &str, const float x, const float y, const Color &color = Color::WHITE);
	Color PopulateMarkup(Graphics::VertexArray &va, const std::string &str, const float x, const float y, const Color &color = Color::WHITE);
	Graphics::VertexBuffer* CreateVertexBuffer(const Graphics::VertexArray &va, const bool bIsStatic) const;
	Graphics::VertexBuffer* CreateVertexBuffer(const Graphics::VertexArray &va, const std::string &str, const bool bIsStatic);
	Graphics::VertexBuffer* GetCachedVertexBuffer(const std::string &str);

	// general baseline-to-baseline height
	float GetHeight() const { return m_height; }
	// general descender height
	float GetDescender() const { return m_descender; }

	struct Glyph {
		Glyph() : advX(0), advY(0), width(0), height(0), texWidth(0), texHeight(0), offX(0), offY(0), offU(0), offV(0), ftFace(nullptr), ftIndex(0) {}
		float advX, advY;
		float width, height;
		float texWidth, texHeight;
		int offX, offY;
		float offU, offV; //atlas UV offset
		FT_Face ftFace;
		Uint32 ftIndex;
	};
	const Glyph &GetGlyph(Uint32 ch);

	static int GetGlyphCount() { return s_glyphCount; }
	static void ClearGlyphCount() { s_glyphCount = 0; }

	RefCountedPtr<Graphics::Texture> GetTexture() const  { return m_texture; }
	Graphics::Material* GetMaterial() const { return m_mat.get(); }

private:
	TextureFont(const TextureFont &);
	TextureFont &operator=(const TextureFont &);

	void AddCachedVertexBuffer(Graphics::VertexBuffer *pVB, const std::string &str);
	Uint32 CleanVertexBufferCache();

	FontConfig m_config;
	Graphics::Renderer *m_renderer;
	float m_scale;

	FT_Library m_ftLib;
	FT_Stroker m_stroker;

	FT_Face GetFTFace(const FontConfig::Face &face);
	std::map<FontConfig::Face,std::pair<FT_Face,RefCountedPtr<FileSystem::FileData>>> m_faces;

	Glyph BakeGlyph(Uint32 chr);

	float GetKern(const Glyph &a, const Glyph &b);

	void AddGlyphGeometry(Graphics::VertexArray &va, const Glyph &glyph, const float x, const float y, const Color &color);
	float m_height;
	float m_descender;
	std::unique_ptr<Graphics::Material> m_mat;
	Graphics::RenderState *m_renderState;

	static int s_glyphCount;

	std::map<Uint32,Glyph> m_glyphs;

	// UV offsets for glyphs
	unsigned int m_atlasU;
	unsigned int m_atlasV;
	unsigned int m_atlasVIncrement;

	RefCountedPtr<Graphics::Texture> m_texture;
	Graphics::TextureFormat m_texFormat;

	std::vector<unsigned char> m_buf;
	int m_bufWidth, m_bufHeight;
	int m_bpp;

	typedef std::unordered_map<std::string, std::pair<double,RefCountedPtr<Graphics::VertexBuffer>>> VBHashMap;
	typedef VBHashMap::iterator VBHashMapIter;
	typedef VBHashMap::const_iterator VBHashMapConstIter;
	VBHashMap m_vbTextCache;
	double m_lfLastCacheCleanTime;
};

}

#endif
