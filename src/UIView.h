// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _UIVIEW_H
#define _UIVIEW_H

#include "View.h"

// wrapper to allow new UI to be switched to by the existing view system
// remove this once all existing views are ported to the new UI
class UIView : public View {
public:
	UIView(const char *templateName) : m_templateName(templateName) {}

	virtual void Update();
	virtual void Draw3D();

protected:
	virtual void OnSwitchTo();
	virtual void OnSwitchFrom();

private:
	const char *m_templateName;
};

#endif
