#include "Button.h"

namespace UI {

// XXX STYLE
static const float buttonSize = 32.0f;

Metrics Button::GetMetrics(const vector2f &hint)
{
	Metrics metrics = Single::GetMetrics(hint - vector2f(buttonSize));

	metrics.minimum += vector2f(buttonSize);
	metrics.ideal += vector2f(buttonSize);
	metrics.maximum += vector2f(buttonSize);

	return metrics;
}

void Button::Draw()
{
	// XXX STYLE
	
	vector2f drawSize(buttonSize);
	if (GetInnerWidget()) drawSize += GetInnerWidget()->GetSize();

	GLfloat array[4*2] = {
		0,          drawSize.y,
		drawSize.x, drawSize.y,
		drawSize.x, 0,
		0,          0
	};

	glColor4f(0.8f,0.8f,0.3f,1.0f);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof(GLfloat)*2, &array[0]);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);

	Container::Draw();
}

}
