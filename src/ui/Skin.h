#ifndef _UI_SKIN_H
#define _UI_SKIN_H

#include "libs.h"
#include "SmartPtr.h"
#include "graphics/Renderer.h"
#include "graphics/Material.h"
#include "Point.h"

namespace UI {

class Skin {
public:
	Skin(const std::string &filename, Graphics::Renderer *renderer);

	void DrawBackgroundNormal(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(s_backgroundNormal, pos, size);
	}
	void DrawBackgroundActive(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(s_backgroundActive, pos, size);
	}

	void DrawButtonNormal(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(s_buttonNormal, pos, size);
	}
	void DrawButtonActive(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(s_buttonActive, pos, size);
	}

	void DrawCheckBoxNormal(const Point &pos, const Point &size) const {
		DrawRectElement(s_checkboxNormal, pos, size);
	}
	void DrawCheckBoxChecked(const Point &pos, const Point &size) const {
		DrawRectElement(s_checkboxChecked, pos, size);
	}

#if 0
	void DrawHSlider(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(s_hSlider, pos, size);
	}
#endif

	struct RectElement {
		RectElement(unsigned int x, unsigned int y, unsigned int w, unsigned int h) : pos(x,y), size(w,h) {}
		const Point pos;
		const Point size;
	};

	struct BorderedRectElement : public RectElement {
		BorderedRectElement(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int _borderWidth) : RectElement(x, y, w, h), borderWidth(_borderWidth) {}
		const unsigned int borderWidth;
	};

	static const BorderedRectElement s_backgroundNormal;
	static const BorderedRectElement s_backgroundActive;

	static const BorderedRectElement s_buttonNormal;
	static const BorderedRectElement s_buttonActive;

	static const RectElement s_checkboxNormal;
	static const RectElement s_checkboxChecked;

private:
	Graphics::Renderer *m_renderer;
	RefCountedPtr<Graphics::Texture> m_texture;
	RefCountedPtr<Graphics::Material> m_material;

	void DrawRectElement(const RectElement &element, const Point &pos, const Point &size) const;
	void DrawBorderedRectElement(const BorderedRectElement &element, const Point &pos, const Point &size) const;
};

}

#endif
