// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ModelViewer.h"
#include "FileSystem.h"
#include "GameConfig.h"
#include "GameSaveError.h"
#include "ModManager.h"
#include "PngWriter.h"
#include "SDL_keycode.h"
#include "StringF.h"
#include "core/Log.h"
#include "core/OS.h"
#include "graphics/Drawables.h"
#include "graphics/Graphics.h"
#include "graphics/Light.h"
#include "graphics/Material.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Types.h"
#include "graphics/VertexArray.h"
#include "graphics/opengl/RendererGL.h"
#include "lua/Lua.h"
#include "scenegraph/Animation.h"
#include "scenegraph/BinaryConverter.h"
#include "scenegraph/DumpVisitor.h"
#include "scenegraph/FindNodeVisitor.h"
#include "scenegraph/ModelSkin.h"

#include "imgui/imgui.h"

#include <iterator>

#include "Pi.h"
#include "scenegraph/Node.h"

//default options
ModelViewer::Options::Options() :
	attachGuns(false),
	showTags(false),
	showDockingLocators(false),
	showCollMesh(false),
	showAabb(false),
	showGeomBBox(false),
	showShields(false),
	showGrid(false),
	showVerticalGrids(false),
	showLandingPad(false),
	showUI(true),
	wireframe(false),
	mouselookEnabled(false),
	gridInterval(10.f),
	lightPreset(0),
	orthoView(false),
	metricsWindow(false)
{
}

//some utility functions
namespace {
	//azimuth/elevation in degrees to a dir vector
	vector3f az_el_to_dir(float yaw, float pitch)
	{
		//0,0 points to "right" (1,0,0)
		vector3f v;
		v.x = cos(DEG2RAD(yaw)) * cos(DEG2RAD(pitch));
		v.y = sin(DEG2RAD(pitch));
		v.z = sin(DEG2RAD(yaw)) * cos(DEG2RAD(pitch));
		return v;
	}

	float zoom_distance(const float base_distance, const float zoom)
	{
		return base_distance * powf(2.0f, zoom);
	}
} // namespace

namespace ImGui {
	bool ColorEdit3(const char *label, Color &color)
	{
		Color4f _c = color.ToColor4f();
		bool changed = ColorEdit3(label, &_c[0]);
		color = Color(_c);
		return changed;
	}
} // namespace ImGui

void ModelViewerApp::Startup()
{
	Application::Startup();
	Log::GetLog()->SetLogFile("output.txt");

	std::unique_ptr<IniConfig> config(new IniConfig);
	config->SetInt("ScrWidth", 1600);
	config->SetInt("ScrHeight", 900);
	config->SetInt("VSync", 1);
	config->SetInt("AntiAliasingMode", 4);

	Lua::Init();

	ModManager::Init();

	Graphics::RendererOGL::RegisterRenderer();

	auto *renderer = StartupRenderer(config.get());
	StartupInput(config.get());
	StartupPiGui();

	GetPiGui()->SetDebugStyle();
	// precache the editor font
	GetPiGui()->GetFont("pionillium", 13);

	NavLights::Init(renderer);
	Shields::Init(renderer);

	//run main loop until quit
	m_modelViewer.Reset(new ModelViewer(this, Lua::manager));
	if (!m_modelName.empty())
		m_modelViewer->SetModel(m_modelName);

	m_modelViewer->ResetCamera();

	QueueLifecycle(m_modelViewer);
}

void ModelViewerApp::Shutdown()
{
	//uninit components
	m_modelViewer.Reset();
	Lua::Uninit();
	Shields::Uninit();
	NavLights::Uninit();
	Graphics::Uninit();

	ShutdownPiGui();
	ShutdownRenderer();
	ShutdownInput();
	Application::Shutdown();
}

void ModelViewerApp::PreUpdate()
{
	HandleEvents();
	GetPiGui()->NewFrame();
}

void ModelViewerApp::PostUpdate()
{
	GetRenderer()->ClearDepthBuffer();
	GetPiGui()->Render();

	if (GetInput()->IsKeyReleased(SDLK_F12)) {
		GetRenderer()->FlushCommandBuffers();
		GetRenderer()->ReloadShaders();
	}
}

ModelViewer::ModelViewer(ModelViewerApp *app, LuaManager *lm) :
	m_input(app->GetInput()),
	m_pigui(app->GetPiGui()),
	m_bindings(m_input),
	m_logWindowSize(350.0f, 500.0f),
	m_animWindowSize(0.0f, 150.0f),
	m_colors({ Color(255, 0, 0),
		Color(0, 255, 0),
		Color(0, 0, 255) }),
	m_modelName(""),
	m_requestedModelName(),
	m_modelIsShip(false),
	m_screenshotQueued(false),
	m_shieldIsHit(false),
	m_shieldHitPan(-1.48f),
	m_renderer(app->GetRenderer()),
	m_decalTexture(0),
	m_rotX(0),
	m_rotY(0),
	m_zoom(0),
	m_baseDistance(100.0f),
	m_rng(time(0))
{
	onModelChanged.connect(sigc::mem_fun(*this, &ModelViewer::OnModelChanged));
	SetupAxes();

	Graphics::MaterialDescriptor desc;

	//for grid, background
	Graphics::RenderStateDesc rsd;
	rsd.depthWrite = false;
	rsd.cullMode = Graphics::CULL_NONE;
	rsd.primitiveType = Graphics::TRIANGLES;
	m_bgMaterial.reset(m_renderer->CreateMaterial("vtxColor", desc, rsd));

	m_gridLines.reset(new Graphics::Drawables::GridLines(m_renderer));
}

void ModelViewer::Start()
{
	UpdateModelList();
	UpdateDecalList();
}

void ModelViewer::End()
{
	ClearModel();
}

void ModelViewer::ReloadModel()
{
	AddLog(stringf("Reloading model %0", m_modelName));
	m_requestedModelName = m_modelName;
	m_resetLogScroll = true;
}

