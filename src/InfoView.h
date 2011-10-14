#ifndef _INFOVIEW
#define _INFOVIEW

#include "View.h"

class InfoView: public View {
public:
	InfoView();
	virtual void Update() {}
	virtual void Draw3D() {}
	virtual void OnSwitchTo();
	void NextPage();
};

#endif /* _INFOVIEW */
