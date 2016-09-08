// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "RendererGL.h"
#include "graphics/Graphics.h"
#include "graphics/Light.h"
#include "graphics/Material.h"
#include "OS.h"
#include "StringF.h"
#include "graphics/Texture.h"
#include "TextureGL.h"
#include "graphics/VertexArray.h"
#include "GLDebug.h"
#include "GasGiantMaterial.h"
#include "GeoSphereMaterial.h"
#include "GenGasGiantColourMaterial.h"
#include "MaterialGL.h"
#include "RenderStateGL.h"
#include "RenderTargetGL.h"
#include "VertexBufferGL.h"
#include "MultiMaterial.h"
#include "Program.h"
#include "RingMaterial.h"
#include "StarfieldMaterial.h"
#include "FresnelColourMaterial.h"
#include "ShieldMaterial.h"
#include "SkyboxMaterial.h"
#include "SphereImpostorMaterial.h"
#include "UIMaterial.h"
#include "VtxColorMaterial.h"
#include "BillboardMaterial.h"

#include <stddef.h> //for offsetof
#include <ostream>
#include <sstream>
#include <iterator>

namespace Graphics {

static Renderer *CreateRenderer(WindowSDL *win, const Settings &vs) {
    return new RendererOGL(win, vs);
}

// static method instantiations
void RendererOGL::RegisterRenderer() {
    Graphics::RegisterRenderer(Graphics::RENDERER_OPENGL, CreateRenderer);
}

// static member instantiations
bool RendererOGL::initted = false;
RendererOGL::AttribBufferMap RendererOGL::s_AttribBufferMap;

// typedefs
typedef std::vector<std::pair<MaterialDescriptor, OGL::Program*> >::const_iterator ProgramIterator;

// ----------------------------------------------------------------------------
RendererOGL::RendererOGL(WindowSDL *window, const Graphics::Settings &vs)
: Renderer(window, window->GetWidth(), window->GetHeight())
, m_numLights(0)
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
				"Pioneer can not run on your graphics card as it does not appear to support OpenGL 3.3\n"
				"Please check to see if your GPU driver vendor has an updated driver - or that drivers are installed correctly."
			);

		if (!gl::exts::var_EXT_texture_compression_s3tc)
			Error(
				"OpenGL extension GL_EXT_texture_compression_s3tc not supported.\n"
				"Pioneer can not run on your graphics card as it does not support compressed (DXTn/S3TC) format textures."
			);
	}

	m_viewportStack.push(Viewport());

	const bool useDXTnTextures = vs.useTextureCompression;
	m_useCompressedTextures = useDXTnTextures;

	const bool useAnisotropicFiltering = vs.useAnisotropicFiltering;
	m_useAnisotropicFiltering = useAnisotropicFiltering;

	//XXX bunch of fixed function states here!
	gl::CullFace(gl::BACK);
	gl::FrontFace(gl::CCW);
	gl::Enable(gl::CULL_FACE);
	gl::Enable(gl::DEPTH_TEST);
	gl::DepthFunc(gl::LESS);
	gl::DepthRange(0.0,1.0);
	gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
	gl::Hint(gl::LINE_SMOOTH_HINT, gl::NICEST);
	gl::Enable(gl::TEXTURE_CUBE_MAP_SEAMLESS);
	gl::Enable(gl::PROGRAM_POINT_SIZE);

	gl::Hint(gl::TEXTURE_COMPRESSION_HINT, gl::NICEST);
	gl::Hint(gl::FRAGMENT_SHADER_DERIVATIVE_HINT, gl::NICEST);

	SetMatrixMode(MatrixMode::MODELVIEW);

	m_modelViewStack.push(matrix4x4f::Identity());
	m_projectionStack.push(matrix4x4f::Identity());

	SetClearColor(Color4f(0.f, 0.f, 0.f, 0.f));
	SetViewport(0, 0, m_width, m_height);

	if (vs.enableDebugMessages)
		GLDebug::Enable();

	// check enum PrimitiveType matches OpenGL values
	assert(POINTS == gl::POINTS);
	assert(LINE_SINGLE == gl::LINES);
	assert(LINE_LOOP == gl::LINE_LOOP);
	assert(LINE_STRIP == gl::LINE_STRIP);
	assert(TRIANGLES == gl::TRIANGLES);
	assert(TRIANGLE_STRIP == gl::TRIANGLE_STRIP);
	assert(TRIANGLE_FAN == gl::TRIANGLE_FAN);
}

