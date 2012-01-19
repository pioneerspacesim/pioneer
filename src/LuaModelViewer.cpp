#include "libs.h"
#include "gui/Gui.h"
#include "collider/collider.h"
#include "LmrModel.h"
#include "ShipType.h"
#include "EquipType.h"
#include "render/Render.h"
#include "Ship.h" // for the flight state and ship animation enums
#include "SpaceStation.h" // for the space station animation enums
#include "TextureCache.h"

enum ModelCategory {
	MODEL_OTHER,
	MODEL_SHIP,
	MODEL_SPACESTATION
};

static const char *ANIMATION_NAMESPACES[] = {
	0,
	"ShipAnimation",
	"SpaceStationAnimation",
};

static const int LMR_ARG_MAX = 40;

static SDL_Surface *g_screen;
static int g_width, g_height;
static int g_mouseMotion[2];
static char g_keyState[SDLK_LAST];
static const int MAX_MOUSE_BTN_IDX = SDL_BUTTON_WHEELDOWN + 1;
static int g_mouseButton[MAX_MOUSE_BTN_IDX];	// inc to 6 as mouseScroll is index 5
static float g_zbias;
static bool g_doBenchmark = false;

static bool setMouseButton(const Uint8 idx, const int value)
{
	if( idx < MAX_MOUSE_BTN_IDX ) {
		g_mouseButton[idx] = value;
		return true;
	}
	return false;
}

static GLuint mytexture;

class Viewer;
static Viewer *g_viewer;

static void PollEvents();
extern void LmrModelCompilerInit(TextureCache *textureCache);

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
				Gui::Fixed *box = new Gui::Fixed(32.0f, 120.0f);
				Add(box, float(200 + i*25), Gui::Screen::GetHeight()-120.0f);

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
		for (int i=0; i<LMR_ARG_MAX; i++) m_anim[i]->SetValue(0);
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

	void MainLoop() __attribute((noreturn));
	void SetSbreParams();
private:
	void TryModel(const SDL_keysym *sym, Gui::TextEntry *entry, Gui::Label *errormsg);
	void VisualizeBoundingRadius(matrix4x4f& trans, double radius);
	bool m_showBoundingRadius;
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
	bool has_ship = m_model->HasTag("ship") || m_model->HasTag("static_ship");
	bool has_station = m_model->HasTag("surface_station") || m_model->HasTag("orbital_station");
	if (has_ship && !has_station) {
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
	} else if (has_station && !has_ship) {
		m_modelCategory = MODEL_SPACESTATION;
		g_params.equipment = 0;
	} else {
		m_modelCategory = MODEL_OTHER;
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
		Render::PrepareFrame();
		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Render::PostProcess();
		Gui::Draw();
		glError();
		Render::SwapBuffers();
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
	glDisable(GL_LIGHTING);
	glColor3f(1,0,1);
	glDepthRange(0.0+g_zbias,1.0);
	glBegin(GL_TRIANGLES);
	for (int i=0; i<m->ni; i+=3) {
		glVertex3fv(&m->pVertex[3*m->pIndex[i]]);
		glVertex3fv(&m->pVertex[3*m->pIndex[i+1]]);
		glVertex3fv(&m->pVertex[3*m->pIndex[i+2]]);
	}
	glEnd();
	glColor3f(1,1,1);
	glDepthRange(0,1.0f-g_zbias);
	for (int i=0; i<m->ni; i+=3) {
		glBegin(GL_LINE_LOOP);
		glVertex3fv(&m->pVertex[3*m->pIndex[i]]);
		glVertex3fv(&m->pVertex[3*m->pIndex[i+1]]);
		glVertex3fv(&m->pVertex[3*m->pIndex[i+2]]);
		glEnd();
	}
	glDepthRange(0,1);
	glEnable(GL_LIGHTING);
}

