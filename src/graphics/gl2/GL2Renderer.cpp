// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GL2Renderer.h"
#include "graphics/Graphics.h"
#include "graphics/Light.h"
#include "graphics/Material.h"
#include "OS.h"
#include "StringF.h"
#include "graphics/Texture.h"
#include "GL2Texture.h"
#include "graphics/VertexArray.h"
#include "GL2Debug.h"
#include "GL2GasGiantMaterial.h"
#include "GL2GeoSphereMaterial.h"
#include "GL2Material.h"
#include "GL2RenderState.h"
#include "GL2RenderTarget.h"
#include "GL2VertexBuffer.h"
#include "GL2MultiMaterial.h"
#include "GL2Program.h"
#include "GL2RingMaterial.h"
#include "GL2StarfieldMaterial.h"
#include "GL2FresnelColourMaterial.h"
#include "GL2ShieldMaterial.h"
#include "GL2SkyboxMaterial.h"
#include "GL2SphereImpostorMaterial.h"
#include "GL2UIMaterial.h"
#include "GL2VtxColorMaterial.h"

#include <stddef.h> //for offsetof
#include <ostream>
#include <sstream>
#include <iterator>

using namespace gl21;

namespace Graphics {

static Renderer *CreateRenderer(WindowSDL *win, const Settings &vs) {
	return new RendererGL2(win, vs);
}

void RendererGL2::RegisterRenderer() {
	Graphics::RegisterRenderer(Graphics::RENDERER_OPENGL, CreateRenderer);
}

typedef std::vector<std::pair<MaterialDescriptor, GL2::Program*> >::const_iterator ProgramIterator;

bool RendererGL2::initted = false;

static std::string glerr_to_string(GLenum err)
{
	switch (err)
	{
	case gl::INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case gl::INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case gl::INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case gl::OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	case gl::STACK_OVERFLOW: //deprecated in GL3
		return "GL_STACK_OVERFLOW";
	case gl::STACK_UNDERFLOW: //deprecated in GL3
		return "GL_STACK_UNDERFLOW";
	default:
		return stringf("Unknown error 0x0%0{x}", err);
	}
}

void RendererGL2::CheckErrors(const char *func /*= nullptr*/, const int line /*= nullptr*/)
{
	PROFILE_SCOPED()
#ifndef PIONEER_PROFILER
	GLenum err = gl::GetError();
	if( err ) {
		// static-cache current err that sparked this
		static GLenum s_prevErr = gl::NO_ERROR_;
		const bool showWarning = (s_prevErr != err);
		s_prevErr = err;
		// now build info string
		std::stringstream ss;
		if(func) {
			ss << "In function " << std::string(func) << "\n";
		}
		if(line>=0) {
			ss << "On line " << std::to_string(line) << "\n";
		}
		ss << "OpenGL error(s) during frame:\n";
		while (err != gl::NO_ERROR_) {
			ss << glerr_to_string(err) << '\n';
			if( err == gl::OUT_OF_MEMORY ) {
				ss << "Out-of-memory on graphics card." << std::endl
					<< "Recommend enabling \"Compress Textures\" in game options." << std::endl
					<< "Also try reducing City and Planet detail settings." << std::endl;
			}
#ifdef _WIN32
			else if (err == gl::INVALID_OPERATION) {
				ss << "Invalid operations can occur if you are using overlay software." << std::endl
					<< "Such as FRAPS, RivaTuner, MSI Afterburner etc." << std::endl
					<< "Please try disabling this kind of software and testing again, thankyou." << std::endl;
			}
#endif
			err = gl::GetError();
		}
		// show warning dialog or just log to output
		if(showWarning)
			Warning("%s", ss.str().c_str());
		else
			Output("%s", ss.str().c_str());
	}
#endif
}

RendererGL2::RendererGL2(WindowSDL *window, const Graphics::Settings &vs)
: Renderer(window, window->GetWidth(), window->GetHeight())
, m_numDirLights(0)
//the range is very large due to a "logarithmic z-buffer" trick used
//http://outerra.blogspot.com/2009/08/logarithmic-z-buffer.html
//http://www.gamedev.net/blog/73/entry-2006307-tip-of-the-day-logarithmic-zbuffer-artifacts-fix/
, m_minZNear(0.0001f)
, m_maxZFar(10000000.0f)
, m_useCompressedTextures(false)
, m_invLogZfarPlus1(0.f)
, m_activeRenderTarget(0)
, m_activeRenderState(nullptr)
, m_matrixMode(MatrixMode::MODELVIEW)
{
	if (!initted) {
		initted = true;

		gl::exts::LoadTest didLoad = gl::sys::LoadFunctions();
		if (!didLoad)
			Error(
				"Pioneer can not run on your graphics card as it does not appear to support OpenGL 2.1\n"
				"Please check to see if your GPU driver vendor has an updated driver - or that drivers are installed correctly."
				);
	}
	m_viewportStack.push(Viewport());

	const bool useDXTnTextures = vs.useTextureCompression;
	m_useCompressedTextures = useDXTnTextures;

	//XXX bunch of fixed function states here!
	gl::CullFace(gl::BACK);
	gl::FrontFace(gl::CCW);
	gl::Enable(gl::CULL_FACE);
	gl::Enable(gl::DEPTH_TEST);
	gl::Enable(gl::LIGHT0);
	gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
	gl::Hint(gl::POINT_SMOOTH_HINT, gl::NICEST);
	gl::Hint(gl::LINE_SMOOTH_HINT, gl::NICEST);

	gl::MatrixMode(gl::MODELVIEW);
	m_modelViewStack.push(matrix4x4f::Identity());
	m_projectionStack.push(matrix4x4f::Identity());

	SetClearColor(Color(0.f));
	SetViewport(0, 0, m_width, m_height);

	if (vs.enableDebugMessages)
		GLDebug::Enable();
}

RendererGL2::~RendererGL2()
{
	while (!m_programs.empty()) delete m_programs.back().second, m_programs.pop_back();
	for (auto state : m_renderStates)
		delete state.second;
}

static const char *gl_error_to_string(GLenum err)
{
	switch (err) {
	case gl::NO_ERROR_: return "(no error)";
	case gl::INVALID_ENUM: return "invalid enum";
	case gl::INVALID_VALUE: return "invalid value";
	case gl::INVALID_OPERATION: return "invalid operation";
	//case gl::INVALID_FRAMEBUFFER_OPERATION: return "invalid framebuffer operation";
	case gl::OUT_OF_MEMORY: return "out of memory";
	default: return "(unknown error)";
	}
}

static void dump_and_clear_opengl_errors(std::ostream &out, GLenum first_error = gl::NO_ERROR_)
{
	GLenum err = ((first_error == gl::NO_ERROR_) ? gl::GetError() : first_error);
	if (err != gl::NO_ERROR_) {
		out << "errors: ";
		do {
			out << gl_error_to_string(err) << " ";
			err = gl::GetError();
		} while (err != gl::NO_ERROR_);
		out << std::endl;
	}
}

static void dump_opengl_value(std::ostream &out, const char *name, GLenum id, int num_elems)
{
	assert(num_elems > 0 && num_elems <= 4);
	assert(name);

	GLdouble e[4];
	gl::GetDoublev(id, e);

	GLenum err = gl::GetError();
	if (err == gl::NO_ERROR_) {
		out << name << " = " << e[0];
		for (int i = 1; i < num_elems; ++i)
			out << ", " << e[i];
		out << "\n";
	}
	else {
		while (err != gl::NO_ERROR_) {
			if (err == gl::INVALID_ENUM) { out << name << " -- not supported\n"; }
			else { out << name << " -- unexpected error (" << err << ") retrieving value\n"; }
			err = gl::GetError();
		}
	}
}

void RendererGL2::WriteRendererInfo(std::ostream &out) const
{
	out << "OpenGL version " << gl::GetString(gl::VERSION);
	out << ", running on " << gl::GetString(gl::VENDOR);
	out << " " << gl::GetString(gl::RENDERER) << "\n";

	out << "Shading language version: " << gl::GetString(gl::SHADING_LANGUAGE_VERSION) << "\n";

	out << "\nImplementation Limits:\n";

	// first, clear all OpenGL error flags
	dump_and_clear_opengl_errors(out);

#define DUMP_GL_VALUE(name) dump_opengl_value(out, #name, name, 1)
#define DUMP_GL_VALUE2(name) dump_opengl_value(out, #name, name, 2)

	DUMP_GL_VALUE(gl::MAX_COMBINED_TEXTURE_IMAGE_UNITS);
	DUMP_GL_VALUE(gl::MAX_CUBE_MAP_TEXTURE_SIZE);
	DUMP_GL_VALUE(gl::MAX_DRAW_BUFFERS);
	DUMP_GL_VALUE(gl::MAX_ELEMENTS_INDICES);
	DUMP_GL_VALUE(gl::MAX_ELEMENTS_VERTICES);
	DUMP_GL_VALUE(gl::MAX_FRAGMENT_UNIFORM_COMPONENTS);
	DUMP_GL_VALUE(gl::MAX_TEXTURE_IMAGE_UNITS);
	DUMP_GL_VALUE(gl::MAX_TEXTURE_LOD_BIAS);
	DUMP_GL_VALUE(gl::MAX_TEXTURE_SIZE);
	DUMP_GL_VALUE(gl::MAX_VERTEX_ATTRIBS);
	DUMP_GL_VALUE(gl::MAX_VERTEX_TEXTURE_IMAGE_UNITS);
	DUMP_GL_VALUE(gl::MAX_VERTEX_UNIFORM_COMPONENTS);
	DUMP_GL_VALUE(gl::NUM_COMPRESSED_TEXTURE_FORMATS);
	DUMP_GL_VALUE(gl::SAMPLE_BUFFERS);
	DUMP_GL_VALUE(gl::SAMPLES);
	DUMP_GL_VALUE2(gl::ALIASED_LINE_WIDTH_RANGE);
	DUMP_GL_VALUE2(gl::MAX_VIEWPORT_DIMS);
	DUMP_GL_VALUE2(gl::SMOOTH_LINE_WIDTH_RANGE);
	DUMP_GL_VALUE2(gl::SMOOTH_POINT_SIZE_RANGE);

#undef DUMP_GL_VALUE
#undef DUMP_GL_VALUE2

	// enumerate compressed texture formats
	{
		dump_and_clear_opengl_errors(out);
		out << "\nCompressed texture formats:\n";

		GLint nformats;
		GLint formats[128]; // XXX 128 should be enough, right?

		gl::GetIntegerv(gl::NUM_COMPRESSED_TEXTURE_FORMATS, &nformats);
		GLenum err = gl::GetError();
		if (err != gl::NO_ERROR_) {
			out << "Get NUM_COMPRESSED_TEXTURE_FORMATS failed\n";
			dump_and_clear_opengl_errors(out, err);
		}
		else {
			assert(nformats >= 0 && nformats < int(COUNTOF(formats)));
			gl::GetIntegerv(gl::COMPRESSED_TEXTURE_FORMATS, formats);
			err = gl::GetError();
			if (err != gl::NO_ERROR_) {
				out << "Get COMPRESSED_TEXTURE_FORMATS failed\n";
				dump_and_clear_opengl_errors(out, err);
			}
			else {
				for (int i = 0; i < nformats; ++i) {
					out << stringf("  %0{x#}\n", unsigned(formats[i]));
				}
			}
		}
	}
	// one last time
	dump_and_clear_opengl_errors(out);
}

bool RendererGL2::GetNearFarRange(float &near_, float &far_) const
{
	near_ = m_minZNear;
	far_ = m_maxZFar;
	return true;
}

bool RendererGL2::BeginFrame()
{
	PROFILE_SCOPED()
	gl::ClearColor(0,0,0,0);
	gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
	return true;
}

bool RendererGL2::EndFrame()
{
	return true;
}

bool RendererGL2::SwapBuffers()
{
	PROFILE_SCOPED()
#ifndef NDEBUG
	// Check if an error occurred during the frame. This is not very useful for
	// determining *where* the error happened. For that purpose, try GDebugger or
	// the KHR_DEBUG extension
	GLenum err;
	err = gl::GetError();
	if (err != gl::NO_ERROR_) {
		std::stringstream ss;
		ss << "OpenGL error(s) during frame:\n";
		while (err != gl::NO_ERROR_) {
			ss << glerr_to_string(err) << std::endl;
			err = gl::GetError();
			if( err == gl::OUT_OF_MEMORY ) {
				ss << "Out-of-memory on graphics card." << std::endl
					<< "Recommend enabling \"Compress Textures\" in game options." << std::endl
					<< "Also try reducing City and Planet detail settings." << std::endl;
			}
		}
		Error("%s", ss.str().c_str());
	}
#endif

	GetWindow()->SwapBuffers();
	return true;
}

bool RendererGL2::SetRenderState(RenderState *rs)
{
	if (m_activeRenderState != rs) {
		static_cast<GL2::RenderState*>(rs)->Apply();
		m_activeRenderState = rs;
	}
	return true;
}

bool RendererGL2::SetRenderTarget(RenderTarget *rt)
{
	PROFILE_SCOPED()
	if (rt)
		static_cast<GL2::RenderTarget*>(rt)->Bind();
	else if (m_activeRenderTarget)
		m_activeRenderTarget->Unbind();

	m_activeRenderTarget = static_cast<GL2::RenderTarget*>(rt);

	return true;
}

bool RendererGL2::SetDepthRange(double near_, double far_)
{
	gl::DepthRange(near_, far_);
	return true;
}

bool RendererGL2::ClearScreen()
{
	m_activeRenderState = nullptr;
	gl::DepthMask(gl::TRUE_);
	gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

	return true;
}

bool RendererGL2::ClearDepthBuffer()
{
	m_activeRenderState = nullptr;
	gl::DepthMask(gl::TRUE_);
	gl::Clear(gl::DEPTH_BUFFER_BIT);

	return true;
}

bool RendererGL2::SetClearColor(const Color &c)
{
	gl::ClearColor(c.r, c.g, c.b, c.a);
	return true;
}

bool RendererGL2::SetViewport(int x, int y, int width, int height)
{
	assert(!m_viewportStack.empty());
	Viewport& currentViewport = m_viewportStack.top();
	currentViewport.x = x;
	currentViewport.y = y;
	currentViewport.w = width;
	currentViewport.h = height;
	gl::Viewport(x, y, width, height);
	return true;
}

bool RendererGL2::SetTransform(const matrix4x4d &m)
{
	PROFILE_SCOPED()
	matrix4x4f mf;
	matrix4x4dtof(m, mf);
	return SetTransform(mf);
}

bool RendererGL2::SetTransform(const matrix4x4f &m)
{
	PROFILE_SCOPED()
	//same as above
	m_modelViewStack.top() = m;
	SetMatrixMode(MatrixMode::MODELVIEW);
	LoadMatrix(&m[0]);
	return true;
}

bool RendererGL2::SetPerspectiveProjection(float fov, float aspect, float znear, float zfar)
{
	PROFILE_SCOPED()

	// update values for log-z hack
	m_invLogZfarPlus1 = 1.0f / (log(zfar+1.0f)/log(2.0f));

	Graphics::SetFov(fov);

	float ymax = znear * tan(fov * M_PI / 360.0);
	float ymin = -ymax;
	float xmin = ymin * aspect;
	float xmax = ymax * aspect;

	const matrix4x4f frustrumMat = matrix4x4f::FrustumMatrix(xmin, xmax, ymin, ymax, znear, zfar);
	SetProjection(frustrumMat);
	return true;
}

bool RendererGL2::SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax)
{
	PROFILE_SCOPED()
	const matrix4x4f orthoMat = matrix4x4f::OrthoFrustum(xmin, xmax, ymin, ymax, zmin, zmax);
	SetProjection(orthoMat);
	return true;
}