RendererOGL::~RendererOGL()
{
	// HACK ANDYC - this crashes when shutting down? They'll be released anyway right?
	//while (!m_programs.empty()) delete m_programs.back().second, m_programs.pop_back();
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
		case gl::INVALID_FRAMEBUFFER_OPERATION: return "invalid framebuffer operation";
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
	} else {
		while (err != gl::NO_ERROR_) {
			if (err == gl::INVALID_ENUM) { out << name << " -- not supported\n"; }
			else { out << name << " -- unexpected error (" << err << ") retrieving value\n"; }
			err = gl::GetError();
		}
	}
}

void RendererOGL::WriteRendererInfo(std::ostream &out) const
{
	out << "OpenGL version " << gl::GetString(gl::VERSION);
	out << ", running on " << gl::GetString(gl::VENDOR);
	out << " " << gl::GetString(gl::RENDERER) << "\n";

	out << "Available extensions:" << "\n";
	{
		out << "Shading language version: " <<  gl::GetString(gl::SHADING_LANGUAGE_VERSION) << "\n";
		GLint numext = 0;
		gl::GetIntegerv(gl::NUM_EXTENSIONS, &numext);
		for (int i = 0; i < numext; ++i) {
			out << "  " << gl::GetStringi(gl::EXTENSIONS, i) << "\n";
		}
	}

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
		} else {
			assert(nformats >= 0 && nformats < int(COUNTOF(formats)));
			gl::GetIntegerv(gl::COMPRESSED_TEXTURE_FORMATS, formats);
			err = gl::GetError();
			if (err != gl::NO_ERROR_) {
				out << "Get COMPRESSED_TEXTURE_FORMATS failed\n";
				dump_and_clear_opengl_errors(out, err);
			} else {
				for (int i = 0; i < nformats; ++i) {
					out << stringf("  %0{x#}\n", unsigned(formats[i]));
				}
			}
		}
	}
	// one last time
	dump_and_clear_opengl_errors(out);
}

bool RendererOGL::GetNearFarRange(float &near_, float &far_) const
{
	near_ = m_minZNear;
	far_ = m_maxZFar;
	return true;
}

bool RendererOGL::BeginFrame()
{
	PROFILE_SCOPED()
	gl::ClearColor(0,0,0,0);
	gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
	return true;
}

bool RendererOGL::EndFrame()
{
	return true;
}

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
	default:
		return stringf("Unknown error 0x0%0{x}", err);
	}
}

void RendererOGL::CheckErrors(const char *func /*= nullptr*/, const int line /*= nullptr*/)
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
			ss << "In function " << std::string(func) << "\nOn line " << std::to_string(line) << "\n";
		}
		ss << "OpenGL error(s) during frame:\n";
		while (err != gl::NO_ERROR_) {
			ss << glerr_to_string(err) << '\n';
			err = gl::GetError();
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
		}
		// show warning dialog or just log to output
		if(showWarning)
			Warning("%s", ss.str().c_str());
		else
			Output("%s", ss.str().c_str());
	}
#endif
}

bool RendererOGL::SwapBuffers()
{
	PROFILE_SCOPED()
#ifndef NDEBUG
	// Check if an error occurred during the frame. This is not very useful for
	// determining *where* the error happened. For that purpose, try GDebugger or
	// the gl::KHR_DEBUG extension
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
	m_stats.NextFrame();
	return true;
}

bool RendererOGL::SetRenderState(RenderState *rs)
{
	if (m_activeRenderState != rs) {
		static_cast<OGL::RenderState*>(rs)->Apply();
		m_activeRenderState = rs;
	}
	CheckRenderErrors(__FUNCTION__,__LINE__);
	return true;
}

