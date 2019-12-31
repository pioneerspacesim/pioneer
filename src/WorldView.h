// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _WORLDVIEW_H
#define _WORLDVIEW_H

#include "UIView.h"
#include "gui/GuiWidget.h"
#include "ship/ShipViewController.h"

class Body;
class Camera;
class SpeedLines;
class NavTunnelWidget;
class Game;

enum VelIconType {
	V_PROGRADE,
	V_RETROGRADE,
	V_BURN
};

enum PlaneType {
	NONE,
	ROTATIONAL,
	PARENT
};

namespace Gui {
	class TexturedQuad;
}

namespace KeyBindings {
	struct ActionBinding;
	struct AxisBinding;
}

namespace UI {
	class Widget;
	class Single;
	class Label;
} // namespace UI

class WorldView : public UIView {
public:
	static void RegisterInputBindings();
	friend class NavTunnelWidget;
	WorldView(Game *game);
	WorldView(const Json &jsonObj, Game *game);
	virtual ~WorldView();
	virtual void ShowAll();
	virtual void Update();
	virtual void Draw3D();
	virtual void Draw();
	static const double PICK_OBJECT_RECT_SIZE;
	virtual void SaveToJson(Json &jsonObj);
	virtual void HandleSDLEvent(SDL_Event &event);

	RefCountedPtr<CameraContext> GetCameraContext() const { return m_cameraContext; }

	ShipViewController shipView;

	int GetActiveWeapon() const;

	std::tuple<double, double, double> CalculateHeadingPitchRoll(enum PlaneType);

	vector3d WorldSpaceToScreenSpace(const Body *body) const;
	vector3d WorldSpaceToScreenSpace(const vector3d &position) const;
	vector3d ShipSpaceToScreenSpace(const vector3d &position) const;
	vector3d GetTargetIndicatorScreenPosition(const Body *body) const;
	vector3d GetMouseDirection() const;
	vector3d CameraSpaceToScreenSpace(const vector3d &pos) const;

	void BeginCameraFrame() { m_cameraContext->BeginFrame(); };
	void EndCameraFrame() { m_cameraContext->EndFrame(); };

	bool ShouldShowLabels() { return m_labelsOn; }

protected:
	virtual void BuildUI(UI::Single *container);
	virtual void OnSwitchTo();
	virtual void OnSwitchFrom();

private:
	void InitObject();

	void RefreshButtonStateAndVisibility();

	enum IndicatorSide {
		INDICATOR_HIDDEN,
		INDICATOR_ONSCREEN,
		INDICATOR_LEFT,
		INDICATOR_RIGHT,
		INDICATOR_TOP,
		INDICATOR_BOTTOM
	};

	struct Indicator {
		vector2f pos;
		vector2f realpos;
		IndicatorSide side;
		Gui::Label *label;
		Indicator() :
			pos(0.0f, 0.0f),
			realpos(0.0f, 0.0f),
			side(INDICATOR_HIDDEN),
			label(0) {}
	};

	void UpdateProjectedObjects();
	void UpdateIndicator(Indicator &indicator, const vector3d &direction);
	void HideIndicator(Indicator &indicator);
	void SeparateLabels(Gui::Label *a, Gui::Label *b);

	void OnToggleLabels();

	void DrawCombatTargetIndicator(const Indicator &target, const Indicator &lead, const Color &c);
	void DrawEdgeMarker(const Indicator &marker, const Color &c);

	void OnPlayerChangeTarget();
	/// Handler for "requestTimeAccelerationInc" event
	void OnRequestTimeAccelInc();
	/// Handler for "requestTimeAccelerationDec" event
	void OnRequestTimeAccelDec();
	void SelectBody(Body *, bool reselectIsDeselect);

	Game *m_game;

	NavTunnelWidget *m_navTunnel;
	std::unique_ptr<SpeedLines> m_speedLines;

	Gui::Label *m_pauseText;
	bool m_labelsOn;

	/* Only use #if WITH_DEVKEYS */
	Gui::Label *m_debugInfo;

	// useful docking locations for new-ui widgets in the HUD
	RefCountedPtr<UI::Widget> m_hudRoot;
	// new-ui HUD components

	Gui::VBox *m_hudSensorGaugeStack;

	sigc::connection m_onHyperspaceTargetChangedCon;
	sigc::connection m_onPlayerChangeTargetCon;
	sigc::connection m_onChangeFlightControlStateCon;
	sigc::connection m_onToggleHudModeCon;
	sigc::connection m_onIncTimeAccelCon;
	sigc::connection m_onDecTimeAccelCon;

	RefCountedPtr<CameraContext> m_cameraContext;
	std::unique_ptr<Camera> m_camera;

	Indicator m_combatTargetIndicator;
	Indicator m_targetLeadIndicator;

	Graphics::RenderState *m_blendState;

	Graphics::Drawables::Line3D m_edgeMarker;
	Graphics::Drawables::Lines m_indicator;

	static struct InputBinding {
		typedef KeyBindings::ActionBinding ActionBinding;
		typedef KeyBindings::AxisBinding AxisBinding;

		ActionBinding *toggleHudMode;
		ActionBinding *increaseTimeAcceleration;
		ActionBinding *decreaseTimeAcceleration;
	} InputBindings;
};

class NavTunnelWidget : public Gui::Widget {
public:
	NavTunnelWidget(WorldView *worldView, Graphics::RenderState *);
	virtual void Draw();
	virtual void GetSizeRequested(float size[2]);
	void DrawTargetGuideSquare(const vector2f &pos, const float size, const Color &c);

private:
	void CreateVertexBuffer(const Uint32 size);

	WorldView *m_worldView;
	Graphics::RenderState *m_renderState;
	RefCountedPtr<Graphics::Material> m_material;
	std::unique_ptr<Graphics::VertexBuffer> m_vbuffer;
};

#endif /* _WORLDVIEW_H */