void ModelViewer::ToggleGuns()
{
	if (!m_gunModel) {
		CreateTestResources();
	}

	if (!m_gunModel) {
		AddLog("test_gun.model not available");
		return;
	}

	m_options.attachGuns = !m_options.attachGuns;
	SceneGraph::Model::TVecMT tags;
	m_model->FindTagsByStartOfName("tag_gun_", tags);
	m_model->FindTagsByStartOfName("tag_gunmount_", tags);
	if (tags.empty()) {
		AddLog("Missing tags \"tag_gun_XXX\" in model");
		return;
	}
	if (m_options.attachGuns) {
		for (auto tag : tags) {
			tag->AddChild(new SceneGraph::ModelNode(m_gunModel.get()));
		}
	} else { //detach
		//we know there's nothing else
		for (auto tag : tags) {
			tag->RemoveChildAt(0);
		}
	}
	return;
}

bool ModelViewer::SetRandomColor()
{
	if (!m_model || !m_model->SupportsPatterns()) return false;

	SceneGraph::ModelSkin skin;
	skin.SetRandomColors(m_rng);
	skin.Apply(m_model.get());

	m_colors = skin.GetColors();

	return true;
}

void ModelViewer::UpdateShield()
{
	if (m_shieldIsHit) {
		m_shieldHitPan += 0.05f;
	}
	if (m_shieldHitPan > 0.34f) {
		m_shieldHitPan = -1.48f;
		m_shieldIsHit = false;
	}
}

void ModelViewer::HitIt()
{
	if (m_model) {
		assert(m_shields.get());
		// pick a point on the shield to serve as the point of impact.
		SceneGraph::StaticGeometry *sg = m_shields->GetFirstShieldMesh();
		if (sg) {
			SceneGraph::StaticGeometry::Mesh &mesh = sg->GetMeshAt(0);

			// Please don't do this in game, no speed guarantee
			const Uint32 posOffs = mesh.vertexBuffer->GetDesc().GetOffset(Graphics::ATTRIB_POSITION);
			const Uint32 stride = mesh.vertexBuffer->GetDesc().stride;
			const Uint32 vtxIdx = m_rng.Int32() % mesh.vertexBuffer->GetSize();

			const Uint8 *vtxPtr = mesh.vertexBuffer->Map<Uint8>(Graphics::BUFFER_MAP_READ);
			const vector3f pos = *reinterpret_cast<const vector3f *>(vtxPtr + vtxIdx * stride + posOffs);
			mesh.vertexBuffer->Unmap();
			m_shields->AddHit(vector3d(pos));
		}
	}
	m_shieldHitPan = -1.48f;
	m_shieldIsHit = true;
}

void ModelViewer::AddLog(const std::string &line)
{
	m_log.push_back(line);
	Output("%s\n", line.c_str());
}

void ModelViewer::ChangeCameraPreset(CameraPreset preset)
{
	if (!m_model) return;

	switch (preset) {
	case CameraPreset::Bottom:
		m_rotX = -90.0f;
		m_rotY = 0.0f;
		break;
	case CameraPreset::Top:
		m_rotX = 90.0f;
		m_rotY = 0.0f;
		break;

	case CameraPreset::Left:
		m_rotX = 0.f;
		m_rotY = 90.0f;
		break;
	case CameraPreset::Right:
		m_rotX = 0.f;
		m_rotY = -90.0f;
		break;

	case CameraPreset::Front:
		m_rotX = 0.f;
		m_rotY = 180.0f;
		break;
	case CameraPreset::Back:
		m_rotX = 0.f;
		m_rotY = 0.0f;
		break;
	}
}

void ModelViewer::ToggleViewControlMode()
{
	m_options.mouselookEnabled = !m_options.mouselookEnabled;
	m_input->SetCapturingMouse(m_options.mouselookEnabled);

	if (m_options.mouselookEnabled) {
		m_viewRot = matrix3x3f::RotateY(DEG2RAD(m_rotY)) * matrix3x3f::RotateX(DEG2RAD(Clamp(m_rotX, -90.0f, 90.0f)));
		m_viewPos = zoom_distance(m_baseDistance, m_zoom) * m_viewRot.VectorZ();
	} else {
		// TODO: re-initialise the turntable style view position from the current mouselook view
		ResetCamera();
	}
}

void ModelViewer::ClearModel()
{
	m_model.reset();

	m_selectedTag = nullptr;
	m_animations.clear();
	m_currentAnimation = nullptr;
	m_patterns.clear();
	m_currentPattern = 0;
	m_currentDecal = 0;

	m_gunModel.reset();
	m_scaleModel.reset();

	m_options.attachGuns = false;
	m_options.mouselookEnabled = false;
	m_input->SetCapturingMouse(false);
	m_viewPos = vector3f(0.0f, 0.0f, 10.0f);
}

void ModelViewer::CreateTestResources()
{
	//load gun model for attachment test
	//landingpad model for scale test
	SceneGraph::Loader loader(m_renderer);
	try {
		SceneGraph::Model *m = loader.LoadModel("test_gun");
		m_gunModel.reset(m);

		m = loader.LoadModel("scale");
		m_scaleModel.reset(m);
	} catch (SceneGraph::LoadingError &) {
		AddLog("Could not load test_gun or scale model");
	}
}

void ModelViewer::DrawBackground()
{
	m_renderer->SetOrthographicProjection(0.f, 1.f, 0.f, 1.f, 0.f, 1.f);
	m_renderer->SetTransform(matrix4x4f::Identity());

	if (!m_bgMesh) {
		const Color top = Color::BLACK;
		const Color bottom = Color(28, 31, 36);
		Graphics::VertexArray bgArr(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE, 6);
		// triangle 1
		bgArr.Add(vector3f(0.f, 0.f, 0.f), bottom);
		bgArr.Add(vector3f(1.f, 0.f, 0.f), bottom);
		bgArr.Add(vector3f(1.f, 1.f, 0.f), top);
		// triangle 2
		bgArr.Add(vector3f(0.f, 0.f, 0.f), bottom);
		bgArr.Add(vector3f(1.f, 1.f, 0.f), top);
		bgArr.Add(vector3f(0.f, 1.f, 0.f), top);

		m_bgMesh.reset(m_renderer->CreateMeshObjectFromArray(&bgArr));
	}

	m_renderer->DrawMesh(m_bgMesh.get(), m_bgMaterial.get());
}

