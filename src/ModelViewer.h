// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef MODELVIEWER_H
#define MODELVIEWER_H

// viewer for sgmodels

#include "libs.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "LmrTypes.h"
#include "LuaManager.h"
#include "scenegraph/SceneGraph.h"
#include "ui/Context.h"

class ModelViewer {
public:
	ModelViewer(Graphics::Renderer *r, LuaManager *l);
	~ModelViewer();

	static void Run(const std::string &modelName);

private:
	bool OnAnimPlay(UI::Widget*, bool reverse);
	bool OnAnimStop(UI::Widget*);
	bool OnPickModel(UI::List*);
	bool OnQuit();
	bool OnReloadModel(UI::Widget*);
	bool OnToggleCollMesh(UI::CheckBox*);
	bool OnToggleGrid(UI::Widget*);
	bool OnToggleGuns(UI::CheckBox*);
	void AddLog(const std::string &line);
	void ChangeCameraPreset(SDLKey, SDLMod);
	void ClearModel();
	void DrawBackground();
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

	//toggleable options
	struct Options {
		bool attachGuns;
		bool showCollMesh;
		bool showGrid;
		bool showUI;
		bool wireframe;
		float gridInterval;
		int lightPreset;

		Options();
	};
	bool m_done;
	bool m_playing;
	bool m_screenshotQueued;
	double m_animTime; //separate, because it may be paused
	double m_frameTime;
	Graphics::Renderer *m_renderer;
	Graphics::Texture *m_decalTexture;
	float m_rotX, m_rotY;
	ModelParams m_modelParams;
	MTRand m_rng;
	SceneGraph::Animation *m_currentAnimation;
	SceneGraph::Model *m_model;
	Options m_options;
	RefCountedPtr<SceneGraph::ModelNode> m_gunModelNode;
	RefCountedPtr<UI::Context> m_ui;
	ScopedPtr<SceneGraph::Model> m_gunModel;
	std::string m_modelName;
	vector3f m_camPos;

	//undecided on this input stuff
	//updating the states of all inputs during PollEvents
	bool m_keyStates[SDLK_LAST];
	bool m_mouseButton[SDL_BUTTON_WHEELDOWN + 1]; //buttons + scroll start at 1
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
};

#endif
