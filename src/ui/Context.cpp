// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Context.h"
#include "FileSystem.h"
#include "text/FontDescriptor.h"
#include "Lua.h"
#include "FileSystem.h"
#include <typeinfo>

namespace UI {

// minimum screen height for scaling. if the screen has fewer vertical pixels
// than this, certain draw elements will be scaled down
static const int SCALE_CUTOFF_HEIGHT = 768;

static const float FONT_SCALE[] = {
	0.7f,  // XSMALL
	0.85f, // SMALL
	1.0f,  // NORMAL
	1.4f,  // LARGE
	1.8f,  // XLARGE

	0.7f,  // HEADING_XSMALL
	0.85f, // HEADING_SMALL
	1.0f,  // HEADING_NORMAL
	1.4f,  // HEADING_LARGE
	1.8f   // HEADING_XLARGE
};

Context::Context(LuaManager *lua, Graphics::Renderer *renderer, int width, int height) : Single(this),
	m_renderer(renderer),
	m_width(width),
	m_height(height),
	m_scale(std::min(float(m_height)/SCALE_CUTOFF_HEIGHT, 1.0f)),
	m_needsLayout(false),
	m_float(new FloatContainer(this)),
	m_eventDispatcher(this),
	m_skin("ui/Skin.ini", renderer, GetScale()),
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
	{
		const Text::FontDescriptor baseFontDesc(Text::FontDescriptor::Load(FileSystem::gameDataFiles, "fonts/UIFont.ini"));
		for (int i = FONT_SMALLEST; i <= FONT_LARGEST; i++) {
			const Text::FontDescriptor fontDesc(baseFontDesc.filename, baseFontDesc.pixelWidth*FONT_SCALE[i]*GetScale(), baseFontDesc.pixelHeight*FONT_SCALE[i]*GetScale(), baseFontDesc.outline, baseFontDesc.advanceXAdjustment);

			m_font[i] = RefCountedPtr<Text::TextureFont>(new Text::TextureFont(fontDesc, renderer));
		}
	}
	{
		const Text::FontDescriptor baseFontDesc(Text::FontDescriptor::Load(FileSystem::gameDataFiles, "fonts/UIHeadingFont.ini"));
		for (int i = FONT_HEADING_SMALLEST; i <= FONT_HEADING_LARGEST; i++) {
			const Text::FontDescriptor fontDesc(baseFontDesc.filename, baseFontDesc.pixelWidth*FONT_SCALE[i]*GetScale(), baseFontDesc.pixelHeight*FONT_SCALE[i]*GetScale(), baseFontDesc.outline, baseFontDesc.advanceXAdjustment);

			m_font[i] = RefCountedPtr<Text::TextureFont>(new Text::TextureFont(fontDesc, renderer));
		}
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
	m_needsLayout = false;

	m_float->Layout();
	Single::Layout();

	if (m_needsLayout) {
		m_float->Layout();
		Single::Layout();
	}

	m_needsLayout = false;

	m_eventDispatcher.LayoutUpdated();
}

void Context::Update()
{
	m_eventDispatcher.Update();

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

void Context::DrawWidget(Widget *w)
{
	const Point &pos = w->GetPosition();
	const Point &drawOffset = w->GetDrawOffset();
	const Point &size = w->GetSize();

	m_drawWidgetPosition += pos;

	const std::pair<Point,Point> &currentScissor(m_scissorStack.top());
	const Point &currentScissorPos(currentScissor.first);
	const Point &currentScissorSize(currentScissor.second);

	const Point newScissorPos(std::max(m_drawWidgetPosition.x, currentScissorPos.x), std::max(m_drawWidgetPosition.y, currentScissorPos.y));

	const Point newScissorSize(
		Clamp(std::min(newScissorPos.x + size.x, currentScissorPos.x + currentScissorSize.x) - newScissorPos.x, 0, m_width),
		Clamp(std::min(newScissorPos.y + size.y, currentScissorPos.y + currentScissorSize.y) - newScissorPos.y, 0, m_height));

	m_scissorStack.push(std::make_pair(newScissorPos, newScissorSize));

	m_renderer->SetScissor(true, vector2f(newScissorPos.x, m_height - newScissorPos.y - newScissorSize.y), vector2f(newScissorSize.x, newScissorSize.y));

	m_drawWidgetPosition += drawOffset;

	m_renderer->SetTransform(matrix4x4f::Translation(m_drawWidgetPosition.x, m_drawWidgetPosition.y, 0));

	w->Draw();

	m_scissorStack.pop();

	m_drawWidgetPosition -= pos + drawOffset;
}

}
