#ifndef _SPACESTATIONVIEW_H
#define _SPACESTATIONVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"
#include "GenericChatForm.h"

class StationSubView;
class StationViewShipView;

class SpaceStationView: public View {
public:
	SpaceStationView();
	virtual void Update();
	virtual void Draw3D();
	virtual void OnSwitchTo();
	virtual void JumpTo(GenericChatForm *f);
	friend class StationViewShipView;
private:
	// hack so StationViewShipView can draw its 3d shit
	sigc::signal<void> onDraw3D;
	StationSubView *m_activeSubview;
};

#endif /* _SPACESTATIONVIEW_H */
