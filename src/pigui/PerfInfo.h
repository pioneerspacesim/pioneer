// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "PerfStats.h"
#include "RefCounted.h"
#include <array>
#include <memory>

namespace PiGui {

	class PerfInfo {
	public:
		PerfInfo();
		~PerfInfo();

		enum CounterType {
			COUNTER_FPS,
			COUNTER_PHYS,
			COUNTER_PIGUI
		};

		// Information about the current process memory usage in KB.
		struct MemoryInfo {
			size_t currentMemSize;
			size_t peakMemSize;
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

		void DrawRendererStats();
		void DrawWorldViewStats();
		void DrawImGuiStats();
		void DrawInputDebug();
		void DrawStatList(const Perf::Stats::FrameInfo &fi);

		static const int NUM_FRAMES = 60;
		struct CounterInfo {
			std::array<float, NUM_FRAMES> history;
			float average = 0.;
			float min = 0.;
			float max = 0.;
		};

		CounterInfo &GetCounter(CounterType ct);

		CounterInfo m_fpsCounter;
		CounterInfo m_physCounter;
		CounterInfo m_piguiCounter;

		MemoryInfo process_mem;
		size_t lua_mem = 0;
		float framesThisSecond = 0;
		float physFramesThisSecond = 0;

		float lastUpdateTime = 0;

		ImGuiState *m_state;
	};

} // namespace PiGui
