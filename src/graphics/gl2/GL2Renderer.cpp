// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GL2Renderer.h"
#include "graphics/Graphics.h"
#include "graphics/Light.h"
#include "graphics/Material.h"
#include "OS.h"
#include "StringF.h"
#include "graphics/Texture.h"
#include "graphics/TextureBuilder.h"
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



namespace Graphics {

static bool CreateWindowAndContext(const char *name, const Graphics::Settings &vs, int samples, int depth_bits, SDL_Window **window, SDL_GLContext *context)
{
	Uint32 winFlags = 0;

	assert(vs.rendererType==Graphics::RendererType::RENDERER_OPENGL_21);

	winFlags |= SDL_WINDOW_OPENGL;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	// cannot initialise 3.x content on OSX with anything but CORE profile
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
	// OSX also forces us to use this for 3.2 onwards
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depth_bits);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, samples ? 1 : 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, samples);

	// need full 32-bit color
	// (need an alpha channel because of the way progress bars are drawn)
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	winFlags |= (vs.hidden ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN);
	if (!vs.hidden && vs.fullscreen)
		winFlags |= SDL_WINDOW_FULLSCREEN;

	(*window) = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, vs.width, vs.height, winFlags);
	if (!(*window))
		return false;

	(*context) = SDL_GL_CreateContext((*window));
	if (!(*context)) {
		SDL_DestroyWindow((*window));
		(*window) = nullptr;
		return false;
	}

	return true;
}

static Renderer *CreateRenderer(const Settings &vs) {
	bool ok;

	const std::string name("Pioneer");
	SDL_Window *window = nullptr;
	SDL_GLContext glContext = nullptr;

	// attempt sequence is:
	// 1- requested mode
	ok = CreateWindowAndContext(name.c_str(), vs, vs.requestedSamples, 24, &window, &glContext);

	// 2- requested mode with no anti-aliasing (skipped if no AA was requested anyway)
	//    (skipped if no AA was requested anyway)
	if (!ok && vs.requestedSamples) {
		Output("Failed to set video mode. (%s). Re-trying without multisampling.\n", SDL_GetError());
		ok = CreateWindowAndContext(name.c_str(), vs, 0, 24, &window, &glContext);
	}

	// 3- requested mode with 16 bit depth buffer
	if (!ok) {
		Output("Failed to set video mode. (%s). Re-trying with 16-bit depth buffer\n", SDL_GetError());
		ok = CreateWindowAndContext(name.c_str(), vs, vs.requestedSamples, 16, &window, &glContext);
	}

	// 4- requested mode with 16-bit depth buffer and no anti-aliasing
	//    (skipped if no AA was requested anyway)
	if (!ok && vs.requestedSamples) {
		Output("Failed to set video mode. (%s). Re-trying with 16-bit depth buffer and no multisampling\n", SDL_GetError());
		ok = CreateWindowAndContext(name.c_str(), vs, 0, 16, &window, &glContext);
	}

	// 5- abort!
	if (!ok) {
		Warning("Failed to set video mode: %s", SDL_GetError());
		return nullptr;
	}

	SDLSurfacePtr surface = LoadSurfaceFromFile(vs.iconFile);
	if (surface)
		SDL_SetWindowIcon(window, surface.Get());

	SDL_SetWindowTitle(window, vs.title);
	SDL_ShowCursor(0);

	SDL_GL_SetSwapInterval((vs.vsync!=0) ? 1 : 0);

	return new RendererGL2(window, vs, glContext);
}

void RendererGL2::RegisterRenderer() {
	Graphics::RegisterRenderer(Graphics::RENDERER_OPENGL_21, CreateRenderer);
}

typedef std::vector<std::pair<MaterialDescriptor, GL2::Program*> >::const_iterator ProgramIterator;

bool RendererGL2::initted = false;
RendererGL2::AttribBufferMap RendererGL2::s_AttribBufferMap;

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
	default:
		return stringf("Unknown error 0x0%0{x}", err);
	}
}

