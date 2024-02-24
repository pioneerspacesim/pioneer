// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "EditorApp.h"

#include "EditorDraw.h"
#include "EditorIcons.h"
#include "MathUtil.h"
#include "Modal.h"

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
#include "imgui/imgui.h"
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

	if (cmdline["--system"]) {
		std::string systemPath = cmdline[1];

		RefCountedPtr<SystemEditor> systemEditor(new SystemEditor(this));

		if (!systemPath.empty()) {
			systemPath = FileSystem::JoinPathBelow(FileSystem::GetDataDir(), systemPath);
			systemEditor->LoadSystemFromDisk(systemPath);
		}

		QueueLifecycle(systemEditor);
		SetAppName("SystemEditor");
		return;
	}

	QueueLifecycle(RefCountedPtr<Application::Lifecycle>(new EditorWelcomeScreen(this)));
}

void EditorApp::AddLoadingTask(TaskSet::Handle handle)
{
	m_loadingTasks.emplace_back(std::move(handle));
}

void EditorApp::SetAppName(std::string_view name)
{
	m_appName = name;
}

void EditorApp::PushModalInternal(Modal *modal)
{
	m_modalStack.push_back(RefCountedPtr<Modal>(modal));
}

void EditorApp::OnStartup()
{
	Log::GetLog()->SetLogFile("editor.txt");

	m_editorCfg.reset(new IniConfig());

	m_editorCfg->SetInt("ScrWidth", 1600);
	m_editorCfg->SetInt("ScrHeight", 900);
	m_editorCfg->SetInt("VSync", 1);
	m_editorCfg->SetInt("AntiAliasingMode", 4);

	m_editorCfg->Read(FileSystem::userFiles, "editor.ini");
	m_editorCfg->Save(); // write defaults if the file doesn't exist

	EnumStrings::Init();
	Lua::Init();
	ModManager::Init();

	ModManager::LoadMods(m_editorCfg.get());

	// get threads up
	Uint32 numThreads = m_editorCfg->Int("WorkerThreads");
	numThreads = numThreads ? numThreads : std::max(OS::GetNumCores() - 1, 1U);
	GetTaskGraph()->SetWorkerThreads(numThreads);

	Lang::Resource &res(Lang::GetResource("core", m_editorCfg->String("Lang", "en")));
	Lang::MakeCore(res);

	Graphics::RendererOGL::RegisterRenderer();

	m_renderer = StartupRenderer(m_editorCfg.get(), false, true);
	StartupInput(m_editorCfg.get());

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

	m_editorCfg.reset();
}

void EditorApp::PreUpdate()
{
	HandleEvents();
	GetPiGui()->NewFrame();
}

void EditorApp::PostUpdate()
{
	// Clean up finished modals
	for (int idx = int(m_modalStack.size()) - 1; idx >= 0; --idx) {
		if (m_modalStack[idx]->Ready())
			m_modalStack.erase(m_modalStack.begin() + idx);
	}

	// Draw modals after cleaning, to ensure application has all of frame+1
	// to process modal results
	for (auto &modal : m_modalStack) {
		modal->Draw();
	}

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

	const ImVec2 winSize = ImGui::GetMainViewport()->Size;
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

// ============================================================================
//  Welcome Screen
// ============================================================================

static inline ImVec4 operator*(const ImVec4& lhs, const float& rhs)   { return ImVec4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs); }

ImVec2 CalcEditorModeButtonSize(const char *label, float iconSize)
{
	ImVec2 textSize = ImGui::CalcTextSize(label, nullptr, false, iconSize);
	return ImVec2(iconSize, iconSize + ImGui::GetStyle().ItemSpacing.y + textSize.y) + ImGui::GetStyle().FramePadding * 2.f;
}

bool DrawEditorModeButton(const char *label, const char *icon, float iconSize, PiGui::Instance *pigui)
{
	ImGuiStyle &style = ImGui::GetStyle();

	ImVec2 size = CalcEditorModeButtonSize(label, iconSize);
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 padding = style.FramePadding;

	bool clicked = ImGui::InvisibleButton(label, size);
	bool hovered = ImGui::IsItemHovered();
	float timer = ImGui::GetCurrentContext()->HoveredIdTimer;

	ImColor regular = style.Colors[ImGuiCol_Text];
	ImColor active = style.Colors[ImGuiCol_ButtonHovered];
	ImColor col = hovered ? ImColor(ImLerp<ImVec4>(regular, active, std::min(1.f, timer * 8.f))) : regular;

	ImDrawList *dl = ImGui::GetWindowDrawList();

	dl->AddRect(pos, pos + size, ImColor(style.Colors[ImGuiCol_FrameBg]), style.FrameRounding, 0, 2.f);

	ImGui::PushFont(pigui->GetFont("icons", iconSize));
	dl->AddText(pos + padding, col, icon);
	ImGui::PopFont();

	ImVec2 textSize = ImGui::CalcTextSize(label, nullptr, false, iconSize);
	ImVec2 textOffset = ImVec2(
		(iconSize - textSize.x) / 2.f,
		iconSize + style.ItemSpacing.y);

	dl->AddText(nullptr, 0, pos + padding + textOffset, regular, label, nullptr, textSize.x);

	return clicked;
}

void EditorWelcomeScreen::Update(float dt)
{
	Draw::BeginHostWindow("##fullscreen");

	PiGui::Instance *pigui = m_app->GetPiGui();

	float iconSize = 128;
	int numButtons = 2;
	ImVec2 size = CalcEditorModeButtonSize("Model Viewer", iconSize);

	ImGui::SetCursorPos((ImGui::GetWindowSize() - ImVec2(size.x * numButtons + ImGui::GetStyle().ItemSpacing.x, size.y)) / 2.f);
	ImGui::BeginGroup();

	if (DrawEditorModeButton("Model Viewer", EICON_SURFACE_STATION, iconSize, pigui)) {
		m_app->QueueLifecycle(RefCountedPtr<ModelViewer>(new ModelViewer(m_app, Lua::manager)));
		m_app->SetAppName("Model Viewer");
		RequestEndLifecycle();
	}

	ImGui::SameLine();

	if (DrawEditorModeButton("System Editor", EICON_SYSTEM_EDITOR, iconSize, pigui)) {
		m_app->QueueLifecycle(RefCountedPtr<SystemEditor>(new SystemEditor(m_app)));
		m_app->SetAppName("System Editor");
		RequestEndLifecycle();
	}

	ImGui::EndGroup();

	ImGui::End();
}
