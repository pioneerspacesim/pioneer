#ifndef _SPACESTATIONVIEW_H
#define _SPACESTATIONVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include "GenericChatForm.h"
#include <stack>

class StationViewShipView;

class SpaceStationView: public View {
public:
	SpaceStationView();
	virtual void Update() {}
	virtual void Draw3D();
	virtual void OnSwitchTo();

	void ActivateForm(GenericChatForm *form);
	void CloseForm();
	void JumpToForm(GenericChatForm *form);

private:
	std::stack<GenericChatForm*> m_activeForms;

	// hack so StationViewShipView can draw its 3d shit
	sigc::signal<void> onDraw3D;
};

#endif /* _SPACESTATIONVIEW_H */