//Draw grid and axes
void ModelViewer::DrawGrid(const matrix4x4f &trans, float radius)
{
	assert(m_options.showGrid);

	// const float dist = zoom_distance(m_baseDistance, m_zoom);

	const float max = powf(10, ceilf(log10f(radius * 1.1)));

	m_renderer->SetTransform(trans);
	m_gridLines->Draw(m_renderer, { max, max }, m_options.gridInterval);

	if (m_options.showVerticalGrids) {
		m_renderer->SetTransform(trans * matrix4x4f::RotateXMatrix(M_PI * 0.5));
		m_gridLines->Draw(m_renderer, { max, max }, m_options.gridInterval);

		m_renderer->SetTransform(trans * matrix4x4f::RotateZMatrix(M_PI * 0.5));
		m_gridLines->Draw(m_renderer, { max, max }, m_options.gridInterval);
	}

	// industry-standard red/green/blue XYZ axis indicator
	m_renderer->SetTransform(trans * matrix4x4f::ScaleMatrix(radius));
	Graphics::Drawables::GetAxes3DDrawable(m_renderer)->Draw(m_renderer);
}

void ModelViewer::DrawModel(const matrix4x4f &mv)
{
	assert(m_model);

	m_model->UpdateAnimations();

	// this causes all debug visuals to be re-generated each frame, useful when scrubbing animations
	// also a good incentive to make your debug visuals *fast*
	m_model->SetDebugFlags(
		(m_options.showAabb ? SceneGraph::Model::DEBUG_BBOX : 0x0) |
		(m_options.showCollMesh ? SceneGraph::Model::DEBUG_COLLMESH : 0x0) |
		(m_options.showTags ? SceneGraph::Model::DEBUG_TAGS : 0x0) |
		(m_options.showDockingLocators ? SceneGraph::Model::DEBUG_DOCKING : 0x0) |
		(m_options.showGeomBBox ? SceneGraph::Model::DEBUG_GEOMBBOX : 0x0) |
		(m_options.wireframe ? SceneGraph::Model::DEBUG_WIREFRAME : 0x0));

	m_model->Render(mv);
	m_navLights->Render(m_renderer);
}

void ModelViewer::Update(float deltaTime)
{
	// if we've requested a different model then switch too it
	if (!m_requestedModelName.empty()) {
		SetModel(m_requestedModelName);
		m_requestedModelName.clear();
	}

	HandleInput();

	UpdateLights();
	UpdateCamera(deltaTime);
	UpdateShield();

	// render the gradient backdrop
	DrawBackground();

	//update animations, draw model etc.
	if (m_model) {
		m_navLights->Update(deltaTime);
		m_shields->SetEnabled(m_options.showShields || m_shieldIsHit);

		//Calculate the impact's radius dependent on time
		const float dif1 = 0.34 - (-1.48f);
		const float dif2 = m_shieldHitPan - (-1.48f);
		//Range from start (0.0) to end (1.0)
		const float dif = dif2 / (dif1 * 1.0f);

		m_shields->Update(m_options.showShields ? 1.0f : (1.0f - dif), 1.0f);

		// setup rendering
		if (!m_options.orthoView) {
			m_renderer->SetPerspectiveProjection(85, Graphics::GetScreenWidth() / float(Graphics::GetScreenHeight()), 0.1f, 100000.f);
		} else {
			/* TODO: Zoom in ortho mode seems don't work as in perspective mode,
				/ I change "screen dimensions" to avoid the problem.
				/ However the zoom needs more care
			*/
			if (m_zoom <= 0.0) m_zoom = 0.01;
			float screenW = Graphics::GetScreenWidth() * m_zoom / 10;
			float screenH = Graphics::GetScreenHeight() * m_zoom / 10;
			matrix4x4f orthoMat = matrix4x4f::OrthoMatrix(screenW, screenH, 0.1f, 100000.0f);
			m_renderer->SetProjection(orthoMat);
		}

		m_renderer->SetTransform(matrix4x4f::Identity());

		// calc camera info
		float zd = 0;
		if (m_options.mouselookEnabled) {
			m_modelViewMat = m_viewRot.Transpose() * matrix4x4f::Translation(-m_viewPos);
		} else {
			m_rotX = Clamp(m_rotX, -90.0f, 90.0f);
			matrix4x4f rot = matrix4x4f::Identity();
			rot.RotateX(DEG2RAD(-m_rotX));
			rot.RotateY(DEG2RAD(-m_rotY));
			if (m_options.orthoView)
				zd = -m_baseDistance;
			else
				zd = -zoom_distance(m_baseDistance, m_zoom);
			m_modelViewMat = matrix4x4f::Translation(0.0f, 0.0f, zd) * rot;
		}

		// draw the model itself
		DrawModel(m_modelViewMat);

		// helper rendering
		if (m_options.showLandingPad) {
			if (!m_scaleModel) CreateTestResources();
			m_scaleModel->Render(m_modelViewMat * matrix4x4f::Translation(0.f, m_landingMinOffset, 0.f));
		}

		if (m_options.showGrid) {
			DrawGrid(m_modelViewMat, m_model->GetDrawClipRadius());
		}
	}

	if (m_options.showUI && !m_screenshotQueued) {
		DrawPiGui();
	}
	if (m_screenshotQueued) {
		m_screenshotQueued = false;
		Screenshot();
	}
}

