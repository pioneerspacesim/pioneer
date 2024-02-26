// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SystemEditor.h"

#include "GalaxyEditAPI.h"
#include "SystemBodyUndo.h"
#include "SystemEditorHelpers.h"
#include "SystemEditorViewport.h"
#include "SystemEditorModals.h"

#include "EnumStrings.h"
#include "FileSystem.h"
#include "JsonUtils.h"
#include "ModManager.h"
#include "Pi.h" // just here for Pi::luaNameGen
#include "SystemView.h"
#include "core/StringUtils.h"

#include "editor/ActionBinder.h"
#include "editor/EditorApp.h"
#include "editor/EditorDraw.h"
#include "editor/EditorIcons.h"
#include "editor/UndoSystem.h"
#include "editor/UndoStepType.h"

#include "galaxy/Galaxy.h"
#include "galaxy/GalaxyGenerator.h"
#include "galaxy/StarSystemGenerator.h"
#include "graphics/Renderer.h"
#include "lua/Lua.h"
#include "lua/LuaNameGen.h"
#include "lua/LuaObject.h"
#include "pigui/PiGui.h"

#include "imgui/imgui.h"

#include "portable-file-dialogs/pfd.h"
// PFD pulls in windows headers sadly
#undef RegisterClass
#undef min
#undef max

#include <chrono>
#include <cstdlib>
#include <memory>

using namespace Editor;

namespace {
	static constexpr const char *OUTLINE_WND_ID = "Outline";
	static constexpr const char *PROPERTIES_WND_ID = "Properties";
	static constexpr const char *VIEWPORT_WND_ID = "Viewport";
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
	m_system(nullptr),
	m_systemInfo(),
	m_undo(new UndoSystem()),
	m_selectedBody(nullptr),
	m_contextBody(nullptr),
	m_pendingOp(),
	m_pendingFileReq(FileRequest_None),
	m_newSystemPath(),
	m_menuBinder(new ActionBinder())
{
	GalacticEconomy::Init();

	LuaObject<SystemBody>::RegisterClass();
	LuaObject<StarSystem>::RegisterClass();

	m_nameGen.reset(new LuaNameGen(Lua::manager));

	Pi::luaNameGen = m_nameGen.get();

	m_galaxy = GalaxyGenerator::Create();
	m_systemLoader.reset(new CustomSystemsDatabase(m_galaxy.Get(), "systems"));

	m_viewport.reset(new SystemEditorViewport(m_app, this));

	m_random.seed({
		// generate random values not dependent on app runtime
		uint32_t(std::chrono::system_clock::now().time_since_epoch().count()),
		UNIVERSE_SEED
	});

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	RegisterMenuActions();
}

SystemEditor::~SystemEditor()
{
}

void SystemEditor::NewSystem(SystemPath path)
{
	ClearSystem();

	auto *newSystem = new StarSystem::GeneratorAPI(path, m_galaxy, nullptr, GetRng());
	m_system.Reset(newSystem);

	newSystem->SetRootBody(newSystem->NewBody());

	SysPolit polit = {};
	polit.govType = Polit::GOV_NONE;

	newSystem->SetSysPolit(polit);

	// mark current file as unsaved
	m_lastSavedUndoStack = size_t(-1);

	m_viewport->SetSystem(m_system);
}

bool SystemEditor::LoadSystemFromDisk(const std::string &absolutePath)
{
	if (ends_with_ci(absolutePath, ".lua")) {
		std::string filepath = FileSystem::GetRelativePath(FileSystem::GetDataDir(), absolutePath);

		if (filepath.empty()) {
			Log::Error("Cannot read .lua Custom System files from outside the game's data directory!");
			return false;
		}

		return LoadSystemFromFile(FileSystem::gameDataFiles.Lookup(filepath));
	} else {
		std::string dirpath = absolutePath.substr(0, absolutePath.find_last_of("/\\"));
		std::string filename = absolutePath.substr(dirpath.size() + 1);

		// Hack: construct a temporary FileSource to load from an arbitrary path
		auto fs = FileSystem::FileSourceFS(dirpath);

		return LoadSystemFromFile(fs.Lookup(filename));
	}
}

bool SystemEditor::LoadSystem(SystemPath path)
{
	RefCountedPtr<const Sector> sec = m_galaxy->GetSector(path.SectorOnly());

	if (path.systemIndex >= sec->m_systems.size()) {
		Log::Error("System {} in sector ({},{},{}) does not exist", path.systemIndex, path.sectorX, path.sectorY, path.sectorZ);
		return false;
	}

	const Sector::System &system = sec->m_systems.at(path.systemIndex);

	// Load a fully-defined custom system from the custom system def
	// NOTE: we cannot (currently) determine which file this custom system originated from
	if (system.GetCustomSystem() && !system.GetCustomSystem()->IsRandom())
		LoadCustomSystem(system.GetCustomSystem());
	else
		LoadSystemFromGalaxy(m_galaxy->GetStarSystem(path));

	m_filepath = "";
	m_filedir = "";

	// mark as unsaved
	m_lastSavedUndoStack = size_t(-1);

	return true;
}

bool SystemEditor::LoadSystemFromFile(const FileSystem::FileInfo &file)
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
		const Json &data = JsonUtils::LoadJson(file.Read());

