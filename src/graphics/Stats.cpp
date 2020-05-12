// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Stats.h"
#include "libs.h"
#include <cassert>

namespace Graphics {

	Stats::Stats() :
		Perf::Stats(),
		m_currentFrame(0U)
	{
		memset(&m_frameStats[0], 0, sizeof(TFrameData) * MAX_FRAMES_STORE);

		m_counterRefs = {
			GetOrCreateCounter("DrawBuffer Calls"),
			GetOrCreateCounter("DrawTriangles Calls"),
			GetOrCreateCounter("DrawPointSprites Calls"),

			GetOrCreateCounter("Buffers Created"),
			GetOrCreateCounter("Buffers Destroyed"),
			GetOrCreateCounter("Buffers In Use", false),

			GetOrCreateCounter("Num Buildings"),
			GetOrCreateCounter("Num Cities"),
			GetOrCreateCounter("Num Ground Stations"),
			GetOrCreateCounter("Num Space Stations"),
			GetOrCreateCounter("Num Atmospheres"),
			GetOrCreateCounter("Num GeoPatches"),
			GetOrCreateCounter("Num Planets"),
			GetOrCreateCounter("Num Gas Giants"),
			GetOrCreateCounter("Num Stars"),
			GetOrCreateCounter("Num Ships"),

			GetOrCreateCounter("Num Billboards"),

			GetOrCreateCounter("Texture2D Count", false),
			GetOrCreateCounter("Texture2D Memory Used", false),
			GetOrCreateCounter("TextureCube Count", false),
			GetOrCreateCounter("TextureCube Memory Used", false),
			GetOrCreateCounter("TextureArray2D Count", false),
			GetOrCreateCounter("TextureArray2D Memory Used", false)
		};
	}

	void Stats::NextFrame()
	{
		Perf::Stats::FlushFrame();

		TFrameData &frame = m_frameStats[m_currentFrame];
		auto &statsFrame = GetFrameStats();
		for (size_t idx = 0; idx < m_counterRefs.size(); idx++) {
			const std::string name = GetNameForCounter(m_counterRefs[idx]);
			frame.m_stats[idx] = statsFrame.at(name);
		}

		m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_STORE;
		memset(&m_frameStats[m_currentFrame], 0, sizeof(TFrameData));
	}

	const Stats::TFrameData &Stats::FrameStatsPrevious() const
	{
		uint32_t frameIdx = m_currentFrame > 0 ? m_currentFrame - 1 : MAX_FRAMES_STORE - 1;
		return m_frameStats[Clamp(m_currentFrame - 1, 0U, MAX_FRAMES_STORE - 1)];
	}

} // namespace Graphics
