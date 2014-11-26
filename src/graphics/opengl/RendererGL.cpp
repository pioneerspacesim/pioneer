// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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

#include <stddef.h> //for offsetof
#include <ostream>
#include <sstream>
#include <iterator>

namespace Graphics {

typedef std::vector<std::pair<MaterialDescriptor, OGL::Program*> >::const_iterator ProgramIterator;

RendererOGL::RendererOGL(WindowSDL *window, const Graphics::Settings &vs)
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
	m_viewportStack.push(Viewport());

	const bool useDXTnTextures = vs.useTextureCompression;
	m_useCompressedTextures = useDXTnTextures;

	//XXX bunch of fixed function states here!
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	SetMatrixMode(MatrixMode::MODELVIEW);

	m_modelViewStack.push(matrix4x4f::Identity());
	m_projectionStack.push(matrix4x4f::Identity());

	SetClearColor(Color4f(0.f, 0.f, 0.f, 0.f));
	SetViewport(0, 0, m_width, m_height);

	if (vs.enableDebugMessages)
		GLDebug::Enable();
}

RendererOGL::~RendererOGL()
{
	// HACK ANDYC - this crashes when shutting down? They'll be released anyway right?
	//while (!m_programs.empty()) delete m_programs.back().second, m_programs.pop_back();
	for (auto state : m_renderStates)
		delete state.second;
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
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	default:
		return stringf("Unknown error 0x0%0{x}", err);
	}
}

//extern 
void CheckRenderErrors()
{
	GLenum err = glGetError();
	if( err ) {
		std::stringstream ss;
		ss << "OpenGL error(s) during frame:\n";
		while (err != GL_NO_ERROR) {
			ss << glerr_to_string(err) << '\n';
			err = glGetError();
			if( err == GL_OUT_OF_MEMORY ) {
				ss << "Out-of-memory on graphics card." << std::endl
					<< "Recommend enabling \"Compress Textures\" in game options." << std::endl
					<< "Also try reducing City and Planet detail settings." << std::endl;
			}
		}
		Warning("%s", ss.str().c_str());
	}
}

