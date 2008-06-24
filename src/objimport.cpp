#include <stdio.h>
#include <string.h>
#include "libs.h"
#include "objimport.h"
#include <GL/gl.h>

ObjMesh *import_obj_mesh(const char *filename)
{
	char buf[1024];
	FILE *f = fopen(filename, "r");
	if (!f) {
		fprintf(stderr, "Failed to load mesh %s\n", filename);
		return 0;
	}

	ObjMesh *m = new ObjMesh;
	float vsum[3];
	memset(vsum, 0, sizeof(float)*3);

	while (fgets(buf, sizeof(buf), f)) {
		if (buf[0] == '#') continue;
		ObjVertex v;
		if (sscanf(buf, "v %f %f %f", &v.p.x, &v.p.y, &v.p.z) == 3) {
			m->vertices.push_back(v);
			vsum[0] += v.p.x;
			vsum[1] += v.p.y;
			vsum[2] += v.p.z;
			continue;
		}
		ObjTriangle t;
		if (sscanf(buf, "f %d/%*d/ %d/%*d/ %d/%*d/", &t.v[0], &t.v[1], &t.v[2]) == 3) {
			t.v[0]--;
			t.v[1]--;
			t.v[2]--;
			m->triangles.push_back(t);
			continue;
		}
	}
	vsum[0] /= m->vertices.size();
	vsum[1] /= m->vertices.size();
	vsum[2] /= m->vertices.size();

	// locate model roughly at 0,0,0
	for (unsigned int i=0; i<m->vertices.size(); i++) {
		m->vertices[i].p.x -= vsum[0];
		m->vertices[i].p.y -= vsum[1];
		m->vertices[i].p.z -= vsum[2];
	}

	printf("%zd vertices\n", m->vertices.size());
	printf("%zd triangles\n", m->triangles.size());
	return m;
}

// why not make a smegging display list...	
void ObjMesh::Render()
{
	glBegin(GL_TRIANGLES);
	for (unsigned int i=0; i<triangles.size(); i++) {
		ObjTriangle &t = triangles[i];

		vector3f p0 = vertices[t.v[0]].p;
		vector3f p1 = vertices[t.v[1]].p;
		vector3f p2 = vertices[t.v[2]].p;

		vector3f n = -vector3f::Normalize(vector3f::Cross(p0-p2, p0-p1));

		glNormal3fv(&n[0]);
		glVertex3fv(&vertices[t.v[0]].p[0]);
		glVertex3fv(&vertices[t.v[1]].p[0]);
		glVertex3fv(&vertices[t.v[2]].p[0]);
	}
	glEnd();
}
