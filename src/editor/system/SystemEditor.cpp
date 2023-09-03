// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SystemEditor.h"

#include "GalaxyEditAPI.h"
#include "SystemEditorHelpers.h"
#include "SystemBodyUndo.h"
#include "SystemEditorViewport.h"

#include "EnumStrings.h"
#include "FileSystem.h"
#include "JsonUtils.h"
#include "ModManager.h"
#include "SystemView.h"
#include "core/StringUtils.h"

#include "editor/EditorApp.h"
#include "editor/EditorDraw.h"
#include "editor/EditorIcons.h"
#include "editor/UndoSystem.h"
#include "editor/UndoStepType.h"

#include "galaxy/Galaxy.h"
#include "galaxy/GalaxyGenerator.h"
#include "galaxy/StarSystemGenerator.h"
#include "lua/Lua.h"
#include "pigui/PiGui.h"

#include "imgui/imgui.h"

#include "portable-file-dialogs/pfd.h"

#include <chrono>
#include <cstdlib>
#include <memory>

using namespace Editor;

namespace {
	static constexpr const char *OUTLINE_WND_ID = "Outline";
	static constexpr const char *PROPERTIES_WND_ID = "Properties";
	static constexpr const char *VIEWPORT_WND_ID = "Viewport";
	static constexpr const char *FILE_MODAL_ID = "File Window Open";
}

