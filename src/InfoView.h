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
	virtual void OnSwitchTo();
	void NextPage();
	void ShowSpinner() { m_showSpinner = true; }
	void HideSpinner() { m_showSpinner = false; }
private:
	std::list<InfoViewPage*> m_pages;
	Gui::Tabbed *m_tabs;
	bool m_showSpinner;
	ShipSpinnerWidget *m_spinner;
	bool m_doUpdate;
};

#endif /* _INFOVIEW */
