// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SYSTEMVIEW_H
#define _SYSTEMVIEW_H

#include "Color.h"
#include "DeleteEmitter.h"
#include "Frame.h"
#include "Input.h"
#include "TransferPlanner.h"
#include "enum_table.h"
#include "graphics/Drawables.h"
#include "matrix4x4.h"
#include "pigui/PiGuiView.h"
#include "vector3.h"
#include "ConnectionTicket.h"

class StarSystem;
class SystemBody;
class Orbit;
class Ship;
class Game;
class Body;

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

struct Projectable {
	enum types {	// <enum name=ProjectableTypes scope='Projectable' public>
		NONE = 0,	// empty projectable, don't try to get members
		OBJECT = 1, // clickable space object, may be without phys.body (other starsystem)
		L4 = 2,
		L5 = 3,
		APOAPSIS = 4,
		PERIAPSIS = 5
	} type;
	enum bases {		// <enum name=ProjectableBases scope='Projectable' public>
		SYSTEMBODY = 0, // ref class SystemBody, may not have a physical body
		BODY = 1,		// generic body
		SHIP = 2,
		PLAYER = 3, // player's ship
		PLANNER = 4 // player's ship planned by transfer planner, refers to player's object
	} base;
	union {
		const Body *body;
		const SystemBody *sbody;
	} ref;
	vector3d screenpos; // x,y - screen coordinate, z - in NDC
	vector3d worldpos;
	float screensize = 0.f; // approximate size in screen pixels

	Projectable(const types t, const bases b, const Body *obj) :
		type(t), base(b)
	{
		ref.body = obj;
	}
	Projectable(const types t, const bases b, const SystemBody *obj) :
		type(t), base(b)
	{
		ref.sbody = obj;
	}
	Projectable() :
		type(NONE) {}
};

struct AtlasBodyLayout {
	SystemBody *body;
	float radius;
	bool isVertical;
	bool isBinary;
	vector2f offset;
	vector2f size;

	std::vector<AtlasBodyLayout> children;
};

class SystemView : public PiGuiView, public DeleteEmitter {
public:
	enum class Mode { // <enum name=SystemViewMode scope='SystemView::Mode' public>
		Orrery = 0,
		Atlas = 1
	};

	SystemView(Game *game);
	~SystemView() override;
	void Update() override;
	void Draw3D() override;
	void OnSwitchFrom() override;

	Projectable *GetSelectedObject();
	void SetSelectedObject(Projectable::types type, Projectable::bases base, SystemBody *sb);
	void SetSelectedObject(Projectable::types type, Projectable::bases base, Body *b);
	void ClearSelectedObject();
	void ViewSelectedObject();
	void ResetViewpoint();

	RefCountedPtr<StarSystem> GetCurrentSystem();

	TransferPlanner *GetTransferPlanner() const { return m_planner; }
	double GetOrbitPlannerStartTime() const { return m_planner->GetStartTime(); }
	double GetOrbitPlannerTime() const { return m_time; }
	void AccelerateTime(float step);
	void SetRealTime();
	std::vector<Projectable> GetProjected() const { return m_projected; }
	void SetVisibility(std::string param);
	void SetZoomMode(bool enable);
	void SetRotateMode(bool enable);
	double ProjectedSize(double size, vector3d pos);
	float AtlasViewPlanetGap(float planetRadius) { return std::max(planetRadius * 0.6, 1.33); }
	float AtlasViewPixelPerUnit();

	float GetZoom() const;

	Mode GetDisplayMode() { return m_displayMode; }
	void SetDisplayMode(Mode displayMode) { m_displayMode = displayMode; }

	// all used colors. defined in system-view-ui.lua
	enum ColorIndex { // <enum name=SystemViewColorIndex scope='SystemView' public>
		GRID = 0,
		GRID_LEG = 1,
		SYSTEMBODY = 2,
		SYSTEMBODY_ORBIT = 3,
		PLAYER_ORBIT = 4,
		PLANNER_ORBIT = 5,
		SELECTED_SHIP_ORBIT = 6,
		SHIP_ORBIT = 7
	};

