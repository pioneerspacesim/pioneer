#ifndef _GUIGRADIENT_H
#define _GUIGRADIENT_H

#include "GuiWidget.h"
#include "Texture.h"
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
		Gradient(float width, float height, const Color &begin, const Color &end, Direction direction = VERTICAL);

		virtual void Draw();
		virtual void GetSizeRequested(float size[2]);

		Direction GetDirection() const { return m_direction; }
		void SetDirection(Direction dir) { m_direction = dir; }

		void SetStop(const Color& col, float pos) { m_texture.SetStop(col, pos); }

	private:

		class GradientTexture : public Texture {
		public:

			GradientTexture(const Color &begin, const Color &end);

			void SetStop(const Color &col, float pos);

			virtual void Bind() {
				if (m_needGenerate)
					GenerateGradient();
				Texture::Bind();
			}

			void DrawGradientQuad(float w, float h, Direction direction);

		private:
			void GenerateGradient();

			std::map<float, Color> m_stops;
			bool m_needGenerate;
		};

		GradientTexture m_texture;
		Direction m_direction;
	};

}

#endif