bool RendererGL2::SetProjection(const matrix4x4f &m)
{
	PROFILE_SCOPED()
	//same as above
	m_projectionStack.top() = m;
	SetMatrixMode(MatrixMode::PROJECTION);
	LoadMatrix(&m[0]);
	return true;
}

bool RendererGL2::SetWireFrameMode(bool enabled)
{
	gl::PolygonMode(gl::FRONT_AND_BACK, enabled ? gl::LINE : gl::FILL);
	return true;
}

bool RendererGL2::SetLights(Uint32 numlights, const Light *lights)
{
	numlights = std::min(numlights, TOTAL_NUM_LIGHTS);
	if (numlights < 1) {
		m_numLights = 0;
		m_numDirLights = 0;
		return false;
	}

	m_numLights = numlights;
	m_numDirLights = 0;

	for (Uint32 i = 0; i<numlights; i++) {
		const Light &l = lights[i];
		m_lights[i].SetPosition(l.GetPosition());
		m_lights[i].SetDiffuse(l.GetDiffuse());
		m_lights[i].SetSpecular(l.GetSpecular());

		if (l.GetType() == Light::LIGHT_DIRECTIONAL)
			m_numDirLights++;

		assert(m_numDirLights <= TOTAL_NUM_LIGHTS);
	}

	return true;
}

