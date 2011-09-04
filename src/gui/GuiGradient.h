#ifndef _GUIGRADIENT_H
#define _GUIGRADIENT_H

#include "GuiWidget.h"
#include <map>

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
		Gradient();
		Gradient(float, float, const Color &begin, const Color &end, Direction dir = VERTICAL);
		~Gradient();
		void SetStop(const Color&, float);
		Direction GetDirection() const { return m_direction; }
		void SetDirection(Direction dir) { m_direction = dir; }
		virtual void Draw();
		virtual void GetSizeRequested(float size[2]);
		GLuint GenerateTexture();
	private:
		GLuint GetTexture();
		std::map<float, Color> m_stops;
		Direction m_direction;
		GLuint m_texture;
	};

}

#endif
