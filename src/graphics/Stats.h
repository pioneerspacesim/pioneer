// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _STATS_H
#define _STATS_H

#include "SDL_stdinc.h"
#include <utility>
#include <vector>

namespace Graphics {

	class Stats {
	public:
		static const Uint32 MAX_FRAMES_STORE = 30U;
		enum StatType {
			// renderer entries
			STAT_DRAWCALL = 0,
			STAT_DRAWTRIS,
			STAT_DRAWPOINTSPRITES,

			// buffers
			STAT_CREATE_BUFFER,
			STAT_DESTROY_BUFFER,

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

			MAX_STAT
		};

		struct TFrameData {
			Uint32 m_stats[MAX_STAT];
		};

		Stats();
		~Stats() {}

		void AddToStatCount(const StatType type, const Uint32 count);
		void NextFrame();

		const TFrameData &FrameStats() const { return m_frameStats[m_currentFrame]; }
		const TFrameData &FrameStatsPrevious() const;

	private:
		TFrameData m_frameStats[MAX_FRAMES_STORE];
		Uint32 m_currentFrame;
	};

} // namespace Graphics

#endif
