#ifndef _SHIPCPANELMULTIFUNCDISPLAYS_H
#define _SHIPCPANELMULTIFUNCDISPLAYS_H

#include "Gui.h"
#include "ShipCpanel.h"

class MsgLogWidget;
class ScannerWidget;
class MultiFuncSelectorWidget;

class MsgLogWidget: public Gui::Fixed {
public:
	void GetSizeRequested(float size[2]);
	MsgLogWidget();
};

class ScannerWidget: public Gui::Widget {
public:
	void GetSizeRequested(float size[2]);
	void Draw();
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
	void OnClickButton(Gui::ISelectable *i, multifuncfunc_t f);

	int m_active;
	Gui::ImageRadioButton *m_buttons[MFUNC_MAX];
	Gui::RadioGroup *m_rg;
};

#endif /* _SHIPCPANELMULTIFUNCDISPLAYS_H */
