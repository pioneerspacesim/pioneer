#ifndef _SYSTEMVIEW_H
#define _SYSTEMVIEW_H

#include "libs.h"
#include "Gui.h"
#include "View.h"
#include "StarSystem.h"

class SystemView: public View {
public:
	SystemView();
	virtual ~SystemView();
	virtual void Update();
	virtual void Draw3D();
private:
	void PutOrbit(StarSystem::SBody *b);
	void PutBody(StarSystem::SBody *b);
	void PutLabel(StarSystem::SBody *b);
	void ViewingTransformTo(StarSystem::SBody *b);
	void OnClickObject(StarSystem::SBody *b, const Gui::MouseButtonEvent *ev);
	void OnClickAccel(float step);
	void ResetViewpoint();

	StarSystem *m_system;
	StarSystem::SBody *m_selectedObject;
	float m_rot_x, m_rot_z;
	float m_zoom;
	double m_time;
	double m_timeStep;
	Gui::ImageButton *m_zoomInButton;
	Gui::ImageButton *m_zoomOutButton;
	Gui::Label *m_timePoint;
};

#endif /* _SYSTEMVIEW_H */
