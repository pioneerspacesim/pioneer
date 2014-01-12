// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef MODELVIEWER_H
#define MODELVIEWER_H

// viewer for sgmodels

#include "libs.h"
#include "LuaManager.h"
#include "NavLights.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "graphics/Drawables.h"
#include "scenegraph/SceneGraph.h"
#include "ui/Context.h"

class ModelViewer {
public:
	ModelViewer(Graphics::Renderer *r, LuaManager *l);
	~ModelViewer();

	static void Run(const std::string &modelName);

private:
	bool OnPickModel(UI::List*);
	bool OnQuit();
	bool OnReloadModel(UI::Widget*);
	bool OnToggleCollMesh(UI::CheckBox*);
	bool OnToggleGrid(UI::Widget*);
	bool OnToggleGuns(UI::CheckBox*);
	void AddLog(const std::string &line);
	void ChangeCameraPreset(SDL_Keycode, SDL_Keymod);
	void ToggleViewControlMode();
	void ClearLog();
	void ClearModel();
	void CreateTestResources();
	void DrawBackground();
	void DrawTags();
	void DrawDockingLocators();
	void DrawCollisionMesh();
	void DrawGrid(const matrix4x4f &trans, float radius);
	void DrawLog();
	void DrawModel();
	void MainLoop();
	void OnAnimChanged(unsigned int, const std::string&);
	void OnAnimSliderChanged(float);
	void OnDecalChanged(unsigned int, const std::string&);
	void OnLightPresetChanged(unsigned int index, const std::string &);
	void OnModelColorsChanged(float);
	void OnPatternChanged(unsigned int, const std::string&);
	void OnThrustChanged(float);
	void PollEvents();
	void ResetCamera();
	void ResetThrusters();
	void Screenshot();
	void SetModel(const std::string& name, bool resetCamera = true);
	void SetupFilePicker();
	void SetupUI();
	void UpdateAnimList();
	void UpdateCamera();
	void UpdateLights();
	void UpdatePatternList();
	void AddAxisIndicator(const SceneGraph::Model::TVecMT &mts, std::vector<Graphics::Drawables::Line3D> &lines);

	//toggleable options
	struct Options {
		bool attachGuns;
		bool showTags;
		bool showDockingLocators;
		bool showCollMesh;
		bool showGrid;
		bool showLandingPad;
		bool showUI;
		bool wireframe;
		bool mouselookEnabled;
		float gridInterval;
		int lightPreset;

		Options();
	};
	bool m_done;
	bool m_screenshotQueued;
	double m_frameTime;
	Graphics::Renderer *m_renderer;
	Graphics::Texture *m_decalTexture;
	vector3f m_viewPos;
	matrix3x3f m_viewRot;
	float m_rotX, m_rotY, m_zoom;
	float m_baseDistance;
	Random m_rng;
	SceneGraph::Animation *m_currentAnimation;
	SceneGraph::Model *m_model;
	Options m_options;
	std::unique_ptr<NavLights> m_navLights;
	std::unique_ptr<SceneGraph::Model> m_gunModel;
	std::unique_ptr<SceneGraph::Model> m_scaleModel;
	std::string m_modelName;
	RefCountedPtr<UI::Context> m_ui;

	//undecided on this input stuff
	//updating the states of all inputs during PollEvents
	std::map<SDL_Keycode,bool> m_keyStates;
	bool m_mouseButton[SDL_BUTTON_RIGHT + 1]; //buttons start at 1
	int m_mouseMotion[2];
	bool m_mouseWheelUp, m_mouseWheelDown;

	//interface stuff that needs to be accessed later (unorganized)
	UI::MultiLineText *m_log;
	RefCountedPtr<UI::Scroller> m_logScroller;

	UI::DropDown *animSelector;
	UI::DropDown *patternSelector;
	UI::DropDown *decalSelector;
	UI::Label *nameLabel;
	UI::Slider *animSlider;
	UI::Label *animValue;
	UI::Slider *colorSliders[9];
	UI::Slider *thrustSliders[2*3]; //thruster sliders 2*xyz (linear & angular)

	sigc::signal<void> onModelChanged;

	std::vector<Graphics::Drawables::Line3D> m_dockingPoints;
	std::vector<Graphics::Drawables::Line3D> m_tagPoints;
};

#endif
