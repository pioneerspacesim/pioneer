// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GAMEUI_LABELOVERLAY_H
#define GAMEUI_LABELOVERLAY_H

#include "ui/Context.h"
#include "text/TextureFont.h"
#include "graphics/Frustum.h"

namespace GameUI {

class LabelOverlay : public UI::Widget {
public:
	struct Marker {
		std::string text;
		vector3f position;
	};

	LabelOverlay(UI::Context *context);

	virtual UI::Point PreferredSize() override {
		return UI::Point(UI::Widget::SIZE_EXPAND);
	}

	virtual void Draw() override;

	Marker *AddMarker(const std::string &text, const vector3f &pos);
	void Clear();

	void SetView(const Graphics::Frustum &view_frustum);

private:
	std::vector<std::unique_ptr<Marker>> m_markers;
	RefCountedPtr<Text::TextureFont> m_font;
	Graphics::Frustum m_view;
};

}

#endif
