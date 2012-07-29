#include "RendererLegacy.h"
#include "Light.h"
#include "Material.h"
#include "Graphics.h"
#include "RendererGLBuffers.h"
#include "StaticMesh.h"
#include "Surface.h"
#include "Texture.h"
#include "VertexArray.h"
#include "TextureGL.h"
#include <stddef.h> //for offsetof
#include <ostream>
#include <sstream>
#include <iterator>

namespace Graphics {

struct MeshRenderInfo : public RenderInfo {
	MeshRenderInfo() :
		numIndices(0),
		vbuf(0),
		ibuf(0)
	{
	}
	virtual ~MeshRenderInfo() {
		//don't delete, if these come from a pool!
		delete vbuf;
		delete ibuf;
	}
	int numIndices;
	VertexBuffer *vbuf;
	IndexBuffer *ibuf;
};

// multiple surfaces can be buffered in one vbo so need to
// save starting offset + amount to draw
struct SurfaceRenderInfo : public RenderInfo {
	SurfaceRenderInfo() : glOffset(0), glAmount(0) {}
	int glOffset; //index start OR vertex start
	int glAmount; //index count OR vertex amount
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
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	SetClearColor(Color(0.f));
	SetViewport(0, 0, m_width, m_height);
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
	ClearScreen();
	return true;
}

bool RendererLegacy::EndFrame()
{
	return true;
}

bool RendererLegacy::SwapBuffers()
{
#ifndef NDEBUG
	GLenum err;
	err = glGetError();
	while (err != GL_NO_ERROR) {
		switch (err) {
			case GL_INVALID_ENUM:
				fprintf(stderr, "GL_INVALID_ENUM\n");
				break;
			case GL_INVALID_VALUE:
				fprintf(stderr, "GL_INVALID_VALUE\n");
				break;
			case GL_INVALID_OPERATION:
				fprintf(stderr, "GL_INVALID_OPERATION\n");
				break;
			case GL_OUT_OF_MEMORY:
				fprintf(stderr, "GL_OUT_OF_MEMORY\n");
				break;
			case GL_STACK_OVERFLOW: //deprecated in GL3
				fprintf(stderr, "GL_STACK_OVERFLOW\n");
				break;
			case GL_STACK_UNDERFLOW: //deprecated in GL3
				fprintf(stderr, "GL_STACK_UNDERFLOW\n");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION_EXT:
				fprintf(stderr, "GL_INVALID_FRAMEBUFFER_OPERATION\n");
				break;
		}
		err = glGetError();
	}
#endif

	Graphics::SwapBuffers();
	return true;
}

bool RendererLegacy::ClearScreen()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return true;
}

bool RendererLegacy::ClearDepthBuffer()
{
	glClear(GL_DEPTH_BUFFER_BIT);

	return true;
}

bool RendererLegacy::SetClearColor(const Color &c)
{
	glClearColor(c.r, c.g, c.b, c.a);
	return true;
}

bool RendererLegacy::SetViewport(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
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

bool RendererLegacy::SetDepthTest(bool enabled)
{
	if (enabled)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	return true;
}

bool RendererLegacy::SetDepthWrite(bool enabled)
{
	if (enabled)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);
	return true;
}

bool RendererLegacy::SetWireFrameMode(bool enabled)
{
	glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
	return true;
}

bool RendererLegacy::SetLights(int numlights, const Light *lights)
{
	if (numlights < 1) return false;

	m_numLights = numlights;

	for (int i=0; i < numlights; i++) {
		const Light &l = lights[i];
		// directional lights have w of 0
		const float pos[] = {
			l.GetPosition().x,
			l.GetPosition().y,
			l.GetPosition().z,
			l.GetType() == Light::LIGHT_DIRECTIONAL ? 0.f : 1.f
		};
		glLightfv(GL_LIGHT0+i, GL_POSITION, pos);
		glLightfv(GL_LIGHT0+i, GL_DIFFUSE, l.GetDiffuse());
		glLightfv(GL_LIGHT0+i, GL_AMBIENT, l.GetAmbient());
		glLightfv(GL_LIGHT0+i, GL_SPECULAR, l.GetSpecular());
		glEnable(GL_LIGHT0+i);
	}
	//XXX should probably disable unused lights (for legacy renderer only)

	Graphics::State::SetLights(numlights, lights);
	
	return true;
}

