#include "libs.h"
#include "EquipType.h"
#include "FileSystem.h"
#include "LmrModel.h"
#include "ModManager.h"
#include "OS.h"
#include "Ship.h" // for the flight state and ship animation enums
#include "ShipType.h"
#include "SpaceStation.h" // for the space station animation enums
#include "collider/collider.h"
#include "graphics/Drawables.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "gui/Gui.h"

using namespace Graphics;

static Renderer *renderer;

enum ModelCategory {
	MODEL_SHIP,
	MODEL_SPACESTATION
};

static const char *ANIMATION_NAMESPACES[] = {
	"ShipAnimation",
	"SpaceStationAnimation",
};

static const int LMR_ARG_MAX = 40;

static int g_width, g_height;
static int g_mouseMotion[2];
static char g_keyState[SDLK_LAST];
static const int MAX_MOUSE_BTN_IDX = SDL_BUTTON_WHEELDOWN + 1;
static int g_mouseButton[MAX_MOUSE_BTN_IDX];	// inc to 6 as mouseScroll is index 5
static float g_zbias;
static bool g_doBenchmark = false;

float gridInterval = 0.0f;

static bool setMouseButton(const Uint8 idx, const int value)
{
	if( idx < MAX_MOUSE_BTN_IDX ) {
		g_mouseButton[idx] = value;
		return true;
	}
	return false;
}

class Viewer;
static Viewer *g_viewer;

static void PollEvents();