bool RendererOGL::SetRenderTarget(RenderTarget *rt)
{
	PROFILE_SCOPED()
	if (rt)
		static_cast<OGL::RenderTarget*>(rt)->Bind();
	else if (m_activeRenderTarget)
		m_activeRenderTarget->Unbind();

	m_activeRenderTarget = static_cast<OGL::RenderTarget*>(rt);
	CheckRenderErrors(__FUNCTION__,__LINE__);

	return true;
}

bool RendererOGL::SetDepthRange(double znear, double zfar)
{
	gl::DepthRange(znear, zfar);
	return true;
}

bool RendererOGL::ClearScreen()
{
	m_activeRenderState = nullptr;
	gl::Enable(gl::DEPTH_TEST);
	gl::DepthMask(gl::TRUE_);
	gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
	CheckRenderErrors(__FUNCTION__,__LINE__);

	return true;
}

bool RendererOGL::ClearDepthBuffer()
{
	m_activeRenderState = nullptr;
	gl::Enable(gl::DEPTH_TEST);
	gl::DepthMask(gl::TRUE_);
	gl::Clear(gl::DEPTH_BUFFER_BIT);
	CheckRenderErrors(__FUNCTION__,__LINE__);

	return true;
}

bool RendererOGL::SetClearColor(const Color &c)
{
	gl::ClearColor(c.r, c.g, c.b, c.a);
	return true;
}

bool RendererOGL::SetViewport(int x, int y, int width, int height)
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

bool RendererOGL::SetTransform(const matrix4x4d &m)
{
	PROFILE_SCOPED()
	matrix4x4f mf;
	matrix4x4dtof(m, mf);
	return SetTransform(mf);
}

bool RendererOGL::SetTransform(const matrix4x4f &m)
{
	PROFILE_SCOPED()
	//same as above
	SetMatrixMode(MatrixMode::MODELVIEW);
	LoadMatrix(m);
	return true;
}

bool RendererOGL::SetPerspectiveProjection(float fov, float aspect, float near_, float far_)
{
	PROFILE_SCOPED()

	// update values for log-z hack
	m_invLogZfarPlus1 = 1.0f / (log1p(far_)/log(2.0f));

	Graphics::SetFov(fov);

	float ymax = near_ * tan(fov * M_PI / 360.0);
	float ymin = -ymax;
	float xmin = ymin * aspect;
	float xmax = ymax * aspect;

	const matrix4x4f frustrumMat = matrix4x4f::FrustumMatrix(xmin, xmax, ymin, ymax, near_, far_);
	SetProjection(frustrumMat);
	return true;
}

bool RendererOGL::SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax)
{
	PROFILE_SCOPED()
	const matrix4x4f orthoMat = matrix4x4f::OrthoFrustum(xmin, xmax, ymin, ymax, zmin, zmax);
	SetProjection(orthoMat);
	return true;
}

bool RendererOGL::SetProjection(const matrix4x4f &m)
{
	PROFILE_SCOPED()
	//same as above
	SetMatrixMode(MatrixMode::PROJECTION);
	LoadMatrix(m);
	return true;
}

bool RendererOGL::SetWireFrameMode(bool enabled)
{
	gl::PolygonMode(gl::FRONT_AND_BACK, enabled ? gl::LINE : gl::FILL);
	return true;
}

bool RendererOGL::SetLights(Uint32 numlights, const Light *lights)
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
		m_lights[i].SetPosition( l.GetPosition() );
		m_lights[i].SetDiffuse( l.GetDiffuse() );
		m_lights[i].SetSpecular( l.GetSpecular() );

		if (l.GetType() == Light::LIGHT_DIRECTIONAL)
			m_numDirLights++;

		assert(m_numDirLights <= TOTAL_NUM_LIGHTS);
	}

	return true;
}

bool RendererOGL::SetAmbientColor(const Color &c)
{
	m_ambient = c;
	return true;
}

bool RendererOGL::SetScissor(bool enabled, const vector2f &pos, const vector2f &size)
{
	if (enabled) {
		gl::Scissor(pos.x,pos.y,size.x,size.y);
		gl::Enable(gl::SCISSOR_TEST);
	} else {
		gl::Disable(gl::SCISSOR_TEST);
	}
	return true;
}

