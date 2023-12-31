// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PerfInfo.h"
#include "Frame.h"
#include "Game.h"
#include "Input.h"
#include "LuaPiGui.h"
#include "Pi.h"
#include "Player.h"
#include "SectorView.h"
#include "Space.h"
#include "core/Log.h"
#include "graphics/Renderer.h"
#include "graphics/Stats.h"
#include "graphics/Texture.h"
#include "lua/Lua.h"
#include "lua/LuaManager.h"
#include "scenegraph/Model.h"

#include <fmt/core.h>
#include <imgui/imgui.h>
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <functional>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
// order of header includes matters, thanks Windows.h!
#include <psapi.h>
#endif

// using namespace PiGui;
using PerfInfo = PiGui::PerfInfo;

struct PerfInfo::ImGuiState {
	bool perfWindowOpen = true;
	bool updatePause = false;
	bool metricsWindowOpen = false;
	bool stackToolOpen = false;
	uint32_t playerModelDebugFlags = 0;

	bool textureCacheViewerOpen = false;

	std::map<std::string, std::vector<std::string>> indirectionMap;

	bool hasSelectedTexture = false;
	std::pair<std::string, std::string> selectedTexture;
};

PerfInfo::PerfInfo() :
	m_state(new ImGuiState({}))
{
	m_fpsCounter.history.fill(0.0);
	m_physCounter.history.fill(0.0);
	m_piguiCounter.history.fill(0.0);
}

PerfInfo::~PerfInfo()
{
	delete m_state;
}

// ============================================================================
//
// Hardware Performance Information
//
// ============================================================================

#define ignoreLine(f) f.ignore(std::numeric_limits<std::streamsize>::max(), '\n')
static PerfInfo::MemoryInfo GetMemoryInfo()
{
	PerfInfo::MemoryInfo ret{};
#if __linux__
	std::ifstream statusFile("/proc/self/status");
	while (statusFile.good() && !statusFile.eof()) {
		if (statusFile.peek() != 'V') {
			ignoreLine(statusFile);
			continue;
		}

		char statName[16] = { 0 };
		statusFile.getline(statName, 16, ':');

		if (strcmp(statName, "VmHWM") == 0) {
			statusFile >> ret.peakMemSize;
		} else if (strcmp(statName, "VmRSS") == 0) {
			statusFile >> ret.currentMemSize;
		}

		ignoreLine(statusFile);
	}
	statusFile.close();

#elif _WIN32
	// Get win32 memory count
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
		// convert from bytes to kilobytes
		ret.peakMemSize = pmc.PeakWorkingSetSize / 1024;
		ret.currentMemSize = pmc.WorkingSetSize / 1024;
	}
#elif __APPLE__
	// TODO: get OSX memory count
#endif

	return ret;
}
#undef ignoreLine

PerfInfo::CounterInfo &PerfInfo::GetCounter(CounterType ct)
{
	switch (ct) {
	case COUNTER_FPS: return m_fpsCounter;
	case COUNTER_PHYS: return m_physCounter;
	case COUNTER_PIGUI: return m_piguiCounter;
	// default value is never reached, calm down -Werror=return-type
	default: return m_fpsCounter;
	}
}

void PerfInfo::ClearCounter(CounterType ct)
{
	CounterInfo &counter = GetCounter(ct);
	counter.history.fill(0.);
	counter.average = 0.;
	counter.max = 0.;
	counter.min = 0.;
}

void PerfInfo::UpdateCounter(CounterType ct, float deltaTime)
{
	// Don't accumulate new frames when performance data is paused.
	if (m_state->updatePause)
		return;

	CounterInfo &counter = GetCounter(ct);

	// Drop the oldest frame, make room for the new frame.
	std::move(counter.history.begin() + 1, counter.history.end(), counter.history.begin());
	counter.history[NUM_FRAMES - 1] = deltaTime * 1e3;

	float timeAccum = 0;
	counter.max = 0.f;
	counter.min = 0.f;
	std::for_each(counter.history.begin(), counter.history.end(), [&](float i) {
		timeAccum += i;
		counter.max = std::max(counter.max, i);
		counter.min = std::min(counter.min, i);
	});
	counter.average = timeAccum / double(NUM_FRAMES);
}

