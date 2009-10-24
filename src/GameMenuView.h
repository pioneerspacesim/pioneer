#ifndef _GAMEMENUVIEW_H
#define _GAMEMENUVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"

class GameMenuView: public View {
public:
	GameMenuView();
	virtual void Update() {}
	virtual void Draw3D() {}
	virtual void OnSwitchTo();
private:
	void OpenSaveDialog();
	View *m_subview;
};

#endif /* _GAMEMENUVIEW_H */
