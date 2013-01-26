// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_CONTEXT_H
#define UI_CONTEXT_H

#include "RefCounted.h"
#include "text/TextureFont.h"

#include "EventDispatcher.h"
#include "Skin.h"

#include "Widget.h"
#include "FloatContainer.h"

#include "Margin.h"
#include "Align.h"
#include "Background.h"
#include "ColorBackground.h"
#include "Gradient.h"
#include "Expand.h"
#include "Box.h"
#include "Grid.h"
#include "Scroller.h"
#include "Icon.h"
#include "Image.h"
#include "Label.h"
#include "MultiLineText.h"
#include "Button.h"
#include "CheckBox.h"
#include "Slider.h"
#include "List.h"
#include "DropDown.h"
#include "TextEntry.h"
#include "SmallButton.h"
#include "Icon.h"

#include "Lua.h"
#include "LuaTable.h"

#include <stack>

class LuaManager;
namespace Graphics { class Renderer; }

namespace UI {

// The UI context is the top-level container, and covers the entire screen.
// Typically you will have one UI context per renderer context. Its slightly
// different to other containers internally to allow it to be a "live" widget
// without a parent container of its own.
//
// It has the simplest layout manager possible - it will only accept a single
// container widget
//
// The context holds resources that are shared by all widgets. Examples of
// such resources are fonts, default styles, textures and so on. New widgets
// are created from a context, and can access their context by calling their
// GetContext() method.
//
// It also holds an event dispatcher for distributing events to its widgets.
//
// The context also manages floating widgets. Floating widgets are drawn last
// over the top of everything else. Events given to a floating widget or its
// children will not propogate to a non-floating widget.

class Context : public Single {
public:
	Context(LuaManager *lua, Graphics::Renderer *renderer, int width, int height, const std::string &lang);
	virtual ~Context();

	// general purpose containers
	UI::HBox *HBox(float spacing = 0.0f) { return new UI::HBox(this, spacing); }
	UI::VBox *VBox(float spacing = 0.0f) { return new UI::VBox(this, spacing); }

	UI::Grid *Grid(const UI::CellSpec &rowSpec, const UI::CellSpec &colSpec) { return new UI::Grid(this, rowSpec, colSpec); }

	// single containers
	UI::Background *Background() { return new UI::Background(this); }
	UI::ColorBackground *ColorBackground(const Color &color) { return new UI::ColorBackground(this, color); }
	UI::Margin *Margin(int margin, Margin::Direction direction = Margin::ALL) { return new UI::Margin(this, margin, direction); };
	UI::Align *Align(UI::Align::Direction direction) { return new UI::Align(this, direction); }
	UI::Gradient *Gradient(const Color &beginColor, const Color &endColor, Gradient::Direction direction = Gradient::VERTICAL) { return new UI::Gradient(this, beginColor, endColor, direction); }
	UI::Expand *Expand(UI::Expand::Direction direction = Expand::BOTH) { return new UI::Expand(this, direction); }
	UI::Scroller *Scroller() { return new UI::Scroller(this); }

	// visual elements
	UI::Image *Image(const std::string &filename, Uint32 sizeControlFlags = 0) { return new UI::Image(this, filename, sizeControlFlags); }
	UI::Label *Label(const std::string &text) { return new UI::Label(this, text); }
	UI::Icon *Icon(const std::string &iconName) { return new UI::Icon(this, iconName); }

	UI::MultiLineText *MultiLineText(const std::string &text) { return new UI::MultiLineText(this, text); }

	UI::Button *Button() { return new UI::Button(this); }
	UI::SmallButton *SmallButton() { return new UI::SmallButton(this); }
	UI::CheckBox *CheckBox() { return new UI::CheckBox(this); }

	UI::HSlider *HSlider() { return new UI::HSlider(this); }
	UI::VSlider *VSlider() { return new UI::VSlider(this); }

	UI::List *List() { return new UI::List(this); }
	UI::DropDown *DropDown() { return new UI::DropDown(this); }

	UI::TextEntry *TextEntry(const std::string &text = "") { return new UI::TextEntry(this, text); }

	// add a floating widget
	Context *AddFloatingWidget(Widget *w, const Point &pos, const Point &size) { m_float->AddWidget(w, pos, size); return this; }
	Context *RemoveFloatingWidget(Widget *w) { m_float->RemoveWidget(w); return this; }

	// considers floating widgets also
	virtual Widget *GetWidgetAt(const Point &pos);

	// event dispatch delegates
	bool Dispatch(const Event &event) { return m_eventDispatcher.Dispatch(event); }
	bool DispatchSDLEvent(const SDL_Event &event) { return m_eventDispatcher.DispatchSDLEvent(event); }

	void RequestLayout() { m_needsLayout = true; }

	void SelectWidget(Widget *target) { m_eventDispatcher.SelectWidget(target); }
	void DeselectWidget(Widget *target) { m_eventDispatcher.DeselectWidget(target); }

	void DisableWidget(Widget *target) { m_eventDispatcher.DisableWidget(target); }
	void EnableWidget(Widget *target) { m_eventDispatcher.EnableWidget(target); }

	virtual void Layout();
	virtual void Update();
	virtual void Draw();

	LuaRef GetTemplateStore() const { return m_templateStore; }
	Widget *CallTemplate(const char *name, const LuaTable &args);
	Widget *CallTemplate(const char *name);

	Graphics::Renderer *GetRenderer() const { return m_renderer; }
	const Skin &GetSkin() const { return m_skin; }

	const float &GetScale() const { return m_scale; }

	RefCountedPtr<Text::TextureFont> GetFont() const { return GetFont(Widget::FONT_NORMAL); }
	RefCountedPtr<Text::TextureFont> GetFont(Widget::Font font) const { return m_font[font]; }

	const Point &GetScissor() const { return m_scissorStack.top().second; }

private:
	virtual Point PreferredSize() { return Point(); }

	Graphics::Renderer *m_renderer;
	int m_width;
	int m_height;

	float m_scale;

	bool m_needsLayout;

	RefCountedPtr<FloatContainer> m_float;

	EventDispatcher m_eventDispatcher;
	Skin m_skin;

	LuaManager *m_lua;

	LuaRef m_templateStore;

	RefCountedPtr<Text::TextureFont> m_font[FONT_MAX];

	// Container will draw widgets through the Context to correctly accumulate
	// positions and offsets
	friend class Container;
	void DrawWidget(Widget *w);

	// support for DrawWidget()
	Point m_drawWidgetPosition;
	std::stack< std::pair<Point,Point> > m_scissorStack;
};

}

#endif
