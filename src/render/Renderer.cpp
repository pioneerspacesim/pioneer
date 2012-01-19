#include "Renderer.h"
#include "Render.h"

/* Most of the contents will be moved to RendererLegacy.h/cpp or similar */
Renderer::Renderer(int w, int h) :
	m_width(w), m_height(h)
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

	glClearColor(0,0,0,0);

	glViewport(0, 0, m_width, m_height);
}

bool Renderer::BeginFrame()
{
	Render::PrepareFrame();
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return true;
}

bool Renderer::EndFrame()
{
	Render::PostProcess();
	return true;
}

bool Renderer::SwapBuffers()
{
	glError();
	Render::SwapBuffers();
	return true;
}

bool Renderer::SetBlendMode(unsigned int m)
{
	//where does SRC_ALPHA, ONE fit in?
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

bool Renderer::DrawLines(int count, const LineVertex *v, unsigned int type)
{
	if (count < 2) return false;

	//this is easy to upgrade to GL3/ES2 level
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(LineVertex), &v[0].position);
	glColorPointer(4, GL_FLOAT, sizeof(LineVertex), &v[0].color);
	glDrawArrays(type, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	return true;
}

bool Renderer::DrawLines2D(int count, const LineVertex2D *v, unsigned int type)
{
	if (count < 2) return false;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof(LineVertex2D), &v[0].position);
	glColorPointer(4, GL_FLOAT, sizeof(LineVertex2D), &v[0].color);
	glDrawArrays(type, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	return true;
}

bool Renderer::DrawTriangleStrip(int count, const ColoredVertex *v)
{
	if (count < 3) return false;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(ColoredVertex), &v[0].position);
	glColorPointer(4, GL_FLOAT, sizeof(ColoredVertex), &v[0].color);
	glNormalPointer(GL_FLOAT, sizeof(ColoredVertex), &v[0].normal);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	return true;
}