void RendererGL2::CheckErrors(const char *func /*= nullptr*/, const int line /*= nullptr*/)
{
	PROFILE_SCOPED()
#ifndef PIONEER_PROFILER
	GLenum err = glGetError();
	if( err ) {
		// static-cache current err that sparked this
		static GLenum s_prevErr = GL_NO_ERROR;
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
		while (err != GL_NO_ERROR) {
			ss << glerr_to_string(err) << '\n';
			if( err == GL_OUT_OF_MEMORY ) {
				ss << "Out-of-memory on graphics card." << std::endl
					<< "Recommend enabling \"Compress Textures\" in game options." << std::endl
					<< "Also try reducing City and Planet detail settings." << std::endl;
			}
#ifdef _WIN32
			else if (err == GL_INVALID_OPERATION) {
				ss << "Invalid operations can occur if you are using overlay software." << std::endl
					<< "Such as FRAPS, RivaTuner, MSI Afterburner etc." << std::endl
					<< "Please try disabling this kind of software and testing again, thankyou." << std::endl;
			}
#endif
			err = glGetError();
		}
		// show warning dialog or just log to output
		if(showWarning)
			Warning("%s", ss.str().c_str());
		else
			Output("%s", ss.str().c_str());
	}
#endif
}

RendererGL2::RendererGL2(SDL_Window *window, const Graphics::Settings &vs, SDL_GLContext &glContext)
: Renderer(window, vs.width, vs.height)
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
, m_glContext(glContext)
{
	GLenum glew_err;
	if ((glew_err = glewInit()) != GLEW_OK)
		Error("GLEW initialisation failed: %s", glewGetErrorString(glew_err));

	// pump this once as glewExperimental is necessary but spews a single error
	glGetError();

	if (!glewIsSupported("GL_VERSION_2_1") )
	{
		Error(
			"Pioneer can not run on your graphics card as it does not appear to support OpenGL 2.1\n"
			"Please check to see if your GPU driver vendor has an updated driver - or that drivers are installed correctly."
		);
	}
	m_viewportStack.push(Viewport());

	const bool useDXTnTextures = vs.useTextureCompression;
	m_useCompressedTextures = useDXTnTextures;

	//XXX bunch of fixed function states here!
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHT0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glMatrixMode(GL_MODELVIEW);
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

	SDL_GL_DeleteContext(m_glContext);
}

static const char *gl_error_to_string(GLenum err)
{
	switch (err) {
	case GL_NO_ERROR: return "(no error)";
	case GL_INVALID_ENUM: return "invalid enum";
	case GL_INVALID_VALUE: return "invalid value";
	case GL_INVALID_OPERATION: return "invalid operation";
	//case GL_INVALID_FRAMEBUFFER_OPERATION: return "invalid framebuffer operation";
	case GL_OUT_OF_MEMORY: return "out of memory";
	default: return "(unknown error)";
	}
}

static void dump_and_clear_opengl_errors(std::ostream &out, GLenum first_error = GL_NO_ERROR)
{
	GLenum err = ((first_error == GL_NO_ERROR) ? glGetError() : first_error);
	if (err != GL_NO_ERROR) {
		out << "errors: ";
		do {
			out << gl_error_to_string(err) << " ";
			err = glGetError();
		} while (err != GL_NO_ERROR);
		out << std::endl;
	}
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
	}
	else {
		while (err != GL_NO_ERROR) {
			if (err == GL_INVALID_ENUM) { out << name << " -- not supported\n"; }
			else { out << name << " -- unexpected error (" << err << ") retrieving value\n"; }
			err = glGetError();
		}
	}
}