bool RendererGL2::SetAmbientColor(const Color &c)
{
	m_ambient = c;
	return true;
}

bool RendererGL2::SetScissor(bool enabled, const vector2f &pos, const vector2f &size)
{
	if (enabled) {
		gl::Scissor(pos.x,pos.y,size.x,size.y);
		gl::Enable(gl::SCISSOR_TEST);
	}
	else
		gl::Disable(gl::SCISSOR_TEST);
	return true;
}

void RendererGL2::SetMaterialShaderTransforms(Material *m)
{
	m->SetCommonUniforms(m_modelViewStack.top(), m_projectionStack.top());
	CheckRenderErrors(__FUNCTION__,__LINE__);
}

bool RendererGL2::DrawTriangles(const VertexArray *v, RenderState *rs, Material *m, PrimitiveType t)
{
	PROFILE_SCOPED()
	if (!v || v->position.size() < 3) return false;

	SetRenderState(rs);

	SetMaterialShaderTransforms(m);

	m->Apply();
	EnableVertexAttributes(v);

	gl::DrawArrays(t, 0, v->GetNumVerts());

	m->Unapply();
	DisableVertexAttributes();

	return true;
}

bool RendererGL2::DrawPointSprites(const Uint32 count, const vector3f *positions, RenderState *rs, Material *material, float size)
{
	PROFILE_SCOPED()
	if (count < 1 || !material || !material->texture0) return false;

	VertexArray va(ATTRIB_POSITION | ATTRIB_UV0, count * 6);

	matrix4x4f rot(GetCurrentModelView());
	rot.ClearToRotOnly();
	rot = rot.Inverse();

	const float sz = 0.5f*size;
	const vector3f rotv1 = rot * vector3f(sz, sz, 0.0f);
	const vector3f rotv2 = rot * vector3f(sz, -sz, 0.0f);
	const vector3f rotv3 = rot * vector3f(-sz, -sz, 0.0f);
	const vector3f rotv4 = rot * vector3f(-sz, sz, 0.0f);

	//do two-triangle quads. Could also do indexed surfaces.
	//GL2 renderer should use actual point sprites
	//(see history of Render.cpp for point code remnants)
	for (Uint32 i=0; i<count; i++) {
		const vector3f &pos = positions[i];

		va.Add(pos+rotv4, vector2f(0.f, 0.f)); //top left
		va.Add(pos+rotv3, vector2f(0.f, 1.f)); //bottom left
		va.Add(pos+rotv1, vector2f(1.f, 0.f)); //top right

		va.Add(pos+rotv1, vector2f(1.f, 0.f)); //top right
		va.Add(pos+rotv3, vector2f(0.f, 1.f)); //bottom left
		va.Add(pos+rotv2, vector2f(1.f, 1.f)); //bottom right
	}

	DrawTriangles(&va, rs, material);

	return true;
}

