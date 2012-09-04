#include "NewModelViewer.h"
#include "FileSystem.h"
#include "graphics/Graphics.h"
#include "graphics/Light.h"
#include "graphics/TextureBuilder.h"
#include "newmodel/Newmodel.h"
#include "OS.h"
#include "Pi.h"
#include <sstream>

//default options
ModelViewer::Options::Options()
: showGrid(false)
, showUI(true)
, gridInterval(10.f)
, lightPreset(0)
{
}

namespace {
	vector3f az_el_to_dir(float yaw, float pitch) {
		//0,0 points to "right" (1,0,0)
		vector3f v;
		v.x = cos(DEG2RAD(yaw)) * cos(DEG2RAD(pitch));
		v.y = sin(DEG2RAD(pitch));
		v.z = sin(DEG2RAD(yaw)) * cos(DEG2RAD(pitch));
		return v;
	}
}

ModelViewer::ModelViewer(Graphics::Renderer *r, LuaManager *lm, int width, int height)
: m_done(false)
, m_screenshotQueued(false)
, m_collMesh(0)
, m_frameTime(0.f)
, m_renderer(r)
, m_width(width)
, m_height(height)
, m_model(0)
, m_logString("")
, m_modelName("")
, m_camPos(0.f)
{
	//LOD system needs to know the screen width
	m_modelParams.scrWidth = m_width;

	m_ui.Reset(new UI::Context(lm, r, width, height));
	SetupUI();

	m_keyStates.fill(false);
	m_mouseMotion.fill(0);
	m_mouseButton.fill(false);

	//sweet pioneer badge for decal testing
	m_decalTexture = Graphics::TextureBuilder(
		"icons/badge.png",
		Graphics::LINEAR_CLAMP,
		true, true, false).GetOrCreateTexture(m_renderer, "model");
}

ModelViewer::~ModelViewer()
{
	if (m_model) {
		delete m_model;
		m_model = 0;
	}
}

void ModelViewer::Run(int argc, char** argv)
{
	int width = 800;
	int height = 600;
	Graphics::Renderer *renderer;
	ModelViewer *viewer;

#if 0
	//read command line options, if no model name is specified
	//the model selector is shown
	if ((argc<=1) || (0==strcmp(argv[1],"--help"))) {
		printf("Usage:\nmodelviewer <width> <height> <model name>\n");
	}
#endif
	if (argc >= 3) {
		width = atoi(argv[1]);
		height = atoi(argv[2]);
	}
	const std::string modelName = argv[3];

	//init components
	FileSystem::Init();
	FileSystem::rawFileSystem.MakeDirectory(FileSystem::GetUserDir());
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		OS::Error("SDL initialization failed: %s\n", SDL_GetError());
	Lua::Init();
	lua_State *l = Lua::manager->GetLuaState();
	PersistentTable::Init(l);

	//video
	Graphics::Settings videoSettings = {};
	videoSettings.width = width;
	videoSettings.height = height;
	videoSettings.shaders = true;
	videoSettings.requestedSamples = 4;
	videoSettings.vsync = true;
	renderer = Graphics::Init(videoSettings);

	OS::LoadWindowIcon();
	SDL_WM_SetCaption("Newmodelviewer","Newmodelviewer");

	//run main loop until quit
	viewer = new ModelViewer(renderer, Lua::manager, width, height);
	viewer->SetModel(modelName);
	viewer->MainLoop();

	//uninit components
	PersistentTable::Uninit(Lua::manager->GetLuaState());
	Lua::Uninit();
	delete renderer;
	Graphics::Uninit();
	FileSystem::Uninit();
	SDL_Quit();
}

bool ModelViewer::OnReloadModel(UI::Widget *w)
{
	SetModel(m_modelName);
	return true;
}

bool ModelViewer::OnToggleGrid(UI::Widget *)
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

void ModelViewer::AddLog(const std::string &line)
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

void ModelViewer::DrawBackground()
{
	m_renderer->SetDepthWrite(false);
	m_renderer->SetBlendMode(Graphics::BLEND_SOLID);
	m_renderer->SetOrthographicProjection(0.f, 1.f, 0.f, 1.f, -1.f, 1.f);
	m_renderer->SetTransform(matrix4x4f::Identity());

	static Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE);
	va.Clear();
	const Color4f top = Color::BLACK;
	const Color4f bottom = Color4f(0.3f);
	va.Add(vector3f(0.f, 0.f, 0.f), bottom);
	va.Add(vector3f(1.f, 0.f, 0.f), bottom);
	va.Add(vector3f(1.f, 1.f, 0.f), top);

	va.Add(vector3f(0.f, 0.f, 0.f), bottom);
	va.Add(vector3f(1.f, 1.f, 0.f), top);
	va.Add(vector3f(0.f, 1.f, 0.f), top);

	m_renderer->DrawTriangles(&va, Graphics::vtxColorMaterial);
}

