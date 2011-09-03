#include "TerrainBody.h"

TerrainBody::TerrainBody(SBody *sbody) : Body(), m_sbody(sbody)
{
}

TerrainBody::TerrainBody() : Body(), m_sbody(0)
{
}

void TerrainBody::InitTerrainBody()
{
	assert(m_sbody);
}