bool RendererGL2::DrawPointSprites(const Uint32 count, const vector3f *positions, const vector2f *offsets, const float *sizes, RenderState *rs, Material *material)
{
	PROFILE_SCOPED()
	/*if (count == 0 || !material || !material->texture0) 
		return false;

	#pragma pack(push, 4)
	struct PosNormVert {
		vector3f pos;
		vector3f norm;
	};
	#pragma pack(pop)
	
	RefCountedPtr<VertexBuffer> drawVB;
	AttribBufferIter iter = s_AttribBufferMap.find(std::make_pair(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL, count));
	if (iter == s_AttribBufferMap.end()) 
	{
		// NB - we're (ab)using the normal type to hold (uv coordinate offset value + point size)
		Graphics::VertexBufferDesc vbd;
		vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
		vbd.attrib[0].format   = Graphics::ATTRIB_FORMAT_FLOAT3;
		vbd.attrib[1].semantic = Graphics::ATTRIB_NORMAL;
		vbd.attrib[1].format   = Graphics::ATTRIB_FORMAT_FLOAT3;
		vbd.numVertices = count;
		vbd.usage = Graphics::BUFFER_USAGE_DYNAMIC;	// we could be updating this per-frame

		// VertexBuffer
		RefCountedPtr<VertexBuffer> vb;
		vb.Reset(CreateVertexBuffer(vbd));

		// add to map
		s_AttribBufferMap[std::make_pair(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL, count)] = vb;
		drawVB = vb;
	}
	else
	{
		drawVB = iter->second;
	}

	// got a buffer so use it and fill it with newest data
	PosNormVert* vtxPtr = drawVB->Map<PosNormVert>(Graphics::BUFFER_MAP_WRITE);
	assert(drawVB->GetDesc().stride == sizeof(PosNormVert));
	for(Uint32 i=0 ; i<count ; i++)
	{
		vtxPtr[i].pos	= positions[i];
		vtxPtr[i].norm	= vector3f(offsets[i], Clamp(sizes[i], 0.1f, FLT_MAX));
	}
	drawVB->Unmap();

	SetTransform(matrix4x4f::Identity());
	DrawBuffer(drawVB.Get(), rs, material, Graphics::POINTS);
	GetStats().AddToStatCount(Graphics::Stats::STAT_DRAWPOINTSPRITES, 1);
	CheckRenderErrors(__FUNCTION__,__LINE__);*/

	return true;
}

