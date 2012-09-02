#include "ModelViewer.h"
#include "libs.h"
#include "utils.h"
#include "FileSystem.h"
#include "ModManager.h"
#include "OS.h"
#include "StringF.h"
#include "graphics/Drawables.h"
#include "graphics/Graphics.h"
#include "graphics/Graphics.h"
#include "graphics/Light.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"
#include "graphics/VertexArray.h"
#include <sstream>

using Graphics::Light;
using Graphics::Renderer;
using Newmodel::NModel;

static char g_keyState[SDLK_LAST];
static const int MAX_MOUSE_BTN_IDX = SDL_BUTTON_WHEELDOWN + 1;
static int g_mouseButton[MAX_MOUSE_BTN_IDX];    // inc to 6 as mouseScroll is index 5
static int g_mouseMotion[2];
static int g_width, g_height;
static Renderer *renderer;
static SDL_Surface *g_screen;

//some utility functions
static bool set_mouse_button(const Uint8 idx, const int value)
{
    if( idx < MAX_MOUSE_BTN_IDX ) {
        g_mouseButton[idx] = value;
        return true;
    }
    return false;
}

static vector3f az_el_to_dir(float yaw, float pitch)
{
    //0,0 points to "right" (1,0,0)
    vector3f v;
    v.x = cos(DEG2RAD(yaw)) * cos(DEG2RAD(pitch));
    v.y = sin(DEG2RAD(pitch));
    v.z = sin(DEG2RAD(yaw)) * cos(DEG2RAD(pitch));
    return v;
}
//end utility funcs

//widget-label pair
static void add_pair(UI::Context *c, UI::Box *parent, UI::Widget *widget, const std::string &label)
{
    const Uint32 flags = 0;
    parent->PackEnd(
        c->HBox(5.f)->PackEnd(widget, flags)->PackEnd(c->Label(label)
        ->SetFontSize(UI::Widget::FONT_SIZE_SMALL), flags)
    );
}

//begin stuff
Viewer::Viewer(float _width, float _height) : Gui::Fixed(_width, _height),
    m_playing(false),
    m_screenshotQueued(false),
    m_cmesh(0),
    m_currentTime(0.001 * SDL_GetTicks()),
    m_geom(0),
    m_model(0),
    m_logString(""),
    m_modelName("")
{
    SetupUI();

    //load gun model for attachment test
    {
        Newmodel::Loader loader(renderer);
        try {
            Newmodel::NModel *m = loader.LoadModel("test_gun");
            m_gunModel.Reset(m);
            m_gunModelNode.Reset(new Newmodel::ModelNode(m_gunModel.Get()));
        } catch (Newmodel::LoadingError &) {
            AddLog("Could not load test_gun model");
        }
    }

    // sweet pioneer badge for decal testing
    m_decalTexture = Graphics::TextureBuilder(
        "icons/badge.png",
        Graphics::LINEAR_CLAMP,
        true, true, false).GetOrCreateTexture(renderer, "model");

    m_space = new CollisionSpace();
    Gui::Screen::AddBaseWidget(this, 0, 0);
    SetTransparency(true);

    ShowAll();
    Show();
}

Viewer::~Viewer()
{
    ClearModel();
    delete m_space;
    delete m_ui;
}

