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

		Gradient(float width, float height, const Color &begin, const Color &end, Direction direction = VERTICAL);

		virtual void Draw();
		virtual void GetSizeRequested(float size[2]);

	private:

		class GradientTextureDescriptor : public Graphics::TextureDescriptor {
		public:
			GradientTextureDescriptor(const Color &beginColor, const Color &endColor, Direction direction);

			virtual const Graphics::TextureDescriptor::Data *GetData() const;

			virtual bool IsEqual(const TextureDescriptor &b) const {
				if (!TextureDescriptor::IsEqual(b)) return false;
				const GradientTextureDescriptor *bb = dynamic_cast<const GradientTextureDescriptor*>(&b);
				return (bb && bb->direction == direction && bb->beginColor == beginColor && bb->endColor == endColor);
			}

			virtual GradientTextureDescriptor *Clone() const {
				return new GradientTextureDescriptor(*this);
			}

			const Color beginColor;
			const Color endColor;
			const Direction direction;
		};

		ScopedPtr<TexturedQuad> m_quad;
	};

}

#endif
