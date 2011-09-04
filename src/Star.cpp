#include "Star.h"

Star::Star() : TerrainBody()
{
}

Star::Star(SBody *sbody): TerrainBody(sbody)
{
	m_hasDoubleFrame = false;
}
