#include "NewModelViewer.h"
#include "FileSystem.h"
#include "graphics/Graphics.h"
#include "newmodel/Newmodel.h"
#include "OS.h"
#include "Pi.h"

ModelViewer::ModelViewer(Graphics::Renderer *r, LuaManager *lm, int width, int height)
: m_done(false)
, m_collMesh(0)
, m_frameTime(0.f)
, m_renderer(r)
, m_width(width)
, m_height(height)
, m_model(0)
, m_camPos(0.f)
{
	//LOD system needs to know the screen width
	m_modelParams.scrWidth = m_width;

	m_ui.Reset(new UI::Context(lm, r, width, height));
	SetupUI();

	m_keyStates.fill(false);
	m_mouseMotion.fill(0);
	m_mouseButton.fill(false);
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
	Pi::luaManager = new LuaManager();
	lua_State *l = Pi::luaManager->GetLuaState();
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
	viewer = new ModelViewer(renderer, Pi::luaManager, width, height);
	viewer->SetModel(modelName);
	viewer->MainLoop();

	//uninit components
	PersistentTable::Uninit(Pi::luaManager->GetLuaState());
	delete Pi::luaManager;
	delete renderer;
	Graphics::Uninit();
	FileSystem::Uninit();
	SDL_Quit();
}

void ModelViewer::DrawBackground()
{
	m_renderer->SetDepthWrite(false);
	m_renderer->SetBlendMode(Graphics::BLEND_SOLID);
	m_renderer->SetOrthographicProjection(0.f, 1.f, 0.f, 1.f, -1.f, 1.f);

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

struct Poptions {
	Poptions() : gridInterval(10.f) { }
	float gridInterval;
};
static Poptions m_options;

//Draw grid and axes
void ModelViewer::DrawGrid(const matrix4x4f &trans, float radius)
{
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
}

void ModelViewer::DrawModel()
{
	assert(m_model);

	//set perspective projeciton, update mv matrix, render
	//get maximum z range
	float znear, zfar;
	m_renderer->GetNearFarRange(znear, zfar);
	m_renderer->SetPerspectiveProjection(85, m_width/float(m_height), znear, zfar);
	m_renderer->SetDepthTest(true);
	m_renderer->SetDepthWrite(true);
	m_renderer->SetBlendMode(Graphics::BLEND_SOLID);

	const matrix4x4f mv = matrix4x4f::Translation(-m_camPos) * m_modelRot.InverseOf();

	DrawGrid(mv, m_model->GetDrawClipRadius());

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

		m_ui->Draw();

		m_renderer->SwapBuffers();
	}
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
	 * printscr - screenshot
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

void ModelViewer::SetModel(const std::string &filename)
{
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
	} catch (Newmodel::LoadingError &err) {
		// report the error and show model picker.
		m_model = 0;
		nameLabel->SetText(stringf("Could not load model %0: %1", filename.c_str(), err.what()).c_str());
	}

	ResetCamera();
}

void ModelViewer::SetupUI()
{
	m_ui->SetInnerWidget(nameLabel = m_ui->Label("Pie"));
	m_ui->Layout();
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
