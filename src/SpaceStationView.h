#ifndef _SPACESTATIONVIEW_H
#define _SPACESTATIONVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"
#include "GenericChatForm.h"

class StationViewShipView;

class SpaceStationView: public View {
public:
	SpaceStationView();
	virtual void Update();
	virtual void Draw3D();
	virtual void OnSwitchTo();
	virtual void JumpToForm(GenericChatForm *f);
	friend class StationViewShipView;
private:
	// hack so StationViewShipView can draw its 3d shit
	sigc::signal<void> onDraw3D;
	GenericChatForm *m_baseSubView;
	GenericChatForm *m_jumpToForm;
};

#endif /* _SPACESTATIONVIEW_H */