//Draw grid and axes
void ModelViewer::DrawGrid(const matrix4x4f &trans, float radius)
{
	assert(m_options.showGrid);

	const float dist = abs(m_camPos.z);

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

	m_renderer->SetTransform(trans);
	m_renderer->DrawLines(points.size(), &points[0], Color(0.5f));//Color(0.0f,0.2f,0.0f,1.0f));

	//industry-standard red/green/blue XYZ axis indiactor
	const int numAxVerts = 6;
	const vector3f vts[numAxVerts] = {
		//X
		vector3f(0.f, 0.f, 0.f),
		vector3f(radius, 0.f, 0.f),

		//Y
		vector3f(0.f, 0.f, 0.f),
		vector3f(0.f, radius, 0.f),

		//Z
		vector3f(0.f, 0.f, 0.f),
		vector3f(0.f, 0.f, radius),
	};
	const Color col[numAxVerts] = {
		Color(1.f, 0.f, 0.f),
		Color(1.f, 0.f, 0.f),

		Color(0.f, 0.f, 1.f),
		Color(0.f, 0.f, 1.f),

		Color(0.f, 1.f, 0.f),
		Color(0.f, 1.f, 0.f)
	};

	m_renderer->SetDepthTest(true);
	m_renderer->SetDepthWrite(true);
	m_renderer->DrawLines(numAxVerts, &vts[0], &col[0]);
}

void ModelViewer::DrawLog()
{
	const Color4f yellowish = Color4f(0.9, 0.9, 0.3f, 1.f);
	m_renderer->SetTransform(matrix4x4f::Identity());
	m_ui->GetContext()->GetFont()->RenderString(m_logString.c_str(), m_width - 512.f, 10.f, yellowish);
}

void ModelViewer::DrawModel()
{
	assert(m_model);
	m_renderer->SetBlendMode(Graphics::BLEND_SOLID);

	//get maximum z range
	float znear, zfar;
	m_renderer->GetNearFarRange(znear, zfar);
	m_renderer->SetPerspectiveProjection(85, m_width/float(m_height), znear, zfar);
	m_renderer->SetTransform(matrix4x4f::Identity());
	UpdateLights();

	const matrix4x4f mv = matrix4x4f::Translation(-m_camPos) * m_modelRot.InverseOf();

	if (m_options.showGrid)
		DrawGrid(mv, m_model->GetDrawClipRadius());

	m_renderer->SetDepthTest(true);
	m_renderer->SetDepthWrite(true);

	m_model->Render(m_renderer, mv, &m_modelParams);
}

void ModelViewer::MainLoop()
{
	Uint32 lastTime = SDL_GetTicks();
	while (!m_done)
	{
		m_frameTime = (SDL_GetTicks() - lastTime) * 0.001f;
		lastTime = SDL_GetTicks();

		m_renderer->ClearScreen();

		PollEvents();
		UpdateCamera();

		DrawBackground();

		//update animations, draw model etc.
		if (m_model)
			DrawModel();

		if (m_options.showUI && !m_screenshotQueued) {
			m_ui->Draw();
			DrawLog(); //assuming the screen is pixel sized ortho after UI
		}
		if (m_screenshotQueued) {
			m_screenshotQueued = false;
			Screenshot();
		}

		m_renderer->SwapBuffers();
	}
}

void ModelViewer::OnLightPresetChanged(unsigned int index, const std::string &)
{
	m_options.lightPreset = std::min<unsigned int>(index, 2);
}

void ModelViewer::PollEvents()
{
	/*
	 * Special butans
	 *
	 * Space: reset camera
	 * Keyboard: rotate view
	 * plus/minus: zoom view
	 * Shift: zoom faster
	 *
	 * Planned:
	 * numpad - preset camera views, blenderish
	 * g - grid
	 * tab - toggle ui (always invisible on screenshots)
	 */
	m_mouseMotion[0] = m_mouseMotion[1] = 0;

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		//ui gets all events
		m_ui->DispatchSDLEvent(event);

		switch (event.type)
		{
		case SDL_QUIT:
			m_done = true;
			break;
		case SDL_MOUSEMOTION:
			m_mouseMotion[0] += event.motion.xrel;
			m_mouseMotion[1] += event.motion.yrel;
			break;
		case SDL_MOUSEBUTTONDOWN:
			m_mouseButton[event.button.button] = true;
			break;
		case SDL_MOUSEBUTTONUP:
			m_mouseButton[event.button.button] = false;
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				m_done = true;
				break;
			case SDLK_SPACE:
				ResetCamera();
				break;
			case SDLK_TAB:
				m_options.showUI = !m_options.showUI;
				break;
			case SDLK_PRINT:
				m_screenshotQueued = true;
				break;
			}
			m_keyStates[event.key.keysym.sym] = true;
			break;
		case SDL_KEYUP:
			m_keyStates[event.key.keysym.sym] = false;
			break;
		default:
			break;
		}
	}
}

