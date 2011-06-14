#ifndef _SYSTEMINFOVIEW_H
#define _SYSTEMINFOVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include "View.h"

class StarSystem;
class SBody;

class SystemInfoView: public View {
public:
	SystemInfoView();
	virtual void Update();
	virtual void Draw3D();
	virtual void OnSwitchTo();
	void NextPage();
private:
	void SystemChanged(StarSystem *s);
	void UpdateEconomyTab();
	void OnBodySelected(SBody *b);
	void OnClickBackground(Gui::MouseButtonEvent *e);
	void PutBodies(SBody *body, Gui::Fixed *container, int dir, float pos[2], int &majorBodies, int &starports, float &prevSize);
	Gui::VBox *m_infoBox;
	Gui::Label *m_econInfo;
	Gui::Label *m_econMajImport, *m_econMinImport;
	Gui::Label *m_econMajExport, *m_econMinExport;
	Gui::Label *m_econIllegal;
	Gui::Fixed *m_sbodyInfoTab, *m_econInfoTab;
	Gui::Tabbed *m_tabs;
	StarSystem *m_system;
	bool m_refresh;
};

#endif /* _SYSTEMINFOVIEW_H */
