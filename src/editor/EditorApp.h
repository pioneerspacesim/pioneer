// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "core/GuiApplication.h"
#include "core/TaskGraph.h"

namespace argh {
	class parser;
} // namespace argh

namespace Graphics {
	class Renderer;
} // namespace Graphics

namespace Editor {
	class Modal;

	class EditorApp : public GuiApplication {
	public:
		EditorApp();
		~EditorApp();

		static EditorApp *Get();

		void Initialize(argh::parser &cmdline);

		void AddLoadingTask(TaskSet::Handle handle);

		void SetAppName(std::string_view name);

		template<typename T, typename ...Args>
		RefCountedPtr<T> PushModal(Args&& ...args)
		{
			T *modal = new T(this, args...);
			PushModalInternal(modal);
			return RefCountedPtr<T>(modal);
		}

	protected:
		void OnStartup() override;
		void OnShutdown() override;

		void PreUpdate() override;
		void PostUpdate() override;

		friend class LoadingPhase;
		std::vector<TaskSet::Handle> &GetLoadingTasks() { return m_loadingTasks; }

	private:
		void PushModalInternal(Modal *m);

		std::vector<TaskSet::Handle> m_loadingTasks;
		Graphics::Renderer *m_renderer;

		std::vector<RefCountedPtr<Modal>> m_modalStack;
		std::unique_ptr<IniConfig> m_editorCfg;

		std::string m_appName;
	};

	class LoadingPhase : public Application::Lifecycle {
	public:
		LoadingPhase(EditorApp *app) :
			Application::Lifecycle(true),
			m_app(app)
		{}

	protected:
		void Update(float dt) override;

	private:
		EditorApp *m_app;
		float minRuntime = 1.f;
	};

	class EditorWelcomeScreen : public Application::Lifecycle {
	public:
		EditorWelcomeScreen(EditorApp *app) :
			Application::Lifecycle(),
			m_app(app)
		{}

	protected:
		void Update(float dt) override;

	private:
		EditorApp *m_app;
	};

} // namespace Editor