bool RendererGL2::DrawBuffer(VertexBuffer* vb, RenderState* state, Material* mat, PrimitiveType pt)
{
	PROFILE_SCOPED()
	SetRenderState(state);
	mat->Apply();

	SetMaterialShaderTransforms(mat);

	auto gvb = static_cast<GL2::VertexBuffer*>(vb);

	gvb->Bind();
	EnableVertexAttributes(gvb);

	gl::DrawArrays(pt, 0, gvb->GetVertexCount());

	DisableVertexAttributes(gvb);
	gvb->Release();

	return true;
}

bool RendererGL2::DrawBufferIndexed(VertexBuffer *vb, IndexBuffer *ib, RenderState *state, Material *mat, PrimitiveType pt)
{
	PROFILE_SCOPED()
	SetRenderState(state);
	mat->Apply();

	SetMaterialShaderTransforms(mat);

	auto gvb = static_cast<GL2::VertexBuffer*>(vb);
	auto gib = static_cast<GL2::IndexBuffer*>(ib);

	gvb->Bind();
	gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, gib->GetBuffer());
	EnableVertexAttributes(gvb);

	gl::DrawElements(pt, ib->GetIndexCount(), gl::UNSIGNED_SHORT, 0);

	DisableVertexAttributes(gvb);
	gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, 0);
	gvb->Release();
	

	return true;
}