void Viewer::SetupUI()
{
    /*
     * To do: stats display
     */
	m_luaManager.Reset(new LuaManager());
	m_ui = new UI::Context(m_luaManager.Get(), renderer, g_width, g_height);
	UI::LuaInit();

    UI::Context *c = m_ui;
    UI::Box *box;
    UI::Box *buttBox; //it was for buttons...
    UI::Box *animBox;
    UI::Button *b1, *gridBtn, *reloadBtn;
    UI::Button *playBtn, *stopBtn, *revBtn;
    UI::CheckBox *radiusCheck, *gunsCheck;

    c->SetInnerWidget((box = c->VBox(5.f)));

    Uint32 battrs = 0;

    box->PackEnd((buttBox = c->VBox(5.f)), battrs);
    add_pair(c, buttBox, (b1 = c->Button()), "Pick another model");
    add_pair(c, buttBox, (reloadBtn = c->Button()), "Reload model");
    add_pair(c, buttBox, (gridBtn = c->Button()), "Grid mode");
    add_pair(c, buttBox, (radiusCheck = c->CheckBox()), "Show bounding radius");
    add_pair(c, buttBox, (gunsCheck = c->CheckBox()), "Attach guns");
    add_pair(c, buttBox, (c->CheckBox()), "Draw collision mesh");

    box->PackEnd(animBox = c->VBox(5.f), battrs);
    animBox->PackEnd(c->Label("Animation:"));
    animBox->PackEnd(m_animSelector = c->DropDown()->AddOption("None"));
    add_pair(c, animBox, playBtn = c->Button(), "Play/Pause");
    add_pair(c, animBox, revBtn = c->Button(), "Play reverse");
    add_pair(c, animBox, stopBtn = c->Button(), "Stop");

    b1->onClick.connect(sigc::mem_fun(*this, &Viewer::PickAnotherModel));
    reloadBtn->onClick.connect(sigc::bind(sigc::mem_fun(*this, &Viewer::OnReloadModel), reloadBtn));
    gridBtn->onClick.connect(sigc::bind(sigc::mem_fun(*this, &Viewer::OnToggleGrid), gridBtn));
    radiusCheck->onClick.connect(sigc::bind(sigc::mem_fun(*this, &Viewer::OnToggleBoundingRadius), radiusCheck));
    gunsCheck->onClick.connect(sigc::bind(sigc::mem_fun(*this, &Viewer::OnToggleGuns), gunsCheck));

    playBtn->onClick.connect(sigc::bind(sigc::mem_fun(*this, &Viewer::OnAnimPlay), playBtn, false));
    revBtn->onClick.connect(sigc::bind(sigc::mem_fun(*this, &Viewer::OnAnimPlay), revBtn, true));
    stopBtn->onClick.connect(sigc::bind(sigc::mem_fun(*this, &Viewer::OnAnimStop), stopBtn));

    UI::DropDown *ddown;
    buttBox->PackEnd(c->Label("Pattern:"));
    buttBox->PackEnd((m_patternSelector = c->DropDown()->AddOption("Default")));
    buttBox->PackEnd(c->Label("Lights:"));
    buttBox->PackEnd((ddown = c->DropDown()
        ->AddOption("1  Front white")
        ->AddOption("2  Two-point")
        ->AddOption("3  Backlight")
        )
    );

    m_patternSelector->onOptionSelected.connect(sigc::mem_fun(*this, &Viewer::OnPatternChanged));
    ddown->onOptionSelected.connect(sigc::mem_fun(*this, &Viewer::OnLightPresetChanged));

    //3x3 colour sliders
    const Uint32 all = UI::Box::BOX_FILL | UI::Box::BOX_EXPAND;
    const Uint32 expand = UI::Box::BOX_EXPAND;
    const float spacing = 5.f;
    c->AddFloatingWidget(
        c->HBox()->PackEnd( //three columns
            c->VBox()->PackEnd(UI::WidgetSet( //three rows
                c->HBox(spacing)->PackEnd(c->Label("R"))->PackEnd(m_sliders[0] = c->HSlider(), expand),
                c->HBox(spacing)->PackEnd(c->Label("G"))->PackEnd(m_sliders[1] = c->HSlider(), expand),
                c->HBox(spacing)->PackEnd(c->Label("B"))->PackEnd(m_sliders[2] = c->HSlider(), expand)
                )), all
        )->PackEnd(
            c->VBox()->PackEnd(UI::WidgetSet( //three rows
                c->HBox(spacing)->PackEnd(c->Label("R"))->PackEnd(m_sliders[3] = c->HSlider(), expand),
                c->HBox(spacing)->PackEnd(c->Label("G"))->PackEnd(m_sliders[4] = c->HSlider(), expand),
                c->HBox(spacing)->PackEnd(c->Label("B"))->PackEnd(m_sliders[5] = c->HSlider(), expand)
                )), all
        )->PackEnd(
            c->VBox()->PackEnd(UI::WidgetSet( //three rows
                c->HBox(spacing)->PackEnd(c->Label("R"))->PackEnd(m_sliders[6] = c->HSlider(), expand),
                c->HBox(spacing)->PackEnd(c->Label("G"))->PackEnd(m_sliders[7] = c->HSlider(), expand),
                c->HBox(spacing)->PackEnd(c->Label("B"))->PackEnd(m_sliders[8] = c->HSlider(), expand)
                )), all
        )
    , vector2f(g_width-520.f, g_height-100.f), vector2f(500.f, 300.f));

    //connect slider signals, set initial values
    const float values[] = {
        1.f, 0.f, 0.f,
        0.f, 1.f, 0.f,
        0.f, 0.f, 1.f
    };
    for(unsigned int i=0; i<3*3; i++) {
        m_sliders[i]->SetValue(values[i]);
        m_sliders[i]->onValueChanged.connect(sigc::mem_fun(*this, &Viewer::OnModelColorsChanged));
    }

    // Thrust sliders
    UI::Box *thrustSliderBox =
    c->HBox(spacing)->PackEnd(
        // Column 1, Linear thrust sliders
        c->VBox()->PackEnd(
            // Rows X,Y,Z
            UI::WidgetSet(
                c->Label("Linear"),
                c->HBox()->PackEnd(c->Label("X"))->PackEnd(m_tSliders[0] = c->HSlider(), expand),
                c->HBox()->PackEnd(c->Label("Y"))->PackEnd(m_tSliders[1] = c->HSlider(), expand),
                c->HBox()->PackEnd(c->Label("Z"))->PackEnd(m_tSliders[2] = c->HSlider(), expand)
            )
        ), expand
    )->PackEnd(
        //Column 2, Angular thrust sliders
        c->VBox()->PackEnd(
            // Rows X,Y,Z
            UI::WidgetSet(
                c->Label("Angular"),
                c->HBox()->PackEnd(c->Label("Pitch"))->PackEnd(m_tSliders[3] = c->HSlider(), expand),
                c->HBox()->PackEnd(c->Label("Yaw"))->PackEnd(m_tSliders[4] = c->HSlider(), expand),
                c->HBox()->PackEnd(c->Label("Roll"))->PackEnd(m_tSliders[5] = c->HSlider(), expand)
            )
        ), expand
    );
    for(unsigned int i=0; i<2*3; i++) {
        m_tSliders[i]->SetValue(0.5f);
        m_tSliders[i]->onValueChanged.connect(sigc::mem_fun(*this, &Viewer::OnThrustChanged));
    }

	c->AddFloatingWidget(thrustSliderBox, vector2f(0.f, g_height-120.f), vector2f(250.f, 100.f));
    c->Layout();
}

