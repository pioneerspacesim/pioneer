// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GeoPatchContext.h"
#include "Pi.h"
#include "RefCounted.h"
#include "graphics/Frustum.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"

#include "perlin.h"
#include "vcacheopt/vcacheopt.h"
#include <algorithm>
#include <deque>

// static instances
int GeoPatchContext::edgeLen = 0;
int GeoPatchContext::numTris = 0;
double GeoPatchContext::frac = 0.0;
RefCountedPtr<Graphics::IndexBuffer> GeoPatchContext::indices;
int GeoPatchContext::prevEdgeLen = 0;

//static
void GeoPatchContext::GenerateIndices()
{
	if (prevEdgeLen == edgeLen)
		return;

	//
	Uint32 *idx;
	std::vector<Uint32> pl_short;

	int tri_count = 0;
	{
		// calculate how many tri's there are
		tri_count = (VBO_COUNT_MID_IDX() / 3);
		for (int i = 0; i < 4; ++i) {
			tri_count += (VBO_COUNT_HI_EDGE() / 3);
		}

		// pre-allocate enough space
		pl_short.reserve(tri_count);

		// add all of the middle indices
		for (int i = 0; i < VBO_COUNT_MID_IDX(); ++i) {
			pl_short.push_back(0);
		}
		// add the HI detail indices
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < VBO_COUNT_HI_EDGE(); ++j) {
				pl_short.push_back(0);
			}
		}
	}
	// want vtx indices for tris
	idx = &pl_short[0];
	for (int x = 0; x < edgeLen - 1; x++) {
		for (int y = 0; y < edgeLen - 1; y++) {
			// 1st tri
			idx[0] = x + edgeLen * y;
			idx[1] = x + 1 + edgeLen * y;
			idx[2] = x + edgeLen * (y + 1);
			idx += 3;

			// 2nd tri
			idx[0] = x + 1 + edgeLen * y;
			idx[1] = x + 1 + edgeLen * (y + 1);
			idx[2] = x + edgeLen * (y + 1);
			idx += 3;
		}
	}

	// populate the N indices lists from the arrays built during InitTerrainIndices()
	// iterate over each index list and optimize it
	{
		VertexCacheOptimizerUInt vco;
		VertexCacheOptimizerUInt::Result res = vco.Optimize(&pl_short[0], tri_count);
		assert(0 == res);
		//create buffer & copy
		indices.Reset(Pi::renderer->CreateIndexBuffer(pl_short.size(), Graphics::BUFFER_USAGE_STATIC));
		Uint32 *idxPtr = indices->Map(Graphics::BUFFER_MAP_WRITE);
		for (Uint32 j = 0; j < pl_short.size(); j++) {
			idxPtr[j] = pl_short[j];
		}
		indices->Unmap();
	}

	prevEdgeLen = edgeLen;
}

void GeoPatchContext::Init()
{
	frac = 1.0 / double(edgeLen - 3);
	numTris = 2 * (edgeLen - 1) * (edgeLen - 1);

	GenerateIndices();
}