void ModelViewer::SetDecals(const std::string &texname)
{
	if (!m_model) return;

	m_decalTexture = Graphics::TextureBuilder::Decal(stringf("textures/decals/%0.dds", texname)).GetOrCreateTexture(m_renderer, "decal");

	m_model->SetDecalTexture(m_decalTexture, 0);
	m_model->SetDecalTexture(m_decalTexture, 1);
	m_model->SetDecalTexture(m_decalTexture, 2);
	m_model->SetDecalTexture(m_decalTexture, 3);
}

void ModelViewer::SetupAxes()
{
	auto *page = m_input->GetBindingPage("ModelViewer");
	auto *group = page->GetBindingGroup("View");

	// Don't add this to REGISTER_INPUT_BINDING because these bindings aren't used by the game
#define AXIS(val, name, axis, positive, negative)                                                \
	m_input->AddAxisBinding(name, group, InputBindings::Axis(axis, { positive }, { negative })); \
	m_bindings.val = m_bindings.AddAxis(name)

#define ACTION(val, name, b1, b2)                                                  \
	m_input->AddActionBinding(name, group, InputBindings::Action({ b1 }, { b2 })); \
	m_bindings.val = m_bindings.AddAction(name)

	AXIS(zoomAxis, "BindZoomAxis", {}, SDLK_EQUALS, SDLK_MINUS);

	AXIS(moveForward, "BindMoveForward", {}, SDLK_w, SDLK_s);
	AXIS(moveLeft, "BindMoveLeft", {}, SDLK_a, SDLK_d);
	AXIS(moveUp, "BindMoveUp", {}, SDLK_q, SDLK_e);

	// Like Blender, but a bit different because we like that
	// 1 - front (+ctrl back)
	// 7 - top (+ctrl bottom)
	// 3 - left (+ctrl right)
	// 2,4,6,8 incrementally rotate

	ACTION(viewFront, "BindViewFront", SDLK_KP_1, SDLK_m);
	m_bindings.viewFront->onPressed.connect([=]() {
		this->ChangeCameraPreset(m_input->KeyModState() & KMOD_CTRL ? CameraPreset::Back : CameraPreset::Front);
	});

	ACTION(viewLeft, "BindViewLeft", SDLK_KP_3, SDLK_PERIOD);
	m_bindings.viewLeft->onPressed.connect([=]() {
		this->ChangeCameraPreset(m_input->KeyModState() & KMOD_CTRL ? CameraPreset::Right : CameraPreset::Left);
	});

	ACTION(viewTop, "BindViewTop", SDLK_KP_7, SDLK_u);
	m_bindings.viewTop->onPressed.connect([=]() {
		this->ChangeCameraPreset(m_input->KeyModState() & KMOD_CTRL ? CameraPreset::Bottom : CameraPreset::Top);
	});

	AXIS(rotateViewLeft, "BindRotateViewLeft", {}, SDLK_KP_6, SDLK_KP_4);
	AXIS(rotateViewUp, "BindRotateViewUp", {}, SDLK_KP_8, SDLK_KP_2);

#undef AXIS
#undef ACTION

	m_input->AddInputFrame(&m_bindings);
}

void ModelViewer::HandleInput()
{
	// FIXME: better handle dispatching input to Action/Axis bindings

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
	 * o - switch orthographic<->perspective
	 *
	 */

	if (m_input->IsKeyPressed(SDLK_ESCAPE)) {
		if (m_model) {
			ClearModel();
			ResetCamera();
			UpdateModelList();
			UpdateDecalList();
		} else {
			RequestEndLifecycle();
		}
	}

	if (m_input->IsKeyPressed(SDLK_SPACE)) {
		ResetCamera();
		ResetThrusters();
	}

	if (m_input->IsKeyPressed(SDLK_TAB))
		m_options.showUI = !m_options.showUI;

	if (m_input->IsKeyPressed(SDLK_t))
		m_options.showTags = !m_options.showTags;

	if (m_input->IsKeyPressed(SDLK_PRINTSCREEN))
		m_screenshotQueued = true;

	if (m_input->IsKeyPressed(SDLK_o))
		m_options.orthoView = !m_options.orthoView;

	if (m_input->IsKeyPressed(SDLK_z))
		m_options.wireframe = !m_options.wireframe;

	if (m_input->IsKeyPressed(SDLK_f))
		ToggleViewControlMode();

	if (m_input->IsKeyPressed(SDLK_F6))
		SaveModelToBinary();

	if (m_input->IsKeyPressed(SDLK_F11) && m_input->KeyModState() & KMOD_SHIFT)
		m_renderer->ReloadShaders();

	//landing pad test
	if (m_input->IsKeyPressed(SDLK_p)) {
		m_options.showLandingPad = !m_options.showLandingPad;
		AddLog(stringf("Scale/landing pad test %0", m_options.showLandingPad ? "on" : "off"));
	}

	if (m_input->IsKeyPressed(SDLK_i)) {
		m_options.metricsWindow = !m_options.metricsWindow;
	}

	// random colors, eastereggish
	if (m_input->IsKeyPressed(SDLK_r))
		SetRandomColor();
}

void ModelViewer::UpdateModelList()
{
	m_fileNames.clear();

	const std::string basepath("models");
	FileSystem::FileSource &fileSource = FileSystem::gameDataFiles;
	for (FileSystem::FileEnumerator files(fileSource, basepath, FileSystem::FileEnumerator::Recurse); !files.Finished(); files.Next()) {
		const FileSystem::FileInfo &info = files.Current();
		const std::string &fpath = info.GetPath();

		//check it's the expected type
		if (info.IsFile()) {
			if (ends_with_ci(fpath, ".model"))
				m_fileNames.push_back(info.GetName().substr(0, info.GetName().size() - 6));
			else if (ends_with_ci(fpath, ".sgm"))
				m_fileNames.push_back(info.GetName());
		}
	}
}

