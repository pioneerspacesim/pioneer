#ifndef _VIEW_H
#define _VIEW_H

#include "libs.h"
#include "Serializer.h"
#include "gui/Gui.h"

namespace Graphics { class Renderer; }

/*
 * For whatever draws crap into the main area of the screen.
 * Eg:
 *  game 3d view
 *  system map
 *  sector map
 */
class View: public Gui::Fixed {
public:
	View();
	virtual ~View();
	virtual void ShowAll();
	virtual void HideAll();
	// called before Gui::Draw will call widget ::Draw methods.
	virtual void Draw3D() = 0;
	// for checking key states, mouse crud
	virtual void Update() = 0;
	virtual void Save(Serializer::Writer &wr) {}
	virtual void Load(Serializer::Reader &rd) {}
	virtual void OnSwitchTo() = 0;

	void SetRenderer(Graphics::Renderer *r) { m_renderer = r; }

protected:
	// each view can put some buttons in the bottom right of the cpanel
	Gui::Fixed *m_rightButtonBar;
	Gui::Fixed *m_rightRegion1;
	Gui::Fixed *m_rightRegion2;
	Graphics::Renderer *m_renderer;
};

#endif /* _VIEW_H */