		const CustomSystem *csys = m_systemLoader->LoadSystemFromJSON(file.GetName(), data);
		if (csys)
			ok = LoadCustomSystem(csys);

		if (ok)
			m_systemInfo.comment = data["comment"];
	} else if (ends_with_ci(file.GetPath(), ".lua")) {
		const CustomSystem *csys = m_systemLoader->LoadSystem(file.GetPath());
		if (csys)
			ok = LoadCustomSystem(csys);
	}

	if (!ok)
		m_filepath.clear();

	OnFilepathChanged();

	return ok;
}

bool SystemEditor::WriteSystem(const std::string &filepath)
{
	Log::Info("Writing to path: {}/{}", FileSystem::GetDataDir(), filepath);
	// FIXME: need better file-saving interface for the user
	// FileSystem::FileSourceFS(FileSystem::GetDataDir()).OpenWriteStream(filepath, FileSystem::FileSourceFS::WRITE_TEXT);

	FILE *f = fopen(filepath.c_str(), "w");

	if (!f) {
		OnSaveComplete(false);
		return false;
	}

	Json systemdef = Json::object();

	m_system->DumpToJson(systemdef);

	if (m_systemInfo.randomFaction)
		systemdef.erase("faction");
	else
		systemdef["faction"] = m_systemInfo.faction;

	if (m_systemInfo.randomLawlessness)
		systemdef.erase("lawlessness");

	if (m_systemInfo.explored == CustomSystemInfo::EXPLORE_Random)
		systemdef.erase("explored");
	else
		systemdef["explored"] = m_systemInfo.explored == CustomSystemInfo::EXPLORE_ExploredAtStart;

	systemdef["comment"] = m_systemInfo.comment;

	std::string jsonData = systemdef.dump(1, '\t');

	fwrite(jsonData.data(), 1, jsonData.size(), f);
	fclose(f);

	OnSaveComplete(true);
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

	// NOTE: we don't run the PopulateSystem generator here, due to its
	// reliance on filled-out Sector information
	// As a result, population information will not be correct

	if (!system->GetRootBody()) {
		Log::Error("Custom system doesn't have a root body");
		return false;
	}

	m_system = system;
	m_viewport->SetSystem(system);

	CustomSystemInfo::ExplorationState explored = csys->explored ?
		CustomSystemInfo::EXPLORE_ExploredAtStart :
		CustomSystemInfo::EXPLORE_Unexplored;

	m_systemInfo.explored = csys->want_rand_explored ? CustomSystemInfo::EXPLORE_Random : explored;
	m_systemInfo.randomLawlessness = csys->want_rand_lawlessness;
	m_systemInfo.randomFaction = csys->faction == nullptr;
	m_systemInfo.faction = csys->faction ? csys->faction->name : "";

	return true;
}

void SystemEditor::LoadSystemFromGalaxy(RefCountedPtr<StarSystem> system)
{
	if (!system->GetRootBody()) {
		Log::Error("Randomly-generated system doesn't have a root body");
		return;
	}

	ClearSystem();

	StarSystem::EditorAPI::RemoveFromCache(system.Get());

	m_system = system;
	m_viewport->SetSystem(system);

	bool explored = system->GetExplored() == StarSystem::eEXPLORED_AT_START;

	m_systemInfo.explored = explored ? CustomSystemInfo::EXPLORE_ExploredAtStart : CustomSystemInfo::EXPLORE_Unexplored;
	m_systemInfo.randomLawlessness = false;
	m_systemInfo.randomFaction = system->GetFaction();
	m_systemInfo.faction = system->GetFaction() ? system->GetFaction()->name : "";
}