void PerfInfo::Update(float deltaTime)
{
	UpdateCounter(COUNTER_FPS, deltaTime);

	lastUpdateTime += deltaTime;
	if (lastUpdateTime > 1.0) {
		lastUpdateTime = fmod(lastUpdateTime, 1.0);

		lua_mem = ::Lua::manager->GetMemoryUsage();
		process_mem = GetMemoryInfo();
	}
}

// TODO: evaluate whether this method of tracking FPS is necessary.
void PerfInfo::UpdateFrameInfo(int fS, int pfS)
{
	framesThisSecond = fS;
	physFramesThisSecond = pfS;
}

void PerfInfo::SetUpdatePause(bool pause)
{
	m_state->updatePause = pause;
}

// ============================================================================
//
// Main Performance Window
//
// ============================================================================

namespace ImGui {
	IMGUI_API void Value(const char *prefix, const std::string &str)
	{
		ImGui::TextWrapped("%s: %s", prefix, str.c_str());
	}
} // namespace ImGui

static constexpr double scale_MB = 1024.0 * 1024.0;

void PerfInfo::Draw()
{
	if (m_state->perfWindowOpen)
		DrawPerfWindow();

	if (m_state->textureCacheViewerOpen)
		DrawTextureCache();

	if (m_state->hasSelectedTexture &&
		Pi::renderer->GetCachedTexture(m_state->selectedTexture.first, m_state->selectedTexture.second))
		DrawTextureInspector();

	if (m_state->metricsWindowOpen)
		ImGui::ShowMetricsWindow(&m_state->metricsWindowOpen);

	if (m_state->stackToolOpen)
		ImGui::ShowStackToolWindow(&m_state->stackToolOpen);
}

void PerfInfo::DrawPerfWindow()
{
	if (ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_NoNav)) {
		ImGui::Text("%.1f fps (%.1f ms) %.1f physics ups (%.1f ms/u)", framesThisSecond, m_fpsCounter.average, physFramesThisSecond, m_physCounter.average);
		ImGui::PlotLines("Frame Time (ms)", m_fpsCounter.history.data(), m_fpsCounter.history.size(), 0, nullptr, 2.0, 33.0, { 0, 45 });
		ImGui::PlotLines("Update Time (ms)", m_physCounter.history.data(), m_physCounter.history.size(), 0, nullptr, 0.0, 10.0, { 0, 25 });
		ImGui::PlotLines("Pigui Time (ms)", m_piguiCounter.history.data(), m_piguiCounter.history.size(), 0, nullptr, 0.0, 5.0, { 0, 25 });
		if (ImGui::Button(m_state->updatePause ? "Unpause" : "Pause")) {
			SetUpdatePause(!m_state->updatePause);
		}

		if (process_mem.currentMemSize)
			ImGui::Text("%.1f MB process memory usage (%.1f MB peak)", (process_mem.currentMemSize * 1e-3), (process_mem.peakMemSize * 1e-3));
		ImGui::Text("%.3f MB Lua memory usage", double(lua_mem) / scale_MB);
		ImGui::Spacing();

		if (ImGui::BeginTabBar("PerfInfoTabs")) {
			if (ImGui::BeginTabItem("Renderer")) {
				DrawRendererStats();
				DrawImGuiStats();
				ImGui::EndTabItem();
			}

			if (false && ImGui::BeginTabItem("Input")) {
				DrawInputDebug();
				ImGui::EndTabItem();
			}

			if (Pi::game) {
				if (Pi::player->GetFlightState() != Ship::HYPERSPACE && ImGui::BeginTabItem("WorldView")) {
					DrawWorldViewStats();
					ImGui::EndTabItem();
				}
			}

			PiGui::RunHandler(Pi::GetFrameTime(), "debug-tabs");

			ImGui::EndTabBar();
		}
	}

	ImGui::End();

	PiGui::RunHandler(Pi::GetFrameTime(), "debug");
}

