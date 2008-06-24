#ifndef _OBJIMPORT_H
#define _OBJIMPORT_H

#include <vector>
#include "vector3.h"

struct ObjVertex {
	vector3f p;
};

struct ObjTriangle {
	int v[3];
};

struct ObjMesh {
	std::vector<ObjVertex> vertices;
	std::vector<ObjTriangle> triangles;
	void Render();
};

ObjMesh *import_obj_mesh(const char *filename);

#endif /* _OBJIMPORT_H */
