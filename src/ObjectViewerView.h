#ifndef _OBJECTVIEWERVIEW_H
#define _OBJECTVIEWERVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"

class Body;

class ObjectViewerView: public View {
public:
	ObjectViewerView();
	virtual void Update();
	virtual void Draw3D();
	virtual void OnSwitchTo() {}
private:
	float viewingDist;
	Gui::Label *m_infoLabel;
	const Body* lastTarget;
};

#endif /* _OBJECTVIEWERVIEW_H */
