#include "Context.h"
#include "FileSystem.h"
#include "text/FontDescriptor.h"
#include "Lua.h"
#include <typeinfo>

namespace UI {

Context::Context(LuaManager *lua, Graphics::Renderer *renderer, int width, int height) : Single(this),
	m_renderer(renderer),
	m_width(width),
	m_height(height),
	m_needsLayout(false),
	m_float(new FloatContainer(this)),
	m_eventDispatcher(this),
	m_skin("textures/widgets.png", renderer),
	m_lua(lua)
{
	lua_State *l = m_lua->GetLuaState();
	lua_newtable(l);
	m_templateStore = LuaRef(l, -1);

	SetSize(Point(m_width,m_height));

	m_float->SetSize(Point(m_width,m_height));
	m_float->Attach(this);

	// XXX should do point sizes, but we need display DPI first
	// XXX TextureFont could load multiple sizes into the same object/atlas
	for (int i = FONT_SIZE_XSMALL; i < FONT_SIZE_MAX; i++) {
		int pixelSize = i*3 + 14;
		m_font[i] = RefCountedPtr<Text::TextureFont>(new Text::TextureFont(Text::FontDescriptor("TitilliumText22L004.otf", pixelSize, pixelSize, false, -1.0f), renderer));
	}

	m_scissorStack.push(std::make_pair(Point(0,0), Point(m_width,m_height)));
}

Context::~Context() {
    m_float->Detach();
}

Widget *Context::GetWidgetAt(const Point &pos)
{
	Widget *w = m_float->GetWidgetAt(pos);
	if (!w || w == m_float.Get())
		w = Single::GetWidgetAt(pos);
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
	r->SetBlendMode(Graphics::BLEND_ALPHA);
	r->SetDepthTest(false);

	// XXX GL renderer enables lighting by default. if all draws use materials
	// that's ok, but for filled regions (ie ColorBackground) its not right. a
	// scissored version of Renderer::ClearScreen would be the most efficient,
	// but I'm not quite ready to do it yet.
	glDisable(GL_LIGHTING);

	Single::Draw();
    m_float->Draw();

	r->SetScissor(false);
}

Widget *Context::CallTemplate(const char *name, const LuaTable &args)
{
	lua_State *l = m_lua->GetLuaState();

	m_templateStore.PushCopyToStack();
	const LuaTable t(l, -1);
	if (!t.Get<bool,const char *>(name))
		return 0;

	t.PushValueToStack<const char*>(name);
	lua_pushvalue(l, args.GetIndex());
	pi_lua_protected_call(m_lua->GetLuaState(), 1, 1);

	return LuaObject<UI::Widget>::CheckFromLua(-1);
}

Widget *Context::CallTemplate(const char *name)
{
	return CallTemplate(name, LuaTable(m_lua->GetLuaState()));
}

#if 0
void Context::PushScissor(const Point &pos, const Point &size)
{
	if (m_scissorStack.size() > 0) {
		const std::pair<Point,Point> &currentScissor = m_scissorStack.top();
		const Point &currentPos = currentScissor.first;
		const Point &currentSize = currentScissor.second;

		//Point newPos = Point(currentPos.x + pos.x, currentPos.y + pos.y); // XXX ensure >= currentPos, clamp to currentSize
		Point newPos(Point(std::min(currentSize.x, pos.x), std::min(currentSize.y, pos.y)) + currentPos);
		//Point newSize = size;                                             // XXX clamp to currentSize
		Point newSize(std::min(currentSize.x, size.x), std::min(currentSize.y, size.y));

		m_scissorStack.push(std::make_pair(newPos, newSize));
	}
	else
		m_scissorStack.push(std::make_pair(pos, size));

	ApplyScissor();
}

void Context::PopScissor()
{
	m_scissorStack.pop();
	if (m_scissorStack.size() > 0)
		ApplyScissor();
	else
		m_renderer->SetScissor(false);
}

void Context::ApplyScissor()
{
	const std::pair<Point,Point> &currentScissor = m_scissorStack.top();
	const Point &pos = currentScissor.first;
	const Point &size = currentScissor.second;

    vector2f flippedPos(pos.x, m_height-pos.y-size.y);
	m_renderer->SetScissor(true, flippedPos, vector2f(size.x, size.y));
}

void Context::PushDrawOffset(const Point &drawOffset)
{
	if (m_drawOffsetStack.size() > 0)
		m_drawOffsetStack.push(m_drawOffsetStack.top() + drawOffset);
	else
		m_drawOffsetStack.push(drawOffset);
	
	ApplyDrawOffset();
}

void Context::PopDrawOffset()
{
	m_drawOffsetStack.pop();
	if (m_drawOffsetStack.size() > 0)
		ApplyDrawOffset();
	else
		m_renderer->SetTransform(matrix4x4f::Identity());
}

void Context::ApplyDrawOffset()
{
	const Point &currentOffset(m_drawOffsetStack.top());
	m_renderer->SetTransform(matrix4x4f::Translation(currentOffset.x, currentOffset.y, 0));
}

void Context::ApplyDrawTransform(const Point &pos)
{
	if (m_drawOffsetStack.size() > 0) {
		const Point &currentOffset(m_drawOffsetStack.top());
		m_renderer->SetTransform(matrix4x4f::Translation(currentOffset.x + pos.x, currentOffset.y + pos.y, 0));
	}
	else
		m_renderer->SetTransform(matrix4x4f::Translation(pos.x, pos.y, 0));
}
#endif

void Context::DrawWidget(Widget *w)
{
	const Point &pos = w->GetPosition();
	const Point &drawOffset = w->GetDrawOffset();
	const Point &size = w->GetSize();

	m_drawWidgetPosition += pos;
	m_drawWidgetOffset += drawOffset;

	const std::pair<Point,Point> &currentScissor(m_scissorStack.top());
	const Point &currentScissorPos(currentScissor.first);
	const Point &currentScissorSize(currentScissor.second);

	const Point newScissorPos(std::max(m_drawWidgetPosition.x + m_drawWidgetOffset.x, currentScissorPos.x), std::max(m_drawWidgetPosition.y + m_drawWidgetOffset.y, currentScissorPos.y));
	const Point newScissorSize(
		Clamp(std::min(newScissorPos.x + size.x, currentScissorPos.x + currentScissorSize.x) - newScissorPos.x, 0, m_width),
		Clamp(std::min(newScissorPos.y + size.y, currentScissorPos.y + currentScissorSize.y) - newScissorPos.y, 0, m_height));

	m_scissorStack.push(std::make_pair(newScissorPos, newScissorSize));

    m_renderer->SetScissor(true, vector2f(newScissorPos.x, m_height - newScissorPos.y - newScissorSize.y), vector2f(newScissorSize.x, newScissorSize.y));

	m_renderer->SetTransform(matrix4x4f::Translation(m_drawWidgetPosition.x + m_drawWidgetOffset.x, m_drawWidgetPosition.y + m_drawWidgetOffset.y, 0));

	w->Draw();

	m_scissorStack.pop();

	m_drawWidgetPosition -= pos;
	m_drawWidgetOffset -= drawOffset;
}

}
