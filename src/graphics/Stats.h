// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _STATS_H
#define _STATS_H

#include "PerfStats.h"

#include "SDL_stdinc.h"
#include <utility>
#include <vector>

namespace Graphics {

	/**
	* Graphics::Stats is a wrapper over a PerfStats instance.
	* Use AddToStatCount(Stats::StatType, uint32_t) to easily update renderer stats.
	*/
	class Stats : protected Perf::Stats {
	public:
		static const Uint32 MAX_FRAMES_STORE = 30U;
		enum StatType {
			// renderer entries
			STAT_DRAWCALL = 0,
			STAT_NUM_POINTS,
			STAT_NUM_LINES,
			STAT_NUM_TRIS,

			// buffers
			STAT_CREATE_BUFFER,
			STAT_DESTROY_BUFFER,
			STAT_BUFFER_INUSE,
			STAT_DRAW_UNIFORM_BUFFER_INUSE,
			STAT_DRAW_UNIFORM_BUFFER_ALLOCS,
			STAT_DYNAMIC_DRAW_BUFFER_INUSE,
			STAT_DYNAMIC_DRAW_BUFFER_CREATED,

			STAT_NUM_RENDER_STATES,
			STAT_NUM_SHADER_PROGRAMS,
			STAT_NUM_CMDLIST_FLUSHES,

			// objects
			STAT_BUILDINGS,
			STAT_CITIES,
			STAT_GROUNDSTATIONS,
			STAT_SPACESTATIONS,
			STAT_ATMOSPHERES,
			STAT_PATCHES,
			STAT_PLANETS,
			STAT_GASGIANTS,
			STAT_STARS,
			STAT_SHIPS,

			// scenegraph entries
			STAT_BILLBOARD,

			// resource utilization stats
			STAT_NUM_TEXTURE2D,
			STAT_MEM_TEXTURE2D,
			STAT_NUM_TEXTURECUBE,
			STAT_MEM_TEXTURECUBE,
			STAT_NUM_TEXTUREARRAY2D,
			STAT_MEM_TEXTUREARRAY2D,

			MAX_STAT
		};

		struct TFrameData {
			uint32_t m_stats[MAX_STAT];
		};

		Stats();
		~Stats() {}

		void AddToStatCount(const StatType type, const uint32_t count) const
		{
			CounterAdd(m_counterRefs.at(type), count);
		}

		void DecStatCount(const StatType type, const uint32_t count) const
		{
			CounterDec(m_counterRefs.at(type), count);
		}

		void SetStatCount(const StatType type, const uint32_t count) const
		{
			CounterSet(m_counterRefs.at(type), count);
		}

		void NextFrame();

		const TFrameData &FrameStatsPrevious() const;
		const FrameInfo &GetFullStats() const { return GetFrameStats(); }

	private:
		TFrameData m_frameStats[MAX_FRAMES_STORE];
		Uint32 m_currentFrame;

		std::vector<Perf::Stats::CounterRef> m_counterRefs;
	};

} // namespace Graphics

#endif
