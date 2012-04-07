#include "TextureFont.h"
#include "libs.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "TextSupport.h"
#include "utils.h"
#include <algorithm>

#include FT_GLYPH_H
#include FT_STROKER_H

#define PARAGRAPH_SPACING 1.5f

namespace Text {

int TextureFont::s_glyphCount = 0;

void TextureFont::AddGlyphGeometry(Graphics::VertexArray *va, Uint32 chr, float x, float y, const Color &c)
{
	glfglyph_t *glyph = &m_glyphs[chr];

	const float offx = x + float(glyph->offx);
	const float offy = y + float(m_pixSize - glyph->offy);
	const float offU = glyph->offU;
	const float offV = glyph->offV;
	
	va->Add(vector3f(offx,                 offy,                  0.0f), c, vector2f(offU, offV));
	va->Add(vector3f(offx,                 offy+glyph->texHeight, 0.0f), c, vector2f(offU, offV + glyph->height));
	va->Add(vector3f(offx+glyph->texWidth, offy,                  0.0f), c, vector2f(offU + glyph->width, offV));

	va->Add(vector3f(offx+glyph->texWidth, offy,                  0.0f), c, vector2f(offU + glyph->width, offV));
	va->Add(vector3f(offx,                 offy+glyph->texHeight, 0.0f), c, vector2f(offU, offV + glyph->height));
	va->Add(vector3f(offx+glyph->texWidth, offy+glyph->texHeight, 0.0f), c, vector2f(offU + glyph->width, offV + glyph->height));

	s_glyphCount++;
}

void TextureFont::MeasureString(const char *str, float &w, float &h)
{
	w = 0;
	h = GetHeight();

	float line_width = 0;

	int i = 0;
	while (str[i]) {
		if (str[i] == '\n') {
			if (line_width > w) w = line_width;
			line_width = 0;
			h += GetHeight()*PARAGRAPH_SPACING;
			i++;
		}
		
		else {
			Uint32 chr;
			int n = conv_mb_to_wc(&chr, &str[i]);
			assert(n);
			i += n;

			line_width += m_glyphs[chr].advx;

			if (str[i]) {
				Uint32 chr2;
				n = conv_mb_to_wc(&chr2, &str[i]);
				assert(n);

				FT_UInt a = FT_Get_Char_Index(m_face, chr);
				FT_UInt b = FT_Get_Char_Index(m_face, chr2);

				FT_Vector kern;
				FT_Get_Kerning(m_face, a, b, FT_KERNING_UNFITTED, &kern);
				line_width += float(kern.x) / 64.0;
			}
		}
	}

	if (line_width > w) w = line_width;
	h += m_descender;
}

void TextureFont::MeasureCharacterPos(const char *str, int charIndex, float &charX, float &charY) const
{
	assert(str && (charIndex >= 0));

	const float lineSpacing = GetHeight()*PARAGRAPH_SPACING;
	float x = 0.0f, y = GetHeight();
	int i = 0;
	Uint32 chr;
	int len = conv_mb_to_wc(&chr, &str[i]);
	while (str[i] && (i < charIndex)) {
		Uint32 nextChar;
		i += len;
		len = conv_mb_to_wc(&nextChar, &str[i]);
		assert(!str[i] || len); // assert valid encoding

		if (chr == '\n') {
			x = 0.0f;
			y += lineSpacing;
		} else {
			std::map<Uint32,glfglyph_t>::const_iterator it = m_glyphs.find(chr);
			assert(it != m_glyphs.end());
			float advance = it->second.advx;

			if (nextChar != '\n' && nextChar != '\0') {
				FT_UInt a = FT_Get_Char_Index(m_face, chr);
				FT_UInt b = FT_Get_Char_Index(m_face, nextChar);
				FT_Vector kern;
				FT_Get_Kerning(m_face, a, b, FT_KERNING_UNFITTED, &kern);
				advance += float(kern.x) / 64.0f;
			}

			x += advance;
		}

		chr = nextChar;
	}

	charX = x;
	charY = y;
}

int TextureFont::PickCharacter(const char *str, float mouseX, float mouseY) const
{
	assert(str && mouseX >= 0.0f && mouseY >= 0.0f);

	// at the point of the mouse in-box test, the vars have the following values:
	// i1: the index of the character being tested
	// i2: the index of the next character
	// charBytes: the number of bytes used to encode the next character
	// right: the right edge of the box being tested
	// bottom:  the bottom of the box being tested
	// x: the x-coordinate of the next character
	// chr1: the Unicode value of the character being tested
	// chr2: the Unicode value of the next character

	const float lineSpacing = GetHeight()*PARAGRAPH_SPACING;
	Uint32 chr2 = '\n'; // pretend we've just had a new line
	float bottom = GetHeight() - lineSpacing, x = 0.0f;
	int i2 = 0, charBytes = 0;
	do {
		int i1 = i2;
		Uint32 chr1 = chr2;

		// read the next character
		i2 += charBytes;
		charBytes = conv_mb_to_wc(&chr2, &str[i2]);
		assert(!str[i2] || charBytes); // assert valid encoding

		float right;
		if (chr1 == '\n') {
			right = std::numeric_limits<float>::max();
			x = 0.0f;
		} else {
			std::map<Uint32,glfglyph_t>::const_iterator it = m_glyphs.find(chr1);
			assert(it != m_glyphs.end());
			float advance = it->second.advx;

			if (chr2 != '\n' && chr2 != '\0') {
				FT_UInt a = FT_Get_Char_Index(m_face, chr1);
				FT_UInt b = FT_Get_Char_Index(m_face, chr2);
				FT_Vector kern;
				FT_Get_Kerning(m_face, a, b, FT_KERNING_UNFITTED, &kern);
				advance += float(kern.x) / 64.0f;
			}

			right = x + (advance / 2.0f);
			x += advance;
		}

		if ((mouseY < bottom) && (mouseX < right))
			return i1;

		if (chr1 == '\n')
			bottom += lineSpacing;
	} while (charBytes);

	return i2;
}

void TextureFont::RenderString(const char *str, float x, float y, const Color &color)
{
	m_renderer->SetBlendMode(Graphics::BLEND_ALPHA);
	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0);

