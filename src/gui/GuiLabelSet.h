// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GUILABELSET_H
#define GUILABELSET_H

#include "GuiWidget.h"
#include <vector>

/*
 * Collection of clickable labels. Used by the WorldView for clickable
 * bodies, and SystemView, SectorView etc.
 */
namespace Gui {
class LabelSet: public Widget {
public:
	class LabelSetItem {
	public:
		LabelSetItem(std::string text_, sigc::slot<void> onClick_, float screenx_, float screeny_) :
			text(text_), onClick(onClick_), screenx(screenx_), screeny(screeny_), hasOwnColor(false)
		{}

		LabelSetItem(std::string text_, sigc::slot<void> onClick_, float screenx_, float screeny_, const Color &c) :
			text(text_), onClick(onClick_), screenx(screenx_), screeny(screeny_), hasOwnColor(true), color(c)
		{}

		std::string text;
		sigc::slot<void> onClick;
		float screenx, screeny;
		bool hasOwnColor;
		Color color;
	};

	LabelSet();
	bool OnMouseDown(MouseButtonEvent *e);
	virtual void Draw();
	virtual void GetSizeRequested(float size[2]);
	void Clear();
	void Add(std::string text, sigc::slot<void> onClick, float screenx, float screeny);
	/** Overrides color set by SetLabelColor */
	void Add(std::string text, sigc::slot<void> onClick, float screenx, float screeny, const Color &col);
	void SetLabelsClickable(bool v) { m_labelsClickable = v; }
	void SetLabelsVisible(bool v) { m_labelsVisible = v; }
	void SetLabelColor(const Color &c) { m_labelColor = c; }
	bool CanPutItem(float x, float y);
private:
	std::vector<LabelSetItem> m_items;
	bool m_labelsVisible;
	bool m_labelsClickable;
	Color m_labelColor;

	RefCountedPtr<Text::TextureFont> m_font;
};
}

#endif /* GUILABELSET_H */
