#include "RendererLegacy.h"
#include "Light.h"
#include "Material.h"
#include "Render.h"
#include "RendererGLBuffers.h"
#include "StaticMesh.h"
#include "Surface.h"
#include "Texture.h"
#include "VertexArray.h"
#include <stddef.h> //for offsetof

struct GLRenderInfo : public RenderInfo {
	GLRenderInfo() :
		numIndices(0),
		vbuf(0),
		ibuf(0)
	{
	}
	virtual ~GLRenderInfo() {
		//don't delete, if these come from a pool!
		delete vbuf;
		delete ibuf;
	}
	int numIndices;
	VertexBuffer *vbuf;
	IndexBuffer *ibuf;
};

RendererLegacy::RendererLegacy(int w, int h) :
	Renderer(w, h),
	m_minZNear(10.f),
	m_maxZFar(1000000.0f)
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

	SetClearColor(Color(0.f));

	glViewport(0, 0, m_width, m_height);
}

RendererLegacy::~RendererLegacy()
{

}

bool RendererLegacy::GetNearFarRange(float &near, float &far) const
{
	near = m_minZNear;
	far = m_maxZFar;
	return true;
}

bool RendererLegacy::BeginFrame()
{
	ClearScreen(true, true);
	return true;
}

bool RendererLegacy::EndFrame()
{
	return true;
}

bool RendererLegacy::SwapBuffers()
{
	glError();
	Render::SwapBuffers();
	return true;
}

bool RendererLegacy::ClearScreen(bool color, bool depth)
{
	if (!color && !depth) return false;

	GLbitfield mask = (
		(color ? GL_COLOR_BUFFER_BIT : 0) |
		(depth ? GL_DEPTH_BUFFER_BIT : 0)
	);
	glClear(mask);

	return true;
}

bool RendererLegacy::SetClearColor(const Color &c)
{
	glClearColor(c.r, c.g, c.b, c.a);
	return true;
}

bool RendererLegacy::SetTransform(const matrix4x4d &m)
{
	//XXX you might still need the occasional push/pop
	//GL2+ or ES2 renderers can forego the classic matrix stuff entirely and use uniforms
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(&m[0]);
	return true;
}

bool RendererLegacy::SetTransform(const matrix4x4f &m)
{
	//same as above
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&m[0]);
	return true;
}

bool RendererLegacy::SetPerspectiveProjection(float fov, float aspect, float near, float far)
{
	double ymax = near * tan(fov * M_PI / 360.0);
	double ymin = -ymax;
	double xmin = ymin * aspect;
	double xmax = ymax * aspect;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(xmin, xmax, ymin, ymax, near, far);
	return true;
}

bool RendererLegacy::SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(xmin, xmax, ymin, ymax, zmin, zmax);
	return true;
}

bool RendererLegacy::SetBlendMode(BlendMode m)
{
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

bool RendererLegacy::DrawLines(int count, const vector3f *v, const Color &c, LineType t)
{
	if (count < 2 || !v) return false;

	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	glColor4f(c.r, c.g, c.b, c.a);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), v);
	glDrawArrays(t, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.f, 1.f, 1.f, 1.f);

	glPopAttrib();

	return true;
}

bool RendererLegacy::DrawLines2D(int count, const vector2f *v, const Color &c, LineType t)
{
	if (count < 2 || !v) return false;

	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	glColor4f(c.r, c.g, c.b, c.a);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof(vector2f), v);
	glDrawArrays(t, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.f, 1.f, 1.f, 1.f);

	glPopAttrib();

	return true;
}

bool RendererLegacy::DrawPoints(int count, const vector3f *points, const Color *colors, float size)
{
	if (count < 1 || !points || !colors) return false;

	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	glPointSize(size);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, points);
	glColorPointer(4, GL_FLOAT, 0, colors);
	glDrawArrays(GL_POINTS, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glPointSize(1.f); // XXX wont't be necessary

	glPopAttrib();

	return true;
}

