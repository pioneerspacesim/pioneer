// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SYSTEMVIEW_H
#define _SYSTEMVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "UIView.h"
#include "graphics/Drawables.h"

class StarSystem;
class SystemBody;
class Orbit;

class SystemView: public UIView {
public:
	SystemView();
	virtual ~SystemView();
	virtual void Update();
	virtual void Draw3D();
private:
	static const double PICK_OBJECT_RECT_SIZE;
	void PutOrbit(const Orbit *orb, const vector3d &offset, const Color &color, double planetRadius = 0.0);
	void PutBody(const SystemBody *b, const vector3d &offset, const matrix4x4f &trans);
	void PutLabel(const SystemBody *b, const vector3d &offset);
	void PutSelectionBox(const SystemBody *b, const vector3d &rootPos, const Color &col);
	void PutSelectionBox(const vector3d &worldPos, const Color &col);
	void GetTransformTo(const SystemBody *b, vector3d &pos);
	void OnClickObject(const SystemBody *b);
	void OnClickAccel(float step);
	void OnClickRealt();
	void ResetViewpoint();
	void MouseWheel(bool up);

	RefCountedPtr<StarSystem> m_system;
	const SystemBody *m_selectedObject;
	float m_rot_x, m_rot_z;
	float m_zoom, m_zoomTo;
	double m_time;
	bool m_realtime;
	double m_timeStep;
	Gui::ImageButton *m_zoomInButton;
	Gui::ImageButton *m_zoomOutButton;
	Gui::Label *m_timePoint;
	Gui::Label *m_infoLabel;
	Gui::Label *m_infoText;
	Gui::LabelSet *m_objectLabels;
	sigc::connection m_onMouseWheelCon;

	std::unique_ptr<Graphics::Drawables::Disk> m_bodyIcon;
	Graphics::RenderState *m_lineState;
};

#endif /* _SYSTEMVIEW_H */