const char *Editor::GetBodyIcon(const SystemBody *body) {
	if (body->GetType() == SystemBody::TYPE_GRAVPOINT)
		return EICON_GRAVPOINT;
	if (body->GetType() == SystemBody::TYPE_STARPORT_ORBITAL)
		return EICON_SPACE_STATION;
	if (body->GetType() == SystemBody::TYPE_STARPORT_SURFACE)
		return EICON_SURFACE_STATION;
	if (body->GetType() == SystemBody::TYPE_PLANET_ASTEROID)
		return EICON_ASTEROID;
	if (body->GetSuperType() == SystemBody::SUPERTYPE_ROCKY_PLANET)
		return (!body->GetParent() || body->GetParent()->GetSuperType() < SystemBody::SUPERTYPE_ROCKY_PLANET) ?
			EICON_ROCKY_PLANET : EICON_MOON;
	if (body->GetSuperType() == SystemBody::SUPERTYPE_GAS_GIANT)
		return EICON_GAS_GIANT;
	if (body->GetSuperType() == SystemBody::SUPERTYPE_STAR)
		return EICON_SUN;

	return "?";
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
	m_system(nullptr),
	m_selectedBody(nullptr),
	m_pendingOp()
{
	GalacticEconomy::Init();

	m_galaxy = GalaxyGenerator::Create();
	m_systemLoader.reset(new CustomSystemsDatabase(m_galaxy.Get(), "systems"));

	m_viewport.reset(new SystemEditorViewport(m_app, this));

	m_random.seed({
		// generate random values not dependent on app runtime
		uint32_t(std::chrono::system_clock::now().time_since_epoch().count()),
		UNIVERSE_SEED
	});

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

SystemEditor::~SystemEditor()
{
}

void SystemEditor::NewSystem()
{
	ClearSystem();

	SystemPath path(0, 0, 0, 0);

	auto *newSystem = new StarSystem::GeneratorAPI(path, m_galaxy, nullptr, GetRng());
	m_system.Reset(newSystem);

	newSystem->SetRootBody(newSystem->NewBody());
}

bool SystemEditor::LoadSystemFromDisk(const std::string &absolutePath)
{
	if (ends_with_ci(absolutePath, ".lua")) {
		std::string filepath = FileSystem::GetRelativePath(FileSystem::GetDataDir(), absolutePath);

		if (filepath.empty()) {
			Log::Error("Cannot read .lua Custom System files from outside the game's data directory!");
			return false;
		}

		return LoadSystem(FileSystem::gameDataFiles.Lookup(filepath));
	} else {
		std::string dirpath = absolutePath.substr(0, absolutePath.find_last_of("/\\"));
		std::string filename = absolutePath.substr(dirpath.size() + 1);

		// Hack: construct a temporary FileSource to load from an arbitrary path
		auto fs = FileSystem::FileSourceFS(dirpath);

		return LoadSystem(fs.Lookup(filename));
	}
}

bool SystemEditor::LoadSystem(const FileSystem::FileInfo &file)
{
	if (!file.Exists()) {
		Log::Error("Cannot open file path {}", file.GetAbsolutePath());
		return false;
	}

	ClearSystem();

	m_filepath = file.GetAbsolutePath();
	m_filedir = file.GetAbsoluteDir();

	bool ok = false;
	if (ends_with_ci(file.GetPath(), ".json")) {
		const CustomSystem *csys = m_systemLoader->LoadSystemFromJSON(file.GetName(), JsonUtils::LoadJson(file.Read()));
		if (csys)
			ok = LoadCustomSystem(csys);
	} else if (ends_with_ci(file.GetPath(), ".lua")) {
		const CustomSystem *csys = m_systemLoader->LoadSystem(file.GetPath());
		if (csys)
			ok = LoadCustomSystem(csys);
	}

	if (ok) {
		std::string windowTitle = fmt::format("System Editor - {}", m_filepath);
		SDL_SetWindowTitle(m_app->GetRenderer()->GetSDLWindow(), windowTitle.c_str());
	} else {
		m_filepath.clear();
	}


	return ok;
}

bool SystemEditor::WriteSystem(const std::string &filepath)
{
	Log::Info("Writing to path: {}/{}", FileSystem::GetDataDir(), filepath);
	// FIXME: need better file-saving interface for the user
	// FileSystem::FileSourceFS(FileSystem::GetDataDir()).OpenWriteStream(filepath, FileSystem::FileSourceFS::WRITE_TEXT);

	FILE *f = fopen(filepath.c_str(), "w");

	if (!f)
		return false;

	// StarSystem::EditorAPI::ExportToLua(f, m_system.Get(), m_galaxy.Get());

	Json systemdef = Json::object();

	m_system->DumpToJson(systemdef);

	std::string jsonData = systemdef.dump(1, '\t');

	fwrite(jsonData.data(), 1, jsonData.size(), f);
	fclose(f);

	return true;
}

bool SystemEditor::LoadCustomSystem(const CustomSystem *csys)
{
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
	m_viewport->SetSystem(system);

	return true;
}

void SystemEditor::ClearSystem()
{
	m_undo->Clear();

	m_system.Reset();
	m_viewport->SetSystem(m_system);
	m_selectedBody = nullptr;
	m_pendingOp = {};

	m_filepath.clear();

	SDL_SetWindowTitle(m_app->GetRenderer()->GetSDLWindow(), "System Editor");
}

// Here to avoid needing to drag in the Galaxy header in SystemEditor.h
RefCountedPtr<Galaxy> SystemEditor::GetGalaxy() { return m_galaxy; }

void SystemEditor::SetSelectedBody(SystemBody *body)
{
	// note: using const_cast here to work with Projectables which store a const pointer
	m_selectedBody = body;
}

void SystemEditor::Start()
{
}

void SystemEditor::End()
{
}

void SystemEditor::ActivateOpenDialog()
{
	// FIXME: need to handle loading files outside of game data dir
	m_openFile.reset(new pfd::open_file(
		"Open Custom System File",
		FileSystem::JoinPath(FileSystem::GetDataDir(), m_filedir),
		{
			"All System Definition Files", "*.lua *.json",
			"Lua System Definition (.lua)", "*.lua",
			"JSON System Definition (.json)", "*.json"
		})
	);

	ImGui::OpenPopup(m_fileActionActiveModal);
}

void SystemEditor::ActivateSaveDialog()
{
	// FIXME: need to handle saving files outside of game data dir
	m_saveFile.reset(new pfd::save_file(
		"Save Custom System File",
		FileSystem::JoinPath(FileSystem::GetDataDir(), m_filedir),
		{
			"JSON System Definition (.json)", "*.json"
		})
	);

	ImGui::OpenPopup(m_fileActionActiveModal);
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

	m_fileActionActiveModal = ImGui::GetID(FILE_MODAL_ID);

	DrawInterface();

	HandleBodyOperations();

	if (m_openFile && m_openFile->ready(0)) {
		std::vector<std::string> resultFiles = m_openFile->result();
		m_openFile.reset();

		if (!resultFiles.empty()) {
			Log::Info("OpenFile: {}", resultFiles[0]);
			LoadSystemFromDisk(resultFiles[0]);
		}
	}

	if (m_saveFile && m_saveFile->ready(0)) {
		std::string filePath = m_saveFile->result();
		m_saveFile.reset();

		if (!filePath.empty()) {
			Log::Info("SaveFile: {}", filePath);
			bool success = WriteSystem(filePath);

			if (success)
				m_lastSavedUndoStack = m_undo->GetCurrentEntry();
		}
	}

	bool fileModalOpen = ImGui::IsPopupOpen(m_fileActionActiveModal, ImGuiPopupFlags_AnyPopupLevel);

	// Finished all pending file operations
	if (fileModalOpen && !m_openFile && !m_saveFile) {
		auto &popupStack = ImGui::GetCurrentContext()->OpenPopupStack;
		for (size_t idx = 0; idx < popupStack.size(); ++idx) {
			if (popupStack[idx].PopupId == m_fileActionActiveModal) {
				ImGui::ClosePopupToLevel(idx, true);
				break;
			}
		}
	}
}

void SystemEditor::HandleBodyOperations()
{
	if (m_pendingOp.type == BodyRequest::TYPE_None)
		return;

	if (m_pendingOp.type == BodyRequest::TYPE_Add) {

		// TODO: generate body parameters according to m_pendingOp.newBodyType
		SystemBody *body;

		if (!m_pendingOp.parent)
			body = StarSystem::EditorAPI::NewBody(m_system.Get());
		else
			body = StarSystem::EditorAPI::NewBodyAround(m_system.Get(), GetRng(), m_pendingOp.parent, m_pendingOp.idx);

		if (!body) {
			Log::Error("Failed to create requested body.");
			m_pendingOp = {};
			return;
		}

		GetUndo()->BeginEntry("Add Body");
		GetUndo()->AddUndoStep<SystemEditorUndo::ReorderStarSystemBodies>(m_system.Get());

		// Mark the body for removal on undo
		GetUndo()->AddUndoStep<SystemEditorUndo::ManageStarSystemBody>(m_system.Get(), body);
		// Add the new body to its parent
		GetUndo()->AddUndoStep<SystemEditorUndo::AddRemoveChildBody>(m_pendingOp.parent, body, m_pendingOp.idx);

		GetUndo()->AddUndoStep<SystemEditorUndo::ReorderStarSystemBodies>(m_system.Get(), true);
		GetUndo()->AddUndoStep<SystemEditor::UndoSetSelection>(this, body);
		GetUndo()->EndEntry();

	}

	if (m_pendingOp.type == BodyRequest::TYPE_Delete) {

		std::vector<SystemBody *> toDelete { m_pendingOp.body };
		size_t sliceBegin = 0;

		// Iterate over all child bodies of this system body and mark for deletion
		while (sliceBegin < toDelete.size()) {
			size_t sliceEnd = toDelete.size();
			for (size_t idx = sliceBegin; idx < sliceEnd; idx++) {
				SystemBody *body = toDelete[idx];
				if (body->HasChildren())
					for (auto &child : body->GetChildren())
						toDelete.push_back(child);
			}
			sliceBegin = sliceEnd;
		}

		GetUndo()->BeginEntry("Delete Body");
		GetUndo()->AddUndoStep<SystemEditorUndo::ReorderStarSystemBodies>(m_system.Get());

		// Record deletion of each marked body in reverse order (ending with the topmost body to delete)
		for (auto &child : reverse_container(toDelete)) {
			SystemBody *parent = child->GetParent();
			size_t idx = SystemBody::EditorAPI::GetIndexInParent(child);
			GetUndo()->AddUndoStep<SystemEditorUndo::AddRemoveChildBody>(parent, idx);
			GetUndo()->AddUndoStep<SystemEditorUndo::ManageStarSystemBody>(m_system.Get(), nullptr, child, true);
		}

		GetUndo()->AddUndoStep<SystemEditorUndo::ReorderStarSystemBodies>(m_system.Get(), true);
		GetUndo()->AddUndoStep<SystemEditor::UndoSetSelection>(this, nullptr);
		GetUndo()->EndEntry();

	}

	if (m_pendingOp.type == BodyRequest::TYPE_Reparent) {

		size_t sourceIdx = SystemBody::EditorAPI::GetIndexInParent(m_pendingOp.body);

		if (m_pendingOp.parent == m_pendingOp.body->GetParent() && m_pendingOp.idx > sourceIdx) {
			m_pendingOp.idx -= 1;
		}

		GetUndo()->BeginEntry("Reorder Body");
		GetUndo()->AddUndoStep<SystemEditorUndo::ReorderStarSystemBodies>(m_system.Get());
		GetUndo()->AddUndoStep<SystemEditorUndo::AddRemoveChildBody>(m_pendingOp.body->GetParent(), sourceIdx);
		GetUndo()->AddUndoStep<SystemEditorUndo::AddRemoveChildBody>(m_pendingOp.parent, m_pendingOp.body, m_pendingOp.idx);
		GetUndo()->AddUndoStep<SystemEditorUndo::ReorderStarSystemBodies>(m_system.Get(), true);
		GetUndo()->EndEntry();

	}

	if (m_pendingOp.type == BodyRequest::TYPE_Resort) {

		GetUndo()->BeginEntry("Sort by Semi-Major Axis");
		StarSystem::EditorAPI::SortBodyHierarchy(m_system.Get(), GetUndo());
		GetUndo()->EndEntry();

	}

	// Clear the pending operation
	m_pendingOp = {};
}

// ─── Interface Rendering ─────────────────────────────────────────────────────

void SystemEditor::SetupLayout(ImGuiID dockspaceID)
{
	ImGuiID nodeID = ImGui::DockBuilderAddNode(dockspaceID);

	ImGui::DockBuilderSetNodePos(nodeID, ImGui::GetWindowPos());
	ImGui::DockBuilderSetNodeSize(nodeID, ImGui::GetWindowSize());

	ImGuiID leftSide = ImGui::DockBuilderSplitNode(nodeID, ImGuiDir_Left, 0.2, nullptr, &nodeID);
	ImGuiID rightSide = ImGui::DockBuilderSplitNode(nodeID, ImGuiDir_Right, 0.2 / (1.0 - 0.2), nullptr, &nodeID);
	// ImGuiID bottom = ImGui::DockBuilderSplitNode(nodeID, ImGuiDir_Down, 0.2, nullptr, &nodeID);

	ImGui::DockBuilderDockWindow(OUTLINE_WND_ID, leftSide);
	ImGui::DockBuilderDockWindow(PROPERTIES_WND_ID, rightSide);
	ImGui::DockBuilderDockWindow(VIEWPORT_WND_ID, nodeID);

	ImGui::DockBuilderFinish(dockspaceID);
}

void SystemEditor::DrawInterface()
{
	Draw::ShowUndoDebugWindow(GetUndo());
	// ImGui::ShowDemoWindow();
	ImGui::ShowMetricsWindow();

	static bool isFirstRun = true;

	Draw::BeginHostWindow("HostWindow", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar()) {

		if (ImGui::BeginMenu("File")) {

			if (ImGui::MenuItem("New File", "Ctrl+N")) {
				// TODO: show nag dialog if attempting open with unsaved changes

				NewSystem();
			}

			if (ImGui::MenuItem("Open File", "Ctrl+O")) {
				// TODO: show nag dialog if attempting open with unsaved changes

				ActivateOpenDialog();
			}

			if (ImGui::MenuItem("Save", "Ctrl+S")) {
				// Cannot write back .lua files
				if (ends_with_ci(m_filepath, ".lua"))
					ActivateSaveDialog();
				else
					WriteSystem(m_filepath);
			}

			if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"))
				ActivateSaveDialog();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit")) {

			if (m_selectedBody) {
				if (ImGui::MenuItem("Add Child", "Ctrl+A")) {
					m_pendingOp.type = BodyRequest::TYPE_Add;
					m_pendingOp.parent = m_selectedBody ? m_selectedBody : m_system->GetRootBody().Get();
					m_pendingOp.newBodyType = SystemBody::BodyType::TYPE_GRAVPOINT;
				}

				if (ImGui::MenuItem("Add Sibling", "Ctrl+Shift+A")) {
					if (!m_selectedBody || !m_selectedBody->GetParent()) {
						return;
					}

					m_pendingOp.type = BodyRequest::TYPE_Add;
					m_pendingOp.parent = m_selectedBody->GetParent();
					m_pendingOp.idx = SystemBody::EditorAPI::GetIndexInParent(m_selectedBody) + 1;
					m_pendingOp.newBodyType = SystemBody::BodyType::TYPE_GRAVPOINT;
				}

				if (m_selectedBody != m_system->GetRootBody() && ImGui::MenuItem("Delete", "Ctrl+W")) {
					m_pendingOp.type = BodyRequest::TYPE_Delete;
					m_pendingOp.body = m_selectedBody;
				}
			}

			if (ImGui::MenuItem("Sort Bodies")) {
				m_pendingOp.type = BodyRequest::TYPE_Resort;
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	ImGuiID dockspaceID = ImGui::GetID("DockSpace");

	if (isFirstRun)
		SetupLayout(dockspaceID);

	ImGui::DockSpace(dockspaceID);

	// BUG: Right-click on button can break undo handling if it happens after active InputText is submitted
	// We work around it by rendering the viewport first
	m_viewport->Update(m_app->DeltaTime());

	if (ImGui::Begin(OUTLINE_WND_ID)) {
		ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 14));
		DrawOutliner();
		ImGui::PopFont();
	}
	ImGui::End();

	if (ImGui::Begin(PROPERTIES_WND_ID)) {
		// Adjust default window label position
		ImGui::PushItemWidth(ImFloor(ImGui::GetWindowSize().x * 0.55f));

		if (m_selectedBody)
			DrawBodyProperties();
		else
			DrawSystemProperties();

		ImGui::PopItemWidth();
	}
	ImGui::End();

#if 0
	if (ImGui::Begin("ModList")) {
		for (const auto &mod : ModManager::EnumerateMods()) {
			if (!mod.enabled)
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 64, 64, 255));

			ImGui::PushFont(m_app->GetPiGui()->GetFont("orbiteer", 14));
			ImGui::TextUnformatted(mod.name.c_str());
			ImGui::PopFont();

			ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 12));
			ImGui::TextUnformatted(mod.path.c_str());
			ImGui::PopFont();

			if (!mod.enabled)
				ImGui::PopStyleColor();

			ImGui::Spacing();
		}
	}
	ImGui::End();
