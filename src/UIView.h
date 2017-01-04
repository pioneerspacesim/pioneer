// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UIVIEW_H
#define UIVIEW_H

#include "View.h"

namespace UI {
	class Widget;
	class Single;
}

// wrapper to allow new UI to be switched to by the existing view system
// remove this once all existing views are ported to the new UI
class UIView : public View {
public:
	UIView(const char *templateName) : m_templateName(templateName) {}
	UIView() : m_templateName(0) {}

	virtual void Update() {}
	virtual void Draw3D() {}
	const char* GetTemplateName() { return m_templateName; }

protected:
	virtual void BuildUI(UI::Single *container);
	virtual void OnSwitchTo();
	virtual void OnSwitchFrom();

	UI::Widget *BuildTemplateUI();

private:
	const char *m_templateName;
};

#endif