static int g_wheelMoveDir = -1;
static int g_renderType = 0;
static float g_frameTime;
static EquipSet g_equipment;
static LmrObjParams g_params = {
	0, // animation namespace
	0.0, // time
	{}, // animation stages
	{}, // animation positions
	"PIONEER", // label
	&g_equipment, // equipment
	Ship::FLYING, // flightState

	{ 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, 0.0f },

	{	// pColor[3]
	{ { .2f, .2f, .5f, 1 }, { 1, 1, 1 }, { 0, 0, 0 }, 100.0 },
	{ { 0.5f, 0.5f, 0.5f, 1 }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
	{ { 0.8f, 0.8f, 0.8f, 1 }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },
};

class Viewer: public Gui::Fixed {
public:
	Gui::Adjustment *m_linthrust[3];
	Gui::Adjustment *m_angthrust[3];
	Gui::Adjustment *m_anim[LMR_ARG_MAX];
	Gui::TextEntry *m_animEntry[LMR_ARG_MAX];
	Gui::Label *m_trisReadout;
	LmrCollMesh *m_cmesh;
	LmrModel *m_model;
	CollisionSpace *m_space;
	Geom *m_geom;
	ModelCategory m_modelCategory;

	void SetModel(LmrModel *);

	void PickModel(const std::string &initial_name, const std::string &initial_errormsg);

	void PickModel() {
		PickModel("", "");
	}

	float GetAnimValue(int i) {
		std::string val = m_animEntry[i]->GetText();
		return float(atof(val.c_str()));
	}

	Viewer(): Gui::Fixed(float(g_width), float(g_height)) {
		m_model = 0;
		m_cmesh = 0;
		m_geom = 0;
		m_space = new CollisionSpace();
		m_showBoundingRadius = false;
		m_showGrid = false;
		Gui::Screen::AddBaseWidget(this, 0, 0);
		SetTransparency(true);

		m_trisReadout = new Gui::Label("");
		Add(m_trisReadout, 500, 0);
		{
			Gui::Button *b = new Gui::SolidButton();
			b->SetShortcut(SDLK_c, KMOD_NONE);
			b->onClick.connect(sigc::mem_fun(*this, &Viewer::OnClickChangeView));
			Add(b, 10, 10);
			Add(new Gui::Label("[c] Change view (normal, collision mesh"), 30, 10);
		}
		{
			Gui::Button *b = new Gui::SolidButton();
			b->SetShortcut(SDLK_r, KMOD_NONE);
			b->onClick.connect(sigc::mem_fun(*this, &Viewer::OnResetAdjustments));
			Add(b, 10, 30);
			Add(new Gui::Label("[r] Reset thruster and anim sliders"), 30, 30);
		}
		{
			Gui::Button *b = new Gui::SolidButton();
			b->SetShortcut(SDLK_m, KMOD_NONE);
			b->onClick.connect(sigc::mem_fun(*this, &Viewer::OnClickRebuildCollMesh));
			Add(b, 10, 50);
			Add(new Gui::Label("[m] Rebuild collision mesh"), 30, 50);
		}
		{
			Gui::Button *b = new Gui::SolidButton();
			b->SetShortcut(SDLK_p, KMOD_NONE);
			b->onClick.connect(sigc::mem_fun(*this, &Viewer::OnClickToggleBenchmark));
			Add(b, 10, 70);
			Add(new Gui::Label("[p] Toggle performance test (renders models 1000 times per frame)"), 30, 70);
		}
		{
			Gui::Button *b = new Gui::SolidButton();
			b->SetShortcut(SDLK_b, KMOD_LSHIFT);
			b->onClick.connect(sigc::mem_fun(*this, &Viewer::OnToggleBoundingRadius));
			Add(b, 10, 90);
			Add(new Gui::Label("[shift-b] Visualize bounding radius"), 30, 90);
		}
		{
			Gui::Button *b = new Gui::SolidButton();
			b->SetShortcut(SDLK_g, KMOD_LSHIFT);
			b->onClick.connect(sigc::mem_fun(*this, &Viewer::OnToggleGrid));
			Add(b, 10, 110);
			Add(new Gui::Label("[shift-g] Toggle grid"), 30, 110);
		}
#if 0
		{
			Gui::Button *b = new Gui::SolidButton();
			b->SetShortcut(SDLK_g, KMOD_NONE);
			b->onClick.connect(sigc::mem_fun(*this, &Viewer::OnToggleGearState));
			Add(b, 10, 30);
			Add(new Gui::Label("[g] Toggle gear state"), 30, 30);
		}
#endif /* 0 */
		{
			Add(new Gui::Label("Linear thrust"), 0, Gui::Screen::GetHeight()-140.0f);
			for (int i=0; i<3; i++) {
				m_linthrust[i] = new Gui::Adjustment();
				m_linthrust[i]->SetValue(0.5);
				Gui::VScrollBar *v = new Gui::VScrollBar();
				v->SetAdjustment(m_linthrust[i]);
				Add(v, float(i*25), Gui::Screen::GetHeight()-120.0f);
			}

			Add(new Gui::Label("Angular thrust"), 100, Gui::Screen::GetHeight()-140.0f);
			for (int i=0; i<3; i++) {
				m_angthrust[i] = new Gui::Adjustment();
				m_angthrust[i]->SetValue(0.5);
				Gui::VScrollBar *v = new Gui::VScrollBar();
				v->SetAdjustment(m_angthrust[i]);
				Add(v, float(100 + i*25), Gui::Screen::GetHeight()-120.0f);
			}

			Add(new Gui::Label("Animations (0 gear, 1-4 are time - ignore them comrade)"),
					200, Gui::Screen::GetHeight()-140.0f);

			for (int i=0; i<LMR_ARG_MAX; i++) {
				m_anim[i] = 0;

				float x = float(200+i*25);
				float w = 32.0f;
				if (x >= Gui::Screen::GetWidth()-w)
					break;

				Gui::Fixed *box = new Gui::Fixed(w, 120.0f);
				Add(box, x, Gui::Screen::GetHeight()-120.0f);

				m_anim[i] = new Gui::Adjustment();
				m_anim[i]->SetValue(0);
				Gui::VScrollBar *v = new Gui::VScrollBar();
				v->SetAdjustment(m_anim[i]);
				box->Add(v, 0, 42.0f);
				char buf[32];
				snprintf(buf, sizeof(buf), "%d", i);
				box->Add(new Gui::Label(buf), 0, 0);

				m_animEntry[i] = new Gui::TextEntry();
				box->Add(m_animEntry[i], 0, 16.0f);
				m_anim[i]->onValueChanged.connect(sigc::bind(sigc::mem_fun(this, &Viewer::OnAnimChange), m_anim[i], m_animEntry[i]));
				OnAnimChange(m_anim[i], m_animEntry[i]);
			}
		}

		ShowAll();
		Show();
	}

	void OnAnimChange(Gui::Adjustment *a, Gui::TextEntry *e) {
		char buf[128];
		snprintf(buf, sizeof(buf), "%.2f", a->GetValue());
		e->SetText(buf);
	}

	void OnResetAdjustments() {
		for (int i=0; i<LMR_ARG_MAX; i++) {
			if(m_anim[i])
				m_anim[i]->SetValue(0);
		}
		for (int i=0; i<3; i++) {
			m_linthrust[i]->SetValue(0.5);
			m_angthrust[i]->SetValue(0.5);
		}
	}
	void OnClickToggleBenchmark() {
		g_doBenchmark = !g_doBenchmark;
	}
	void OnClickRebuildCollMesh() {
		m_space->RemoveGeom(m_geom);
		delete m_geom;
		delete m_cmesh;

		m_cmesh = new LmrCollMesh(m_model, &g_params);
		m_geom = new Geom(m_cmesh->geomTree);
		m_space->AddGeom(m_geom);
	}

	void OnToggleGearState() {
		if (g_wheelMoveDir == -1) g_wheelMoveDir = +1;
		else g_wheelMoveDir = -1;
	}

	void OnClickChangeView() {
		g_renderType++;
		// XXX raytraced view disabled
		if (g_renderType > 1) g_renderType = 0;
	}

	void OnToggleBoundingRadius() {
		m_showBoundingRadius = !m_showBoundingRadius;
	}
	void OnToggleGrid() {
		if (!m_showGrid) {
			m_showGrid = true;
			gridInterval = 1.0f;
		}
		else {
			gridInterval = powf(10, ceilf(log10f(gridInterval))+1);
			if (gridInterval >= 10000.0f) {
				m_showGrid = false;
				gridInterval = 0.0f;
			}
		}
	}

	void MainLoop() __attribute((noreturn));
	void SetSbreParams();
private:
	void TryModel(const SDL_keysym *sym, Gui::TextEntry *entry, Gui::Label *errormsg);
	void VisualizeBoundingRadius(matrix4x4f& trans, double radius);
	void DrawGrid(matrix4x4f& trans, double radius);
	bool m_showBoundingRadius;
	bool m_showGrid;
};

void Viewer::SetModel(LmrModel *model)
{
	m_model = model;
	// clear old geometry
	if (m_cmesh) delete m_cmesh;
	if (m_geom) {
		m_space->RemoveGeom(m_geom);
		delete m_geom;
	}

	// set up model parameters
	// inefficient (looks up and searches tags table separately for each tag)
	bool has_station = m_model->HasTag("surface_station") || m_model->HasTag("orbital_station");
    if (!has_station) {
		m_modelCategory = MODEL_SHIP;
		const std::string name = model->GetName();
		std::map<std::string,ShipType>::const_iterator it = ShipType::types.begin();
		while (it != ShipType::types.end()) {
			if (it->second.lmrModelName == name)
				break;
			else
				++it;
		}
		if (it != ShipType::types.end())
			g_equipment.InitSlotSizes(it->first);
		else
			g_equipment.InitSlotSizes(ShipType::EAGLE_LRF);
		g_params.equipment = &g_equipment;
	} else {
		m_modelCategory = MODEL_SPACESTATION;
		g_params.equipment = 0;
	}

	g_params.animationNamespace = ANIMATION_NAMESPACES[m_modelCategory];

	// construct geometry
	m_cmesh = new LmrCollMesh(m_model, &g_params);
	m_geom = new Geom(m_cmesh->geomTree);
	m_space->AddGeom(m_geom);
}

void Viewer::TryModel(const SDL_keysym *sym, Gui::TextEntry *entry, Gui::Label *errormsg)
{
	if (sym->sym == SDLK_RETURN) {
		LmrModel *m = 0;
		try {
			m = LmrLookupModelByName(entry->GetText().c_str());
		} catch (LmrModelNotFoundException) {
			errormsg->SetText("Could not find model: " + entry->GetText());
		}
		if (m) SetModel(m);
	}
}

void Viewer::PickModel(const std::string &initial_name, const std::string &initial_errormsg)
{
	Gui::Fixed *f = new Gui::Fixed();
	f->SetSizeRequest(Gui::Screen::GetWidth()*0.5f, Gui::Screen::GetHeight()*0.5);
	Gui::Screen::AddBaseWidget(f, Gui::Screen::GetWidth()*0.25f, Gui::Screen::GetHeight()*0.25f);

	f->Add(new Gui::Label("Enter the name of the model you want to view:"), 0, 0);

	Gui::Label *errormsg = new Gui::Label(initial_errormsg);
	f->Add(errormsg, 0, 64);

	Gui::TextEntry *entry = new Gui::TextEntry();
	entry->SetText(initial_name);
	entry->onKeyPress.connect(sigc::bind(sigc::mem_fun(this, &Viewer::TryModel), entry, errormsg));
	entry->Show();
	f->Add(entry, 0, 32);

	m_model = 0;

	while (!m_model) {
		this->Hide();
		f->ShowAll();
		PollEvents();
		renderer->ClearScreen();
		Gui::Draw();
		renderer->SwapBuffers();
	}
	Gui::Screen::RemoveBaseWidget(f);
	delete f;
	this->Show();
}

void Viewer::SetSbreParams()
{
	float gameTime = SDL_GetTicks() * 0.001f;

#if 0
	-- get_arg() indices
	ARG_ALL_TIME_SECONDS = 1
	ARG_ALL_TIME_MINUTES = 2
	ARG_ALL_TIME_HOURS = 3
	ARG_ALL_TIME_DAYS = 4

	ARG_STATION_BAY1_STAGE = 6
	ARG_STATION_BAY1_POS   = 10

	ARG_SHIP_WHEEL_STATE = 0
	ARG_SHIP_EQUIP_SCOOP = 5
	ARG_SHIP_EQUIP_ENGINE = 6
	ARG_SHIP_EQUIP_ECM = 7
	ARG_SHIP_EQUIP_SCANNER = 8
	ARG_SHIP_EQUIP_ATMOSHIELD = 9
	ARG_SHIP_EQUIP_LASER0 = 10
	ARG_SHIP_EQUIP_LASER1 = 11
	ARG_SHIP_EQUIP_MISSILE0 = 12
	ARG_SHIP_EQUIP_MISSILE1 = 13
	ARG_SHIP_EQUIP_MISSILE2 = 14
	ARG_SHIP_EQUIP_MISSILE3 = 15
	ARG_SHIP_EQUIP_MISSILE4 = 16
	ARG_SHIP_EQUIP_MISSILE5 = 17
	ARG_SHIP_EQUIP_MISSILE6 = 18
	ARG_SHIP_EQUIP_MISSILE7 = 19
	ARG_SHIP_FLIGHT_STATE = 20

	-- get_arg_string() indices
	ARGSTR_ALL_LABEL = 0
	ARGSTR_STATION_ADMODEL1 = 4
	ARGSTR_STATION_ADMODEL2 = 5
	ARGSTR_STATION_ADMODEL3 = 6
	ARGSTR_STATION_ADMODEL4 = 7
#endif

	if (m_modelCategory == MODEL_SHIP) {
		g_params.animValues[Ship::ANIM_WHEEL_STATE] = GetAnimValue(0);

		g_equipment.Set(Equip::SLOT_FUELSCOOP,  0, (GetAnimValue( 5) > 0.5) ? Equip::FUEL_SCOOP            : Equip::NONE);
		g_equipment.Set(Equip::SLOT_ENGINE,     0, (GetAnimValue( 6) > 0.5) ? Equip::DRIVE_CLASS4          : Equip::NONE);
		g_equipment.Set(Equip::SLOT_ECM,        0, (GetAnimValue( 7) > 0.5) ? Equip::ECM_ADVANCED          : Equip::NONE);
		g_equipment.Set(Equip::SLOT_SCANNER,    0, (GetAnimValue( 8) > 0.5) ? Equip::SCANNER               : Equip::NONE);
		g_equipment.Set(Equip::SLOT_ATMOSHIELD, 0, (GetAnimValue( 9) > 0.5) ? Equip::ATMOSPHERIC_SHIELDING : Equip::NONE);
		g_equipment.Set(Equip::SLOT_LASER,      0, (GetAnimValue(10) > 0.5) ? Equip::PULSECANNON_4MW       : Equip::NONE);
		g_equipment.Set(Equip::SLOT_LASER,      1, (GetAnimValue(11) > 0.5) ? Equip::PULSECANNON_4MW       : Equip::NONE);
		g_equipment.Set(Equip::SLOT_MISSILE,    0, (GetAnimValue(12) > 0.5) ? Equip::MISSILE_SMART         : Equip::NONE);
		g_equipment.Set(Equip::SLOT_MISSILE,    1, (GetAnimValue(13) > 0.5) ? Equip::MISSILE_SMART         : Equip::NONE);
		g_equipment.Set(Equip::SLOT_MISSILE,    2, (GetAnimValue(14) > 0.5) ? Equip::MISSILE_SMART         : Equip::NONE);
		g_equipment.Set(Equip::SLOT_MISSILE,    3, (GetAnimValue(15) > 0.5) ? Equip::MISSILE_SMART         : Equip::NONE);
		g_equipment.Set(Equip::SLOT_MISSILE,    4, (GetAnimValue(16) > 0.5) ? Equip::MISSILE_SMART         : Equip::NONE);
		g_equipment.Set(Equip::SLOT_MISSILE,    5, (GetAnimValue(17) > 0.5) ? Equip::MISSILE_SMART         : Equip::NONE);
		g_equipment.Set(Equip::SLOT_MISSILE,    6, (GetAnimValue(18) > 0.5) ? Equip::MISSILE_SMART         : Equip::NONE);
		g_equipment.Set(Equip::SLOT_MISSILE,    7, (GetAnimValue(19) > 0.5) ? Equip::MISSILE_SMART         : Equip::NONE);
	} else if (m_modelCategory == MODEL_SPACESTATION) {
		g_params.animStages[SpaceStation::ANIM_DOCKING_BAY_1] = int(GetAnimValue(6) * 7.0);
		g_params.animStages[SpaceStation::ANIM_DOCKING_BAY_2] = int(GetAnimValue(7) * 7.0);
		g_params.animStages[SpaceStation::ANIM_DOCKING_BAY_3] = int(GetAnimValue(8) * 7.0);
		g_params.animStages[SpaceStation::ANIM_DOCKING_BAY_4] = int(GetAnimValue(9) * 7.0);
		g_params.animValues[SpaceStation::ANIM_DOCKING_BAY_1] = GetAnimValue(10);
		g_params.animValues[SpaceStation::ANIM_DOCKING_BAY_2] = GetAnimValue(11);
		g_params.animValues[SpaceStation::ANIM_DOCKING_BAY_3] = GetAnimValue(12);
		g_params.animValues[SpaceStation::ANIM_DOCKING_BAY_4] = GetAnimValue(13);
	}

/*
	for (int i=0; i<LMR_ARG_MAX; i++) {
		params.argDoubles[i] = GetAnimValue(i);
	}
*/

	g_params.time = gameTime;

	g_params.linthrust[0] = 2.0f * (m_linthrust[0]->GetValue() - 0.5f);
	g_params.linthrust[1] = 2.0f * (m_linthrust[1]->GetValue() - 0.5f);
	g_params.linthrust[2] = 2.0f * (m_linthrust[2]->GetValue() - 0.5f);
	g_params.angthrust[0] = 2.0f * (m_angthrust[0]->GetValue() - 0.5f);
	g_params.angthrust[1] = 2.0f * (m_angthrust[1]->GetValue() - 0.5f);
	g_params.angthrust[2] = 2.0f * (m_angthrust[2]->GetValue() - 0.5f);
}


static void render_coll_mesh(const LmrCollMesh *m)
{
	Material mat;
	mat.unlit = true;
	mat.diffuse = Color(1.f, 0.f, 1.f);
	glDepthRange(0.0+g_zbias,1.0);
	VertexArray va(ATTRIB_POSITION, m->ni * 3);
	for (int i=0; i<m->ni; i+=3) {
		va.Add(static_cast<vector3f>(&m->pVertex[3*m->pIndex[i]]));
		va.Add(static_cast<vector3f>(&m->pVertex[3*m->pIndex[i+1]]));
		va.Add(static_cast<vector3f>(&m->pVertex[3*m->pIndex[i+2]]));
	}
	renderer->DrawTriangles(&va, &mat);

	mat.diffuse = Color(1.f);
	glDepthRange(0,1.0f-g_zbias);
	renderer->SetWireFrameMode(true);
	renderer->DrawTriangles(&va, &mat);
	renderer->SetWireFrameMode(false);
	glDepthRange(0,1);
}

double camera_zoom = 1.0;
vector3f g_campos(0.0f, 0.0f, 100.0f);
matrix4x4f g_camorient;

void Viewer::MainLoop()
{
	Uint32 lastTurd = SDL_GetTicks();

	Uint32 t = SDL_GetTicks();
	int numFrames = 0, fps = 0, numTris = 0;
	Uint32 lastFpsReadout = SDL_GetTicks();
	g_campos = vector3f(0.0f, 0.0f, m_cmesh->GetBoundingRadius());
	g_camorient = matrix4x4f::Identity();
	matrix4x4f modelRot = matrix4x4f::Identity();

	printf("Geom tree build in %dms\n", SDL_GetTicks() - t);

	for (;;) {
		PollEvents();

		if (g_keyState[SDLK_LSHIFT] || g_keyState[SDLK_RSHIFT]) {
			if (g_keyState[SDLK_UP]) g_camorient = g_camorient * matrix4x4f::RotateXMatrix(g_frameTime);
			if (g_keyState[SDLK_DOWN]) g_camorient = g_camorient * matrix4x4f::RotateXMatrix(-g_frameTime);
			if (g_keyState[SDLK_LEFT]) g_camorient = g_camorient * matrix4x4f::RotateYMatrix(-g_frameTime);
			if (g_keyState[SDLK_RIGHT]) g_camorient = g_camorient * matrix4x4f::RotateYMatrix(g_frameTime);
			if (g_mouseButton[3]) {
				float rx = 0.01f*g_mouseMotion[1];
				float ry = 0.01f*g_mouseMotion[0];
				g_camorient = g_camorient * matrix4x4f::RotateXMatrix(rx);
				g_camorient = g_camorient * matrix4x4f::RotateYMatrix(ry);
				if (g_mouseButton[1]) {
					g_campos = g_campos - g_camorient * vector3f(0.0f,0.0f,1.0f) * 0.01 *
						m_model->GetDrawClipRadius();
				}
			}
		} else {
			if (g_keyState[SDLK_UP]) modelRot = modelRot * matrix4x4f::RotateXMatrix(g_frameTime);
			if (g_keyState[SDLK_DOWN]) modelRot = modelRot * matrix4x4f::RotateXMatrix(-g_frameTime);
			if (g_keyState[SDLK_LEFT]) modelRot = modelRot * matrix4x4f::RotateYMatrix(-g_frameTime);
			if (g_keyState[SDLK_RIGHT]) modelRot = modelRot * matrix4x4f::RotateYMatrix(g_frameTime);
			if (g_mouseButton[3]) {
				float rx = 0.01f*g_mouseMotion[1];
				float ry = 0.01f*g_mouseMotion[0];
				modelRot = modelRot * matrix4x4f::RotateXMatrix(rx);
				modelRot = modelRot * matrix4x4f::RotateYMatrix(ry);
			}
		}
		float rate = 1.f;
		if (g_keyState[SDLK_LSHIFT]) rate = 10.f;
		if (g_keyState[SDLK_EQUALS] || g_keyState[SDLK_KP_PLUS]) g_campos = g_campos - g_camorient * vector3f(0.0f,0.0f,1.f) * rate;
		if (g_keyState[SDLK_MINUS] || g_keyState[SDLK_KP_MINUS]) g_campos = g_campos + g_camorient * vector3f(0.0f,0.0f,1.f) * rate;
		if (g_keyState[SDLK_PAGEUP]) g_campos = g_campos - g_camorient * vector3f(0.0f,0.0f,0.5f);
		if (g_keyState[SDLK_PAGEDOWN]) g_campos = g_campos + g_camorient * vector3f(0.0f,0.0f,0.5f);

//		geom->MoveTo(modelRot, vector3d(0.0,0.0,0.0));

		renderer->SetPerspectiveProjection(85, g_width/float(g_height), 1.f, 10000.f);
		renderer->SetTransform(matrix4x4f::Identity());
		renderer->ClearScreen();

		SetSbreParams();

		int beforeDrawTriStats = LmrModelGetStatsTris();

		if (g_renderType == 0) {
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			matrix4x4f m = g_camorient.InverseOf() * matrix4x4f::Translation(-g_campos) * modelRot.InverseOf();
			if (g_doBenchmark) {
				for (int i=0; i<1000; i++) m_model->Render(m, &g_params);
			} else {
				m_model->Render(m, &g_params);
			}
			glPopAttrib();
		} else if (g_renderType == 1) {
			glPushMatrix();
			matrix4x4f m = g_camorient.InverseOf() * matrix4x4f::Translation(-g_campos) * modelRot.InverseOf();
			glMultMatrixf(&m[0]);
			render_coll_mesh(m_cmesh);
			glPopMatrix();
		}

		if (m_showBoundingRadius) {
			matrix4x4f mo = g_camorient.InverseOf() * matrix4x4f::Translation(-g_campos);// * modelRot.InverseOf();
			VisualizeBoundingRadius(mo, m_model->GetDrawClipRadius());
		}

		if (m_showGrid) {
			matrix4x4f m = g_camorient.InverseOf() * matrix4x4f::Translation(-g_campos) * modelRot.InverseOf();
			DrawGrid(m, m_model->GetDrawClipRadius());
		}

		Graphics::UnbindAllBuffers();

		{
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
		}

		Gui::Draw();

		renderer->SwapBuffers();
		numFrames++;
		g_frameTime = (SDL_GetTicks() - lastTurd) * 0.001f;
		lastTurd = SDL_GetTicks();

		if (SDL_GetTicks() - lastFpsReadout > 1000) {
			numTris = LmrModelGetStatsTris();
			fps = numFrames;
			numFrames = 0;
			lastFpsReadout = SDL_GetTicks();
			LmrModelClearStatsTris();
		}

		//space->Collide(onCollision);
	}
}

static void PollEvents()
{
	SDL_Event event;

	g_mouseMotion[0] = g_mouseMotion[1] = 0;
	while (SDL_PollEvent(&event)) {
		Gui::HandleSDLEvent(&event);
		switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
                    if (g_viewer->m_model) {
                        g_viewer->PickModel();
                    } else {
                        SDL_Quit();
                        exit(0);
                    }
				}
				//XXX can't reliably fullscreen
				//if (event.key.keysym.sym == SDLK_F11) SDL_WM_ToggleFullScreen(g_screen);
				g_keyState[event.key.keysym.sym] = 1;
				break;
			case SDL_KEYUP:
				g_keyState[event.key.keysym.sym] = 0;
				break;
			case SDL_MOUSEBUTTONDOWN:
				setMouseButton(event.button.button, 1);
	//			Pi::onMouseButtonDown.emit(event.button.button,
	//					event.button.x, event.button.y);
				break;
			case SDL_MOUSEBUTTONUP:
				setMouseButton(event.button.button, 0);
	//			Pi::onMouseButtonUp.emit(event.button.button,
	//					event.button.x, event.button.y);
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
	Drawables::Circle circ(radius, Color(0.f, 0.f, 1.f, 1.f));
	circ.Draw(renderer);
}

void Viewer::DrawGrid(matrix4x4f& trans, double radius)
{
	const float dist = abs(g_campos.z);

	const float max = std::min(powf(10, ceilf(log10f(dist))), ceilf(radius/gridInterval)*gridInterval);

	std::vector<vector3f> points;

	for (float x = -max; x <= max; x += gridInterval) {
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

	g_zbias = 2.0/(1<<16);

	Gui::Init(renderer, g_width, g_height, g_width, g_height);

	LmrModelCompilerInit(renderer);
	LmrNotifyScreenWidth(g_width);

	ShipType::Init();

	g_viewer = new Viewer();
	if (argc >= 4) {
		try {
			LmrModel *m = LmrLookupModelByName(argv[3]);
			g_viewer->SetModel(m);
		} catch (LmrModelNotFoundException) {
			g_viewer->PickModel(argv[3], std::string("Could not find model: ") + argv[3]);
		}
	} else {
		g_viewer->PickModel();
	}

	g_viewer->MainLoop();
	//XXX looks like this is never reached
	FileSystem::Uninit();
	delete renderer;
	return 0;
}