bool RendererGL2::DrawBufferInstanced(VertexBuffer* vb, RenderState* state, Material* mat, InstanceBuffer* instb, PrimitiveType pt)
{
	PROFILE_SCOPED()
	SetRenderState(state);
	mat->Apply();

	SetMaterialShaderTransforms(mat);

	vb->Bind();
	instb->Bind();
	gl::DrawArraysInstancedARB(pt, 0, vb->GetVertexCount(), instb->GetInstanceCount());
	instb->Release();
	vb->Release();
	CheckRenderErrors(__FUNCTION__,__LINE__);

	m_stats.AddToStatCount(Stats::STAT_DRAWCALL, 1);

	return true;
}

bool RendererGL2::DrawBufferIndexedInstanced(VertexBuffer *vb, IndexBuffer *ib, RenderState *state, Material *mat, InstanceBuffer* instb, PrimitiveType pt)
{
	PROFILE_SCOPED()
	SetRenderState(state);
	mat->Apply();

	SetMaterialShaderTransforms(mat);

	vb->Bind();
	ib->Bind();
	instb->Bind();
	gl::DrawElementsInstancedARB(pt, ib->GetIndexCount(), gl::UNSIGNED_SHORT, 0, instb->GetInstanceCount());
	instb->Release();
	ib->Release();
	vb->Release();
	CheckRenderErrors(__FUNCTION__,__LINE__);

	m_stats.AddToStatCount(Stats::STAT_DRAWCALL, 1);

	return true;
}

void RendererGL2::EnableVertexAttributes(const VertexBuffer* gvb)
{
	PROFILE_SCOPED()
	const auto &desc = gvb->GetDesc();
	// Enable the Vertex attributes
	for (Uint8 i = 0; i < MAX_ATTRIBS; i++) {
		const auto& attr = desc.attrib[i];
		switch (attr.semantic) {
		case ATTRIB_POSITION:		gl::EnableVertexAttribArray(0);		break;
		case ATTRIB_NORMAL:			gl::EnableVertexAttribArray(1);		break;
		case ATTRIB_DIFFUSE:		gl::EnableVertexAttribArray(2);		break;
		case ATTRIB_UV0:			gl::EnableVertexAttribArray(3);		break;
		case ATTRIB_NONE:
		default:
			return;
		}
	}
}

void RendererGL2::DisableVertexAttributes(const VertexBuffer* gvb)
{
	PROFILE_SCOPED()
	const auto &desc = gvb->GetDesc();
	// Enable the Vertex attributes
	for (Uint8 i = 0; i < MAX_ATTRIBS; i++) {
		const auto& attr = desc.attrib[i];
		switch (attr.semantic) {
		case ATTRIB_POSITION:		gl::DisableVertexAttribArray(0);			break;
		case ATTRIB_NORMAL:			gl::DisableVertexAttribArray(1);			break;
		case ATTRIB_DIFFUSE:		gl::DisableVertexAttribArray(2);			break;
		case ATTRIB_UV0:			gl::DisableVertexAttribArray(3);			break;
		case ATTRIB_NONE:
		default:
			return;
		}
	}
}


void RendererGL2::EnableVertexAttributes(const VertexArray *v)
{
	PROFILE_SCOPED();

	if (!v) return;
	assert(v->position.size() > 0); //would be strange

	// XXX could be 3D or 2D
	m_vertexAttribsSet.push_back(0);
	gl::EnableVertexAttribArray(0);	// Enable the attribute at that location
	gl::VertexAttribPointer(0, 3, gl::FLOAT, gl::FALSE_, 0, reinterpret_cast<const GLvoid *>(&v->position[0]));

	if (v->HasAttrib(ATTRIB_NORMAL)) {
		assert(! v->normal.empty());
		m_vertexAttribsSet.push_back(1);
		gl::EnableVertexAttribArray(1);	// Enable the attribute at that location
		gl::VertexAttribPointer(1, 3, gl::FLOAT, gl::FALSE_, 0, reinterpret_cast<const GLvoid *>(&v->normal[0]));
	}
	if (v->HasAttrib(ATTRIB_DIFFUSE)) {
		assert(! v->diffuse.empty());
		m_vertexAttribsSet.push_back(2);
		gl::EnableVertexAttribArray(2);	// Enable the attribute at that location
		gl::VertexAttribPointer(2, 4, gl::UNSIGNED_BYTE, gl::TRUE_, 0, reinterpret_cast<const GLvoid *>(&v->diffuse[0]));	// only normalise the colours
	}
	if (v->HasAttrib(ATTRIB_UV0)) {
		assert(! v->uv0.empty());
		m_vertexAttribsSet.push_back(3);
		gl::EnableVertexAttribArray(3);	// Enable the attribute at that location
		gl::VertexAttribPointer(3, 2, gl::FLOAT, gl::FALSE_, 0, reinterpret_cast<const GLvoid *>(&v->uv0[0]));
	}
}

