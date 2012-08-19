#ifndef _GUIIMAGE_H
#define _GUIIMAGE_H

#include "GuiWidget.h"
#include "GuiTexturedQuad.h"
#include "Color.h"

namespace Gui {
	class Image: public Widget {
	public:
		Image(const char *filename);
		virtual void Draw();
		virtual void GetSizeRequested(float size[2]);
		void SetModulateColor(const Color &color) { m_color = color; }
		void SetRenderDimensions(const float wide, const float high);
	private:
		ScopedPtr<TexturedQuad> m_quad;
		Color m_color;
		float m_width, m_height;
	};
}

#endif /* _GUIIMAGE_H */