bool RendererOGL::SwapBuffers()
{
	PROFILE_SCOPED()
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
			ss << glerr_to_string(err) << std::endl;
			err = glGetError();
			if( err == GL_OUT_OF_MEMORY ) {
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

bool RendererOGL::SetRenderState(RenderState *rs)
{
	if (m_activeRenderState != rs) {
		static_cast<OGL::RenderState*>(rs)->Apply();
		m_activeRenderState = rs;
	}
	CheckRenderErrors();
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
	CheckRenderErrors();

	return true;
}

bool RendererOGL::ClearScreen()
{
	m_activeRenderState = nullptr;
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CheckRenderErrors();

	return true;
}

bool RendererOGL::ClearDepthBuffer()
{
	m_activeRenderState = nullptr;
	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT);
	CheckRenderErrors();

	return true;
}

bool RendererOGL::SetClearColor(const Color &c)
{
	glClearColor(c.r, c.g, c.b, c.a);
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
	glViewport(x, y, width, height);
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
	m_invLogZfarPlus1 = 1.0f / (log(far_+1.0f)/log(2.0f));

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
	glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
	return true;
}

bool RendererOGL::SetLights(int numlights, const Light *lights)
{
	if (numlights < 1) return false;

	const int NumLights = std::min(numlights, int(TOTAL_NUM_LIGHTS));

	m_numLights = NumLights;
	m_numDirLights = 0;

	for (int i=0; i<NumLights; i++) {
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
		glScissor(pos.x,pos.y,size.x,size.y);
		glEnable(GL_SCISSOR_TEST);
	} else {
		glDisable(GL_SCISSOR_TEST);
	}
	return true;
}

void RendererOGL::SetMaterialShaderTransforms(Material *m)
{
	m->SetCommonUniforms(m_modelViewStack.top(), m_projectionStack.top());
	CheckRenderErrors();
}

bool RendererOGL::DrawLines(int count, const vector3f *v, const Color *c, RenderState* state, PrimitiveType t)
{
	PROFILE_SCOPED()
	Drawables::Lines lines;
	lines.SetData(count, v, c);
	lines.Draw(this, state, t);
	return true;
}

bool RendererOGL::DrawLines(int count, const vector3f *v, const Color &c, RenderState *state, PrimitiveType t)
{
	PROFILE_SCOPED()
	Drawables::Lines lines;
	lines.SetData(count, v, c);
	lines.Draw(this, state, t);
	return true;
}

bool RendererOGL::DrawPoints(int count, const vector3f *points, const Color *colors, Graphics::RenderState *state, float size)
{
	struct TPos {
		vector3f pos;
		Color4ub col;
	};

	MaterialDescriptor md;
	md.vertexColors = true;
	static std::unique_ptr<Material> mat(CreateMaterial(md));
	
	// Create vtx & index buffers and copy data
	VertexBufferDesc vbd;
	vbd.attrib[0].semantic	= ATTRIB_POSITION;
	vbd.attrib[0].format	= ATTRIB_FORMAT_FLOAT3;
	vbd.attrib[1].semantic	= ATTRIB_DIFFUSE;
	vbd.attrib[1].format	= ATTRIB_FORMAT_UBYTE4;
	vbd.numVertices = count;
	vbd.usage = BUFFER_USAGE_STATIC;
	
	// VertexBuffer
	std::unique_ptr<VertexBuffer> vb;
	vb.reset(CreateVertexBuffer(vbd));
	TPos* vtxPtr = vb->Map<TPos>(BUFFER_MAP_WRITE);
	assert(vb->GetDesc().stride == sizeof(TPos));
	for(Sint32 i=0 ; i<count ; i++)
	{
		vtxPtr[i].pos = points[i];
		vtxPtr[i].col = colors[i];
	}
	vb->Unmap();

	return DrawBuffer(vb.get(), state, mat.get(), POINTS);
}

bool RendererOGL::DrawTriangles(const VertexArray *v, RenderState *rs, Material *m, PrimitiveType t)
{
	PROFILE_SCOPED()
	if (!v || v->position.size() < 3) return false;
	CheckRenderErrors();

	VertexBufferDesc vbd;
	Uint32 attribIdx = 0;
	assert(v->HasAttrib(ATTRIB_POSITION));
	vbd.attrib[attribIdx].semantic	= ATTRIB_POSITION;
	vbd.attrib[attribIdx].format	= ATTRIB_FORMAT_FLOAT3;
	++attribIdx;

	if( v->HasAttrib(ATTRIB_NORMAL) ) {
		vbd.attrib[attribIdx].semantic	= ATTRIB_NORMAL;
		vbd.attrib[attribIdx].format	= ATTRIB_FORMAT_FLOAT3;
		++attribIdx;
	}
	if( v->HasAttrib(ATTRIB_DIFFUSE) ) {
		vbd.attrib[attribIdx].semantic	= ATTRIB_DIFFUSE;
		vbd.attrib[attribIdx].format	= ATTRIB_FORMAT_UBYTE4;
		++attribIdx;
	}
	if( v->HasAttrib(ATTRIB_UV0) ) {
		vbd.attrib[attribIdx].semantic	= ATTRIB_UV0;
		vbd.attrib[attribIdx].format	= ATTRIB_FORMAT_FLOAT2;
		++attribIdx;
	}
	vbd.numVertices = v->position.size();
	vbd.usage = BUFFER_USAGE_STATIC;
	
	// VertexBuffer
	std::unique_ptr<VertexBuffer> vb;
	vb.reset(CreateVertexBuffer(vbd));
	vb->Populate(*v);
	CheckRenderErrors();

	const bool res = DrawBuffer(vb.get(), rs, m, t);
	CheckRenderErrors();

	return res;
}

bool RendererOGL::DrawPointSprites(int count, const vector3f *positions, RenderState *rs, Material *material, float size)
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
	//OGL renderer should use actual point sprites
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

	DrawTriangles(&va, rs, material);

	return true;
}

bool RendererOGL::DrawBuffer(VertexBuffer* vb, RenderState* state, Material* mat, PrimitiveType pt)
{
	PROFILE_SCOPED()
	CheckRenderErrors();
	SetRenderState(state);
	mat->Apply();

	SetMaterialShaderTransforms(mat);

	auto gvb = static_cast<OGL::VertexBuffer*>(vb);

	gvb->Bind();
	EnableVertexAttributes(gvb);

	glDrawArrays(pt, 0, gvb->GetVertexCount());

	DisableVertexAttributes(gvb);
	gvb->Release();
	CheckRenderErrors();

	return true;
}

bool RendererOGL::DrawBufferIndexed(VertexBuffer *vb, IndexBuffer *ib, RenderState *state, Material *mat, PrimitiveType pt)
{
	PROFILE_SCOPED()
	CheckRenderErrors();
	SetRenderState(state);
	mat->Apply();

	SetMaterialShaderTransforms(mat);

	auto gvb = static_cast<OGL::VertexBuffer*>(vb);
	auto gib = static_cast<OGL::IndexBuffer*>(ib);

	gvb->Bind();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gib->GetBuffer());
	EnableVertexAttributes(gvb);

	glDrawElements(pt, ib->GetIndexCount(), GL_UNSIGNED_SHORT, 0);

	DisableVertexAttributes(gvb);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	gvb->Release();
	CheckRenderErrors();

	return true;
}

