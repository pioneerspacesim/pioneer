#include "TextureFont.h"
#include "gui/GuiScreen.h"
#include "libs.h"

#include FT_GLYPH_H
#include FT_STROKER_H

#define PARAGRAPH_SPACING 1.5f

#define TEXTURE_FONT_ENTER \
	glEnable(GL_BLEND); \
	glEnable(GL_TEXTURE_2D); \
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

#define TEXTURE_FONT_LEAVE \
	glDisable(GL_TEXTURE_2D); \
	glDisable(GL_BLEND);

int TextureFont::s_glyphCount = 0;

void TextureFont::RenderGlyph(Uint32 chr, float x, float y)
{
	glfglyph_t *glyph = &m_glyphs[chr];
	glBindTexture(GL_TEXTURE_2D, glyph->tex);
	const float ox = x + float(glyph->offx);
	const float oy = y + float(m_pixSize - glyph->offy);
	glBegin(GL_QUADS);
		float allocSize[2] = { m_texSize*glyph->width, m_texSize*glyph->height };
		const float w = glyph->width;
		const float h = glyph->height;
		glTexCoord2f(0,h);
		glVertex2f(ox,oy+allocSize[1]);
		glTexCoord2f(w,h);
		glVertex2f(ox+allocSize[0],oy+allocSize[1]);
		glTexCoord2f(w,0);
		glVertex2f(ox+allocSize[0],oy);
		glTexCoord2f(0,0);
		glVertex2f(ox,oy);
	glEnd();

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

void TextureFont::RenderString(const char *str, float x, float y)
{
	TEXTURE_FONT_ENTER;

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
			if (glyph->tex) RenderGlyph(chr, roundf(px), py);

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
	TEXTURE_FONT_LEAVE;
}

void TextureFont::RenderMarkup(const char *str, float x, float y)
{
	TEXTURE_FONT_ENTER;

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
			if (glyph->tex) RenderGlyph(chr, roundf(px), py);

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

	TEXTURE_FONT_LEAVE;
}

TextureFont::TextureFont(FontManager &fm, const std::string &config_filename) : Font(fm, config_filename)
{
	std::string filename_ttf = GetConfig().String("FontFile");
	if (filename_ttf.length() == 0) {
		fprintf(stderr, "'%s' does not name a FontFile to use\n", config_filename.c_str());
		abort();
	}

	float scale[2];
	Gui::Screen::GetCoords2Pixels(scale);

	int a_width = GetConfig().Int("PixelWidth") / scale[0];
	int a_height = GetConfig().Int("PixelHeight") / scale[1];

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

	for (Uint32 chr=32; chr<255; chr++) {
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

		glEnable (GL_TEXTURE_2D);
		glGenTextures (1, &glfglyph.tex);
		glBindTexture (GL_TEXTURE_2D, glfglyph.tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, sz, sz, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, pixBuf);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glDisable (GL_TEXTURE_2D);

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