void RendererOGL::SetMaterialShaderTransforms(Material *m)
{
	m->SetCommonUniforms(m_modelViewStack.top(), m_projectionStack.top());
	CheckRenderErrors(__FUNCTION__,__LINE__);
}

bool RendererOGL::DrawTriangles(const VertexArray *v, RenderState *rs, Material *m, PrimitiveType t)
{
	PROFILE_SCOPED()
	if (!v || v->position.size() < 3) return false;

	const AttributeSet attribs = v->GetAttributeSet();
	RefCountedPtr<VertexBuffer> drawVB;

	// see if we have a buffer to re-use
	AttribBufferIter iter = s_AttribBufferMap.find(std::make_pair(attribs, v->position.size()));
	if (iter == s_AttribBufferMap.end()) {
		// not found a buffer so create a new one
		VertexBufferDesc vbd;
		Uint32 attribIdx = 0;
		assert(v->HasAttrib(ATTRIB_POSITION));
		vbd.attrib[attribIdx].semantic = ATTRIB_POSITION;
		vbd.attrib[attribIdx].format = ATTRIB_FORMAT_FLOAT3;
		++attribIdx;

		if (v->HasAttrib(ATTRIB_NORMAL)) {
			vbd.attrib[attribIdx].semantic = ATTRIB_NORMAL;
			vbd.attrib[attribIdx].format = ATTRIB_FORMAT_FLOAT3;
			++attribIdx;
		}
		if (v->HasAttrib(ATTRIB_DIFFUSE)) {
			vbd.attrib[attribIdx].semantic = ATTRIB_DIFFUSE;
			vbd.attrib[attribIdx].format = ATTRIB_FORMAT_UBYTE4;
			++attribIdx;
		}
		if (v->HasAttrib(ATTRIB_UV0)) {
			vbd.attrib[attribIdx].semantic = ATTRIB_UV0;
			vbd.attrib[attribIdx].format = ATTRIB_FORMAT_FLOAT2;
			++attribIdx;
		}
		if (v->HasAttrib(ATTRIB_TANGENT)) {
			vbd.attrib[attribIdx].semantic = ATTRIB_TANGENT;
			vbd.attrib[attribIdx].format = ATTRIB_FORMAT_FLOAT3;
			++attribIdx;
		}
		vbd.numVertices = v->position.size();
		vbd.usage = BUFFER_USAGE_DYNAMIC;	// dynamic since we'll be reusing these buffers if possible

		// VertexBuffer
		RefCountedPtr<VertexBuffer> vb;
		vb.Reset(CreateVertexBuffer(vbd));
		vb->Populate(*v);

		// add to map
		s_AttribBufferMap[std::make_pair(attribs, v->position.size())] = vb;
		drawVB = vb;
	}
	else {
		// got a buffer so use it and fill it with newest data
		drawVB = iter->second;
		drawVB->Populate(*v);
	}

	const bool res = DrawBuffer(drawVB.Get(), rs, m, t);
	CheckRenderErrors(__FUNCTION__,__LINE__);
	
	m_stats.AddToStatCount(Stats::STAT_DRAWTRIS, 1);

	return res;
}

bool RendererOGL::DrawPointSprites(const Uint32 count, const vector3f *positions, RenderState *rs, Material *material, float size)
{
	PROFILE_SCOPED()
	if (count == 0 || !material || !material->texture0) 
		return false;

	size = Clamp(size, 0.1f, FLT_MAX);

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
		vtxPtr[i].norm	= vector3f(0.0f, 0.0f, size);
	}
	drawVB->Unmap();

	SetTransform(matrix4x4f::Identity());
	DrawBuffer(drawVB.Get(), rs, material, Graphics::POINTS);
	GetStats().AddToStatCount(Graphics::Stats::STAT_DRAWPOINTSPRITES, 1);
	CheckRenderErrors(__FUNCTION__,__LINE__);

	return true;
}

