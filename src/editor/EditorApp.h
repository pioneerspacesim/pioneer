// Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
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

	class EditorApp : public GuiApplication {
	public:
		EditorApp();
		~EditorApp();

		static EditorApp *Get();

		void Initialize(argh::parser &cmdline);

		void AddLoadingTask(TaskSet::Handle handle);

	protected:
		void OnStartup() override;
		void OnShutdown() override;

		void PreUpdate() override;
		void PostUpdate() override;

		friend class LoadingPhase;
		std::vector<TaskSet::Handle> &GetLoadingTasks() { return m_loadingTasks; }

	private:
		std::vector<TaskSet::Handle> m_loadingTasks;
		Graphics::Renderer *m_renderer;
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
		float minRuntime = 2.f;
	};

} // namespace Editor
