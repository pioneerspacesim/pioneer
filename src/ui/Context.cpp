#include "Context.h"
#include "FileSystem.h"
#include "text/FontDescriptor.h"

namespace UI {

Context::Context(Graphics::Renderer *renderer) :
	m_renderer(renderer),
	m_skin("textures/widgets.png", renderer),
	m_font(RefCountedPtr<Text::TextureFont>(new Text::TextureFont(Text::FontDescriptor("TitilliumText22L004.otf", 14, 14, false, -1.0f), renderer)))
{
}

}
