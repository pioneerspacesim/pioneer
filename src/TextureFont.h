#ifndef _TEXTUREFONT_H
#define _TEXTUREFONT_H

#include "Font.h"
#include "Color.h"
#include "graphics/TextureDescriptor.h"

namespace Graphics { class Renderer; }
namespace Gui { class TexturedQuad; }

class TextureFont : public Font {

private:

#if 0
	class GlyphTextureDescriptor : public Graphics::TextureDescriptor {
	public:
		GlyphTextureDescriptor(const std::string &filename, Uint32 codePoint, const void *data, const vector2f &size);

		virtual const Graphics::TextureDescriptor::Data *GetData() const;

		virtual bool Compare(const Graphics::TextureDescriptor &b) const {
	        if (type != b.type) return TextureDescriptor::Compare(b);
	        const GlyphTextureDescriptor &bb = static_cast<const GlyphTextureDescriptor&>(b);
			return (filename < bb.filename && codePoint < bb.codePoint);
		}

		virtual GlyphTextureDescriptor *Clone() const {
			return new GlyphTextureDescriptor(*this);
		}

		const std::string filename;
		const Uint32 codePoint;
	
	private:
		const void *m_data;
		const vector2f m_size;
	};
#endif

public:
	TextureFont(const FontConfig &fc);
	~TextureFont();

	void RenderString(Graphics::Renderer *r, const char *str, float x, float y, const Color &color = Color::WHITE);
	Color RenderMarkup(Graphics::Renderer *r, const char *str, float x, float y, const Color &color = Color::WHITE);
	void MeasureString(const char *str, float &w, float &h);
	void MeasureCharacterPos(const char *str, int charIndex, float &x, float &y) const;
	int PickCharacter(const char *str, float mouseX, float mouseY) const;

	// of Ms
	float GetHeight() const { return m_height; }
	float GetWidth() const { return m_width; }
	float GetDescender() const { return m_descender; }
	struct glfglyph_t {
		Gui::TexturedQuad *quad;
		float advx, advy;
		float width, height;
		int offx, offy;
	};
	const glfglyph_t &GetGlyph(Uint32 ch) { return m_glyphs[ch]; }

	static int GetGlyphCount() { return s_glyphCount; }
	static void ClearGlyphCount() { s_glyphCount = 0; }

private:
	void RenderGlyph(Graphics::Renderer *r, Uint32 chr, float x, float y, const Color &color);
	float m_height;
	float m_width;
	float m_descender;
	int m_texSize, m_pixSize;

	static int s_glyphCount;
	std::map<Uint32,glfglyph_t> m_glyphs;
};

#endif
