#ifndef _WORLDVIEW_H
#define _WORLDVIEW_H

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include "render/Render.h"
#include "Serializer.h"
#include "Background.h"
#include "EquipType.h"

class Body;
class Frame;
class LabelSet;
class Ship;

class WorldView: public View {
public:
	WorldView();
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
	void ApplyExternalViewRotation(matrix4x4d &m);
	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);
	enum CamType { CAM_FRONT, CAM_REAR, CAM_EXTERNAL };
	void SetCamType(enum CamType);
	enum CamType GetCamType() const;
	int GetNumLights() const { return m_numLights; }
	void ToggleTargetActions();
	void GetNearFarClipPlane(float *outNear, float *outFar) const;
	int GetActiveWeapon() const;
	void OnClickBlastoff();

	sigc::signal<void> onChangeCamType;

	double m_externalViewRotX, m_externalViewRotY;
	double m_externalViewDist;

private:
	void RefreshButtonStateAndVisibility();
	void UpdateCommsOptions();
	void ProjectObjsToScreenPos(const Frame *cam_frame);
	void DrawTargetSquares();
	void DrawTargetSquare(const Body* const target);
	void DrawCombatTargetIndicator(const Ship* const target);

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
	Background::Starfield m_starfield;
	Background::MilkyWay m_milkyWay;

#if DEVKEYS
	Gui::Label *m_debugInfo;
#endif

	Gui::Label *m_hudVelocity, *m_hudTargetDist, *m_hudAltitude, *m_hudPressure, *m_hudHyperspaceInfo, *m_hudTargetInfo;
	Gui::MeterBar *m_hudHullTemp, *m_hudWeaponTemp, *m_hudHullIntegrity, *m_hudShieldIntegrity;
	Gui::MeterBar *m_hudTargetHullIntegrity, *m_hudTargetShieldIntegrity;

	sigc::connection m_onHyperspaceTargetChangedCon;
	sigc::connection m_onPlayerEquipmentChangeCon;

	sigc::connection m_onPlayerChangeTargetCon;
	sigc::connection m_onChangeFlightControlStateCon;
	sigc::connection m_onMouseButtonDown;

	Gui::LabelSet *m_bodyLabels;
	Gui::Label *m_targetDist, *m_targetSpeed, *m_combatDist, *m_combatSpeed;
	bool m_velocityIndicatorOnscreen;
	int m_velocityIndicatorPos[2];
	bool m_targLeadOnscreen;
	vector3d m_targLeadPos;
	bool m_navVelocityIndicatorOnscreen;
	int m_navVelocityIndicatorPos[2];
};

#endif /* _WORLDVIEW_H */
