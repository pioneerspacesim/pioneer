#ifndef _SPACESTATIONVIEW_H
#define _SPACESTATIONVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"

class SpaceStationView: public View {
public:
	SpaceStationView();
	virtual void Update();
	virtual void Draw3D();
private:
	void OnClickRequestLaunch();
};

#endif /* _SPACESTATIONVIEW_H */
