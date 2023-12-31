// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SECTORVIEW_H
#define _SECTORVIEW_H

#include "ConnectionTicket.h"
#include "DeleteEmitter.h"
#include "Input.h"
#include "JsonFwd.h"

#include "galaxy/SystemPath.h"
#include "pigui/PiGuiView.h"

#include "vector3.h"
#include "matrix4x4.h"

class Game;
class Galaxy;
class SectorMap;
struct SectorMapContext;

class SectorView : public PiGuiView, public DeleteEmitter {
public:
	SectorView(Game *game);
	SectorView(const Json &jsonObj, Game *game);
	~SectorView() override;

	void Update() override;
	// void ShowAll() override;
	void Draw3D() override;

	void DrawPiGui() override;

	SystemPath GetCurrent() const { return m_current; }
	SystemPath GetHyperspaceTarget() const { return m_hyperspaceTarget; }
	SystemPath GetSelected() const { return m_selected; }
	SectorMap &GetMap() { return *m_map; }

	void GotoCurrentSystem();
	void GotoSelectedSystem();
	void GotoHyperspaceTarget();
	void SwitchToPath(const SystemPath &path);
	void SetHyperspaceTarget(const SystemPath &path);
	void SetDrawOutRangeLabels(bool value) { m_drawOutRangeLabels = value; }
	void SetAutomaticSystemSelection(bool value) { m_automaticSystemSelection = value; }
	void ResetHyperspaceTarget();
	void ResetView();

	void SaveToJson(Json &jsonObj) override;

	// HyperJump Route Planner
	bool MoveRouteItemUp(const std::vector<SystemPath>::size_type element);
	bool MoveRouteItemDown(const std::vector<SystemPath>::size_type element);
	void UpdateRouteItem(const std::vector<SystemPath>::size_type element, const SystemPath &path);
	void AddToRoute(const SystemPath &path);
	bool RemoveRouteItem(const std::vector<SystemPath>::size_type element);
	void ClearRoute();
	std::vector<SystemPath> GetRoute();
	const std::string AutoRoute(const SystemPath &start, const SystemPath &target, std::vector<SystemPath> &outRoute) const;
	void SetDrawRouteLines(bool value);

	sigc::signal<void> onHyperspaceTargetChanged;

protected:
	void OnSwitchTo() override;
	void OnSwitchFrom() override;

	struct InputBinding : public Input::InputFrame {
		using InputFrame::InputFrame;

		Action *mapToggleSelectionFollowView;
		Action *mapWarpToCurrent;
		Action *mapWarpToSelected;
		Action *mapViewReset;

		void RegisterBindings() override;
	} InputBindings;

private:

	void InitDefaults();
	void InitObject();
	const SystemPath &CheckPathInRoute(const SystemPath &path);
	void SetSelected(const SystemPath &path);
	void SetupLines(const vector3f &playerAbsPos, const matrix4x4f &trans);
	void GetPlayerPosAndStarSize(vector3f &playerPosOut, float &currentStarSizeOut);

	class SectorMapCallbacks;
	SectorMapContext CreateMapContext();

	SystemPath m_current;
	SystemPath m_selected;
	SystemPath m_hyperspaceTarget;

	bool m_automaticSystemSelection;
	bool m_drawOutRangeLabels;
	bool m_drawRouteLines;
	bool m_inSystem;
	bool m_setupLines;
	float m_playerHyperspaceRange;
	Uint8 m_detailBoxVisible;

	ConnectionTicket m_onToggleSelectionFollowView;
	ConnectionTicket m_onWarpToCurrent;
	ConnectionTicket m_onWarpToSelected;
	ConnectionTicket m_onViewReset;

	Game &m_game;
	std::unique_ptr<SectorMap> m_map;
	std::vector<SystemPath> m_route;

};

#endif /* _SECTORVIEW_H */
