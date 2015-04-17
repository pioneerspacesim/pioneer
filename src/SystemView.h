// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
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
class Ship;
class Game;

enum BurnDirection {
	PROGRADE,
	NORMAL,
	RADIAL,
};

enum ShipDrawing {
	BOXES,
	ORBITS,
	OFF
};

class TransferPlanner {
public:
	TransferPlanner();
	vector3d GetVel();
	vector3d GetOffsetVel();
	void IncreaseFactor(), ResetFactor(), DecreaseFactor();
	void AddDv(BurnDirection d, double dv);
	void ResetDv(BurnDirection d);
	std::string printDv(BurnDirection d);
	std::string printFactor();
private:
	double m_dvPrograde;
	double m_dvNormal;
	double m_dvRadial;
	double m_factor;       // dv multiplier
	const double m_factorFactor = 5.0; // m_factor multiplier
};

class SystemView: public UIView {
public:
	SystemView(Game* game);
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
	void OnIncreaseFactorButtonClick(void), OnResetFactorButtonClick(void), OnDecreaseFactorButtonClick(void);
	void OnToggleShipsButtonClick(void);
	void ResetViewpoint();
	void MouseWheel(bool up);
	void RefreshShips(void);
	void DrawShips(const double t, const vector3d &offset);
	void LabelShip(Ship *s, const vector3d &offset);
	void OnClickShip(Ship *s);

	Game* m_game;
	RefCountedPtr<StarSystem> m_system;
	const SystemBody *m_selectedObject;
	bool m_unexplored;
	TransferPlanner *m_planner;
	std::list<std::pair<Ship*, Orbit>> m_contacts;
	Gui::LabelSet *m_shipLabels;
	ShipDrawing m_shipDrawing;
	float m_rot_x, m_rot_z;
	float m_zoom, m_zoomTo;
	double m_time;
	bool m_realtime;
	double m_timeStep;
	Gui::ImageButton *m_zoomInButton;
	Gui::ImageButton *m_zoomOutButton;
	Gui::ImageButton *m_toggleShipsButton;
	Gui::ImageButton *m_plannerIncreaseFactorButton, *m_plannerResetFactorButton, *m_plannerDecreaseFactorButton;
	Gui::ImageButton *m_plannerAddProgradeVelButton;
	Gui::ImageButton *m_plannerAddRetrogradeVelButton;
	Gui::ImageButton *m_plannerAddNormalVelButton;
	Gui::ImageButton *m_plannerAddAntiNormalVelButton;
	Gui::ImageButton *m_plannerAddRadiallyInVelButton;
	Gui::ImageButton *m_plannerAddRadiallyOutVelButton;
	Gui::ImageButton *m_plannerZeroProgradeVelButton, *m_plannerZeroNormalVelButton, *m_plannerZeroRadialVelButton;
	Gui::Label *m_timePoint;
	Gui::Label *m_infoLabel;
	Gui::Label *m_infoText;
	Gui::Label *m_plannerFactorText, *m_plannerProgradeDvText, *m_plannerNormalDvText, *m_plannerRadialDvText;
	Gui::LabelSet *m_objectLabels;
	sigc::connection m_onMouseWheelCon;

	std::unique_ptr<Graphics::Drawables::Disk> m_bodyIcon;
	std::unique_ptr<Gui::TexturedQuad> m_periapsisIcon;
	std::unique_ptr<Gui::TexturedQuad> m_apoapsisIcon;
	Graphics::RenderState *m_lineState;
	Graphics::Drawables::Lines m_orbits;
	Graphics::Drawables::Lines m_selectBox;
};

#endif /* _SYSTEMVIEW_H */
