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

#if 0
		class GradientTextureDescriptor : public Graphics::TextureDescriptor {
		public:
			GradientTextureDescriptor(const Color &beginColor, const Color &endColor, Direction direction);

			virtual const Graphics::TextureDescriptor::Data *GetData() const;

			virtual bool Compare(const TextureDescriptor &b) const {
		        if (type != b.type) return TextureDescriptor::Compare(b);
		        const GradientTextureDescriptor &bb = static_cast<const GradientTextureDescriptor&>(b);
		        return (beginColor < bb.beginColor && endColor < bb.endColor && direction < bb.direction);
			}

			virtual GradientTextureDescriptor *Clone() const {
				return new GradientTextureDescriptor(*this);
			}

			const Color beginColor;
			const Color endColor;
			const Direction direction;
		};
#endif

		ScopedPtr<TexturedQuad> m_quad;
	};

}

#endif