	float px = x;
	float py = y;

	int i = 0;
	while (str[i]) {
		if (str[i] == '\n') {
			px = x;
			py += GetHeight()*PARAGRAPH_SPACING;
			i++;
		}
		
		else {
			Uint32 chr;
			int n = conv_mb_to_wc(&chr, &str[i]);
			assert(n);
			i += n;

			glfglyph_t *glyph = &m_glyphs[chr];
			AddGlyphGeometry(&va, chr, roundf(px), py, color);

			if (str[i]) {
				Uint32 chr2;
				n = conv_mb_to_wc(&chr2, &str[i]);
				assert(n);

				FT_UInt a = FT_Get_Char_Index(m_face, chr);
				FT_UInt b = FT_Get_Char_Index(m_face, chr2);

				FT_Vector kern;
				FT_Get_Kerning(m_face, a, b, FT_KERNING_UNFITTED, &kern);
				px += float(kern.x) / 64.0;
			}

			px += glyph->advx;
		}
	}

	m_renderer->DrawTriangles(&va, &m_mat);
}

Color TextureFont::RenderMarkup(const char *str, float x, float y, const Color &color)
{
	m_renderer->SetBlendMode(Graphics::BLEND_ALPHA);
	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0);

	float px = x;
	float py = y;

	Color c = color;

	int i = 0;
	while (str[i]) {
		if (str[i] == '#') {
			int hexcol;
			if (sscanf(str+i, "#%3x", &hexcol)==1) {
				c.r = float((hexcol&0xf00)>>4)/255.0f;
				c.g = float((hexcol&0xf0))/255.0f;
				c.b = float((hexcol&0xf)<<4)/255.0f;
				i+=4;
				continue;
			}
		}

		if (str[i] == '\n') {
			px = x;
			py += GetHeight()*PARAGRAPH_SPACING;
			i++;
		}
		
		else {
			Uint32 chr;
			int n = conv_mb_to_wc(&chr, &str[i]);
			assert(n);
			i += n;

			glfglyph_t *glyph = &m_glyphs[chr];
			AddGlyphGeometry(&va, chr, roundf(px), py, c);

			// XXX kerning doesn't skip markup
			if (str[i]) {
				Uint32 chr2;
				n = conv_mb_to_wc(&chr2, &str[i]);
				assert(n);

				FT_UInt a = FT_Get_Char_Index(m_face, chr);
				FT_UInt b = FT_Get_Char_Index(m_face, chr2);

				FT_Vector kern;
				FT_Get_Kerning(m_face, a, b, FT_KERNING_UNFITTED, &kern);
				px += float(kern.x) / 64.0;
			}

			px += glyph->advx;
		}
	}

	m_renderer->DrawTriangles(&va, &m_mat);
	return c;
}