void RendererOGL::EnableVertexAttributes(const VertexBuffer* gvb)
{
	PROFILE_SCOPED()
	CheckRenderErrors();
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
	CheckRenderErrors();
}

void RendererOGL::DisableVertexAttributes(const VertexBuffer* gvb)
{
	PROFILE_SCOPED()
	CheckRenderErrors();
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
	CheckRenderErrors();
}


void RendererOGL::EnableVertexAttributes(const VertexArray *v)
{
	PROFILE_SCOPED();
	CheckRenderErrors();

	if (!v) return;
	assert(v->position.size() > 0); //would be strange

	// XXX could be 3D or 2D
	m_vertexAttribsSet.push_back(0);
	glEnableVertexAttribArray(0);	// Enable the attribute at that location
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLvoid *>(&v->position[0]));
	CheckRenderErrors();

	if (v->HasAttrib(ATTRIB_NORMAL)) {
		assert(! v->normal.empty());
		m_vertexAttribsSet.push_back(1);
		glEnableVertexAttribArray(1);	// Enable the attribute at that location
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLvoid *>(&v->normal[0]));
		CheckRenderErrors();
	}
	if (v->HasAttrib(ATTRIB_DIFFUSE)) {
		assert(! v->diffuse.empty());
		m_vertexAttribsSet.push_back(2);
		glEnableVertexAttribArray(2);	// Enable the attribute at that location
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, reinterpret_cast<const GLvoid *>(&v->diffuse[0]));	// only normalise the colours
		CheckRenderErrors();
	}
	if (v->HasAttrib(ATTRIB_UV0)) {
		assert(! v->uv0.empty());
		m_vertexAttribsSet.push_back(3);
		glEnableVertexAttribArray(3);	// Enable the attribute at that location
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLvoid *>(&v->uv0[0]));
		CheckRenderErrors();
	}
}

void RendererOGL::DisableVertexAttributes()
{
	PROFILE_SCOPED();
	CheckRenderErrors();

	for (auto i : m_vertexAttribsSet) {
		glDisableVertexAttribArray(i);
		CheckRenderErrors();
	}
	m_vertexAttribsSet.clear();
}

Material *RendererOGL::CreateMaterial(const MaterialDescriptor &d)
{
	PROFILE_SCOPED()
	CheckRenderErrors();
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
	CheckRenderErrors();
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
	CheckRenderErrors();
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
	CheckRenderErrors();

	return p;
}

Texture *RendererOGL::CreateTexture(const TextureDescriptor &descriptor)
{
	CheckRenderErrors();
	return new TextureGL(descriptor, m_useCompressedTextures);
}

RenderState *RendererOGL::CreateRenderState(const RenderStateDesc &desc)
{
	CheckRenderErrors();
	const uint32_t hash = lookup3_hashlittle(&desc, sizeof(RenderStateDesc), 0);
	auto it = m_renderStates.find(hash);
	if (it != m_renderStates.end()) {
		CheckRenderErrors();
		return it->second;
	} else {
		auto *rs = new OGL::RenderState(desc);
		m_renderStates[hash] = rs;
		CheckRenderErrors();
		return rs;
	}
}

RenderTarget *RendererOGL::CreateRenderTarget(const RenderTargetDesc &desc)
{
	CheckRenderErrors();
	OGL::RenderTarget* rt = new OGL::RenderTarget(desc);
	rt->Bind();
	if (desc.colorFormat != TEXTURE_NONE) {
		Graphics::TextureDescriptor cdesc(
			desc.colorFormat,
			vector2f(desc.width, desc.height),
			vector2f(desc.width, desc.height),
			LINEAR_CLAMP,
			false,
			false);
		TextureGL *colorTex = new TextureGL(cdesc, false);
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
				false);
			TextureGL *depthTex = new TextureGL(ddesc, false);
			rt->SetDepthTexture(depthTex);
		} else {
			rt->CreateDepthRenderbuffer();
		}
	}
	rt->CheckStatus();
	rt->Unbind();
	CheckRenderErrors();
	return rt;
}

VertexBuffer *RendererOGL::CreateVertexBuffer(const VertexBufferDesc &desc)
{
	return new OGL::VertexBuffer(desc);
}

IndexBuffer *RendererOGL::CreateIndexBuffer(Uint32 size, BufferUsage usage)
{
	return new OGL::IndexBuffer(size, usage);
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
	PROFILE_SCOPED()
	if( mm != m_matrixMode ) {
		m_matrixMode = mm;
	}
}

void RendererOGL::PushMatrix()
{
	PROFILE_SCOPED()
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
	PROFILE_SCOPED()
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
	PROFILE_SCOPED()
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
	PROFILE_SCOPED()
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
	PROFILE_SCOPED()
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
	PROFILE_SCOPED()
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
