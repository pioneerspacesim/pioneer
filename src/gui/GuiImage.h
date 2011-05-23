#ifndef _GUIIMAGE_H
#define _GUIIMAGE_H

#include "GuiWidget.h"
#include <string>

namespace Gui {
	class Image: public Widget {
	public:
		Image(const char *filename);
		virtual void Draw();
		virtual ~Image();
		virtual void GetSizeRequested(float size[2]);
		void SetModulateColor(float r, float g, float b, float a);
	private:
		GLuint m_tex;
		int m_imgw, m_imgh;
		float m_invtexw, m_invtexh;
		float m_col[4];
	};
}

#endif /* _GUIIMAGE_H */
