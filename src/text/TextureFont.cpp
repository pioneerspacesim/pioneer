// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TextureFont.h"
#include "libs.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "TextSupport.h"
#include "utils.h"
#include <algorithm>

#define DUMP_GLYPH_ATLAS 0
#if DUMP_GLYPH_ATLAS
#include "PngWriter.h"
#include "FileSystem.h"
#include "StringF.h"
#endif

#include FT_GLYPH_H
#include FT_STROKER_H

static const int FONT_TEXTURE_WIDTH = 1024;
static const int FONT_TEXTURE_MAX_HEIGHT = 1024;

#if DUMP_GLYPH_ATLAS
static std::string atlas_image_name(const Text::FontDescriptor &desc)
{
	const int font_size = desc.pixelHeight ? desc.pixelHeight : int(desc.pointSize);
	return stringf("font-atlas-%0%1-%2.png", desc.filename, (desc.outline ? "-outline" : ""), font_size);
}
#endif

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
	m_renderer->SetBlendMode(Graphics::BLEND_ALPHA_PREMULT);
	m_vertices.Clear();

	const Color premult_color = Color(color.r * color.a, color.g * color.a, color.b * color.a, color.a);

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

	m_renderer->DrawTriangles(&m_vertices, m_mat.Get());
}

Color TextureFont::RenderMarkup(const char *str, float x, float y, const Color &color)
{
	m_renderer->SetBlendMode(Graphics::BLEND_ALPHA_PREMULT);
	m_vertices.Clear();

	float px = x;
	float py = y;

	Color c = color;
	Color premult_c = Color(c.r*c.a, c.g*c.a, c.b*c.a, c.a);

	int i = 0;
	while (str[i]) {
		if (str[i] == '#') {
			int hexcol;
			if (sscanf(str+i, "#%3x", &hexcol)==1) {
				c.r = float((hexcol&0xf00)>>4)/255.0f;
				c.g = float((hexcol&0xf0))/255.0f;
				c.b = float((hexcol&0xf)<<4)/255.0f;
				// retain alpha value from RenderMarkup color parameter
				premult_c.r = c.r * c.a;
				premult_c.g = c.g * c.a;
				premult_c.b = c.b * c.a;
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

	m_renderer->DrawTriangles(&m_vertices, m_mat.Get());
	return c;
}

TextureFont::TextureFont(const FontDescriptor &descriptor, Graphics::Renderer *renderer)
	: Font(descriptor)
	, m_renderer(renderer)
	, m_vertices(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0)
	, m_glyphsFast(MAX_FAST_GLYPHS)
{
	memset(&m_glyphsFast[0], 0, sizeof(glfglyph_t)*MAX_FAST_GLYPHS);

	int err; // used to store freetype error return codes
	const int a_width = GetDescriptor().pixelWidth;
	const int a_height = GetDescriptor().pixelHeight;

	const float advx_adjust = GetDescriptor().advanceXAdjustment;

	FT_Set_Pixel_Sizes(m_face, a_width, a_height);

	// UV offsets for glyphs
	int atlasU = 0;
	int atlasV = 0;
	int atlasVIncrement = 0;

	const bool outline = GetDescriptor().outline;

	// temporary pixel buffer for the glyph atlas
	const Graphics::TextureFormat tex_format = outline ? Graphics::TEXTURE_LUMINANCE_ALPHA_88 : Graphics::TEXTURE_INTENSITY_8;
	const int tex_bpp = outline ? 2 : 1;
	std::vector<unsigned char> pixBuf(tex_bpp * FONT_TEXTURE_WIDTH * FONT_TEXTURE_MAX_HEIGHT);
	std::fill(pixBuf.begin(), pixBuf.end(), 0);

	FT_Stroker stroker(0);
	if (outline) {
		if (FT_Stroker_New(GetFreeTypeLibrary(), &stroker)) {
			fprintf(stderr, "Freetype stroker init error\n");
			abort();
		}

		//1*64 = stroke width
		FT_Stroker_Set(stroker, 1*64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	}

	// generate and store glyphs for each character in the specified ranges
	for (int i = 0;; i += 2) {
		Uint32 first = CHARACTER_RANGES[i];
		Uint32 last = CHARACTER_RANGES[i+1];
		if (first == 0 && last == 0)
			break;

		for (Uint32 chr = first; chr <= last; chr++) {
			glfglyph_t glfglyph;
			FT_Glyph glyph;

			glfglyph.ftIndex = FT_Get_Char_Index(m_face, chr);

			err = FT_Load_Char(m_face, chr, FT_LOAD_FORCE_AUTOHINT);
			if (err) {
				fprintf(stderr, "Error %d loading glyph\n", err);
				continue;
			}

			// get base glyph again
			err = FT_Get_Glyph(m_face->glyph, &glyph);
			if (err) {
				fprintf(stderr, "Glyph get error %d\n", err);
				continue;
			}

			// convert to bitmap
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
					FT_Done_Glyph(strokeGlyph);
					continue;
				}

				//convert to bitmap
				if (strokeGlyph->format != FT_GLYPH_FORMAT_BITMAP) {
					err = FT_Glyph_To_Bitmap(&strokeGlyph, FT_RENDER_MODE_NORMAL, 0, 1);
					if (err) {
						fprintf(stderr, "Couldn't convert glyph to bitmap, error %d\n", err);
						FT_Done_Glyph(strokeGlyph);
						continue;
					}
				}

				const FT_BitmapGlyph bmStrokeGlyph = FT_BitmapGlyph(strokeGlyph);

				//don't run off atlas borders
				atlasVIncrement = std::max(atlasVIncrement, bmStrokeGlyph->bitmap.rows);
				if (atlasU + bmStrokeGlyph->bitmap.width > FONT_TEXTURE_WIDTH) {
					atlasU = 0;
					atlasV += atlasVIncrement;
				}

				if (atlasV + bmStrokeGlyph->bitmap.rows > FONT_TEXTURE_MAX_HEIGHT) {
					char utf8buf[8];
					int len = utf8_encode_char(chr, utf8buf);
					utf8buf[len] = '\0';
					fprintf(stderr, "glyph doesn't fit in atlas (U+%04X; height = %d; char: %s; atlasV = %d)\n", chr, bmStrokeGlyph->bitmap.rows, utf8buf, atlasV);
					FT_Done_Glyph(strokeGlyph);
					continue;
				}

				assert(tex_bpp == 2);
				assert(tex_format == Graphics::TEXTURE_LUMINANCE_ALPHA_88);

				//copy to the atlas texture
				//stroke first
				int pitch = bmStrokeGlyph->bitmap.pitch;
				for (int row=0; row < bmStrokeGlyph->bitmap.rows; row++) {
					for (int col=0; col < bmStrokeGlyph->bitmap.width; col++) {
						//assume black outline
						const int d = 2*FONT_TEXTURE_WIDTH*(row+atlasV) + 2*(col+atlasU);
						const int s = pitch*row + col;
						pixBuf[d+0] = 0; // luminance
						pixBuf[d+1] = bmStrokeGlyph->bitmap.buffer[s]; // alpha
					}
				}

				//overlay normal glyph (luminance only)
				int xoff = (bmStrokeGlyph->bitmap.width - bmGlyph->bitmap.width) / 2;
				int yoff = (bmStrokeGlyph->bitmap.rows - bmGlyph->bitmap.rows) / 2;
				pitch = bmGlyph->bitmap.pitch;
				for (int row=0; row < bmGlyph->bitmap.rows; row++) {
					for (int col=0; col < bmGlyph->bitmap.width; col++) {
						const int d = 2*FONT_TEXTURE_WIDTH*(row+atlasV+xoff) + 2*(col+atlasU+yoff);
						const int s = pitch*row + col;
						pixBuf[d] = bmGlyph->bitmap.buffer[s]; // luminance
					}
				}

				glfglyph.width = bmStrokeGlyph->bitmap.width;
				glfglyph.height = bmStrokeGlyph->bitmap.rows;
				glfglyph.offx = bmStrokeGlyph->left;
				glfglyph.offy = bmStrokeGlyph->top;
				glfglyph.offU = atlasU;
				glfglyph.offV = atlasV;

				atlasU += bmStrokeGlyph->bitmap.width;

				FT_Done_Glyph(strokeGlyph);
			}

			else {
				//don't run off atlas borders
				atlasVIncrement = std::max(atlasVIncrement, bmGlyph->bitmap.rows);
				if (atlasU + bmGlyph->bitmap.width >= FONT_TEXTURE_WIDTH) {
					atlasU = 0;
					atlasV += atlasVIncrement;
				}

				if (atlasV + bmGlyph->bitmap.rows > FONT_TEXTURE_MAX_HEIGHT) {
					char utf8buf[8];
					int len = utf8_encode_char(chr, utf8buf);
					utf8buf[len] = '\0';
					fprintf(stderr, "glyph doesn't fit in atlas (U+%04X; height = %d; char: %s)\n", chr, bmGlyph->bitmap.rows, utf8buf);
					continue;
				}

				assert(tex_bpp == 1);
				assert(tex_format == Graphics::TEXTURE_INTENSITY_8);

				//copy glyph bitmap to the atlas texture
				//the glyphs are upside down in the texture due to how freetype stores them
				//but it's just a matter of adjusting the texcoords
				const int pitch = bmGlyph->bitmap.pitch;
				const int rows = bmGlyph->bitmap.rows;
				for (int row=0; row < rows; row++) {
					for (int col=0; col < bmGlyph->bitmap.width; col++) {
						const int d = FONT_TEXTURE_WIDTH*(row+atlasV) + (col+atlasU);
						const int s = pitch*row + col;
						pixBuf[d] = bmGlyph->bitmap.buffer[s]; // alpha
					}
				}

				glfglyph.width = bmGlyph->bitmap.width;
				glfglyph.height = bmGlyph->bitmap.rows;
				glfglyph.offx = bmGlyph->left;
				glfglyph.offy = bmGlyph->top;
				glfglyph.offU = atlasU;
				glfglyph.offV = atlasV;

				atlasU += bmGlyph->bitmap.width;
			}

			FT_Done_Glyph(glyph);

			glfglyph.advx = float(m_face->glyph->advance.x) / 64.f + advx_adjust;
			glfglyph.advy = float(m_face->glyph->advance.y) / 64.f;

			if (chr < MAX_FAST_GLYPHS)
				m_glyphsFast[chr] = glfglyph;
			else
				m_glyphs[chr] = glfglyph;
		}
	}

	// we may not have used the whole texture;
	// pick the smallest power-of-two texture height that can hold all the glyphs we've got
	const int used_height = atlasV + atlasVIncrement;
	const int tex_height = ceil_pow2(used_height);
	const vector2f tex_size(FONT_TEXTURE_WIDTH, tex_height);

	// fill in glyph metrics
	const float inv_width = 1.0f / tex_size.x;
	const float inv_height = 1.0f / tex_size.y;
	for (int i = 0; i < MAX_FAST_GLYPHS; ++i)
	{
		glfglyph_t &metrics = m_glyphsFast[i];
		if (!is_zero_exact(metrics.width))
		{
			metrics.texWidth = metrics.width * inv_width;
			metrics.texHeight = metrics.height * inv_height;
			metrics.offU *= inv_width;
			metrics.offV *= inv_height;
		}
	}
	for (std::map<Uint32,glfglyph_t>::iterator it = m_glyphs.begin(); it != m_glyphs.end(); ++it)
	{
		glfglyph_t &metrics = it->second;
		if (!is_zero_exact(metrics.width))
		{
			metrics.texWidth = metrics.width * inv_width;
			metrics.texHeight = metrics.height * inv_height;
			metrics.offU *= inv_width;
			metrics.offV *= inv_height;
		}
	}

	Graphics::MaterialDescriptor desc;
	desc.vertexColors = true; //to allow per-character colors
	desc.textures = 1;
	m_mat.Reset(m_renderer->CreateMaterial(desc));
	Graphics::TextureDescriptor textureDescriptor(tex_format, tex_size, Graphics::NEAREST_CLAMP, false, false);
	m_texture.Reset(m_renderer->CreateTexture(textureDescriptor));
	m_mat->texture0 = m_texture.Get();

#if DUMP_GLYPH_ATLAS
	const std::string name = atlas_image_name(GetDescriptor());
	write_png(FileSystem::userFiles, name.c_str(),
			&pixBuf[0], FONT_TEXTURE_WIDTH, tex_height, FONT_TEXTURE_WIDTH*tex_bpp, tex_bpp);
	printf("Font atlas written to '%s'\n", name.c_str());
#endif

	//upload atlas
	m_texture->Update(&pixBuf[0], tex_size, tex_format);

	if (outline)
		FT_Stroker_Done(stroker);

	m_height = float(m_face->height) / 64.f * float(m_face->size->metrics.y_scale) / 65536.f;
	m_descender = -float(m_face->descender) / 64.f * float(m_face->size->metrics.y_scale) / 65536.f;
}

}
