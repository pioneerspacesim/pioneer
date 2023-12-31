// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "ViewportWindow.h"

namespace Background {
	class Container;
}

class StarSystem;
class SystemMapViewport;

namespace Editor {

	class SystemEditor;

	class SystemEditorViewport : public ViewportWindow {
	public:
		SystemEditorViewport(EditorApp *app, SystemEditor *editor);
		~SystemEditorViewport();

		void SetSystem(RefCountedPtr<StarSystem> system);

		SystemMapViewport *GetMap() { return m_map.get(); }

	protected:
		void OnUpdate(float deltaTime) override;
		void OnRender(Graphics::Renderer *renderer) override;

		void OnHandleInput(bool clicked, bool released, ImVec2 mousePos) override;

		void OnDraw() override;
		bool OnCloseRequested() override;

		const char *GetWindowName() override { return "Viewport"; }

	private:

		void DrawTimelineControls();
		bool DrawIcon(ImGuiID id, const ImVec2 &iconPos, const ImColor &color, const char *icon, const char *label = nullptr);

		EditorApp *m_app;
		SystemEditor *m_editor;

		std::unique_ptr<SystemMapViewport> m_map;
		std::unique_ptr<Background::Container> m_background;
	};
};