#endif

	ImGui::End();

	DrawFileActionModal();

	DrawPickSystemModal();

	if (isFirstRun)
		isFirstRun = false;
}

void SystemEditor::DrawOutliner()
{
	ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 16));

	std::string name = m_system.Valid() ? m_system->GetName() : "<None>";
	std::string label = fmt::format("System: {}", name);
	if (ImGui::Selectable(label.c_str(), !m_selectedBody)) {
		m_selectedBody = nullptr;
	}

	ImGui::Spacing();

	if (!m_system) {
		ImGui::PopFont();
		return;
	}

	if (ImGui::BeginChild("OutlinerList")) {
		std::vector<std::pair<SystemBody *, size_t>> m_systemStack {
			{ m_system->GetRootBody().Get(), 0 }
		};

		if (!DrawBodyNode(m_system->GetRootBody().Get(), true)) {
			ImGui::EndChild();
			ImGui::PopFont();
			return;
		}

		while (!m_systemStack.empty()) {
			auto &pair = m_systemStack.back();

			if (pair.second >= pair.first->GetNumChildren()) {
				m_systemStack.pop_back();
				ImGui::TreePop();
				continue;
			}

			SystemBody *body = pair.first->GetChildren()[pair.second++];
			if (DrawBodyNode(body, false))
				m_systemStack.push_back({ body, 0 });
		}
	}
	ImGui::EndChild();

	ImGui::PopFont();
}

