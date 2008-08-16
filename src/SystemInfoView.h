#ifndef _SYSTEMINFOVIEW_H
#define _SYSTEMINFOVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"
#include "StarSystem.h"
#include "GenericSystemView.h"

class SystemInfoView: public GenericSystemView {
public:
	SystemInfoView();
	virtual void Update();
	virtual void Draw3D();
private:

	void SystemChanged(StarSystem *s);
	void OnBodySelected(StarSystem::SBody *b);
	void PutBodies(StarSystem::SBody *body, int dir, float pos[2], int &majorBodies);
	StarSystem::SBody *m_bodySelected;
	Gui::Label *m_infoText;
};

#endif /* _SYSTEMINFOVIEW_H */
