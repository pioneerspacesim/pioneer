// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef __GEOPATCHID_H__
#define __GEOPATCHID_H__

#include <SDL_stdinc.h>

class GeoPatchID {
private:
	uint64_t mPatchID;

public:
	GeoPatchID(const uint64_t init) :
		mPatchID(init) {}

	static const uint64_t MAX_SHIFT_DEPTH = 61;

	uint64_t NextPatchID(const int depth, const int idx) const;
	int GetPatchIdx(const int depth) const;
	int GetPatchFaceIdx() const;
};

#endif //__GEOPATCHID_H__
