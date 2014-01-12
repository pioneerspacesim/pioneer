// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUIIMAGE_H
#define _GUIIMAGE_H

#include "GuiWidget.h"
#include "GuiTexturedQuad.h"
#include "Color.h"

namespace Gui {
	class Image: public Widget {
	public:
		Image(const char *filename);
		Image(const char *filename, float renderWidth, float renderHeight);
		virtual void Draw();
		virtual void GetSizeRequested(float size[2]);
		void SetModulateColor(const Color &color) { m_color = color; }
		void SetRenderDimensions(const float wide, const float high);
	private:
		void InitTexture(const char* filename);
		std::unique_ptr<TexturedQuad> m_quad;
		Color m_color;
		float m_width, m_height;
	};
}

#endif /* _GUIIMAGE_H */