void ModelViewer::ResetCamera()
{
	if (!m_model)
		m_camPos = vector3f(0.f, 0.f, 100.f);
	else
		m_camPos = vector3f(0.0f, 0.0f, m_model->GetDrawClipRadius() * 1.5f);
	//m_camOrient = matrix4x4f::Identity();
	m_modelRot = matrix4x4f::Identity();
}

void ModelViewer::Screenshot()
{
	char buf[256];
	const time_t t = time(0);
	const struct tm *_tm = localtime(&t);
	strftime(buf, sizeof(buf), "modelviewer-%Y%m%d-%H%M%S.png", _tm);
	Screendump(buf, m_width, m_height);
	AddLog(stringf("Screenshot %0 saved", buf));
}

void ModelViewer::SetModel(const std::string &filename)
{
	AddLog(stringf("Loading model %0...", filename));
	if (m_model) {
		delete m_model;
		m_model = 0;
	}
	if (m_collMesh) {
		delete m_collMesh;
		m_collMesh = 0;
	}

	try {
		Newmodel::Loader loader(m_renderer);
		m_model = loader.LoadModel(filename);
		nameLabel->SetText(filename);
		//needed to get camera distance right
		m_collMesh = m_model->CreateCollisionMesh(0);
		m_modelName = filename;
		m_model->SetDecalTexture(m_decalTexture, 0);
		AddLog("Done.");
	} catch (Newmodel::LoadingError &err) {
		// report the error and show model picker.
		m_model = 0;
		AddLog(stringf("Could not load model %0: %1", filename, err.what()));
	}

	ResetCamera();
}

//add a horizontal button/label pair to a box
static void add_pair(RefCountedPtr<UI::Context> c, UI::Box *box, UI::Widget *widget, const std::string &label)
{
	box->PackEnd(
		c->HBox()->PackEnd(
			UI::WidgetSet(
				widget,
				c->Label(label)
			)
		)
	);
}

void ModelViewer::SetupUI()
{
	UI::VBox* box = m_ui->VBox();
	UI::Button *toggleGridButton;
	UI::Button *reloadButton;
	box->PackEnd(nameLabel = m_ui->Label("Pie"));
	add_pair(m_ui, box, reloadButton = m_ui->Button(), "Reload model");
	add_pair(m_ui, box, toggleGridButton = m_ui->Button(), "Grid mode");

	//light dropdown
	UI::DropDown *lightSelector;
	box->PackEnd(m_ui->Label("Lights:"));
	box->PackEnd(
		lightSelector = m_ui->DropDown()
			->AddOption("1  Front white")
			->AddOption("2  Two-point")
			->AddOption("3  Backlight")
	);
	m_ui->SetInnerWidget(box);
	m_ui->Layout();

	//event handlers
	toggleGridButton->onClick.connect(sigc::bind(sigc::mem_fun(*this, &ModelViewer::OnToggleGrid), toggleGridButton));
	reloadButton->onClick.connect(sigc::bind(sigc::mem_fun(*this, &ModelViewer::OnReloadModel), reloadButton));
	lightSelector->onOptionSelected.connect(sigc::mem_fun(*this, &ModelViewer::OnLightPresetChanged));
}

void ModelViewer::UpdateCamera()
{
	float rate = 5.f * m_frameTime;
	if (m_keyStates[SDLK_LSHIFT]) rate = 50.f * m_frameTime;

	//zoom
	if (m_keyStates[SDLK_EQUALS] || m_keyStates[SDLK_KP_PLUS]) m_camPos = m_camPos - vector3f(0.0f,0.0f,1.f) * rate;
	if (m_keyStates[SDLK_MINUS] || m_keyStates[SDLK_KP_MINUS]) m_camPos = m_camPos + vector3f(0.0f,0.0f,1.f) * rate;

	//rotate
	if (m_keyStates[SDLK_UP]) m_modelRot = m_modelRot * matrix4x4f::RotateXMatrix(m_frameTime);
	if (m_keyStates[SDLK_DOWN]) m_modelRot = m_modelRot * matrix4x4f::RotateXMatrix(-m_frameTime);
	if (m_keyStates[SDLK_LEFT]) m_modelRot = m_modelRot * matrix4x4f::RotateYMatrix(-m_frameTime);
	if (m_keyStates[SDLK_RIGHT]) m_modelRot = m_modelRot * matrix4x4f::RotateYMatrix(m_frameTime);

	//mouse rotate when right button held
	if (m_mouseButton[SDL_BUTTON_RIGHT]) {
		const float rx = 0.01f*m_mouseMotion[1];
		const float ry = 0.01f*m_mouseMotion[0];
		m_modelRot = m_modelRot * matrix4x4f::RotateXMatrix(rx);
		m_modelRot = m_modelRot * matrix4x4f::RotateYMatrix(ry);
	}
}

void ModelViewer::UpdateLights()
{
	using Graphics::Light;
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

	m_renderer->SetLights(2, &lights[0]);
}
