// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Stats.h"
#include "libs.h"
#include <cassert>

namespace Graphics {

	Stats::Stats() : m_currentFrame(0U)
	{
		memset(&m_frameStats[0], 0, sizeof(TFrameData) * MAX_FRAMES_STORE);
	}

	void Stats::AddToStatCount(const StatType type, const Uint32 count)
	{
		m_frameStats[m_currentFrame].m_stats[type] += count;
	}

	void Stats::NextFrame()
	{
		++m_currentFrame;
		if(m_currentFrame >= MAX_FRAMES_STORE) {
			m_currentFrame = 0;
		}
		assert(m_currentFrame >= 0 && m_currentFrame < MAX_FRAMES_STORE);
		memset(&m_frameStats[m_currentFrame], 0, sizeof(TFrameData));
	}

	const Stats::TFrameData& Stats::FrameStatsPrevious() const {
		return m_frameStats[Clamp(m_currentFrame-1, 0U, MAX_FRAMES_STORE-1)];
	}
}
