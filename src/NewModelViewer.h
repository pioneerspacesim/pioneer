#ifndef _NEWMODELVIWEWER_H
#define _NEWMODELVIEWER_H
/*
 * Viewer for the new format models.
 * Has nothing to do with Lua.
 */
#include "libs.h"
#include "graphics/Renderer.h"
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
	void DrawBackground();
	void DrawGrid(const matrix4x4f &trans, float radius);
	void DrawModel();
	void MainLoop();
	void PollEvents();
	void ResetCamera();
	void SetModel(const std::string& name);
	void SetupUI();
	void UpdateCamera();

	bool m_done;
	CollMesh *m_collMesh;
	float m_frameTime;
	Graphics::Renderer *m_renderer;
	int m_height;
	int m_width;
	matrix4x4f m_modelRot;
	ModelParams m_modelParams;
	Newmodel::NModel *m_model;
	RefCountedPtr<UI::Context> m_ui;
	vector3f m_camPos;

	//undecided on this input stuff
	std::array<bool, SDLK_LAST> m_keyStates;
	std::array<int, 2> m_mouseMotion;
	std::array<bool, SDL_BUTTON_WHEELDOWN + 1> m_mouseButton; //buttons + scroll start at 1

	//unorganized interface stuff
	UI::Label *nameLabel;
};

#endif
