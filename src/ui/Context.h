#ifndef _UI_CONTEXT_H
#define _UI_CONTEXT_H

#include "RefCounted.h"
#include "text/TextureFont.h"

#include "Skin.h"

#include "Margin.h"
#include "Background.h"
#include "Box.h"
#include "Image.h"
#include "Label.h"
#include "MultiLineText.h"
#include "Button.h"

namespace Graphics { class Renderer; }

namespace UI {

// The UI context holds resources that are shared by all widgets. Examples of
// such resources are fonts, default styles, textures and so on. New widgets
// are created from a context, and can access their context by calling their
// GetContext() method.

class Context {
public:
	Context(Graphics::Renderer *renderer);

	// general purpose containers
	UI::HBox *HBox() { return new UI::HBox(this); }
	UI::VBox *VBox() { return new UI::VBox(this); }

	// single containers
	UI::Background *Background(const Color &color) { return new UI::Background(this, color); }
	UI::Margin *Margin(float margin) { return new UI::Margin(this, margin); };

	// visual elements
	UI::Image *Image(const std::string &filename, Image::StretchMode stretchMode = Image::STRETCH_PRESERVE) { return new UI::Image(this, filename, stretchMode); }
	UI::Label *Label(const std::string &text) { return new UI::Label(this, text); }

	UI::MultiLineText *MultiLineText(const std::string &text) { return new UI::MultiLineText(this, text); }

	UI::Button *Button() { return new UI::Button(this); }

	Graphics::Renderer *GetRenderer() const { return m_renderer; }
	const Skin &GetSkin() const { return m_skin; }
	RefCountedPtr<Text::TextureFont> GetFont() const { return m_font; }

private:
	Graphics::Renderer *m_renderer;
	Skin m_skin;
	RefCountedPtr<Text::TextureFont> m_font;
};

}

#endif