void ModelViewer::UpdateDecalList()
{
	m_decals.clear();
	m_currentDecal = 0;

	const std::string basepath("textures/decals");
	FileSystem::FileSource &fileSource = FileSystem::gameDataFiles;
	for (FileSystem::FileEnumerator files(fileSource, basepath); !files.Finished(); files.Next()) {
		const FileSystem::FileInfo &info = files.Current();
		const std::string &fpath = info.GetPath();

		//check it's the expected type
		if (info.IsFile() && ends_with_ci(fpath, ".dds")) {
			m_decals.push_back(info.GetName().substr(0, info.GetName().size() - 4));
		}
	}
}

void ModelViewer::ResetCamera()
{
	m_baseDistance = m_model ? m_model->GetDrawClipRadius() * 1.5f : 100.f;
	m_rotX = m_rotY = 0.f;
	m_zoom = 0.f;
}

void ModelViewer::ResetThrusters()
{
	m_angularThrust = vector3f{};
	m_linearThrust = vector3f{};
}

void ModelViewer::Screenshot()
{
	char buf[256];
	const time_t t = time(0);
	const struct tm *_tm = localtime(&t);
	strftime(buf, sizeof(buf), "modelviewer-%Y%m%d-%H%M%S.png", _tm);
	Graphics::ScreendumpState sd;
	m_renderer->Screendump(sd);
	write_screenshot(sd, buf);
	AddLog(stringf("Screenshot %0 saved", buf));
}

void ModelViewer::SaveModelToBinary()
{
	if (!m_model)
		return AddLog("No current model to binarize");

	//load the current model in a pristine state (no navlights, shields...)
	//and then save it into binary

	std::unique_ptr<SceneGraph::Model> model;
	try {
		SceneGraph::Loader ld(m_renderer);
		model.reset(ld.LoadModel(m_modelName));
	} catch (...) {
		//minimal error handling, this is not expected to happen since we got this far.
		AddLog("Could not load model");
		return;
	}

	try {
		SceneGraph::BinaryConverter bc(m_renderer);
		bc.Save(m_modelName, model.get());
		AddLog("Saved binary model file");
	} catch (const CouldNotOpenFileException &) {
		AddLog("Could not open file or directory for writing");
	} catch (const CouldNotWriteToFileException &) {
		AddLog("Error while writing to file");
	}
}

void ModelViewer::SetModel(const std::string &filename)
{
	AddLog(stringf("Loading model %0...", filename));

	//this is necessary to reload textures
	m_renderer->RemoveAllCachedTextures();

	ClearModel();

	try {
		if (ends_with_ci(filename, ".sgm")) {
			//binary loader expects extension-less name. Might want to change this.
			m_modelName = filename.substr(0, filename.size() - 4);
			SceneGraph::BinaryConverter bc(m_renderer);
			m_model.reset(bc.Load(m_modelName));
		} else {
			m_modelName = filename;
			SceneGraph::Loader loader(m_renderer, true, false);
			m_model.reset(loader.LoadModel(filename));

			//dump warnings
			for (std::vector<std::string>::const_iterator it = loader.GetLogMessages().begin();
				 it != loader.GetLogMessages().end(); ++it) {
				AddLog(*it);
				Output("%s\n", (*it).c_str());
			}
		}

		Shields::ReparentShieldNodes(m_model.get());

		//set decal textures, max 4 supported.
		//Identical texture at the moment
		SetDecals("pioneer");
		Output("\n\n");

		SceneGraph::DumpVisitor d(m_model.get());
		m_model->GetRoot()->Accept(d);
		AddLog(d.GetModelStatistics());

		// If we've got the tag_landing set then use it for an offset otherwise grab the AABB
		const SceneGraph::MatrixTransform *mt = m_model->FindTagByName("tag_landing");
		if (mt)
			m_landingMinOffset = mt->GetTransform().GetTranslate().y;
		else if (m_model->GetCollisionMesh())
			m_landingMinOffset = m_model->GetCollisionMesh()->GetAabb().min.y;
		else
			m_landingMinOffset = 0.0f;

		//note: stations won't demonstrate full docking light logic in MV
		m_navLights.reset(new NavLights(m_model.get()));
		m_navLights->SetEnabled(true);

		m_shields.reset(new Shields(m_model.get()));
	} catch (SceneGraph::LoadingError &err) {
		// report the error and show model picker.
		m_model.reset();
		AddLog(stringf("Could not load model %0: %1", filename, err.what()));
	}

	if (m_model)
		onModelChanged.emit();
}

void ModelViewer::OnModelChanged()
{
	ResetCamera();
	ResetThrusters();
	m_model->SetColors(m_colors);

	SceneGraph::FindNodeVisitor visitor(SceneGraph::FindNodeVisitor::MATCH_NAME_STARTSWITH, "thruster_");
	m_model->GetRoot()->Accept(visitor);
	m_modelIsShip = !visitor.GetResults().empty();
	m_modelSupportsDecals = m_model->SupportsDecals();

	m_modelHasShields = m_shields.get() && m_shields->GetFirstShieldMesh();

	m_animations = m_model->GetAnimations();
	m_currentAnimation = m_animations.size() ? m_animations.front() : nullptr;
	if (m_currentAnimation)
		m_model->SetAnimationActive(0, true);

	m_patterns.clear();
	m_currentPattern = 0;
	m_modelSupportsPatterns = m_model->SupportsPatterns();
	if (m_modelSupportsPatterns) {
		for (const auto &pattern : m_model->GetPatterns()) {
			m_patterns.push_back(pattern.name);
		}
	}
}

