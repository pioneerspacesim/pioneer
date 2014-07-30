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

int TextureFont::s_glyphCount = 0;

void TextureFont::AddGlyphGeometry(Graphics::VertexArray *va, const Glyph &glyph, float x, float y, const Color &c)
{
	const float offX = x + float(glyph.offX);
	const float offY = y + GetHeight() - float(glyph.offY);
	const float offU = glyph.offU;
	const float offV = glyph.offV;

	const vector3f p0(offX,             offY,              0.0f);
	const vector3f p1(offX,             offY+glyph.height, 0.0f);
	const vector3f p2(offX+glyph.width, offY,              0.0f);
	const vector3f p3(offX+glyph.width, offY+glyph.height, 0.0f);

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

			const Glyph &glyph = GetGlyph(chr);

			line_width += glyph.advX;

			if (str[i]) {
				Uint32 chr2;
				n = utf8_decode_char(&chr2, &str[i]);
				assert(n);
				line_width += GetKern(glyph, GetGlyph(chr2));
			}
		}
	}

	if (line_width > w) w = line_width;
	h += GetHeight() + GetDescender();
}

void TextureFont::MeasureCharacterPos(const char *str, int charIndex, float &charX, float &charY)
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
			const Glyph &glyph = GetGlyph(chr);
			float advance = glyph.advX;

			if (nextChar != '\n' && nextChar != '\0')
				advance += GetKern(glyph, GetGlyph(nextChar));

			x += advance;
		}

		chr = nextChar;
	}

	charX = x;
	charY = y;
}

int TextureFont::PickCharacter(const char *str, float mouseX, float mouseY)
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
			const Glyph &glyph = GetGlyph(chr1);
			float advance = glyph.advX;

			if (chr2 != '\n' && chr2 != '\0')
				advance += GetKern(glyph, GetGlyph(chr2));

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

			const Glyph &glyph = GetGlyph(chr);
			AddGlyphGeometry(&m_vertices, glyph, roundf(px), py, premult_color);

			if (str[i]) {
				Uint32 chr2;
				n = utf8_decode_char(&chr2, &str[i]);
				assert(n);

				px += GetKern(glyph, GetGlyph(chr2));
			}

			px += glyph.advX;
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
			unsigned hexcol;
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

			const Glyph &glyph = GetGlyph(chr);
			AddGlyphGeometry(&m_vertices, glyph, roundf(px), py, premult_c);

			// XXX kerning doesn't skip markup
			if (str[i]) {
				Uint32 chr2;
				n = utf8_decode_char(&chr2, &str[i]);
				assert(n);

				px += GetKern(glyph, GetGlyph(chr2));
			}

			px += glyph.advX;
		}
	}

	m_renderer->DrawTriangles(&m_vertices, m_renderState, m_mat.get());
	return c;
}

const TextureFont::Glyph &TextureFont::GetGlyph(Uint32 chr)
{
	auto i = m_glyphs.find(chr);
	if (i != m_glyphs.end())
		return (*i).second;

	m_glyphs[chr] = BakeGlyph(chr);
	return m_glyphs[chr];
}


