// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "GeoPatchContext.h"
#include "perlin.h"
#include "Pi.h"
#include "RefCounted.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/Frustum.h"
#include "graphics/Graphics.h"
#include "graphics/VertexArray.h"
#include "graphics/gl2/GeoSphereMaterial.h"
#include "vcacheopt/vcacheopt.h"
#include <deque>
#include <algorithm>



void GeoPatchContext::Cleanup() {
	midIndices.reset();
	for (int i=0; i<4; i++) {
		loEdgeIndices[i].reset();
		hiEdgeIndices[i].reset();
	}
	if (indices_vbo) {
		indices_vbo = 0;
	}
	for (int i=0; i<NUM_INDEX_LISTS; i++) {
		if (indices_list[i]) {
			glDeleteBuffersARB(1, &indices_list[i]);
		}
	}
	delete [] vbotemp;
}

void GeoPatchContext::updateIndexBufferId(const GLuint edge_hi_flags) {
	assert(edge_hi_flags < GLuint(NUM_INDEX_LISTS));
	indices_vbo = indices_list[edge_hi_flags];
	indices_tri_count = indices_tri_counts[edge_hi_flags];
}

int GeoPatchContext::getIndices(std::vector<unsigned short> &pl, const unsigned int edge_hi_flags)
{
	// calculate how many tri's there are
	int tri_count = (VBO_COUNT_MID_IDX() / 3);
	for( int i=0; i<4; ++i ) {
		if( edge_hi_flags & (1 << i) ) {
			tri_count += (VBO_COUNT_HI_EDGE() / 3);
		} else {
			tri_count += (VBO_COUNT_LO_EDGE() / 3);
		}
	}

	// pre-allocate enough space
	pl.reserve(tri_count);

	// add all of the middle indices
	for(int i=0; i<VBO_COUNT_MID_IDX(); ++i) {
		pl.push_back(midIndices[i]);
	}
	// selectively add the HI or LO detail indices
	for (int i=0; i<4; i++) {
		if( edge_hi_flags & (1 << i) ) {
			for(int j=0; j<VBO_COUNT_HI_EDGE(); ++j) {
				pl.push_back(hiEdgeIndices[i][j]);
			}
		} else {
			for(int j=0; j<VBO_COUNT_LO_EDGE(); ++j) {
				pl.push_back(loEdgeIndices[i][j]);
			}
		}
	}

	return tri_count;
}

