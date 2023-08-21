// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _SYSTEMVIEW_H
#define _SYSTEMVIEW_H

#include "Color.h"
#include "DeleteEmitter.h"
#include "Frame.h"
#include "Input.h"
#include "Orbit.h"
#include "TransferPlanner.h"
#include "enum_table.h"
#include "graphics/Drawables.h"
#include "matrix4x4.h"
#include "pigui/PiGuiView.h"
#include "vector3.h"

#include <sigc++/signal.h>

class GuiApplication;
class StarSystem;
class SystemBody;
class Orbit;
class Ship;
class Game;
class Body;

namespace Background {
	class Container;
}

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

struct ProjectedOrbit {
	Orbit orbit;
	Color color;
	double planetRadius;
};

struct Projectable {
	enum types {	// <enum name=ProjectableTypes scope='Projectable' public>
		NONE = 0,	// empty projectable, don't try to get members
		OBJECT = 1, // clickable space object, may be without phys.body (other starsystem)
		L4 = 2,
		L5 = 3,
		APOAPSIS = 4,
		PERIAPSIS = 5,
		ORBIT = 6 // <enum skip>
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
	vector3f screenpos;		// x,y - screen coordinate, z - in NDC
	float screensize = 0.f; // approximate size in screen pixels
	vector3d worldpos;
	int orbitIdx = -1;

	Projectable(const types t, const bases b, const Body *obj, const vector3d &pos = vector3d()) :
		type(t), base(b), worldpos(pos)
	{
		ref.body = obj;
	}
	Projectable(const types t, const bases b, const SystemBody *obj, const vector3d &pos = vector3d()) :
		type(t), base(b), worldpos(pos)
	{
		ref.sbody = obj;
	}
	Projectable() :
		type(NONE), worldpos() {}

	void *getRef() const { return base == SYSTEMBODY ? (void *)ref.sbody : (void *)ref.body; }

	bool operator==(const Projectable &rhs) const
	{
		return (type == rhs.type && base == rhs.base && getRef() == rhs.getRef());
	}
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

class SystemMapViewport;

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

	Mode GetDisplayMode() { return m_displayMode; }
	void SetDisplayMode(Mode displayMode) { m_displayMode = displayMode; }

	TransferPlanner *GetTransferPlanner() const { return m_planner; }
	double GetOrbitPlannerStartTime() const { return m_planner->GetStartTime(); }

	SystemMapViewport *GetMap() { return m_map.get(); }

private:
	void RefreshShips(void);
	void AddShipTracks(double atTime);

	void CalculateShipPositionAtTime(const Ship *s, Orbit o, double t, vector3d &pos);
	void CalculateFramePositionAtTime(FrameId frameId, double t, vector3d &pos);

	double CalculateStarportHeight(const SystemBody *body);

private:
	Game *m_game;

	std::unique_ptr<SystemMapViewport> m_map;
	TransferPlanner *m_planner;
	std::list<std::pair<Ship *, Orbit>> m_contacts;

	Mode m_displayMode;
	bool m_viewingCurrentSystem;
	bool m_unexplored;
};

class SystemMapViewport {
public:
	SystemMapViewport(GuiApplication *app);
	~SystemMapViewport();

	void Update(float deltaTime);
	void HandleInput(float deltaTime);
	void Draw3D();

	Projectable *GetSelectedObject() { return &m_selectedObject; }
	void SetSelectedObject(Projectable p) { m_selectedObject = p; }
	void ClearSelectedObject();

	void ViewSelectedObject();
	void ResetViewpoint();

	Projectable *GetViewedObject() { return &m_viewedObject; }

	ShowLagrange GetShowLagrange() { return m_showL4L5; }
	ShipDrawing GetShipDrawing() { return m_shipDrawing; }
	GridDrawing GetGridDrawing() { return m_gridDrawing; }

	// Push a tracked object / sensor contact for display on the map.
	// Object tracks are cleared every frame.
	void AddObjectTrack(Projectable p);

