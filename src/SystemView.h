#ifndef _SYSTEMVIEW_H
#define _SYSTEMVIEW_H

#include "libs.h"
#include "gui/Gui.h"
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
	static const double PICK_OBJECT_RECT_SIZE;
	void PutOrbit(SBody *b, vector3d offset);
	void PutBody(SBody *b, vector3d offset);
	void PutLabel(SBody *b, vector3d offset);
	void PutSelectionBox(const SBody *b, const vector3d &rootPos, const Color &col);
	void PutSelectionBox(const vector3d &worldPos, const Color &col);
	void GetTransformTo(SBody *b, vector3d &pos);
	void OnClickObject(SBody *b);
	void OnClickAccel(float step);
	void ResetViewpoint();
	void MouseButtonDown(int button, int x, int y);

	RefCountedPtr<StarSystem> m_system;
	SBody *m_selectedObject;
	float m_rot_x, m_rot_z;
	float m_zoom;
	double m_time;
	double m_timeStep;
	Gui::ImageButton *m_zoomInButton;
	Gui::ImageButton *m_zoomOutButton;
	Gui::Label *m_timePoint;
	Gui::Label *m_infoLabel;
	Gui::Label *m_infoText;
	Gui::LabelSet *m_objectLabels;
	sigc::connection m_onMouseButtonDown;
};

#endif /* _SYSTEMVIEW_H */
