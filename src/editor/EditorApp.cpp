// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "EditorApp.h"

#include "EditorDraw.h"

#include "FileSystem.h"
#include "Lang.h"
#include "ModManager.h"
#include "ModelViewer.h"
#include "SDL_keycode.h"
#include "EnumStrings.h"

#include "argh/argh.h"
#include "core/IniConfig.h"
#include "core/OS.h"
#include "graphics/Graphics.h"
#include "lua/Lua.h"
#include "graphics/opengl/RendererGL.h"

#include "system/SystemEditor.h"

using namespace Editor;

EditorApp::EditorApp() :
	GuiApplication("Pioneer Editor")
{
}

EditorApp::~EditorApp()
{
}

EditorApp *EditorApp::Get()
{
	static EditorApp app = EditorApp();
	return &app;
}

void EditorApp::Initialize(argh::parser &cmdline)
{
	// Load / register editor modules and frames here

	if (cmdline[{"-mv", "--modelviewer"}]) {
		std::string modelName;
		cmdline({"-mv", "--modelviewer"}, "") >> modelName;

		RefCountedPtr<ModelViewer> modelViewer(new ModelViewer(this, Lua::manager));

		if (!modelName.empty())
			modelViewer->SetModel(modelName);

		QueueLifecycle(modelViewer);
		SetAppName("ModelViewer");
		return;
	}

	if (cmdline("--system")) {
		std::string systemPath;
		cmdline("--system") >> systemPath;

		RefCountedPtr<SystemEditor> systemEditor(new SystemEditor(this));

		if (!systemPath.empty())
			systemEditor->LoadSystem(systemPath);

		QueueLifecycle(systemEditor);
		SetAppName("SystemEditor");
		return;
	}
}

void EditorApp::AddLoadingTask(TaskSet::Handle handle)
{
	m_loadingTasks.emplace_back(std::move(handle));
}

void EditorApp::SetAppName(std::string_view name)
{
	m_appName = name;
}

void EditorApp::OnStartup()
{
	Log::GetLog()->SetLogFile("editor.txt");

	IniConfig cfg;
	cfg.SetInt("ScrWidth", 1600);
	cfg.SetInt("ScrHeight", 900);
	cfg.SetInt("VSync", 1);
	cfg.SetInt("AntiAliasingMode", 4);

	cfg.Read(FileSystem::userFiles, "editor.ini");
	cfg.Save(); // write defaults if the file doesn't exist

	EnumStrings::Init();
	Lua::Init();
	ModManager::Init();

	ModManager::LoadMods(&cfg);

	// get threads up
	Uint32 numThreads = cfg.Int("WorkerThreads");
	numThreads = numThreads ? numThreads : std::max(OS::GetNumCores() - 1, 1U);
	GetTaskGraph()->SetWorkerThreads(numThreads);

	Lang::Resource &res(Lang::GetResource("core", cfg.String("Lang")));
	Lang::MakeCore(res);

	Graphics::RendererOGL::RegisterRenderer();

	m_renderer = StartupRenderer(&cfg);
	StartupInput(&cfg);

	StartupPiGui();

	// precache the editor font
	GetPiGui()->GetFont("pionillium", 13);
	GetPiGui()->SetDebugStyle();

	RefCountedPtr<LoadingPhase> loader (new LoadingPhase(this));
	QueueLifecycle(loader);
}

void EditorApp::OnShutdown()
{
	Lua::Uninit();
	Graphics::Uninit();

	ModManager::Uninit();

	ShutdownPiGui();
	ShutdownRenderer();
	ShutdownInput();
}

void EditorApp::PreUpdate()
{
	HandleEvents();
	GetPiGui()->NewFrame();
}

void EditorApp::PostUpdate()
{
	GetRenderer()->ClearDepthBuffer();
	GetPiGui()->Render();

	if (GetInput()->IsKeyReleased(SDLK_F11)) {
		GetRenderer()->FlushCommandBuffers();
		GetRenderer()->ReloadShaders();
	}

	bool ctrl = GetInput()->IsKeyDown(SDLK_LCTRL) || GetInput()->IsKeyDown(SDLK_RCTRL);
	bool shift = GetInput()->IsKeyDown(SDLK_LSHIFT) || GetInput()->IsKeyDown(SDLK_RSHIFT);

	if (ctrl && shift && GetInput()->IsKeyPressed(SDLK_p)) {
		RequestProfileFrame(m_appName);
	}
}

// ============================================================================
//  Loading Phase
// ============================================================================

void LoadingPhase::Update(float dt)
{
	constexpr const char *loadingMessage = "Loading...";

	const ImVec2 winSize = { float(Graphics::GetScreenWidth()), float(Graphics::GetScreenHeight()) };
	const ImVec2 textSize = ImGui::CalcTextSize(loadingMessage);

	ImGui::SetNextWindowBgAlpha(0.0);
	ImGui::SetNextWindowPos({ 0.f, 0.f }, ImGuiCond_Always);
	ImGui::SetNextWindowSize(winSize, ImGuiCond_Always);

	ImGui::Begin("##fullscreen", NULL,
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoInputs);

	// Animate the loading screen text while we wait
	ImColor col(1.f, 1.f, 1.f, cosf(m_app->GetTime() * 4.f));
	ImGui::PushStyleColor(ImGuiCol_Text, (ImU32) col);

	ImGui::SetCursorScreenPos((winSize - textSize) / 2.0);
	ImGui::TextUnformatted(loadingMessage);

	ImGui::PopStyleColor(1);

	ImGui::End();

	// ==========================================

	std::vector<TaskSet::Handle> &runningTasks = m_app->GetLoadingTasks();

	// Iterate all currently pending loading tasks and check for completion
	auto iter = runningTasks.begin();
	while (iter != runningTasks.end()) {
		if (iter->IsComplete()) {
			m_app->GetTaskGraph()->CompleteTaskSet(*iter);
			iter = runningTasks.erase(iter);
		} else {
			++iter;
		}
	}

	// Once all tasks are completed, we can end the loading screen
	if (runningTasks.size() == 0 && m_app->GetTime() >= minRuntime) {
		RequestEndLifecycle();
	}
}
