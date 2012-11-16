#include "NewModelViewer.h"
#include "FileSystem.h"
#include "graphics/Graphics.h"
#include "graphics/Light.h"
#include "graphics/TextureBuilder.h"
#include "newmodel/Newmodel.h"
#include "newmodel/DumpVisitor.h"
#include "newmodel/FindNodeVisitor.h"
#include "OS.h"
#include "Pi.h"
#include <sstream>

//default options
ModelViewer::Options::Options()
: attachGuns(false)
, showCollMesh(false)
, showGrid(false)
, showUI(true)
, wireframe(false)
, gridInterval(10.f)
, lightPreset(0)
{
}

//some utility functions
namespace {
	//azimuth/elevation in degrees to a dir vector
	vector3f az_el_to_dir(float yaw, float pitch) {
		//0,0 points to "right" (1,0,0)
		vector3f v;
		v.x = cos(DEG2RAD(yaw)) * cos(DEG2RAD(pitch));
		v.y = sin(DEG2RAD(pitch));
		v.z = sin(DEG2RAD(yaw)) * cos(DEG2RAD(pitch));
		return v;
	}

	//extract color from RGB sliders
	Color4ub get_slider_color(UI::Slider *r, UI::Slider *g, UI::Slider *b) {
		return Color4ub(r->GetValue() * 255.f, g->GetValue() * 255.f, b->GetValue() * 255.f);
	}

	float get_thrust(const UI::Slider *s) {
		return 1.f - (2.f * s->GetValue());
	}

	//add a horizontal button/label pair to a box
	void add_pair(UI::Context *c, UI::Box *box, UI::Widget *widget, const std::string &label) {
		box->PackEnd(c->HBox()->PackEnd(UI::WidgetSet( widget, c->Label(label) )));
	}
}

ModelViewer::ModelViewer(Graphics::Renderer *r, LuaManager *lm, int width, int height)
: m_done(false)
, m_playing(false)
, m_screenshotQueued(false)
, m_animTime(0.001 * SDL_GetTicks())
, m_frameTime(0.f)
, m_renderer(r)
, m_height(height)
, m_width(width)
, m_rng(time(0))
, m_currentAnimation(0)
, m_model(0)
, m_modelName("")
, m_camPos(0.f)
{
	m_ui.Reset(new UI::Context(lm, r, width, height));

	m_log = m_ui->MultiLineText("");
	m_log->SetFont(UI::Widget::FONT_XSMALL);
	m_logScroller = m_ui->Scroller();
	m_logScroller->SetInnerWidget(m_log);

	std::fill(m_keyStates, m_keyStates + COUNTOF(m_keyStates), false);
	std::fill(m_mouseButton, m_mouseButton + COUNTOF(m_mouseButton), false);
	std::fill(m_mouseMotion, m_mouseMotion + 2, 0);

	//sweet pioneer badge for decal testing
	m_decalTexture = Graphics::TextureBuilder(
		"icons/badge.png",
		Graphics::LINEAR_CLAMP,
		true, true, false).GetOrCreateTexture(m_renderer, "model");

	//load gun model for attachment test
	{
		Newmodel::Loader loader(m_renderer);
		try {
			Newmodel::NModel *m = loader.LoadModel("test_gun");
			m_gunModel.Reset(m);
			m_gunModelNode.Reset(new Newmodel::ModelNode(m_gunModel.Get()));
		} catch (Newmodel::LoadingError &) {
			AddLog("Could not load test_gun model");
		}
	}

	//some widgets
	animSlider = 0;

	onModelChanged.connect(sigc::mem_fun(*this, &ModelViewer::SetupUI));
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

	if (argc >= 3) {
		width = std::max(width, atoi(argv[1]));
		height = std::max(height, atoi(argv[2]));
	}
	const std::string modelName = argv[3];

	//init components
	FileSystem::Init();
	FileSystem::userFiles.MakeDirectory(""); // ensure the config directory exists
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		OS::Error("SDL initialization failed: %s\n", SDL_GetError());
	Lua::Init();

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
	Lua::Uninit();
	delete renderer;
	Graphics::Uninit();
	FileSystem::Uninit();
	SDL_Quit();
}

