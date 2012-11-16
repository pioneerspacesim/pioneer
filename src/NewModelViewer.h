#ifndef _NEWMODELVIWEWER_H
#define _NEWMODELVIEWER_H
/*
 * Viewer for the new format models.
 * Has nothing to do with Lua.
 */
#include "libs.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "LmrTypes.h"
#include "LuaManager.h"
#include "newmodel/NModel.h"
#include "newmodel/ModelNode.h"
#include "ui/Context.h"

class ModelViewer {
public:
	ModelViewer(Graphics::Renderer *r, LuaManager *l, int width, int height);
	~ModelViewer();

	static void Run(int argc, char** argv);

private:
	bool OnAnimPlay(UI::Widget *w, bool reverse);
	bool OnAnimStop(UI::Widget *w);
	bool OnReloadModel(UI::Widget *w);
	bool OnToggleCollMesh(UI::CheckBox *w);
	bool OnToggleGrid(UI::Widget *);
	bool OnToggleGuns(UI::CheckBox *w);
	void AddLog(const std::string &line);
	void ChangeCameraPreset(SDLKey, SDLMod);
	void DrawBackground();
	void DrawCollisionMesh();
	void DrawGrid(const matrix4x4f &trans, float radius);
	void DrawLog();
	void DrawModel();
	void MainLoop();
	void OnAnimChanged(unsigned int, const std::string&);
	void OnAnimSliderChanged(float);
	void OnLightPresetChanged(unsigned int index, const std::string &);
	void OnModelColorsChanged(float);
	void OnPatternChanged(unsigned int, const std::string&);
	void OnThrustChanged(float);
	void PollEvents();
	void ResetCamera();
	void ResetThrusters();
	void Screenshot();
	void SetModel(const std::string& name, bool resetCamera = true);
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
	int m_height;
	int m_width;
	matrix4x4f m_modelRot;
	ModelParams m_modelParams;
	MTRand m_rng;
	Newmodel::Animation *m_currentAnimation;
	Newmodel::NModel *m_model;
	Options m_options;
	RefCountedPtr<Newmodel::ModelNode> m_gunModelNode;
	RefCountedPtr<UI::Context> m_ui;
	ScopedPtr<Newmodel::NModel> m_gunModel;
	std::string m_modelName;
	vector3f m_camPos;

	//undecided on this input stuff
	//updating the states of all inputs during PollEvents
	bool m_keyStates[SDLK_LAST];
	bool m_mouseButton[SDL_BUTTON_WHEELDOWN + 1]; //buttons + scroll start at 1
	int m_mouseMotion[2];

	//interface stuff that needs to be accessed later (unorganized)
	UI::MultiLineText *m_log;
	UI::Scroller *m_logScroller;

	UI::DropDown *animSelector;
	UI::DropDown *patternSelector;
	UI::Label *nameLabel;
	UI::Slider *animSlider;
	UI::Slider *colorSliders[9];
	UI::Slider *thrustSliders[2*3]; //thruster sliders 2*xyz (linear & angular)

	sigc::signal<void> onModelChanged;
};

#endif
