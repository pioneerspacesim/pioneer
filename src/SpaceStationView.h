#ifndef _SPACESTATIONVIEW_H
#define _SPACESTATIONVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include "GenericChatForm.h"
#include "ChatForm.h"
#include <stack>

class StationViewShipView;

class SpaceStationView: public View {
public:
	SpaceStationView();
	virtual void Update() {}
	virtual void Draw3D();
	virtual void OnSwitchTo();

	void ActivateForm(ChatForm *form);
	void CloseForm();
	void JumpToForm(ChatForm *form);

private:
	std::stack<ChatForm*> m_activeForms;

	// hack so StationViewShipView can draw its 3d shit
	sigc::signal<void> onDraw3D;
};

#endif /* _SPACESTATIONVIEW_H */
