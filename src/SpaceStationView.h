#ifndef _SPACESTATIONVIEW_H
#define _SPACESTATIONVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include "Form.h"
#include "FaceVideoLink.h"
#include <stack>

class SpaceStationView: public View {
public:
	SpaceStationView();
	virtual ~SpaceStationView();
	virtual void Update();
	virtual void Draw3D();
	virtual void OnSwitchTo();

	void ActivateForm(Form *form);
	void CloseForm();
	void CloseAllForms();
	void JumpToForm(Form *form);

private:
	void SetupForFaceForm(FaceForm *form);

	sigc::connection m_undockConnection;

	std::stack<Form*> m_activeForms;

	Gui::Label *m_title;

	Gui::Label *m_money;
	Gui::Label *m_cargoSpaceUsed;
	Gui::Label *m_cargoSpaceFree;
	Gui::Label *m_equipmentMass;
	Gui::Label *m_titleLabel;
	Gui::Label *m_legalstatus;

	FaceVideoLink *m_videoLink;

	// hack so StationViewShipView can draw its 3d shit
	sigc::signal<void> onDraw3D;
};

#endif /* _SPACESTATIONVIEW_H */
