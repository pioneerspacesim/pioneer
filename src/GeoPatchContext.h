// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOPATCHCONTEXT_H
#define _GEOPATCHCONTEXT_H

#include <SDL_stdinc.h>

#include "vector3.h"
#include "Random.h"
#include "galaxy/StarSystem.h"
#include "graphics/Material.h"
#include "graphics/VertexBuffer.h"
#include "terrain/Terrain.h"
#include "GeoPatchID.h"

#include <deque>

// hold the 16 possible terrain edge connections
static const unsigned NUM_INDEX_LISTS = 16;

// maximumpatch depth
#define GEOPATCH_MAX_DEPTH  15 + (2*Pi::detail.fracmult) //15

namespace Graphics { class Renderer; }
class SystemBody;
class GeoPatch;
class GeoSphere;

class GeoPatchContext : public RefCounted {
private:
	static int edgeLen;
	static int numTris;

	static double frac;

	static inline int VBO_COUNT_LO_EDGE() { return 3*(edgeLen/2); }
	static inline int VBO_COUNT_HI_EDGE() { return 3*(edgeLen-1); }
	static inline int VBO_COUNT_MID_IDX() { return (4*3*(edgeLen-3)) + 2*(edgeLen-3)*(edgeLen-3)*3; }
	//                                            ^^ serrated teeth bit  ^^^ square inner bit

	static inline int IDX_VBO_LO_OFFSET(const int i) { return i*sizeof(unsigned short)*3*(edgeLen/2); }
	static inline int IDX_VBO_HI_OFFSET(const int i) { return (i*sizeof(unsigned short)*VBO_COUNT_HI_EDGE())+IDX_VBO_LO_OFFSET(4); }

	static std::unique_ptr<unsigned short[]> midIndices;
	static std::unique_ptr<unsigned short[]> loEdgeIndices[4];
	static std::unique_ptr<unsigned short[]> hiEdgeIndices[4];
	static RefCountedPtr<Graphics::IndexBuffer> indices_list[NUM_INDEX_LISTS];
	static int prevEdgeLen;

	static int GetIndices(std::vector<unsigned short> &pl, const unsigned int edge_hi_flags);
	static void GenerateIndices();

public:
	#pragma pack(push, 4)
	struct VBOVertex
	{
		vector3f pos;
		vector3f norm;
		Color4ub col;
	};
	#pragma pack(pop)

	GeoPatchContext(const int _edgeLen) {
		edgeLen = _edgeLen;
		Init();
	}

	~GeoPatchContext() {
		Cleanup();
	}

	static void Refresh() {
		Cleanup();
		Init();
	}

	static void Cleanup();
	static void Init();

	static inline Graphics::IndexBuffer* GetIndexBuffer(const Uint32 idx) { return indices_list[idx].Get(); }

	static inline int NUMVERTICES() { return edgeLen*edgeLen; }

	static inline int GetEdgeLen() { return edgeLen; }
	static inline int GetNumTris() { return numTris; }
	static inline double GetFrac() { return frac; }
};

#endif /* _GEOPATCHCONTEXT_H */
