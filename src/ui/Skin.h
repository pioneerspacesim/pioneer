// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_SKIN_H
#define UI_SKIN_H

#include "libs.h"
#include "SmartPtr.h"
#include "graphics/Renderer.h"
#include "graphics/Material.h"
#include "Point.h"

namespace UI {

class Skin {
public:
	Skin(const std::string &filename, Graphics::Renderer *renderer, float scale);

	void DrawBackgroundNormal(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(m_backgroundNormal, pos, size);
	}
	void DrawBackgroundHover(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(m_backgroundActive, pos, size);
	}
	void DrawBackgroundActive(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(m_backgroundActive, pos, size);
	}

	void DrawButtonDisabled(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(m_buttonDisabled, pos, size);
	}
	void DrawButtonNormal(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(m_buttonNormal, pos, size);
	}
	void DrawButtonHover(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(m_buttonHover, pos, size);
	}
	void DrawButtonActive(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(m_buttonActive, pos, size);
	}

	void DrawCheckBoxDisabled(const Point &pos, const Point &size) const {
		DrawRectElement(m_checkboxDisabled, pos, size);
	}
	void DrawCheckBoxNormal(const Point &pos, const Point &size) const {
		DrawRectElement(m_checkboxNormal, pos, size);
	}
	void DrawCheckBoxHover(const Point &pos, const Point &size) const {
		DrawRectElement(m_checkboxHover, pos, size);
	}
	void DrawCheckBoxActive(const Point &pos, const Point &size) const {
		DrawRectElement(m_checkboxActive, pos, size);
	}
	void DrawCheckBoxCheckedDisabled(const Point &pos, const Point &size) const {
		DrawRectElement(m_checkboxCheckedDisabled, pos, size);
	}
	void DrawCheckBoxCheckedNormal(const Point &pos, const Point &size) const {
		DrawRectElement(m_checkboxCheckedNormal, pos, size);
	}
	void DrawCheckBoxCheckedHover(const Point &pos, const Point &size) const {
		DrawRectElement(m_checkboxCheckedHover, pos, size);
	}
	void DrawCheckBoxCheckedActive(const Point &pos, const Point &size) const {
		DrawRectElement(m_checkboxCheckedActive, pos, size);
	}

#if 0
	void DrawHSlider(const Point &pos, const Point &size) const {
		DrawBorderedRectElement(m_hSlider, pos, size);
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

	const BorderedRectElement &BackgroundNormal() const { return m_backgroundNormal; }
	const BorderedRectElement &BackgroundActive() const { return m_backgroundActive; }

	const BorderedRectElement &ButtonDisabled() const { return m_buttonDisabled; }
	const BorderedRectElement &ButtonNormal()   const { return m_buttonNormal; }
	const BorderedRectElement &ButtonHover()    const { return m_buttonHover; }
	const BorderedRectElement &ButtonActive()   const { return m_buttonActive; }

	const RectElement &CheckboxDisabled()        const { return m_checkboxDisabled; }
	const RectElement &CheckboxNormal()          const { return m_checkboxNormal; }
	const RectElement &CheckboxHover()           const { return m_checkboxHover; }
	const RectElement &CheckboxActive()          const { return m_checkboxActive; }
	const RectElement &CheckboxCheckedDisabled() const { return m_checkboxCheckedDisabled; }
	const RectElement &CheckboxCheckedNormal()   const { return m_checkboxCheckedNormal; }
	const RectElement &CheckboxCheckedHover()    const { return m_checkboxCheckedHover; }
	const RectElement &CheckboxCheckedActive()   const { return m_checkboxCheckedActive; }

	unsigned int ButtonMinInnerSize() const { return m_buttonMinInnerSize; }
	unsigned int SliderMinInnerSize() const { return m_sliderMinInnerSize; }

	float ListAlphaNormal() const { return m_listAlphaNormal; }
	float ListAlphaSelect() const { return m_listAlphaSelect; }
	float ListAlphaHover()  const { return m_listAlphaHover; }

private:
	Graphics::Renderer *m_renderer;

	float m_scale;

	RefCountedPtr<Graphics::Texture> m_texture;
	RefCountedPtr<Graphics::Material> m_material;

	void DrawRectElement(const RectElement &element, const Point &pos, const Point &size) const;
	void DrawBorderedRectElement(const BorderedRectElement &element, const Point &pos, const Point &size) const;

	RectElement LoadRectElement(const std::string &spec);
	BorderedRectElement LoadBorderedRectElement(const std::string &spec);

	BorderedRectElement m_backgroundNormal;
	BorderedRectElement m_backgroundActive;

	BorderedRectElement m_buttonDisabled;
	BorderedRectElement m_buttonNormal;
	BorderedRectElement m_buttonHover;
	BorderedRectElement m_buttonActive;

	RectElement m_checkboxDisabled;
	RectElement m_checkboxNormal;
	RectElement m_checkboxHover;
	RectElement m_checkboxActive;

	RectElement m_checkboxCheckedDisabled;
	RectElement m_checkboxCheckedNormal;
	RectElement m_checkboxCheckedHover;
	RectElement m_checkboxCheckedActive;

	unsigned int m_buttonMinInnerSize;
	unsigned int m_sliderMinInnerSize;

	float m_listAlphaNormal;
	float m_listAlphaSelect;
	float m_listAlphaHover;
};

}

#endif