TextureFont::TextureFont(const FontDescriptor &descriptor, Graphics::Renderer *renderer) : Font(descriptor), m_renderer(renderer)
{
	int err; // used to store freetype error return codes
	const int a_width = GetDescriptor().pixelWidth;
	const int a_height = GetDescriptor().pixelHeight;

	const float advx_adjust = GetDescriptor().advanceXAdjustment;

	m_pixSize = a_height;

	FT_Set_Pixel_Sizes(m_face, a_width, a_height);

	const int sz = 512; //current fonts use maybe 1/4 of this...
	m_texSize = sz;

	//UV offsets for glyphs
	int atlasU = 0;
	int atlasV = 0;
	int atlasVIncrement = 0;

	//temporary RGBA pixel buffer for the glyph atlas
	std::vector<unsigned char> pixBuf(4*sz*sz);
	std::fill(pixBuf.begin(), pixBuf.end(), 0);

	Graphics::TextureDescriptor textureDescriptor(Graphics::TEXTURE_RGBA, vector2f(sz,sz), Graphics::NEAREST_CLAMP);
	m_texture.Reset(m_renderer->CreateTexture(textureDescriptor));
	m_mat.texture0 = m_texture.Get();
	m_mat.unlit = true;
	m_mat.vertexColors = true; //to allow per-character colors
	
	bool outline = GetDescriptor().outline;

	FT_Stroker stroker(0);
	if (outline) {
		if (FT_Stroker_New(GetFreeTypeLibrary(), &stroker)) {
			fprintf(stderr, "Freetype stroker init error\n");
			abort();
		}

		//1*64 = stroke width
		FT_Stroker_Set(stroker, 1*64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	}

	//load 479 characters
	for (Uint32 chr=s_firstCharacter; chr<s_lastCharacter; chr++) {
		glfglyph_t glfglyph;
		FT_Glyph glyph;

		err = FT_Load_Char(m_face, chr, FT_LOAD_FORCE_AUTOHINT);
		if (err) {
			fprintf(stderr, "Error %d loading glyph\n", err);
			continue;
		}

		//get base glyph again
		err = FT_Get_Glyph(m_face->glyph, &glyph);
		if (err) {
			fprintf(stderr, "Glyph get error %d\n", err);
			continue;
		}

		//convert to bitmap
		if (glyph->format != FT_GLYPH_FORMAT_BITMAP) {
			err = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
			if (err) {
				fprintf(stderr, "Couldn't convert glyph to bitmap, error %d\n", err);
				continue;
			}
		}

		const FT_BitmapGlyph bmGlyph = FT_BitmapGlyph(glyph);

		if (outline) {
			FT_Glyph strokeGlyph;

			err = FT_Get_Glyph(m_face->glyph, &strokeGlyph);
			if (err) {
				fprintf(stderr, "Glyph get error %d\n", err);
				continue;
			}
	
			err = FT_Glyph_Stroke(&strokeGlyph, stroker, 1);
			if (err) {
				fprintf(stderr, "Glyph stroke error %d\n", err);
				continue;
			}
	
			//convert to bitmap
			if (strokeGlyph->format != FT_GLYPH_FORMAT_BITMAP) {
				err = FT_Glyph_To_Bitmap(&strokeGlyph, FT_RENDER_MODE_NORMAL, 0, 1);
				if (err) {
					fprintf(stderr, "Couldn't convert glyph to bitmap, error %d\n", err);
					continue;
				}
			}
	
			const FT_BitmapGlyph bmStrokeGlyph = FT_BitmapGlyph(strokeGlyph);

			//don't run off atlas borders
			atlasVIncrement = std::max(atlasVIncrement, bmStrokeGlyph->bitmap.rows);
			if (atlasU + bmStrokeGlyph->bitmap.width > sz) {
				atlasU = 0;
				atlasV += atlasVIncrement;
			}
	
			//copy to the atlas texture
			//stroke first
			int pitch = bmStrokeGlyph->bitmap.pitch;
			for (int row=0; row < bmStrokeGlyph->bitmap.rows; row++) {
				for (int col=0; col < bmStrokeGlyph->bitmap.width; col++) {
					//assume black outline
					const int d = 4*sz*(row+atlasV) + 4*(col+atlasU);
					const int s = pitch*row + col;
					pixBuf[d]  = pixBuf[d+1] = pixBuf[d+2] = 0; //lum
					pixBuf[d+3] = bmStrokeGlyph->bitmap.buffer[s]; //alpha
				}
			}
	
			//overlay normal glyph (luminance only)
			int xoff = (bmStrokeGlyph->bitmap.width - bmGlyph->bitmap.width) / 2;
			int yoff = (bmStrokeGlyph->bitmap.rows - bmGlyph->bitmap.rows) / 2;
			pitch = bmGlyph->bitmap.pitch;
			for (int row=0; row < bmGlyph->bitmap.rows; row++) {
				for (int col=0; col < bmGlyph->bitmap.width; col++) {
					const int d = 4*sz*(row+atlasV+xoff) + 4*(col+atlasU+yoff);
					const int s = pitch*row + col;
					pixBuf[d] = pixBuf[d+1] = pixBuf[d+2] = bmGlyph->bitmap.buffer[s];
				}
			}
	
			glfglyph.width = bmStrokeGlyph->bitmap.width / float(sz);
			glfglyph.height = bmStrokeGlyph->bitmap.rows / float(sz);
			glfglyph.offx = bmStrokeGlyph->left;
			glfglyph.offy = bmStrokeGlyph->top;
			glfglyph.offU = atlasU / float(sz);
			glfglyph.offV = atlasV / float(sz);

			atlasU += bmStrokeGlyph->bitmap.width;

			FT_Done_Glyph(strokeGlyph);
		}

		else {
			//don't run off atlas borders
			atlasVIncrement = std::max(atlasVIncrement, bmGlyph->bitmap.rows);
			if (atlasU + bmGlyph->bitmap.width >= sz) {
				atlasU = 0;
				atlasV += atlasVIncrement;
			}

			//copy glyph bitmap to the atlas texture
			//the glyphs are upside down in the texture due to how freetype stores them
			//but it's just a matter of adjusting the texcoords
			const int pitch = bmGlyph->bitmap.pitch;
			const int rows = bmGlyph->bitmap.rows;
			for (int row=0; row < rows; row++) {
				for (int col=0; col < bmGlyph->bitmap.width; col++) {
					const int d = 4*sz*(row+atlasV) + 4*(col+atlasU);
					const int s = pitch*row + col;
					pixBuf[d] = pixBuf[d+1] = pixBuf[d+2] = pixBuf[d+3] = bmGlyph->bitmap.buffer[s];
				}
			}
	
			glfglyph.width = bmGlyph->bitmap.width / float(sz);
			glfglyph.height = bmGlyph->bitmap.rows / float(sz);
			glfglyph.offx = bmGlyph->left;
			glfglyph.offy = bmGlyph->top;
			glfglyph.offU = atlasU / float(sz);
			glfglyph.offV = atlasV / float(sz);

			atlasU += bmGlyph->bitmap.width;
		}

		FT_Done_Glyph(glyph);

		glfglyph.texWidth = m_texSize*glfglyph.width;
		glfglyph.texHeight = m_texSize*glfglyph.height;

		glfglyph.advx = float(m_face->glyph->advance.x) / 64.f + advx_adjust;
		glfglyph.advy = float(m_face->glyph->advance.y) / 64.f;
		m_glyphs[chr] = glfglyph;
	}

	//upload atlas
	m_texture->Update(&pixBuf[0], vector2f(sz,sz), Graphics::IMAGE_RGBA, Graphics::IMAGE_UNSIGNED_BYTE);

	if (outline)
		FT_Stroker_Done(stroker);

	m_height = float(a_height);
	m_width = float(a_width);
	m_descender = -float(m_face->descender) / 64.f;
}

}
