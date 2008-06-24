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
	private:
		GLuint m_tex;
		int m_imgw, m_imgh;
		float m_invtexw, m_invtexh;
	};
}

#endif /* _GUIIMAGE_H */
