// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace Perf {
	/**
	* Simple atomic counter implementation for performance counters.
	*
	* Create counters by calling auto ref = stats->GetOrCreateCounter("Triangles Drawn").
	* Once you have a counter reference, you can call stats->CounterAdd(ref, 12) to update the counter.
	* The Counter* functions are thread-safe so long as they are called with a proper CounterRef.
	* Call FlushFrame() to collate all stats from the current frame and write them to the frame cache.
	*/
	class Stats {
	public:
		using FrameInfo = std::map<std::string, uint32_t>;

		// Simple opaque struct to make it more difficult to accidentally clobber memory or threading constraints
		struct CounterRef {
			CounterRef() = delete;
			explicit CounterRef(std::nullptr_t) :
				id(0) {}
			size_t id;

		private:
			friend class Stats;
			CounterRef(std::size_t t) :
				id(t) {}
		};

		CounterRef GetOrCreateCounter(std::string name, bool resetOnNewFrame = true);

		void CounterAdd(CounterRef ref, uint32_t amount) const
		{
			assert(ref.id != 0);
			m_counters.at(ref.id).ctr.fetch_add(amount);
		}

		void CounterSet(CounterRef ref, uint32_t value) const
		{
			assert(ref.id != 0);
			m_counters.at(ref.id).ctr.store(value);
		}

		void CounterReset(CounterRef ref) const
		{
			assert(ref.id != 0);
			m_counters.at(ref.id).ctr.store(0);
		}

		const FrameInfo &GetFrameStats() const { return m_frameCache; }
		const FrameInfo &GetPrevFrameStats() const { return m_prevFrameCache; };

		std::string GetNameForCounter(CounterRef ref) const { return m_definedCounters.at(ref.id); }

		// Terminate the current frame and make performance counters available with GetFrameStats().
		void FlushFrame();

	private:
		static const size_t MAX_FRAME_COUNT = 10;

		struct Counter {
			Counter(bool reset) :
				resetOnNewFrame(reset){};
			std::atomic<uint32_t> ctr;
			bool resetOnNewFrame;
		};

		// hashmap of counter name to counter value
		// mutable because the only thing we're modifying via non-const references is the atomic counters
		mutable std::map<std::size_t, Counter> m_counters;

		// Cache the previous two frames
		FrameInfo m_frameCache;
		FrameInfo m_prevFrameCache;

		// defined counter names
		std::map<std::size_t, std::string> m_definedCounters;

		// mutex used to synchronize updates to definedCounters
		std::mutex m_counterMutex;
		// store a std::hash instance so we don't have to recreate it every time we want to hash a string.
		std::hash<std::string> m_strHashFn;
	};

} // namespace Perf
