#include "libs.h"
#include "gui/Gui.h"
#include "collider/collider.h"
#include "FileSystem.h"
#include "newmodel/Newmodel.h"
#include "graphics/Drawables.h"
#include "graphics/Material.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"

using namespace Graphics;

static Renderer *renderer;

static SDL_Surface *g_screen;
static int g_width, g_height;
static int g_mouseMotion[2];
static char g_keyState[SDLK_LAST];
static const int MAX_MOUSE_BTN_IDX = SDL_BUTTON_WHEELDOWN + 1;
static int g_mouseButton[MAX_MOUSE_BTN_IDX];	// inc to 6 as mouseScroll is index 5
static bool g_doBenchmark = false;
static vector3f g_campos(0.0f, 0.0f, 100.0f);
static matrix4x4f g_camorient;

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

static int g_renderType = 0;
static float g_frameTime;

class Viewer: public Gui::Fixed {
public:
	CollisionSpace *m_space;
	CollMesh *m_cmesh;
	Geom *m_geom;
	Gui::Label *m_trisReadout;
	//Newmodel::NModel *m_model;
	Model *m_model;

	void ResetCamera();

	void SetModel(Model *);

	void PickModel(const std::string &initial_name, const std::string &initial_errormsg);

	void PickModel() {
		PickModel("", "");
	}

	Viewer(): Gui::Fixed(float(g_width), float(g_height)),
		m_model(0),
		m_cmesh(0),
		m_geom(0)
	{
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

		ShowAll();
		Show();
	}

	~Viewer() {
		delete m_model;
	}

	void OnAnimChange(Gui::Adjustment *a, Gui::TextEntry *e) {
		char buf[128];
		snprintf(buf, sizeof(buf), "%.2f", a->GetValue());
		e->SetText(buf);
	}

	void OnClickToggleBenchmark() {
		g_doBenchmark = !g_doBenchmark;
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
private:
	void TryModel(const SDL_keysym *sym, Gui::TextEntry *entry, Gui::Label *errormsg);
	void VisualizeBoundingRadius(matrix4x4f& trans, double radius);
	void DrawGrid(matrix4x4f& trans, double radius);
	bool m_showBoundingRadius;
	bool m_showGrid;
};

void Viewer::ResetCamera()
{
	g_campos = vector3f(0.0f, 0.0f, 10.f);
	g_camorient = matrix4x4f::Identity();
	matrix4x4f modelRot = matrix4x4f::Identity();
}

void Viewer::SetModel(Model *model)
{
	// clear old geometry
	if (m_model) delete m_model;
	if (m_cmesh) delete m_cmesh;
	if (m_geom) {
		m_space->RemoveGeom(m_geom);
		delete m_geom;
	}

	m_model = model;
	m_cmesh = m_model->CreateCollisionMesh(0);
	m_geom = new Geom(m_cmesh->GetGeomTree());
	m_space->AddGeom(m_geom);
	ResetCamera();
}

void Viewer::TryModel(const SDL_keysym *sym, Gui::TextEntry *entry, Gui::Label *errormsg)
{
	if (sym->sym == SDLK_RETURN) {
		Newmodel::Loader load(renderer);
		try {
			Model *mo = load.LoadModel(entry->GetText());
			SetModel(mo);
		} catch (Newmodel::LoadingError &) {
			errormsg->SetText("Could not find model: " + entry->GetText());
		}	
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

	Uint32 t = SDL_GetTicks();
	int numFrames = 0, fps = 0, numTris = 0;
	Uint32 lastFpsReadout = SDL_GetTicks();
	//g_campos = vector3f(0.0f, 0.0f, m_cmesh->GetBoundingRadius());
	g_camorient = matrix4x4f::Identity();
	matrix4x4f modelRot = matrix4x4f::Identity();

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

		renderer->SetPerspectiveProjection(85, g_width/float(g_height), 1.f, 10000.f);
		renderer->SetTransform(matrix4x4f::Identity());
		renderer->ClearScreen();
#if 0
		int beforeDrawTriStats = LmrModelGetStatsTris();
#endif
		if (g_renderType == 0) {
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			matrix4x4f m = g_camorient.InverseOf() * matrix4x4f::Translation(-g_campos) * modelRot.InverseOf();
			if (g_doBenchmark) {
				for (int i=0; i<1000; i++) m_model->Render(renderer, m, 0);
			} else {
				//m_model->Render(renderer, m, &g_params);
				m_model->Render(renderer, m, 0);
			}
			glPopAttrib();
		} else if (g_renderType == 1) {
			glPushMatrix();
			matrix4x4f m = g_camorient.InverseOf() * matrix4x4f::Translation(-g_campos) * modelRot.InverseOf();
			glMultMatrixf(&m[0]);
#if 0
			render_coll_mesh(m_cmesh);
#endif
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
		
		Gui::Draw();
		
		renderer->SwapBuffers();
		numFrames++;
		g_frameTime = (SDL_GetTicks() - lastTurd) * 0.001f;
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
				g_keyState[event.key.keysym.sym] = 1;
				break;
			case SDL_KEYUP:
				g_keyState[event.key.keysym.sym] = 0;
				break;
			case SDL_MOUSEBUTTONDOWN:
				setMouseButton(event.button.button, 1);
				break;
			case SDL_MOUSEBUTTONUP:
				setMouseButton(event.button.button, 0);
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

	renderer = Graphics::Init(g_width, g_height, true);
	Gui::Init(renderer, g_width, g_height, g_width, g_height);

	g_viewer = new Viewer();
	if (argc >= 4) {
		try {
			Newmodel::Loader loader(renderer);
			Model *mo = loader.LoadModel(argv[3]);
			g_viewer->SetModel(mo);
		} catch (Newmodel::LoadingError &) {
			g_viewer->PickModel(argv[3], std::string("Could not find model: ") + argv[3]);
		}
	} else {
		g_viewer->PickModel();
	}

	g_viewer->MainLoop();
	FileSystem::Uninit();
	delete renderer;
	return 0;
}
