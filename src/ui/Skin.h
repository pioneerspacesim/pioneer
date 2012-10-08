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
		RectElement() {}
		RectElement(unsigned int x, unsigned int y, unsigned int w, unsigned int h) : pos(x,y), size(w,h) {}
		Point pos;
		Point size;
	};

	struct BorderedRectElement : public RectElement {
		BorderedRectElement() : borderWidth(0) {}
		BorderedRectElement(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int _borderWidth) : RectElement(x, y, w, h), borderWidth(_borderWidth) {}
		unsigned int borderWidth;
	};

	static BorderedRectElement s_backgroundNormal;
	static BorderedRectElement s_backgroundActive;

	static BorderedRectElement s_buttonDisabled;
	static BorderedRectElement s_buttonNormal;
	static BorderedRectElement s_buttonHover;
	static BorderedRectElement s_buttonActive;

	static RectElement s_checkboxDisabled;
	static RectElement s_checkboxNormal;
	static RectElement s_checkboxHover;
	static RectElement s_checkboxActive;
	static RectElement s_checkboxCheckedDisabled;
	static RectElement s_checkboxCheckedNormal;
	static RectElement s_checkboxCheckedHover;
	static RectElement s_checkboxCheckedActive;

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

	RectElement LoadRectElement(const std::string &spec);
	BorderedRectElement LoadBorderedRectElement(const std::string &spec);
};

}

#endif
