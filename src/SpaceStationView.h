#ifndef _SPACESTATIONVIEW_H
#define _SPACESTATIONVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"

class StationSubView;
class StationViewShipView;

class SpaceStationView: public View {
public:
	SpaceStationView();
	virtual void Update();
	virtual void Draw3D();
	void GotoShipyard();
	void GotoCommodities();
	virtual void OnSwitchTo();
	void OnClickRequestLaunch();
	friend class StationViewShipView;
private:
	// hack so StationViewShipView can draw its 3d shit
	sigc::signal<void> onDraw3D;
};

#endif /* _SPACESTATIONVIEW_H */
