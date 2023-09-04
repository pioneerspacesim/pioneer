// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "Input.h"
#include "Random.h"
#include "RefCounted.h"
#include "core/Application.h"
#include "galaxy/SystemPath.h"

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

const char *GetBodyIcon(const SystemBody *body);

class SystemEditor : public Application::Lifecycle {
public:
	SystemEditor(EditorApp *app);
	~SystemEditor();

	void NewSystem();
	bool LoadSystemFromDisk(const std::string &absolutePath);

	// Write the currently edited system out to disk as a JSON file
	bool WriteSystem(const std::string &filepath);

	Random &GetRng() { return m_random; }
	RefCountedPtr<Galaxy> GetGalaxy();

	void SetSelectedBody(SystemBody *body);
	SystemBody *GetSelectedBody() { return m_selectedBody; }

protected:
	void Start() override;
	void Update(float deltaTime) override;
	void End() override;

	void HandleInput();

private:
	void ClearSystem();
	bool LoadSystem(const FileSystem::FileInfo &file);
	bool LoadCustomSystem(const CustomSystem *system);
	void LoadSystemFromGalaxy(RefCountedPtr<StarSystem> system);

	void SetupLayout(ImGuiID dockspaceID);
	void DrawInterface();

	bool DrawBodyNode(SystemBody *body, bool isRoot);
	void HandleOutlinerDragDrop(SystemBody *refBody);
	void DrawOutliner();

	void DrawBodyProperties();
	void DrawSystemProperties();

	void EditName(const char *undo_label, std::string *name);

	void DrawUndoDebug();

	void ActivateOpenDialog();
	void ActivateSaveDialog();

	void DrawFileActionModal();
	void DrawPickSystemModal();

	void HandleBodyOperations();

	UndoSystem *GetUndo() { return m_undo.get(); }

private:
	class UndoSetSelection;

	EditorApp *m_app;

	RefCountedPtr<Galaxy> m_galaxy;
	RefCountedPtr<StarSystem> m_system;
	std::unique_ptr<CustomSystemsDatabase> m_systemLoader;

	std::unique_ptr<SystemEditorViewport> m_viewport;

	Random m_random;
	std::unique_ptr<class LuaNameGen> m_nameGen;

	std::unique_ptr<UndoSystem> m_undo;
	size_t m_lastSavedUndoStack;

	std::string m_filepath;
	std::string m_filedir;

	SystemBody *m_selectedBody;

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

	BodyRequest m_pendingOp;

	std::unique_ptr<pfd::open_file> m_openFile;
	std::unique_ptr<pfd::save_file> m_saveFile;

	SystemPath m_openSystemPath;

	ImGuiID m_fileActionActiveModal;
};

} // namespace Editor
