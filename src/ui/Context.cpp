#include "Context.h"
#include "FileSystem.h"
#include "text/FontDescriptor.h"

namespace UI {

Context::Context(Graphics::Renderer *renderer, int width, int height) : Single(this),
	m_renderer(renderer),
	m_width(float(width)),
	m_height(float(height)),
	m_eventDispatcher(this),
	m_skin("textures/widgets.png", renderer),
	m_font(RefCountedPtr<Text::TextureFont>(new Text::TextureFont(Text::FontDescriptor("TitilliumText22L004.otf", 14, 14, false, -1.0f), renderer)))
{
	SetSize(vector2f(m_width,m_height));
}

void Context::SetScissor(bool enabled, const vector2f &pos, const vector2f &size)
{
	if (enabled) {
		vector2f flippedPos(pos.x, m_height-pos.y-size.y);
		m_renderer->SetScissor(true, flippedPos, size);
	}
	else
		m_renderer->SetScissor(false);

}

}
