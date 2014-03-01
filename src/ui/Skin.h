// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_SKIN_H
#define UI_SKIN_H

#include "libs.h"
#include "SmartPtr.h"
#include "graphics/Renderer.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "Point.h"

#include <SDL_stdinc.h>

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

	void DrawSmallButtonDisabled(const Point &pos, const Point &size) const {
		DrawRectElement(m_smallButtonDisabled, pos, size);
	}
	void DrawSmallButtonNormal(const Point &pos, const Point &size) const {
		DrawRectElement(m_smallButtonNormal, pos, size);
	}
	void DrawSmallButtonHover(const Point &pos, const Point &size) const {
		DrawRectElement(m_smallButtonHover, pos, size);
	}
	void DrawSmallButtonActive(const Point &pos, const Point &size) const {
		DrawRectElement(m_smallButtonActive, pos, size);
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

	void DrawSliderVerticalGutter(const Point &pos, const Point &size) const {
		DrawVerticalEdgedRectElement(m_sliderVerticalGutter, pos, size);
	}
	void DrawSliderHorizontalGutter(const Point &pos, const Point &size) const {
		DrawHorizontalEdgedRectElement(m_sliderHorizontalGutter, pos, size);
	}
	void DrawSliderVerticalButtonNormal(const Point &pos, const Point &size) const {
		DrawRectElement(m_sliderVerticalButtonNormal, pos, size);
	}
	void DrawSliderVerticalButtonHover(const Point &pos, const Point &size) const {
		DrawRectElement(m_sliderVerticalButtonHover, pos, size);
	}
	void DrawSliderVerticalButtonActive(const Point &pos, const Point &size) const {
		DrawRectElement(m_sliderVerticalButtonActive, pos, size);
	}
	void DrawSliderHorizontalButtonNormal(const Point &pos, const Point &size) const {
		DrawRectElement(m_sliderHorizontalButtonNormal, pos, size);
	}
	void DrawSliderHorizontalButtonHover(const Point &pos, const Point &size) const {
		DrawRectElement(m_sliderHorizontalButtonHover, pos, size);
	}
	void DrawSliderHorizontalButtonActive(const Point &pos, const Point &size) const {
		DrawRectElement(m_sliderHorizontalButtonActive, pos, size);
	}

	void DrawGaugeBackground(const Point &pos, const Point &size) const {
		DrawHorizontalEdgedRectElement(m_gaugeBackground, pos, size);
	}
	void DrawGaugeMask(const Point &pos, const Point &size) const {
		DrawHorizontalEdgedRectElement(m_gaugeMask, pos, size, Graphics::BLEND_SET_ALPHA);
	}
	void DrawGaugeFillNormal(const Point &pos, const Point &size) const {
		DrawRectElement(m_gaugeFillNormal, pos, size, Graphics::BLEND_DEST_ALPHA);
	}
	void DrawGaugeFillWarning(const Point &pos, const Point &size) const {
		DrawRectElement(m_gaugeFillWarning, pos, size, Graphics::BLEND_DEST_ALPHA);
	}
	void DrawGaugeFillCritical(const Point &pos, const Point &size) const {
		DrawRectElement(m_gaugeFillCritical, pos, size, Graphics::BLEND_DEST_ALPHA);
	}


	void DrawRectColor(const Color &col, const Point &pos, const Point &size) const;
	void DrawRectNormal(const Point &pos, const Point &size) const {
		DrawRectColor(Color(0,0,0,AlphaNormal_ub()), pos, size);
	}
	void DrawRectHover(const Point &pos, const Point &size) const {
		DrawRectColor(Color(0,0,0,AlphaHover_ub()), pos, size);
	}
	void DrawRectSelect(const Point &pos, const Point &size) const {
		DrawRectColor(Color(0,0,0,AlphaSelect_ub()), pos, size);
	}


	struct RectElement {
		RectElement() {}
		RectElement(unsigned int x, unsigned int y, unsigned int w, unsigned int h) : pos(x,y), size(w,h) {}
		Point pos;
		Point size;
	};

	struct BorderedRectElement : public RectElement {
		BorderedRectElement() : borderWidth(0), borderHeight(0), paddingX(0), paddingY(0) {}
		BorderedRectElement(unsigned int x, unsigned int y, unsigned int w, unsigned int h,
				            unsigned int _borderWidth, unsigned int _borderHeight, unsigned int _paddingX, unsigned int _paddingY) :
			RectElement(x, y, w, h), borderWidth(_borderWidth), borderHeight(_borderHeight), paddingX(_paddingX), paddingY(_paddingY) {}
		unsigned int borderWidth;
		unsigned int borderHeight;
		unsigned int paddingX;
		unsigned int paddingY;
	};

	struct EdgedRectElement : public RectElement {
		EdgedRectElement() : edgeWidth(0) {}
		EdgedRectElement(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int _edgeWidth) :
			RectElement(x, y, w, h), edgeWidth(_edgeWidth) {}
		unsigned int edgeWidth;
	};

	const BorderedRectElement &BackgroundNormal() const { return m_backgroundNormal; }
	const BorderedRectElement &BackgroundActive() const { return m_backgroundActive; }

	const BorderedRectElement &ButtonDisabled() const { return m_buttonDisabled; }
	const BorderedRectElement &ButtonNormal()   const { return m_buttonNormal; }
	const BorderedRectElement &ButtonHover()    const { return m_buttonHover; }
	const BorderedRectElement &ButtonActive()   const { return m_buttonActive; }

	const RectElement &SmallButtonDisabled() const { return m_smallButtonDisabled; }
	const RectElement &SmallButtonNormal()   const { return m_smallButtonNormal; }
	const RectElement &SmallButtonHover()    const { return m_smallButtonHover; }
	const RectElement &SmallButtonActive()   const { return m_smallButtonActive; }

	const RectElement &CheckboxDisabled()        const { return m_checkboxDisabled; }
	const RectElement &CheckboxNormal()          const { return m_checkboxNormal; }
	const RectElement &CheckboxHover()           const { return m_checkboxHover; }
	const RectElement &CheckboxActive()          const { return m_checkboxActive; }
	const RectElement &CheckboxCheckedDisabled() const { return m_checkboxCheckedDisabled; }
	const RectElement &CheckboxCheckedNormal()   const { return m_checkboxCheckedNormal; }
	const RectElement &CheckboxCheckedHover()    const { return m_checkboxCheckedHover; }
	const RectElement &CheckboxCheckedActive()   const { return m_checkboxCheckedActive; }

	const EdgedRectElement &SliderVerticalGutter()   const { return m_sliderVerticalGutter; }
	const EdgedRectElement &SliderHorizontalGutter() const { return m_sliderHorizontalGutter; }
	const RectElement &SliderVerticalButtonNormal()  const { return m_sliderVerticalButtonNormal; }
	const RectElement &SliderVerticalButtonHover()   const { return m_sliderVerticalButtonHover; }
	const RectElement &SliderVerticalButtonActive()  const { return m_sliderVerticalButtonActive; }
	const RectElement &SliderHorizontalButtonNormal() const { return m_sliderHorizontalButtonNormal; }
	const RectElement &SliderHorizontalButtonHover()   const { return m_sliderHorizontalButtonHover; }
	const RectElement &SliderHorizontalButtonActive()  const { return m_sliderHorizontalButtonActive; }

	const EdgedRectElement &GaugeBackground() const { return m_gaugeBackground; }
	const EdgedRectElement &GaugeMask()       const { return m_gaugeMask; }
	const RectElement &GaugeFillNormal()      const { return m_gaugeFillNormal; }
	const RectElement &GaugeFillWarning()     const { return m_gaugeFillWarning; }
	const RectElement &GaugeFillCritical()    const { return m_gaugeFillCritical; }

	unsigned int ButtonMinInnerSize() const { return m_buttonMinInnerSize; }

	float AlphaNormal() const { return m_alphaNormal; }
	float AlphaSelect() const { return m_alphaSelect; }
	float AlphaHover()  const { return m_alphaHover; }

	Uint8 AlphaNormal_ub() const { return m_alphaNormal * 255; }
	Uint8 AlphaSelect_ub() const { return m_alphaSelect * 255; }
	Uint8 AlphaHover_ub()  const { return m_alphaHover * 255; }

	Graphics::RenderState *GetAlphaBlendState() const { return m_alphaBlendState; }
	Graphics::RenderState *GetRenderState(Graphics::BlendMode) const;

private:
	Graphics::Renderer *m_renderer;

	float m_scale;

	RefCountedPtr<Graphics::Texture> m_texture;
	RefCountedPtr<Graphics::Material> m_textureMaterial;
	RefCountedPtr<Graphics::Material> m_colorMaterial;

	Graphics::RenderState *m_alphaBlendState;
	Graphics::RenderState *m_alphaSetState;
	Graphics::RenderState *m_alphaMaskState;

	void DrawRectElement(const RectElement &element, const Point &pos, const Point &size, Graphics::BlendMode blendMode = Graphics::BLEND_ALPHA) const;
	void DrawBorderedRectElement(const BorderedRectElement &element, const Point &pos, const Point &size, Graphics::BlendMode blendMode = Graphics::BLEND_ALPHA) const;
	void DrawVerticalEdgedRectElement(const EdgedRectElement &element, const Point &pos, const Point &size, Graphics::BlendMode blendMode = Graphics::BLEND_ALPHA) const;
	void DrawHorizontalEdgedRectElement(const EdgedRectElement &element, const Point &pos, const Point &size, Graphics::BlendMode blendMode = Graphics::BLEND_ALPHA) const;

	RectElement LoadRectElement(const std::string &spec);
	BorderedRectElement LoadBorderedRectElement(const std::string &spec);
	EdgedRectElement LoadEdgedRectElement(const std::string &spec);

	BorderedRectElement m_backgroundNormal;
	BorderedRectElement m_backgroundActive;

	BorderedRectElement m_buttonDisabled;
	BorderedRectElement m_buttonNormal;
	BorderedRectElement m_buttonHover;
	BorderedRectElement m_buttonActive;

	RectElement m_smallButtonDisabled;
	RectElement m_smallButtonNormal;
	RectElement m_smallButtonHover;
	RectElement m_smallButtonActive;

	RectElement m_checkboxDisabled;
	RectElement m_checkboxNormal;
	RectElement m_checkboxHover;
	RectElement m_checkboxActive;

	RectElement m_checkboxCheckedDisabled;
	RectElement m_checkboxCheckedNormal;
	RectElement m_checkboxCheckedHover;
	RectElement m_checkboxCheckedActive;

	EdgedRectElement m_sliderVerticalGutter;
	EdgedRectElement m_sliderHorizontalGutter;
	RectElement m_sliderVerticalButtonNormal;
	RectElement m_sliderVerticalButtonHover;
	RectElement m_sliderVerticalButtonActive;
	RectElement m_sliderHorizontalButtonNormal;
	RectElement m_sliderHorizontalButtonHover;
	RectElement m_sliderHorizontalButtonActive;

	EdgedRectElement m_gaugeBackground;
	EdgedRectElement m_gaugeMask;
	RectElement m_gaugeFillNormal;
	RectElement m_gaugeFillWarning;
	RectElement m_gaugeFillCritical;

	unsigned int m_buttonMinInnerSize;

	float m_alphaNormal;
	float m_alphaSelect;
	float m_alphaHover;
};

}

#endif
