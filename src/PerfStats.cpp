// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PerfStats.h"
#include "utils.h"
#include <mutex>
#include <stdexcept>
#include <tuple>
#include <utility>

using namespace Perf;

Stats::CounterRef Stats::GetOrCreateCounter(std::string str, bool resetOnNewFrame)
{
	size_t hash = m_strHashFn(str);
	if (hash == 0)
		throw std::runtime_error("Hash of string " + str + " equals zero. This should never happen.");

	m_counterMutex.lock();
	m_definedCounters.emplace(hash, str);

	// Use std::piecewise_construct, as it's the only overload for non-copyable,
	// non-movable objects like std::atomic
	m_counters.emplace(std::piecewise_construct,
		std::forward_as_tuple(hash),
		std::forward_as_tuple(resetOnNewFrame));
	m_counterMutex.unlock();

	return CounterRef(hash);
}

void Stats::FlushFrame()
{
	m_counterMutex.lock();

	FrameInfo lastFrame;

	try {
		for (auto iter = m_counters.begin(); iter != m_counters.end(); iter++) {
			lastFrame.emplace(m_definedCounters[iter->first], iter->second.ctr.load());
			if (iter->second.resetOnNewFrame && !m_neverReset)
				iter->second.ctr.store(0);
		}

		m_frameCache = lastFrame;
	} catch (std::runtime_error &e) {
		Output("Error updating stats for frame: %s\n", e.what());
	}

	m_counterMutex.unlock();
}
