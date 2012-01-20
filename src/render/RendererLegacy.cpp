#include "RendererLegacy.h"

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

bool RendererLegacy::SetBlendMode(unsigned int m)
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

bool RendererLegacy::DrawLines(int count, const LineVertex *v, unsigned int type)
{
	if (count < 2) return false;

	//this is easy to upgrade to GL3/ES2 level
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(LineVertex), &v[0].position);
	glColorPointer(4, GL_FLOAT, sizeof(LineVertex), &v[0].color);
	glDrawArrays(type, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	return true;
}

bool RendererLegacy::DrawLines2D(int count, const LineVertex2D *v, unsigned int type)
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

bool RendererLegacy::DrawTriangles2D(const VertexArray *v, const Material *m, unsigned int t)
{
	if (!v || v->GetNumVerts() < 3) return false;

	// XXX uses standard 3D VertexArray
	return DrawTriangles(v, m, t);
}

bool RendererLegacy::DrawTriangles(const VertexArray *v, const Material *m, unsigned int t)
{
	if (!v || v->position.size() < 3) return false;

	const bool diffuse = !v->diffuse.empty();
	const bool textured = (m && m->texture0 && v->uv0.size() == v->position.size());
	const bool normals = !v->normal.empty();
	const unsigned int numverts = v->position.size();

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, (const GLvoid *)&v->position[0]);
	if (diffuse) {
		assert(v->diffuse.size() == v->position.size());
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, (const GLvoid *)&v->diffuse[0]);
	}
	if (normals) {
		assert(v->normal.size() == v->position.size());
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, (const GLvoid *)&v->normal[0]);
	}
	if (textured) {
		assert(v->uv0.size() == v->position.size());
		glEnable(GL_TEXTURE_2D);
		m->texture0->Bind();
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid *)&v->uv0[0]);
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

	return true;
}