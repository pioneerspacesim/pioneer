// Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "EditorApp.h"

#include "EditorDraw.h"

#include "FileSystem.h"
#include "ModManager.h"
#include "core/IniConfig.h"
#include "graphics/Graphics.h"
#include "lua/Lua.h"
#include "graphics/opengl/RendererGL.h"

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
}

void EditorApp::AddLoadingTask(TaskSet::Handle handle)
{
	m_loadingTasks.emplace_back(std::move(handle));
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

	Lua::Init();

	ModManager::Init();

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