bool RendererOGL::DrawPointSprites(const Uint32 count, const vector3f *positions, const vector2f *offsets, const float *sizes, RenderState *rs, Material *material)
{
	PROFILE_SCOPED()
	if (count == 0 || !material || !material->texture0) 
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
	CheckRenderErrors(__FUNCTION__,__LINE__);

	return true;
}

bool RendererOGL::DrawBuffer(VertexBuffer* vb, RenderState* state, Material* mat, PrimitiveType pt)
{
	PROFILE_SCOPED()
	SetRenderState(state);
	mat->Apply();

	SetMaterialShaderTransforms(mat);

	vb->Bind();
	gl::DrawArrays(pt, 0, vb->GetVertexCount());
	vb->Release();
	CheckRenderErrors(__FUNCTION__,__LINE__);

	m_stats.AddToStatCount(Stats::STAT_DRAWCALL, 1);

	return true;
}

bool RendererOGL::DrawBufferIndexed(VertexBuffer *vb, IndexBuffer *ib, RenderState *state, Material *mat, PrimitiveType pt)
{
	PROFILE_SCOPED()
	SetRenderState(state);
	mat->Apply();

	SetMaterialShaderTransforms(mat);

	vb->Bind();
	ib->Bind();
	gl::DrawElements(pt, ib->GetIndexCount(), gl::UNSIGNED_INT, 0);
	ib->Release();
	vb->Release();
	CheckRenderErrors(__FUNCTION__,__LINE__);

	m_stats.AddToStatCount(Stats::STAT_DRAWCALL, 1);

	return true;
}

bool RendererOGL::DrawBufferInstanced(VertexBuffer* vb, RenderState* state, Material* mat, InstanceBuffer* instb, PrimitiveType pt)
{
	PROFILE_SCOPED()
	SetRenderState(state);
	mat->Apply();

	SetMaterialShaderTransforms(mat);

	vb->Bind();
	instb->Bind();
	gl::DrawArraysInstanced(pt, 0, vb->GetVertexCount(), instb->GetInstanceCount());
	instb->Release();
	vb->Release();
	CheckRenderErrors(__FUNCTION__,__LINE__);

	m_stats.AddToStatCount(Stats::STAT_DRAWCALL, 1);

	return true;
}

bool RendererOGL::DrawBufferIndexedInstanced(VertexBuffer *vb, IndexBuffer *ib, RenderState *state, Material *mat, InstanceBuffer* instb, PrimitiveType pt)
{
	PROFILE_SCOPED()
	SetRenderState(state);
	mat->Apply();

	SetMaterialShaderTransforms(mat);

	vb->Bind();
	ib->Bind();
	instb->Bind();
	gl::DrawElementsInstanced(pt, ib->GetIndexCount(), gl::UNSIGNED_INT, 0, instb->GetInstanceCount());
	instb->Release();
	ib->Release();
	vb->Release();
	CheckRenderErrors(__FUNCTION__,__LINE__);

	m_stats.AddToStatCount(Stats::STAT_DRAWCALL, 1);

	return true;
}

