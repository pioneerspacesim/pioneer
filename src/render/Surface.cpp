#include "Surface.h"

Surface::Surface() :
	vertices(0), mat(0), primitiveType(TRIANGLES)
{
}

int Surface::GetNumVerts() const
{
	if (vertices)
		return vertices->position.size();
	else
		return 0;
}
