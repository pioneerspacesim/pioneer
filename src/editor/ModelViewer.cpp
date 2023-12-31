// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ModelViewer.h"

#include "FileSystem.h"
#include "GameSaveError.h"
#include "NavLights.h"
#include "PngWriter.h"
#include "Random.h"
#include "SDL_keycode.h"

#include "core/Log.h"

#include "editor/EditorApp.h"
#include "editor/ModelViewerWidget.h"
#include "editor/EditorDraw.h"

#include "scenegraph/BinaryConverter.h"
#include "scenegraph/DumpVisitor.h"
#include "scenegraph/FindNodeVisitor.h"
#include "scenegraph/SceneGraph.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

using namespace Editor;

namespace {
	static constexpr const char *SELECTOR_WND_NAME = "Select Model";
	static constexpr const char *TAGS_WND_NAME = "Tags";
	static constexpr const char *HIERARCHY_WND_NAME = "Hierarchy";
	static constexpr const char *LOG_WND_NAME = "Log";
}

ModelViewer::ModelViewer(EditorApp *app, LuaManager *lm) :
	m_app(app),
	m_input(app->GetInput()),
	m_pigui(app->GetPiGui()),
	m_renderer(app->GetRenderer()),
	m_modelName(""),
	m_requestedModelName(),
	m_decalTexture(0),
	m_shieldHitPan(-1.48f)
{
	m_modelWindow.reset(new ModelViewerWidget(app));

	m_modelWindow->GetUIExtOverlay().connect(sigc::mem_fun(this, &ModelViewer::DrawTagNames));
	m_modelWindow->GetUIExtMenu().connect(sigc::mem_fun(this, &ModelViewer::DrawShipControls));

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

ModelViewer::~ModelViewer()
{
}

void ModelViewer::Start()
{
	NavLights::Init(m_renderer);
	Shields::Init(m_renderer);

	UpdateModelList();
	UpdateDecalList();

	Log::GetLog()->printCallback.connect(sigc::mem_fun(this, &ModelViewer::AddLog));

	m_modelWindow->OnAppearing();
}

void ModelViewer::End()
{
	ClearModel();

	Shields::Uninit();
	NavLights::Uninit();
}

void ModelViewer::ReloadModel()
{
	Log::Info("Reloading model {}", m_modelName);
	m_requestedModelName = m_modelName;
	m_resetLogScroll = true;
}

void ModelViewer::ToggleGuns()
{
	if (!m_gunModel) {
		CreateTestResources();
	}

	if (!m_gunModel) {
		Log::Info("test_gun.model not available");
		return;
	}

	m_attachGuns = !m_attachGuns;
	std::vector<SceneGraph::Tag *> tags;

	SceneGraph::Model *model = m_modelWindow->GetModel();
	model->FindTagsByStartOfName("tag_gun_", tags);
	model->FindTagsByStartOfName("tag_gunmount_", tags);
	if (tags.empty()) {
		Log::Info("Missing tags \"tag_gun_XXX\" in model");
		return;
	}

	if (m_attachGuns) {
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

void ModelViewer::UpdateShield()
{
	if (m_shieldIsHit) {
		m_shieldHitPan += 0.05f;
	}
	if (m_shieldHitPan > 0.34f) {
		m_shieldHitPan = -1.48f;
		m_shieldIsHit = false;
	}

	if (m_modelWindow->GetModel()) {
		m_shields->SetEnabled(m_showShields || m_shieldIsHit);

		//Calculate the impact's radius dependent on time
		const float dif1 = 0.34 - (-1.48f);
		const float dif2 = m_shieldHitPan - (-1.48f);
		//Range from start (0.0) to end (1.0)
		const float dif = dif2 / (dif1 * 1.0f);

		m_shields->Update(m_showShields ? 1.0f : (1.0f - dif), 1.0f);
	}
}

void ModelViewer::HitIt()
{
	if (m_modelWindow->GetModel()) {
		assert(m_shields.get());

		// pick a point on the shield to serve as the point of impact.
		SceneGraph::StaticGeometry *sg = m_shields->GetFirstShieldMesh();
		if (sg) {
			SceneGraph::StaticGeometry::Mesh &mesh = sg->GetMeshAt(0);

			Random rng(uint32_t(m_app->GetTime()));

			// Please don't do this in game, no speed guarantee
			const Uint32 posOffs = mesh.vertexBuffer->GetDesc().GetOffset(Graphics::ATTRIB_POSITION);
			const Uint32 stride = mesh.vertexBuffer->GetDesc().stride;
			const Uint32 vtxIdx = rng.Int32() % mesh.vertexBuffer->GetSize();

			const Uint8 *vtxPtr = mesh.vertexBuffer->Map<Uint8>(Graphics::BUFFER_MAP_READ);
			const vector3f pos = *reinterpret_cast<const vector3f *>(vtxPtr + vtxIdx * stride + posOffs);
			mesh.vertexBuffer->Unmap();
			m_shields->AddHit(vector3d(pos));
		}
	}
	m_shieldHitPan = -1.48f;
	m_shieldIsHit = true;
}

void ModelViewer::AddLog(Time::DateTime, Log::Severity sv, std::string_view line)
{
	if (sv < Log::Severity::Verbose)
		m_log.push_back(std::string(line));
}

void ModelViewer::ClearModel()
{
	m_shields.reset();
	m_modelWindow->ClearModel();
	m_modelName.clear();

	m_selectedTag = nullptr;
	m_currentDecal = 0;

	m_gunModel.reset();

	m_showShields = false;
	m_attachGuns = false;

	m_modelWindow->GetOptions().mouselookEnabled = false;
	m_input->SetCapturingMouse(false);
}

void ModelViewer::CreateTestResources()
{
	//load gun model for attachment test
	SceneGraph::Loader loader(m_renderer);
	try {
		SceneGraph::Model *m = loader.LoadModel("test_gun");
		m_gunModel.reset(m);
	} catch (SceneGraph::LoadingError &) {
		Log::Warning("Could not load test_gun model");
	}
}

void ModelViewer::Update(float deltaTime)
{
	// if we've requested a different model then switch too it
	if (!m_requestedModelName.empty()) {
		SetModel(m_requestedModelName);
		m_requestedModelName.clear();
	}

	HandleInput();

	UpdateShield();

	DrawPiGui();

	m_modelWindow->Update(deltaTime);

	if (m_screenshotQueued) {
		m_screenshotQueued = false;
		Screenshot();
	}
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
		if (m_modelWindow->GetModel()) {
			ClearModel();
			UpdateModelList();
			UpdateDecalList();
		} else {
			RequestEndLifecycle();
		}
	}

	if (m_input->IsKeyPressed(SDLK_SPACE)) {
		ResetThrusters();
	}

	if (m_input->IsKeyPressed(SDLK_TAB)) {
		m_showUI = !m_showUI;
		m_modelWindow->GetOptions().hideUI = !m_showUI;
	}

	if (m_input->IsKeyPressed(SDLK_PRINTSCREEN))
		m_screenshotQueued = true;

	if (m_input->IsKeyPressed(SDLK_F6))
		SaveModelToBinary();

	if (m_input->IsKeyPressed(SDLK_F11) && m_input->KeyModState() & KMOD_SHIFT)
		m_renderer->ReloadShaders();

	if (m_input->IsKeyPressed(SDLK_i)) {
		m_metricsWindow = !m_metricsWindow;
	}
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
	Log::Verbose("Screenshot {} saved", buf);
}

void ModelViewer::SaveModelToBinary()
{
	if (!m_modelWindow->GetModel())
		return Log::Warning("No current model to binarize");

	//load the current model in a pristine state (no navlights, shields...)
	//and then save it into binary

	std::unique_ptr<SceneGraph::Model> model;
	try {
		SceneGraph::Loader ld(m_renderer);
		model.reset(ld.LoadModel(m_modelName));
	} catch (...) {
		//minimal error handling, this is not expected to happen since we got this far.
		Log::Warning("Could not load model");
		return;
	}

	try {
		SceneGraph::BinaryConverter bc(m_renderer);
		bc.Save(m_modelName, model.get());
		Log::Info("Saved binary model file");
	} catch (const CouldNotOpenFileException &) {
		Log::Warning("Could not open file or directory for writing");
	} catch (const CouldNotWriteToFileException &) {
		Log::Warning("Error while writing to file");
	}
}

void ModelViewer::SetModel(const std::string &filename)
{
	Log::Info("Loading model {}...", filename);

	//this is necessary to reload textures
	m_renderer->RemoveAllCachedTextures();

	ClearModel();

	if (m_modelWindow->LoadModel(filename)) {
		m_modelName = filename;
		OnModelLoaded();
	}
}

void ModelViewer::OnModelLoaded()
{
	SceneGraph::Model *model = m_modelWindow->GetModel();

	ResetThrusters();

	m_shields.reset(new Shields(model));
	SceneGraph::DumpVisitor d(model);
	model->GetRoot()->Accept(d);
	Log::Verbose("{}", d.GetModelStatistics());

	SceneGraph::FindNodeVisitor visitor(SceneGraph::FindNodeVisitor::MATCH_NAME_STARTSWITH, "thruster_");
	model->GetRoot()->Accept(visitor);
	m_modelIsShip = !visitor.GetResults().empty();

	m_shields.reset(new Shields(model));

	m_modelSupportsDecals = model->SupportsDecals();
	m_modelHasShields = m_shields.get() && m_shields->GetFirstShieldMesh();
}

void ModelViewer::DrawModelSelector()
{
	if (!m_modelName.empty()) {
		ImGui::PushFont(m_pigui->GetFont("pionillium", 14));
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Model: %s", m_modelName.c_str());
		ImGui::PopFont();

		ImGui::SameLine();
		if (ImGui::Button("Reload Model"))
			ReloadModel();
	}

	if (ImGui::BeginChild("FileList")) {
		for (const auto &name : m_fileNames) {
			if (ImGui::Selectable(name.c_str())) {
				m_requestedModelName = name;
			}
		}
	}
	ImGui::EndChild();
}

void ModelViewer::DrawModelTags()
{
	SceneGraph::Model *model = m_modelWindow->GetModel();
	if (!model)
		return;

	const uint32_t numTags = model->GetNumTags();
	if (!numTags) return;

	for (uint32_t i = 0; i < numTags; i++) {
		auto *tag = model->GetTagByIndex(i);
		if (ImGui::Selectable(tag->GetName().c_str(), tag == m_selectedTag)) {
			m_selectedTag = tag;
		}
	}
}

void ModelViewer::DrawTagNames()
{
	if (!m_selectedTag)
		return;

	auto size = ImGui::GetWindowSize();
	m_renderer->SetTransform(matrix4x4f::Identity());

	vector3f point = m_modelWindow->GetModelViewMat() * m_selectedTag->GetGlobalTransform().GetTranslate();
	point = Graphics::ProjectToScreen(m_renderer, point);

	ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(point.x + 8.0f, size.y - point.y);
	ImGui::GetWindowDrawList()->AddText(pos, IM_COL32_WHITE, m_selectedTag->GetName().c_str());
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

	void ApplyTag(SceneGraph::Tag &tag) override
	{
		DisplayNode(tag, "Tag");
	}
};

void ModelViewer::DrawModelHierarchy()
{
	if (!m_modelWindow->GetModel())
		return;

	NodeHierarchyVisitor v = {};
	m_modelWindow->GetModel()->GetRoot()->Accept(v);
}

void ModelViewer::DrawShipControls()
{
	bool showMenu = m_modelIsShip || m_modelHasShields || m_modelSupportsDecals;

	if (!showMenu || !Draw::MenuButton("Ship"))
		return;

	if (m_modelSupportsDecals) {
		ImGui::SeparatorText("Decals");
		ImGui::Spacing();

		const char *preview_name = m_decals[m_currentDecal].c_str();
		if (ImGui::BeginCombo("Decals", preview_name)) {
			for (size_t idx = 0; idx < m_decals.size(); idx++) {
				const bool selected = m_currentDecal == idx;
				if (ImGui::Selectable(m_decals[idx].c_str(), selected) && !selected) {
					m_currentDecal = idx;
					m_modelWindow->SetDecals(m_decals[idx]);
				}
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();
	}

	if (m_modelHasShields) {
		ImGui::SeparatorText("Shields");
		ImGui::Spacing();

		ImGui::Checkbox("Show Shields", &m_showShields);
		if (ImGui::Button("Test Shield Hit"))
			HitIt();

		ImGui::Spacing();
	}

	if (m_modelIsShip) {
		bool valuesChanged = false;
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

		ImGui::Spacing();

		if (ImGui::Button("Reset Thrusters", ImVec2(-FLT_MIN, 0.f))) {
			valuesChanged = true;

			m_linearThrust = {};
			m_angularThrust = {};
		}

		if (valuesChanged)
			m_modelWindow->GetModel()->SetThrust(m_linearThrust, m_angularThrust);

		ImGui::Spacing();

		ImGui::SeparatorText("Weapons");
		ImGui::Spacing();

		if (ImGui::Button("Attach Test Guns"))
			ToggleGuns();
	}

	ImGui::EndMenu();
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

void ModelViewer::SetupLayout(ImGuiID dockspaceID)
{
	ImGui::DockBuilderRemoveNode(dockspaceID);
	ImGuiID nodeID = ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);

	ImGui::DockBuilderSetNodePos(nodeID, { 0.f, 0.f });
	ImGui::DockBuilderSetNodeSize(nodeID, ImGui::GetWindowSize());

	ImGuiID sideUp = ImGui::DockBuilderSplitNode(nodeID, ImGuiDir_Right, 0.2, nullptr, &nodeID);
	ImGuiID sideDown = ImGui::DockBuilderSplitNode(sideUp, ImGuiDir_Down, 0.3, nullptr, &sideUp);

	// NOTE: will be collapsed until used
	// ImGuiID centerDown = ImGui::DockBuilderSplitNode(nodeID, ImGuiDir_Down, 0.2, nullptr, &nodeID);

	ImGui::DockBuilderGetNode(nodeID)->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar;
	ImGui::DockBuilderGetNode(dockspaceID)->LocalFlags |= ImGuiDockNodeFlags_NoDockingSplitMe;

	ImGui::DockBuilderDockWindow(SELECTOR_WND_NAME, sideUp);
	ImGui::DockBuilderDockWindow(TAGS_WND_NAME, sideUp);
	ImGui::DockBuilderDockWindow(HIERARCHY_WND_NAME, sideUp);
	ImGui::DockBuilderDockWindow(LOG_WND_NAME, sideDown);
	ImGui::DockBuilderDockWindow("Model Viewer", nodeID);

	ImGui::DockBuilderFinish(dockspaceID);

	ImGui::SetWindowFocus(TAGS_WND_NAME);
}

void ModelViewer::DrawPiGui()
{
	if (m_metricsWindow)
		ImGui::ShowMetricsWindow();

	Draw::BeginHostWindow("HostWindow", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking);

	ImGuiID dockspaceID = ImGui::GetID("DockSpace");
	static bool isFirstRun = true;

	// TODO: need some way to load user's docking layout but ensure windows are
	// docked into dock nodes
	if (isFirstRun /* !ImGui::DockBuilderGetNode(dockspaceID) */)
		SetupLayout(dockspaceID);

	ImGui::DockSpace(dockspaceID);

	if (!m_showUI || m_screenshotQueued) {
		ImGui::End();
		return;
	}

	ImGui::PushFont(m_pigui->GetFont("pionillium", 13));

	if (ImGui::Begin(SELECTOR_WND_NAME))
		DrawModelSelector();
	ImGui::End();

	if (m_modelWindow->GetModel()) {
		if (ImGui::Begin(HIERARCHY_WND_NAME))
			DrawModelHierarchy();
		ImGui::End();

		if (ImGui::Begin(TAGS_WND_NAME))
			DrawModelTags();
		ImGui::End();
	}

	if (ImGui::Begin(LOG_WND_NAME))
		DrawLog();
	ImGui::End();

	if (isFirstRun) {
		// focus back to the model viewer
		ImGui::SetWindowFocus("Model Viewer");
	}

	ImGui::PopFont();

	isFirstRun = false;
	ImGui::End();
}
