// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _VIEW_H
#define _VIEW_H

#include "JsonFwd.h"

namespace Graphics {
	class Renderer;
}

/*
 * For whatever draws crap into the main area of the screen.
 * Eg:
 *  game 3d view
 *  system map
 *  sector map
 */
class View {
public:
	View();
	virtual ~View();
	virtual void Draw(){};
	// called before Gui::Draw will call widget ::Draw methods.
	virtual void Draw3D() = 0;
	// for checking key states, mouse crud
	virtual void Update() = 0;
	// Called during the pigui frame to draw UI
	virtual void DrawPiGui(){};
	virtual void SaveToJson(Json &jsonObj) {}
	virtual void LoadFromJson(const Json &jsonObj) {}

	void Attach();
	void Detach();

	void SetRenderer(Graphics::Renderer *r) { m_renderer = r; }

protected:
	virtual void OnSwitchTo() = 0;
	virtual void OnSwitchFrom() {}
	Graphics::Renderer *m_renderer;
};

#endif /* _VIEW_H */
