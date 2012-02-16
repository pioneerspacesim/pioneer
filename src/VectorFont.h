#ifndef _VECTORFONT_H
#define _VECTORFONT_H

#include "Font.h"
#include <SDL_stdinc.h>

class VectorFont : public Font
{
public:
	VectorFont(const FontConfig &fc);
	virtual ~VectorFont();

	void RenderGlyph(int chr);
	void RenderString(const char *str);
	void RenderMarkup(const char *str);
	void MeasureString(const char *str, float &w, float &h);
	//void RenderGlyph(int chr);
	void GetStringGeometry(const char *str,
			void (*index_callback)(int num, Uint16 *vals),
			void (*vertex_callback)(int num, float offsetX, float offsetY, float *vals));
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
		int numvtx;
		float advx, advy;
	};
	std::map<int,glfglyph_t> m_glyphs;
};

#endif