void PerfInfo::DrawRendererStats()
{
	const Graphics::Stats::TFrameData &stats = Pi::renderer->GetStats().FrameStatsPrevious();
	const Uint32 numDrawCalls = stats.m_stats[Graphics::Stats::STAT_DRAWCALL];
	const Uint32 numTris = stats.m_stats[Graphics::Stats::STAT_NUM_TRIS];
	const Uint32 numLines = stats.m_stats[Graphics::Stats::STAT_NUM_LINES];
	const Uint32 numPoints = stats.m_stats[Graphics::Stats::STAT_NUM_POINTS];
	const Uint32 numCmdListFlushes = stats.m_stats[Graphics::Stats::STAT_NUM_CMDLIST_FLUSHES];
	const Uint32 numBuffersCreated = stats.m_stats[Graphics::Stats::STAT_CREATE_BUFFER];
	const Uint32 numBuffersInUse = stats.m_stats[Graphics::Stats::STAT_BUFFER_INUSE];
	const Uint32 numDynamicBuffersCreated = stats.m_stats[Graphics::Stats::STAT_DYNAMIC_DRAW_BUFFER_CREATED];
	const Uint32 numDynamicBuffersInUse = stats.m_stats[Graphics::Stats::STAT_DYNAMIC_DRAW_BUFFER_INUSE];
	const Uint32 numDrawBuffers = stats.m_stats[Graphics::Stats::STAT_DRAW_UNIFORM_BUFFER_INUSE];
	const Uint32 numDrawBufferAllocs = stats.m_stats[Graphics::Stats::STAT_DRAW_UNIFORM_BUFFER_ALLOCS];
	const Uint32 numRenderStates = stats.m_stats[Graphics::Stats::STAT_NUM_RENDER_STATES];
	const Uint32 numShaderPrograms = stats.m_stats[Graphics::Stats::STAT_NUM_SHADER_PROGRAMS];

	const Uint32 numDrawBuildings = stats.m_stats[Graphics::Stats::STAT_BUILDINGS];
	const Uint32 numDrawCities = stats.m_stats[Graphics::Stats::STAT_CITIES];
	const Uint32 numDrawGroundStations = stats.m_stats[Graphics::Stats::STAT_GROUNDSTATIONS];
	const Uint32 numDrawSpaceStations = stats.m_stats[Graphics::Stats::STAT_SPACESTATIONS];
	const Uint32 numDrawAtmospheres = stats.m_stats[Graphics::Stats::STAT_ATMOSPHERES];
	const Uint32 numDrawPlanets = stats.m_stats[Graphics::Stats::STAT_PLANETS];
	const Uint32 numDrawGasGiants = stats.m_stats[Graphics::Stats::STAT_GASGIANTS];
	const Uint32 numDrawStars = stats.m_stats[Graphics::Stats::STAT_STARS];
	const Uint32 numDrawShips = stats.m_stats[Graphics::Stats::STAT_SHIPS];
	const Uint32 numDrawBillBoards = stats.m_stats[Graphics::Stats::STAT_BILLBOARD];

	const Uint32 numTex2ds = stats.m_stats[Graphics::Stats::STAT_NUM_TEXTURE2D];
	const Uint32 tex2dMemUsage = stats.m_stats[Graphics::Stats::STAT_MEM_TEXTURE2D];
	const Uint32 numTexCubemaps = stats.m_stats[Graphics::Stats::STAT_NUM_TEXTURECUBE];
	const Uint32 texCubeMemUsage = stats.m_stats[Graphics::Stats::STAT_MEM_TEXTURECUBE];
	const Uint32 numTexArray2ds = stats.m_stats[Graphics::Stats::STAT_NUM_TEXTUREARRAY2D];
	const Uint32 texArray2dMemUsage = stats.m_stats[Graphics::Stats::STAT_MEM_TEXTUREARRAY2D];
	const Uint32 numCachedTextures = numTex2ds + numTexCubemaps + numTexArray2ds;
	const Uint32 cachedTextureMemUsage = tex2dMemUsage + texCubeMemUsage + texArray2dMemUsage;

	ImGui::Text("Renderer:");
	ImGui::Text("%u Draw calls, %u CommandList flushes",
		numDrawCalls, numCmdListFlushes);

	ImGui::Indent();
	ImGui::Text("%u points", numPoints);
	ImGui::Text("%u lines", numLines);
	ImGui::Text("%u tris (%.2fM tris/sec)", numTris, numTris * framesThisSecond * 1e-6);
	ImGui::Unindent();
	ImGui::Spacing();

	ImGui::Text("%u Buildings, %u Cities, %u Gd.Stations, %u Sp.Stations",
		numDrawBuildings, numDrawCities, numDrawGroundStations, numDrawSpaceStations);
	ImGui::Text("%u Atmospheres, %u Planets, %u Gas Giants, %u Stars, %u Ships",
		numDrawAtmospheres, numDrawPlanets, numDrawGasGiants, numDrawStars, numDrawShips);
	ImGui::Text("%u Billboards, %u GeoPatches (%d tris)",
		numDrawBillBoards, Pi::statNumPatches, Pi::statSceneTris);
	ImGui::Text("%u Buffers Created (%u in use)", numBuffersCreated, numBuffersInUse);
	ImGui::Text("%u Dynamic Draw Buffers Created (%u in use)", numDynamicBuffersCreated, numDynamicBuffersInUse);
	ImGui::Text("%u Draw Uniform Buffers (%u allocations)", numDrawBuffers, numDrawBufferAllocs);
	ImGui::Spacing();

	ImGui::Text("%u cached shader programs", numShaderPrograms);
	ImGui::Text("%u cached render states", numRenderStates);
	ImGui::Text("%u cached textures, using %.3f MB VRAM", numCachedTextures, double(cachedTextureMemUsage) / scale_MB);

	if (ImGui::Button("Open Texture Cache Visualizer"))
		m_state->textureCacheViewerOpen = true;

	if (ImGui::Button("Reload Shaders"))
		Pi::renderer->ReloadShaders();

	ImGui::Text("%u Texture2D in cache (%.3f MB)", numTex2ds, double(tex2dMemUsage) / scale_MB);
	ImGui::Text("%u Cubemaps in cache (%.3f MB)", numTexCubemaps, double(texCubeMemUsage) / scale_MB);
	ImGui::Text("%u TextureArray2D in cache (%.3f MB)", numTexArray2ds, double(texArray2dMemUsage) / scale_MB);
}

