// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
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
	int GetActiveWeapon() const;

	void ResetHyperspaceButton();

	sigc::signal<void> onChangeCamType;

	const Indicator *GetNavIndicator() const { return &m_navTargetIndicator; }
	const Indicator *GetNavProgradeIndicator() const { return &m_navVelIndicator; }
	const Indicator *GetNavRetrogradeIndicator() const { return &m_retroNavVelIndicator; }
	const Indicator *GetFrameIndicator() const { return &m_frameIndicator; }
	const Indicator *GetFrameProgradeIndicator() const { return &m_velIndicator; }
	const Indicator *GetFrameRetrogradeIndicator() const { return &m_retroVelIndicator; }
	const Indicator *GetForwardIndicator() const { return &m_forwardIndicator; }
	const Indicator *GetBackwardIndicator() const { return &m_backwardIndicator; }
	const Indicator *GetLeftIndicator() const { return &m_leftIndicator; }
	const Indicator *GetRightIndicator() const { return &m_rightIndicator; }
	const Indicator *GetUpIndicator() const { return &m_upIndicator; }
	const Indicator *GetDownIndicator() const { return &m_downIndicator; }
	const Indicator *GetNormalIndicator() const { return &m_normalIndicator; }
	const Indicator *GetAntiNormalIndicator() const { return &m_antiNormalIndicator; }
	const Indicator *GetRadialInIndicator() const { return &m_radialInIndicator; }
	const Indicator *GetRadialOutIndicator() const { return &m_radialOutIndicator; }
	const Indicator *GetAwayFromFrameIndicator() const { return &m_awayFromFrameIndicator; }
	const Indicator *GetCombatTargetIndicator() const { return &m_combatTargetIndicator; }
	const Indicator *GetCombatTargetLeadIndicator() const { return &m_targetLeadIndicator; }
	const Indicator *GetManeuverIndicator() const { return &m_burnIndicator; }
	
	const vector3d GetNavProgradeVelocity() const;
	const vector3d GetFrameProgradeVelocity() const;

	const vector3d GetProjectedScreenPos(Body *body) const { if(m_projectedPos.find(body) != m_projectedPos.end()) return m_projectedPos.at(body); else return vector3d(0,0,0); }
protected:
	virtual void BuildUI(UI::Single *container);
	virtual void OnSwitchTo();
	virtual void OnSwitchFrom();
private:
	void InitObject();

	void RefreshHyperspaceButton();
	void RefreshButtonStateAndVisibility();

	void ChangeInternalCameraMode(InternalCameraController::Mode m);

	void UpdateProjectedObjects();
	void UpdateIndicator(Indicator &indicator, const vector3d &direction);
	void HideIndicator(Indicator &indicator);

	void OnToggleLabels();

	void OnClickHyperspace(Gui::MultiStateImageButton *b);
	void OnChangeFlightState(Gui::MultiStateImageButton *b);
	void OnHyperspaceTargetChanged();
	void OnPlayerChangeTarget();
	void OnPlayerChangeFlightControlState();
	/// Handler for "requestTimeAccelerationInc" event
	void OnRequestTimeAccelInc();
	/// Handler for "requestTimeAccelerationDec" event
	void OnRequestTimeAccelDec();
	void SelectBody(Body *, bool reselectIsDeselect);
	Body* PickBody(const double screenX, const double screenY) const;
	void MouseWheel(bool up);
	bool OnClickHeadingLabel(void);
	void RefreshHeadingPitch(void);

	Game* m_game;

	PlaneType m_curPlane;
	NavTunnelWidget *m_navTunnel;
	std::unique_ptr<SpeedLines> m_speedLines;

	Gui::Label *m_flightStatus, *m_debugText;
	Gui::MultiStateImageButton *m_flightControlButton;
	Gui::MultiStateImageButton *m_hyperspaceButton;
	bool m_labelsOn;
	enum CamType m_camType;

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

	Gui::Label *m_hudVelocity, *m_hudTargetDist, *m_hudAltitude, *m_hudPressure, *m_hudTargetInfo;
	Gui::MeterBar *m_hudHullTemp, *m_hudWeaponTemp, *m_hudHullIntegrity, *m_hudShieldIntegrity;
	Gui::MeterBar *m_hudTargetHullIntegrity, *m_hudTargetShieldIntegrity;
	Gui::MeterBar *m_hudFuelGauge;
	Gui::VBox *m_hudSensorGaugeStack;

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
	Indicator m_retroNavVelIndicator;
	Indicator m_navTargetIndicator;
	Indicator m_combatTargetIndicator;
	Indicator m_targetLeadIndicator;
	Indicator m_mouseDirIndicator;
	Indicator m_frameIndicator;
	Indicator m_forwardIndicator;
	Indicator m_backwardIndicator;
	Indicator m_upIndicator;
	Indicator m_downIndicator;
	Indicator m_leftIndicator;
	Indicator m_rightIndicator;
	Indicator m_normalIndicator;
	Indicator m_antiNormalIndicator;
	Indicator m_radialInIndicator;
	Indicator m_radialOutIndicator;
	Indicator m_awayFromFrameIndicator;

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
