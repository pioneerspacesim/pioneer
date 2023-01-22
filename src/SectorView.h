// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SECTORVIEW_H
#define _SECTORVIEW_H

#include "ConnectionTicket.h"
#include "DeleteEmitter.h"
#include "Input.h"
#include "galaxy/Sector.h"
#include "galaxy/SystemPath.h"
#include "graphics/Drawables.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "pigui/PiGuiView.h"
#include <set>
#include <string>
#include <vector>

class Game;
class Galaxy;

class SectorView : public PiGuiView, public DeleteEmitter {
public:
	SectorView(Game *game);
	SectorView(const Json &jsonObj, Game *game);
	~SectorView() override;

	void Update() override;
	// void ShowAll() override;
	void Draw3D() override;

	void DrawPiGui() override;

	vector3f GetPosition() const { return m_pos; }
	SystemPath GetCurrent() const { return m_current; }
	SystemPath GetSelected() const { return m_selected; }
	void SwitchToPath(const SystemPath &path);
	SystemPath GetHyperspaceTarget() const { return m_hyperspaceTarget; }
	void SetHyperspaceTarget(const SystemPath &path);
	void ResetHyperspaceTarget();
	void GotoSector(const SystemPath &path);
	void GotoSystem(const SystemPath &path);
	void GotoCurrentSystem() { GotoSystem(m_current); }
	void GotoSelectedSystem() { GotoSystem(m_selected); }
	void GotoHyperspaceTarget() { GotoSystem(m_hyperspaceTarget); }
	bool IsCenteredOn(const SystemPath &path);
	void SaveToJson(Json &jsonObj) override;

	sigc::signal<void> onHyperspaceTargetChanged;

	double GetZoomLevel() const;
	void ZoomIn();
	void ZoomOut();
	vector3f GetCenterSector();
	double GetCenterDistance();
	void SetDrawUninhabitedLabels(bool value) { m_drawUninhabitedLabels = value; }
	void SetDrawVerticalLines(bool value) { m_drawVerticalLines = value; }
	void SetDrawOutRangeLabels(bool value) { m_drawOutRangeLabels = value; }
	void SetAutomaticSystemSelection(bool value) { m_automaticSystemSelection = value; }
	std::vector<SystemPath> GetNearbyStarSystemsByName(std::string pattern);
	const std::set<const Faction *> &GetVisibleFactions() { return m_visibleFactions; }
	const std::set<const Faction *> &GetHiddenFactions() { return m_hiddenFactions; }
	void SetFactionVisible(const Faction *faction, bool visible);
	void SetZoomMode(bool enable);
	void SetRotateMode(bool enable);
	void ResetView();
	void SetLabelParams(std::string fontName, int fontSize, float gap, Color highlight, Color shade);
	void DrawLabels();
	void SetLabelsVisibility(bool hideLabels) { m_hideLabels = hideLabels; }

	// HyperJump Route Planner
	bool MoveRouteItemUp(const std::vector<SystemPath>::size_type element);
	bool MoveRouteItemDown(const std::vector<SystemPath>::size_type element);
	void UpdateRouteItem(const std::vector<SystemPath>::size_type element, const SystemPath &path);
	void AddToRoute(const SystemPath &path);
	bool RemoveRouteItem(const std::vector<SystemPath>::size_type element);
	void ClearRoute();
	std::vector<SystemPath> GetRoute();
	const std::string AutoRoute(const SystemPath &start, const SystemPath &target, std::vector<SystemPath> &outRoute) const;
	void SetDrawRouteLines(bool value) { m_drawRouteLines = value; }

protected:
	void OnSwitchTo() override;
	void OnSwitchFrom() override;

	struct InputBinding : public Input::InputFrame {
		using InputFrame::InputFrame;

		Action *mapToggleSelectionFollowView;
		Action *mapWarpToCurrent;
		Action *mapWarpToSelected;
		Action *mapViewReset;

		Axis *mapViewMoveForward;
		Axis *mapViewMoveLeft;
		Axis *mapViewMoveUp;
		Axis *mapViewYaw;
		Axis *mapViewPitch;
		Axis *mapViewZoom;

		void RegisterBindings() override;
	} InputBindings;

private:
	void InitDefaults();
	void InitObject();

	void DrawNearSectors(const matrix4x4f &modelview);
	void DrawNearSector(const int sx, const int sy, const int sz, const matrix4x4f &trans);
	void PutSystemLabels(RefCountedPtr<Sector> sec, const vector3f &origin, int drawRadius);
	void PutSystemLabel(const Sector::System &sys);