void PerfInfo::DrawWorldViewStats()
{
	vector3d pos = Pi::player->GetPosition();
	vector3d abs_pos = Pi::player->GetPositionRelTo(Pi::game->GetSpace()->GetRootFrame());

	const FrameId playerFrame = Pi::player->GetFrame();

	ImGui::TextUnformatted(fmt::format("Player Position: {:.5}, {:.5}, {:.5}", pos.x, pos.y, pos.z).c_str());
	ImGui::TextUnformatted(fmt::format("Absolute Position: {:.5}, {:.5}, {:.5}", abs_pos.x, abs_pos.y, abs_pos.z).c_str());

	const Frame *frame = Frame::GetFrame(playerFrame);
	const SystemPath &path(frame->GetSystemBody()->GetPath());

	std::string tempStr;
	tempStr = fmt::format("Relative to frame: {} [{}, {}, {}, {}, {}]",
		frame->GetLabel(), path.sectorX, path.sectorY, path.sectorZ, path.systemIndex, path.bodyIndex);

	ImGui::TextUnformatted(tempStr.c_str());

	tempStr = fmt::format("Distance from frame: {:.2f} km, rotating: {}, has rotation: {}",
		pos.Length() / 1000.0, frame->IsRotFrame(), frame->HasRotFrame());

	ImGui::TextUnformatted(tempStr.c_str());

	ImGui::Spacing();

	//Calculate lat/lon for ship position
	const vector3d dir = pos.NormalizedSafe();
	const float lat = RAD2DEG(asin(dir.y));
	const float lon = RAD2DEG(atan2(dir.x, dir.z));

	ImGui::TextUnformatted(fmt::format("Lat / Lon: {:.8} / {:.8}", lat, lon).c_str());

	char aibuf[256];
	Pi::player->AIGetStatusText(aibuf);

	ImGui::TextUnformatted(aibuf);

	ImGui::Spacing();
	ImGui::TextUnformatted("Player Model ShowFlags:");

	using Flags = SceneGraph::Model::DebugFlags;

	bool showColl = m_state->playerModelDebugFlags & Flags::DEBUG_COLLMESH;
	bool showBBox = m_state->playerModelDebugFlags & Flags::DEBUG_BBOX;
	bool showTags = m_state->playerModelDebugFlags & Flags::DEBUG_TAGS;

	bool changed = ImGui::Checkbox("Show Collision Mesh", &showColl);
	changed |= ImGui::Checkbox("Show Bounding Box", &showBBox);
	changed |= ImGui::Checkbox("Show Tag Locations", &showTags);

	/* clang-format off */
	if (changed) {
		m_state->playerModelDebugFlags = (showColl ? Flags::DEBUG_COLLMESH : 0)
			| (showBBox ? Flags::DEBUG_BBOX : 0)
			| (showTags ? Flags::DEBUG_TAGS : 0);
		Pi::player->GetModel()->SetDebugFlags(m_state->playerModelDebugFlags);
	}
	/* clang-format on */

	if (Pi::player->GetNavTarget() && Pi::player->GetNavTarget()->GetSystemBody()) {
		const auto *sbody = Pi::player->GetNavTarget()->GetSystemBody();
		ImGui::TextUnformatted(fmt::format("Name: {}, Population: {}", sbody->GetName(), sbody->GetPopulation() * 1e9).c_str());
	}

	if (Pi::GetView() == Pi::game->GetSectorView()) {
		if (ImGui::Button("Dump Selected System")) {
			SystemPath path = Pi::game->GetSectorView()->GetSelected();
			RefCountedPtr<StarSystem> system = Pi::game->GetGalaxy()->GetStarSystem(path);

			if (system)
				system->Dump(Log::GetLog()->GetLogFileHandle());
		}
	}

}