#define TEXSIZE 512
float wank[TEXSIZE][TEXSIZE];
float aspectRatio = 1.0;
double camera_zoom = 1.0;
vector3f g_campos(0.0f, 0.0f, 100.0f);
matrix4x4f g_camorient;
extern int stat_rayTriIntersections;
static void raytraceCollMesh(vector3d camPos, vector3d camera_up, vector3d camera_forward, CollisionSpace *space)
{
	memset(wank, 0, sizeof(float)*TEXSIZE*TEXSIZE);

	vector3d toPoint, xMov, yMov;

	vector3d topLeft, topRight, botRight, cross;
	topLeft = topRight = botRight = camera_forward * camera_zoom;
	cross = camera_forward.Cross(camera_up) * aspectRatio;
	topLeft = topLeft + camera_up - cross;
	topRight = topRight + camera_up + cross;
	botRight = botRight - camera_up + cross;

	xMov = topRight - topLeft;
	yMov = botRight - topRight;
	float xstep = 1.0f / TEXSIZE;
	float ystep = 1.0f / TEXSIZE;
	float xpos, ypos;
	ypos = 0.0f;
	GeomTree::stats_rayTriIntersections = 0;

	Uint32 t = SDL_GetTicks();
	for (int y=0; y<TEXSIZE; y++, ypos += ystep) {
		xpos = 0.0f;
		for (int x=0; x<TEXSIZE; x++, xpos += xstep) {
			toPoint = (topLeft + (xMov * xpos) + (yMov * ypos)).Normalized();
			
			CollisionContact c;
			space->TraceRay(camPos, toPoint, 1000000.0f, &c);

			if (c.triIdx != -1) {
				wank[x][y] = 100.0/(10*c.dist);
			} else {
				wank[x][y] = 0;
			}
		}
	}
	printf("%.3f million rays/sec, %.2f tri isect tests per ray\n", (TEXSIZE*TEXSIZE)/(1000.0*(SDL_GetTicks()-t)),
				GeomTree::stats_rayTriIntersections/float(TEXSIZE*TEXSIZE));
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mytexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXSIZE, TEXSIZE, 0, GL_LUMINANCE, GL_FLOAT, wank);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//glActiveTexture(GL_TEXTURE0);
	glDisable(GL_LIGHTING);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBegin(GL_TRIANGLE_FAN);
		glTexCoord2i(0,1);
		glVertex3f(1,1,0);

		glTexCoord2i(0,0);
		glVertex3f(0,1,0);
		
		glTexCoord2i(1,0);
		glVertex3f(0,0,0);
		
		glTexCoord2i(1,1);
		glVertex3f(1,0,0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

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

	Render::State::SetZnearZfar(1.0f, 10000.0f);

	for (;;) {
		PollEvents();
		Render::PrepareFrame();

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

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		float fracH = g_height / float(g_width);
		glFrustum(-1, 1, -fracH, fracH, 1.0f, 10000.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
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
		} else {
			matrix4x4f tran = modelRot * g_camorient;//.InverseOf();
			vector3d forward = vector3d(tran * vector3f(0.0,0.0,-1.0));
			vector3d up = vector3d(tran * vector3f(0.0,1.0,0.0));
			raytraceCollMesh(vector3d(modelRot * g_campos), up, forward, m_space);
		}
		Render::State::UseProgram(0);
		if (m_showBoundingRadius) {
			matrix4x4f mo = g_camorient.InverseOf() * matrix4x4f::Translation(-g_campos);// * modelRot.InverseOf();
			VisualizeBoundingRadius(mo, m_model->GetDrawClipRadius());
		}
		Render::UnbindAllBuffers();

		{
			char buf[128];
			Aabb aabb = m_cmesh->GetAabb();
			snprintf(buf, sizeof(buf), "%d triangles, %d fps, %.3fm tris/sec\ncollision mesh size: %.1fx%.1fx%.1f (radius %.1f)\nClipping radius %.1f",
					(g_renderType == 0 ? 
						LmrModelGetStatsTris() - beforeDrawTriStats :
						m_cmesh->m_numTris),
					fps,
					numTris/1000000.0f,
					aabb.max.x-aabb.min.x,
					aabb.max.y-aabb.min.y,
					aabb.max.z-aabb.min.z,
					aabb.GetBoundingRadius(),
					m_model->GetDrawClipRadius());
			m_trisReadout->SetText(buf);
		}
		
		Render::PostProcess();
		Gui::Draw();
		
		glError();
		Render::SwapBuffers();
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
				if (event.key.keysym.sym == SDLK_F11) SDL_WM_ToggleFullScreen(g_screen);
				if (event.key.keysym.sym == SDLK_s && (g_viewer->m_model)) {
					Render::ToggleShaders();
				}
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
	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glMultMatrixf(&trans[0]);
	glColor4f(0.5f, 0.9f, 0.9f, 1.0f);
	glColor3f(0,0,1);
	glBegin(GL_LINE_LOOP);
	for (float theta=0; theta < 2*M_PI; theta += 0.05*M_PI) {
		glVertex3f(radius*sin(theta), radius*cos(theta), 0);
	}
	glEnd();
	glPopAttrib();
	glPopMatrix();
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

	const SDL_VideoInfo *info = NULL;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Video initialization failed: %s\n", SDL_GetError());
		exit(-1);
	}

	info = SDL_GetVideoInfo();

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	g_zbias = 2.0/(1<<16);

	Uint32 flags = SDL_OPENGL;

	if ((g_screen = SDL_SetVideoMode(g_width, g_height, info->vfmt->BitsPerPixel, flags)) == 0) {
		// fall back on 16-bit depth buffer...
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		fprintf(stderr, "Failed to set video mode. (%s). Re-trying with 16-bit depth buffer.\n", SDL_GetError());
		if ((g_screen = SDL_SetVideoMode(g_width, g_height, info->vfmt->BitsPerPixel, flags)) == 0) {
			fprintf(stderr, "Video mode set failed: %s\n", SDL_GetError());
		}
	}
	glewInit();

	glShadeModel(GL_SMOOTH);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

	glGenTextures(1, &mytexture);
	glBindTexture(GL_TEXTURE_2D, mytexture);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glClearColor(0,0,0,0);
	glViewport(0, 0, g_width, g_height);

	TextureCache *textureCache = new TextureCache;

	Render::Init(g_width, g_height);
	Gui::Init(g_width, g_height, g_width, g_height);

	LmrModelCompilerInit(textureCache);
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
	return 0;
}
