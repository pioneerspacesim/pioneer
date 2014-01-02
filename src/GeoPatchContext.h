// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOPATCHCONTEXT_H
#define _GEOPATCHCONTEXT_H

#include <SDL_stdinc.h>

#include "vector3.h"
#include "Random.h"
#include "galaxy/StarSystem.h"
#include "graphics/Material.h"
#include "terrain/Terrain.h"
#include "GeoPatchID.h"

#include <deque>

static const int GEOPATCH_MAX_EDGELEN = 55;

// hold the 16 possible terrain edge connections
static const int NUM_INDEX_LISTS = 16;

namespace Graphics { class Renderer; }
class SystemBody;
class GeoPatch;
class GeoPatchContext;
class GeoSphere;

class GeoPatchContext : public RefCounted {
public:
	#pragma pack(4)
	struct VBOVertex
	{
		float x,y,z;
		float nx,ny,nz;
		unsigned char col[4];
		float padding;
	};
	#pragma pack()

	int edgeLen;

	inline int VBO_COUNT_LO_EDGE() const { return 3*(edgeLen/2); }
	inline int VBO_COUNT_HI_EDGE() const { return 3*(edgeLen-1); }
	inline int VBO_COUNT_MID_IDX() const { return (4*3*(edgeLen-3))    + 2*(edgeLen-3)*(edgeLen-3)*3; }
	//                                            ^^ serrated teeth bit  ^^^ square inner bit

	inline int IDX_VBO_LO_OFFSET(int i) const { return i*sizeof(unsigned short)*3*(edgeLen/2); }
	inline int IDX_VBO_HI_OFFSET(int i) const { return (i*sizeof(unsigned short)*VBO_COUNT_HI_EDGE())+IDX_VBO_LO_OFFSET(4); }
	inline int IDX_VBO_MAIN_OFFSET()    const { return IDX_VBO_HI_OFFSET(4); }
	inline int IDX_VBO_COUNT_ALL_IDX()	const { return ((edgeLen-1)*(edgeLen-1))*2*3; }

	inline int NUMVERTICES() const { return edgeLen*edgeLen; }

	double frac;

	std::unique_ptr<unsigned short[]> midIndices;
	std::unique_ptr<unsigned short[]> loEdgeIndices[4];
	std::unique_ptr<unsigned short[]> hiEdgeIndices[4];
	GLuint indices_vbo;
	GLuint indices_list[NUM_INDEX_LISTS];
	GLuint indices_tri_count;
	GLuint indices_tri_counts[NUM_INDEX_LISTS];
	VBOVertex *vbotemp;

	GeoPatchContext(int _edgeLen) : edgeLen(_edgeLen) {
		Init();
	}

	~GeoPatchContext() {
		Cleanup();
	}

	void Refresh() {
		Cleanup();
		Init();
	}

	void Cleanup();

	void updateIndexBufferId(const GLuint edge_hi_flags);

	int getIndices(std::vector<unsigned short> &pl, const unsigned int edge_hi_flags);

	void Init();
};

#endif /* _GEOPATCHCONTEXT_H */
