// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
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
	View(const std::string &name);
	virtual ~View();
	// for checking key states, mouse crud
	virtual void Update() {};
	// called before DrawPiGui, do all 3d rendering to the framebuffer.
	virtual void Draw3D() {};
	// Called during the pigui frame to submit UI widgets
	virtual void DrawPiGui();
	virtual void SaveToJson(Json &jsonObj) {}
	virtual void LoadFromJson(const Json &jsonObj) {}

	void Attach();
	void Detach();

	void SetRenderer(Graphics::Renderer *r) { m_renderer = r; }

	const std::string &GetViewName() const { return m_handlerName; }
protected:
	virtual void OnSwitchTo() {}
	virtual void OnSwitchFrom() {}
	// Submit all C++-side UI contents before Lua UI submission is kicked
	virtual void DrawUIContents() {}

	Graphics::Renderer *m_renderer;
	std::string m_handlerName;
};

#endif /* _VIEW_H */
