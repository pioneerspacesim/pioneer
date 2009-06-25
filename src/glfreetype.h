#ifndef _GLFREETYPE_H
#define _GLFREETYPE_H

#include <map>
#include <SDL_stdinc.h>

class FontFace
{
	public:
	FontFace(const char *filename_ttf);
	void RenderGlyph(int chr);
	void RenderString(const char *str);
	void RenderMarkup(const char *str);
	void MeasureString(const char *str, float &w, float &h);
	// of Ms
	float GetHeight() { return m_height; }
	float GetWidth() { return m_width; }
	private:
	float m_height;
	float m_width;
	struct glfglyph_t {
		float *varray;
		Uint16 *iarray;
		int numidx;
		float advx, advy;
	};
	std::map<int,glfglyph_t> m_glyphs;
};
class TextureFontFace
{
	public:
	TextureFontFace(const char *filename_ttf, int width, int height);
	void RenderGlyph(int chr, float x, float y);
	void LayoutString(const char *_str, float maxWidth);
	void MeasureLayout(const char *_str, const float maxWidth, float outSize[2]);
	void RenderString(const char *str);
	void RenderMarkup(const char *str);
	void MeasureString(const char *str, float &w, float &h);
	// of Ms
	float GetHeight() { return m_height; }
	float GetWidth() { return m_width; }
	private:
	float m_height;
	float m_width;
	float m_descender;
	struct glfglyph_t {
		unsigned int tex;
		float advx, advy;
		float width, height;
		int offx, offy;
	};
	int m_texSize, m_pixSize;
	std::map<int,glfglyph_t> m_glyphs;
};
void GLFTInit();

#endif /* _GLFREETYPE_H */