void PerfInfo::DrawInputDebug()
{
	std::ostringstream output;
	auto frameList = Pi::input->GetInputFrames();
	uint32_t index = 0;
	for (const auto *frame : frameList) {
		ImGui::Text("InputFrame %d: modal=%d", index, frame->modal);
		ImGui::Indent();
		uint32_t actionNum = 0;
		for (const auto *action : frame->actions) {
			ImGui::Text("Action %d: active=%d (%d, %d)", actionNum, action->m_active, action->binding.m_active, action->binding2.m_active);
			if (action->binding.Enabled()) {
				output << action->binding;
				ImGui::TextUnformatted(output.str().c_str());
				output.str("");
			}
			if (action->binding2.Enabled()) {
				output << action->binding2;
				ImGui::TextUnformatted(output.str().c_str());
				output.str("");
			}

			ImGui::Separator();
			actionNum++;
		}

		ImGui::Spacing();

		actionNum = 0;
		for (const auto *axis : frame->axes) {
			ImGui::Text("Axis %d: value=%.2f (%d, %d)", actionNum, axis->m_value, axis->positive.m_active, axis->negative.m_active);
			if (axis->positive.Enabled()) {
				output << axis->positive;
				ImGui::TextUnformatted(output.str().c_str());
				output.str("");
			}

			if (axis->negative.Enabled()) {
				output << axis->negative;
				ImGui::TextUnformatted(output.str().c_str());
				output.str("");
			}

			ImGui::Separator();
			actionNum++;
		}
		ImGui::Unindent();

		ImGui::Spacing();
		index++;
	}
}

void PerfInfo::DrawImGuiStats()
{
	ImGui::NewLine();
	auto &io = ImGui::GetIO();
	ImGui::Text("ImGui stats:");
	ImGui::Text("%d verts, %d tris", io.MetricsRenderVertices, io.MetricsRenderIndices / 3);
	ImGui::Text("%d active windows (%d visible)", io.MetricsActiveWindows, io.MetricsRenderWindows);
	ImGui::Text("%d current allocations", io.MetricsActiveAllocations);

	if (ImGui::Button("Toggle Metrics Window")) {
		m_state->metricsWindowOpen = !m_state->metricsWindowOpen;
	}

	if (ImGui::Button("Toggle Stack Tool")) {
		m_state->stackToolOpen = !m_state->stackToolOpen;
	}
}

void PerfInfo::DrawStatList(const Perf::Stats::FrameInfo &fi)
{
	ImGui::BeginChild("FrameInfo");
	ImGui::Columns(2);
	for (auto &t : fi) {
		ImGui::Text("%s:", t.first.c_str());
		ImGui::NextColumn();
		ImGui::Text("%'d", t.second);
		ImGui::NextColumn();
	}
	ImGui::Columns();
	ImGui::EndChild();
}

// ============================================================================
//
// Texture Cache Visualizer
//
// ============================================================================