Material *RendererOGL::CreateMaterial(const MaterialDescriptor &d)
{
	PROFILE_SCOPED()
	MaterialDescriptor desc = d;

	OGL::Material *mat = 0;
	OGL::Program *p = 0;

	if (desc.lighting) {
		desc.dirLights = m_numDirLights;
	}

	// Create the material. It will be also used to create the shader,
	// like a tiny factory
	switch (desc.effect) {
	case EFFECT_VTXCOLOR:
		mat = new OGL::VtxColorMaterial();
		break;
	case EFFECT_UI:
		mat = new OGL::UIMaterial();
		break;
	case EFFECT_PLANETRING:
		mat = new OGL::RingMaterial();
		break;
	case EFFECT_STARFIELD:
		mat = new OGL::StarfieldMaterial();
		break;
	case EFFECT_GEOSPHERE_TERRAIN:
	case EFFECT_GEOSPHERE_TERRAIN_WITH_LAVA:
	case EFFECT_GEOSPHERE_TERRAIN_WITH_WATER:
		mat = new OGL::GeoSphereSurfaceMaterial();
		break;
	case EFFECT_GEOSPHERE_SKY:
		mat = new OGL::GeoSphereSkyMaterial();
		break;
	case EFFECT_GEOSPHERE_STAR:
		mat = new OGL::GeoSphereStarMaterial();
		break;
	case EFFECT_FRESNEL_SPHERE:
		mat = new OGL::FresnelColourMaterial();
		break;
	case EFFECT_SHIELD:
		mat = new OGL::ShieldMaterial();
		break;
	case EFFECT_SKYBOX:
		mat = new OGL::SkyboxMaterial();
		break;
	case EFFECT_SPHEREIMPOSTOR:
		mat = new OGL::SphereImpostorMaterial();
		break;
	case EFFECT_GASSPHERE_TERRAIN:
		mat = new OGL::GasGiantSurfaceMaterial();
		break;
	case EFFECT_GEN_GASGIANT_TEXTURE:
		mat = new OGL::GenGasGiantColourMaterial();
		break;
	case EFFECT_BILLBOARD_ATLAS:
	case EFFECT_BILLBOARD:
		mat = new OGL::BillboardMaterial();
		break;
	default:
		if (desc.lighting)
			mat = new OGL::LitMultiMaterial();
		else
			mat = new OGL::MultiMaterial();
	}

	mat->m_renderer = this;
	mat->m_descriptor = desc;

	p = GetOrCreateProgram(mat); // XXX throws ShaderException on compile/link failure

	mat->SetProgram(p);
	CheckRenderErrors(__FUNCTION__,__LINE__);
	return mat;
}

bool RendererOGL::ReloadShaders()
{
	Output("Reloading " SIZET_FMT " programs...\n", m_programs.size());
	for (ProgramIterator it = m_programs.begin(); it != m_programs.end(); ++it) {
		it->second->Reload();
	}
	Output("Done.\n");

	return true;
}

OGL::Program* RendererOGL::GetOrCreateProgram(OGL::Material *mat)
{
	PROFILE_SCOPED()
	const MaterialDescriptor &desc = mat->GetDescriptor();
	OGL::Program *p = 0;

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
	CheckRenderErrors(__FUNCTION__,__LINE__);

	return p;
}

Texture *RendererOGL::CreateTexture(const TextureDescriptor &descriptor)
{
	PROFILE_SCOPED()
	return new TextureGL(descriptor, m_useCompressedTextures, m_useAnisotropicFiltering);
}

RenderState *RendererOGL::CreateRenderState(const RenderStateDesc &desc)
{
	PROFILE_SCOPED()
	const uint32_t hash = lookup3_hashlittle(&desc, sizeof(RenderStateDesc), 0);
	auto it = m_renderStates.find(hash);
	if (it != m_renderStates.end()) {
		CheckRenderErrors(__FUNCTION__,__LINE__);
		return it->second;
	} else {
		auto *rs = new OGL::RenderState(desc);
		m_renderStates[hash] = rs;
		CheckRenderErrors(__FUNCTION__,__LINE__);
		return rs;
	}
}

RenderTarget *RendererOGL::CreateRenderTarget(const RenderTargetDesc &desc)
{
	PROFILE_SCOPED()
	OGL::RenderTarget* rt = new OGL::RenderTarget(desc);
	CheckRenderErrors(__FUNCTION__,__LINE__);
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
			0, Graphics::TEXTURE_2D);
		TextureGL *colorTex = new TextureGL(cdesc, false, false);
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
				0, Graphics::TEXTURE_2D);
			TextureGL *depthTex = new TextureGL(ddesc, false, false);
			rt->SetDepthTexture(depthTex);
		} else {
			rt->CreateDepthRenderbuffer();
		}
	}
	rt->CheckStatus();
	rt->Unbind();
	CheckRenderErrors(__FUNCTION__,__LINE__);
	return rt;
}

VertexBuffer *RendererOGL::CreateVertexBuffer(const VertexBufferDesc &desc)
{
	m_stats.AddToStatCount(Stats::STAT_CREATE_BUFFER, 1);
	return new OGL::VertexBuffer(desc);
}

