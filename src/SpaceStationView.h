#ifndef _SPACESTATIONVIEW_H
#define _SPACESTATIONVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"

class StationSubView;

class SpaceStationView: public View {
public:
	SpaceStationView();
	virtual void Update();
	virtual void Draw3D();
	void GotoShipyard();
	void GotoCommodities();
	virtual void OnSwitchTo();
	void OnClickRequestLaunch();
private:
};

#endif /* _SPACESTATIONVIEW_H */
