#include "Shader.h"
#include "Material.h"
#include "Graphics.h"
#include "RendererGL2.h"
#include "RendererGLBuffers.h"
#include "Texture.h"
#include "VertexArray.h"

namespace Graphics {

Shader *simpleTextured;
Shader *flatProg;
Shader *flatTextured;

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
	State::SetZnearZfar(near, far);
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

void RendererGL2::ApplyMaterial(const Material *mat)
{
	glPushAttrib(GL_ENABLE_BIT);

	if (!mat) {
		simpleShader->Use();
		return;
	}

	const bool flat = !mat->vertexColors;

	//choose shader
	Shader *s = 0;
	if (mat->shader) {
		s = mat->shader;
	} else {
		if (flat && mat->texture0) s = flatTextured;
		else if (flat) s = flatProg;
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
		mat->texture0->Bind();
		s->SetUniform("texture0", 0);
	}
}

void RendererGL2::UnApplyMaterial(const Material *mat)
{
	glPopAttrib();
	if (mat) {
		if (mat->texture0) {
			mat->texture0->Unbind();
		}
	}
	// XXX won't be necessary
	m_currentShader = 0;
	glUseProgram(0);
}

}
