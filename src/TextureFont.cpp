#include "TextureFont.h"
#include "gui/GuiScreen.h"
#include "libs.h"

#include FT_GLYPH_H
#include FT_STROKER_H

#define PARAGRAPH_SPACING 1.5f

int TextureFont::s_glyphCount = 0;

void TextureFont::RenderGlyph(Uint32 chr, float x, float y)
{
	glfglyph_t *glyph = &m_glyphs[chr];

	const float offx = x + float(glyph->offx);
	const float offy = y + float(m_pixSize - glyph->offy);

	const float w = m_texSize*glyph->width;
	const float h = m_texSize*glyph->height;

	glyph->texture->DrawUIQuad(offx, offy, w, h, 0, 0, glyph->width, glyph->height);

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

void TextureFont::RenderString(const char *str, float x, float y)
{
	glEnable(GL_BLEND);

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
			if (glyph->texture) RenderGlyph(chr, roundf(px), py);

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

	glDisable(GL_BLEND);
}

void TextureFont::RenderMarkup(const char *str, float x, float y)
{
	glEnable(GL_BLEND);

	float px = x;
	float py = y;

	int i = 0;
	while (str[i]) {
		if (str[i] == '#') {
			int hexcol;
			if (sscanf(str+i, "#%3x", &hexcol)==1) {
				Uint8 col[3];
				col[0] = (hexcol&0xf00)>>4;
				col[1] = (hexcol&0xf0);
				col[2] = (hexcol&0xf)<<4;
				glColor3ubv(col);
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
			if (glyph->texture) RenderGlyph(chr, roundf(px), py);

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

	glDisable(GL_BLEND);
}

TextureFont::TextureFont(FontManager &fm, const FontConfig &fc) : Font(fm, fc)
{
	std::string filename_ttf = GetConfig().String("FontFile");
	if (filename_ttf.length() == 0) {
		fprintf(stderr, "'%s' does not name a FontFile to use\n", GetConfig().GetFilename().c_str());
		abort();
	}

	float scale[2];
	Gui::Screen::GetCoords2Pixels(scale);

	int a_width = int(GetConfig().Int("PixelWidth") / scale[0]);
	int a_height = int(GetConfig().Int("PixelHeight") / scale[1]);

	float advx_adjust = GetConfig().Float("AdvanceXAdjustment");

	int err;
	m_pixSize = a_height;
	if (0 != (err = FT_New_Face(GetFontManager().GetFreeTypeLibrary(), std::string(PIONEER_DATA_DIR "/fonts/" + filename_ttf).c_str(), 0, &m_face))) {
		fprintf(stderr, "Terrible error! Couldn't load '%s'; error %d.\n", filename_ttf.c_str(), err);
		abort();
	}

	FT_Set_Pixel_Sizes(m_face, a_width, a_height);
	int nbit = 0;
	int sz = a_height;
	while (sz) { sz >>= 1; nbit++; }
	sz = (64 > (1<<nbit) ? 64 : (1<<nbit));
	m_texSize = sz;

	unsigned char *pixBuf = new unsigned char[2*sz*sz];
	
	bool outline = GetConfig().Int("Outline");

	FT_Stroker stroker;
	if (outline) {
		if (FT_Stroker_New(GetFontManager().GetFreeTypeLibrary(), &stroker)) {
			fprintf(stderr, "Freetype stroker init error\n");
			abort();
		}

		//1*64 = stroke width
		FT_Stroker_Set(stroker, 1*64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	}

	for (Uint32 chr=0x20; chr<0x1ff; chr++) {
		memset(pixBuf, 0, 2*sz*sz);
	
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

		FT_BitmapGlyph bmGlyph = FT_BitmapGlyph(glyph);

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
	
			FT_BitmapGlyph bmStrokeGlyph = FT_BitmapGlyph(strokeGlyph);
	
			//copy to a square luminance+alpha buffer
			//stroke first
			int pitch = bmStrokeGlyph->bitmap.pitch;
			for (int row=0; row < bmStrokeGlyph->bitmap.rows; row++) {
				for (int col=0; col < bmStrokeGlyph->bitmap.width; col++) {
					//assume black outline
					pixBuf[2*sz*row + 2*col] = 0; //lum
					pixBuf[2*sz*row + 2*col+1] = bmStrokeGlyph->bitmap.buffer[pitch*row + col]; //alpha
				}
			}
	
			//overlay normal glyph (luminance only)
			int xoff = (bmStrokeGlyph->bitmap.width - bmGlyph->bitmap.width) / 2;
			int yoff = (bmStrokeGlyph->bitmap.rows - bmGlyph->bitmap.rows) / 2;
			pitch = bmGlyph->bitmap.pitch;
			for (int row=0; row < bmStrokeGlyph->bitmap.rows; row++) {
				for (int col=0; col < bmStrokeGlyph->bitmap.width; col++) {
					bool over = (row >= bmGlyph->bitmap.rows || col >= bmGlyph->bitmap.width);
					unsigned char value = (over) ? 0 : bmGlyph->bitmap.buffer[pitch*row + col];
					unsigned int idx = 2*sz*(row+yoff) + 2*(col+xoff);
					pixBuf[idx] += value;
					assert(pixBuf[idx] < 256);
				}
			}
	
			glfglyph.width = bmStrokeGlyph->bitmap.width / float(sz);
			glfglyph.height = bmStrokeGlyph->bitmap.rows / float(sz);
			glfglyph.offx = bmStrokeGlyph->left;
			glfglyph.offy = bmStrokeGlyph->top;

			FT_Done_Glyph(strokeGlyph);
		}

		else {
			// face->glyph->bitmap
			// copy to square buffer GL can stomach
			const int pitch = bmGlyph->bitmap.pitch;
			for (int row=0; row < bmGlyph->bitmap.rows; row++) {
				for (int col=0; col < bmGlyph->bitmap.width; col++) {
					pixBuf[2*sz*row + 2*col] = bmGlyph->bitmap.buffer[pitch*row + col];
					pixBuf[2*sz*row + 2*col+1] = bmGlyph->bitmap.buffer[pitch*row + col];
				}
			}
	
			glfglyph.width = bmGlyph->bitmap.width / float(sz);
			glfglyph.height = bmGlyph->bitmap.rows / float(sz);
			glfglyph.offx = bmGlyph->left;
			glfglyph.offy = bmGlyph->top;
		}

		FT_Done_Glyph(glyph);

		glfglyph.texture = new GlyphTexture(pixBuf, sz, sz);

		glfglyph.advx = float(m_face->glyph->advance.x) / 64.0 + advx_adjust;
		glfglyph.advy = float(m_face->glyph->advance.y) / 64.0;
		m_glyphs[chr] = glfglyph;
	}

	delete [] pixBuf;

	if (outline)
		FT_Stroker_Done(stroker);

	m_height = float(a_height);
	m_width = float(a_width);
	m_descender = -float(m_face->descender) / 64.0;
}

TextureFont::~TextureFont()
{
	for (std::map<Uint32,glfglyph_t>::const_iterator i = m_glyphs.begin(); i != m_glyphs.end(); ++i) {
		if ((*i).second.texture)
			delete (*i).second.texture;
	}
}


TextureFont::GlyphTexture::GlyphTexture(Uint8 *data, int width, int height) :
	Texture(GL_TEXTURE_2D, Format(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE), CLAMP, NEAREST, false)
{
	CreateFromArray(data, width, height);
}

void TextureFont::GlyphTexture::Bind()
{
	Texture::Bind();
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}
