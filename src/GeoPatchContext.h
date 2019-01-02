// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOPATCHCONTEXT_H
#define _GEOPATCHCONTEXT_H

#include <SDL_stdinc.h>

#include "GeoPatchID.h"
#include "Random.h"
#include "galaxy/StarSystem.h"
#include "graphics/Material.h"
#include "graphics/VertexBuffer.h"
#include "terrain/Terrain.h"
#include "vector3.h"

#include <deque>

// maximumpatch depth
#define GEOPATCH_MAX_DEPTH 15 + (2 * Pi::detail.fracmult) //15

namespace Graphics {
	class Renderer;
}
class SystemBody;
class GeoPatch;
class GeoSphere;

class GeoPatchContext : public RefCounted {
private:
	static int edgeLen;
	static int numTris;

	static double frac;

	static inline int VBO_COUNT_HI_EDGE() { return 3 * (edgeLen - 1); }
	static inline int VBO_COUNT_MID_IDX() { return (4 * 3 * (edgeLen - 3)) + 2 * (edgeLen - 3) * (edgeLen - 3) * 3; }
	//                                            ^^ serrated teeth bit  ^^^ square inner bit

	static inline int IDX_VBO_LO_OFFSET(const int i) { return i * sizeof(Uint32) * 3 * (edgeLen / 2); }
	static inline int IDX_VBO_HI_OFFSET(const int i) { return (i * sizeof(Uint32) * VBO_COUNT_HI_EDGE()) + IDX_VBO_LO_OFFSET(4); }

	static RefCountedPtr<Graphics::IndexBuffer> indices;
	static int prevEdgeLen;

	static void GenerateIndices();

public:
	struct VBOVertex {
		vector3f pos;
		vector3f norm;
		Color4ub col;
		vector2f uv;
	};

	GeoPatchContext(const int _edgeLen)
	{
		edgeLen = _edgeLen + 2; // +2 for the skirt
		Init();
	}

	static void Refresh()
	{
		Init();
	}

	static void Init();

	static inline Graphics::IndexBuffer *GetIndexBuffer() { return indices.Get(); }

	static inline int NUMVERTICES() { return edgeLen * edgeLen; }

	static inline int GetEdgeLen() { return edgeLen; }
	static inline int GetNumTris() { return numTris; }
	static inline double GetFrac() { return frac; }
};

#endif /* _GEOPATCHCONTEXT_H */
