// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "Input.h"
#include "Shields.h"
#include "core/GuiApplication.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "pigui/PiGui.h"

#include <memory>

class LuaManager;

namespace PiGui {
	class Instance;
}

namespace SceneGraph {
	class Model;
	class Tag;
}

namespace Editor {

class EditorApp;
class ModelViewerWidget;

class ModelViewer : public Application::Lifecycle {
public:
	enum class CameraPreset : uint8_t {
		Front,
		Back,
		Left,
		Right,
		Top,
		Bottom
	};

	ModelViewer(EditorApp *app, LuaManager *l);
	~ModelViewer();

	void SetModel(const std::string &modelName);

protected:
	void Start() override;
	void Update(float deltaTime) override;
	void End() override;
	void HandleInput();

private:
	void AddLog(Time::DateTime, Log::Severity, std::string_view line);

	void UpdateModelList();
	void UpdateDecalList();
	void UpdateShield();

	void ReloadModel();
	void ClearModel();
	void OnModelLoaded();

	void ToggleGuns();
	void HitIt();

	void CreateTestResources();

	void ResetThrusters();
	void Screenshot();
	void SaveModelToBinary();

	void SetupLayout(ImGuiID dockspaceID);
	void DrawModelSelector();
	void DrawModelTags();
	void DrawTagNames();
	void DrawModelHierarchy();
	void DrawShipControls();
	void DrawLog();
	void DrawPiGui();

private:
	EditorApp *m_app;
	Input::Manager *m_input;
	PiGui::Instance *m_pigui;
	Graphics::Renderer *m_renderer;

	std::vector<std::string> m_log;
	bool m_resetLogScroll = false;

	vector3f m_linearThrust = {};
	vector3f m_angularThrust = {};

	std::unique_ptr<ModelViewerWidget> m_modelWindow;

	std::vector<std::string> m_fileNames;
	std::string m_modelName;
	std::string m_requestedModelName;

	SceneGraph::Tag *m_selectedTag = nullptr;

	bool m_modelSupportsDecals = false;
	std::vector<std::string> m_decals;
	uint32_t m_currentDecal = 0;
	Graphics::Texture *m_decalTexture;

	bool m_modelIsShip = false;
	bool m_modelHasShields = false;

	std::unique_ptr<Shields> m_shields;
	std::unique_ptr<SceneGraph::Model> m_gunModel;

	bool m_screenshotQueued = false;
	bool m_shieldIsHit = false;
	float m_shieldHitPan;

	bool m_attachGuns = false;
	bool m_showShields = false;
	bool m_showUI = true;
	bool m_metricsWindow = false;
};

} // namespace Editor