void ModelViewer::DrawModelSelector()
{
	vector2f selectorSize = m_windowSize * vector2f(0.4, 0.8);
	ImGui::SetNextWindowSize({ selectorSize.x, selectorSize.y }, ImGuiCond_Always);
	vector2f selectorPos = m_windowSize * 0.5 - selectorSize * 0.5;
	ImGui::SetNextWindowPos({ selectorPos.x, selectorPos.y }, ImGuiCond_Always);

	auto flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
	bool b_open = true; // Use the window close button to quit the modelviewer
	if (ImGui::Begin("Select Model", &b_open, flags)) {
		if (ImGui::BeginChild("FileList", ImVec2(0.0, -ImGui::GetFrameHeightWithSpacing() * 10.0f))) {
			for (const auto &name : m_fileNames) {
				if (ImGui::Selectable(name.c_str())) {
					m_requestedModelName = name;
				}
			}
		}
		ImGui::EndChild();
		ImGui::Spacing();
		ImGui::Text("Log:");
		if (ImGui::BeginChild("Log")) {
			DrawLog();
		}
		ImGui::EndChild();
	}
	ImGui::End();

	if (!b_open || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
		RequestEndLifecycle();
	}
}

void ModelViewer::DrawModelTags()
{
	const uint32_t numTags = m_model->GetNumTags();
	if (!numTags) return;

	for (uint32_t i = 0; i < numTags; i++) {
		auto *tag = m_model->GetTagByIndex(i);
		if (ImGui::Selectable(tag->GetName().c_str(), tag == m_selectedTag)) {
			m_selectedTag = tag;
		}
	}
}

void ModelViewer::DrawTagNames()
{
	if (!m_selectedTag) return;
	auto size = ImGui::GetWindowSize();
	m_renderer->SetTransform(matrix4x4f::Identity());

	vector3f point = m_modelViewMat * m_selectedTag->GetTransform().GetTranslate();
	point = Graphics::ProjectToScreen(m_renderer, point);
	ImGui::SetCursorPos({ point.x + 8.0f, size.y - point.y });
	ImGui::TextUnformatted(m_selectedTag->GetName().c_str());

	// ImGui::SetCursorPos({ 0.5f * size.x, 0.5f * size.y });
	// ImGui::Text("%f %f %f", point.x, point.y, point.z);
}

struct NodeHierarchyVisitor : SceneGraph::NodeVisitor {
	void DisplayNode(SceneGraph::Node &node, std::string_view nodeType)
	{
		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		if (nodeType.empty()) {
			ImGui::TreeNodeEx(node.GetName().c_str(), flags);
		} else {
			std::string name = fmt::format("[{}] {}", nodeType, node.GetName());
			ImGui::TreeNodeEx(name.c_str(), flags);
		}
	}

	void DisplayGroup(SceneGraph::Group &node, std::string_view nodeType)
	{
		std::string name = fmt::format("[{}] {}", nodeType, node.GetName());
		if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			node.Traverse(*this);

			ImGui::TreePop();
		}
	}

	void ApplyNode(SceneGraph::Node &node) override
	{
		DisplayNode(node, "");
	}

	void ApplyGroup(SceneGraph::Group &node) override
	{
		DisplayGroup(node, "Group");
	}

	void ApplyMatrixTransform(SceneGraph::MatrixTransform &m) override
	{
		DisplayGroup(m, "MT");
	}

	void ApplyLOD(SceneGraph::LOD &l) override
	{
		DisplayGroup(l, "LOD");
	}

	void ApplyCollisionGeometry(SceneGraph::CollisionGeometry &cg) override
	{
		DisplayNode(cg, "CG");
	}
};

void ModelViewer::DrawModelHierarchy()
{
	NodeHierarchyVisitor v = {};
	m_model->GetRoot()->Accept(v);
}

void ModelViewer::DrawShipControls()
{
	bool valuesChanged = false;
	if (m_modelIsShip) {
		ImGui::TextUnformatted("Linear Thrust");
		ImGui::Spacing();

		valuesChanged |= ImGui::SliderFloat("X", &m_linearThrust.x, -1.0, 1.0);
		valuesChanged |= ImGui::SliderFloat("Y", &m_linearThrust.y, -1.0, 1.0);
		valuesChanged |= ImGui::SliderFloat("Z", &m_linearThrust.z, -1.0, 1.0);

		ImGui::Spacing();
		ImGui::TextUnformatted("Angular Thrust");
		ImGui::Spacing();

		valuesChanged |= ImGui::SliderFloat("Pitch", &m_angularThrust.x, -1.0, 1.0);
		valuesChanged |= ImGui::SliderFloat("Yaw", &m_angularThrust.y, -1.0, 1.0);
		valuesChanged |= ImGui::SliderFloat("Roll", &m_angularThrust.z, -1.0, 1.0);

		if (valuesChanged)
			m_model->SetThrust(m_linearThrust, m_angularThrust);

		ImGui::Spacing();
	}

	ImGui::TextUnformatted("Pattern Colors");
	ImGui::Spacing();

	if (ImGui::Button("Set Random Colors", ImVec2(-1.f, 0.f)))
		SetRandomColor();

	valuesChanged = false;
	valuesChanged |= ImGui::ColorEdit3("Color 1", m_colors[0]);
	valuesChanged |= ImGui::ColorEdit3("Color 2", m_colors[1]);
	valuesChanged |= ImGui::ColorEdit3("Color 3", m_colors[2]);

	if (valuesChanged)
		m_model->SetColors(m_colors);

	if (m_currentAnimation) {
		ImGui::Spacing();
		ImGui::TextUnformatted("Animation Progress");
		float progress = m_currentAnimation->GetProgress();
		ImGui::SetNextItemWidth(-1.0f);
		if (ImGui::SliderFloat("##anim-progress", &progress, 0.0, 1.0))
			m_currentAnimation->SetProgress(progress);
	}
}