bool Viewer::OnAnimPlay(UI::Widget *w, bool reverse)
{
    Newmodel::Animation::Direction dir = reverse ? Newmodel::Animation::REVERSE : Newmodel::Animation::FORWARD;
    const std::string animname = m_animSelector->GetSelectedOption();
    m_playing = !m_playing;
    if (m_playing) {
        int success = m_model->PlayAnimation(animname, dir);
        if (success)
            AddLog(stringf("Playing animation \"%0\"", animname));
        else {
            AddLog(stringf("Model does not have animation \"%0\"", animname));
            m_playing = false;
        }
    } else {
        AddLog("Animation paused");
    }
    return m_playing;
}

bool Viewer::OnAnimStop(UI::Widget *w)
{
    if (m_playing) AddLog("Animation stopped");
    m_playing = false;
    m_model->StopAnimations();
    return false;
}

bool Viewer::OnReloadModel(UI::Widget *w)
{
    try {
        AddLog("Reloading model...");
        Newmodel::Loader loader(renderer);
        NModel *mo = loader.LoadModel(m_modelName);
        SetModel(mo, m_modelName);
        AddLog("Model loaded");
    } catch (Newmodel::LoadingError &err) {
        PickModel(m_modelName, stringf("Could not load model %0: %1", m_modelName, err.what()));
    }
    return true;
}

void Viewer::AddLog(const std::string &line)
{
    m_logLines.push_back(line);
    if (m_logLines.size() > 8) m_logLines.pop_front();

    std::stringstream ss;
    for(std::list<std::string>::const_iterator it = m_logLines.begin();
        it != m_logLines.end();
        ++it)
    {
        ss << *it << std::endl;
    }
    m_logString = ss.str();
}

void Viewer::ClearModel()
{
    // clear old geometry
    if (m_model) {
        delete m_model;
        m_model = 0;
    }
    if (m_cmesh) {
        delete m_cmesh;
        m_cmesh = 0;
    }
    if (m_geom) {
        m_space->RemoveGeom(m_geom);
        delete m_geom;
        m_geom = 0;
    }
}

