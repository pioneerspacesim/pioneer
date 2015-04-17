// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _WORLDVIEW_H
#define _WORLDVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "gui/GuiWidget.h"
#include "UIView.h"
#include "Serializer.h"
#include "SpeedLines.h"
#include "Background.h"
#include "Camera.h"
#include "CameraController.h"

class Body;
class Frame;
class LabelSet;
class Ship;
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

namespace Gui { class TexturedQuad; }

namespace UI {
	class Widget;
	class Single;
	class Label;
}

class WorldView: public UIView {
public:
	friend class NavTunnelWidget;
	WorldView(Game* game);
	WorldView(const Json::Value &jsonObj, Game* game);
	virtual ~WorldView();
	virtual void ShowAll();
	virtual void Update();
	virtual void Draw3D();
	virtual void Draw();
	static const double PICK_OBJECT_RECT_SIZE;
	virtual void SaveToJson(Json::Value &jsonObj);
	enum CamType {
		CAM_INTERNAL,
		CAM_EXTERNAL,
		CAM_SIDEREAL
	};
	void SetCamType(enum CamType);
	enum CamType GetCamType() const { return m_camType; }
	CameraController *GetCameraController() const { return m_activeCameraController; }
	void ToggleTargetActions();
	void ShowTargetActions();
	void HideTargetActions();
	int GetActiveWeapon() const;
	void OnClickBlastoff();

	sigc::signal<void> onChangeCamType;

protected:
	virtual void BuildUI(UI::Single *container);
	virtual void OnSwitchTo();
	virtual void OnSwitchFrom();
private:
	void InitObject();

	void RefreshHyperspaceButton();
	void RefreshButtonStateAndVisibility();
	void UpdateCommsOptions();

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
		Indicator(): pos(0.0f, 0.0f), realpos(0.0f, 0.0f), side(INDICATOR_HIDDEN), label(0) {}
	};

	void UpdateProjectedObjects();
	void UpdateIndicator(Indicator &indicator, const vector3d &direction);
	void HideIndicator(Indicator &indicator);
	void SeparateLabels(Gui::Label *a, Gui::Label *b);

	void OnToggleLabels();

	void DrawCombatTargetIndicator(const Indicator &target, const Indicator &lead, const Color &c);
	void DrawTargetSquare(const Indicator &marker, const Color &c);
	void DrawVelocityIndicator(const Indicator &marker, VelIconType d, const Color &c);
	void DrawImageIndicator(const Indicator &marker, Gui::TexturedQuad *quad, const Color &c);
	void DrawEdgeMarker(const Indicator &marker, const Color &c);

	Gui::Button *AddCommsOption(const std::string &msg, int ypos, int xoffset, int optnum);
	void AddCommsNavOption(const std::string &msg, Body *target);
	void OnClickCommsNavOption(Body *target);
	void BuildCommsNavOptions();

	void HideLowThrustPowerOptions();
	void ShowLowThrustPowerOptions();
	void OnClickLowThrustPower();
	void OnSelectLowThrustPower(float power);

	void OnClickHyperspace();
	void OnChangeWheelsState(Gui::MultiStateImageButton *b);
	void OnChangeFlightState(Gui::MultiStateImageButton *b);
	void OnHyperspaceTargetChanged();
	void OnPlayerDockOrUndock();
	void OnPlayerChangeTarget();
	void OnPlayerChangeFlightControlState();
	void SelectBody(Body *, bool reselectIsDeselect);
	Body* PickBody(const double screenX, const double screenY) const;
	void MouseWheel(bool up);
	bool OnClickHeadingLabel(void);
	void RefreshHeadingPitch(void);

	Game* m_game;

	PlaneType m_curPlane;
	NavTunnelWidget *m_navTunnel;
	std::unique_ptr<SpeedLines> m_speedLines;

	Gui::ImageButton *m_hyperspaceButton;

	Gui::Label *m_pauseText;
	Gui::Label *m_showCameraName;
	Gui::Fixed *m_commsOptions;
	Gui::VBox *m_commsNavOptions;
	Gui::HBox *m_commsNavOptionsContainer;
	Gui::Fixed *m_lowThrustPowerOptions;
	Gui::Label *m_flightStatus, *m_debugText;
	Gui::ImageButton *m_launchButton;
	Gui::MultiStateImageButton *m_wheelsButton;
	Gui::MultiStateImageButton *m_flightControlButton;
	bool m_labelsOn;
	enum CamType m_camType;
	Uint32 m_showTargetActionsTimeout;
	Uint32 m_showLowThrustPowerTimeout;
	Uint32 m_showCameraNameTimeout;

