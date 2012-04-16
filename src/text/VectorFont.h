#ifndef _TEXT_VECTORFONT_H
#define _TEXT_VECTORFONT_H

#include "Font.h"
#include <SDL_stdinc.h>
#include <map>

namespace Text {

class VectorFont : public Font
{
public:
	VectorFont(const FontDescriptor &descriptor);
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
	float GetHeight() const { return m_height; }
	float GetWidth() const { return m_width; }
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

}

#endif
