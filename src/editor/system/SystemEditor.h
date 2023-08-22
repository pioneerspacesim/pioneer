// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "Input.h"
#include "Random.h"
#include "RefCounted.h"
#include "core/Application.h"

#include <memory>

// Forward declaration
typedef unsigned int ImGuiID;

class Galaxy;
class StarSystem;
class SystemBody;
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

	bool LoadSystem(const std::string &filepath);
	void WriteSystem(const std::string &filepath);

	Random &GetRng() { return m_random; }
	RefCountedPtr<Galaxy> GetGalaxy();

	void SetSelectedBody(SystemBody *body);
	SystemBody *GetSystemBody();

protected:
	void Start() override;
	void Update(float deltaTime) override;
	void End() override;

	void HandleInput();

private:
	void SetupLayout(ImGuiID dockspaceID);
	void DrawInterface();

	bool DrawBodyNode(SystemBody *body, bool isRoot);
	void HandleOutlinerDragDrop(SystemBody *refBody);
	void DrawOutliner();

	void DrawBodyProperties();
	void DrawSystemProperties();

	void EditName(const char *undo_label, std::string *name);

	void DrawUndoDebug();

	UndoSystem *GetUndo() { return m_undo.get(); }

private:
	class UndoSetSelection;

	EditorApp *m_app;

	RefCountedPtr<Galaxy> m_galaxy;
	RefCountedPtr<StarSystem> m_system;
	std::unique_ptr<CustomSystemsDatabase> m_systemLoader;

	std::unique_ptr<SystemEditorViewport> m_viewport;

	Random m_random;

	std::unique_ptr<UndoSystem> m_undo;

	std::string m_filepath;

	SystemBody *m_selectedBody;

	struct ReparentRequest {
		SystemBody *parent = nullptr;
		SystemBody *body = nullptr;
		size_t idx = 0;
	};

	ReparentRequest m_pendingReparent;
};

} // namespace Editor