bool RendererLegacy::DrawPoints2D(int count, const vector2f *points, const Color *colors, float size)
{
	if (count < 1 || !points || !colors) return false;

	glDisable(GL_LIGHTING);

	glPointSize(size);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, points);
	glColorPointer(4, GL_FLOAT, 0, colors);
	glDrawArrays(GL_POINTS, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glPointSize(1.f); // XXX wont't be necessary

	return true;
}

bool RendererLegacy::DrawTriangles(const VertexArray *v, const Material *m, PrimitiveType t)
{
	if (!v || v->position.size() < 3) return false;

	ApplyMaterial(m);
	EnableClientStates(v);

	glDrawArrays(t, 0, v->GetNumVerts());

	UnApplyMaterial(m);
	DisableClientStates();

	return true;
}

bool RendererLegacy::DrawSurface(const Surface *s)
{
	if (!s || !s->GetVertices() || s->indices.size() < 3) return false;

	const Material *m = s->GetMaterial().Get();
	const VertexArray *v = s->GetVertices();

	ApplyMaterial(m);
	EnableClientStates(v);

	glDrawElements(s->m_primitiveType, s->indices.size(), GL_UNSIGNED_SHORT, &s->indices[0]);

	UnApplyMaterial(m);
	DisableClientStates();

	return true;
}

bool RendererLegacy::DrawPointSprites(int count, const vector3f *positions, const Material *material, float size)
{
	if (count < 1 || !material || !material->texture0) return false;

	material->texture0->Bind();
	Render::PutPointSprites(count, const_cast<vector3f*>(positions), size, material->diffuse);
	material->texture0->Unbind();

	return true;
}

bool RendererLegacy::DrawStaticMesh(StaticMesh *t)
{
	if (!t) return false;

	//Approach:
	//on first render, buffer vertices from all surfaces to a vbo
	//since surfaces can have different materials (but they should have the same vertex format?)
	//bind buffer, set pointers and then draw each surface
	//(save buffer offsets in surfaces' render info)

	// prepare the buffer on first run
	if (!t->cached) {
		if (!BufferStaticMesh(t))
			return false;
	}
	GLRenderInfo *info = static_cast<GLRenderInfo*>(t->m_renderInfo);

	//draw each surface
	info->vbuf->Bind();
	if (info->ibuf) {
		info->ibuf->Bind();
	}

	SurfaceList &surfaces = t->m_surfaces;
	SurfaceList::iterator surface;
	for (surface = surfaces.begin(); surface != surfaces.end(); ++surface) {
		ApplyMaterial((*surface)->GetMaterial().Get());
		if (info->ibuf) {
			info->vbuf->DrawIndexed(t->m_primitiveType, (*surface)->glOffset, (*surface)->glAmount);
		} else {
			//draw unindexed per surface
			info->vbuf->Draw(t->m_primitiveType, (*surface)->glOffset, (*surface)->glAmount);
		}
		UnApplyMaterial((*surface)->GetMaterial().Get());
	}
	if (info->ibuf)
		info->ibuf->Unbind();
	info->vbuf->Unbind();

	return true;
}

void RendererLegacy::ApplyMaterial(const Material *mat)
{
	glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT);
	if (!mat) {
		glDisable(GL_LIGHTING);
		return;
	}

	if (!mat->vertexColors)
		glColor4f(mat->diffuse.r, mat->diffuse.g, mat->diffuse.b, mat->diffuse.a);
	
	if (mat->unlit) {
		glDisable(GL_LIGHTING);
	} else {
		glEnable(GL_LIGHTING);
		glMaterialfv (GL_FRONT, GL_DIFFUSE, &mat->diffuse[0]);
		//todo: the rest
	}
	if (mat->twoSided) {
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
		glDisable(GL_CULL_FACE);
	}
	if (mat->texture0) {
		glEnable(GL_TEXTURE_2D);
		mat->texture0->Bind();
	}
}

