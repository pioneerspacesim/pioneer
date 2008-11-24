#ifndef _INFOVIEW
#define _INFOVIEW

#include "libs.h"
#include "Gui.h"
#include "View.h"

class InfoView: public View {
public:
	InfoView();
	void UpdateInfo();
	virtual void Update();
	virtual void Draw3D();
	virtual void OnSwitchTo() {}
private:
	Gui::Label *info1;
};

#endif /* _INFOVIEW */