	void DrawFarSectors(const matrix4x4f &modelview);
	void BuildFarSector(RefCountedPtr<Sector> sec, const vector3f &origin, std::vector<vector3f> &points, std::vector<Color> &colors);
	void PutFactionLabels(const vector3f &secPos);
	void AddStarBillboard(const matrix4x4f &modelview, const vector3f &pos, const Color &col, float size);

	void OnClickSystem(const SystemPath &path);
	const SystemPath &CheckPathInRoute(const SystemPath &path);

	RefCountedPtr<Sector> GetCached(const SystemPath &loc) { return m_sectorCache->GetCached(loc); }
	void ShrinkCache();
	void SetSelected(const SystemPath &path);

	void MouseWheel(bool up);

	RefCountedPtr<Galaxy> m_galaxy;

	bool m_inSystem;

	SystemPath m_current;
	SystemPath m_selected;

	vector3f m_pos;
	vector3f m_posMovingTo;

	float m_rotXDefault, m_rotZDefault, m_zoomDefault;

	float m_rotX, m_rotZ;
	float m_rotXMovingTo, m_rotZMovingTo;

	float m_zoom;
	float m_zoomClamped;
	float m_zoomMovingTo;

	bool m_rotateWithMouseButton = false;
	bool m_rotateView = false;
	bool m_zoomView = false;
	bool m_manualMove = false;

	SystemPath m_hyperspaceTarget;
	bool m_automaticSystemSelection;

	bool m_drawUninhabitedLabels;
	bool m_drawOutRangeLabels;
	bool m_drawVerticalLines;

	//Gui::LabelSet *m_clickableLabels;

	std::set<const Faction *> m_visibleFactions;
	std::set<const Faction *> m_hiddenFactions;

	Uint8 m_detailBoxVisible;

	ConnectionTicket m_onMouseWheelCon;
	ConnectionTicket m_onToggleSelectionFollowView;
	ConnectionTicket m_onWarpToCurrent;
	ConnectionTicket m_onWarpToSelected;
	ConnectionTicket m_onViewReset;

	RefCountedPtr<SectorCache::Slave> m_sectorCache;
	std::string m_previousSearch;

	float m_playerHyperspaceRange;

	// HyperJump Route Planner Stuff
	std::vector<SystemPath> m_route;

	bool m_drawRouteLines;
	bool m_setupRouteLines;
	void DrawRouteLines(const matrix4x4f &trans);
	void SetupRouteLines(const vector3f &playerAbsPos);
	void GetPlayerPosAndStarSize(vector3f &playerPosOut, float &currentStarSizeOut);

	std::vector<vector3f> m_farstars;
	std::vector<Color> m_farstarsColor;

	vector3f m_secPosFar;
	int m_radiusFar;
	bool m_toggledFaction;

	int m_cacheXMin;
	int m_cacheXMax;
	int m_cacheYMin;
	int m_cacheYMax;
	int m_cacheZMin;
	int m_cacheZMax;

	std::unique_ptr<ImDrawList> m_drawList;

	std::unique_ptr<Graphics::VertexArray> m_lineVerts;
	std::unique_ptr<Graphics::VertexArray> m_secLineVerts;
	std::unique_ptr<Graphics::VertexArray> m_starVerts;

	RefCountedPtr<Graphics::Material> m_starMaterial;
	RefCountedPtr<Graphics::Material> m_fresnelMat;
	RefCountedPtr<Graphics::Material> m_lineMat;
	RefCountedPtr<Graphics::Material> m_farStarsMat;

	std::unique_ptr<Graphics::Drawables::Sphere3D> m_jumpSphere;

	Graphics::Drawables::Lines m_lines;
	Graphics::Drawables::Lines m_sectorlines;
	Graphics::Drawables::Lines m_routeLines;
	Graphics::Drawables::Points m_farstarsPoints;

	class Label;
	class StarLabel;
	class FactionLabel;
	struct Labels {
		// the constructor can only be defined after defining the Label class
		Labels(SectorView &view);
		// settings and globals for labels
		// this is not hardcode, these are the defaults
		std::string fontName = "orbiteer";
		int fontSize = 15;
		float gap = 2.f;
		ImFont *starLabelFont = nullptr;
		ImFont *factionHomeFont = nullptr;
		ImFont *factionNameFont = nullptr;
		ImRect starLabelHoverArea;
		ImRect factionHomeHoverArea;
		ImRect factionNameHoverArea;
		ImU32 highlightColor = IM_COL32(255, 255, 255, 100);
		ImU32 shadeColor = IM_COL32(25, 51, 82, 200);
		// owning object
		SectorView &view;
		// array
		std::vector<std::unique_ptr<Label>> array;
	};
	Labels m_labels;
	bool m_hideLabels = false;
};

#endif /* _SECTORVIEW_H */
