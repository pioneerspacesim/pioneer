#include "Shader.h"
#include "MaterialGL2.h"
#include "Graphics.h"
#include "RendererGL2.h"
#include "RendererGLBuffers.h"
#include "Texture.h"
#include "TextureGL.h"
#include "VertexArray.h"
#include <sstream>

namespace Graphics {

Shader *simpleTextured;
Shader *flatProg;
Shader *flatTextured;

static std::vector<std::pair<MaterialDescriptor, Graphics::Shader *> > progMap;
typedef std::vector<std::pair<MaterialDescriptor, Graphics::Shader *> >::const_iterator ProgramMapIterator;

RendererGL2::RendererGL2(int w, int h) :
	RendererLegacy(w, h)
{
	//the range is very large due to a "logarithmic z-buffer" trick used
	//http://outerra.blogspot.com/2009/08/logarithmic-z-buffer.html
	//http://www.gamedev.net/blog/73/entry-2006307-tip-of-the-day-logarithmic-zbuffer-artifacts-fix/
	m_minZNear = 0.0001f;
	m_maxZFar = 10000000.0f;
	simpleTextured = new Shader("simpleTextured");
	flatProg = new Shader("flat");
	flatTextured = new Shader("flat", "#define TEXTURE0 1\n");
}

RendererGL2::~RendererGL2()
{
	delete simpleTextured;
	delete flatProg;
	delete flatTextured;
}

bool RendererGL2::BeginFrame()
{
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return true;
}

bool RendererGL2::SetPerspectiveProjection(float fov, float aspect, float near, float far)
{
	double ymax = near * tan(fov * M_PI / 360.0);
	double ymin = -ymax;
	double xmin = ymin * aspect;
	double xmax = ymax * aspect;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(xmin, xmax, ymin, ymax, near, far);

	// update values for log-z hack
	Graphics::State::SetZnearZfar(near, far);
	return true;
}

bool RendererGL2::DrawLines(int count, const vector3f *v, const Color *c, LineType t)
{
	if (count < 2 || !v) return false;

	simpleShader->Use();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), v);
	glColorPointer(4, GL_FLOAT, sizeof(Color), c);
	glDrawArrays(t, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	simpleShader->Unuse();

	return true;
}

bool RendererGL2::DrawLines(int count, const vector3f *v, const Color &c, LineType t)
{
	if (count < 2 || !v) return false;

	flatProg->Use();
	flatProg->SetUniform("color", c);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), v);
	glDrawArrays(t, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	flatProg->Unuse();

	return true;
}

Material *RendererGL2::CreateMaterial(const MaterialDescriptor &desc)
{
	MaterialGL2 *mat = new MaterialGL2;
	mat->newStyleHack = true;

	Shader *s = 0;
	for(ProgramMapIterator it = progMap.begin();
		it != progMap.end();
		++it)
	{
		if ((*it).first == desc) {
			s = (*it).second;
			break;
		}
	}
	if (!s) { //new
		std::stringstream ss;
		ss << "#define TEXTURE0 1" << std::endl;
		if (desc.specularMap)
			ss << "#define MAP_SPECULAR 1" << std::endl;
		if (desc.glowMap)
			ss << "#define MAP_EMISSIVE 1" << std::endl;
		if (desc.usePatterns)
			ss << "#define MAP_COLOR 1" << std::endl;
		s = new Shader("gl2/nm", ss.str().c_str());
		progMap.push_back(std::make_pair<MaterialDescriptor, Shader *>(desc, s));
	}
	mat->shader = s;
	mat->descriptor = desc;
	return mat;
}

void RendererGL2::ApplyMaterial(const Material *mat)
{
	if (!mat) {
		simpleShader->Use();
		return;
	}
	//XXX hack, obviously
	else if (mat->newStyleHack) {
		//XXX const will not make sense
		static_cast<const MaterialGL2*>(mat)->Apply();
		if (mat->blend)
			SetBlendMode(BLEND_ALPHA);
		return;
	}
	glPushAttrib(GL_ENABLE_BIT);

	const bool flat = !mat->vertexColors;

	//choose shader
	Shader *s = 0;
	if (mat->shader) {
		s = mat->shader;
	} else {
		if (flat && mat->texture0) s = flatTextured;
		else if (flat && !mat->texture0) s = flatProg;
		else if (!flat && mat->texture0) s = simpleTextured;
		else s = simpleShader;
	}

	assert(s);
	s->Use();

	//set parameters
	if (flat)
		s->SetUniform("color", mat->diffuse);
	if (mat->unlit) {
		//nothing to do right now
	} else {
		//specular etc properties
	}
	if (mat->twoSided) {
		glDisable(GL_CULL_FACE);
	}
	if (mat->texture0) {
		static_cast<TextureGL*>(mat->texture0)->Bind();
		s->SetUniform("texture0", 0);
	}
	if (mat->blend) {
		SetBlendMode(BLEND_ALPHA);
	}
}

void RendererGL2::UnApplyMaterial(const Material *mat)
{
	if (mat) {
		//XXX hack, obviously
		if (mat->newStyleHack) {
			//XXX const will not make sense
			static_cast<const MaterialGL2*>(mat)->Unapply();
			return;
		}
		if (mat->texture0) {
			static_cast<TextureGL*>(mat->texture0)->Unbind();
		}
	}
	glPopAttrib();
	// XXX won't be necessary
	m_currentShader = 0;
	glUseProgram(0);
}

}