void ModelViewer::DrawModelOptions()
{
	float itmWidth = ImGui::CalcItemWidth();

	ImGui::PushFont(m_pigui->GetFont("pionillium", 14));
	ImGui::AlignTextToFramePadding();
	ImGui::TextUnformatted(m_modelName.c_str());
	ImGui::PopFont();

	ImGui::SameLine();
	if (ImGui::Button("Reload Model"))
		ReloadModel();

	ImGui::Checkbox("Show Scale Model", &m_options.showLandingPad);
	ImGui::Checkbox("Show Collision Mesh", &m_options.showCollMesh);
	m_options.showAabb = m_options.showCollMesh;
	ImGui::Checkbox("Show Geometry Bounds", &m_options.showGeomBBox);
	ImGui::Checkbox("Show Tags", &m_options.showTags);
	m_options.showDockingLocators = m_options.showTags;

	ImGui::Spacing();

	ImGui::Checkbox("##ToggleGrid", &m_options.showGrid);
	ImGui::SameLine(0.0, 4.0f);
	ImGui::SetNextItemWidth(itmWidth - 4.0f - ImGui::GetFrameHeight());

	std::string currentGridMode = std::to_string(int(m_options.gridInterval)) + "x";
	if (ImGui::BeginCombo("Grid Mode", currentGridMode.c_str())) {
		if (ImGui::Selectable("1m"))
			m_options.gridInterval = 1.0f;

		if (ImGui::Selectable("10m"))
			m_options.gridInterval = 10.0f;

		if (ImGui::Selectable("100m"))
			m_options.gridInterval = 100.0f;

		if (ImGui::Selectable("1000m"))
			m_options.gridInterval = 1000.0f;

		ImGui::EndCombo();
	}

	if (m_options.showGrid) {
		ImGui::Checkbox("Show Vertical Grids", &m_options.showVerticalGrids);
	}

	if (m_modelHasShields) {
		ImGui::Spacing();

		ImGui::Checkbox("Show Shields", &m_options.showShields);
		if (ImGui::Button("Test Shield Hit"))
			HitIt();
	}

	if (m_modelIsShip) {
		if (ImGui::Button("Attach Test Guns"))
			ToggleGuns();
	}

	ImGui::Spacing();

	if (m_modelSupportsPatterns) {
		const char *preview_name = m_patterns[m_currentPattern].c_str();
		if (ImGui::BeginCombo("Pattern", preview_name)) {
			for (size_t idx = 0; idx < m_patterns.size(); idx++) {
				const bool selected = m_currentPattern == idx;
				if (ImGui::Selectable(m_patterns[idx].c_str(), selected) && !selected) {
					m_currentPattern = idx;
					m_model->SetPattern(idx);
				}
			}

			ImGui::EndCombo();
		}
	}

	if (m_modelSupportsDecals) {
		const char *preview_name = m_decals[m_currentDecal].c_str();
		if (ImGui::BeginCombo("Decals", preview_name)) {
			for (size_t idx = 0; idx < m_decals.size(); idx++) {
				const bool selected = m_currentDecal == idx;
				if (ImGui::Selectable(m_decals[idx].c_str(), selected) && !selected) {
					m_currentDecal = idx;
					SetDecals(m_decals[idx]);
				}
			}

			ImGui::EndCombo();
		}
	}

	if (!m_animations.empty()) {
		if (ImGui::BeginCombo("Animation", m_currentAnimation->GetName().c_str())) {
			for (const auto anim : m_animations) {
				const bool selected = m_currentAnimation == anim;
				if (ImGui::Selectable(anim->GetName().c_str(), selected) && !selected) {
					// selected a new animation entry
					m_model->SetAnimationActive(m_model->FindAnimationIndex(m_currentAnimation), false);
					m_model->SetAnimationActive(m_model->FindAnimationIndex(anim), true);
					m_currentAnimation = anim;
				}
			}

			ImGui::EndCombo();
		}
	}

	static std::vector<std::string> lightSetups = {
		"Front Light", "Two-point", "Backlight"
	};

	uint32_t &currentLights = m_options.lightPreset;
	if (ImGui::BeginCombo("Lights", lightSetups[currentLights].c_str())) {
		for (size_t idx = 0; idx < lightSetups.size(); idx++) {
			const bool selected = currentLights == idx;
			if (ImGui::Selectable(lightSetups[idx].c_str(), selected) && !selected) {
				currentLights = idx;
			}
		}

		ImGui::EndCombo();
	}
}

void ModelViewer::DrawLog()
{
	if (ImGui::BeginChild("ScrollArea")) {
		for (const auto &message : m_log) {
			ImGui::TextWrapped("%s", message.c_str());
		}

		if (m_resetLogScroll || ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
			ImGui::SetScrollHereY(1.0f);
			m_resetLogScroll = false;
		}
	}
	ImGui::EndChild();
}

constexpr ImGuiWindowFlags fullscreenFlags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBringToFrontOnFocus;
constexpr ImGuiWindowFlags tabWindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