bool RendererLegacy::SetAmbientColor(const Color &c)
{
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, c);
	Graphics::State::SetGlobalSceneAmbientColor(c);
	return true;
}

bool RendererLegacy::SetScissor(bool enabled, const vector2f &pos, const vector2f &size)
{
	if (enabled) {
		glScissor(pos.x,pos.y,size.x,size.y);
		glEnable(GL_SCISSOR_TEST);
	}
	else
		glDisable(GL_SCISSOR_TEST);
	return true;
}

bool RendererLegacy::DrawLines(int count, const vector3f *v, const Color *c, LineType t)
{
	if (count < 2) return false;

	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), v);
	glColorPointer(4, GL_FLOAT, sizeof(Color), c);
	glDrawArrays(t, 0, count);
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

	//XXX enable when multisampling is not available
	//glEnable(GL_LINE_SMOOTH);

	glColor4f(c.r, c.g, c.b, c.a);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), v);
	glDrawArrays(t, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.f, 1.f, 1.f, 1.f);

	//glDisable(GL_LINE_SMOOTH);

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
	if (!s || !s->GetVertices() || s->GetNumIndices() < 3) return false;

	const Material *m = s->GetMaterial().Get();
	const VertexArray *v = s->GetVertices();

	ApplyMaterial(m);
	EnableClientStates(v);

	glDrawElements(s->GetPrimtiveType(), s->GetNumIndices(), GL_UNSIGNED_SHORT, s->GetIndexPointer());

	UnApplyMaterial(m);
	DisableClientStates();

	return true;
}

bool RendererLegacy::DrawPointSprites(int count, const vector3f *positions, const Material *material, float size)
{
	if (count < 1 || !material || !material->texture0) return false;

	SetBlendMode(BLEND_ALPHA_ONE);
	SetDepthWrite(false);
	VertexArray va(ATTRIB_POSITION | ATTRIB_UV0, count * 6);

	matrix4x4f rot;
	glGetFloatv(GL_MODELVIEW_MATRIX, &rot[0]);
	rot.ClearToRotOnly();
	rot = rot.InverseOf();

	const float sz = 0.5f*size;
	const vector3f rotv1 = rot * vector3f(sz, sz, 0.0f);
	const vector3f rotv2 = rot * vector3f(sz, -sz, 0.0f);
	const vector3f rotv3 = rot * vector3f(-sz, -sz, 0.0f);
	const vector3f rotv4 = rot * vector3f(-sz, sz, 0.0f);

	//do two-triangle quads. Could also do indexed surfaces.
	//GL2 renderer should use actual point sprites
	//(see history of Render.cpp for point code remnants)
	for (int i=0; i<count; i++) {
		const vector3f &pos = positions[i];

		va.Add(pos+rotv4, vector2f(0.f, 0.f)); //top left
		va.Add(pos+rotv3, vector2f(0.f, 1.f)); //bottom left
		va.Add(pos+rotv1, vector2f(1.f, 0.f)); //top right

		va.Add(pos+rotv1, vector2f(1.f, 0.f)); //top right
		va.Add(pos+rotv3, vector2f(0.f, 1.f)); //bottom left
		va.Add(pos+rotv2, vector2f(1.f, 1.f)); //bottom right
	}
	DrawTriangles(&va, material);
	SetBlendMode(BLEND_SOLID);
	SetDepthWrite(true);

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
	MeshRenderInfo *meshInfo = static_cast<MeshRenderInfo*>(t->GetRenderInfo());

	//draw each surface
	meshInfo->vbuf->Bind();
	if (meshInfo->ibuf) {
		meshInfo->ibuf->Bind();
	}

	for (StaticMesh::SurfaceIterator surface = t->SurfacesBegin(); surface != t->SurfacesEnd(); ++surface) {
		SurfaceRenderInfo *surfaceInfo = static_cast<SurfaceRenderInfo*>((*surface)->GetRenderInfo());

		ApplyMaterial((*surface)->GetMaterial().Get());
		if (meshInfo->ibuf) {
			meshInfo->vbuf->DrawIndexed(t->GetPrimtiveType(), surfaceInfo->glOffset, surfaceInfo->glAmount);
		} else {
			//draw unindexed per surface
			meshInfo->vbuf->Draw(t->GetPrimtiveType(), surfaceInfo->glOffset, surfaceInfo->glAmount);
		}
		UnApplyMaterial((*surface)->GetMaterial().Get());
	}
	if (meshInfo->ibuf)
		meshInfo->ibuf->Unbind();
	meshInfo->vbuf->Unbind();

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
	if (mat->texture0)
		static_cast<TextureGL*>(mat->texture0)->Bind();
}