void RendererLegacy::UnApplyMaterial(const Material *mat)
{
	glPopAttrib();
	if (!mat) return;
	if (mat->texture0) {
		mat->texture0->Unbind();
	}
}

void RendererLegacy::EnableClientStates(const VertexArray *v)
{
	if (!v) return;
	assert(v->position.size() > 0); //would be strange
	// XXX could be 3D or 2D
	m_clientStates.push_back(GL_VERTEX_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->position[0]));

	if (v->HasAttrib(ATTRIB_DIFFUSE)) {
		m_clientStates.push_back(GL_COLOR_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->diffuse[0]));
	}
	if (v->HasAttrib(ATTRIB_NORMAL)) {
		m_clientStates.push_back(GL_NORMAL_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->normal[0]));
	}
	if (v->HasAttrib(ATTRIB_UV0)) {
		m_clientStates.push_back(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, reinterpret_cast<const GLvoid *>(&v->uv0[0]));
	}
}

void RendererLegacy::DisableClientStates()
{
	for(int i=0; i!=m_clientStates.size(); i++)
		glDisableClientState(m_clientStates[i]);
	m_clientStates.clear();
}

bool RendererLegacy::BufferStaticMesh(StaticMesh *mesh)
{
	const AttributeSet set = mesh->GetAttributeSet();
	bool background = false;
	bool lmr = false;
	//XXX does this really have to support every case. I don't know.
	if (set == (ATTRIB_POSITION | ATTRIB_NORMAL | ATTRIB_UV0))
		lmr = true;
	else if (set == (ATTRIB_POSITION | ATTRIB_DIFFUSE))
		background = true;
	else
		return false;

	if (mesh->m_renderInfo == 0)
		mesh->m_renderInfo = new GLRenderInfo();
	GLRenderInfo *info = static_cast<GLRenderInfo*>(mesh->m_renderInfo);

	const int totalVertices = mesh->GetNumVerts();

	//surfaces should have a matching vertex specification!!
	//XXX just take vertices from the first surface as a LMR hack
	bool lmrHack = false;

	VertexBuffer *buf = 0;
	SurfaceList &surfaces = mesh->m_surfaces;
	SurfaceList::iterator surface;
	for (surface = surfaces.begin(); surface != surfaces.end(); ++surface) {
		const int numsverts = (*surface)->GetNumVerts();
		const VertexArray *va = (*surface)->GetVertices();

		int offset = 0;
		if (lmr && !lmrHack) {
			ScopedArray<ModelVertex> vts(new ModelVertex[numsverts]);
			for(int j=0; j<numsverts; j++) {
				vts[j].position = va->position[j];
				vts[j].normal = va->normal[j];
				vts[j].uv = va->uv0[j];
			}

			if (!buf)
				buf = new VertexBuffer(totalVertices);
			buf->Bind();
			buf->BufferData<ModelVertex>(numsverts, vts.Get());
			lmrHack = true;
		} else if (background) {
			ScopedArray<UnlitVertex> vts(new UnlitVertex[numsverts]);
			for(int j=0; j<numsverts; j++) {
				vts[j].position = va->position[j];
				vts[j].color = va->diffuse[j];
			}

			if (!buf)
				buf= new UnlitVertexBuffer(totalVertices);
			buf->Bind();
			offset = buf->BufferData<UnlitVertex>(numsverts, vts.Get());
		}
		(*surface)->glAmount = numsverts;
		(*surface)->glOffset = offset;

		//buffer indices from each surface, if in use
		if ((*surface)->IsIndexed()) {
			assert(background == false);
			if (!info->ibuf)
				info->ibuf = new IndexBuffer(mesh->GetNumIndices());
			info->ibuf->Bind();
			const std::vector<unsigned short> &indices = (*surface)->indices;
			const int ioffset = info->ibuf->BufferIndexData(indices.size(), &indices[0]);
			(*surface)->glOffset = ioffset;
			(*surface)->glAmount = indices.size();
		}
	}
	assert(buf);
	info->vbuf = buf;
	mesh->cached = true;

	return true;
}