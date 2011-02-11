#ifndef _SHIPCPANEL_H
#define _SHIPCPANEL_H

#include "libs.h"
#include "Gui.h"
#include "ShipCpanelMultiFuncDisplays.h"

class Body;
class SpaceStation;

class ShipCpanel: public Gui::Fixed {
public:
	ShipCpanel();
	virtual ~ShipCpanel();
	virtual void Draw();
	void Update();
	MsgLogWidget *MsgLog() { return m_msglog; }
private:
	enum MapView { MAP_SECTOR, MAP_SYSTEM, MAP_INFO, MAP_GALACTIC };

	void OnChangeCamView(Gui::MultiStateImageButton *b);
	void OnChangeToMapView(Gui::MultiStateImageButton *b);
	void OnChangeMapView(enum MapView);
	void OnChangeInfoView(Gui::MultiStateImageButton *b);
	void OnClickTimeaccel(int val);
	void OnClickComms(Gui::MultiStateImageButton *b);
	void OnDockingClearanceExpired(const SpaceStation *);

	void OnUserChangeMultiFunctionDisplay(multifuncfunc_t f);
	void ChangeMultiFunctionDisplay(multifuncfunc_t selected);
	void OnMultiFuncGrabFocus(multifuncfunc_t);
	void OnMultiFuncUngrabFocus(multifuncfunc_t);
	void HideMapviewButtons();

	enum MapView m_currentMapView;
	multifuncfunc_t m_userSelectedMfuncWidget;
	Gui::Label *m_clock;

	sigc::connection m_connOnDockingClearanceExpired;

	MultiFuncSelectorWidget *m_mfsel;
	ScannerWidget *m_scanner;
	MsgLogWidget *m_msglog;
	UseEquipWidget *m_useEquipWidget;
	Gui::ImageRadioButton *m_timeAccelButtons[6];
	Gui::Widget *m_mapViewButtons[4];
};

#endif /* _SHIP_CPANEL_H */