void RendererLegacy::UnApplyMaterial(const Material *mat)
{
	glPopAttrib();
	if (!mat) return;
	if (mat->texture0)
		static_cast<TextureGL*>(mat->texture0)->Unbind();
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
	for (std::vector<GLenum>::const_iterator i = m_clientStates.begin(); i != m_clientStates.end(); ++i)
		glDisableClientState(*i);
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

	MeshRenderInfo *meshInfo = new MeshRenderInfo();
	mesh->SetRenderInfo(meshInfo);

	const int totalVertices = mesh->GetNumVerts();

	//surfaces should have a matching vertex specification!!
	//XXX just take vertices from the first surface as a LMR hack
	bool lmrHack = false;

	VertexBuffer *buf = 0;
	for (StaticMesh::SurfaceIterator surface = mesh->SurfacesBegin(); surface != mesh->SurfacesEnd(); ++surface) {
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

		SurfaceRenderInfo *surfaceInfo = new SurfaceRenderInfo();
		surfaceInfo->glOffset = offset;
		surfaceInfo->glAmount = numsverts;
		(*surface)->SetRenderInfo(surfaceInfo);

		//buffer indices from each surface, if in use
		if ((*surface)->IsIndexed()) {
			assert(background == false);
			if (!meshInfo->ibuf)
				meshInfo->ibuf = new IndexBuffer(mesh->GetNumIndices());
			meshInfo->ibuf->Bind();
			const int ioffset = meshInfo->ibuf->BufferIndexData((*surface)->GetNumIndices(), (*surface)->GetIndexPointer());
			surfaceInfo->glOffset = ioffset;
			surfaceInfo->glAmount = (*surface)->GetNumIndices();
		}
	}
	assert(buf);
	meshInfo->vbuf = buf;
	mesh->cached = true;

	return true;
}

Texture *RendererLegacy::CreateTexture(const TextureDescriptor &descriptor)
{
	return new TextureGL(descriptor);
}


// XXX very heavy. in the future when all GL calls are made through the
// renderer, we can probably do better by trackingn current state and
// only restoring the things that have changed
void RendererLegacy::PushState()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
}

