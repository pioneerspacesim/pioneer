#include "Gui.h"

#define LINE_SPACING      1.25f
#define PARAGRAPH_SPACING 1.75f

namespace Gui {

static double _clip[2][4];
static vector3d _clipoffset;
static bool _do_clip;

static void init_clip_test()
{
	matrix4x4d m;
	if (glIsEnabled(GL_CLIP_PLANE1)) {
		glGetClipPlane(GL_CLIP_PLANE1, _clip[0]);
		glGetClipPlane(GL_CLIP_PLANE3, _clip[1]);
		
		glGetDoublev (GL_MODELVIEW_MATRIX, &m[0]);
		_clipoffset.x = m[12];
		_clipoffset.y = m[13];
		_clipoffset.z = 0;

		_do_clip = true;
	} else {
		_do_clip = false;
	}
}

/* does a line of text pass top and bottom clip planes? */
static bool line_clip_test(float topy, float bottomy)
{
	if (!_do_clip) return true;
	topy = _clipoffset.y + topy*Gui::Screen::GetCoords2Pixels()[1];
	bottomy = _clipoffset.y + bottomy*Gui::Screen::GetCoords2Pixels()[1];

	if ((bottomy*_clip[0][1] + _clip[0][3] > 0) &&
	    (topy*_clip[1][1] + _clip[1][3] > 0)) return true;
	return false;
}

TextLayout::TextLayout(const char *_str, RefCountedPtr<TextureFont> font, ColourMarkupMode markup)
{
	// XXX ColourMarkupSkip not correctly implemented yet
	assert(markup != ColourMarkupSkip);

	m_colourMarkup = markup;
	m_font = font ? font : Gui::Screen::GetFont();

	str = reinterpret_cast<char *>(malloc(strlen(_str)+1));
	strcpy(str, _str);

	m_justify = false;
	float wordWidth = 0;
	char *wordstart = str;

	int i = 0;
	while (str[i]) {
		wordWidth = 0;
		wordstart = &str[i];

		while (str[i] && str[i] != ' ' && str[i] != '\r' && str[i] != '\n') {
			/* skip color control code things! */
			if ((markup != ColourMarkupNone) && (str[i] == '#')) {
				unsigned int hexcol;
				if (sscanf(&str[i], "#%3x", &hexcol)==1) {
					i+=4;
					continue;
				}
			}

			Uint32 chr;
			int n = conv_mb_to_wc(&chr, &str[i]);
			assert(n);
			i += n;

			const TextureFont::glfglyph_t &glyph = m_font->GetGlyph(chr);
			wordWidth += glyph.advx;

			// XXX this should do kerning
		}

		words.push_back(word_t(wordstart, wordWidth));

		if (str[i]) {
			if (str[i] == '\n') words.push_back(word_t(0,0));
			str[i++] = 0;
		}
	}
}

void TextLayout::MeasureSize(const float width, float outSize[2]) const
{
	float fontScale[2];
	Gui::Screen::GetCoords2Pixels(fontScale);
	_MeasureSizeRaw(width / fontScale[0], outSize);
	outSize[0] = ceil(outSize[0] * fontScale[0]);
	outSize[1] = ceil(outSize[1] * fontScale[1]);
}

void TextLayout::Render(const float width) const
{
	float fontScale[2];
	Gui::Screen::GetCoords2Pixels(fontScale);
	GLdouble modelMatrix[16];
	glPushMatrix();
	glGetDoublev (GL_MODELVIEW_MATRIX, modelMatrix);
	float x = modelMatrix[12];
	float y = modelMatrix[13];
	glLoadIdentity();
	glTranslatef(floor(x/fontScale[0])*fontScale[0],
			floor(y/fontScale[1])*fontScale[1], 0);
	glScalef(fontScale[0], fontScale[1], 1);
	_RenderRaw(width / fontScale[0]);
	glPopMatrix();
}

void TextLayout::_RenderRaw(float maxWidth) const
{
	float py = 0;
	init_clip_test();

	glPushMatrix();

	const float spaceWidth = m_font->GetGlyph(' ').advx;

	std::list<word_t>::const_iterator wpos = this->words.begin();
	// build lines of text
	while (wpos != this->words.end()) {
		float len = 0;
		int num = 0;

		std::list<word_t>::const_iterator i = wpos;
		len += (*i).advx;
		num++;
		bool overflow = false;
		bool explicit_newline = false;
		if ((*i).word != 0) {
			++i;
			for (; i != this->words.end(); ++i) {
				if ((*i).word == 0) {
					// newline
					explicit_newline = true;
					num++;
					break;
				}
				if (len + spaceWidth + (*i).advx > maxWidth) { overflow = true; break; }
				len += (*i).advx + spaceWidth;
				num++;
			}
		}

		float _spaceWidth;
		if ((m_justify) && (num>1) && overflow) {
			float spaceleft = maxWidth - len;
			_spaceWidth = spaceWidth + (spaceleft/float(num-1));
		} else {
			_spaceWidth = spaceWidth;
		}

		if (line_clip_test(py, py+m_font->GetHeight()*2.0)) {
			float px = 0;
			for (int j=0; j<num; j++) {
				if ((*wpos).word) {
					if (m_colourMarkup == ColourMarkupUse)
						m_font->RenderMarkup((*wpos).word, round(px), round(py));
					else
						m_font->RenderString((*wpos).word, round(px), round(py));
				}
				px += (*wpos).advx + _spaceWidth;
				wpos++;
			}
		} else {
			for (int j=0; j<num; j++) wpos++;
		}
		py += m_font->GetHeight() * (explicit_newline ? PARAGRAPH_SPACING : LINE_SPACING);
	}
	glPopMatrix();
}

void TextLayout::_MeasureSizeRaw(const float layoutWidth, float outSize[2]) const
{
	outSize[0] = 0;
	outSize[1] = 0;

	const float spaceWidth = m_font->GetGlyph(' ').advx;

	// build lines of text
	for (std::list<word_t>::const_iterator wpos = words.begin(); wpos != words.end(); ) {
		float len = 0;
		int num = 0;
		bool explicit_newline = false;

		std::list<word_t>::const_iterator i = wpos;
		len += (*i).advx;
		num++;
		bool overflow = false;
		if ((*i).word != 0) {
			++i;
			for (; i != words.end(); ++i) {
				if ((*i).word == 0) {
					// newline
					explicit_newline = true;
					num++;
					break;
				}
				if (len + spaceWidth + (*i).advx > layoutWidth) { overflow = true; break; }
				len += (*i).advx + spaceWidth;
				num++;
			}
		}

		float _spaceWidth;
		if ((m_justify) && (num>1) && overflow) {
			float spaceleft = layoutWidth - len;
			_spaceWidth = spaceWidth + (spaceleft/float(num-1));
		} else {
			_spaceWidth = spaceWidth;
		}

		float lineLen = 0;
		for (int j=0; j<num; j++) {
			word_t word = (*wpos);
			lineLen += word.advx;
			if (j < num-1) lineLen += _spaceWidth;
			wpos++;
		}
		if (lineLen > outSize[0]) outSize[0] = lineLen;
		outSize[1] += m_font->GetHeight() * (explicit_newline ? PARAGRAPH_SPACING : LINE_SPACING);
	}
	if (outSize[1] > 0.0f)
		outSize[1] += m_font->GetDescender();
}

}
