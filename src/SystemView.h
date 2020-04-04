// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SYSTEMVIEW_H
#define _SYSTEMVIEW_H

#include "Color.h"
#include "UIView.h"
#include "graphics/Drawables.h"
#include "matrix4x4.h"
#include "vector3.h"
#include "enum_table.h"
#include "Frame.h"

class StarSystem;
class SystemBody;
class Orbit;
class Ship;
class Game;
class Body;

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

enum class GridDrawing {
	GRID,
	GRID_AND_LEGS,
	OFF
};

enum ShowLagrange {
	LAG_ICON,
	LAG_ICONTEXT,
	LAG_OFF
};


class TransferPlanner {
public:
	TransferPlanner();
	vector3d GetVel() const;
	vector3d GetOffsetVel() const;
	vector3d GetPosition() const;
	double GetStartTime() const;
	void SetPosition(const vector3d &position);
	void IncreaseFactor(), ResetFactor(), DecreaseFactor();
	void AddStartTime(double timeStep);
	void ResetStartTime();
	double GetFactor() const { return m_factor; }
	void AddDv(BurnDirection d, double dv);
	double GetDv(BurnDirection d);
	void ResetDv(BurnDirection d);
	void ResetDv();
	std::string printDeltaTime();
	std::string printDv(BurnDirection d);
	std::string printFactor();

private:
	double m_dvPrograde;
	double m_dvNormal;
	double m_dvRadial;
	double m_factor; // dv multiplier
	const double m_factorFactor = 5.0; // m_factor multiplier
	vector3d m_position;
	vector3d m_velocity;
	double m_startTime;
};

struct Projectable
{
	enum types { // <enum name=ProjectableTypes scope='Projectable' public>
		NONE = 0, // empty projectable, don't try to get members
		OBJECT = 1, // clickable space object, may be without phys.body (other starsystem)
		L4 = 2,
		L5 = 3,
		APOAPSIS = 4,
		PERIAPSIS = 5
	} type;
	enum bases { // <enum name=ProjectableBases scope='Projectable' public>
		SYSTEMBODY = 0, // ref class SystemBody, may not have a physical body
		BODY = 1, // generic body
		SHIP = 2,
		PLAYER = 3, // player's ship
		PLANNER = 4 // player's ship planned by transfer planner, refers to player's object
	} base;
	union{
		const Body* body;
		const SystemBody* sbody;
	} ref;
	vector3d screenpos; // x,y - screen coordinate, z - in NDC

	Projectable(const types t, const bases b, const Body* obj) : type(t), base(b)
	{
		ref.body = obj;
	}
	Projectable(const types t, const bases b, const SystemBody* obj) : type(t), base(b)
	{
		ref.sbody = obj;
	}
	Projectable() : type(NONE) {}
};

class SystemView : public UIView, public DeleteEmitter {
public:
	SystemView(Game *game);
	virtual ~SystemView();
	virtual void Update();
	virtual void Draw3D();
	virtual void OnSwitchTo() { Update(); Draw3D(); }

	Projectable* GetSelectedObject();
	void SetSelectedObject(Projectable::types type, Projectable::bases base, SystemBody *sb);
	void SetSelectedObject(Projectable::types type, Projectable::bases base, Body *b);
	TransferPlanner* GetTransferPlanner() const { return m_planner; }
	double GetOrbitPlannerStartTime() const { return m_planner->GetStartTime(); }
	double GetOrbitPlannerTime() const { return m_time; }
	void AccelerateTime(float step);
	void SetRealTime();
	std::vector<Projectable> GetProjected() const { return m_projected; }
	void BodyInaccessible(Body *b);
	void SetVisibility(std::string param);
	void SetZoomMode(bool enable);
	void SetRotateMode(bool enable);
	double ProjectedSize(double size, vector3d pos);

	// all used colors. defined in system-view-ui.lua
	enum ColorIndex { // <enum name=SystemViewColorIndex scope='SystemView' public>
		GRID = 0,
		SYSTEMBODY = 1,
		SYSTEMBODY_ORBIT = 2,
		PLAYER_ORBIT = 3,
		PLANNER_ORBIT = 4,
		SELECTED_SHIP_ORBIT = 5,
		SHIP_ORBIT = 6
	};

	Color svColor[7];
	void SetColor(ColorIndex color_index, Color* color_value) { svColor[color_index] = *color_value; }

private:
	bool m_rotateWithMouseButton = false;
	bool m_rotateView = false;
	bool m_zoomView = false;
	std::vector<Projectable> m_projected;
	static const double PICK_OBJECT_RECT_SIZE;
	static const Uint16 N_VERTICES_MAX;
	const float CAMERA_FOV = 50.f;
	const float CAMERA_FOV_RADIANS = CAMERA_FOV / 57.295779f;
	matrix4x4f m_cameraSpace;
	template <typename RefType>
	void PutOrbit(Projectable::bases base, RefType *ref, const Orbit *orb, const vector3d &offset, const Color &color, const double planetRadius = 0.0, const bool showLagrange = false);
	void PutBody(const SystemBody *b, const vector3d &offset, const matrix4x4f &trans);
	void GetTransformTo(const SystemBody *b, vector3d &pos);
	void GetTransformTo(Projectable &p, vector3d &pos);
	void ResetViewpoint();
	void MouseWheel(bool up);
	void RefreshShips(void);
	void DrawShips(const double t, const vector3d &offset);
	void PrepareGrid();
	void DrawGrid();
	template <typename T>
	void AddProjected(Projectable::types type, Projectable::bases base, T *ref, vector3d &pos);
	template <typename T>
	void AddNotProjected(Projectable::types type, Projectable::bases base, T *ref, const vector3d &worldscaledpos);
	void CalculateShipPositionAtTime(const Ship *s, Orbit o, double t, vector3d &pos);
	void CalculateFramePositionAtTime(FrameId frameId, double t, vector3d &pos);
	double GetOrbitTime(double t, const SystemBody* b);
	double GetOrbitTime(double t, const Body* b);

	Game *m_game;
	RefCountedPtr<StarSystem> m_system;
	Projectable m_selectedObject;
	std::vector<SystemBody *> m_displayed_sbody;
	bool m_unexplored;
	ShowLagrange m_showL4L5;
	TransferPlanner *m_planner;
	std::list<std::pair<Ship *, Orbit>> m_contacts;
	ShipDrawing m_shipDrawing;
	GridDrawing m_gridDrawing;
	int m_grid_lines;
	float m_rot_x, m_rot_y;
	float m_rot_x_to, m_rot_y_to;
	float m_zoom, m_zoomTo;
	int m_animateTransition;
	vector3d m_trans;
	vector3d m_transTo;
	double m_time;
	bool m_realtime;
	double m_timeStep;
	sigc::connection m_onMouseWheelCon;

	std::unique_ptr<Graphics::Drawables::Disk> m_bodyIcon;
	Graphics::RenderState *m_lineState;
	Graphics::Drawables::Lines m_orbits;
	Graphics::Drawables::Lines m_selectBox;

	std::unique_ptr<vector3f[]> m_orbitVts;
	std::unique_ptr<Color[]> m_orbitColors;

	std::unique_ptr<Graphics::VertexArray> m_lineVerts;
	Graphics::Drawables::Lines m_lines;
};

#endif /* _SYSTEMVIEW_H */
