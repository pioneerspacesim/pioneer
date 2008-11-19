#ifndef _GLFREETYPE_H
#define _GLFREETYPE_H

#include <map>
#include <SDL_stdinc.h>

class FontFace
{
	public:
//	FontFace(FT_Face a_face);
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
void GLFTInit();

#endif /* _GLFREETYPE_H */
