// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Context.h"
#include "FileSystem.h"
#include "text/FontDescriptor.h"
#include "Lua.h"
#include "FontConfig.h"
#include "FileSystem.h"
#include <typeinfo>

namespace UI {

// minimum screen height for scaling. if the screen has fewer vertical pixels
// than this, certain draw elements will be scaled down
static const int SCALE_CUTOFF_HEIGHT = 768;

static const float FONT_SCALE[] = {
	0.0f,  // INTERNAL (dummy)
	0.7f,  // XSMALL
	0.85f, // SMALL
	1.0f,  // NORMAL
	1.4f,  // LARGE
	1.8f   // XLARGE
};

static FontConfig font_config(const std::string &path) {
	RefCountedPtr<FileSystem::FileData> config_data = FileSystem::gameDataFiles.ReadFile(path);
	FontConfig fc;
	fc.Load(*config_data);
	config_data.Reset();
	return fc;
}

Context::Context(LuaManager *lua, Graphics::Renderer *renderer, int width, int height) : Single(this),
	m_renderer(renderer),
	m_width(width),
	m_height(height),
	m_scale(std::min(float(m_height)/SCALE_CUTOFF_HEIGHT, 1.0f)),
	m_needsLayout(false),
	m_float(new FloatContainer(this)),
	m_eventDispatcher(this),
	m_skin(FileSystem::JoinPath(FileSystem::GetDataDir(), "ui/Skin.ini"), renderer, GetScale()),
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
	const Text::FontDescriptor baseFontDesc(font_config("fonts/UIFont.ini").GetDescriptor());
	for (int i = FONT_SIZE_XSMALL; i < FONT_SIZE_MAX; i++) {
		const Text::FontDescriptor fontDesc(baseFontDesc.filename, baseFontDesc.pixelWidth*FONT_SCALE[i]*GetScale(), baseFontDesc.pixelHeight*FONT_SCALE[i]*GetScale(), baseFontDesc.outline, baseFontDesc.advanceXAdjustment);

		m_font[i] = RefCountedPtr<Text::TextureFont>(new Text::TextureFont(fontDesc, renderer));
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