void RendererGL2::WriteRendererInfo(std::ostream &out) const
{
	out << "OpenGL version " << glGetString(GL_VERSION);
	out << ", running on " << glGetString(GL_VENDOR);
	out << " " << glGetString(GL_RENDERER) << "\n";

	out << "Shading language version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

	out << "\nImplementation Limits:\n";

	// first, clear all OpenGL error flags
	dump_and_clear_opengl_errors(out);

#define DUMP_GL_VALUE(name) dump_opengl_value(out, #name, name, 1)
#define DUMP_GL_VALUE2(name) dump_opengl_value(out, #name, name, 2)

	DUMP_GL_VALUE(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
	DUMP_GL_VALUE(GL_MAX_CUBE_MAP_TEXTURE_SIZE);
	DUMP_GL_VALUE(GL_MAX_DRAW_BUFFERS);
	DUMP_GL_VALUE(GL_MAX_ELEMENTS_INDICES);
	DUMP_GL_VALUE(GL_MAX_ELEMENTS_VERTICES);
	DUMP_GL_VALUE(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_IMAGE_UNITS);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_LOD_BIAS);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_SIZE);
	DUMP_GL_VALUE(GL_MAX_VERTEX_ATTRIBS);
	DUMP_GL_VALUE(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
	DUMP_GL_VALUE(GL_MAX_VERTEX_UNIFORM_COMPONENTS);
	DUMP_GL_VALUE(GL_NUM_COMPRESSED_TEXTURE_FORMATS);
	DUMP_GL_VALUE(GL_SAMPLE_BUFFERS);
	DUMP_GL_VALUE(GL_SAMPLES);
	DUMP_GL_VALUE2(GL_ALIASED_LINE_WIDTH_RANGE);
	DUMP_GL_VALUE2(GL_MAX_VIEWPORT_DIMS);
	DUMP_GL_VALUE2(GL_SMOOTH_LINE_WIDTH_RANGE);
	DUMP_GL_VALUE2(GL_SMOOTH_POINT_SIZE_RANGE);

#undef DUMP_GL_VALUE
#undef DUMP_GL_VALUE2

	// enumerate compressed texture formats
	{
		dump_and_clear_opengl_errors(out);
		out << "\nCompressed texture formats:\n";

		GLint nformats;
		GLint formats[128]; // XXX 128 should be enough, right?

		glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &nformats);
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			out << "Get NUM_COMPRESSED_TEXTURE_FORMATS failed\n";
			dump_and_clear_opengl_errors(out, err);
		}
		else {
			assert(nformats >= 0 && nformats < int(COUNTOF(formats)));
			glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, formats);
			err = glGetError();
			if (err != GL_NO_ERROR) {
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
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return true;
}

bool RendererGL2::EndFrame()
{
	return true;
}

bool RendererGL2::SwapBuffers()
{
	PROFILE_SCOPED()
	CheckRenderErrors(__FUNCTION__,__LINE__);

	SDL_GL_SwapWindow(m_window);
	return true;
}

bool RendererGL2::SetRenderState(RenderState *rs)
{
	if (m_activeRenderState != rs) {
		static_cast<GL2::RenderState*>(rs)->Apply();
		m_activeRenderState = rs;
	}
	CheckRenderErrors(__FUNCTION__,__LINE__);
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
	CheckRenderErrors(__FUNCTION__,__LINE__);
	return true;
}

bool RendererGL2::SetDepthRange(double near_, double far_)
{
	glDepthRange(near_, far_);
	return true;
}

bool RendererGL2::ClearScreen()
{
	m_activeRenderState = nullptr;
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return true;
}

bool RendererGL2::ClearDepthBuffer()
{
	m_activeRenderState = nullptr;
	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT);

	return true;
}

bool RendererGL2::SetClearColor(const Color &c)
{
	glClearColor(c.r, c.g, c.b, c.a);
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
	glViewport(x, y, width, height);
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
	glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
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
		glScissor(pos.x,pos.y,size.x,size.y);
		glEnable(GL_SCISSOR_TEST);
	}
	else
		glDisable(GL_SCISSOR_TEST);
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

bool RendererGL2::DrawPointSprites(const Uint32 count, const vector3f *positions, RenderState *rs, Material *material, float size)
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

bool RendererGL2::DrawPointSprites(const Uint32 count, const vector3f *positions, const vector2f *offsets, const float *sizes, RenderState *rs, Material *material)
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

