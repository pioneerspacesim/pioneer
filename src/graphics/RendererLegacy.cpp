// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "RendererLegacy.h"
#include "Graphics.h"
#include "Light.h"
#include "Material.h"
#include "MaterialLegacy.h"
#include "OS.h"
#include "RendererGLBuffers.h"
#include "StaticMesh.h"
#include "StringF.h"
#include "Surface.h"
#include "Texture.h"
#include "TextureGL.h"
#include "VertexArray.h"
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

RendererLegacy::RendererLegacy(const Graphics::Settings &vs)
: Renderer(vs.width, vs.height)
, m_numDirLights(0)
, m_minZNear(10.f)
, m_maxZFar(1000000.0f)
, m_useCompressedTextures(false)
{
	const bool useDXTnTextures = vs.useTextureCompression && glewIsSupported("GL_EXT_texture_compression_s3tc");
	m_useCompressedTextures = useDXTnTextures;

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
	glAlphaFunc(GL_GREATER, 0.5f);

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

static std::string glerr_to_string(GLenum err)
{
	switch (err)
	{
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	case GL_STACK_OVERFLOW: //deprecated in GL3
		return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW: //deprecated in GL3
		return "GL_STACK_UNDERFLOW";
	case GL_INVALID_FRAMEBUFFER_OPERATION_EXT:
		return "GL_INVALID_FRAMEBUFFER_OPERATION";
	default:
		return stringf("Unknown error 0x0%0{x}", err);
	}
}

bool RendererLegacy::SwapBuffers()
{
#ifndef NDEBUG
	// Check if an error occurred during the frame. This is not very useful for
	// determining *where* the error happened. For that purpose, try GDebugger or
	// the GL_KHR_DEBUG extension
	GLenum err;
	err = glGetError();
	if (err != GL_NO_ERROR) {
		std::stringstream ss;
		ss << "OpenGL error(s) during frame:\n";
		while (err != GL_NO_ERROR) {
			ss << glerr_to_string(err) << '\n';
			err = glGetError();
		}
		OS::Error("%s", ss.str().c_str());
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
	//XXX this is not pretty but there's no standard way of converting between them.
	for (int i=0; i<16; ++i) {
		m_currentTransform[i] = m[i];
	}
	//XXX you might still need the occasional push/pop
	//GL2+ or ES2 renderers can forego the classic matrix stuff entirely and use uniforms
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(&m[0]);
	return true;
}

bool RendererLegacy::SetTransform(const matrix4x4f &m)
{
	//same as above
	m_currentTransform = m;
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
	m_numDirLights = 0;

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
		glLightfv(GL_LIGHT0+i, GL_SPECULAR, l.GetSpecular());
		glEnable(GL_LIGHT0+i);

		if (l.GetType() == Light::LIGHT_DIRECTIONAL)
			m_numDirLights++;

		assert(m_numDirLights < 5);
	}
	//XXX should probably disable unused lights (for legacy renderer only)

	Graphics::State::SetLights(numlights, lights);

	return true;
}

bool RendererLegacy::SetAmbientColor(const Color &c)
{
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, c);
	m_ambient = c;
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

bool RendererLegacy::DrawTriangles(const VertexArray *v, Material *m, PrimitiveType t)
{
	if (!v || v->position.size() < 3) return false;

	m->Apply();
	EnableClientStates(v);

	glDrawArrays(t, 0, v->GetNumVerts());

	m->Unapply();
	DisableClientStates();

	return true;
}

bool RendererLegacy::DrawSurface(const Surface *s)
{
	if (!s || !s->GetVertices() || s->GetNumIndices() < 3) return false;

	const Material *m = s->GetMaterial().Get();
	const VertexArray *v = s->GetVertices();

	const_cast<Material*>(m)->Apply();
	EnableClientStates(v);

	glDrawElements(s->GetPrimtiveType(), s->GetNumIndices(), GL_UNSIGNED_SHORT, s->GetIndexPointer());

	const_cast<Material*>(m)->Unapply();
	DisableClientStates();

	return true;
}

bool RendererLegacy::DrawPointSprites(int count, const vector3f *positions, Material *material, float size)
{
	if (count < 1 || !material || !material->texture0) return false;

	SetDepthWrite(false);
	VertexArray va(ATTRIB_POSITION | ATTRIB_UV0, count * 6);

	matrix4x4f rot(GetCurrentTransform());
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

		const_cast<Material*>((*surface)->GetMaterial().Get())->Apply();
		if (meshInfo->ibuf) {
			meshInfo->vbuf->DrawIndexed(t->GetPrimtiveType(), surfaceInfo->glOffset, surfaceInfo->glAmount);
		} else {
			//draw unindexed per surface
			meshInfo->vbuf->Draw(t->GetPrimtiveType(), surfaceInfo->glOffset, surfaceInfo->glAmount);
		}
		const_cast<Material*>((*surface)->GetMaterial().Get())->Unapply();
	}
	if (meshInfo->ibuf)
		meshInfo->ibuf->Unbind();
	meshInfo->vbuf->Unbind();

	return true;
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

	int indexAdjustment = 0;

	VertexBuffer *buf = 0;
	for (StaticMesh::SurfaceIterator surface = mesh->SurfacesBegin(); surface != mesh->SurfacesEnd(); ++surface) {
		const int numsverts = (*surface)->GetNumVerts();
		const VertexArray *va = (*surface)->GetVertices();

		int offset = 0;
		if (lmr) {
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

			//XXX should do this adjustment in RendererGL2Buffers
			const unsigned short *originalIndices = (*surface)->GetIndexPointer();
			std::vector<unsigned short> adjustedIndices((*surface)->GetNumIndices());
			for (int i = 0; i < (*surface)->GetNumIndices(); ++i)
				adjustedIndices[i] = originalIndices[i] + indexAdjustment;

			if (!meshInfo->ibuf)
				meshInfo->ibuf = new IndexBuffer(mesh->GetNumIndices());
			meshInfo->ibuf->Bind();
			const int ioffset = meshInfo->ibuf->BufferIndexData((*surface)->GetNumIndices(), &adjustedIndices[0]);
			surfaceInfo->glOffset = ioffset;
			surfaceInfo->glAmount = (*surface)->GetNumIndices();

			indexAdjustment += (*surface)->GetNumVerts();
		}
	}
	assert(buf);
	meshInfo->vbuf = buf;
	mesh->cached = true;

	return true;
}

Material *RendererLegacy::CreateMaterial(const MaterialDescriptor &desc)
{
	MaterialLegacy *m;
	switch (desc.effect) {
	case EFFECT_STARFIELD:
		m = new StarfieldMaterialLegacy();
		break;
	case EFFECT_GEOSPHERE_TERRAIN:
	case EFFECT_GEOSPHERE_TERRAIN_WITH_LAVA:
	case EFFECT_GEOSPHERE_TERRAIN_WITH_WATER:
		m = new GeoSphereSurfaceMaterialLegacy();
		break;
	default:
		m = new MaterialLegacy();
	}

	m->vertexColors = desc.vertexColors;
	m->unlit = !desc.lighting;
	m->twoSided = desc.twoSided;
	m->m_descriptor = desc;
	return m;
}

Texture *RendererLegacy::CreateTexture(const TextureDescriptor &descriptor)
{
	return new TextureGL(descriptor, m_useCompressedTextures);
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
	glPushAttrib(GL_ALL_ATTRIB_BITS & (~GL_POINT_BIT));
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
