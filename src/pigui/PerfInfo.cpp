// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PerfInfo.h"
#include "Pi.h"
#include "graphics/Renderer.h"
#include "graphics/Stats.h"
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
	if (m_updatePause)
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
}

// TODO: evaluate whether this method of tracking FPS is necessary.
void PerfInfo::UpdateFrameInfo(int fS, int pfS)
{
	framesThisSecond = fS;
	physFramesThisSecond = pfS;

	lua_mem = Lua::manager->GetMemoryUsage();
	process_mem = GetMemoryInfo();
}

void PerfInfo::Draw()
{
	const Graphics::Stats::TFrameData &stats = Pi::renderer->GetStats().FrameStatsPrevious();
	const Uint32 numDrawCalls = stats.m_stats[Graphics::Stats::STAT_DRAWCALL];
	const Uint32 numBuffersCreated = stats.m_stats[Graphics::Stats::STAT_CREATE_BUFFER];
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

	auto &io = ImGui::GetIO();

	// ImGui::SetNextWindowFocus();
	if (ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_NoNav)) {
		auto info = GetMemoryInfo();
		ImGui::Text("%.1f fps (%.1f ms) %.1f physics ups (%.1f ms/u)", framesThisSecond, frameTimeAverage, physFramesThisSecond, physFrameTimeAverage);
		ImGui::PlotLines("Frame Time (ms)", m_fpsGraph.data(), m_fpsGraph.size(), 0, nullptr, 0.0, 33.0, { 0, 80 });
		ImGui::PlotLines("Update Time (ms)", m_physFpsGraph.data(), m_physFpsGraph.size(), 0, nullptr, 0.0, 10.0, { 0, 40 });
		if (ImGui::Button(m_updatePause ? "Unpause" : "Pause")) {
			SetUpdatePause(!m_updatePause);
		}

		if (process_mem.currentMemSize)
			ImGui::Text("%.1f MB process memory usage (%.1f MB peak)", (info.currentMemSize * 1e-3), (info.peakMemSize * 1e-3));
		ImGui::Text("%.3f MB Lua memory usage", double(lua_mem) / pow(1024.0, 2));
		ImGui::Spacing();

		ImGui::Text("Renderer:");
		ImGui::Text("%d tris (%.3f M tris/sec), %d GeoPatches, %d glyphs",
			Pi::statSceneTris, Pi::statSceneTris * framesThisSecond * 1e-6, Pi::statNumPatches, Text::TextureFont::GetGlyphCount());
		ImGui::Text("%u draw calls (%u tris, %u point sprites, %u billboards)",
			numDrawCalls, numDrawTris, numDrawPointSprites, numDrawBillBoards);
		ImGui::Text("%u Buildings, %u Cities, %u Gd.Stations, %u Sp.Stations",
			numDrawBuildings, numDrawCities, numDrawGroundStations, numDrawSpaceStations);
		ImGui::Text("%u Atmospheres, %u Planets, %u Gas Giants, %u Stars, %u Ships",
			numDrawAtmospheres, numDrawPlanets, numDrawGasGiants, numDrawStars, numDrawShips);
		ImGui::Text("%u Buffers Created", numBuffersCreated);
		ImGui::Spacing();

		ImGui::Text("ImGui stats:");
		ImGui::Text("%d verts, %d tris", io.MetricsRenderVertices, io.MetricsRenderIndices / 3);
		ImGui::Text("%d active windows (%d visible)", io.MetricsActiveWindows, io.MetricsRenderWindows);
		ImGui::Text("%d current allocations", io.MetricsActiveAllocations);

		ImGui::Spacing();
	}
	ImGui::End();
}
