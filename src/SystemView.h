// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SYSTEMVIEW_H
#define _SYSTEMVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include "graphics/Drawables.h"

class StarSystem;
class SystemBody;

class SystemView: public View {
public:
	SystemView();
	virtual ~SystemView();
	virtual void Update();
	virtual void Draw3D();
protected:
	virtual void OnSwitchTo() {}
private:
	static const double PICK_OBJECT_RECT_SIZE;
	void PutOrbit(SystemBody *b, vector3d offset);
	void PutBody(SystemBody *b, vector3d offset, const matrix4x4f &trans);
	void PutLabel(SystemBody *b, vector3d offset);
	void PutSelectionBox(const SystemBody *b, const vector3d &rootPos, const Color &col);
	void PutSelectionBox(const vector3d &worldPos, const Color &col);
	void GetTransformTo(SystemBody *b, vector3d &pos);
	void OnClickObject(SystemBody *b);
	void OnClickAccel(float step);
	void ResetViewpoint();
	void MouseButtonDown(int button, int x, int y);

	RefCountedPtr<StarSystem> m_system;
	SystemBody *m_selectedObject;
	float m_rot_x, m_rot_z;
	float m_zoom, m_zoomTo;
	double m_time;
	double m_timeStep;
	Gui::ImageButton *m_zoomInButton;
	Gui::ImageButton *m_zoomOutButton;
	Gui::Label *m_timePoint;
	Gui::Label *m_infoLabel;
	Gui::Label *m_infoText;
	Gui::LabelSet *m_objectLabels;
	sigc::connection m_onMouseButtonDown;

	ScopedPtr<Graphics::Drawables::Disk> m_bodyIcon;
};

#endif /* _SYSTEMVIEW_H */
