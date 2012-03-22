#ifndef _GUIGRADIENT_H
#define _GUIGRADIENT_H

#include "GuiWidget.h"
#include "graphics/Texture.h"
#include <map>

namespace Graphics { class Renderer; }

namespace Gui {

	/*
	 * Draw a horizontal or vertical gradient.
	 * Can also generate a gradient texture.
	 */
	class Gradient : public Gui::Widget {
	public:
		enum Direction {
			HORIZONTAL,
			VERTICAL
		};

		Gradient(float width, float height, const Color &beginColor, const Color &endColor, Direction direction = VERTICAL);

		virtual void Draw();
		virtual void GetSizeRequested(float size[2]);

	private:
		ScopedPtr<TexturedQuad> m_quad;
	};

}

#endif
