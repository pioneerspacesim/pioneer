#ifndef _WORLDVIEW_H
#define _WORLDVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include "Serializer.h"
#include "Background.h"
#include "EquipType.h"
#include "Camera.h"

class Body;
class Frame;
class LabelSet;
class Ship;

class WorldView: public View {
public:
	WorldView();
	WorldView(Serializer::Reader &reader);
	virtual ~WorldView();
	virtual void ShowAll();
	virtual void Update();
	virtual void Draw3D();
	virtual void Draw();
	virtual void OnSwitchTo();
	static const double PICK_OBJECT_RECT_SIZE;
	bool GetShowLabels() { return m_labelsOn; }
	void DrawBgStars();
	vector3d GetExternalViewTranslation();
	matrix4x4d GetExternalViewRotation();
	vector3d GetSiderealViewTranslation();
	matrix4x4d GetSiderealViewRotation();
	virtual void Save(Serializer::Writer &wr);
	enum CamType { CAM_FRONT, CAM_REAR, CAM_EXTERNAL, CAM_SIDEREAL };
	void SetCamType(enum CamType);
	enum CamType GetCamType() const;
	int GetNumLights() const { return m_numLights; }
	void ToggleTargetActions();
	void ShowTargetActions();
	void HideTargetActions();
	int GetActiveWeapon() const;
	void OnClickBlastoff();

	sigc::signal<void> onChangeCamType;

	double m_externalViewRotX, m_externalViewRotY;
	double m_externalViewDist;
	
	matrix4x4d m_siderealViewOrient;
	double m_siderealViewDist;

private:
	void InitObject();

	void RefreshButtonStateAndVisibility();
	void UpdateCommsOptions();

	enum IndicatorSide {
		INDICATOR_HIDDEN,
		INDICATOR_ONSCREEN,
		INDICATOR_LEFT,
		INDICATOR_RIGHT,
		INDICATOR_TOP,
		INDICATOR_BOTTOM
	};

	struct Indicator {
		float pos[2];
		IndicatorSide side;
		Gui::Label *label;
		Indicator() {
			pos[0] = pos[1] = 0;
			side = INDICATOR_HIDDEN;
			label = 0;
		}
	};

	void UpdateSiderealView();
	
	void UpdateProjectedObjects();
	void UpdateIndicator(Indicator &indicator, const vector3d &direction);
	void HideIndicator(Indicator &indicator);
	void SeparateLabels(Gui::Label *a, Gui::Label *b);

	void DrawCrosshair(float px, float py, float sz, const Color &c);
	void DrawCombatTargetIndicator(const Indicator &target, const Indicator &lead, const Color &c);
	void DrawTargetSquare(const Indicator &marker, const Color &c);
	void DrawVelocityIndicator(const Indicator &marker, const Color &c);
	void DrawCircleIndicator(const Indicator &marker, const Color &c);
	void DrawImageIndicator(const Indicator &marker, const char *icon_path, const Color &c);
	void DrawEdgeMarker(const Indicator &marker, const Color &c);

	Gui::Button *AddCommsOption(const std::string msg, int ypos, int optnum);
	void AddCommsNavOption(const std::string msg, Body *target);
	void OnClickCommsNavOption(Body *target);
	void BuildCommsNavOptions();

	void OnClickHyperspace();
	void OnChangeWheelsState(Gui::MultiStateImageButton *b);
	void OnChangeLabelsState(Gui::MultiStateImageButton *b);
	void OnChangeFlightState(Gui::MultiStateImageButton *b);
	void OnHyperspaceTargetChanged();
	void OnPlayerEquipmentChange(Equip::Type e);
	void OnPlayerDockOrUndock();
	void OnPlayerChangeTarget();
	void OnPlayerChangeFlightControlState();
	void SelectBody(Body *, bool reselectIsDeselect);
	Body* PickBody(const double screenX, const double screenY) const;
	void MouseButtonDown(int button, int x, int y);

	matrix4x4d m_prevShipOrient;
	
	Gui::ImageButton *m_hyperspaceButton;
	bool m_showHyperspaceButton;

	Gui::Fixed *m_commsOptions;
	Gui::VBox *m_commsNavOptions;
	Gui::HBox *m_commsNavOptionsContainer;
	Gui::Label *m_flightStatus, *m_debugText;
	Gui::ImageButton *m_launchButton;
	Gui::MultiStateImageButton *m_wheelsButton;
	Gui::MultiStateImageButton *m_flightControlButton;
	bool m_labelsOn;
	enum CamType m_camType;
	int m_numLights;
	Uint32 m_showTargetActionsTimeout;

#if WITH_DEVKEYS
	Gui::Label *m_debugInfo;
#endif

	Gui::Label *m_hudVelocity, *m_hudTargetDist, *m_hudAltitude, *m_hudPressure, *m_hudHyperspaceInfo, *m_hudTargetInfo;
	Gui::MeterBar *m_hudHullTemp, *m_hudWeaponTemp, *m_hudHullIntegrity, *m_hudShieldIntegrity;
	Gui::MeterBar *m_hudTargetHullIntegrity, *m_hudTargetShieldIntegrity;
	Gui::MeterBar *m_hudFuelGauge;

	sigc::connection m_onHyperspaceTargetChangedCon;
	sigc::connection m_onPlayerEquipmentChangeCon;

	sigc::connection m_onPlayerChangeTargetCon;
	sigc::connection m_onChangeFlightControlStateCon;
	sigc::connection m_onMouseButtonDown;

	Gui::LabelSet *m_bodyLabels;
	std::map<Body*,vector3d> m_projectedPos;

	Camera *m_frontCamera, *m_rearCamera, *m_externalCamera, *m_siderealCamera;
	Camera *m_activeCamera;

	Indicator m_velIndicator;
	Indicator m_navVelIndicator;
	Indicator m_navTargetIndicator;
	Indicator m_combatTargetIndicator;
	Indicator m_targetLeadIndicator;
	Indicator m_mouseDirIndicator;
};

#endif /* _WORLDVIEW_H */
