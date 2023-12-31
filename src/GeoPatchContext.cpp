// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GeoPatchContext.h"

#include "Pi.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "utils.h"

#include "perlin.h"
#include "vcacheopt/vcacheopt.h"
#include <algorithm>
#include <deque>

// static instances
int GeoPatchContext::m_edgeLen = 0;
int GeoPatchContext::m_numTris = 0;
double GeoPatchContext::m_frac = 0.0;
RefCountedPtr<Graphics::IndexBuffer> GeoPatchContext::m_indices;
int GeoPatchContext::m_prevEdgeLen = 0;

//static
void GeoPatchContext::GenerateIndices()
{
	if (m_prevEdgeLen == m_edgeLen)
		return;

	std::vector<Uint32> pl_short;

	int tri_count = 0;
	// calculate how many tri's there are
	tri_count = (VBO_COUNT_MID_IDX() / 3);
	for (int i = 0; i < 4; ++i) {
		tri_count += (VBO_COUNT_HI_EDGE() / 3);
	}

	// pre-allocate and fill enough space
	pl_short.assign(tri_count + VBO_COUNT_MID_IDX() + VBO_COUNT_HI_EDGE() * 4, 0);

	Output("GenerateIndices: triangles count = %i, mid indexes = %i, hi edges = %i\n", tri_count, VBO_COUNT_MID_IDX(), VBO_COUNT_HI_EDGE());

	// want vtx indices for tris
	Uint32 *idx = &pl_short[0];
	for (int x = 0; x < m_edgeLen - 1; x++) {
		for (int y = 0; y < m_edgeLen - 1; y++) {
			// 1st tri
			idx[0] = x + m_edgeLen * y;
			idx[1] = x + 1 + m_edgeLen * y;
			idx[2] = x + m_edgeLen * (y + 1);
			idx += 3;

			// 2nd tri
			idx[0] = x + 1 + m_edgeLen * y;
			idx[1] = x + 1 + m_edgeLen * (y + 1);
			idx[2] = x + m_edgeLen * (y + 1);
			idx += 3;
		}
	}
	// populate the N indices lists from the arrays built during InitTerrainIndices()
	// iterate over each index list and optimize it
	{
		VertexCacheOptimizerUInt vco;
#ifndef NDEBUG
		VertexCacheOptimizerUInt::Result res = vco.Optimize(&pl_short[0], tri_count);
		assert(0 == res);
#else
		vco.Optimize(&pl_short[0], tri_count);
#endif
		//create buffer & copy
		m_indices.Reset(Pi::renderer->CreateIndexBuffer(pl_short.size(), Graphics::BUFFER_USAGE_STATIC));
		Uint32 *idxPtr = m_indices->Map(Graphics::BUFFER_MAP_WRITE);
		for (Uint32 j = 0; j < pl_short.size(); j++) {
			idxPtr[j] = pl_short[j];
		}
		m_indices->Unmap();
	}

	m_prevEdgeLen = m_edgeLen;
}

void GeoPatchContext::Init()
{
	m_frac = 1.0 / double(m_edgeLen - 3);
	m_numTris = 2 * (m_edgeLen - 1) * (m_edgeLen - 1);

	GenerateIndices();
}
