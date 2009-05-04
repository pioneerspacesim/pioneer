#ifndef _SYSTEMVIEW_H
#define _SYSTEMVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"

class StarSystem;
class SBody;

class SystemView: public View {
public:
	SystemView();
	virtual ~SystemView();
	virtual void Update();
	virtual void Draw3D();
	virtual void OnSwitchTo() {}
private:
	void PutOrbit(SBody *b, vector3d offset);
	void PutBody(SBody *b, vector3d offset);
	void PutLabel(SBody *b, vector3d offset);
	void GetTransformTo(SBody *b, vector3d &pos);
	void OnClickObject(SBody *b, const Gui::MouseButtonEvent *ev);
	void OnClickAccel(float step);
	void ResetViewpoint();

	StarSystem *m_system;
	SBody *m_selectedObject;
	float m_rot_x, m_rot_z;
	float m_zoom;
	double m_time;
	double m_timeStep;
	Gui::ImageButton *m_zoomInButton;
	Gui::ImageButton *m_zoomOutButton;
	Gui::Label *m_timePoint;
};

#endif /* _SYSTEMVIEW_H */
