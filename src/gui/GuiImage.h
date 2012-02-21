#ifndef _GUIIMAGE_H
#define _GUIIMAGE_H

#include "GuiWidget.h"
#include <string>

namespace Gui {
	class Texture;

	class Image: public Widget {
	public:
		Image(const char *filename);
		virtual void Draw();
		virtual void GetSizeRequested(float size[2]);
		void SetModulateColor(float r, float g, float b, float a);
	private:
		Texture *m_texture;
		float m_col[4];
	};
}

#endif /* _GUIIMAGE_H */
