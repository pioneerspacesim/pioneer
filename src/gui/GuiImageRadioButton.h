#ifndef _GUIIMAGERADIOBUTTON_H
#define _GUIIMAGERADIOBUTTON_H

#include "GuiRadioButton.h"
#include <string>

class RadioGroup;

namespace Gui {
	class ImageRadioButton: public RadioButton {
	public:
		ImageRadioButton(RadioGroup *, const char *img_normal, const char *img_pressed);
		virtual ~ImageRadioButton();
		virtual void Draw();
		virtual void GetSizeRequested(float size[2]);
		void SetRenderDimensions(const float wide, const float high);
	private:
		Image *m_imgNormal;
		Image *m_imgPressed;
	};
}

#endif /* _GUIIMAGERADIOBUTTON_H */
