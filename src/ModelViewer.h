#include "collider/collider.h"
#include "gui/Gui.h"
#include "LuaManager.h"
#include "newmodel/Newmodel.h"
#include "ui/Context.h"

struct Options {
	bool attachGuns;
	bool showBoundingBoxes;
	bool showBoundingRadius;
	bool showGrid;
	float gridInterval;
	int lightPreset;

	Options()
	: attachGuns(false)
	, showBoundingBoxes(false)
	, showBoundingRadius(false)
	, showGrid(false)
	, gridInterval(0.f)
	, lightPreset(0)
	{ }
};

class Viewer: public Gui::Fixed {
private: //data members
	bool m_playing;
	bool m_screenshotQueued;
	CollisionSpace *m_space;
	CollMesh *m_cmesh;
	double m_currentTime;
	float m_frameTime;
	Geom *m_geom;
	Graphics::Texture *m_decalTexture;
	Gui::Label *m_trisReadout;
	matrix4x4f m_camOrient;
	matrix4x4f m_modelRot;
	ModelParams m_modelParams;
	Newmodel::NModel *m_model;
	Options m_options;
	RefCountedPtr<Newmodel::ModelNode> m_gunModelNode;
	ScopedPtr<LuaManager> m_luaManager;
	ScopedPtr<Model> m_gunModel;
	std::list<std::string> m_logLines;
	std::string m_logString;
	std::string m_modelName;
	UI::Context *m_ui;
	UI::DropDown *m_animSelector;
	UI::DropDown *m_patternSelector;
	UI::Slider *m_sliders[3*3]; //color sliders 3*rgb
	UI::Slider *m_tSliders[2*3]; //thruster sliders 2*xyz (linear & angular)
	vector3f m_campos;

private: //methods
	bool OnAnimPlay(UI::Widget *w, bool reverse);
	bool OnAnimStop(UI::Widget *w);
	bool OnReloadModel(UI::Widget *w);
	bool OnToggleBoundingRadius(UI::Widget *w);
	bool OnToggleGrid(UI::Widget *w);
	bool OnToggleGuns(UI::CheckBox *w);
	bool PickAnotherModel();
	void AddLog(const std::string &message);
	void ClearModel();
	void DrawGrid(matrix4x4f& trans, double radius);
	void DrawLog();
	void OnLightPresetChanged(unsigned int index, const std::string &);
	void OnModelColorsChanged(float v=0.f);
	void OnPatternChanged(unsigned int index, const std::string &);
	void OnThrustChanged(float v);
	void PollEvents();
	void ResetCamera();
	void ResetSliders();
	void Screenshot();
	void SetupUI();
	void TryModel(const SDL_keysym *sym, Gui::TextEntry *entry, Gui::Label *errormsg);
	void UpdateAnimList();
	void UpdateLights();
	void UpdatePatternList();
	void UpdateTime();
	void VisualizeBoundingRadius(matrix4x4f& trans, double radius);

public:
	Viewer(float w, float h);
	~Viewer();

	void SetModel(Newmodel::NModel *, const std::string &modelname);
	void PickModel(const std::string &initial_name, const std::string &initial_errormsg);
	void PickModel();

	void MainLoop() __attribute((noreturn));

private:
};