//Draw grid and axes
void Viewer::DrawGrid(matrix4x4f& trans, double radius)
{
    const float dist = abs(m_campos.z);

    const float max = std::min(powf(10, ceilf(log10f(dist))), ceilf(radius/m_options.gridInterval)*m_options.gridInterval);

    static std::vector<vector3f> points;
	points.clear();

    for (float x = -max; x <= max; x += m_options.gridInterval) {
        points.push_back(vector3f(x,0,-max));
        points.push_back(vector3f(x,0,max));
        points.push_back(vector3f(0,x,-max));
        points.push_back(vector3f(0,x,max));

        points.push_back(vector3f(x,-max,0));
        points.push_back(vector3f(x,max,0));
        points.push_back(vector3f(0,-max,x));
        points.push_back(vector3f(0,max,x));

        points.push_back(vector3f(-max,x,0));
        points.push_back(vector3f(max,x,0));
        points.push_back(vector3f(-max,0,x));
        points.push_back(vector3f(max,0,x));
    }

    renderer->SetTransform(trans);
    renderer->DrawLines(points.size(), &points[0], Color(0.0f,0.2f,0.0f,1.0f));
}

void Viewer::DrawLog()
{
    Gui::Screen::EnterOrtho();
    Gui::Screen::PushFont("ConsoleFont");
    Gui::Screen::RenderString(m_logString, 300, 20);
    Gui::Screen::PopFont();
    Gui::Screen::LeaveOrtho();
}

void Viewer::Screenshot()
{
    char buf[256];
    const time_t t = time(0);
    struct tm *_tm = localtime(&t);
    strftime(buf, sizeof(buf), "modelviewer-%Y%m%d-%H%M%S.png", _tm);
    Screendump(buf, g_width, g_height);
    AddLog("Screenshot saved");
}

void Viewer::UpdateAnimList()
{
    m_animSelector->Clear();
	const std::vector<Newmodel::Animation*> &anims = m_model->GetAnimations();
	for(unsigned int i=0; i<anims.size(); i++) {
		m_animSelector->AddOption(anims[i]->GetName());
	}
    m_animSelector->Layout();
}

void Viewer::UpdateLights()
{
    Light lights[2];

    switch(m_options.lightPreset) {
    case 0:
        //Front white
        lights[0] = Light(Light::LIGHT_DIRECTIONAL, az_el_to_dir(90,0), Color(1.0f, 1.0f, 1.0f), Color(0.f), Color(1.f));
        lights[1] = Light(Light::LIGHT_DIRECTIONAL, az_el_to_dir(0,-90), Color(0.05, 0.05f, 0.1f), Color(0.f), Color(1.f));
        break;
    case 1:
        //Two-point
        lights[0] = Light(Light::LIGHT_DIRECTIONAL, az_el_to_dir(120,0), Color(0.9f, 0.8f, 0.8f), Color(0.f), Color(1.f));
        lights[1] = Light(Light::LIGHT_DIRECTIONAL, az_el_to_dir(-30,-90), Color(0.7f, 0.5f, 0.0f), Color(0.f), Color(1.f));
        break;
    case 2:
        //Backlight
        lights[0] = Light(Light::LIGHT_DIRECTIONAL, az_el_to_dir(-75,20), Color(1.f), Color(0.f), Color(1.f));
        lights[1] = Light(Light::LIGHT_DIRECTIONAL, az_el_to_dir(0,-90), Color(0.05, 0.05f, 0.1f), Color(0.f), Color(1.f));
        break;
    };

    renderer->SetLights(2, &lights[0]);
}

void Viewer::UpdatePatternList()
{
    m_patternSelector->Clear();
	const Newmodel::PatternContainer &pats = m_model->GetPatterns();
	for(unsigned int i=0; i<pats.size(); i++) {
		m_patternSelector->AddOption(pats[i].name);
	}
	m_ui->Layout();
}

void Viewer::UpdateTime()
{
    static double lastTime = 0;
    const double newtime = 0.001 * SDL_GetTicks();
    if (m_playing) {
        m_currentTime += newtime - lastTime;
    }
    lastTime = newtime;
}

void Viewer::OnLightPresetChanged(unsigned int index, const std::string &)
{
    m_options.lightPreset = std::min<unsigned int>(index, 2);
}

static Color4ub GetColor(UI::Slider *r, UI::Slider *g, UI::Slider *b)
{
    return Color4ub(r->GetValue() * 255.f, g->GetValue() * 255.f, b->GetValue() * 255.f);
}