void RendererGL2::DisableVertexAttributes()
{
	PROFILE_SCOPED();

	for (auto i : m_vertexAttribsSet) {
		gl::DisableVertexAttribArray(i);
	}
	m_vertexAttribsSet.clear();
}

Material *RendererGL2::CreateMaterial(const MaterialDescriptor &d)
{
	PROFILE_SCOPED()
	MaterialDescriptor desc = d;

	GL2::Material *mat = 0;
	GL2::Program *p = 0;

	if (desc.lighting) {
		desc.dirLights = m_numDirLights;
	}

	// Create the material. It will be also used to create the shader,
	// like a tiny factory
	switch (desc.effect) {
	case EFFECT_VTXCOLOR:
		mat = new GL2::VtxColorMaterial();
		break;
	case EFFECT_UI:
		mat = new GL2::UIMaterial();
		break;
	case EFFECT_PLANETRING:
		mat = new GL2::RingMaterial();
		break;
	case EFFECT_STARFIELD:
		mat = new GL2::StarfieldMaterial();
		break;
	case EFFECT_GEOSPHERE_TERRAIN:
	case EFFECT_GEOSPHERE_TERRAIN_WITH_LAVA:
	case EFFECT_GEOSPHERE_TERRAIN_WITH_WATER:
		mat = new GL2::GeoSphereSurfaceMaterial();
		break;
	case EFFECT_GEOSPHERE_SKY:
		mat = new GL2::GeoSphereSkyMaterial();
		break;
	case EFFECT_GEOSPHERE_STAR:
		mat = new GL2::GeoSphereStarMaterial();
		break;
	case EFFECT_FRESNEL_SPHERE:
		mat = new GL2::FresnelColourMaterial();
		break;
	case EFFECT_SHIELD:
		mat = new GL2::ShieldMaterial();
		break;
	case EFFECT_SKYBOX:
		mat = new GL2::SkyboxMaterial();
		break;
	case EFFECT_SPHEREIMPOSTOR:
		mat = new GL2::SphereImpostorMaterial();
		break;
	case EFFECT_GASSPHERE_TERRAIN:
		mat = new GL2::GasGiantSurfaceMaterial();
		break;
	default:
		if (desc.lighting)
			mat = new GL2::LitMultiMaterial();
		else
			mat = new GL2::MultiMaterial();
	}

	mat->m_renderer = this;
	mat->m_descriptor = desc;

	p = GetOrCreateProgram(mat); // XXX throws ShaderException on compile/link failure

	mat->SetProgram(p);
	return mat;
}

bool RendererGL2::ReloadShaders()
{
	Output("Reloading " SIZET_FMT " programs...\n", m_programs.size());
	for (ProgramIterator it = m_programs.begin(); it != m_programs.end(); ++it) {
		it->second->Reload();
	}
	Output("Done.\n");

	return true;
}

GL2::Program* RendererGL2::GetOrCreateProgram(GL2::Material *mat)
{
	const MaterialDescriptor &desc = mat->GetDescriptor();
	GL2::Program *p = 0;

	// Find an existing program...
	for (ProgramIterator it = m_programs.begin(); it != m_programs.end(); ++it) {
		if ((*it).first == desc) {
			p = (*it).second;
			break;
		}
	}

	// ...or create a new one
	if (!p) {
		p = mat->CreateProgram(desc);
		m_programs.push_back(std::make_pair(desc, p));
	}

	return p;
}

Texture *RendererGL2::CreateTexture(const TextureDescriptor &descriptor)
{
	return new GL2Texture(descriptor, m_useCompressedTextures);
}

RenderState *RendererGL2::CreateRenderState(const RenderStateDesc &desc)
{
	const uint32_t hash = lookup3_hashlittle(&desc, sizeof(RenderStateDesc), 0);
	auto it = m_renderStates.find(hash);
	if (it != m_renderStates.end())
		return it->second;
	else {
		auto *rs = new GL2::RenderState(desc);
		m_renderStates[hash] = rs;
		return rs;
	}
}

RenderTarget *RendererGL2::CreateRenderTarget(const RenderTargetDesc &desc)
{
	GL2::RenderTarget* rt = new GL2::RenderTarget(desc);
	rt->Bind();
	if (desc.colorFormat != TEXTURE_NONE) {
		Graphics::TextureDescriptor cdesc(
			desc.colorFormat,
			vector2f(desc.width, desc.height),
			vector2f(desc.width, desc.height),
			LINEAR_CLAMP,
			false,
			false,
			false,
			0,Graphics::TEXTURE_2D);
		GL2Texture *colorTex = new GL2Texture(cdesc, false);
		rt->SetColorTexture(colorTex);
	}
	if (desc.depthFormat != TEXTURE_NONE) {
		if (desc.allowDepthTexture) {
			Graphics::TextureDescriptor ddesc(
				TEXTURE_DEPTH,
				vector2f(desc.width, desc.height),
				vector2f(desc.width, desc.height),
				LINEAR_CLAMP,
				false,
				false,
				false,
				0,Graphics::TEXTURE_2D);
			GL2Texture *depthTex = new GL2Texture(ddesc, false);
			rt->SetDepthTexture(depthTex);
		} else {
			rt->CreateDepthRenderbuffer();
		}
	}
	rt->CheckStatus();
	rt->Unbind();
	return rt;
}

