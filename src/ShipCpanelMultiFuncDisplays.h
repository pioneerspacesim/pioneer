#ifndef _SHIPCPANELMULTIFUNCDISPLAYS_H
#define _SHIPCPANELMULTIFUNCDISPLAYS_H

#include "Gui.h"
#include "ShipCpanel.h"

class MsgLogWidget;
class ScannerWidget;
class MultiFuncSelectorWidget;

class MsgLogWidget: public IMultiFunc, public Gui::Fixed {
public:
	MsgLogWidget();
	void GetSizeRequested(float size[2]);
	void PushMessage(const std::string &sender, const std::string &msg);
	virtual void Update();
private:
	float tempMsgAge;
	Gui::Label *tempMsg;
	struct QueuedMsg {
		QueuedMsg(std::string s, std::string m): sender(s), message(m) {}
		std::string sender;
		std::string message;
	};
	std::list<QueuedMsg> m_msgQueue;
};

class ScannerWidget: public IMultiFunc, public Gui::Widget {
public:
	void GetSizeRequested(float size[2]);
	void Draw();
	virtual void Update() {}
private:
	void DrawBlobs(bool below);
	void DrawDistanceRings();
};


class MultiFuncSelectorWidget: public Gui::Fixed {
public:
	MultiFuncSelectorWidget();
	sigc::signal<void, multifuncfunc_t> onSelect;
private:
	void UpdateButtons();
	void OnClickButton(multifuncfunc_t f);

	int m_active;
	Gui::ImageRadioButton *m_buttons[MFUNC_MAX];
	Gui::RadioGroup *m_rg;
};

#endif /* _SHIPCPANELMULTIFUNCDISPLAYS_H */
