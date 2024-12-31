// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

namespace Editor
{
	class EditorApp;

	class EditorWindow {
	public:
		EditorWindow(EditorApp *app);
		~EditorWindow();

		virtual void OnAppearing() = 0;
		virtual void OnDisappearing() = 0;

		virtual void Update(float deltaTime);

		virtual const char *GetWindowName() = 0;

		bool GetCanBeClosed() const { return m_canBeClosed; }
		void SetCanBeClosed(bool closeable) { m_canBeClosed = closeable; }

	protected:

		virtual void OnDraw() = 0;

		virtual bool OnCloseRequested() = 0;

		EditorApp *GetApp() { return m_app; }

	private:
		EditorApp *m_app;
		bool m_canBeClosed;
	};
}