VertexBuffer *RendererGL2::CreateVertexBuffer(const VertexBufferDesc &desc)
{
	return new GL2::VertexBuffer(desc);
}

IndexBuffer *RendererGL2::CreateIndexBuffer(Uint32 size, BufferUsage usage)
{
	return new GL2::IndexBuffer(size, usage);
}

InstanceBuffer *RendererGL2::CreateInstanceBuffer(Uint32 size, BufferUsage usage)
{
	m_stats.AddToStatCount(Stats::STAT_CREATE_BUFFER, 1);
	return new GL2::InstanceBuffer(size, usage);
}

// XXX very heavy. in the future when all GL calls are made through the
// renderer, we can probably do better by trackingn current state and
// only restoring the things that have changed
void RendererGL2::PushState()
{
	SetMatrixMode(MatrixMode::PROJECTION);
	PushMatrix();
	SetMatrixMode(MatrixMode::MODELVIEW);
	PushMatrix();
	m_viewportStack.push( m_viewportStack.top() );
	gl::PushAttrib(gl::ALL_ATTRIB_BITS & (~gl::POINT_BIT));
}

void RendererGL2::PopState()
{
	gl::PopAttrib();
	m_viewportStack.pop();
	assert(!m_viewportStack.empty());
	SetMatrixMode(MatrixMode::PROJECTION);
	PopMatrix();
	SetMatrixMode(MatrixMode::MODELVIEW);
	PopMatrix();
}

void RendererGL2::SetMatrixMode(MatrixMode mm)
{
	PROFILE_SCOPED()
	if( mm != m_matrixMode ) {
		switch (mm) {
			case MatrixMode::MODELVIEW:
				gl::MatrixMode(gl::MODELVIEW);
				break;
			case MatrixMode::PROJECTION:
				gl::MatrixMode(gl::PROJECTION);
				break;
		}
		m_matrixMode = mm;
	}
}

void RendererGL2::PushMatrix()
{
	PROFILE_SCOPED()

	gl::PushMatrix();
	switch(m_matrixMode) {
		case MatrixMode::MODELVIEW:
			m_modelViewStack.push(m_modelViewStack.top());
			break;
		case MatrixMode::PROJECTION:
			m_projectionStack.push(m_projectionStack.top());
			break;
	}
}

void RendererGL2::PopMatrix()
{
	PROFILE_SCOPED()
	gl::PopMatrix();
	switch(m_matrixMode) {
		case MatrixMode::MODELVIEW:
			m_modelViewStack.pop();
			assert(m_modelViewStack.size());
			break;
		case MatrixMode::PROJECTION:
			m_projectionStack.pop();
			assert(m_projectionStack.size());
			break;
	}
}

void RendererGL2::LoadIdentity()
{
	PROFILE_SCOPED()
	gl::LoadIdentity();
	switch(m_matrixMode) {
		case MatrixMode::MODELVIEW:
			m_modelViewStack.top() = matrix4x4f::Identity();
			break;
		case MatrixMode::PROJECTION:
			m_projectionStack.top() = matrix4x4f::Identity();
			break;
	}
}

void RendererGL2::LoadMatrix(const matrix4x4f &m)
{
	PROFILE_SCOPED()
	gl::LoadMatrixf(&m[0]);
	switch(m_matrixMode) {
		case MatrixMode::MODELVIEW:
			m_modelViewStack.top() = m;
			break;
		case MatrixMode::PROJECTION:
			m_projectionStack.top() = m;
			break;
	}
}

void RendererGL2::Translate( const float x, const float y, const float z )
{
	PROFILE_SCOPED()
	gl::Translatef(x,y,z);
	switch(m_matrixMode) {
		case MatrixMode::MODELVIEW:
			m_modelViewStack.top().Translate(x,y,z);
			break;
		case MatrixMode::PROJECTION:
			m_projectionStack.top().Translate(x,y,z);
			break;
	}
}

void RendererGL2::Scale( const float x, const float y, const float z )
{
	PROFILE_SCOPED()
	gl::Scalef(x,y,z);
	switch(m_matrixMode) {
		case MatrixMode::MODELVIEW:
			m_modelViewStack.top().Scale(x,y,z);
			break;
		case MatrixMode::PROJECTION:
			m_modelViewStack.top().Scale(x,y,z);
			break;
	}
}

}
