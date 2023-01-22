// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef MODELVIEWER_H
#define MODELVIEWER_H

#include "Input.h"
#include "NavLights.h"
#include "Shields.h"
#include "core/GuiApplication.h"
#include "graphics/Drawables.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "libs.h"
#include "lua/LuaManager.h"
#include "pigui/PiGui.h"
#include "scenegraph/SceneGraph.h"

#include <memory>

class ModelViewer;

class ModelViewerApp : public GuiApplication {
public:
	ModelViewerApp() :
		GuiApplication("Model Viewer")
	{}

	void SetInitialModel(std::string &modelName) { m_modelName = modelName; }
	std::string &GetModelName() { return m_modelName; }

protected:
	void Startup() override;
	void Shutdown() override;

	void PreUpdate() override;
	void PostUpdate() override;

	friend class ModelViewer;

private:
	std::string m_modelName;
	RefCountedPtr<ModelViewer> m_modelViewer;
};

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

	ModelViewer(ModelViewerApp *app, LuaManager *l);

	void SetModel(const std::string &modelName);
	bool SetRandomColor();
	void ResetCamera();
	void ChangeCameraPreset(CameraPreset preset);

protected:
	void Start() override;
	void Update(float deltaTime) override;
	void End() override;
	void SetupAxes();
	void HandleInput();

private:
	void AddLog(const std::string &line);

	void UpdateModelList();
	void UpdateDecalList();
	void UpdateShield();

	void UpdateCamera(float deltaTime);
	void UpdateLights();

	void ReloadModel();
	void SetDecals(const std::string &file);

	void OnModelChanged();

	void ToggleGuns();
	void HitIt();

	void ToggleViewControlMode();
	void ClearModel();
	void CreateTestResources();
	void DrawBackground();
	void DrawGrid(const matrix4x4f &trans, float radius);
	void DrawModel(const matrix4x4f &mv);

	void ResetThrusters();
	void Screenshot();
	void SaveModelToBinary();

	void DrawModelSelector();
	void DrawModelOptions();
	void DrawModelTags();
	void DrawTagNames();
	void DrawModelHierarchy();
	void DrawShipControls();
	void DrawLog();
	void DrawPiGui();

private:
	//toggleable options
	struct Options {
		bool attachGuns;
		bool showTags;
		bool showDockingLocators;
		bool showCollMesh;
		bool showAabb;
		bool showGeomBBox;
		bool showShields;
		bool showGrid;
		bool showVerticalGrids;
		bool showLandingPad;
		bool showUI;
		bool wireframe;
		bool mouselookEnabled;
		float gridInterval;
		uint32_t lightPreset;
		bool orthoView;
		bool metricsWindow;

		Options();
	};

private:
	Input::Manager *m_input;
	PiGui::Instance *m_pigui;

	struct Inputs : Input::InputFrame {
		using InputFrame::InputFrame;

		Axis *moveForward;
		Axis *moveLeft;
		Axis *moveUp;
		Axis *zoomAxis;

		Axis *rotateViewLeft;
		Axis *rotateViewUp;

		Action *viewTop;
		Action *viewLeft;
		Action *viewFront;
	} m_bindings;

	vector2f m_windowSize;
	vector2f m_logWindowSize;
	vector2f m_animWindowSize;
	std::vector<std::string> m_log;
	bool m_resetLogScroll = false;

	vector3f m_linearThrust = {};
	vector3f m_angularThrust = {};

	// Model pattern colors
	std::vector<Color> m_colors;

	std::vector<std::string> m_fileNames;
	std::string m_modelName;
	std::string m_requestedModelName;

	std::unique_ptr<SceneGraph::Model> m_model;
	bool m_modelIsShip = false;

	SceneGraph::MatrixTransform *m_selectedTag = nullptr;

	std::vector<SceneGraph::Animation *> m_animations;
	SceneGraph::Animation *m_currentAnimation = nullptr;

	bool m_modelSupportsPatterns = false;
	std::vector<std::string> m_patterns;
	uint32_t m_currentPattern = 0;

	bool m_modelSupportsDecals = false;
	std::vector<std::string> m_decals;
	uint32_t m_currentDecal = 0;

	bool m_modelHasShields = false;
	std::unique_ptr<Shields> m_shields;
	std::unique_ptr<NavLights> m_navLights;
	std::unique_ptr<SceneGraph::Model> m_gunModel;
	std::unique_ptr<SceneGraph::Model> m_scaleModel;

	bool m_screenshotQueued;
	bool m_shieldIsHit;
	float m_shieldHitPan;
	Graphics::Renderer *m_renderer;
	Graphics::Texture *m_decalTexture;
	matrix4x4f m_modelViewMat;
	vector3f m_viewPos;
	matrix3x3f m_viewRot;
	float m_rotX, m_rotY, m_zoom;
	float m_baseDistance;
	Random m_rng;

	Options m_options;
	float m_landingMinOffset;

	std::unique_ptr<Graphics::Material> m_bgMaterial;
	std::unique_ptr<Graphics::MeshObject> m_bgMesh;

	sigc::signal<void> onModelChanged;

	std::unique_ptr<Graphics::Drawables::GridLines> m_gridLines;
};

#endif
