// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _INFOVIEW
#define _INFOVIEW

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include "ShipSpinnerWidget.h"

class InfoViewPage;
class ShipSpinnerWidget;

class InfoView: public View {
public:
	InfoView();
	void UpdateInfo();
	virtual void Update();
	virtual void Draw3D();
	void NextPage();
	void ShowSpinner() { m_showSpinner = true; }
	void HideSpinner() { m_showSpinner = false; }
protected:
	virtual void OnSwitchTo();
private:
	std::list<InfoViewPage*> m_pages;
	Gui::Tabbed *m_tabs;
	bool m_showSpinner;
	ShipSpinnerWidget *m_spinner;
	bool m_doUpdate;
};

#endif /* _INFOVIEW */