void RendererLegacy::PopState()
{
	glPopAttrib();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

static void dump_opengl_value(std::ostream &out, const char *name, GLenum id, int num_elems)
{
	assert(num_elems > 0 && num_elems <= 4);
	assert(name);

	GLdouble e[4];
	glGetDoublev(id, e);

	GLenum err = glGetError();
	if (err == GL_NO_ERROR) {
		out << name << " = " << e[0];
		for (int i = 1; i < num_elems; ++i)
			out << ", " << e[i];
		out << "\n";
	} else {
		while (err != GL_NO_ERROR) {
			if (err == GL_INVALID_ENUM) { out << name << " -- not supported\n"; }
			else { out << name << " -- unexpected error (" << err << ") retrieving value\n"; }
			err = glGetError();
		}
	}
}

bool RendererLegacy::PrintDebugInfo(std::ostream &out)
{
	out << "OpenGL version " << glGetString(GL_VERSION);
	out << ", running on " << glGetString(GL_VENDOR);
	out << " " << glGetString(GL_RENDERER) << "\n";

	if (glewIsSupported("GL_VERSION_2_0"))
		out << "Shading language version: " <<  glGetString(GL_SHADING_LANGUAGE_VERSION_ARB) << "\n";

	out << "Available extensions:" << "\n";
	GLint numext = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numext);
	if (glewIsSupported("GL_VERSION_3_0")) {
		for (int i = 0; i < numext; ++i) {
			out << "  " << glGetStringi(GL_EXTENSIONS, i) << "\n";
		}
	}
	else {
		out << "  ";
		std::istringstream ext(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
		std::copy(
			std::istream_iterator<std::string>(ext),
			std::istream_iterator<std::string>(),
			std::ostream_iterator<std::string>(out, "\n  "));
	}

	out << "\nImplementation Limits:\n";

	// first, clear all OpenGL error flags
	while (glGetError() != GL_NO_ERROR) {}

#define DUMP_GL_VALUE(name) dump_opengl_value(out, #name, name, 1)
#define DUMP_GL_VALUE2(name) dump_opengl_value(out, #name, name, 2)

	DUMP_GL_VALUE(GL_MAX_3D_TEXTURE_SIZE);
	DUMP_GL_VALUE(GL_MAX_ATTRIB_STACK_DEPTH);
	DUMP_GL_VALUE(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH);
	DUMP_GL_VALUE(GL_MAX_CLIP_PLANES);
	DUMP_GL_VALUE(GL_MAX_COLOR_ATTACHMENTS_EXT);
	DUMP_GL_VALUE(GL_MAX_COLOR_MATRIX_STACK_DEPTH);
	DUMP_GL_VALUE(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
	DUMP_GL_VALUE(GL_MAX_CUBE_MAP_TEXTURE_SIZE);
	DUMP_GL_VALUE(GL_MAX_DRAW_BUFFERS);
	DUMP_GL_VALUE(GL_MAX_ELEMENTS_INDICES);
	DUMP_GL_VALUE(GL_MAX_ELEMENTS_VERTICES);
	DUMP_GL_VALUE(GL_MAX_EVAL_ORDER);
	DUMP_GL_VALUE(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS);
	DUMP_GL_VALUE(GL_MAX_LIGHTS);
	DUMP_GL_VALUE(GL_MAX_LIST_NESTING);
	DUMP_GL_VALUE(GL_MAX_MODELVIEW_STACK_DEPTH);
	DUMP_GL_VALUE(GL_MAX_NAME_STACK_DEPTH);
	DUMP_GL_VALUE(GL_MAX_PIXEL_MAP_TABLE);
	DUMP_GL_VALUE(GL_MAX_PROJECTION_STACK_DEPTH);
	DUMP_GL_VALUE(GL_MAX_RENDERBUFFER_SIZE_EXT);
	DUMP_GL_VALUE(GL_MAX_SAMPLES_EXT);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_COORDS);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_IMAGE_UNITS);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_LOD_BIAS);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_SIZE);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_STACK_DEPTH);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_UNITS);
	DUMP_GL_VALUE(GL_MAX_VARYING_FLOATS);
	DUMP_GL_VALUE(GL_MAX_VERTEX_ATTRIBS);
	DUMP_GL_VALUE(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
	DUMP_GL_VALUE(GL_MAX_VERTEX_UNIFORM_COMPONENTS);
	DUMP_GL_VALUE(GL_NUM_COMPRESSED_TEXTURE_FORMATS);
	DUMP_GL_VALUE(GL_SAMPLE_BUFFERS);
	DUMP_GL_VALUE(GL_SAMPLES);
	DUMP_GL_VALUE2(GL_ALIASED_LINE_WIDTH_RANGE);
	DUMP_GL_VALUE2(GL_ALIASED_POINT_SIZE_RANGE);
	DUMP_GL_VALUE2(GL_MAX_VIEWPORT_DIMS);
	DUMP_GL_VALUE2(GL_SMOOTH_LINE_WIDTH_RANGE);
	DUMP_GL_VALUE2(GL_SMOOTH_POINT_SIZE_RANGE);

#undef DUMP_GL_VALUE
#undef DUMP_GL_VALUE2

	return true;
}

}