void SystemEditor::HandleOutlinerDragDrop(SystemBody *refBody)
{
	// Handle drag-drop re-order/re-parent
	SystemBody *dropBody = nullptr;
	Draw::DragDropTarget dropTarget = Draw::HierarchyDragDrop("SystemBody", ImGui::GetID(refBody), &refBody, &dropBody, sizeof(SystemBody *));

	if (dropTarget != Draw::DragDropTarget::DROP_NONE && refBody != dropBody) {
		size_t targetIdx = SystemBody::EditorAPI::GetIndexInParent(refBody);

		m_pendingOp.type = BodyRequest::TYPE_Reparent;
		m_pendingOp.body = dropBody;
		m_pendingOp.parent = dropTarget == Draw::DROP_CHILD ? refBody : refBody->GetParent();
		m_pendingOp.idx = 0;

		if (dropTarget == Draw::DROP_BEFORE)
			m_pendingOp.idx = targetIdx;
		else if (dropTarget == Draw::DROP_AFTER)
			m_pendingOp.idx = targetIdx + 1;
	}
}

bool SystemEditor::DrawBodyNode(SystemBody *body, bool isRoot)
{
	ImGuiTreeNodeFlags flags =
		ImGuiTreeNodeFlags_DefaultOpen |
		ImGuiTreeNodeFlags_OpenOnDoubleClick |
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_SpanFullWidth |
		ImGuiTreeNodeFlags_FramePadding;

	if (body->GetNumChildren() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	if (body == m_selectedBody)
		flags |= ImGuiTreeNodeFlags_Selected;

	ImGuiID bodyId = ImGui::GetID(body);
	std::string name = fmt::format("{} {}###{:x}", GetBodyIcon(body), body->GetName(), bodyId);
	bool open = ImGui::TreeNodeEx(name.c_str(), flags);

	if (body == m_selectedBody)
		ImGui::SetItemDefaultFocus();

	if (!isRoot) {
		HandleOutlinerDragDrop(body);
	}

	if (ImGui::IsItemActivated()) {
		m_viewport->GetMap()->SetSelectedObject({ Projectable::OBJECT, Projectable::SYSTEMBODY, body });
		m_selectedBody = body;
	}

	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Center")) {
			m_viewport->GetMap()->SetViewedObject({ Projectable::OBJECT, Projectable::SYSTEMBODY, body });
		}

		if (ImGui::MenuItem("Add Child")) {
			m_pendingOp.type = BodyRequest::TYPE_Add;
			m_pendingOp.parent = body;
			m_pendingOp.idx = body->GetNumChildren();
		}

		if (body->GetParent() && ImGui::MenuItem("Add Sibling")) {
			m_pendingOp.type = BodyRequest::TYPE_Add;
			m_pendingOp.parent = body->GetParent();
			m_pendingOp.idx = SystemBody::EditorAPI::GetIndexInParent(body) + 1;
		}

		// TODO: "add body" context menu
		if (body->GetParent() && ImGui::MenuItem("Delete")) {
			m_pendingOp.type = BodyRequest::TYPE_Delete;
			m_pendingOp.body = body;
		}

		ImGui::EndPopup();
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

	ImGui::PushID(m_selectedBody);
	ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 13));

	SystemBody::EditorAPI::EditProperties(m_selectedBody, GetRng(), GetUndo());

	ImGui::PopFont();
	ImGui::PopID();
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

void SystemEditor::DrawPickSystemModal()
{

}

void SystemEditor::DrawFileActionModal()
{
	ImGui::SetNextWindowPos(ImGui::GetIO().DisplaySize * 0.5, ImGuiCond_Always, ImVec2(0.5, 0.5));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30, 30));

	ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 16));
	if (ImGui::BeginPopupModal(FILE_MODAL_ID, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
		ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 14));
		ImGui::TextUnformatted("Waiting on a file action to complete...");
		ImGui::PopFont();
		ImGui::EndPopup();
	}
	ImGui::PopFont();
	ImGui::PopStyleVar();
}