void Viewer::OnModelColorsChanged(float)
{
	std::vector<Color4ub> colors;
	colors.push_back(GetColor(m_sliders[0], m_sliders[1], m_sliders[2]));
	colors.push_back(GetColor(m_sliders[3], m_sliders[4], m_sliders[5]));
	colors.push_back(GetColor(m_sliders[6], m_sliders[7], m_sliders[8]));
	m_model->SetColors(renderer, colors);
}

void Viewer::OnPatternChanged(unsigned int index, const std::string &)
{
	assert(index < m_model->GetPatterns().size());
	m_model->SetPattern(index);
}

inline float GetThrust(const UI::Slider *s)
{
    return 1.f - (2.f * s->GetValue());
}

void Viewer::OnThrustChanged(float)
{
    m_modelParams.linthrust[0] = GetThrust(m_tSliders[0]);
    m_modelParams.linthrust[1] = GetThrust(m_tSliders[1]);
    m_modelParams.linthrust[2] = GetThrust(m_tSliders[2]);

    // angthrusts are negated in ship.cpp for some reason
    m_modelParams.angthrust[0] = -GetThrust(m_tSliders[3]);
    m_modelParams.angthrust[1] = -GetThrust(m_tSliders[4]);
    m_modelParams.angthrust[2] = -GetThrust(m_tSliders[5]);
}

void Viewer::ResetSliders()
{
    for (unsigned int i=0; i<6; i++) {
        m_tSliders[i]->SetValue(0.5f);
    }
}

bool Viewer::OnToggleGrid(UI::Widget *w)
{
    if (!m_options.showGrid) {
        m_options.showGrid = true;
        m_options.gridInterval = 1.0f;
    }
    else {
        m_options.gridInterval = powf(10, ceilf(log10f(m_options.gridInterval))+1);
        if (m_options.gridInterval >= 10000.0f) {
            m_options.showGrid = false;
            m_options.gridInterval = 0.0f;
        }
    }
    AddLog(m_options.showGrid
        ? stringf("Grid: %0{d}", int(m_options.gridInterval))
        : "Grid: off");
    return m_options.showGrid;
}

bool Viewer::OnToggleGuns(UI::CheckBox *w)
{
    if (!m_gunModel.Valid() || !m_gunModelNode.Valid()) {
        AddLog("test_gun.model not available");
        return false;
    }

    m_options.attachGuns = !m_options.attachGuns;
    Newmodel::Group *tagL = m_model->FindTagByName("tag_gun_left");
    Newmodel::Group *tagR = m_model->FindTagByName("tag_gun_right");
    if (!tagL || !tagR) {
        AddLog("Missing tags gun_left and gun_right in model");
        return false;
    }
    if (m_options.attachGuns) {
        tagL->AddChild(m_gunModelNode.Get());
        tagR->AddChild(m_gunModelNode.Get());
    } else { //detach
        //we know there's nothing else
        tagL->RemoveChildAt(0);
        tagR->RemoveChildAt(0);
    }
    return true;
}

bool Viewer::OnToggleBoundingRadius(UI::Widget *w)
{
    m_options.showBoundingRadius = !m_options.showBoundingRadius;
    return m_options.showBoundingRadius;
}

void Viewer::ResetCamera()
{
    m_campos = vector3f(0.0f, 0.0f, m_model->GetDrawClipRadius() * 1.5f);
    m_camOrient = matrix4x4f::Identity();
    m_modelRot = matrix4x4f::Identity();
}

void Viewer::SetModel(NModel *model, const std::string &name)
{
    ClearModel();

    m_modelName = name;
    m_model = model;
    m_cmesh = m_model->CreateCollisionMesh(0);
    m_geom = new Geom(m_cmesh->GetGeomTree());
    m_space->AddGeom(m_geom);
    ResetCamera();

    UpdateAnimList();
    UpdatePatternList();
    OnModelColorsChanged();
    m_model->SetDecalTexture(m_decalTexture, 0);
	m_model->SetLabel("Pioneer");
}

void Viewer::TryModel(const SDL_keysym *sym, Gui::TextEntry *entry, Gui::Label *errormsg)
{
    if (sym->sym == SDLK_RETURN) {
        const std::string &name = entry->GetText();
        Newmodel::Loader load(renderer);
        try {
            NModel *mo = load.LoadModel(name);
            SetModel(mo, name);
        } catch (Newmodel::LoadingError &err) {
            errormsg->SetText(stringf("Could not load model %0: %1", name, err.what()));
        }
    }
}

