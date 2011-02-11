#ifndef _SYSTEMINFOVIEW_H
#define _SYSTEMINFOVIEW_H

#include "libs.h"
#include "Gui.h"
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
private:
	void SystemChanged(StarSystem *s);
	void UpdateEconomyTab();
	void OnBodySelected(SBody *b);
	void OnClickBackground(Gui::MouseButtonEvent *e);
	void PutBodies(SBody *body, Gui::Fixed *container, int dir, float pos[2], int &majorBodies, float &prevSize);
	SBody *m_bodySelected;
	Gui::VBox *m_infoBox;
	Gui::Label *m_econInfo;
	Gui::Label *m_econMajImport, *m_econMinImport;
	Gui::Label *m_econMajExport, *m_econMinExport;
	Gui::Label *m_econIllegal;
	Gui::Fixed *m_sbodyInfoTab, *m_econInfoTab;
	StarSystem *m_system;
	bool m_refresh;
};

#endif /* _SYSTEMINFOVIEW_H */
