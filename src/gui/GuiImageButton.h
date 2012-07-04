#ifndef _GUIIMAGEBUTTON_H
#define _GUIIMAGEBUTTON_H

#include "GuiWidget.h"
#include "GuiButton.h"

#include <string>

namespace Gui {
	class ImageButton: public Button {
	public:
		ImageButton(const char *img_normal);
		ImageButton(const char *img_normal, const char *img_pressed);
		virtual void Draw();
		virtual ~ImageButton();
		virtual void GetSizeRequested(float size[2]);
	private:
		void LoadImages(const char *img_normal, const char *img_pressed);
		Image *m_imgNormal;
		Image *m_imgPressed;
	};
}

#endif /* _GUIIMAGEBUTTON_H */
