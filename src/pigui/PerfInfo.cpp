// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PerfInfo.h"
#include "Game.h"
#include "Pi.h"
#include "graphics/Renderer.h"
#include "graphics/Stats.h"
#include "graphics/Texture.h"
#include "lua/Lua.h"
#include "lua/LuaManager.h"
#include "text/TextureFont.h"

#include <imgui/imgui.h>
#include <algorithm>
#include <fstream>
#include <functional>

#ifdef _WIN32
#include <windows.h>
// order of header includes matters, thanks Windows.h!
#include <psapi.h>
#endif

using namespace PiGUI;

struct PerfInfo::ImGuiState {
	bool perfWindowOpen = true;
	bool updatePause = false;

	bool textureCacheViewerOpen = false;

	std::map<std::string, std::vector<std::string>> indirectionMap;

	bool hasSelectedTexture = false;
	std::pair<std::string, std::string> selectedTexture;
};

PerfInfo::PerfInfo() :
	m_state(new ImGuiState({}))
{
	m_fpsGraph.fill(0.0);
	m_physFpsGraph.fill(0.0);
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

void PerfInfo::Update(float deltaTime, float physTime)
{
	// Don't accumulate new frames when performance data is paused.
	if (m_state->updatePause)
		return;

	// Drop the oldest frame, make room for the new frame.
	std::move(m_fpsGraph.begin() + 1, m_fpsGraph.end(), m_fpsGraph.begin());
	std::move(m_physFpsGraph.begin() + 1, m_physFpsGraph.end(), m_physFpsGraph.begin());
	m_fpsGraph[NUM_FRAMES - 1] = deltaTime;
	m_physFpsGraph[NUM_FRAMES - 1] = physTime;

	float fpsAccum = 0;
	frameTimeMax = 0.f;
	frameTimeMin = 0.f;
	std::for_each(m_fpsGraph.begin(), m_fpsGraph.end(), [&](float i) {
		fpsAccum += i;
		frameTimeMax = std::max(frameTimeMax, i);
		frameTimeMin = std::min(frameTimeMin, i);
	});
	frameTimeAverage = fpsAccum / double(NUM_FRAMES);

	float physFpsAccum = 0;
	physFrameTimeMax = 0.f;
	physFrameTimeMin = 0.f;
	std::for_each(m_physFpsGraph.begin(), m_physFpsGraph.end(), [&](float i) {
		physFpsAccum += i;
		physFrameTimeMax = std::max(physFrameTimeMax, i);
		physFrameTimeMin = std::min(physFrameTimeMin, i);
	});
	physFrameTimeAverage = physFpsAccum / double(NUM_FRAMES);

	lastUpdateTime += deltaTime;
	if (lastUpdateTime > 1000.0) {
		lastUpdateTime = fmod(lastUpdateTime, 1000.0);

		lua_mem = Lua::manager->GetMemoryUsage();
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

ImTextureID GetImTextureID(const Graphics::Texture *tex)
{
	return reinterpret_cast<ImTextureID>(tex->GetTextureID() | 0UL);
}

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
}

void PerfInfo::DrawPerfWindow()
{
	if (ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_NoNav)) {
		ImGui::Text("%.1f fps (%.1f ms) %.1f physics ups (%.1f ms/u)", framesThisSecond, frameTimeAverage, physFramesThisSecond, physFrameTimeAverage);
		ImGui::PlotLines("Frame Time (ms)", m_fpsGraph.data(), m_fpsGraph.size(), 0, nullptr, 0.0, 33.0, { 0, 60 });
		ImGui::PlotLines("Update Time (ms)", m_physFpsGraph.data(), m_physFpsGraph.size(), 0, nullptr, 0.0, 10.0, { 0, 25 });
		if (ImGui::Button(m_state->updatePause ? "Unpause" : "Pause")) {
			SetUpdatePause(!m_state->updatePause);
		}

		if (process_mem.currentMemSize)
			ImGui::Text("%.1f MB process memory usage (%.1f MB peak)", (process_mem.currentMemSize * 1e-3), (process_mem.peakMemSize * 1e-3));
		ImGui::Text("%.3f MB Lua memory usage", double(lua_mem) / scale_MB);
		ImGui::Spacing();
	}

	if (ImGui::BeginTabBar("PerfInfoTabs")) {
		if (ImGui::BeginTabItem("ImGui")) {
			DrawImGuiStats();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Renderer")) {
			DrawRendererStats();
			ImGui::EndTabItem();
		}

		if (Pi::game && Pi::game->GetGalaxy() && ImGui::BeginTabItem("Galaxy Stats")) {
			auto &stats = Pi::game->GetGalaxy()->GetStats();
			stats.FlushFrame();
			DrawStatList(stats.GetFrameStats());
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
	ImGui::End();
}

void PerfInfo::DrawRendererStats()
{
	const Graphics::Stats::TFrameData &stats = Pi::renderer->GetStats().FrameStatsPrevious();
	const Uint32 numDrawCalls = stats.m_stats[Graphics::Stats::STAT_DRAWCALL];
	const Uint32 numBuffersCreated = stats.m_stats[Graphics::Stats::STAT_CREATE_BUFFER];
	const Uint32 numBuffersInUse = stats.m_stats[Graphics::Stats::STAT_BUFFER_INUSE];
	const Uint32 numDrawTris = stats.m_stats[Graphics::Stats::STAT_DRAWTRIS];
	const Uint32 numDrawPointSprites = stats.m_stats[Graphics::Stats::STAT_DRAWPOINTSPRITES];
	const Uint32 numDrawBuildings = stats.m_stats[Graphics::Stats::STAT_BUILDINGS];
	const Uint32 numDrawCities = stats.m_stats[Graphics::Stats::STAT_CITIES];
	const Uint32 numDrawGroundStations = stats.m_stats[Graphics::Stats::STAT_GROUNDSTATIONS];
	const Uint32 numDrawSpaceStations = stats.m_stats[Graphics::Stats::STAT_SPACESTATIONS];
	const Uint32 numDrawAtmospheres = stats.m_stats[Graphics::Stats::STAT_ATMOSPHERES];
	const Uint32 numDrawPatches = stats.m_stats[Graphics::Stats::STAT_PATCHES];
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
	ImGui::Text("%d tris (%.3f M tris/sec), %d GeoPatches, %d glyphs",
		Pi::statSceneTris, Pi::statSceneTris * framesThisSecond * 1e-6, Pi::statNumPatches, Text::TextureFont::GetGlyphCount());
	ImGui::Text("%u draw calls (%u tris, %u point sprites, %u billboards)",
		numDrawCalls, numDrawTris, numDrawPointSprites, numDrawBillBoards);
	ImGui::Text("%u Buildings, %u Cities, %u Gd.Stations, %u Sp.Stations",
		numDrawBuildings, numDrawCities, numDrawGroundStations, numDrawSpaceStations);
	ImGui::Text("%u Atmospheres, %u Planets, %u Gas Giants, %u Stars, %u Ships",
		numDrawAtmospheres, numDrawPlanets, numDrawGasGiants, numDrawStars, numDrawShips);
	ImGui::Text("%u Buffers Created (%u in use)", numBuffersCreated, numBuffersInUse);
	ImGui::Spacing();

	ImGui::Text("%u cached textures, using %.3f MB VRAM", numCachedTextures, double(cachedTextureMemUsage) / scale_MB);

	if (ImGui::Button("Open Texture Cache Visualizer"))
		m_state->textureCacheViewerOpen = true;
	ImGui::Text("%u Texture2D in cache (%.3f MB)", numTex2ds, double(tex2dMemUsage) / scale_MB);
	ImGui::Text("%u Cubemaps in cache (%.3f MB)", numTexCubemaps, double(texCubeMemUsage) / scale_MB);
	ImGui::Text("%u TextureArray2D in cache (%.3f MB)", numTexArray2ds, double(texArray2dMemUsage) / scale_MB);
}

void PerfInfo::DrawImGuiStats()
{
	auto &io = ImGui::GetIO();
	ImGui::Text("ImGui stats:");
	ImGui::Text("%d verts, %d tris", io.MetricsRenderVertices, io.MetricsRenderIndices / 3);
	ImGui::Text("%d active windows (%d visible)", io.MetricsActiveWindows, io.MetricsRenderWindows);
	ImGui::Text("%d current allocations", io.MetricsActiveAllocations);
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
	ImGui::Image(GetImTextureID(tex), { 128, 128 }, { 0, 0 }, { texUVs.x, texUVs.y });

	auto pos1 = ImGui::GetCursorPos();
	ImGui::SetCursorPos(pos0);

	auto texSize = tex->GetDescriptor().dataSize;
	ImGui::Text("%ux%u", uint32_t(texSize.x), uint32_t(texSize.y));
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
					const int num_columns = std::max(int(ImGui::GetWindowContentRegionWidth()) / item_width, 1);
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
			ImGui::Image(GetImTextureID(tex), { 256, 256 }, { 0, 0 }, { texUVs.x, texUVs.y });
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
