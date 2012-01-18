#include "Renderer.h"

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