IndexBuffer *RendererOGL::CreateIndexBuffer(Uint32 size, BufferUsage usage)
{
	m_stats.AddToStatCount(Stats::STAT_CREATE_BUFFER, 1);
	return new OGL::IndexBuffer(size, usage);
}

InstanceBuffer *RendererOGL::CreateInstanceBuffer(Uint32 size, BufferUsage usage)
{
	m_stats.AddToStatCount(Stats::STAT_CREATE_BUFFER, 1);
	return new OGL::InstanceBuffer(size, usage);
}

// XXX very heavy. in the future when all GL calls are made through the
// renderer, we can probably do better by trackingn current state and
// only restoring the things that have changed
void RendererOGL::PushState()
{
	SetMatrixMode(MatrixMode::PROJECTION);
	PushMatrix();
	SetMatrixMode(MatrixMode::MODELVIEW);
	PushMatrix();
	m_viewportStack.push( m_viewportStack.top() );
}

void RendererOGL::PopState()
{
	m_viewportStack.pop();
	assert(!m_viewportStack.empty());
	const Viewport& cvp = m_viewportStack.top();
	SetViewport(cvp.x, cvp.y, cvp.w, cvp.h);

	SetMatrixMode(MatrixMode::PROJECTION);
	PopMatrix();
	SetMatrixMode(MatrixMode::MODELVIEW);
	PopMatrix();
}

void RendererOGL::SetMatrixMode(MatrixMode mm)
{
	if( mm != m_matrixMode ) {
		m_matrixMode = mm;
	}
}

void RendererOGL::PushMatrix()
{
	switch(m_matrixMode) {
		case MatrixMode::MODELVIEW:
			m_modelViewStack.push(m_modelViewStack.top());
			break;
		case MatrixMode::PROJECTION:
			m_projectionStack.push(m_projectionStack.top());
			break;
	}
}

void RendererOGL::PopMatrix()
{
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

void RendererOGL::LoadIdentity()
{
	switch(m_matrixMode) {
		case MatrixMode::MODELVIEW:
			m_modelViewStack.top() = matrix4x4f::Identity();
			break;
		case MatrixMode::PROJECTION:
			m_projectionStack.top() = matrix4x4f::Identity();
			break;
	}
}

void RendererOGL::LoadMatrix(const matrix4x4f &m)
{
	switch(m_matrixMode) {
		case MatrixMode::MODELVIEW:
			m_modelViewStack.top() = m;
			break;
		case MatrixMode::PROJECTION:
			m_projectionStack.top() = m;
			break;
	}
}

void RendererOGL::Translate( const float x, const float y, const float z )
{
	switch(m_matrixMode) {
		case MatrixMode::MODELVIEW:
			m_modelViewStack.top().Translate(x,y,z);
			break;
		case MatrixMode::PROJECTION:
			m_projectionStack.top().Translate(x,y,z);
			break;
	}
}

void RendererOGL::Scale( const float x, const float y, const float z )
{
	switch(m_matrixMode) {
		case MatrixMode::MODELVIEW:
			m_modelViewStack.top().Scale(x,y,z);
			break;
		case MatrixMode::PROJECTION:
			m_modelViewStack.top().Scale(x,y,z);
			break;
	}
}

bool RendererOGL::Screendump(ScreendumpState &sd)
{
	sd.width = GetWindow()->GetWidth();
	sd.height = GetWindow()->GetHeight();
	sd.bpp = 3; // XXX get from window

	// pad rows to 4 bytes, which is the default row alignment for OpenGL
	sd.stride = (3*sd.width + 3) & ~3;

	sd.pixels.reset(new Uint8[sd.stride * sd.height]);

	gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
	gl::PixelStorei(gl::PACK_ALIGNMENT, 4); // never trust defaults
	gl::ReadBuffer(gl::FRONT);
	gl::ReadPixels(0, 0, sd.width, sd.height, gl::RGB, gl::UNSIGNED_BYTE, sd.pixels.get());
	gl::Finish();

	return true;
}

}
