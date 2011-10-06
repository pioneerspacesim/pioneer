#ifndef _SPACESTATIONVIEW_H
#define _SPACESTATIONVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include "Form.h"
#include "FormController.h"
#include "FaceVideoLink.h"

class SpaceStationView: public View {
public:
	SpaceStationView();
	virtual ~SpaceStationView();
	virtual void Update();
	virtual void Draw3D() {}
	virtual void OnSwitchTo();

private:
	void RefreshForForm(Form *);

    FormController *m_formController;
};

#endif /* _SPACESTATIONVIEW_H */
