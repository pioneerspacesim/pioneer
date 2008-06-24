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
private:
	void OnClickHyperspace();
	Gui::ImageButton *m_hyperspaceButton;
	GLuint m_bgstarsDlist;
};

#endif /* _WORLDVIEW_H */