bool RendererGL2::DrawBuffer(VertexBuffer* vb, RenderState* state, Material* mat, PrimitiveType pt)
{
	PROFILE_SCOPED()
	SetRenderState(state);
	mat->Apply();

	SetMaterialShaderTransforms(mat);

	auto gvb = static_cast<GL2::VertexBuffer*>(vb);

	gvb->Bind();
	EnableVertexAttributes(gvb);

	glDrawArrays(pt, 0, gvb->GetVertexCount());

	DisableVertexAttributes(gvb);
	gvb->Release();

	CheckRenderErrors(__FUNCTION__,__LINE__);
	m_stats.AddToStatCount(Stats::STAT_DRAWCALL, 1);

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
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gib->GetBuffer());
	EnableVertexAttributes(gvb);

	glDrawElements(pt, ib->GetIndexCount(), GL_UNSIGNED_INT, 0);

	DisableVertexAttributes(gvb);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	gvb->Release();

	CheckRenderErrors(__FUNCTION__,__LINE__);
	m_stats.AddToStatCount(Stats::STAT_DRAWCALL, 1);

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
	glDrawArraysInstancedARB(pt, 0, vb->GetVertexCount(), instb->GetInstanceCount());
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
	glDrawElementsInstancedARB(pt, ib->GetIndexCount(), GL_UNSIGNED_INT, 0, instb->GetInstanceCount());
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
		case ATTRIB_POSITION:		glEnableVertexAttribArray(0);		break;
		case ATTRIB_NORMAL:			glEnableVertexAttribArray(1);		break;
		case ATTRIB_DIFFUSE:		glEnableVertexAttribArray(2);		break;
		case ATTRIB_UV0:			glEnableVertexAttribArray(3);		break;
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
		case ATTRIB_POSITION:		glDisableVertexAttribArray(0);			break;
		case ATTRIB_NORMAL:			glDisableVertexAttribArray(1);			break;
		case ATTRIB_DIFFUSE:		glDisableVertexAttribArray(2);			break;
		case ATTRIB_UV0:			glDisableVertexAttribArray(3);			break;
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
	glEnableVertexAttribArray(0);	// Enable the attribute at that location
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLvoid *>(&v->position[0]));

	if (v->HasAttrib(ATTRIB_NORMAL)) {
		assert(! v->normal.empty());
		m_vertexAttribsSet.push_back(1);
		glEnableVertexAttribArray(1);	// Enable the attribute at that location
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLvoid *>(&v->normal[0]));
	}
	if (v->HasAttrib(ATTRIB_DIFFUSE)) {
		assert(! v->diffuse.empty());
		m_vertexAttribsSet.push_back(2);
		glEnableVertexAttribArray(2);	// Enable the attribute at that location
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, reinterpret_cast<const GLvoid *>(&v->diffuse[0]));	// only normalise the colours
	}
	if (v->HasAttrib(ATTRIB_UV0)) {
		assert(! v->uv0.empty());
		m_vertexAttribsSet.push_back(3);
		glEnableVertexAttribArray(3);	// Enable the attribute at that location
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLvoid *>(&v->uv0[0]));
	}
}

void RendererGL2::DisableVertexAttributes()
{
	PROFILE_SCOPED();

	for (auto i : m_vertexAttribsSet) {
		glDisableVertexAttribArray(i);
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
	return new GL2::GL2Texture(descriptor, m_useCompressedTextures);
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
		GL2::GL2Texture *colorTex = new GL2::GL2Texture(cdesc, false);
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
			GL2::GL2Texture *depthTex = new GL2::GL2Texture(ddesc, false);
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
	glPushAttrib(GL_ALL_ATTRIB_BITS & (~GL_POINT_BIT));
}

void RendererGL2::PopState()
{
	glPopAttrib();
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
				glMatrixMode(GL_MODELVIEW);
				break;
			case MatrixMode::PROJECTION:
				glMatrixMode(GL_PROJECTION);
				break;
		}
		m_matrixMode = mm;
	}
}

void RendererGL2::PushMatrix()
{
	PROFILE_SCOPED()

	glPushMatrix();
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
	glPopMatrix();
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
	glLoadIdentity();
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
	glLoadMatrixf(&m[0]);
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
	glTranslatef(x,y,z);
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
	glScalef(x,y,z);
	switch(m_matrixMode) {
		case MatrixMode::MODELVIEW:
			m_modelViewStack.top().Scale(x,y,z);
			break;
		case MatrixMode::PROJECTION:
			m_modelViewStack.top().Scale(x,y,z);
			break;
	}
}

bool RendererGL2::Screendump(ScreendumpState &sd)
{
	int w, h;
	SDL_GetWindowSize(m_window, &w, &h);
	sd.width = w;
	sd.height = h;
	sd.bpp = 4; // XXX get from window

	// pad rows to 4 bytes, which is the default row alignment for OpenGL
	sd.stride = ((sd.bpp * sd.width) + 3) & ~3;

	sd.pixels.reset(new Uint8[sd.stride * sd.height]);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 4); // never trust defaults
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, sd.width, sd.height, GL_RGBA, GL_UNSIGNED_BYTE, sd.pixels.get());
	glFinish();

	return true;
}

bool RendererGL2::FrameGrab(ScreendumpState &sd)
{
	int w, h;
	SDL_GetWindowSize(m_window, &w, &h);
	sd.width = w;
	sd.height = h;
	sd.bpp = 4; // XXX get from window

	sd.stride = (4 * sd.width);

	sd.pixels.reset(new Uint8[sd.stride * sd.height]);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 4); // never trust defaults
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, sd.width, sd.height, GL_RGBA, GL_UNSIGNED_BYTE, sd.pixels.get());
	glFinish();

	return true;
}

}
