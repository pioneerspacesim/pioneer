#ifndef _VIEW_H
#define _VIEW_H

#include "libs.h"
#include "Gui.h"

/*
 * For whatever draws crap into the main area of the screen.
 * Eg:
 *  game 3d view
 *  system map
 *  sector map
 */
class View: public Gui::Fixed {
public:
	View(): Gui::Fixed(0, 64, 640, 416) {
		m_rightButtonBar = new Gui::Fixed(512, 0, 128, 26);
		m_rightButtonBar->SetBgColor(.65, .65, .65);

		m_rightRegion2 = new Gui::Fixed(517, 26, 122, 17);
		m_rightRegion2->SetTransparency(true);
	}
	virtual ~View() { delete m_rightButtonBar; delete m_rightRegion2; }
	virtual void ShowAll() {
		m_rightButtonBar->ShowAll();
		m_rightRegion2->ShowAll();
		Gui::Fixed::ShowAll();
	}
	virtual void HideAll() {
		m_rightButtonBar->HideAll();
		m_rightRegion2->HideAll();
		Gui::Fixed::HideAll();
	}
	// called before Gui::Draw will call widget ::Draw methods.
	virtual void Draw3D() = 0;
	// for checking key states, mouse crud
	virtual void Update() = 0;
protected:
	// each view can put some buttons in the bottom right of the cpanel
	Gui::Fixed *m_rightButtonBar;
//	Gui::Fixed *m_rightRegion1;
	Gui::Fixed *m_rightRegion2;
};

#endif /* _VIEW_H */
