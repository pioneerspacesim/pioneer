#include "RendererLegacy.h"
#include "Render.h"
#include "Texture.h"
#include "Light.h"
#include <stddef.h> //for offsetof

RendererLegacy::RendererLegacy(int w, int h) :
	Renderer(w, h)
{
	glShadeModel(GL_SMOOTH);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	glClearColor(0,0,0,0);

	glViewport(0, 0, m_width, m_height);
}

RendererLegacy::~RendererLegacy()
{

}

bool RendererLegacy::BeginFrame()
{
	Render::PrepareFrame();
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return true;
}

bool RendererLegacy::EndFrame()
{
	Render::PostProcess();
	return true;
}

bool RendererLegacy::SwapBuffers()
{
	glError();
	Render::SwapBuffers();
	return true;
}

bool RendererLegacy::SetBlendMode(BlendMode m)
{
	//where does SRC_ALPHA, ONE fit in?
	switch (m) {
	case BLEND_SOLID:
		glDisable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ZERO);
		break;
	case BLEND_ADDITIVE:
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		break;
	case BLEND_ALPHA:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BLEND_ALPHA_ONE:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case BLEND_ALPHA_PREMULT:
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	default:
		return false;
	}
	return true;
}

bool RendererLegacy::SetLights(int numlights, const Light *lights)
{
	if (numlights < 1) return false;

	m_numLights = numlights;

	for (int i=0; i < numlights; i++) {
		const Light &l = lights[i];
		// directional lights have the length of 1
		const float pos[] = {
			l.GetPosition().x,
			l.GetPosition().y,
			l.GetPosition().z,
			l.GetType() == Light::LIGHT_DIRECTIONAL ? 1.f : 0.f
		};
		glLightfv(GL_LIGHT0+i, GL_POSITION, pos);
		glLightfv(GL_LIGHT0+i, GL_DIFFUSE, l.GetDiffuse());
		glLightfv(GL_LIGHT0+i, GL_AMBIENT, l.GetAmbient());
		glLightfv(GL_LIGHT0+i, GL_SPECULAR, l.GetSpecular());
		glEnable(GL_LIGHT0+i);
	}

	return true;
}

bool RendererLegacy::SetAmbientColor(const Color &c)
{
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, c);

	return true;
}

bool RendererLegacy::DrawLines(int count, const LineVertex *v, LineType type)
{
	if (count < 2) return false;

	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	//this is easy to upgrade to GL3/ES2 level
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(LineVertex), &v[0].position);
	glColorPointer(4, GL_FLOAT, sizeof(LineVertex), &v[0].color);
	glDrawArrays(type, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glPopAttrib();

	return true;
}

bool RendererLegacy::DrawLines2D(int count, const LineVertex2D *v, LineType type)
{
	if (count < 2) return false;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof(LineVertex2D), &v[0].position);
	glColorPointer(4, GL_FLOAT, sizeof(LineVertex2D), &v[0].color);
	glDrawArrays(type, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	return true;
}

bool RendererLegacy::DrawTriangles2D(const VertexArray *v, const Material *m, PrimitiveType t)
{
	if (!v || v->GetNumVerts() < 3) return false;

	glPushAttrib(GL_ENABLE_BIT);
	// XXX assuming GUI+unlit
	glDisable(GL_LIGHTING);

	const bool diffuse = !v->diffuse.empty();
	const bool textured = (m && m->texture0 && v->uv0.size() == v->position.size());
	const unsigned int numverts = v->position.size();

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->position[0]));
	if (diffuse) {
		assert(v->diffuse.size() == v->position.size());
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->diffuse[0]));
	}
	if (textured) {
		assert(v->uv0.size() == v->position.size());
		glEnable(GL_TEXTURE_2D);
		m->texture0->Bind();
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->uv0[0]));
	}

	glDrawArrays(t, 0, numverts);
	glDisableClientState(GL_VERTEX_ARRAY);

	if (diffuse)
		glDisableClientState(GL_COLOR_ARRAY);
	if (textured) {
		m->texture0->Unbind();
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	glPopAttrib();

	return true;
}

