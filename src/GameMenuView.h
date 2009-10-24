#ifndef _GAMEMENUVIEW_H
#define _GAMEMENUVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"

extern std::string GetFullSavefileDirPath();

class GameMenuView: public View {
public:
	GameMenuView();
	virtual void Update() {}
	virtual void Draw3D() {}
	virtual void OnSwitchTo();
	void OpenLoadDialog();
	void OpenSaveDialog();
private:
	View *m_subview;
};

#endif /* _GAMEMENUVIEW_H */
