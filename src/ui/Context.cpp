#include "Context.h"
#include "FileSystem.h"
#include "text/FontDescriptor.h"
#include "LuaObject.h"

namespace UI {

Context::Context(Graphics::Renderer *renderer, int width, int height) : Single(this),
	m_renderer(renderer),
	m_width(float(width)),
	m_height(float(height)),
	m_needsLayout(false),
	m_float(new FloatContainer(this)),
	m_eventDispatcher(this),
	m_skin("textures/widgets.png", renderer)
{
	SetSize(vector2f(m_width,m_height));

	m_float->SetSize(vector2f(m_width,m_height));
	m_float->Attach(this);

	// XXX should do point sizes, but we need display DPI first
	// XXX TextureFont could load multiple sizes into the same object/atlas
	for (int i = FONT_SIZE_XSMALL; i < FONT_SIZE_MAX; i++) {
		int pixelSize = i*3 + 14;
		m_font[i] = RefCountedPtr<Text::TextureFont>(new Text::TextureFont(Text::FontDescriptor("TitilliumText22L004.otf", pixelSize, pixelSize, false, -1.0f), renderer));
	}
}

Context::~Context() {
    m_float->Detach();
}

Widget *Context::GetWidgetAtAbsolute(const vector2f &pos) {
	Widget *w = m_float->GetWidgetAtAbsolute(pos);
	if (!w || w == m_float.Get())
		w = Single::GetWidgetAtAbsolute(pos);
	return w;
}

void Context::Layout()
{
	m_float->Layout();
	Single::Layout();
	m_eventDispatcher.LayoutUpdated();
	m_needsLayout = false;
}

void Context::Update()
{
	if (m_needsLayout)
		Layout();

	m_float->Update();
	Single::Update();
}

void Context::Draw()
{
	Graphics::Renderer *r = GetRenderer();

	r->SetOrthographicProjection(0, m_width, m_height, 0, -1, 1);
	r->SetTransform(matrix4x4f::Identity());
	r->SetClearColor(Color::BLACK);
	r->SetDepthTest(false);

	// XXX GL renderer enables lighting by default. if all draws use materials
	// that's ok, but for filled regions (ie ColorBackground) its not right. a
	// scissored version of Renderer::ClearScreen would be the most efficient,
	// but I'm not quite ready to do it yet.
	glDisable(GL_LIGHTING);

	Single::Draw();
    m_float->Draw();

	DisableScissor();
}

void Context::AddToCatalog(const std::string &name, Widget *widget)
{
	m_catalog.insert(std::make_pair(name, RefCountedPtr<Widget>(widget)));
}

void Context::RemoveFromCatalog(const std::string &name)
{
	CatalogMap::iterator i = m_catalog.find(name);
	if (i != m_catalog.end())
		m_catalog.erase(i);
}

Widget *Context::GetFromCatalog(const std::string &name)
{
	CatalogMap::iterator i = m_catalog.find(name);
	if (i != m_catalog.end())
		return (*i).second.Get();
	return 0;
}

void Context::EnableScissor(const vector2f &pos, const vector2f &size)
{
	vector2f flippedPos(pos.x, m_height-pos.y-floorf(size.y));
	m_renderer->SetScissor(true, flippedPos, size);
}

void Context::DisableScissor()
{
	m_renderer->SetScissor(false);
}

}