void Viewer::PickModel(const std::string &initial_name, const std::string &initial_errormsg)
{
    //delete old
    ClearModel();

    Gui::Fixed *f = new Gui::Fixed();
    f->SetSizeRequest(Gui::Screen::GetWidth()*0.5f, Gui::Screen::GetHeight()*0.5);
    Gui::Screen::AddBaseWidget(f, Gui::Screen::GetWidth()*0.25f, Gui::Screen::GetHeight()*0.25f);

    f->Add(new Gui::Label("Enter the name of the model you want to view (esc to quit app):"), 0, 0);

    Gui::Label *errormsg = new Gui::Label(initial_errormsg);
    f->Add(errormsg, 0, 64);

    Gui::TextEntry *entry = new Gui::TextEntry();
    entry->SetText(initial_name);
    entry->onKeyPress.connect(sigc::bind(sigc::mem_fun(this, &Viewer::TryModel), entry, errormsg));
    entry->Show();
    f->Add(entry, 0, 32);

    while (!m_model) {
        this->Hide();
        f->ShowAll();
        PollEvents();
        renderer->ClearScreen();
        Gui::Draw();
		DrawLog();
        renderer->SwapBuffers();
    }
    Gui::Screen::RemoveBaseWidget(f);
    delete f;
    this->Show();
}

void Viewer::PickModel()
{
    PickModel("", "");
}

bool Viewer::PickAnotherModel()
{
    PickModel();
    return true;
}

#if 0
static void render_coll_mesh(const LmrCollMesh *m)
{
    Material mat;
    mat.unlit = true;
    mat.diffuse = Color(1.f, 0.f, 1.f);
    glDepthRange(0.0,0.9f);
    VertexArray va(ATTRIB_POSITION, m->ni * 3);
    for (int i=0; i<m->ni; i+=3) {
        va.Add(static_cast<vector3f>(&m->pVertex[3*m->pIndex[i]]));
        va.Add(static_cast<vector3f>(&m->pVertex[3*m->pIndex[i+1]]));
        va.Add(static_cast<vector3f>(&m->pVertex[3*m->pIndex[i+2]]));
    }
    renderer->DrawTriangles(&va, &mat);

    mat.diffuse = Color(1.f);
    glPolygonOffset(-1.f, -1.f);
    renderer->SetWireFrameMode(true);
    renderer->DrawTriangles(&va, &mat);
    renderer->SetWireFrameMode(false);
    glPolygonOffset(0.f, 0.f);
}
#endif