void SystemEditor::ClearSystem()
{
	GetUndo()->Clear();
	m_lastSavedUndoStack = GetUndo()->GetStateHash();

	m_system.Reset();
	m_systemInfo = {};
	m_viewport->SetSystem(m_system);
	SetSelectedBody(nullptr);
	m_pendingOp = {};

	m_filepath.clear();
	m_newSystemPath.systemIndex = 0;

	OnFilepathChanged();
}

// Here to avoid needing to drag in the Galaxy header in SystemEditor.h
RefCountedPtr<Galaxy> SystemEditor::GetGalaxy() { return m_galaxy; }

void SystemEditor::SetSelectedBody(SystemBody *body)
{
	// note: using const_cast here to work with Projectables which store a const pointer
	m_selectedBody = body;
	m_contextBody = body;
}

void SystemEditor::Start()
{
}

void SystemEditor::End()
{
}

void SystemEditor::RegisterMenuActions()
{
	m_menuBinder->BeginMenu("File");

	m_menuBinder->AddAction("New", {
		"New System", ImGuiKey_N | ImGuiKey_ModCtrl,
		[&]() {
			if (HasUnsavedChanges()) {
				m_unsavedFileModal = m_app->PushModal<UnsavedFileModal>();
				m_pendingFileReq = FileRequest_New;
				return;
			}

			ActivateNewSystemDialog();
		}
	});

	m_menuBinder->AddAction("Open", {
		"Open File", ImGuiKey_O | ImGuiKey_ModCtrl,
		[&]() {
			if (HasUnsavedChanges()) {
				m_unsavedFileModal = m_app->PushModal<UnsavedFileModal>();
				m_pendingFileReq = FileRequest_Open;
				return;
			}

			ActivateOpenDialog();
		}
	});

	m_menuBinder->AddAction("Save", {
		"Save", ImGuiKey_S | ImGuiKey_ModCtrl,
		[&]() { return m_system.Valid(); },
		sigc::mem_fun(this, &SystemEditor::SaveCurrentFile)
	});

	m_menuBinder->AddAction("SaveAs", {
		"Save As", ImGuiKey_S | ImGuiKey_ModCtrl | ImGuiKey_ModShift,
		[&]() { return m_system.Valid(); },
		sigc::mem_fun(this, &SystemEditor::ActivateSaveDialog)
	});

	m_menuBinder->AddAction("Quit", {
		"Quit", ImGuiKey_Q | ImGuiKey_ModCtrl,
		[this]() {
			if (HasUnsavedChanges()) {
				m_unsavedFileModal = m_app->PushModal<UnsavedFileModal>();
				m_pendingFileReq = FileRequest_Quit;
			} else {
				RequestEndLifecycle();
			}
		}
	});

	m_menuBinder->EndMenu();

	m_menuBinder->BeginMenu("Edit");

	auto hasSelectedBody = [&]() { return m_contextBody != nullptr; };
	auto hasParentBody = [&]() { return m_contextBody && m_contextBody->GetParent(); };

	m_menuBinder->BeginGroup("Body");

	m_menuBinder->AddAction("Center", {
		"Center on Body", {}, hasSelectedBody,
		[&]() {
			Projectable p = { Projectable::OBJECT, Projectable::SYSTEMBODY, m_contextBody };
			m_viewport->GetMap()->SetViewedObject(p);
		}
	});

	m_menuBinder->AddAction("AddChild", {
		"Add Child", ImGuiKey_A | ImGuiKey_ModCtrl, hasSelectedBody,
		[&]() {
			m_pendingOp.type = BodyRequest::TYPE_Add;
			m_pendingOp.parent = m_contextBody ? m_contextBody : m_system->GetRootBody().Get();
			m_pendingOp.newBodyType = SystemBody::BodyType::TYPE_GRAVPOINT;
		}
	});

	m_menuBinder->AddAction("AddSibling", {
		"Add Sibling", ImGuiKey_A | ImGuiKey_ModCtrl | ImGuiKey_ModShift, hasParentBody,
		[&]() {
			m_pendingOp.type = BodyRequest::TYPE_Add;
			m_pendingOp.parent = m_contextBody->GetParent();
			m_pendingOp.idx = SystemBody::EditorAPI::GetIndexInParent(m_contextBody) + 1;
			m_pendingOp.newBodyType = SystemBody::BodyType::TYPE_GRAVPOINT;
		}
	});

	m_menuBinder->AddAction("Delete", {
		"Delete Body", ImGuiKey_W | ImGuiKey_ModCtrl, hasParentBody,
		[&]() {
			m_pendingOp.type = BodyRequest::TYPE_Delete;
			m_pendingOp.body = m_contextBody;
		}
	});

	m_menuBinder->EndGroup();

	m_menuBinder->AddAction("Sort", {
		"Sort Bodies", ImGuiKey_None,
		[&]() { return m_system.Valid(); },
		[&]() { m_pendingOp.type = BodyRequest::TYPE_Resort; }
	});

	m_menuBinder->EndGroup();
}

