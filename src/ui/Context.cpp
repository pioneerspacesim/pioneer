// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Context.h"
#include "FileSystem.h"
#include "text/FontConfig.h"
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
	1.8f,  // HEADING_XLARGE

	0.7f,  // MONO_XSMALL
	0.85f, // MONO_SMALL
	1.0f,  // MONO_NORMAL
	1.4f,  // MONO_LARGE
	1.8f   // MONO_XLARGE
};

Context::Context(LuaManager *lua, Graphics::Renderer *renderer, int width, int height):
	Context(lua, renderer, width, height, std::min(float(height)/SCALE_CUTOFF_HEIGHT, 1.0f))
{}

Context::Context(LuaManager *lua, Graphics::Renderer *renderer, int width, int height, float scale) : Container(this),
	m_renderer(renderer),
	m_width(width),
	m_height(height),
	m_scale(scale),
	m_needsLayout(false),
	m_mousePointer(nullptr),
	m_mousePointerEnabled(true),
	m_eventDispatcher(this),
	m_skin("ui/Skin.ini", renderer, scale),
	m_lua(lua)
{
	lua_State *l = m_lua->GetLuaState();
	lua_newtable(l);
	m_templateStore = LuaRef(l, -1);

	SetSize(Point(m_width,m_height));
	m_visible = true;

	NewLayer();

	// XXX should do point sizes, but we need display DPI first
	// XXX TextureFont could load multiple sizes into the same object/atlas

	{
	const Text::FontConfig config("UIFont");
	for (int i = FONT_SMALLEST; i <= FONT_LARGEST; i++)
		m_font[i] = RefCountedPtr<Text::TextureFont>(new Text::TextureFont(config, renderer, FONT_SCALE[i]*GetScale()));
	}

	{
	const Text::FontConfig config("UIHeadingFont");
	for (int i = FONT_HEADING_SMALLEST; i <= FONT_HEADING_LARGEST; i++)
		m_font[i] = RefCountedPtr<Text::TextureFont>(new Text::TextureFont(config, renderer, FONT_SCALE[i]*GetScale()));
	}

	{
	const Text::FontConfig config("UIMonoFont");
	for (int i = FONT_MONO_SMALLEST; i <= FONT_MONO_LARGEST; i++)
		m_font[i] = RefCountedPtr<Text::TextureFont>(new Text::TextureFont(config, renderer, FONT_SCALE[i]*GetScale()));
	}

	m_scissorStack.push(std::make_pair(Point(0,0), Point(m_width,m_height)));
}

Layer *Context::NewLayer()
{
	Layer *layer = new Layer(this);
	AddWidget(layer);
	SetWidgetDimensions(layer, Point(0), Point(m_width, m_height));
	m_layers.push_back(layer);
	m_needsLayout = true;
	return layer;
}

void Context::DropLayer()
{
	// dropping the last layer would be bad
	if(m_layers.size() > 1)
	{
		RemoveWidget(m_layers.back());
		m_layers.pop_back();
		m_needsLayout = true;
	}
}