TextureFont::Glyph TextureFont::BakeGlyph(Uint32 chr)
{
	int err;
	Glyph glyph;
	FT_Glyph ftGlyph;

	const FontConfig::Face &face = m_config.GetFaceForCodePoint(chr);
	glyph.ftFace = GetFTFace(face);
	glyph.ftIndex = FT_Get_Char_Index(glyph.ftFace, chr);

	err = FT_Load_Char(glyph.ftFace, chr, FT_LOAD_FORCE_AUTOHINT);
	if (err) {
		Output("Error %d loading glyph\n", err);
		return Glyph();
	}

	// get base glyph again
	err = FT_Get_Glyph(glyph.ftFace->glyph, &ftGlyph);
	if (err) {
		Output("Glyph get error %d\n", err);
		return Glyph();
	}

	// convert to bitmap
	if (ftGlyph->format != FT_GLYPH_FORMAT_BITMAP) {
		err = FT_Glyph_To_Bitmap(&ftGlyph, FT_RENDER_MODE_NORMAL, 0, 1);
		if (err) {
			Output("Couldn't convert glyph to bitmap, error %d\n", err);
			return Glyph();
		}
	}

	const FT_BitmapGlyph bmGlyph = FT_BitmapGlyph(ftGlyph);

	glyph.advX = float(glyph.ftFace->glyph->advance.x) / 64.f + face.advanceXAdjustment;
	glyph.advY = float(glyph.ftFace->glyph->advance.y) / 64.f;

	if (!bmGlyph->bitmap.rows || !bmGlyph->bitmap.width) {
		// no bitmap, we can just return advance metrics (for eg space)
		FT_Done_Glyph(ftGlyph);
		return glyph;
	}

	if (m_config.IsOutline()) {
		FT_Glyph strokeGlyph;

		err = FT_Get_Glyph(glyph.ftFace->glyph, &strokeGlyph);
		if (err) {
			Output("Glyph get error %d\n", err);
			return Glyph();
		}

		err = FT_Glyph_Stroke(&strokeGlyph, m_stroker, 1);
		if (err) {
			Output("Glyph stroke error %d\n", err);
			FT_Done_Glyph(strokeGlyph);
			return Glyph();
		}

		//convert to bitmap
		if (strokeGlyph->format != FT_GLYPH_FORMAT_BITMAP) {
			err = FT_Glyph_To_Bitmap(&strokeGlyph, FT_RENDER_MODE_NORMAL, 0, 1);
			if (err) {
				Output("Couldn't convert glyph to bitmap, error %d\n", err);
				FT_Done_Glyph(strokeGlyph);
				return Glyph();
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
			return Glyph();
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

		glyph.width = bmStrokeGlyph->bitmap.width;
		glyph.height = bmStrokeGlyph->bitmap.rows;
		glyph.offX = bmStrokeGlyph->left;
		glyph.offY = bmStrokeGlyph->top;
		glyph.offU = float(m_atlasU) / float(ATLAS_SIZE);
		glyph.offV = float(m_atlasV) / float(ATLAS_SIZE);
		glyph.texWidth = float(glyph.width) / float(ATLAS_SIZE);
		glyph.texHeight = float(glyph.height) / float(ATLAS_SIZE);

		m_texture->Update(&m_buf[0], vector2f(m_atlasU, m_atlasV), vector2f(glyph.width, glyph.height), m_texFormat);

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
			return Glyph();
		}

		// draw the glyph into the draw buffer
		const int pitch = bmGlyph->bitmap.pitch;
		const int rows = bmGlyph->bitmap.rows;
		m_buf.resize(ALIGN(pitch,4)*rows);
		std::fill(m_buf.begin(), m_buf.end(), 0);
		for (int y = 0; y < rows; y++)
			memcpy(&m_buf[ALIGN(pitch,4)*y], &(bmGlyph->bitmap.buffer[pitch*y]), pitch);

		glyph.width = bmGlyph->bitmap.width;
		glyph.height = bmGlyph->bitmap.rows;
		glyph.offX = bmGlyph->left;
		glyph.offY = bmGlyph->top;
		glyph.offU = float(m_atlasU) / float(ATLAS_SIZE);
		glyph.offV = float(m_atlasV) / float(ATLAS_SIZE);
		glyph.texWidth = float(glyph.width) / float(ATLAS_SIZE);
		glyph.texHeight = float(glyph.height) / float(ATLAS_SIZE);

		m_texture->Update(&m_buf[0], vector2f(m_atlasU, m_atlasV), vector2f(glyph.width, glyph.height), m_texFormat);

		m_atlasU += glyph.width;
	}

	FT_Done_Glyph(ftGlyph);

	return glyph;
}

TextureFont::TextureFont(const FontConfig &config, Graphics::Renderer *renderer, float scale)
	: m_config(config)
	, m_renderer(renderer)
	, m_scale(scale)
	, m_ftLib(nullptr)
	, m_stroker(nullptr)
	, m_vertices(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0)
	, m_atlasU(0)
	, m_atlasV(0)
	, m_atlasVIncrement(0)
{
	FT_Error err; // used to store freetype error return codes

	err = FT_Init_FreeType(&m_ftLib);
	if (err != 0) {
		Output("Couldn't create FreeType library context (%d)\n", err);
		abort();
	}

	if (m_config.IsOutline()) {
		if (FT_Stroker_New(m_ftLib, &m_stroker)) {
			Output("Freetype stroker init error\n");
			abort();
		}

		//1*64 = stroke width
		FT_Stroker_Set(m_stroker, 1*64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	}

	m_texFormat = m_config.IsOutline() ? Graphics::TEXTURE_LUMINANCE_ALPHA_88 : Graphics::TEXTURE_INTENSITY_8;
	m_bpp = m_config.IsOutline() ? 2 : 1;

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

	// font-wide metrics. we assume that these match for all faces
	// XXX loop all the faces and take the min/max as appropriate
	FT_Face ftFace = GetFTFace(m_config.GetFaceForCodePoint(0x20));
	m_height = float(ftFace->height) / 64.f * float(ftFace->size->metrics.y_scale) / 65536.f;
	m_descender = -float(ftFace->descender) / 64.f * float(ftFace->size->metrics.y_scale) / 65536.f;
}

TextureFont::~TextureFont()
{
	for (auto i = m_faces.begin(); i != m_faces.end(); ++i)
		FT_Done_Face((*i).second.first);
	if (m_stroker)
		FT_Stroker_Done(m_stroker);
	FT_Done_FreeType(m_ftLib);
}

FT_Face TextureFont::GetFTFace(const FontConfig::Face &face)
{
	{
	auto i = m_faces.find(face);
	if (i != m_faces.end())
		return (*i).second.first;
	}

	RefCountedPtr<FileSystem::FileData> fd;
	
	fd = FileSystem::gameDataFiles.ReadFile("fonts/" + face.fontFile);
	if (!fd) {
		Output("Terrible error! Couldn't load '%s'.\n", face.fontFile.c_str());
		abort();
	}

	FT_Face ftFace;
	FT_Error err;

	if (0 != (err = FT_New_Memory_Face(m_ftLib,
			reinterpret_cast<const FT_Byte*>(fd->GetData()),
			fd->GetSize(), 0, &ftFace))) {
		Output("Terrible error! Couldn't understand '%s'; error %d.\n", face.fontFile.c_str(), err);
		abort();
	}

	FT_Set_Pixel_Sizes(ftFace, m_scale*face.pixelWidth, m_scale*face.pixelHeight);

	m_faces.insert(std::make_pair(face, std::make_pair(ftFace, fd)));

	return ftFace;
}

float TextureFont::GetKern(const Glyph &a, const Glyph &b)
{
	if (a.ftFace != b.ftFace)
		return 0.0f;

	FT_Vector kern;
	FT_Get_Kerning(a.ftFace, a.ftIndex, b.ftIndex, FT_KERNING_UNFITTED, &kern);
	return float(kern.x) / 64.0f;
}

}
