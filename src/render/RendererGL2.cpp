#include "Material.h"
#include "Render.h"
#include "RendererGL2.h"
#include "RendererGLBuffers.h"
#include "Texture.h"
#include "VertexArray.h"

Render::Shader *simpleTextured;
Render::Shader *flatProg;
Render::Shader *flatTextured;

RendererGL2::RendererGL2(int w, int h) :
	RendererLegacy(w, h)
{
	simpleTextured = new Render::Shader("simpleTextured");
	flatProg = new Render::Shader("flat");
	flatTextured = new Render::Shader("flat", "#define TEXTURE0 1\n");
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
	Render::State::SetZnearZfar(near, far);
	return true;
}

bool RendererGL2::DrawLines(int count, const LineVertex *v, LineType t)
{
	if (count < 2 || !v) return false;

	Render::State::UseProgram(Render::simpleShader);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(LineVertex), &v[0].position);
	glColorPointer(4, GL_FLOAT, sizeof(LineVertex), &v[0].color);
	glDrawArrays(t, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	Render::State::UseProgram(0);

	return true;
}

bool RendererGL2::DrawLines(int count, const vector3f *v, const Color &c, LineType t)
{
	if (count < 2 || !v) return false;

	Render::State::UseProgram(flatProg);
	flatProg->SetUniform("color", c);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), v);
	glDrawArrays(t, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	Render::State::UseProgram(0);

	return true;
}

void RendererGL2::ApplyMaterial(const Material *mat)
{
	glPushAttrib(GL_ENABLE_BIT);

	if (!mat) {
		Render::State::UseProgram(Render::simpleShader);
		return;
	}

	const bool flat = !mat->vertexColors;

	//choose shader
	Render::Shader *s = 0;
	if (mat->shader) {
		s = mat->shader;
	} else {
		if (flat && mat->texture0) s = flatTextured;
		else if (flat) s = flatProg;
		else s = Render::simpleShader;
	}

	assert(s);
	Render::State::UseProgram(s);

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
	Render::State::UseProgram(0);
}