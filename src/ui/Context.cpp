#include "Context.h"
#include "FileSystem.h"
#include "text/FontDescriptor.h"
#include "Lua.h"

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
}

Context::~Context() {
    m_float->Detach();
}

Widget *Context::GetWidgetAtAbsolute(const Point &pos) {
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

void Context::PushTransform(const matrix4x4f &transform)
{
	if (m_transformStack.size() > 0)
		m_transformStack.push(m_transformStack.top() * transform);
	else
		m_transformStack.push(transform);
	m_renderer->SetTransform(m_transformStack.top());
}

void Context::PopTransform()
{
	m_transformStack.pop();
	if (m_transformStack.size() > 0)
		m_renderer->SetTransform(m_transformStack.top());
	else
		m_renderer->SetTransform(matrix4x4f::Identity());
}

}
