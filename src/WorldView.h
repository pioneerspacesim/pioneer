#ifndef _WORLDVIEW_H
#define _WORLDVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"

class WorldView: public View {
public:
	WorldView();
	virtual void Update();
	virtual void Draw3D();
	matrix4x4d viewingRotation;
	static const float PICK_OBJECT_RECT_SIZE;
private:
	void OnClickHyperspace();
	void OnChangeWheelsState(Gui::MultiStateImageButton *b);
	virtual void OnMouseDown(Gui::MouseButtonEvent *e);
	Body* PickBody(const float screenX, const float screenY) const;
	Gui::ImageButton *m_hyperspaceButton;
	GLuint m_bgstarsDlist;
};

#endif /* _WORLDVIEW_H */