	Color svColor[8];
	void SetColor(ColorIndex color_index, Color *color_value) { svColor[color_index] = *color_value; }

private:
	struct InputBindings : public Input::InputFrame {
		using InputFrame::InputFrame;
		void RegisterBindings() override;

		Axis *mapViewPitch;
		Axis *mapViewYaw;
		Axis *mapViewZoom;
	} m_input;

	void DrawOrreryView();
	void DrawAtlasView();

	void LayoutSystemBody(SystemBody *body, AtlasBodyLayout &layout);
	void RenderAtlasBody(const AtlasBodyLayout &layout, vector3f pos, const matrix4x4f &cameraTrans);

	template <typename RefType>
	void PutOrbit(Projectable::bases base, RefType *ref, const Orbit *orb, const vector3d &offset, const Color &color, const double planetRadius = 0.0, const bool showLagrange = false);
	void PutBody(const SystemBody *b, const vector3d &offset, const matrix4x4f &trans);
	void GetTransformTo(const SystemBody *b, vector3d &pos);
	void GetTransformTo(Projectable &p, vector3d &pos);
	void MouseWheel(bool up);
	void RefreshShips(void);
	void DrawShips(const double t, const vector3d &offset);

	// draw a grid with `radius` * 2 gridlines on an evenly spaced 1-AU grid
	void DrawGrid(uint32_t radius);

	// Project a position in the current renderer project to screenspace and add it to the list of projected objects
	template <typename T>
	void AddProjected(Projectable::types type, Projectable::bases base, T *ref, const vector3d &worldpos, float screensize = 0.f);
	void CalculateShipPositionAtTime(const Ship *s, Orbit o, double t, vector3d &pos);
	void CalculateFramePositionAtTime(FrameId frameId, double t, vector3d &pos);
	double GetOrbitTime(double t, const SystemBody *b);
	double GetOrbitTime(double t, const Body *b);

	Game *m_game;

	RefCountedPtr<StarSystem> m_system;
	Projectable m_selectedObject;
	Projectable m_viewedObject;
	std::vector<Projectable> m_projected;
	std::vector<SystemBody *> m_displayed_sbody;
	bool m_unexplored;
	bool m_viewingCurrentSystem;
	std::list<std::pair<Ship *, Orbit>> m_contacts;

	AtlasBodyLayout m_atlasLayout = {};

	Mode m_displayMode;
	ShowLagrange m_showL4L5;
	TransferPlanner *m_planner;
	ShipDrawing m_shipDrawing;
	GridDrawing m_gridDrawing;

	bool m_rotateWithMouseButton = false;
	bool m_rotateView = false;
	bool m_zoomView = false;
	const float CAMERA_FOV = 50.f;
	const float CAMERA_FOV_RADIANS = CAMERA_FOV / 57.295779f;
	matrix4x4f m_cameraSpace;

	float m_rot_x, m_rot_y;
	float m_rot_x_to, m_rot_y_to;
	float m_zoom, m_zoomTo;
	float m_atlasZoom, m_atlasZoomTo, m_atlasZoomDefault;
	vector2f m_atlasPos, m_atlasPosTo, m_atlasPosDefault;
	float m_atlasViewW, m_atlasViewH;
	int m_animateTransition;
	vector3d m_trans;
	vector3d m_transTo;
	double m_time;
	bool m_realtime;
	double m_timeStep;

	ConnectionTicket m_onMouseWheelCon;

	std::unique_ptr<Graphics::Drawables::Disk> m_bodyIcon;
	std::unique_ptr<Graphics::Material> m_bodyMat;
	std::unique_ptr<Graphics::Material> m_atlasMat;
	std::unique_ptr<Graphics::Material> m_lineMat;
	std::unique_ptr<Graphics::Material> m_gridMat;
	Graphics::Drawables::Lines m_orbits;
	Graphics::Drawables::Lines m_selectBox;

	std::unique_ptr<vector3f[]> m_orbitVts;
	std::unique_ptr<Color[]> m_orbitColors;

	std::unique_ptr<Graphics::VertexArray> m_lineVerts;
	Graphics::Drawables::Lines m_lines;
};

#endif /* _SYSTEMVIEW_H */
