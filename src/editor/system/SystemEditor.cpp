// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SystemEditor.h"

#include "GalaxyEditAPI.h"
#include "SystemEditorHelpers.h"

#include "EnumStrings.h"
#include "FileSystem.h"

#include "editor/EditorApp.h"
#include "editor/EditorDraw.h"
#include "editor/UndoSystem.h"
#include "editor/UndoStepType.h"

#include "galaxy/Galaxy.h"
#include "galaxy/GalaxyGenerator.h"
#include "galaxy/StarSystemGenerator.h"

#include "lua/Lua.h"

#include "imgui/imgui.h"
#include "system/SystemBodyUndo.h"

#include <memory>

using namespace Editor;

namespace {
	static constexpr const char *OUTLINE_WND_ID = "Outline";
	static constexpr const char *PROPERTIES_WND_ID = "Properties";
	static constexpr const char *VIEWPORT_WND_ID = "Viewport";

}

class SystemEditor::UndoSetSelection : public UndoStep {
public:
	UndoSetSelection(SystemEditor *editor, SystemBody *newSelection) :
		m_editor(editor),
		m_selection(newSelection)
	{
		Swap();
	}

	void Swap() override {
		std::swap(m_editor->m_selectedBody, m_selection);
	}

private:
	SystemEditor *m_editor;
	SystemBody *m_selection;
};

SystemEditor::SystemEditor(EditorApp *app) :
	m_app(app),
	m_undo(new UndoSystem()),
	m_selectedBody(nullptr)
{
	GalacticEconomy::Init();

	m_galaxy = GalaxyGenerator::Create();
	m_systemLoader.reset(new CustomSystemsDatabase(m_galaxy.Get(), "systems"));

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

SystemEditor::~SystemEditor()
{
}

bool SystemEditor::LoadSystem(const std::string &filepath)
{
	const CustomSystem *csys = m_systemLoader->LoadSystem(filepath);
	if (!csys)
		return false;

	SystemPath path = {csys->sectorX, csys->sectorY, csys->sectorZ, csys->systemIndex};
	Uint32 _init[5] = { Uint32(csys->seed), Uint32(csys->sectorX), Uint32(csys->sectorY), Uint32(csys->sectorZ), UNIVERSE_SEED };
	Random rng(_init, 5);

	RefCountedPtr<StarSystem::GeneratorAPI> system(new StarSystem::GeneratorAPI(path, m_galaxy, nullptr, rng));
	auto customStage = std::make_unique<StarSystemCustomGenerator>();

	if (!customStage->ApplyToSystem(rng, system, csys)) {
		Log::Error("System is fully random, cannot load from file");
		return false;
	}

	// FIXME: need to run StarSystemPopulateGenerator here to finish filling out system
	// Setting up faction affinity etc. requires running full gamut of generator stages

	// auto populateStage = std::make_unique<PopulateStarSystemGenerator>();
	// GalaxyGenerator::StarSystemConfig config;
	// config.isCustomOnly = true;

	// populateStage->Apply(rng, m_galaxy, system, &config);

	if (!system->GetRootBody()) {
		Log::Error("Custom system doesn't have a root body");
		return false;
	}

	m_system = system;
	m_filepath = filepath;

	return true;
}

void SystemEditor::WriteSystem(const std::string &filepath)
{
	Log::Info("Writing to path: {}/{}", FileSystem::GetDataDir(), filepath);
	// FIXME: need better file-saving interface for the user
	FILE *f = FileSystem::FileSourceFS(FileSystem::GetDataDir()).OpenWriteStream(filepath, FileSystem::FileSourceFS::WRITE_TEXT);

	if (!f)
		return;

	StarSystem::EditorAPI::ExportToLua(f, m_system.Get(), m_galaxy.Get());

	fclose(f);
}

void SystemEditor::Start()
{
}

void SystemEditor::End()
{
}

// ─── Update Loop ─────────────────────────────────────────────────────────────

void SystemEditor::HandleInput()
{

}

void SystemEditor::Update(float deltaTime)
{
	ImGuiID editorID = ImGui::GetID("System Editor");
	if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z, editorID, ImGuiInputFlags_RouteGlobal)) {
		GetUndo()->Redo();
	} else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z, editorID, ImGuiInputFlags_RouteGlobal)) {
		GetUndo()->Undo();
	}

	DrawInterface();

	if (ImGui::IsKeyPressed(ImGuiKey_F1)) {
		WriteSystem(m_filepath);
	}
}

// ─── Interface Rendering ─────────────────────────────────────────────────────

void SystemEditor::SetupLayout(ImGuiID dockspaceID)
{
	ImGuiID nodeID = ImGui::DockBuilderAddNode(dockspaceID);

	ImGui::DockBuilderSetNodePos(nodeID, ImGui::GetWindowPos());
	ImGui::DockBuilderSetNodeSize(nodeID, ImGui::GetWindowSize());

	ImGuiID leftSide = ImGui::DockBuilderSplitNode(nodeID, ImGuiDir_Left, 0.25, nullptr, &nodeID);
	ImGuiID rightSide = ImGui::DockBuilderSplitNode(nodeID, ImGuiDir_Right, 0.25 / (1.0 - 0.25), nullptr, &nodeID);
	// ImGuiID bottom = ImGui::DockBuilderSplitNode(nodeID, ImGuiDir_Down, 0.2, nullptr, &nodeID);

	ImGui::DockBuilderDockWindow(OUTLINE_WND_ID, leftSide);
	ImGui::DockBuilderDockWindow(PROPERTIES_WND_ID, rightSide);
	ImGui::DockBuilderDockWindow(VIEWPORT_WND_ID, nodeID);

	ImGui::DockBuilderFinish(dockspaceID);
}