bool RendererLegacy::DrawTriangles(const VertexArray *v, const Material *m, PrimitiveType t)
{
	if (!v || v->position.size() < 3) return false;

	glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT);
	if (!m || m->unlit) glDisable(GL_LIGHTING);

	const bool diffuse = !v->diffuse.empty();
	const bool textured = (m && m->texture0 && v->uv0.size() == v->position.size());
	const bool normals = !v->normal.empty();
	const unsigned int numverts = v->position.size();
	const bool twoSided = (m && m->twoSided);

	if (twoSided) {
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
		glDisable(GL_CULL_FACE);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->position[0]));
	if (diffuse) {
		assert(v->diffuse.size() == v->position.size());
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->diffuse[0]));
	}
	if (normals) {
		assert(v->normal.size() == v->position.size());
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->normal[0]));
	}
	if (textured) {
		assert(v->uv0.size() == v->position.size());
		glEnable(GL_TEXTURE_2D);
		m->texture0->Bind();
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->uv0[0]));
	}
	glDrawArrays(t, 0, numverts);
	glDisableClientState(GL_VERTEX_ARRAY);
	if (diffuse)
		glDisableClientState(GL_COLOR_ARRAY);
	if (normals)
		glDisableClientState(GL_NORMAL_ARRAY);
	if (textured) {
		m->texture0->Unbind();
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
	}

	glPopAttrib();

	return true;
}

bool RendererLegacy::DrawSurface2D(const Surface *s)
{
	if (!s || !s->vertices || s->indices.size() < 3) return false;

	Material *m = s->mat;
	VertexArray *v = s->vertices;
	const bool diffuse = !v->diffuse.empty();
	const bool textured = (m && m->texture0 && v->uv0.size() == v->position.size());
	// no need for normals

	glPushAttrib(GL_LIGHTING_BIT);

	if (!m || m->unlit) glDisable(GL_LIGHTING);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->position[0]));
	if (diffuse) {
		assert(v->diffuse.size() == v->position.size());
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->diffuse[0]));
	}
	if (textured) {
		assert(v->uv0.size() == v->position.size());
		glEnable(GL_TEXTURE_2D);
		m->texture0->Bind();
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid *)&v->uv0[0]);
	}

	glDrawElements(GL_TRIANGLES, s->indices.size(), GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(&s->indices[0]));

	glDisableClientState(GL_VERTEX_ARRAY);
	if (diffuse)
		glDisableClientState(GL_COLOR_ARRAY);
	if (textured) {
		m->texture0->Unbind();
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
	}

	glPopAttrib();

	return true;
}

//position, color.
struct UnlitVertex {
	vector3f position;
	Color color;
};

bool RendererLegacy::DrawStaticMesh(StaticMesh *t)
{
	if (!t) return false;

	// prepare it
	if (!t->cached) {
		glGenBuffers(1, &t->buffy);

		const int numvertices = t->GetNumVerts();
		assert(numvertices > 0);

		UnlitVertex *vts = new UnlitVertex[numvertices];
		int next = 0;
		for (int i=0; i < t->numSurfaces; i++) {
			for(int j=0; j<t->surfaces[i].GetNumVerts(); j++) {
				vts[next].position = t->surfaces[i].vertices->position[j];
				vts[next].color = t->surfaces[i].vertices->diffuse[j];
				next++;
			}
		}

		//buffer
		glBindBuffer(GL_ARRAY_BUFFER, t->buffy);
		glBufferData(GL_ARRAY_BUFFER, sizeof(UnlitVertex)*numvertices, vts, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		t->cached = true;
		delete[] vts;
	}
	assert(t->cached == true);

	//draw it
	glBindBuffer(GL_ARRAY_BUFFER, t->buffy);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(UnlitVertex), reinterpret_cast<const GLvoid *>(offsetof(UnlitVertex, position)));
	glColorPointer(4, GL_FLOAT, sizeof(UnlitVertex), reinterpret_cast<const GLvoid *>(offsetof(UnlitVertex, color)));
	int start = 0;
	// XXX save start & numverts somewhere
	// XXX this is not indexed
	for (int i=0; i < t->numSurfaces; i++) {
		glDrawArrays(t->primitiveType, start, t->surfaces[i].GetNumVerts());
		start += t->surfaces[i].GetNumVerts();
	}
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}