void Viewer::MainLoop()
{
    Uint32 lastTurd = SDL_GetTicks();

    //Uint32 t = SDL_GetTicks();
    int numFrames = 0;
    //int fps = 0
    //int numTris = 0;
    //Uint32 lastFpsReadout = SDL_GetTicks();
    //m_campos = vector3f(0.0f, 0.0f, m_cmesh->GetBoundingRadius());
    m_modelParams.scrWidth = g_width;

    for (;;) {
        PollEvents();
        UpdateTime();

        if (g_keyState[SDLK_LSHIFT] || g_keyState[SDLK_RSHIFT]) {
            if (g_keyState[SDLK_UP]) m_camOrient = m_camOrient * matrix4x4f::RotateXMatrix(m_frameTime);
            if (g_keyState[SDLK_DOWN]) m_camOrient = m_camOrient * matrix4x4f::RotateXMatrix(-m_frameTime);
            if (g_keyState[SDLK_LEFT]) m_camOrient = m_camOrient * matrix4x4f::RotateYMatrix(-m_frameTime);
            if (g_keyState[SDLK_RIGHT]) m_camOrient = m_camOrient * matrix4x4f::RotateYMatrix(m_frameTime);
            if (g_mouseButton[3]) {
                float rx = 0.01f*g_mouseMotion[1];
                float ry = 0.01f*g_mouseMotion[0];
                m_camOrient = m_camOrient * matrix4x4f::RotateXMatrix(rx);
                m_camOrient = m_camOrient * matrix4x4f::RotateYMatrix(ry);
                if (g_mouseButton[1]) {
                    m_campos = m_campos - m_camOrient * vector3f(0.0f,0.0f,1.0f) * 0.01 *
                        m_model->GetDrawClipRadius();
                }
            }
        } else {
            if (g_keyState[SDLK_UP]) m_modelRot = m_modelRot * matrix4x4f::RotateXMatrix(m_frameTime);
            if (g_keyState[SDLK_DOWN]) m_modelRot = m_modelRot * matrix4x4f::RotateXMatrix(-m_frameTime);
            if (g_keyState[SDLK_LEFT]) m_modelRot = m_modelRot * matrix4x4f::RotateYMatrix(-m_frameTime);
            if (g_keyState[SDLK_RIGHT]) m_modelRot = m_modelRot * matrix4x4f::RotateYMatrix(m_frameTime);
            if (g_mouseButton[3]) {
                float rx = 0.01f*g_mouseMotion[1];
                float ry = 0.01f*g_mouseMotion[0];
                m_modelRot = m_modelRot * matrix4x4f::RotateXMatrix(rx);
                m_modelRot = m_modelRot * matrix4x4f::RotateYMatrix(ry);
            }
        }
        float rate = 5.f * m_frameTime;
        if (g_keyState[SDLK_LSHIFT]) rate = 20.f * m_frameTime;
        if (g_keyState[SDLK_EQUALS] || g_keyState[SDLK_KP_PLUS]) m_campos = m_campos - m_camOrient * vector3f(0.0f,0.0f,1.f) * rate;
        if (g_keyState[SDLK_MINUS] || g_keyState[SDLK_KP_MINUS]) m_campos = m_campos + m_camOrient * vector3f(0.0f,0.0f,1.f) * rate;
        if (g_keyState[SDLK_PAGEUP]) m_campos = m_campos - m_camOrient * vector3f(0.0f,0.0f,0.5f);
        if (g_keyState[SDLK_PAGEDOWN]) m_campos = m_campos + m_camOrient * vector3f(0.0f,0.0f,0.5f);

        float znear;
        float zfar;
        renderer->GetNearFarRange(znear, zfar);
        renderer->SetPerspectiveProjection(85, g_width/float(g_height), znear, zfar);
        renderer->SetTransform(matrix4x4f::Identity());
        renderer->ClearScreen();
        renderer->SetDepthTest(true);
        UpdateLights();
        m_model->UpdateAnimations(m_currentTime);

		glPushAttrib(GL_ALL_ATTRIB_BITS);
		matrix4x4f m = m_camOrient.InverseOf() * matrix4x4f::Translation(-m_campos) * m_modelRot.InverseOf();
		m_model->Render(renderer, m, &m_modelParams);
		glPopAttrib();

        if (m_options.showBoundingRadius) {
            matrix4x4f mo = m_camOrient.InverseOf() * matrix4x4f::Translation(-m_campos);// * modelRot.InverseOf();
            VisualizeBoundingRadius(mo, m_model->GetDrawClipRadius());
        }

        if (m_options.showGrid) {
            const float rad = m_model->GetDrawClipRadius();
            DrawGrid(m, rad);
            const int numAxVerts = 6;
            const vector3f vts[numAxVerts] = {
                //X
                vector3f(0.f, 0.f, 0.f),
                vector3f(rad, 0.f, 0.f),

                //Y
                vector3f(0.f, 0.f, 0.f),
                vector3f(0.f, rad, 0.f),

                //Z
                vector3f(0.f, 0.f, 0.f),
                vector3f(0.f, 0.f, rad),
            };
            const Color col[numAxVerts] = {
                Color(1.f, 0.f, 0.f),
                Color(1.f, 0.f, 0.f),

                Color(0.f, 0.f, 1.f),
                Color(0.f, 0.f, 1.f),

                Color(0.f, 1.f, 0.f),
                Color(0.f, 1.f, 0.f)
            };
            renderer->SetDepthTest(false);
            renderer->DrawLines(numAxVerts, &vts[0], &col[0]);
            renderer->SetDepthTest(true);
        }

        {
#if 0
            char buf[256];
            Aabb aabb = m_cmesh->GetAabb();
            snprintf(buf, sizeof(buf), "%d triangles, %d fps, %.3fm tris/sec\ncollision mesh size: %.1fx%.1fx%.1f (radius %.1f)\nClipping radius %.1f\nGrid interval: %d metres",
                    (g_renderType == 0 ?
                        LmrModelGetStatsTris() - beforeDrawTriStats :
                        m_cmesh->m_numTris),
                    fps,
                    numTris/1000000.0f,
                    aabb.max.x-aabb.min.x,
                    aabb.max.y-aabb.min.y,
                    aabb.max.z-aabb.min.z,
                    aabb.GetBoundingRadius(),
                    m_model->GetDrawClipRadius(),
                    int(gridInterval));
            m_trisReadout->SetText(buf);
#endif
        }

        //Gui::Draw();
        m_ui->Update();
        if (!m_screenshotQueued) {
            renderer->SetDepthTest(false);
            renderer->SetOrthographicProjection(0, g_width, g_height, 0, -1, 1);
            renderer->SetTransform(matrix4x4f::Identity());
            m_ui->Draw();
            DrawLog();
        }
        renderer->SwapBuffers();
        if (m_screenshotQueued) {
            m_screenshotQueued = false;
            Screenshot();
        }
        numFrames++;
        m_frameTime = (SDL_GetTicks() - lastTurd) * 0.001f;
        lastTurd = SDL_GetTicks();
#if 0
        if (SDL_GetTicks() - lastFpsReadout > 1000) {
            numTris = LmrModelGetStatsTris();
            fps = numFrames;
            numFrames = 0;
            lastFpsReadout = SDL_GetTicks();
            LmrModelClearStatsTris();
        }
#endif
    }
}

