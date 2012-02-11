#include "Material.h"
#include "Render.h"
#include "RendererGL2.h"
#include "RendererGLBuffers.h"
#include "Texture.h"
#include "VertexArray.h"

template<> void Buffer<LineVertex>::Draw(GLenum pt, int start, int count) {
	glVertexPointer(3, GL_FLOAT, sizeof(LineVertex), reinterpret_cast<const GLvoid *>(offsetof(LineVertex, position)));
	glColorPointer(4, GL_FLOAT, sizeof(LineVertex), reinterpret_cast<const GLvoid *>(offsetof(LineVertex, color)));
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDrawArrays(pt, start, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

Render::Shader *simpleTextured;
Render::Shader *flatProg;
Buffer<LineVertex> *lineBuffer;

RendererGL2::RendererGL2(int w, int h) :
	RendererLegacy(w, h)
{
	simpleTextured = new Render::Shader("simpleTextured");
	flatProg = new Render::Shader("flat");
	lineBuffer = new Buffer<LineVertex>(1000);
}

RendererGL2::~RendererGL2()
{
	delete simpleTextured;
	delete flatProg;
	delete lineBuffer;
}

bool RendererGL2::BeginFrame()
{
	lineBuffer->Reset();
	Render::PrepareFrame();
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
	lineBuffer->Bind();
	//since it's drawn immediately, there isn't much point in having the offset
	int offset = lineBuffer->BufferData(count, v);
	lineBuffer->Draw(t, offset, count);
	lineBuffer->Unbind();
	Render::State::UseProgram(0);

	return true;
}

bool RendererGL2::DrawLines(int count, const vector3f *v, const Color &c, LineType t)
{
	if (count < 2 || !v) return false;

	Render::State::UseProgram(flatProg);
	flatProg->SetUniform4f("color", c);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vector3f), v);
	glDrawArrays(t, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	Render::State::UseProgram(0);

	return true;
}

// XXX copypaste from legacy renderer
bool RendererGL2::DrawTriangles(const VertexArray *v, const Material *m, PrimitiveType t)
{
	if (!v || v->GetNumVerts() < 3) return false;

	// if no material, per vertex colours
	// flat, if no per-vertex colours supplied
	// unlit, if unlit
	// textured, it texture0 set
	// two sided, if two sided

	ApplyMaterial(m);
	EnableClientStates(v);

	glDrawArrays(t, 0, v->GetNumVerts());

	UnApplyMaterial(m);
	DisableClientStates();

	return true;
}

void RendererGL2::ApplyMaterial(const Material *mat)
{
	glPushAttrib(GL_ENABLE_BIT);

	//choose shader
	Render::Shader *s = 0;
	if (!mat) {
		s = Render::simpleShader;
	} else {
		if (mat->shader)
			s = mat->shader;
		else
			s = Render::simpleShader;
	}
	assert(s);
	Render::State::UseProgram(s);

	/*if (flat) {
		assert(diffuse == false);
		s->SetUniform4f("color", m->diffuse);
	}*/
	if (mat) {
		if (mat->unlit) {

		} else {
			//glMaterialfv (GL_FRONT, GL_DIFFUSE, &mat->diffuse[0]);
			//todo: the rest
		}
		if (mat->twoSided) {
			glDisable(GL_CULL_FACE);
		}
		if (mat->texture0) {
			mat->texture0->Bind();
			s->SetUniform1i("texture0", 0);
		}
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