void ModelViewer::DrawPiGui()
{
	m_windowSize = vector2f(Graphics::GetScreenWidth(), Graphics::GetScreenHeight());
	float leftWidth = m_windowSize.x / 5.0;
	float rightWidth = m_windowSize.x / 5.0;
	float leftDiv = m_windowSize.y / 1.8;
	float rightDiv = m_windowSize.y / 1.6;

	if (!m_model) {
		DrawModelSelector();
		return;
	}

	ImGui::PushFont(m_pigui->GetFont("pionillium", 13));

	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ m_windowSize.x, m_windowSize.y });
	ImGui::Begin("##background-display", nullptr, fullscreenFlags);
	DrawTagNames();
	ImGui::End();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0);
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ leftWidth, leftDiv });
	if (ImGui::Begin("##window-topleft", nullptr, tabWindowFlags)) {
		DrawModelOptions();
	}
	ImGui::End();

	ImGui::SetNextWindowPos({ 0, leftDiv });
	ImGui::SetNextWindowSize({ leftWidth, m_windowSize.y - leftDiv });
	if (ImGui::Begin("##window-bottomleft", nullptr, tabWindowFlags)) {
		DrawShipControls();
	}
	ImGui::End();

	ImGui::SetNextWindowPos({ m_windowSize.x - rightWidth, 0 });
	ImGui::SetNextWindowSize({ rightWidth, rightDiv });
	if (ImGui::Begin("##window-topright", nullptr, tabWindowFlags)) {
		if (ImGui::BeginTabBar("##tabbar-topright")) {
			if (ImGui::BeginTabItem("Tags")) {
				DrawModelTags();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Hierarchy")) {
				DrawModelHierarchy();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();

	ImGui::SetNextWindowPos({ m_windowSize.x - rightWidth, rightDiv });
	ImGui::SetNextWindowSize({ rightWidth, m_windowSize.y - rightDiv });
	if (ImGui::Begin("##window-bottomright", nullptr, tabWindowFlags)) {
		DrawLog();
	}
	ImGui::End();
	ImGui::PopStyleVar(1);

	if (m_options.metricsWindow)
		ImGui::ShowDemoWindow();

	ImGui::PopFont();
}

void ModelViewer::UpdateCamera(float deltaTime)
{
	static const float BASE_ZOOM_RATE = 1.0f / 12.0f;
	float zoomRate = (BASE_ZOOM_RATE * 8.0f) * deltaTime;
	float rotateRate = 25.f * deltaTime;
	float moveRate = 10.0f * deltaTime;

	bool isShiftPressed = m_input->KeyState(SDLK_LSHIFT);

	if (isShiftPressed) {
		zoomRate *= 8.0f;
		moveRate *= 4.0f;
		rotateRate *= 4.0f;
	}

	std::array<int, 2> mouseMotion;
	m_input->GetMouseMotion(mouseMotion.data());
	bool rightMouseDown = m_input->MouseButtonState(SDL_BUTTON_RIGHT);
	if (m_options.mouselookEnabled) {
		const float degrees_per_pixel = 0.2f;
		if (!rightMouseDown) {
			// yaw and pitch
			const float rot_y = degrees_per_pixel * mouseMotion[0];
			const float rot_x = degrees_per_pixel * mouseMotion[1];
			const matrix3x3f rot =
				matrix3x3f::RotateX(DEG2RAD(rot_x)) *
				matrix3x3f::RotateY(DEG2RAD(rot_y));

			m_viewRot = m_viewRot * rot;
		} else {
			// roll
			m_viewRot = m_viewRot * matrix3x3f::RotateZ(DEG2RAD(degrees_per_pixel * mouseMotion[0]));
		}

		vector3f motion(
			m_bindings.moveLeft->GetValue(),
			m_bindings.moveUp->GetValue(),
			m_bindings.moveForward->GetValue());

		m_viewPos += m_viewRot * motion;
	} else {
		//zoom
		m_zoom += m_bindings.zoomAxis->GetValue() * BASE_ZOOM_RATE;

		//zoom with mouse wheel
		int mouseWheel = m_input->GetMouseWheel();
		if (mouseWheel) m_zoom += mouseWheel > 0 ? -BASE_ZOOM_RATE : BASE_ZOOM_RATE;

		m_zoom = Clamp(m_zoom, -10.0f, 10.0f); // distance range: [baseDistance * 1/1024, baseDistance * 1024]

		//rotate

		if (m_input->IsKeyDown(SDLK_UP)) m_rotX += rotateRate;
		if (m_input->IsKeyDown(SDLK_DOWN)) m_rotX -= rotateRate;
		if (m_input->IsKeyDown(SDLK_LEFT)) m_rotY += rotateRate;
		if (m_input->IsKeyDown(SDLK_RIGHT)) m_rotY -= rotateRate;

		m_rotX += rotateRate * m_bindings.rotateViewLeft->GetValue();
		m_rotY += rotateRate * -m_bindings.rotateViewUp->GetValue();

		//mouse rotate when right button held
		if (rightMouseDown) {
			m_rotY += 0.2f * mouseMotion[0];
			m_rotX += 0.2f * mouseMotion[1];
		}
	}
}

void ModelViewer::UpdateLights()
{
	using Graphics::Light;
	std::vector<Light> lights;

	switch (m_options.lightPreset) {
	case 0:
		//Front white
		lights.push_back(Light(Light::LIGHT_DIRECTIONAL, az_el_to_dir(90, 0), Color::WHITE, Color::WHITE));
		lights.push_back(Light(Light::LIGHT_DIRECTIONAL, az_el_to_dir(0, -90), Color(13, 13, 26), Color::WHITE));
		break;
	case 1:
		//Two-point
		lights.push_back(Light(Light::LIGHT_DIRECTIONAL, az_el_to_dir(120, 0), Color(230, 204, 204), Color::WHITE));
		lights.push_back(Light(Light::LIGHT_DIRECTIONAL, az_el_to_dir(-30, -90), Color(178, 128, 0), Color::WHITE));
		break;
	case 2:
		//Backlight
		lights.push_back(Light(Light::LIGHT_DIRECTIONAL, az_el_to_dir(-75, 20), Color::WHITE, Color::WHITE));
		lights.push_back(Light(Light::LIGHT_DIRECTIONAL, az_el_to_dir(0, -90), Color(13, 13, 26), Color::WHITE));
		break;
	case 3:
		//4 lights
		lights.push_back(Light(Light::LIGHT_DIRECTIONAL, az_el_to_dir(0, 90), Color::YELLOW, Color::WHITE));
		lights.push_back(Light(Light::LIGHT_DIRECTIONAL, az_el_to_dir(0, -90), Color::GREEN, Color::WHITE));
		lights.push_back(Light(Light::LIGHT_DIRECTIONAL, az_el_to_dir(0, 45), Color::BLUE, Color::WHITE));
		lights.push_back(Light(Light::LIGHT_DIRECTIONAL, az_el_to_dir(0, -45), Color::WHITE, Color::WHITE));
		break;
	};

	m_renderer->SetLights(int(lights.size()), &lights[0]);
}
