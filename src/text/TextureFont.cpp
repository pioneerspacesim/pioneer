// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TextureFont.h"
#include "FileSystem.h"
#include "libs.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "TextSupport.h"
#include "utils.h"
#include <algorithm>

#include FT_GLYPH_H

static const int ATLAS_SIZE = 1024;

namespace Text {

// range end-points are inclusive
const Uint32 TextureFont::CHARACTER_RANGES[] = {
	0x0020, 0x007E, // Basic Latin (excluding control characters)
	0x00A0, 0x00FF, // Latin-1 Supplement (excluding control characters)
	0x0100, 0x017F, // Latin Extended-A
	0x0180, 0x024F, // Latin Extended-B
	0x0400, 0x04FF, // Cyrillic
	0x0500, 0x0527, // Cyrillic Supplement (excluding unused codepoints)
	0x2DE0, 0x2DFF, // Cyrillic Extended-A
	0xA640, 0xA697, // Cyrillic Extended-B (excluding unused codepoints)
	0xA69F, 0xA69F, // Cyrillic Extended-B (continued)
	0,0 // terminator
};

int TextureFont::s_glyphCount = 0;

void TextureFont::AddGlyphGeometry(Graphics::VertexArray *va, const glfglyph_t &glyph, float x, float y, const Color &c)
{
	const float offx = x + float(glyph.offx);
	const float offy = y + GetHeight() - float(glyph.offy);
	const float offU = glyph.offU;
	const float offV = glyph.offV;

	const vector3f p0(offx,             offy,              0.0f);
	const vector3f p1(offx,             offy+glyph.height, 0.0f);
	const vector3f p2(offx+glyph.width, offy,              0.0f);
	const vector3f p3(offx+glyph.width, offy+glyph.height, 0.0f);

	const vector2f t0(offU,                offV                );
	const vector2f t1(offU,                offV+glyph.texHeight);
	const vector2f t2(offU+glyph.texWidth, offV                );
	const vector2f t3(offU+glyph.texWidth, offV+glyph.texHeight);

	va->Add(p0, c, t0);
	va->Add(p1, c, t1);
	va->Add(p2, c, t2);

	va->Add(p2, c, t2);
	va->Add(p1, c, t1);
	va->Add(p3, c, t3);

	s_glyphCount++;
}

void TextureFont::MeasureString(const char *str, float &w, float &h)
{
	w = h = 0.0f;

	float line_width = 0.0f;

	int i = 0;
	while (str[i]) {
		if (str[i] == '\n') {
			if (line_width > w) w = line_width;
			line_width = 0.0f;
			h += GetHeight();
			i++;
		}

		else {
			Uint32 chr;
			int n = utf8_decode_char(&chr, &str[i]);
			assert(n);
			i += n;

			const glfglyph_t &glyph = GetGlyph(chr);

			line_width += glyph.advx;

			if (str[i]) {
				Uint32 chr2;
				n = utf8_decode_char(&chr2, &str[i]);
				assert(n);

				FT_Vector kern;
				FT_Get_Kerning(m_face, glyph.ftIndex, GetGlyph(chr2).ftIndex, FT_KERNING_UNFITTED, &kern);
				line_width += float(kern.x) / 64.0;
			}
		}
	}

	if (line_width > w) w = line_width;
	h += GetHeight() + GetDescender();
}

void TextureFont::MeasureCharacterPos(const char *str, int charIndex, float &charX, float &charY) const
{
	assert(str && (charIndex >= 0));

	float x = 0.0f, y = GetHeight();
	int i = 0;
	Uint32 chr;
	int len = utf8_decode_char(&chr, &str[i]);
	while (str[i] && (i < charIndex)) {
		Uint32 nextChar;
		i += len;
		len = utf8_decode_char(&nextChar, &str[i]);
		assert(!str[i] || len); // assert valid encoding

		if (chr == '\n') {
			x = 0.0f;
			y += GetHeight();
		} else {
			const glfglyph_t &glyph = GetGlyph(chr);
			float advance = glyph.advx;

			if (nextChar != '\n' && nextChar != '\0') {
				FT_Vector kern;
				FT_Get_Kerning(m_face, glyph.ftIndex, GetGlyph(nextChar).ftIndex, FT_KERNING_UNFITTED, &kern);
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

	Uint32 chr2 = '\n'; // pretend we've just had a new line
	float bottom = 0.0f, x = 0.0f;
	int i2 = 0, charBytes = 0;
	do {
		int i1 = i2;
		Uint32 chr1 = chr2;

		// read the next character
		i2 += charBytes;
		charBytes = utf8_decode_char(&chr2, &str[i2]);
		assert(!str[i2] || charBytes); // assert valid encoding

		float right;
		if (chr1 == '\n') {
			right = std::numeric_limits<float>::max();
			x = 0.0f;
		} else {
			const glfglyph_t &glyph = GetGlyph(chr1);
			float advance = glyph.advx;

			if (chr2 != '\n' && chr2 != '\0') {
				FT_Vector kern;
				FT_Get_Kerning(m_face, glyph.ftIndex, GetGlyph(chr2).ftIndex, FT_KERNING_UNFITTED, &kern);
				advance += float(kern.x) / 64.0f;
			}

			right = x + (advance / 2.0f);
			x += advance;
		}

		if ((mouseY < bottom) && (mouseX < right))
			return i1;

		if (chr1 == '\n')
			bottom += GetHeight();
	} while (charBytes);

	return i2;
}

void TextureFont::RenderString(const char *str, float x, float y, const Color &color)
{
	PROFILE_SCOPED()
	m_vertices.Clear();

	float alpha_f = color.a / 255.0f;
	const Color premult_color = Color(color.r * alpha_f, color.g * alpha_f, color.b * alpha_f, color.a);

	float px = x;
	float py = y;

	int i = 0;
	while (str[i]) {
		if (str[i] == '\n') {
			px = x;
			py += GetHeight();
			i++;
		}

		else {
			Uint32 chr;
			int n = utf8_decode_char(&chr, &str[i]);
			assert(n);
			i += n;

			const glfglyph_t &glyph = GetGlyph(chr);
			AddGlyphGeometry(&m_vertices, glyph, roundf(px), py, premult_color);

			if (str[i]) {
				Uint32 chr2;
				n = utf8_decode_char(&chr2, &str[i]);
				assert(n);

				FT_Vector kern;
				FT_Get_Kerning(m_face, glyph.ftIndex, GetGlyph(chr2).ftIndex, FT_KERNING_UNFITTED, &kern);
				px += float(kern.x) / 64.0;
			}

			px += glyph.advx;
		}
	}

	m_renderer->DrawTriangles(&m_vertices, m_renderState, m_mat.get());
}

Color TextureFont::RenderMarkup(const char *str, float x, float y, const Color &color)
{
	PROFILE_SCOPED()
	m_vertices.Clear();

	float px = x;
	float py = y;

	Color c = color;
	float alpha_f = c.a / 255.0f;
	Color premult_c = Color(c.r * alpha_f, c.g * alpha_f, c.b * alpha_f, c.a);

	int i = 0;
	while (str[i]) {
		if (str[i] == '#') {
			int hexcol;
			if (sscanf(str+i, "#%3x", &hexcol)==1) {
				c.r = float((hexcol&0xf00)>>4);
				c.g = float((hexcol&0xf0));
				c.b = float((hexcol&0xf)<<4);
				// retain alpha value from RenderMarkup color parameter
				premult_c.r = c.r * alpha_f;
				premult_c.g = c.g * alpha_f;
				premult_c.b = c.b * alpha_f;
				i+=4;
				continue;
			}
		}

		if (str[i] == '\n') {
			px = x;
			py += GetHeight();
			i++;
		}

		else {
			Uint32 chr;
			int n = utf8_decode_char(&chr, &str[i]);
			assert(n);
			i += n;

			const glfglyph_t &glyph = GetGlyph(chr);
			AddGlyphGeometry(&m_vertices, glyph, roundf(px), py, premult_c);

			// XXX kerning doesn't skip markup
			if (str[i]) {
				Uint32 chr2;
				n = utf8_decode_char(&chr2, &str[i]);
				assert(n);

				FT_Vector kern;
				FT_Get_Kerning(m_face, glyph.ftIndex, GetGlyph(chr2).ftIndex, FT_KERNING_UNFITTED, &kern);
				px += float(kern.x) / 64.0;
			}

			px += glyph.advx;
		}
	}

	m_renderer->DrawTriangles(&m_vertices, m_renderState, m_mat.get());
	return c;
}

int TextureFont::BakeGlyph(Uint32 chr)
{
	int err;
	glfglyph_t glfglyph;
	FT_Glyph glyph;

	glfglyph.ftIndex = FT_Get_Char_Index(m_face, chr);

	err = FT_Load_Char(m_face, chr, FT_LOAD_FORCE_AUTOHINT);
	if (err) {
		Output("Error %d loading glyph\n", err);
        return err;
	}

	// get base glyph again
	err = FT_Get_Glyph(m_face->glyph, &glyph);
	if (err) {
		Output("Glyph get error %d\n", err);
        return err;
	}

	// convert to bitmap
	if (glyph->format != FT_GLYPH_FORMAT_BITMAP) {
		err = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
		if (err) {
			Output("Couldn't convert glyph to bitmap, error %d\n", err);
            return err;
		}
	}

	const FT_BitmapGlyph bmGlyph = FT_BitmapGlyph(glyph);

	if (m_descriptor.outline) {
		FT_Glyph strokeGlyph;

		err = FT_Get_Glyph(m_face->glyph, &strokeGlyph);
		if (err) {
			Output("Glyph get error %d\n", err);
            return err;
		}

		err = FT_Glyph_Stroke(&strokeGlyph, m_stroker, 1);
		if (err) {
			Output("Glyph stroke error %d\n", err);
			FT_Done_Glyph(strokeGlyph);
            return err;
		}

		//convert to bitmap
		if (strokeGlyph->format != FT_GLYPH_FORMAT_BITMAP) {
			err = FT_Glyph_To_Bitmap(&strokeGlyph, FT_RENDER_MODE_NORMAL, 0, 1);
			if (err) {
				Output("Couldn't convert glyph to bitmap, error %d\n", err);
				FT_Done_Glyph(strokeGlyph);
				return err;
			}
		}

		const FT_BitmapGlyph bmStrokeGlyph = FT_BitmapGlyph(strokeGlyph);

		//don't run off atlas borders
		m_atlasVIncrement = std::max(m_atlasVIncrement, bmStrokeGlyph->bitmap.rows);
		if (m_atlasU + bmStrokeGlyph->bitmap.width > ATLAS_SIZE) {
			m_atlasU = 0;
			m_atlasV += m_atlasVIncrement;
			m_atlasVIncrement = 0;
		}

		if (m_atlasV + bmStrokeGlyph->bitmap.rows > ATLAS_SIZE) {
			char utf8buf[8];
			int len = utf8_encode_char(chr, utf8buf);
			utf8buf[len] = '\0';
			Output("glyph doesn't fit in atlas (U+%04X; height = %d; char: %s; atlasV = %d)\n", chr, bmStrokeGlyph->bitmap.rows, utf8buf, m_atlasV);
			FT_Done_Glyph(strokeGlyph);
			return 0;
		}

		const int pitch = bmGlyph->bitmap.pitch;
		const int rows = bmGlyph->bitmap.rows;
		const int strokePitch = bmStrokeGlyph->bitmap.pitch;
		const int strokeRows = bmStrokeGlyph->bitmap.rows;

		const int xoff = (bmStrokeGlyph->bitmap.width - bmGlyph->bitmap.width) / 2;
		const int yoff = (bmStrokeGlyph->bitmap.rows - bmGlyph->bitmap.rows) / 2;

		// make enough space for both glyphs including offset
		m_buf.resize(ALIGN(strokePitch,4)*strokeRows*2);
		std::fill(m_buf.begin(), m_buf.end(), 0);

		// stroke first into the alpha channel
		for (int y = 0; y < strokeRows; y++) {
			for (int x = 0; x < strokePitch; x++) {
				const int d = ALIGN(strokePitch*2,4)*y+x*2;
				const int s = strokePitch*y+x;
				m_buf[d+1] = bmStrokeGlyph->bitmap.buffer[s]; // alpha
			}
		}

		// now the normal glyph into the luminance channel
		for (int y = 0; y < rows; y++) {
			for (int x = 0; x < pitch; x++) {
				const int d = ALIGN(strokePitch*2,4)*(y+yoff)+(x+xoff)*2;
				const int s = pitch*y+x;
				m_buf[d] = bmGlyph->bitmap.buffer[s]; // luminance
			}
		}

		glfglyph.width = bmStrokeGlyph->bitmap.width;
		glfglyph.height = bmStrokeGlyph->bitmap.rows;
		glfglyph.offx = bmStrokeGlyph->left;
		glfglyph.offy = bmStrokeGlyph->top;
		glfglyph.offU = float(m_atlasU) / float(ATLAS_SIZE);
		glfglyph.offV = float(m_atlasV) / float(ATLAS_SIZE);
		glfglyph.texWidth = float(glfglyph.width) / float(ATLAS_SIZE);
		glfglyph.texHeight = float(glfglyph.height) / float(ATLAS_SIZE);

		m_texture->Update(&m_buf[0], vector2f(m_atlasU, m_atlasV), vector2f(glfglyph.width, glfglyph.height), m_texFormat);

		m_atlasU += bmStrokeGlyph->bitmap.width;

		FT_Done_Glyph(strokeGlyph);
	}

	else {

		//don't run off atlas borders
		m_atlasVIncrement = std::max(m_atlasVIncrement, bmGlyph->bitmap.rows);
		if (m_atlasU + bmGlyph->bitmap.width >= ATLAS_SIZE) {
			m_atlasU = 0;
			m_atlasV += m_atlasVIncrement;
			m_atlasVIncrement = 0;
		}

		if (m_atlasV + bmGlyph->bitmap.rows > ATLAS_SIZE) {
			char utf8buf[8];
			int len = utf8_encode_char(chr, utf8buf);
			utf8buf[len] = '\0';
			Output("glyph doesn't fit in atlas (U+%04X; height = %d; char: %s; atlasV = %d)\n", chr, bmGlyph->bitmap.rows, utf8buf, m_atlasV);
			return 0;
		}

		// draw the glyph into the draw buffer
		const int pitch = bmGlyph->bitmap.pitch;
		const int rows = bmGlyph->bitmap.rows;
		m_buf.resize(ALIGN(pitch,4)*rows);
		std::fill(m_buf.begin(), m_buf.end(), 0);
		for (int y = 0; y < rows; y++)
			memcpy(&m_buf[ALIGN(pitch,4)*y], &(bmGlyph->bitmap.buffer[pitch*y]), pitch);

		glfglyph.width = bmGlyph->bitmap.width;
		glfglyph.height = bmGlyph->bitmap.rows;
		glfglyph.offx = bmGlyph->left;
		glfglyph.offy = bmGlyph->top;
		glfglyph.offU = float(m_atlasU) / float(ATLAS_SIZE);
		glfglyph.offV = float(m_atlasV) / float(ATLAS_SIZE);
		glfglyph.texWidth = float(glfglyph.width) / float(ATLAS_SIZE);
		glfglyph.texHeight = float(glfglyph.height) / float(ATLAS_SIZE);

		m_texture->Update(&m_buf[0], vector2f(m_atlasU, m_atlasV), vector2f(glfglyph.width, glfglyph.height), m_texFormat);

		m_atlasU += glfglyph.width;
	}

	FT_Done_Glyph(glyph);

	glfglyph.advx = float(m_face->glyph->advance.x) / 64.f + m_descriptor.advanceXAdjustment;
	glfglyph.advy = float(m_face->glyph->advance.y) / 64.f;

	if (chr < MAX_FAST_GLYPHS)
		m_glyphsFast[chr] = glfglyph;
	else
		m_glyphs[chr] = glfglyph;

	return 0;
}

TextureFont::TextureFont(const FontDescriptor &descriptor, Graphics::Renderer *renderer)
	: m_descriptor(descriptor)
	, m_renderer(renderer)
	, m_freeTypeLibrary(nullptr)
	, m_face(nullptr)
	, m_stroker(nullptr)
	, m_vertices(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0)
	, m_glyphsFast(MAX_FAST_GLYPHS)
	, m_atlasU(0)
	, m_atlasV(0)
	, m_atlasVIncrement(0)
{
	memset(&m_glyphsFast[0], 0, sizeof(glfglyph_t)*MAX_FAST_GLYPHS);

	FT_Error err; // used to store freetype error return codes

	err = FT_Init_FreeType(&m_freeTypeLibrary);
	if (err != 0) {
		Output("Couldn't create FreeType library context (%d)\n", err);
		abort();
	}

	m_fontFileData = FileSystem::gameDataFiles.ReadFile("fonts/" + m_descriptor.filename);
	if (! m_fontFileData) {
		Output("Terrible error! Couldn't load '%s'.\n", m_descriptor.filename.c_str());
		abort();
	}

	if (0 != (err = FT_New_Memory_Face(m_freeTypeLibrary,
			reinterpret_cast<const FT_Byte*>(m_fontFileData->GetData()),
			m_fontFileData->GetSize(), 0, &m_face))) {
		Output("Terrible error! Couldn't understand '%s'; error %d.\n", m_descriptor.filename.c_str(), err);
		abort();
	}

	const int a_width = m_descriptor.pixelWidth;
	const int a_height = m_descriptor.pixelHeight;

	FT_Set_Pixel_Sizes(m_face, a_width, a_height);

	m_texFormat = m_descriptor.outline ? Graphics::TEXTURE_LUMINANCE_ALPHA_88 : Graphics::TEXTURE_INTENSITY_8;

	m_bpp = m_descriptor.outline ? 2 : 1;

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA_PREMULT;
	rsd.depthWrite = false;
	m_renderState = m_renderer->CreateRenderState(rsd);

	Graphics::MaterialDescriptor desc;
	desc.vertexColors = true; //to allow per-character colors
	desc.textures = 1;
	m_mat.reset(m_renderer->CreateMaterial(desc));
	Graphics::TextureDescriptor textureDescriptor(m_texFormat, vector2f(ATLAS_SIZE), Graphics::NEAREST_CLAMP, false, false);
	m_texture.Reset(m_renderer->CreateTexture(textureDescriptor));
	m_mat->texture0 = m_texture.Get();

	if (m_descriptor.outline) {
		if (FT_Stroker_New(m_freeTypeLibrary, &m_stroker)) {
			Output("Freetype stroker init error\n");
			abort();
		}

		//1*64 = stroke width
		FT_Stroker_Set(m_stroker, 1*64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	}

	// generate and store glyphs for each character in the specified ranges
	for (int i = 0;; i += 2) {
		Uint32 first = CHARACTER_RANGES[i];
		Uint32 last = CHARACTER_RANGES[i+1];
		if (first == 0 && last == 0)
			break;

		for (Uint32 chr = first; chr <= last; chr++)
			BakeGlyph(chr);
	}

	m_height = float(m_face->height) / 64.f * float(m_face->size->metrics.y_scale) / 65536.f;
	m_descender = -float(m_face->descender) / 64.f * float(m_face->size->metrics.y_scale) / 65536.f;
}

TextureFont::~TextureFont()
{
	if (m_stroker)
		FT_Stroker_Done(m_stroker);
	FT_Done_Face(m_face);
	FT_Done_FreeType(m_freeTypeLibrary);
}

}