	// Push an orbital conic for display on the map
	// Expects the orbit's center to be provide as p.worldpos
	// Orbit tracks are cleared every frame.
	void AddOrbitTrack(Projectable p, const Orbit *orbit, Color color, double planetRadius);

	// Push the passed system body tree as object tracks
	// This should be called before pushing any other tracks for best results
	void AddBodyTrack(const SystemBody *b, const vector3d &offset = vector3d());

	RefCountedPtr<StarSystem> GetCurrentSystem();
	void SetCurrentSystem(RefCountedPtr<StarSystem> system);

	void AccelerateTime(float step);
	void SetRealTime();
	void SetReferenceTime(double time) { m_refTime = time; }
	double GetTime() { return m_time; }
	std::vector<Projectable> GetProjected() const { return m_projected; }
	void SetVisibility(std::string param);
	void SetZoomMode(bool enable);
	void SetRotateMode(bool enable);
	void SetDisplayMode(SystemView::Mode displayMode) { m_displayMode = displayMode; }
	void SetBackground(Background::Container *bg) { m_background = bg; }
	double ProjectedSize(double size, vector3d pos);
	float AtlasViewPlanetGap(float planetRadius) { return std::max(planetRadius * 0.6, 1.33); }
	float AtlasViewPixelPerUnit();

	float GetZoom() const;

	// all used colors. defined in system-view-ui.lua
	enum ColorIndex { // <enum name=SystemViewColorIndex scope='SystemMapViewport' public>
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

	sigc::slot<double(const SystemBody *)> GetStarportHeightAboveTerrain;

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

	// Project a track to screenspace with the current renderer state and add it to the list of projected objects
	void AddProjected(Projectable p, Projectable::types type, const vector3d &transformedPos, float screensize = 0.f);
	void RenderBody(const SystemBody *b, const vector3d &pos, const matrix4x4f &trans);
	void RenderOrbit(Projectable p, const ProjectedOrbit *orbitData, const vector3d &transformedPos);

	// draw a grid with `radius` * 2 gridlines on an evenly spaced 1-AU grid
	void DrawGrid(uint32_t radius);

private:
	GuiApplication *m_app;
	Graphics::Renderer *m_renderer;
	Background::Container *m_background;

	RefCountedPtr<StarSystem> m_system;
	Projectable m_selectedObject;
	Projectable m_viewedObject;

	std::vector<Projectable> m_objectTracks;
	std::vector<ProjectedOrbit> m_orbitTracks;

	std::vector<Projectable> m_projected;

	SystemView::Mode m_displayMode; // FIXME: separate Atlas from SystemMapViewport

	AtlasBodyLayout m_atlasLayout = {};
	float m_atlasZoom, m_atlasZoomTo, m_atlasZoomDefault;
	vector2f m_atlasPos, m_atlasPosTo, m_atlasPosDefault;
	float m_atlasViewW, m_atlasViewH;

	ShowLagrange m_showL4L5;
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
	int m_animateTransition;
	vector3d m_trans;
	vector3d m_transTo;
	double m_time;
	double m_refTime;
	bool m_realtime;
	double m_timeStep;

	std::unique_ptr<Graphics::Drawables::Disk> m_bodyIcon;
	std::unique_ptr<Graphics::Material> m_bodyMat;
	std::unique_ptr<Graphics::Material> m_atlasMat;
	std::unique_ptr<Graphics::Material> m_lineMat;
	std::unique_ptr<Graphics::Material> m_gridMat;
	Graphics::Drawables::Lines m_orbits;

	std::unique_ptr<vector3f[]> m_orbitVts;
	std::unique_ptr<Color[]> m_orbitColors;

	std::unique_ptr<Graphics::VertexArray> m_lineVerts;
	Graphics::Drawables::Lines m_lines;
};

#endif /* _SYSTEMVIEW_H */
