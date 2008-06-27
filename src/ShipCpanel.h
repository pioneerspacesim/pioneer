#ifndef _SHIPCPANEL_H
#define _SHIPCPANEL_H

#include "libs.h"
#include "Gui.h"

class ShipCpanel: public Gui::Fixed {
public:
	ShipCpanel();
	virtual void Draw();
	void SetScannerWidget(Widget *w); // must be done each frame
private:
	void OnChangeCamView(Gui::MultiStateImageButton *b);
	void OnChangeMapView(Gui::MultiStateImageButton *b);
	void OnChangeInfoView(Gui::MultiStateImageButton *b);
	void OnClickTimeaccel(Gui::ISelectable *i, double step);
	void OnClickComms();

	Widget *m_scannerWidget;
	Gui::Label *m_clock;
};

#endif /* _SHIP_CPANEL_H */
