#ifndef _SURFACE_H
#define _SURFACE_H

#include "Renderer.h"

// surface with a material
// can have indices
class Surface {
public:
	Surface();
	int GetNumVerts() const;
	std::vector<unsigned short> indices;
	VertexArray *vertices;
	Material* mat;
	PrimitiveType primitiveType;

	// multiple surfaces can be buffered in one vbo so need to
	// save starting offset + amount to draw
	//int startVertex;
	//int numVertices; should be samme as vertices->GetNumVerts()
	//int startIndex;
	//int numIndices; should be same as indices.size()
};

#endif