void SystemEditor::OnFilepathChanged()
{
	if (m_filepath.empty()) {
		SDL_SetWindowTitle(m_app->GetRenderer()->GetSDLWindow(), "System Editor");
	} else {
		std::string windowTitle = fmt::format("System Editor - {}", m_filepath);
		SDL_SetWindowTitle(m_app->GetRenderer()->GetSDLWindow(), windowTitle.c_str());
	}
}

bool SystemEditor::HasUnsavedChanges()
{
	return (m_system && GetUndo()->GetStateHash() != m_lastSavedUndoStack);
}

void SystemEditor::SaveCurrentFile()
{
	// Cannot write back .lua files
	if (m_filepath.empty() || ends_with_ci(m_filepath, ".lua")) {
		ActivateSaveDialog();
	} else {
		WriteSystem(m_filepath);
	}
}

void SystemEditor::OnSaveComplete(bool success)
{
	if (!success) {
		// Cancel any pending actions if we failed to save or cancelled the process
		m_pendingFileReq = FileRequest_None;
		return;
	}

	m_lastSavedUndoStack = GetUndo()->GetStateHash();

	if (m_pendingFileReq != FileRequest_None) {
		HandlePendingFileRequest();
	}
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

	m_fileActionModal = m_app->PushModal<FileActionOpenModal>();
}

void SystemEditor::ActivateSaveDialog()
{
	m_saveFile.reset(new pfd::save_file(
		"Save Custom System File",
		FileSystem::JoinPath(FileSystem::GetDataDir(), m_filedir),
		{
			"JSON System Definition (.json)", "*.json"
		})
	);

	m_fileActionModal = m_app->PushModal<FileActionOpenModal>();
}

void SystemEditor::ActivateNewSystemDialog()
{
	m_newSystemModal = m_app->PushModal<NewSystemModal>(this, &m_newSystemPath);
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

			// Ensure we're saving JSON files on Windows
			if (!ends_with_ci(filePath, ".json")) {
				filePath += ".json";
			}

			// Update current file path and directory from new "save-as" path
			if (WriteSystem(filePath)) {
				m_filepath = filePath;
				m_filedir = filePath.substr(0, filePath.find_last_of("/\\"));

				OnFilepathChanged();
			}
		} else {
			// Signal cancellation/failure to save
			OnSaveComplete(false);
		}
	}

	// User responded to the unsaved changes modal
	if (m_unsavedFileModal && m_unsavedFileModal->Ready()) {
		auto result = m_unsavedFileModal->Result();

		if (result == UnsavedFileModal::Result_Cancel) {
			m_pendingFileReq = FileRequest_None;
		} else if (result == UnsavedFileModal::Result_No) {
			// User doesn't want to save, lose unsaved state
			HandlePendingFileRequest();
		} else {
			// Trigger the save-file flow
			m_menuBinder->TriggerAction("File.Save");
		}

		m_unsavedFileModal.Reset();
	}

	// Finished with all OS file dialogs, can close the file action modal
	if (m_fileActionModal && !m_openFile && !m_saveFile) {
		m_fileActionModal->Close();
		m_fileActionModal.Reset();
	}
}

void SystemEditor::HandlePendingFileRequest()
{
	if (m_pendingFileReq == FileRequest_New) {
		ActivateNewSystemDialog();
	}

	if (m_pendingFileReq == FileRequest_Open) {
		ActivateOpenDialog();
	}

	if (m_pendingFileReq == FileRequest_Quit) {
		ClearSystem();
		RequestEndLifecycle();
	}

	m_pendingFileReq = FileRequest_None;
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
			Log::Error("Body parameters could not be automatically generated for the new body.");

			body = StarSystem::EditorAPI::NewBody(m_system.Get());
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

		// Give the body a basic name based on its position in its parent
		SystemBody::EditorAPI::GenerateDefaultName(body);
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

	m_binderWindowOpen = false;
	m_debugWindowOpen = false;
	m_metricsWindowOpen = false;
	m_undoStackWindowOpen = false;
	m_resetDockingLayout = false;
}

