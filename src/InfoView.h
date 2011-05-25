#ifndef _INFOVIEW
#define _INFOVIEW

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"

class InfoViewPage;

class InfoView: public View {
public:
	InfoView();
	void UpdateInfo();
	virtual void Update();
	virtual void Draw3D();
	virtual void OnSwitchTo() {}
	void NextPage();
private:
	std::list<InfoViewPage*> m_pages;
	Gui::Tabbed *m_tabs;
	bool m_doUpdate;
};

#endif /* _INFOVIEW */
