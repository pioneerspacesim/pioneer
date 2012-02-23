#ifndef _SHIPCPANEL_H
#define _SHIPCPANEL_H

#include "libs.h"
#include "gui/Gui.h"
#include "ShipCpanelMultiFuncDisplays.h"
#include "Ship.h"
#include "Serializer.h"
#include "Game.h"
#include "WorldView.h"

class Body;
class SpaceStation;
namespace Graphics { class Renderer; }
class CameraSwitchWidget;

class ShipCpanel: public Gui::Fixed {
	friend class CameraSwitchWidget;

public:
	ShipCpanel(Graphics::Renderer *r);
    ShipCpanel(Serializer::Reader &rd, Graphics::Renderer *r);
	virtual ~ShipCpanel();
	virtual void Draw();
	void Update();
	MsgLogWidget *MsgLog() { return m_msglog; }
	void SetAlertState(Ship::AlertState as);

	void TimeStepUpdate(float step);

	void Save(Serializer::Writer &wr);

private:
	void InitObject();

	enum MapView { MAP_SECTOR, MAP_SYSTEM, MAP_INFO, MAP_GALACTIC };

	void OnChangeCamView(Gui::MultiStateImageButton *b);
	void OnChangeToMapView(Gui::MultiStateImageButton *b);
	void OnChangeMapView(enum MapView);
	void OnChangeInfoView(Gui::MultiStateImageButton *b);
	void OnClickTimeaccel(Game::TimeAccel val);
	void OnClickComms(Gui::MultiStateImageButton *b);
	void OnDockingClearanceExpired(const SpaceStation *);

	void OnUserChangeMultiFunctionDisplay(multifuncfunc_t f);
	void ChangeMultiFunctionDisplay(multifuncfunc_t selected);
	void OnMultiFuncGrabFocus(multifuncfunc_t);
	void OnMultiFuncUngrabFocus(multifuncfunc_t);
	void HideMapviewButtons();

	void SwitchToCamera(WorldView::CamType t);

	enum MapView m_currentMapView;
	multifuncfunc_t m_userSelectedMfuncWidget;
	Gui::Label *m_clock;

	sigc::connection m_connOnDockingClearanceExpired;

	MultiFuncSelectorWidget *m_mfsel;
	ScannerWidget *m_scanner;
	MsgLogWidget *m_msglog;
	UseEquipWidget *m_useEquipWidget;
	Gui::MultiStateImageButton *m_camButton;
	Gui::RadioGroup *m_leftButtonGroup, *m_rightButtonGroup;
	Gui::ImageRadioButton *m_timeAccelButtons[6];
	Gui::Widget *m_mapViewButtons[4];
	Gui::Image *m_alertLights[3];
};

#endif /* _SHIP_CPANEL_H */
