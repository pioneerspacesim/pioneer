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
private:
	virtual void OnMouseDown(Gui::MouseButtonEvent *e) {}
	matrix4x4d viewingRotation;
	float viewingDist;
	Gui::Label *m_infoLabel;
};

#endif /* _OBJECTVIEWERVIEW_H */
