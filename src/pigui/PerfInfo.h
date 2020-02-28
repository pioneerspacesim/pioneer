// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include <array>

namespace PiGUI {

	class PerfInfo {
	public:
		// Information about the current process memory usage in KB.
		struct MemoryInfo {
			size_t currentMemSize;
			size_t peakMemSize;
		};

		void Draw();

		// Update with current frame and physics update time in ms
		void Update(float frameTime, float physTime);
		void UpdateFrameInfo(int framesThisSecond, int physFramesThisSecond);

		void SetUpdatePause(bool pause) { m_updatePause = pause; }

	private:
		static const int NUM_FRAMES = 60;
		std::array<float, NUM_FRAMES> m_fpsGraph;
		std::array<float, NUM_FRAMES> m_physFpsGraph;
		float frameTimeAverage;
		float frameTimeMax;
		float frameTimeMin;
		float physFrameTimeAverage;
		float physFrameTimeMax;
		float physFrameTimeMin;

		MemoryInfo process_mem;
		size_t lua_mem;
		float framesThisSecond;
		float physFramesThisSecond;

		bool m_updatePause;
	};

} // namespace PiGUI
