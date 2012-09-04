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
#include "ui/Context.h"
#include <array>

class ModelViewer {
public:
	ModelViewer(Graphics::Renderer *r, LuaManager *l, int width, int height);
	~ModelViewer();

	static void Run(int argc, char** argv);

private:
	bool OnReloadModel(UI::Widget *w);
	bool OnToggleGrid(UI::Widget *);
	void AddLog(const std::string &line);
	void DrawBackground();
	void DrawGrid(const matrix4x4f &trans, float radius);
	void DrawLog();
	void DrawModel();
	void MainLoop();
	void OnLightPresetChanged(unsigned int index, const std::string &);
	void PollEvents();
	void ResetCamera();
	void Screenshot();
	void SetModel(const std::string& name);
	void SetupUI();
	void UpdateCamera();
	void UpdateLights();

	//toggleable options
	struct Options {
		bool showGrid;
		bool showUI;
		float gridInterval;
		int lightPreset;

		Options();
	};
	bool m_done;
	bool m_screenshotQueued;
	CollMesh *m_collMesh;
	float m_frameTime;
	Graphics::Renderer *m_renderer;
	Graphics::Texture *m_decalTexture;
	int m_height;
	int m_width;
	matrix4x4f m_modelRot;
	ModelParams m_modelParams;
	Newmodel::NModel *m_model;
	Options m_options;
	RefCountedPtr<UI::Context> m_ui;
	std::list<std::string> m_logLines;
	std::string m_logString;
	std::string m_modelName;
	vector3f m_camPos;

	//undecided on this input stuff
	//updating the states of all inputs during PollEvents
	std::array<bool, SDLK_LAST> m_keyStates;
	std::array<int, 2> m_mouseMotion;
	std::array<bool, SDL_BUTTON_WHEELDOWN + 1> m_mouseButton; //buttons + scroll start at 1

	//interface stuff that needs to be accessed later (unorganized)
	UI::Label *nameLabel;
};

#endif
