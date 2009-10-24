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
	virtual void HideAll();
	void OpenLoadDialog();
	void OpenSaveDialog();
private:
	void OnChangePlanetDetail(int level);
	bool m_changedDetailLevel;
	View *m_subview;
	Gui::RadioButton *m_planetDetail[5];
};

#endif /* _GAMEMENUVIEW_H */
