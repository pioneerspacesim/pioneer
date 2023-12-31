// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "RefCounted.h"

// forward declaration
using ImGuiID = unsigned int;

namespace Editor {

	class EditorApp;

	class Modal : public RefCounted {
	public:
		Modal(EditorApp *app, const char *title, bool canClose);

		bool Ready();
		void Close();

		virtual void Draw();

	protected:
		virtual void DrawInternal() {}

		EditorApp *m_app;
		const char *m_title;
		ImGuiID m_id;
		bool m_shouldClose;
		bool m_canClose;
	};

} // namespace Editor
