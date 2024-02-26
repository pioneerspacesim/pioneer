// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "SystemEditorModals.h"

#include "Input.h"
#include "Random.h"
#include "RefCounted.h"
#include "core/Application.h"
#include "galaxy/SystemPath.h"
#include "GalaxyEditAPI.h"

#include <memory>

// Forward declaration
typedef unsigned int ImGuiID;

namespace pfd {
	class open_file;
	class save_file;
} // namespace pfd

namespace FileSystem {
	class FileInfo;
} // namespace FileSystem

class LuaNameGen;

class Galaxy;
class StarSystem;
class SystemBody;
class CustomSystem;
class CustomSystemsDatabase;

namespace Editor {

class EditorApp;
class UndoSystem;
class SystemEditorViewport;
class ActionBinder;

const char *GetBodyIcon(const SystemBody *body);

class SystemEditor : public Application::Lifecycle {
public:
	SystemEditor(EditorApp *app);
	~SystemEditor();

	void NewSystem(SystemPath path);
	bool LoadSystem(SystemPath path);
	bool LoadSystemFromDisk(const std::string &absolutePath);

	// Write the currently edited system out to disk as a JSON file
	bool WriteSystem(const std::string &filepath);

	Random &GetRng() { return m_random; }
	RefCountedPtr<Galaxy> GetGalaxy();

	void SetSelectedBody(SystemBody *body);
	SystemBody *GetSelectedBody() { return m_selectedBody; }

	void DrawBodyContextMenu(SystemBody *body);

protected:
	void Start() override;
	void Update(float deltaTime) override;
	void End() override;

	void HandleInput();

private:
	bool LoadSystemFromFile(const FileSystem::FileInfo &file);
	bool LoadCustomSystem(const CustomSystem *system);
	void LoadSystemFromGalaxy(RefCountedPtr<StarSystem> system);
	void ClearSystem();

	void OnFilepathChanged();

	void RegisterMenuActions();

	bool HasUnsavedChanges();
	void SaveCurrentFile();
	void OnSaveComplete(bool success);

	void ActivateOpenDialog();
	void ActivateSaveDialog();
	void ActivateNewSystemDialog();

	void HandlePendingFileRequest();
	void HandleBodyOperations();

	void SetupLayout(ImGuiID dockspaceID);
	void DrawInterface();

	void DrawMenuBar();

	bool DrawBodyNode(SystemBody *body, bool isRoot);
	void HandleOutlinerDragDrop(SystemBody *refBody);
	void DrawOutliner();

	void DrawBodyProperties();
	void DrawSystemProperties();

	void DrawUndoDebug();

	UndoSystem *GetUndo() { return m_undo.get(); }

private:
	class UndoSetSelection;

	// Pending file actions which triggered an unsaved changes modal
	enum FileRequestType {
		FileRequest_None,
		FileRequest_Open,
		FileRequest_New,
		FileRequest_Quit
	};

	// Pending actions to the body tree hierarchy that should
	// be handled at the end of the frame
	struct BodyRequest {
		enum Type {
			TYPE_None,
			TYPE_Add,
			TYPE_Delete,
			TYPE_Reparent,
			TYPE_Resort
		};

		Type type = TYPE_None;
		uint32_t newBodyType = 0; // SystemBody::BodyType
		SystemBody *parent = nullptr;
		SystemBody *body = nullptr;
		size_t idx = 0;
	};

	EditorApp *m_app;

	RefCountedPtr<Galaxy> m_galaxy;
	RefCountedPtr<StarSystem> m_system;
	std::unique_ptr<CustomSystemsDatabase> m_systemLoader;

	CustomSystemInfo m_systemInfo;

	std::unique_ptr<SystemEditorViewport> m_viewport;

	Random m_random;
	std::unique_ptr<class LuaNameGen> m_nameGen;

	std::unique_ptr<UndoSystem> m_undo;
	size_t m_lastSavedUndoStack;

	std::string m_filepath;
	std::string m_filedir;

	SystemBody *m_selectedBody;
	SystemBody *m_contextBody;

	BodyRequest m_pendingOp;

	FileRequestType m_pendingFileReq;

	std::unique_ptr<pfd::open_file> m_openFile;
	std::unique_ptr<pfd::save_file> m_saveFile;

	SystemPath m_openSystemPath;

	RefCountedPtr<FileActionOpenModal> m_fileActionModal;
	RefCountedPtr<UnsavedFileModal> m_unsavedFileModal;
	RefCountedPtr<NewSystemModal> m_newSystemModal;

	SystemPath m_newSystemPath;

	std::unique_ptr<ActionBinder> m_menuBinder;

	bool m_metricsWindowOpen = false;
	bool m_undoStackWindowOpen = false;
	bool m_binderWindowOpen = false;
	bool m_debugWindowOpen = false;

	bool m_resetDockingLayout = false;
};

} // namespace Editor