void SystemEditor::DrawInterface()
{
	Draw::ShowUndoDebugWindow(GetUndo());

	static bool isFirstRun = true;

	Draw::BeginHostWindow("HostWindow", nullptr, ImGuiWindowFlags_NoSavedSettings);

	ImGuiID dockspaceID = ImGui::GetID("DockSpace");

	if (isFirstRun)
		SetupLayout(dockspaceID);

	ImGui::DockSpace(dockspaceID);

	if (ImGui::Begin(OUTLINE_WND_ID)) {
		ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 14));
		DrawOutliner();
		ImGui::PopFont();
	}
	ImGui::End();

	if (ImGui::Begin(PROPERTIES_WND_ID)) {
		if (m_selectedBody)
			DrawBodyProperties();
		else
			DrawSystemProperties();
	}
	ImGui::End();

	ImGui::End();

	if (isFirstRun)
		isFirstRun = false;
}

void SystemEditor::DrawOutliner()
{
	ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 16));

	std::string label = fmt::format("System: {}", m_system->GetName());
	if (ImGui::Selectable(label.c_str(), !m_selectedBody)) {
		m_selectedBody = nullptr;
	}

	ImGui::PopFont();

	ImGui::Spacing();

	Draw::BeginHorizontalBar();

	if (ImGui::Button("A##AddBody")) {
		SystemBody *parent = m_selectedBody ? m_selectedBody : m_system->GetRootBody().Get();
		SystemBody *body = StarSystem::EditorAPI::NewBody(m_system.Get());

		GetUndo()->BeginEntry("Add Body");
		GetUndo()->AddUndoStep<SystemEditorHelpers::UndoManageStarSystemBody>(m_system.Get(), body);
		GetUndo()->AddUndoStep<SystemEditorHelpers::UndoAddRemoveChildBody>(parent, body);
		GetUndo()->AddUndoStep<SystemEditor::UndoSetSelection>(this, body);
		GetUndo()->EndEntry();
	}

	bool canDeleteBody = m_selectedBody && m_selectedBody != m_system->GetRootBody().Get();
	if (canDeleteBody && ImGui::Button("D##DeleteBody")) {
		SystemBody *parent = m_selectedBody->GetParent();
		auto iter = std::find(parent->GetChildren().begin(), parent->GetChildren().end(), m_selectedBody);
		size_t idx = std::distance(parent->GetChildren().begin(), iter);

		GetUndo()->BeginEntry("Delete Body");
		GetUndo()->AddUndoStep<SystemEditorHelpers::UndoAddRemoveChildBody>(parent, idx);
		GetUndo()->AddUndoStep<SystemEditorHelpers::UndoManageStarSystemBody>(m_system.Get(), nullptr, m_selectedBody, true);
		GetUndo()->AddUndoStep<SystemEditor::UndoSetSelection>(this, nullptr);
		GetUndo()->EndEntry();
	}

	Draw::EndHorizontalBar();

	if (ImGui::BeginChild("OutlinerList")) {
		std::vector<std::pair<SystemBody *, size_t>> m_systemStack {
			{ m_system->GetRootBody().Get(), 0 }
		};

		if (!DrawBodyNode(m_system->GetRootBody().Get())) {
			ImGui::EndChild();
			return;
		}

		while (!m_systemStack.empty()) {
			auto &pair = m_systemStack.back();

			if (pair.second == pair.first->GetNumChildren()) {
				m_systemStack.pop_back();
				ImGui::TreePop();
				continue;
			}

			SystemBody *body = pair.first->GetChildren()[pair.second++];
			if (DrawBodyNode(body))
				m_systemStack.push_back({ body, 0 });
		}
	}
	ImGui::EndChild();
}

bool SystemEditor::DrawBodyNode(SystemBody *body)
{
	ImGuiTreeNodeFlags flags =
		ImGuiTreeNodeFlags_DefaultOpen |
		ImGuiTreeNodeFlags_OpenOnDoubleClick |
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_SpanFullWidth;

	if (body->GetNumChildren() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	if (body == m_selectedBody)
		flags |= ImGuiTreeNodeFlags_Selected;

	bool open = ImGui::TreeNodeEx(body->GetName().c_str(), flags);

	if (ImGui::IsItemActivated()) {
		m_selectedBody = body;
	}

	// TODO: custom rendering on body entry, e.g. icon / contents etc.

	return open && body->GetNumChildren();
}

void SystemEditor::DrawBodyProperties()
{
	ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 16));
	ImGui::Text("Body: %s (%d)", m_selectedBody->GetName().c_str(), m_selectedBody->GetPath().bodyIndex);
	ImGui::PopFont();

	ImGui::Spacing();

	SystemBody::EditorAPI::EditProperties(m_selectedBody, GetUndo());
}

void SystemEditor::DrawSystemProperties()
{
	if (!m_system) {
		ImGui::Text("No loaded system");
		return;
	}

	SystemPath path = m_system->GetPath();

	ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 16));
	ImGui::Text("%s (%d, %d, %d : %d)",
		m_system->GetName().c_str(),
		path.sectorX, path.sectorY, path.sectorZ, path.systemIndex);
	ImGui::PopFont();

	ImGui::Spacing();

	Random rng (Uint32(m_app->GetTime() * 4.0) ^ m_system->GetSeed());
	StarSystem::EditorAPI::EditName(m_system.Get(), rng, GetUndo());

	StarSystem::EditorAPI::EditProperties(m_system.Get(), GetUndo());
}