void Viewer::PollEvents()
{
    SDL_Event event;

    g_mouseMotion[0] = g_mouseMotion[1] = 0;
    while (SDL_PollEvent(&event)) {
        Gui::HandleSDLEvent(&event);
        m_ui->DispatchSDLEvent(event);
        switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    if (m_model) {
                        PickModel();
                    } else {
                        SDL_Quit();
                        exit(0);
                    }
                }
                if (event.key.keysym.sym == SDLK_F11) SDL_WM_ToggleFullScreen(g_screen);
                if (event.key.keysym.sym == SDLK_PRINT) m_screenshotQueued = true;
                if (event.key.keysym.sym == SDLK_SPACE) {
                    ResetCamera();
                    ResetSliders();
                }
                g_keyState[event.key.keysym.sym] = 1;
                break;
            case SDL_KEYUP:
                g_keyState[event.key.keysym.sym] = 0;
                break;
            case SDL_MOUSEBUTTONDOWN:
                set_mouse_button(event.button.button, 1);
                break;
            case SDL_MOUSEBUTTONUP:
                set_mouse_button(event.button.button, 0);
                break;
            case SDL_MOUSEMOTION:
                g_mouseMotion[0] += event.motion.xrel;
                g_mouseMotion[1] += event.motion.yrel;
                break;
            case SDL_QUIT:
                SDL_Quit();
                exit(0);
                break;
        }
    }
}

void Viewer::VisualizeBoundingRadius(matrix4x4f& trans, double radius)
{
    renderer->SetTransform(trans);
    Graphics::Drawables::Circle circ(radius, Color(0.f, 0.f, 1.f, 1.f));
    circ.Draw(renderer);
}

int main(int argc, char **argv)
{
    if ((argc<=1) || (0==strcmp(argv[1],"--help"))) {
        printf("Usage:\nluamodelviewer <width> <height> <model name>\n");
    }
    if (argc >= 3) {
        g_width = atoi(argv[1]);
        g_height = atoi(argv[2]);
    } else {
        g_width = 800;
        g_height = 600;
    }

    FileSystem::Init();
    FileSystem::rawFileSystem.MakeDirectory(FileSystem::GetUserDir());
    ModManager::Init();

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		OS::Error("SDL initialization failed: %s\n", SDL_GetError());
	}

	Graphics::Settings videoSettings = {};
	videoSettings.width = g_width;
	videoSettings.height = g_height;
	videoSettings.shaders = true;

	renderer = Graphics::Init(videoSettings);

	Gui::Init(renderer, g_width, g_height, g_width, g_height);

	ScopedPtr<Viewer> viewer(new Viewer(g_width, g_height));
    if (argc >= 4) {
        try {
            const std::string &name(argv[3]);
            Newmodel::Loader loader(renderer);
            NModel *mo = loader.LoadModel(name);
            viewer->SetModel(mo, name);
        } catch (Newmodel::LoadingError &err) {
            viewer->PickModel(argv[3], stringf("Could not load model %0: %1", argv[3], err.what()));
        }
    } else {
        viewer->PickModel();
    }

    viewer->MainLoop();
	//XXX looks like this is never reached
    FileSystem::Uninit();
    delete renderer;
    return 0;
}