#if WITH_DEVKEYS
	Gui::Label *m_debugInfo;
#endif

	// useful docking locations for new-ui widgets in the HUD
	RefCountedPtr<UI::Widget> m_hudRoot;
	RefCountedPtr<UI::Single> m_hudDockTop;
	RefCountedPtr<UI::Single> m_hudDockLeft;
	RefCountedPtr<UI::Single> m_hudDockRight;
	RefCountedPtr<UI::Single> m_hudDockBottom;
	RefCountedPtr<UI::Single> m_hudDockCentre;
	// new-ui HUD components
	RefCountedPtr<UI::Label> m_headingInfo, m_pitchInfo;

	Gui::Label *m_hudVelocity, *m_hudTargetDist, *m_hudAltitude, *m_hudPressure,
		   *m_hudHyperspaceInfo, *m_hudTargetInfo;
	Gui::MeterBar *m_hudHullTemp, *m_hudWeaponTemp, *m_hudHullIntegrity, *m_hudShieldIntegrity;
	Gui::MeterBar *m_hudTargetHullIntegrity, *m_hudTargetShieldIntegrity;
	Gui::MeterBar *m_hudFuelGauge;

	sigc::connection m_onHyperspaceTargetChangedCon;
	sigc::connection m_onPlayerChangeTargetCon;
	sigc::connection m_onChangeFlightControlStateCon;
	sigc::connection m_onMouseWheelCon;

	Gui::LabelSet *m_bodyLabels;
	std::map<Body*,vector3d> m_projectedPos;

	RefCountedPtr<CameraContext> m_cameraContext;
	std::unique_ptr<Camera> m_camera;
	std::unique_ptr<InternalCameraController> m_internalCameraController;
	std::unique_ptr<ExternalCameraController> m_externalCameraController;
	std::unique_ptr<SiderealCameraController> m_siderealCameraController;
	CameraController *m_activeCameraController; //one of the above

	Indicator m_velIndicator;
	Indicator m_navVelIndicator;
	Indicator m_burnIndicator;
	Indicator m_retroVelIndicator;
	Indicator m_navTargetIndicator;
	Indicator m_combatTargetIndicator;
	Indicator m_targetLeadIndicator;
	Indicator m_mouseDirIndicator;

	std::unique_ptr<Gui::TexturedQuad> m_indicatorMousedir;
	std::unique_ptr<Gui::TexturedQuad> m_frontCrosshair;
	std::unique_ptr<Gui::TexturedQuad> m_rearCrosshair;
	std::unique_ptr<Gui::TexturedQuad> m_progradeIcon;
	std::unique_ptr<Gui::TexturedQuad> m_retrogradeIcon;
	std::unique_ptr<Gui::TexturedQuad> m_burnIcon;
	std::unique_ptr<Gui::TexturedQuad> m_targetIcon;
	vector2f m_indicatorMousedirSize;

	Graphics::RenderState *m_blendState;

	Graphics::Drawables::Line3D m_edgeMarker;
	Graphics::Drawables::Lines m_crossHair;
	Graphics::Drawables::Lines m_indicator;
};

class NavTunnelWidget: public Gui::Widget {
public:
	NavTunnelWidget(WorldView *worldView, Graphics::RenderState*);
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