bool ModelViewer::OnAnimPlay(UI::Widget *w, bool reverse)
{
	Newmodel::Animation::Direction dir = reverse ? Newmodel::Animation::REVERSE : Newmodel::Animation::FORWARD;
	const std::string animname = animSelector->GetSelectedOption();
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

bool ModelViewer::OnAnimStop(UI::Widget *w)
{
    if (m_playing) AddLog("Animation stopped");
    m_playing = false;
    m_model->StopAnimations();
    return false;
}

bool ModelViewer::OnReloadModel(UI::Widget *w)
{
	//camera is not reset, it would be annoying when
	//tweaking materials
	SetModel(m_modelName, false);
	return true;
}

bool ModelViewer::OnToggleCollMesh(UI::CheckBox *w)
{
	m_options.showCollMesh = !m_options.showCollMesh;
	return m_options.showCollMesh;
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

bool ModelViewer::OnToggleGuns(UI::CheckBox *w)
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

void ModelViewer::AddLog(const std::string &line)
{
	m_log->AppendText(line+"\n");
	m_logScroller->SetScrollPosition(1.0f);
}

void ModelViewer::ChangeCameraPreset(SDLKey key, SDLMod mod)
{
	if (!m_model) return;

	//Blender is:
	//1 - front (+ctrl back)
	//7 - top (+ctrl bottom)
	//3 - right (+ctrl left)
	//2,4,6,8 incrementally rotate (+ctrl pan)

	const bool invert = mod & KMOD_CTRL;

	switch (key)
	{
	case SDLK_KP7:
		m_modelRot = matrix4x4f::RotateXMatrix(invert ? -M_PI/2 : M_PI/2);
		AddLog(invert ? "Bottom view" : "Top view");
		break;
	case SDLK_KP3:
		m_modelRot = matrix4x4f::RotateYMatrix(invert ? M_PI/2 : -M_PI/2);
		AddLog(invert ? "Left view" : "Right view");
		break;
	case SDLK_KP1:
		m_modelRot = matrix4x4f::RotateYMatrix(invert ? M_PI : 0.f);
		AddLog(invert ? "Rear view" : "Front view");
		break;
	case SDLK_KP4:
		m_modelRot = m_modelRot * matrix4x4f::RotateYMatrix(M_PI/12);
		break;
	case SDLK_KP6:
		m_modelRot = m_modelRot * matrix4x4f::RotateYMatrix(-M_PI/12);
		break;
	case SDLK_KP2:
		m_modelRot = m_modelRot * matrix4x4f::RotateXMatrix(-M_PI/12);
		break;
	case SDLK_KP8:
		m_modelRot = m_modelRot * matrix4x4f::RotateXMatrix(M_PI/12);
		break;
	default:
		break;
		//no others yet
	}
	m_camPos = vector3f(0.0f, 0.0f, m_model->GetDrawClipRadius() * 1.5f);
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

// Draw collision mesh as a wireframe overlay
void ModelViewer::DrawCollisionMesh()
{
	CollMesh *mesh = m_model->GetCollisionMesh();
	if(!mesh) return;

	std::vector<vector3f> &vertices = mesh->m_vertices;
	std::vector<int> &indices = mesh->m_indices;
	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE, indices.size() * 3);
	for(unsigned int i=0; i<indices.size(); i++) {
		va.Add(vertices.at(indices.at(i)), Color::WHITE);
	}

	//might want to add some offset
	m_renderer->SetWireFrameMode(true);
	Graphics::vtxColorMaterial->twoSided = true;
	m_renderer->DrawTriangles(&va, Graphics::vtxColorMaterial);
	Graphics::vtxColorMaterial->twoSided = false;
	m_renderer->SetWireFrameMode(false);
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

void ModelViewer::DrawModel()
{
	assert(m_model);
	m_renderer->SetBlendMode(Graphics::BLEND_SOLID);

	m_renderer->SetPerspectiveProjection(85, m_width/float(m_height), 0.1f, 1000.f);
	m_renderer->SetTransform(matrix4x4f::Identity());
	UpdateLights();

	matrix4x4f mv = matrix4x4f::Translation(-m_camPos) * m_modelRot.InverseOf();

	if (m_options.showGrid)
		DrawGrid(mv, m_model->GetDrawClipRadius());

	m_renderer->SetDepthTest(true);
	m_renderer->SetDepthWrite(true);

	m_model->UpdateAnimations(m_animTime);
	if (m_options.wireframe)
		m_renderer->SetWireFrameMode(true);
	m_model->Render(m_renderer, mv, &m_modelParams);
	if (m_options.wireframe)
		m_renderer->SetWireFrameMode(false);

	if (m_options.showCollMesh) {
		m_renderer->SetTransform(mv);
		DrawCollisionMesh();
	}
}

void ModelViewer::MainLoop()
{
	double lastTime = SDL_GetTicks() * 0.001;
	while (!m_done)
	{
		const double ticks = SDL_GetTicks() * 0.001;
		m_frameTime = (ticks - lastTime);
		if (m_playing) {
			m_animTime += (ticks - lastTime);
		}
		lastTime = ticks;

		m_renderer->ClearScreen();

		PollEvents();
		UpdateCamera();

		DrawBackground();

		//update animations, draw model etc.
		if (m_model)
			DrawModel();

		m_ui->Update();
		if (m_options.showUI && !m_screenshotQueued) {
			m_ui->Draw();
		}
		if (m_screenshotQueued) {
			m_screenshotQueued = false;
			Screenshot();
		}

		m_renderer->SwapBuffers();
	}
}

void ModelViewer::OnAnimChanged(unsigned int, const std::string &name)
{
	m_currentAnimation = 0;
	// Find the animation matching the name (could also store the anims in a map
	// when the animationSelector is filled)
	if (!name.empty()) {
		const std::vector<Newmodel::Animation*> &anims = m_model->GetAnimations();
		for (std::vector<Newmodel::Animation*>::const_iterator anim = anims.begin(); anim != anims.end(); ++anim) {
			if ((*anim)->GetName() == name)
				m_currentAnimation = (*anim);
		}
	}
	if (m_currentAnimation)
		animSlider->SetValue(m_currentAnimation->GetProgress());
	else
		animSlider->SetValue(0.0);
}

void ModelViewer::OnAnimSliderChanged(float value)
{
	if (m_currentAnimation)
		m_currentAnimation->SetProgress(value);
}

void ModelViewer::OnLightPresetChanged(unsigned int index, const std::string &)
{
	m_options.lightPreset = std::min<unsigned int>(index, 2);
}

void ModelViewer::OnModelColorsChanged(float)
{
	if (!m_model) return;
	//don't care about the float. Fetch values from all sliders.
	std::vector<Color4ub> colors;
	colors.push_back(get_slider_color(colorSliders[0], colorSliders[1], colorSliders[2]));
	colors.push_back(get_slider_color(colorSliders[3], colorSliders[4], colorSliders[5]));
	colors.push_back(get_slider_color(colorSliders[6], colorSliders[7], colorSliders[8]));
	m_model->SetColors(m_renderer, colors);
}

void ModelViewer::OnPatternChanged(unsigned int index, const std::string &value)
{
	if (!m_model) return;
	assert(index < m_model->GetPatterns().size());
	m_model->SetPattern(index);
}

void ModelViewer::OnThrustChanged(float)
{
	m_modelParams.linthrust[0] = get_thrust(thrustSliders[0]);
	m_modelParams.linthrust[1] = get_thrust(thrustSliders[1]);
	m_modelParams.linthrust[2] = get_thrust(thrustSliders[2]);

	// angthrusts are negated in ship.cpp for some reason
	m_modelParams.angthrust[0] = -get_thrust(thrustSliders[3]);
	m_modelParams.angthrust[1] = -get_thrust(thrustSliders[4]);
	m_modelParams.angthrust[2] = -get_thrust(thrustSliders[5]);
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
	 * printscr - screenshots
	 * tab - toggle ui (always invisible on screenshots)
	 * g - grid
	 *
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
				ResetThrusters();
				break;
			case SDLK_TAB:
				m_options.showUI = !m_options.showUI;
				break;
			case SDLK_PRINT:
				m_screenshotQueued = true;
				break;
			case SDLK_g:
				OnToggleGrid(0);
				break;
			case SDLK_z:
				m_options.wireframe = !m_options.wireframe;
				break;
			case SDLK_F11:
				if (event.key.keysym.mod & KMOD_SHIFT)
					m_renderer->ReloadShaders();
				break;
			case SDLK_KP1:
			case SDLK_KP2:
			case SDLK_KP3:
			case SDLK_KP4:
			case SDLK_KP6:
			case SDLK_KP7:
			case SDLK_KP8:
				ChangeCameraPreset(event.key.keysym.sym, event.key.keysym.mod);
				break;
			case SDLK_r: //random colors, eastereggish
				for(unsigned int i=0; i<3*3; i++) {
					if (colorSliders[i])
						colorSliders[i]->SetValue(m_rng.Double());
				}
				break;
			default:
				break; //shuts up -Wswitch
			} //keysym switch
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

void ModelViewer::ResetThrusters()
{
	if (thrustSliders[0] == 0) return;

	for (unsigned int i=0; i<6; i++) {
		thrustSliders[i]->SetValue(0.5f);
	}
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

void ModelViewer::SetModel(const std::string &filename, bool resetCamera /* true */)
{
	AddLog(stringf("Loading model %0...", filename));
	if (m_model) {
		delete m_model;
		m_model = 0;
	}

	try {
		m_modelName = filename;
		Newmodel::Loader loader(m_renderer);
		m_model = loader.LoadModel(filename);

		//set decal textures, max 4 supported.
		//Identical texture at the moment
		m_model->SetDecalTexture(m_decalTexture, 0);
		m_model->SetDecalTexture(m_decalTexture, 1);
		m_model->SetDecalTexture(m_decalTexture, 2);
		m_model->SetDecalTexture(m_decalTexture, 3);

		Newmodel::DumpVisitor d;
		m_model->GetRoot()->Accept(d);
		AddLog("Done.");
	} catch (Newmodel::LoadingError &err) {
		// report the error and show model picker.
		m_model = 0;
		AddLog(stringf("Could not load model %0: %1", filename, err.what()));
	}

	if (resetCamera)
		ResetCamera();

	onModelChanged.emit();
}

void ModelViewer::SetupUI()
{
	UI::Context *c = m_ui.Get();
	c->SetFont(UI::Widget::FONT_XSMALL);

	for (unsigned int i=0; i<9; i++)
		colorSliders[i] = 0;
	for (unsigned int i=0; i<6; i++)
		thrustSliders[i] = 0;

	//remove old floaters
	if (animSlider) {
		c->RemoveFloatingWidget(animSlider);
		animSlider = 0;
	}

	const float spacing = 5.f;
	UI::Button *reloadButton;
	UI::Button *toggleGridButton;
	UI::CheckBox *collMeshCheck;
	UI::CheckBox *gunsCheck;

	UI::VBox* outerBox = c->VBox();

	UI::VBox* mainBox = c->VBox();
	UI::HBox* sliderBox = c->HBox();

	outerBox->PackEnd(UI::WidgetSet(
		c->Expand()->SetInnerWidget(c->Grid(UI::CellSpec(0.75f,0.25f),1)
			->SetColumn(0, mainBox)
			->SetColumn(1, m_logScroller)
		),
		sliderBox
	));

	c->SetInnerWidget(c->Margin(spacing)->SetInnerWidget(outerBox));

	//model name + reload button: visible even if loading failed
	mainBox->PackEnd(nameLabel = c->Label(m_modelName));
	nameLabel->SetFont(UI::Widget::FONT_NORMAL);
	add_pair(c, mainBox, reloadButton = c->Button(), "Reload model");
	reloadButton->onClick.connect(sigc::bind(sigc::mem_fun(*this, &ModelViewer::OnReloadModel), reloadButton));

	if (m_model == 0) {
		c->Layout();
		return;
	}

	add_pair(c, mainBox, toggleGridButton = c->Button(), "Grid mode");
	add_pair(c, mainBox, collMeshCheck = c->CheckBox(), "Collision mesh");

	//pattern selector
	if (m_model->SupportsPatterns()) {
		mainBox->PackEnd(c->Label("Pattern:"));
		mainBox->PackEnd(patternSelector = c->DropDown()->AddOption("Default"));

		sliderBox->PackEnd(
			c->Grid(3,4)
				->SetColumn(0, UI::WidgetSet(
					c->Label("Color 1"),
					c->HBox(spacing)->PackEnd(c->Label("R"))->PackEnd(colorSliders[0] = c->HSlider()),
					c->HBox(spacing)->PackEnd(c->Label("G"))->PackEnd(colorSliders[1] = c->HSlider()),
					c->HBox(spacing)->PackEnd(c->Label("B"))->PackEnd(colorSliders[2] = c->HSlider())
				))
				->SetColumn(1, UI::WidgetSet(
					c->Label("Color 2"),
					c->HBox(spacing)->PackEnd(c->Label("R"))->PackEnd(colorSliders[3] = c->HSlider()),
					c->HBox(spacing)->PackEnd(c->Label("G"))->PackEnd(colorSliders[4] = c->HSlider()),
					c->HBox(spacing)->PackEnd(c->Label("B"))->PackEnd(colorSliders[5] = c->HSlider())
				))
				->SetColumn(2, UI::WidgetSet(
					c->Label("Color 3"),
					c->HBox(spacing)->PackEnd(c->Label("R"))->PackEnd(colorSliders[6] = c->HSlider()),
					c->HBox(spacing)->PackEnd(c->Label("G"))->PackEnd(colorSliders[7] = c->HSlider()),
					c->HBox(spacing)->PackEnd(c->Label("B"))->PackEnd(colorSliders[8] = c->HSlider())
				))
		);

		//connect slider signals, set initial values (RGB)
		const float values[] = {
			1.f, 0.f, 0.f,
			0.f, 1.f, 0.f,
			0.f, 0.f, 1.f
		};
		for(unsigned int i=0; i<3*3; i++) {
			colorSliders[i]->SetValue(values[i]);
			colorSliders[i]->onValueChanged.connect(sigc::mem_fun(*this, &ModelViewer::OnModelColorsChanged));
		}
		//// slidems end

		patternSelector->onOptionSelected.connect(sigc::mem_fun(*this, &ModelViewer::OnPatternChanged));

		UpdatePatternList();
	}

	//light dropdown
	UI::DropDown *lightSelector;
	mainBox->PackEnd(c->Label("Lights:"));
	mainBox->PackEnd(
		lightSelector = c->DropDown()
			->AddOption("1  Front white")
			->AddOption("2  Two-point")
			->AddOption("3  Backlight")
	);

	add_pair(c, mainBox, gunsCheck = c->CheckBox(), "Attach guns");

	//Animation controls
	if (!m_model->GetAnimations().empty()) {
		//UI::Button *playBtn;
		//UI::Button *revBtn;
		//UI::Button *stopBtn;
		UI::Box *animBox;
		mainBox->PackEnd(animBox = c->VBox(spacing));
		animBox->PackEnd(m_ui->Label("Animation:"));
		animBox->PackEnd(animSelector = m_ui->DropDown()->AddOption("None"));
		//add_pair(m_ui, animBox, playBtn = m_ui->Button(), "Play/Pause");
		//add_pair(m_ui, animBox, revBtn = m_ui->Button(), "Play reverse");
		//add_pair(m_ui, animBox, stopBtn = m_ui->Button(), "Stop");

		c->AddFloatingWidget(animSlider = c->HSlider(), UI::Point(spacing, m_height-300), UI::Point(200, 50));

		//playBtn->onClick.connect(sigc::bind(sigc::mem_fun(*this, &ModelViewer::OnAnimPlay), playBtn, false));
		//revBtn->onClick.connect(sigc::bind(sigc::mem_fun(*this, &ModelViewer::OnAnimPlay), revBtn, true));
		//stopBtn->onClick.connect(sigc::bind(sigc::mem_fun(*this, &ModelViewer::OnAnimStop), stopBtn));
		animSlider->onValueChanged.connect(sigc::mem_fun(*this, &ModelViewer::OnAnimSliderChanged));
		animSelector->onOptionSelected.connect(sigc::mem_fun(*this, &ModelViewer::OnAnimChanged));

		//update anims from model
		UpdateAnimList();
	}

	//// Thrust sliders
	bool supportsThrusters = false;
	{
		Newmodel::FindNodeVisitor fivi(Newmodel::FindNodeVisitor::MATCH_NAME_STARTSWITH, "thruster_");
		m_model->GetRoot()->Accept(fivi);
		supportsThrusters = !fivi.GetResults().empty();
	}
	if (supportsThrusters) {
		sliderBox->PackStart(
			c->Grid(2,4)
				->SetColumn(0, UI::WidgetSet(
					// Column 1, Linear thrust sliders
					c->Label("Linear"),
					c->HBox(spacing)->PackEnd(c->Label("X"))->PackEnd(thrustSliders[0] = c->HSlider()),
					c->HBox(spacing)->PackEnd(c->Label("Y"))->PackEnd(thrustSliders[1] = c->HSlider()),
					c->HBox(spacing)->PackEnd(c->Label("Z"))->PackEnd(thrustSliders[2] = c->HSlider())
				))
				->SetColumn(1, UI::WidgetSet(
					//Column 2, Angular thrust sliders
					c->Label("Angular"),
					c->HBox(spacing)->PackEnd(c->Label("Pitch"))->PackEnd(thrustSliders[3] = c->HSlider()),
					c->HBox(spacing)->PackEnd(c->Label("Yaw"))->PackEnd(thrustSliders[4] = c->HSlider()),
					c->HBox(spacing)->PackEnd(c->Label("Roll"))->PackEnd(thrustSliders[5] = c->HSlider())
				))
		);
		for(unsigned int i=0; i<2*3; i++) {
			thrustSliders[i]->SetValue(0.5f);
			thrustSliders[i]->onValueChanged.connect(sigc::mem_fun(*this, &ModelViewer::OnThrustChanged));
		}
		////thruster sliders end
	}

	c->Layout();

	//event handlers
	collMeshCheck->onClick.connect(sigc::bind(sigc::mem_fun(*this, &ModelViewer::OnToggleCollMesh), collMeshCheck));
	gunsCheck->onClick.connect(sigc::bind(sigc::mem_fun(*this, &ModelViewer::OnToggleGuns), gunsCheck));
	lightSelector->onOptionSelected.connect(sigc::mem_fun(*this, &ModelViewer::OnLightPresetChanged));
	toggleGridButton->onClick.connect(sigc::bind(sigc::mem_fun(*this, &ModelViewer::OnToggleGrid), toggleGridButton));
}

void ModelViewer::UpdateAnimList()
{
	animSelector->Clear();
	if (m_model) {
		const std::vector<Newmodel::Animation*> &anims = m_model->GetAnimations();
		for(unsigned int i=0; i<anims.size(); i++) {
			animSelector->AddOption(anims[i]->GetName());
		}
	}
	animSelector->Layout();
	OnAnimChanged(0, animSelector->GetSelectedOption());
}

void ModelViewer::UpdateCamera()
{
	float rate = 10.f * m_frameTime;
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

void ModelViewer::UpdatePatternList()
{
	patternSelector->Clear();

	if (m_model) {
		const Newmodel::PatternContainer &pats = m_model->GetPatterns();
		for(unsigned int i=0; i<pats.size(); i++) {
			patternSelector->AddOption(pats[i].name);
		}
	}

	m_ui->Layout();
}
