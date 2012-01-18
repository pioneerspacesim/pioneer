#ifndef _GUIIMAGE_H
#define _GUIIMAGE_H

#include "GuiWidget.h"
#include <string>

class UITexture;

namespace Gui {
	class Image: public Widget {
	public:
		Image(const char *filename);
		virtual void Draw();
		virtual void GetSizeRequested(float size[2]);
		void SetModulateColor(float r, float g, float b, float a);
	private:
		UITexture *m_texture;
		float m_col[4];
	};
}

#endif /* _GUIIMAGE_H */
