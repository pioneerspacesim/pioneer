// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _UI_SKIN_H
#define _UI_SKIN_H

#include "libs.h"
#include "SmartPtr.h"
#include "graphics/Renderer.h"
#include "graphics/Material.h"
#include "Point.h"
#include "IniConfig.h"

namespace UI {

class Skin {
public:
	Skin(const std::string &filename, Graphics::Renderer *renderer);

	void DrawBackgroundNormal(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(s_backgroundNormal, pos, size);
	}
	void DrawBackgroundHover(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(s_backgroundActive, pos, size);
	}
	void DrawBackgroundActive(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(s_backgroundActive, pos, size);
	}

	void DrawButtonDisabled(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(s_buttonDisabled, pos, size);
	}
	void DrawButtonNormal(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(s_buttonNormal, pos, size);
	}
	void DrawButtonHover(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(s_buttonHover, pos, size);
	}
	void DrawButtonActive(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(s_buttonActive, pos, size);
	}

	void DrawCheckBoxDisabled(const Point &pos, const Point &size) const {
		DrawRectElement(s_checkboxDisabled, pos, size);
	}
	void DrawCheckBoxNormal(const Point &pos, const Point &size) const {
		DrawRectElement(s_checkboxNormal, pos, size);
	}
	void DrawCheckBoxHover(const Point &pos, const Point &size) const {
		DrawRectElement(s_checkboxHover, pos, size);
	}
	void DrawCheckBoxActive(const Point &pos, const Point &size) const {
		DrawRectElement(s_checkboxActive, pos, size);
	}
	void DrawCheckBoxCheckedDisabled(const Point &pos, const Point &size) const {
		DrawRectElement(s_checkboxCheckedDisabled, pos, size);
	}
	void DrawCheckBoxCheckedNormal(const Point &pos, const Point &size) const {
		DrawRectElement(s_checkboxCheckedNormal, pos, size);
	}
	void DrawCheckBoxCheckedHover(const Point &pos, const Point &size) const {
		DrawRectElement(s_checkboxCheckedHover, pos, size);
	}
	void DrawCheckBoxCheckedActive(const Point &pos, const Point &size) const {
		DrawRectElement(s_checkboxCheckedActive, pos, size);
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

	static const BorderedRectElement s_buttonDisabled;
	static const BorderedRectElement s_buttonNormal;
	static const BorderedRectElement s_buttonHover;
	static const BorderedRectElement s_buttonActive;

	static const RectElement s_checkboxDisabled;
	static const RectElement s_checkboxNormal;
	static const RectElement s_checkboxHover;
	static const RectElement s_checkboxActive;
	static const RectElement s_checkboxCheckedDisabled;
	static const RectElement s_checkboxCheckedNormal;
	static const RectElement s_checkboxCheckedHover;
	static const RectElement s_checkboxCheckedActive;

private:
	class Config : public IniConfig {
	public:
		Config(const std::string &filename);
	};

	Config m_config;

	Graphics::Renderer *m_renderer;
	RefCountedPtr<Graphics::Texture> m_texture;
	RefCountedPtr<Graphics::Material> m_material;

	void DrawRectElement(const RectElement &element, const Point &pos, const Point &size) const;
	void DrawBorderedRectElement(const BorderedRectElement &element, const Point &pos, const Point &size) const;
};

}

#endif