bool DrawTexture(PerfInfo::ImGuiState *m_state, const Graphics::Texture *tex)
{
	ImGui::BeginGroup();
	auto pos0 = ImGui::GetCursorPos();

	const vector2f texUVs = tex->GetDescriptor().texSize;
	ImGui::Image(ImTextureID(tex), { 128, 128 }, { 0, 0 }, { texUVs.x, texUVs.y });

	auto pos1 = ImGui::GetCursorPos();
	ImGui::SetCursorPos(pos0);

	const vector3f &dataSize = tex->GetDescriptor().dataSize;
	ImGui::Text("%ux%u", uint32_t(dataSize.x), uint32_t(dataSize.y));
	ImGui::Text("%.1f KB", double(tex->GetTextureMemSize()) / 1024.0);

	ImGui::SetCursorPos(pos1);
	ImGui::EndGroup();
	return ImGui::IsItemClicked();
}

void PerfInfo::DrawTextureCache()
{
	if (ImGui::Begin("Texture Cache Viewer", &m_state->textureCacheViewerOpen, ImGuiWindowFlags_NoNav)) {
		for (auto &v : m_state->indirectionMap)
			// purge any textures which may have changed / evicted, but don't resize the vector
			// in the average case, this will simply re-fill the vector each frame, a fairly trivial operation.
			v.second.clear();

		for (const auto &t : Pi::renderer->GetTextureCache()) {
			const auto &key = t.first;
			if (t.second->Get()->GetDescriptor().type != Graphics::TEXTURE_2D)
				continue;

			m_state->indirectionMap[key.first].push_back(key.second);
		}

		if (ImGui::BeginTabBar("Texture List")) {
			const int item_width = 128 + ImGui::GetStyle().ItemSpacing.x;
			for (const auto &t : m_state->indirectionMap) {
				if (!ImGui::BeginTabItem(t.first.c_str()))
					continue;

				if (ImGui::BeginChild("##Texture List Scroll")) {
					const int window_width = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
					const int num_columns = std::max(int(window_width) / item_width, 1);
					ImGui::Columns(num_columns);
					if (num_columns > 1) {
						for (int idx = 0; idx < num_columns; idx++)
							ImGui::SetColumnWidth(idx, item_width);
					}

					for (const std::string &name : t.second) {
						const auto *ptr = Pi::renderer->GetCachedTexture(t.first, name);
						if (DrawTexture(m_state, ptr)) {
							m_state->selectedTexture = { t.first, name };
							m_state->hasSelectedTexture = true;
						}

						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("%s", name.c_str());

						if (num_columns > 1)
							ImGui::NextColumn();
					}
				}
				ImGui::EndChild();

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

void PerfInfo::DrawTextureInspector()
{
	const auto &selectedTex = m_state->selectedTexture;
	const auto *tex = Pi::renderer->GetCachedTexture(selectedTex.first, selectedTex.second);
	if (tex != nullptr && m_state->hasSelectedTexture) {
		ImGui::SetNextWindowSize({ 300, 400 }, ImGuiCond_Appearing);
		std::string window_name = selectedTex.second + "###Texture Inspector";
		if (ImGui::Begin(window_name.c_str(), &m_state->hasSelectedTexture)) {
			const Graphics::TextureDescriptor &descriptor = tex->GetDescriptor();

			// If we have POT-extended textures, only display the part of the texture corresponding to the actual texture data.
			const vector2f texUVs = tex->GetDescriptor().texSize;
			ImGui::Image(ImTextureID(tex), { 256, 256 }, { 0, 0 }, { texUVs.x, texUVs.y });
			ImGui::Text("Dimensions: %ux%u", uint32_t(descriptor.dataSize.x), uint32_t(descriptor.dataSize.y));
			ImGui::Value("Mipmap Count", tex->GetDescriptor().numberOfMipMaps);
			ImGui::Value("VRAM Size", tex->GetTextureMemSize());
			if (texUVs.x < 1.0 || texUVs.y < 1.0)
				ImGui::Text("Original Size: %ux%u", int(texUVs.x * descriptor.dataSize.x), int(texUVs.y * descriptor.dataSize.y));

			ImGui::Spacing();
			ImGui::Value("Cache Bucket", selectedTex.first);
			ImGui::Value("Cache ID", selectedTex.second);
			ImGui::Value("TextureID", tex->GetTextureID());
			ImGui::Value("RefCount", tex->GetRefCount());
		}
		ImGui::End();
	}
}