void Context::DropAllLayers()
{
	for (std::vector<Layer*>::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		RemoveWidget(*i);
	m_layers.clear();
	NewLayer();
	m_needsLayout = true;
}

void Context::HandleKeyDown(const KeyboardEvent &event) {
	if (event.keysym.sym == SDLK_ESCAPE) {
		if (m_layers.size()>1) {
			// go back to previous layer
			DropLayer();
		}
	}
}

Widget *Context::GetWidgetAt(const Point &pos)
{
	return GetTopLayer()->GetWidgetAt(pos);
}

void Context::Layout()
{
	// some widgets (eg MultiLineText) can require two layout passes because we
	// don't know their preferred size until after their first layout run. so
	// then we have to do layout again to make sure everyone else gets it right
	m_needsLayout = false;

	LayoutChildren();
	if (m_needsLayout)
		LayoutChildren();

	m_needsLayout = false;

	m_eventDispatcher.LayoutUpdated();
}

void Context::Update()
{
	m_animationController.Update();

	if (m_needsLayout)
		Layout();

	if (m_mousePointer && m_mousePointerEnabled)
		SetWidgetDimensions(m_mousePointer, m_eventDispatcher.GetMousePos()-m_mousePointer->GetHotspot(), m_mousePointer->PreferredSize());

	Container::Update();
}

void Context::Draw()
{
	Graphics::Renderer *r = GetRenderer();
	r->ClearDepthBuffer();

	// Ticket for the viewport mostly
	Graphics::Renderer::StateTicket ticket(r);
	r->SetViewport(0, 0, m_width, m_height);

	// reset renderer for each layer
	for (std::vector<Layer*>::iterator i = m_layers.begin(); i != m_layers.end(); ++i) {
		r->SetOrthographicProjection(0, m_width, m_height, 0, -1, 1);
		r->SetTransform(matrix4x4f::Identity());
		r->SetClearColor(Color::BLACK);

		DrawWidget(*i);

		r->SetScissor(false);
	}

	if (m_mousePointer && m_mousePointerEnabled) {
		r->SetOrthographicProjection(0, m_width, m_height, 0, -1, 1);
		r->SetTransform(matrix4x4f::Identity());
		r->SetClearColor(Color::BLACK);
		DrawWidget(m_mousePointer);
		r->SetScissor(false);
	}
}

Widget *Context::CallTemplate(const char *name, const LuaTable &args)
{
	return ScopedTable(m_templateStore).Call<UI::Widget *>(name, args);
}

Widget *Context::CallTemplate(const char *name)
{
	return CallTemplate(name, ScopedTable(m_lua->GetLuaState()));
}

void Context::DrawWidget(Widget *w)
{
	const Point &pos = w->GetPosition();
	const Point &drawOffset = w->GetDrawOffset();
	const Point &size = w->GetSize();

	const float &animX = w->GetAnimatedPositionX();
	const float &animY = w->GetAnimatedPositionY();

	const bool isAnimating = fabs(animX) < 1.0f || fabs(animY) < 1.0f;

	Point finalPos;
	if (isAnimating) {
		const Point absPos = w->GetAbsolutePosition();

		finalPos = Point(
			animX < 0.0f ? m_width-(m_width-pos.x)*-animX   : (pos.x+size.x)*animX-size.x-absPos.x*(1.0f-animX),
			animY < 0.0f ? m_height-(m_height-pos.y)*-animY : (pos.y+size.y)*animY-size.y-absPos.y*(1.0f-animY)
		);
	}
	else
		finalPos = w->GetPosition();

	m_drawWidgetPosition += finalPos;

	Point newScissorPos, newScissorSize;
	if (isAnimating) {
		newScissorPos = m_drawWidgetPosition;
		newScissorSize = size;
	}
	else {
		const std::pair<Point,Point> &currentScissor(m_scissorStack.top());
		const Point &currentScissorPos(currentScissor.first);
		const Point &currentScissorSize(currentScissor.second);

		newScissorPos = Point(std::max(m_drawWidgetPosition.x, currentScissorPos.x), std::max(m_drawWidgetPosition.y, currentScissorPos.y));

		newScissorSize = Point(
			Clamp(std::min(newScissorPos.x + size.x, currentScissorPos.x + currentScissorSize.x) - newScissorPos.x, 0, m_width),
			Clamp(std::min(newScissorPos.y + size.y, currentScissorPos.y + currentScissorSize.y) - newScissorPos.y, 0, m_height));
	}

	m_scissorStack.push(std::make_pair(newScissorPos, newScissorSize));

	m_renderer->SetScissor(true, vector2f(newScissorPos.x, m_height - newScissorPos.y - newScissorSize.y), vector2f(newScissorSize.x, newScissorSize.y));

	m_drawWidgetPosition += drawOffset;

	m_renderer->SetTransform(matrix4x4f::Translation(m_drawWidgetPosition.x, m_drawWidgetPosition.y, 0));

	float oldOpacity = m_opacityStack.empty() ? 1.0f : m_opacityStack.top();
	float opacity = oldOpacity * w->GetAnimatedOpacity();
	m_opacityStack.push(opacity);
	m_skin.SetOpacity(opacity);

	w->Draw();

	m_opacityStack.pop();
	m_scissorStack.pop();

	m_drawWidgetPosition -= finalPos + drawOffset;
}

void Context::SetMousePointer(const std::string &filename, const Point &hotspot)
{
	Point pos(0);

	if (m_mousePointer) {
		pos = m_mousePointer->GetPosition() + m_mousePointer->GetHotspot();
		RemoveWidget(m_mousePointer);
	}

	m_mousePointer = new MousePointer(this, filename, hotspot);

	AddWidget(m_mousePointer);
	SetWidgetDimensions(m_mousePointer, pos - m_mousePointer->GetHotspot(), m_mousePointer->PreferredSize());
}

}
