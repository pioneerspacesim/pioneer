#ifndef _SHIPCPANEL_H
#define _SHIPCPANEL_H

#include "libs.h"
#include "Gui.h"

class Body;
class SpaceStation;
// MultiFuncDisplay types
class ScannerWidget;
class MsgLogWidget;

enum multifuncfunc_t {
	MFUNC_SCANNER,
	MFUNC_AUTOPILOT,
	MFUNC_EQUIPMENT,
	MFUNC_MSGLOG,
	MFUNC_MAX
};

class ShipCpanel: public Gui::Fixed {
public:
	ShipCpanel();
	virtual ~ShipCpanel();
	virtual void Draw();
	void SetScannerWidget(Widget *w); // must be done each frame
	void SetTemporaryMessage(const Body *sender, const std::string &msg);
	void SetTemporaryMessage(const std::string &sender, const std::string &msg);
private:
	void OnChangeCamView(Gui::MultiStateImageButton *b);
	void OnChangeMapView(Gui::MultiStateImageButton *b);
	void OnChangeInfoView(Gui::MultiStateImageButton *b);
	void OnClickTimeaccel(Gui::ISelectable *i, double step);
	void OnClickComms(Gui::MultiStateImageButton *b);
	void OnDockingClearanceExpired(const SpaceStation *);
	void OnChangeMultiFunctionDisplay(multifuncfunc_t f);

	Widget *m_scannerWidget;
	Gui::Label *m_clock;

	Gui::Label *tempMsg;
	float tempMsgAge;
	struct QueuedMsg {
		QueuedMsg(std::string s, std::string m): sender(s), message(m) {}
		std::string sender;
		std::string message;
	};
	std::list<QueuedMsg> m_msgQueue;
	sigc::connection m_connOnDockingClearanceExpired;

	ScannerWidget *m_scanner;
	MsgLogWidget *m_msglog;
};

#endif /* _SHIP_CPANEL_H */