void GeoPatchContext::Init() {
	frac = 1.0 / double(edgeLen-1);

	vbotemp = new VBOVertex[NUMVERTICES()];

	unsigned short *idx;
	midIndices.reset(new unsigned short[VBO_COUNT_MID_IDX()]);
	for (int i=0; i<4; i++) {
		loEdgeIndices[i].reset(new unsigned short[VBO_COUNT_LO_EDGE()]);
		hiEdgeIndices[i].reset(new unsigned short[VBO_COUNT_HI_EDGE()]);
	}
	/* also want vtx indices for tris not touching edge of patch */
	idx = midIndices.get();
	for (int x=1; x<edgeLen-2; x++) {
		for (int y=1; y<edgeLen-2; y++) {
			idx[0] = x + edgeLen*y;
			idx[1] = x+1 + edgeLen*y;
			idx[2] = x + edgeLen*(y+1);
			idx+=3;

			idx[0] = x+1 + edgeLen*y;
			idx[1] = x+1 + edgeLen*(y+1);
			idx[2] = x + edgeLen*(y+1);
			idx+=3;
		}
	}
	{
		for (int x=1; x<edgeLen-3; x+=2) {
			// razor teeth near edge 0
			idx[0] = x + edgeLen;
			idx[1] = x+1;
			idx[2] = x+1 + edgeLen;
			idx+=3;
			idx[0] = x+1;
			idx[1] = x+2 + edgeLen;
			idx[2] = x+1 + edgeLen;
			idx+=3;
		}
		for (int x=1; x<edgeLen-3; x+=2) {
			// near edge 2
			idx[0] = x + edgeLen*(edgeLen-2);
			idx[1] = x+1 + edgeLen*(edgeLen-2);
			idx[2] = x+1 + edgeLen*(edgeLen-1);
			idx+=3;
			idx[0] = x+1 + edgeLen*(edgeLen-2);
			idx[1] = x+2 + edgeLen*(edgeLen-2);
			idx[2] = x+1 + edgeLen*(edgeLen-1);
			idx+=3;
		}
		for (int y=1; y<edgeLen-3; y+=2) {
			// near edge 1
			idx[0] = edgeLen-2 + y*edgeLen;
			idx[1] = edgeLen-1 + (y+1)*edgeLen;
			idx[2] = edgeLen-2 + (y+1)*edgeLen;
			idx+=3;
			idx[0] = edgeLen-2 + (y+1)*edgeLen;
			idx[1] = edgeLen-1 + (y+1)*edgeLen;
			idx[2] = edgeLen-2 + (y+2)*edgeLen;
			idx+=3;
		}
		for (int y=1; y<edgeLen-3; y+=2) {
			// near edge 3
			idx[0] = 1 + y*edgeLen;
			idx[1] = 1 + (y+1)*edgeLen;
			idx[2] = (y+1)*edgeLen;
			idx+=3;
			idx[0] = 1 + (y+1)*edgeLen;
			idx[1] = 1 + (y+2)*edgeLen;
			idx[2] = (y+1)*edgeLen;
			idx+=3;
		}
	}
	// full detail edge triangles
	{
		idx = hiEdgeIndices[0].get();
		for (int x=0; x<edgeLen-1; x+=2) {
			idx[0] = x; idx[1] = x+1; idx[2] = x+1 + edgeLen;
			idx+=3;
			idx[0] = x+1; idx[1] = x+2; idx[2] = x+1 + edgeLen;
			idx+=3;
		}
		idx = hiEdgeIndices[1].get();
		for (int y=0; y<edgeLen-1; y+=2) {
			idx[0] = edgeLen-1 + y*edgeLen;
			idx[1] = edgeLen-1 + (y+1)*edgeLen;
			idx[2] = edgeLen-2 + (y+1)*edgeLen;
			idx+=3;
			idx[0] = edgeLen-1 + (y+1)*edgeLen;
			idx[1] = edgeLen-1 + (y+2)*edgeLen;
			idx[2] = edgeLen-2 + (y+1)*edgeLen;
			idx+=3;
		}
		idx = hiEdgeIndices[2].get();
		for (int x=0; x<edgeLen-1; x+=2) {
			idx[0] = x + (edgeLen-1)*edgeLen;
			idx[1] = x+1 + (edgeLen-2)*edgeLen;
			idx[2] = x+1 + (edgeLen-1)*edgeLen;
			idx+=3;
			idx[0] = x+1 + (edgeLen-2)*edgeLen;
			idx[1] = x+2 + (edgeLen-1)*edgeLen;
			idx[2] = x+1 + (edgeLen-1)*edgeLen;
			idx+=3;
		}
		idx = hiEdgeIndices[3].get();
		for (int y=0; y<edgeLen-1; y+=2) {
			idx[0] = y*edgeLen;
			idx[1] = 1 + (y+1)*edgeLen;
			idx[2] = (y+1)*edgeLen;
			idx+=3;
			idx[0] = (y+1)*edgeLen;
			idx[1] = 1 + (y+1)*edgeLen;
			idx[2] = (y+2)*edgeLen;
			idx+=3;
		}
	}
	// these edge indices are for patches with no
	// neighbour of equal or greater detail -- they reduce
	// their edge complexity by 1 division
	{
		idx = loEdgeIndices[0].get();
		for (int x=0; x<edgeLen-2; x+=2) {
			idx[0] = x;
			idx[1] = x+2;
			idx[2] = x+1+edgeLen;
			idx += 3;
		}
		idx = loEdgeIndices[1].get();
		for (int y=0; y<edgeLen-2; y+=2) {
			idx[0] = (edgeLen-1) + y*edgeLen;
			idx[1] = (edgeLen-1) + (y+2)*edgeLen;
			idx[2] = (edgeLen-2) + (y+1)*edgeLen;
			idx += 3;
		}
		idx = loEdgeIndices[2].get();
		for (int x=0; x<edgeLen-2; x+=2) {
			idx[0] = x+edgeLen*(edgeLen-1);
			idx[2] = x+2+edgeLen*(edgeLen-1);
			idx[1] = x+1+edgeLen*(edgeLen-2);
			idx += 3;
		}
		idx = loEdgeIndices[3].get();
		for (int y=0; y<edgeLen-2; y+=2) {
			idx[0] = y*edgeLen;
			idx[2] = (y+2)*edgeLen;
			idx[1] = 1 + (y+1)*edgeLen;
			idx += 3;
		}
	}

	// these will hold the optimised indices
	std::vector<unsigned short> pl_short[NUM_INDEX_LISTS];
	// populate the N indices lists from the arrays built during InitTerrainIndices()
	for( int i=0; i<NUM_INDEX_LISTS; ++i ) {
		const unsigned int edge_hi_flags = i;
		indices_tri_counts[i] = getIndices(pl_short[i], edge_hi_flags);
	}

	// iterate over each index list and optimize it
	for( int i=0; i<NUM_INDEX_LISTS; ++i ) {
		int tri_count = indices_tri_counts[i];
		VertexCacheOptimizerUShort vco;
		VertexCacheOptimizerUShort::Result res = vco.Optimize(&pl_short[i][0], tri_count);
		assert(0 == res);
	}

	// everything should be hunky-dory for setting up as OpenGL index buffers now.
	for( int i=0; i<NUM_INDEX_LISTS; ++i ) {
		glGenBuffersARB(1, &indices_list[i]);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, indices_list[i]);
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*indices_tri_counts[i]*3, &(pl_short[i][0]), GL_STATIC_DRAW);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	// default it to the last entry which uses the hi-res borders
	indices_vbo			= indices_list[NUM_INDEX_LISTS-1];
	indices_tri_count	= indices_tri_counts[NUM_INDEX_LISTS-1];

	if (midIndices) {
		midIndices.reset();
		for (int i=0; i<4; i++) {
			loEdgeIndices[i].reset();
			hiEdgeIndices[i].reset();
		}
	}
}

