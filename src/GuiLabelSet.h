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
		LabelSetItem(std::string text, sigc::slot<void> onClick, float screenx, float screeny) {
			this->text = text;
			this->onClick = onClick;
			this->screenx = screenx;
			this->screeny = screeny;
		}
		std::string text;
		sigc::slot<void> onClick;
		int screenx, screeny;
	};

	LabelSet() {
		m_eventMask = EVENT_MOUSEDOWN;
		m_labelsVisible = true;
		m_labelsClickable = true;
		m_labelColor = Color(1.0f,1.0f,1.0f,1.0f);
	}

	bool OnMouseDown(MouseButtonEvent *e);
	virtual void Draw();
	virtual void GetSizeRequested(float size[2]);
	void Clear();
	void Add(std::string text, sigc::slot<void> onClick, float screenx, float screeny);
	void SetLabelsClickable(bool v) { m_labelsClickable = v; }
	void SetLabelsVisible(bool v) { m_labelsVisible = v; }
	void SetLabelColor(const Color &c) { m_labelColor = c; }
private:
	bool CanPutItem(float x, float y);

	std::vector<LabelSetItem> m_items;
	bool m_labelsVisible;
	bool m_labelsClickable;
	Color m_labelColor;
};
}

#endif /* GUILABELSET_H */
