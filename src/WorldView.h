// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _WORLDVIEW_H
#define _WORLDVIEW_H

#include "CameraController.h"
#include "KeyBindings.h"
#include "SpeedLines.h"
#include "UIView.h"
#include "gui/GuiWidget.h"

class Body;
class Frame;
class LabelSet;
class Ship;
class NavTunnelWidget;
class Game;
class SpeedLines;

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
	enum CamType {
		CAM_INTERNAL,
		CAM_EXTERNAL,
		CAM_SIDEREAL,
		CAM_FLYBY
	};
	void SetCamType(enum CamType);
	enum CamType GetCamType() const { return m_camType; }
	CameraController *GetCameraController() const { return m_activeCameraController; }

	/* start deprecated */
	void ChangeFlightState();
	/* end deprecated */

	int GetActiveWeapon() const;
	void OnClickBlastoff();

	sigc::signal<void> onChangeCamType;

	std::tuple<double, double, double> CalculateHeadingPitchRoll(enum PlaneType);

	vector3d WorldSpaceToScreenSpace(Body *body) const;
	vector3d WorldSpaceToScreenSpace(vector3d position) const;
	vector3d ShipSpaceToScreenSpace(vector3d position) const;
	vector3d GetTargetIndicatorScreenPosition(Body *body) const;
	vector3d GetMouseDirection() const;
	vector3d CameraSpaceToScreenSpace(vector3d pos) const;

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

	void ChangeInternalCameraMode(InternalCameraController::Mode m);
	void UpdateCameraName();

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
	void DrawImageIndicator(const Indicator &marker, Gui::TexturedQuad *quad, const Color &c);
	void DrawEdgeMarker(const Indicator &marker, const Color &c);

	void OnPlayerDockOrUndock();
	void OnPlayerChangeTarget();
	void OnPlayerChangeFlightControlState();
	/// Handler for "requestTimeAccelerationInc" event
	void OnRequestTimeAccelInc();
	/// Handler for "requestTimeAccelerationDec" event
	void OnRequestTimeAccelDec();
	void SelectBody(Body *, bool reselectIsDeselect);
	void MouseWheel(bool up);

	Game *m_game;

	NavTunnelWidget *m_navTunnel;
	std::unique_ptr<SpeedLines> m_speedLines;

	Gui::Label *m_pauseText;
	bool m_labelsOn;
	enum CamType m_camType;

	/* Only use #if WITH_DEVKEYS */
	Gui::Label *m_debugInfo;

	// useful docking locations for new-ui widgets in the HUD
	RefCountedPtr<UI::Widget> m_hudRoot;
	// new-ui HUD components

	Gui::VBox *m_hudSensorGaugeStack;

	sigc::connection m_onHyperspaceTargetChangedCon;
	sigc::connection m_onPlayerChangeTargetCon;
	sigc::connection m_onChangeFlightControlStateCon;
	sigc::connection m_onMouseWheelCon;
	sigc::connection m_onToggleHudModeCon;
	sigc::connection m_onIncTimeAccelCon;
	sigc::connection m_onDecTimeAccelCon;

	RefCountedPtr<CameraContext> m_cameraContext;
	std::unique_ptr<Camera> m_camera;
	std::unique_ptr<InternalCameraController> m_internalCameraController;
	std::unique_ptr<ExternalCameraController> m_externalCameraController;
	std::unique_ptr<SiderealCameraController> m_siderealCameraController;
	std::unique_ptr<FlyByCameraController> m_flybyCameraController;
	CameraController *m_activeCameraController; //one of the above

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

		AxisBinding *viewZoom;

		ActionBinding *frontCamera;
		ActionBinding *rearCamera;
		ActionBinding *leftCamera;
		ActionBinding *rightCamera;
		ActionBinding *topCamera;
		ActionBinding *bottomCamera;

		AxisBinding *cameraRoll;
		AxisBinding *cameraPitch;
		AxisBinding *cameraYaw;
		ActionBinding *resetCamera;
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