void SystemEditor::DrawInterface()
{
	static bool isFirstRun = true;

	Draw::BeginHostWindow("HostWindow", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar())
		DrawMenuBar();

	ImGuiID dockspaceID = ImGui::GetID("DockSpace");

	if (!ImGui::DockBuilderGetNode(dockspaceID) || m_resetDockingLayout)
		SetupLayout(dockspaceID);

	ImGui::DockSpace(dockspaceID);

	// Needs to be inside host-context window
	m_menuBinder->Update();

	ImGui::End();

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
		// Adjust default item label position
		ImGui::PushItemWidth(ImFloor(ImGui::GetWindowSize().x * 0.6f));
		ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 13));

		if (m_selectedBody)
			DrawBodyProperties();
		else
			DrawSystemProperties();

		ImGui::PopFont();
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

	if (m_binderWindowOpen)
		m_menuBinder->DrawOverview("Shortcut List", &m_binderWindowOpen);

	if (m_undoStackWindowOpen)
		Draw::ShowUndoDebugWindow(GetUndo(), &m_undoStackWindowOpen);

	if (m_metricsWindowOpen)
		ImGui::ShowMetricsWindow(&m_metricsWindowOpen);

	if (m_debugWindowOpen)
		ImGui::ShowDebugLogWindow(&m_debugWindowOpen);

	if (isFirstRun)
		isFirstRun = false;
}

void SystemEditor::DrawMenuBar()
{
	m_menuBinder->DrawMenuBar();

	if (ImGui::BeginMenu("Windows")) {
		if (ImGui::MenuItem("Metrics Window", nullptr, m_metricsWindowOpen))
			m_metricsWindowOpen = !m_metricsWindowOpen;

		if (ImGui::MenuItem("Undo Stack", nullptr, m_undoStackWindowOpen))
			m_undoStackWindowOpen = !m_undoStackWindowOpen;

		if (ImGui::MenuItem("Shortcut List", nullptr, m_binderWindowOpen))
			m_binderWindowOpen = !m_binderWindowOpen;

		if (ImGui::MenuItem("ImGui Debug Log", nullptr, m_debugWindowOpen))
			m_debugWindowOpen = !m_debugWindowOpen;

		if (ImGui::MenuItem("Reset Layout"))
			m_resetDockingLayout = true;

		ImGui::EndMenu();
	}

	if (HasUnsavedChanges()) {
		ImGui::Text("*");
	}

	ImGui::EndMenuBar();
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
		SetSelectedBody(body);
	}

	if (ImGui::IsItemClicked(0) && ImGui::IsMouseDoubleClicked(0)) {
		m_viewport->GetMap()->SetViewedObject({ Projectable::OBJECT, Projectable::SYSTEMBODY, body });
	}

	// TODO: custom rendering on body entry, e.g. icon / contents etc.

	DrawBodyContextMenu(body);

	return open && body->GetNumChildren();
}

void SystemEditor::DrawBodyProperties()
{
	ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 16));
	ImGui::Text("Body: %s (%d)", m_selectedBody->GetName().c_str(), m_selectedBody->GetPath().bodyIndex);
	ImGui::PopFont();

	ImGui::Spacing();

	ImGui::PushID(m_selectedBody);

	SystemBody::EditorAPI::EditBodyName(m_selectedBody, GetRng(), m_nameGen.get(), GetUndo());

	SystemBody::EditorAPI::EditProperties(m_selectedBody, GetRng(), GetUndo());

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

	StarSystem::EditorAPI::EditName(m_system.Get(), GetRng(), GetUndo());

	StarSystem::EditorAPI::EditProperties(m_system.Get(), m_systemInfo, m_galaxy->GetFactions(), GetUndo());

}

void SystemEditor::DrawBodyContextMenu(SystemBody *body)
{
	if (ImGui::BeginPopupContextItem()) {
		ImGui::PushFont(m_app->GetPiGui()->GetFont("pionillium", 15));

		m_contextBody = body;
		m_menuBinder->DrawGroup("Edit.Body");
		m_contextBody = m_selectedBody;

		ImGui::PopFont();
		ImGui::EndPopup();
	}
}
