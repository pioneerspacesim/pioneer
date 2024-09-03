// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "PerfStats.h"
#include <array>

namespace PiGui {

	class PerfInfo {
	public:
		PerfInfo();
		~PerfInfo();

		enum CounterType {
			COUNTER_FPS,
			COUNTER_PHYS,
			COUNTER_PIGUI,
			COUNTER_PROCMEM,
			COUNTER_LUAMEM,
		};

		// Information about the current process memory usage in KB.
		struct MemoryInfo {
			size_t currentMemSize;
			size_t peakMemSize;
		};

		static constexpr int NUM_FRAMES = 300;
		struct CounterInfo {
			std::array<float, NUM_FRAMES> history;
			float average = 0.f;
			float recent = 0.f;
			float min = 0.f;
			float max = 0.f;

			const char *name;
			const char *unit;

			uint32_t numRecentSamples;

			CounterInfo(const char *n, const char *u, uint32_t recent = 15);
		};

		struct ImGuiState;

		void Draw();

		// Update with current frame and physics update time in ms
		void UpdateCounter(CounterType counter, float counterTime);
		void ClearCounter(CounterType counter);

		void Update(float deltaTime);
		void UpdateFrameInfo(int framesThisSecond, int physFramesThisSecond);

		void SetShowDebugInfo(bool open);
		void SetUpdatePause(bool pause);

	private:
		void DrawPerfWindow();
		void DrawTextureCache();
		void DrawTextureInspector();

		void DrawPerformanceStats();
		void DrawRendererStats();
		void DrawWorldViewStats();
		void DrawImGuiStats();
		void DrawInputDebug();
		void DrawStatList(const Perf::Stats::FrameInfo &fi);

		void DrawCounter(CounterInfo &counter, const char *label, float min, float max, float height, bool drawStats = false);
		CounterInfo &GetCounter(CounterType ct);

		// Per-frame counters
		CounterInfo m_fpsCounter;
		CounterInfo m_physCounter;
		CounterInfo m_piguiCounter;

		// Per-second counters
		CounterInfo m_procMemCounter;
		CounterInfo m_luaMemCounter;

		MemoryInfo process_mem;
		size_t lua_mem = 0;
		float framesThisSecond = 0;
		float physFramesThisSecond = 0;

		float lastUpdateTime = 0;

		ImGuiState *m_state;
	};

} // namespace PiGui
