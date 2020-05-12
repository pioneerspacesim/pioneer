// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "PerfStats.h"
#include "RefCounted.h"
#include <array>
#include <memory>

namespace PiGUI {

	class PerfInfo {
	public:
		PerfInfo();
		~PerfInfo();

		// Information about the current process memory usage in KB.
		struct MemoryInfo {
			size_t currentMemSize;
			size_t peakMemSize;
		};
		struct ImGuiState;

		void Draw();

		// Update with current frame and physics update time in ms
		void Update(float frameTime, float physTime);
		void UpdateFrameInfo(int framesThisSecond, int physFramesThisSecond);

		void SetShowDebugInfo(bool open);
		void SetUpdatePause(bool pause);

	private:
		void DrawPerfWindow();
		void DrawTextureCache();
		void DrawTextureInspector();

		void DrawRendererStats();
		void DrawImGuiStats();
		void DrawStatList(const Perf::Stats::FrameInfo &fi);

		static const int NUM_FRAMES = 60;
		std::array<float, NUM_FRAMES> m_fpsGraph;
		std::array<float, NUM_FRAMES> m_physFpsGraph;
		float frameTimeAverage = 0;
		float frameTimeMax = 0;
		float frameTimeMin = 0;
		float physFrameTimeAverage = 0;
		float physFrameTimeMax = 0;
		float physFrameTimeMin = 0;

		MemoryInfo process_mem;
		size_t lua_mem = 0;
		float framesThisSecond = 0;
		float physFramesThisSecond = 0;

		float lastUpdateTime = 0;

		ImGuiState *m_state;
	};

} // namespace PiGUI
