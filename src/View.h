#ifndef _VIEW_H
#define _VIEW_H

#include "libs.h"
#include "Serializer.h"
#include "gui/Gui.h"

/*
 * For whatever draws crap into the main area of the screen.
 * Eg:
 *  game 3d view
 *  system map
 *  sector map
 */
class View: public Gui::Fixed {
public:
	View(): Gui::Fixed(float(Gui::Screen::GetWidth()), float(Gui::Screen::GetHeight()-64)) {
		Gui::Screen::AddBaseWidget(this, 0, 0);
		
		m_rightButtonBar = new Gui::Fixed(128, 26);
		m_rightButtonBar->SetBgColor(.65f, .65f, .65f, 1.0f);
		Gui::Screen::AddBaseWidget(m_rightButtonBar, Gui::Screen::GetWidth()-128, Gui::Screen::GetHeight()-26);

		m_rightRegion2 = new Gui::Fixed(126, 17);
		m_rightRegion2->SetTransparency(true);
		Gui::Screen::AddBaseWidget(m_rightRegion2, Gui::Screen::GetWidth()-127, Gui::Screen::GetHeight()-45);
		
		m_rightRegion1 = new Gui::Fixed(122, 17);
		m_rightRegion1->SetTransparency(true);
		Gui::Screen::AddBaseWidget(m_rightRegion1, Gui::Screen::GetWidth()-123, Gui::Screen::GetHeight()-62);
	}
	virtual ~View() {
		Gui::Screen::RemoveBaseWidget(m_rightButtonBar);
		Gui::Screen::RemoveBaseWidget(m_rightRegion2);
		Gui::Screen::RemoveBaseWidget(m_rightRegion1);
		delete m_rightButtonBar;
		delete m_rightRegion2;
		delete m_rightRegion1;
	}
	virtual void ShowAll() {
		m_rightButtonBar->ShowAll();
		m_rightRegion2->ShowAll();
		m_rightRegion1->ShowAll();
		Gui::Fixed::ShowAll();
	}
	virtual void HideAll() {
		m_rightButtonBar->HideAll();
		m_rightRegion2->HideAll();
		m_rightRegion1->HideAll();
		Gui::Fixed::HideAll();
	}
	// called before Gui::Draw will call widget ::Draw methods.
	virtual void Draw3D() = 0;
	// for checking key states, mouse crud
	virtual void Update() = 0;
	virtual void Save(Serializer::Writer &wr) {}
	virtual void Load(Serializer::Reader &rd) {}
	virtual void OnSwitchTo() = 0;
protected:
	// each view can put some buttons in the bottom right of the cpanel
	Gui::Fixed *m_rightButtonBar;
	Gui::Fixed *m_rightRegion1;
	Gui::Fixed *m_rightRegion2;
};

#endif /* _VIEW_H */
