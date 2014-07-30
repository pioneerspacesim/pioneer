// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Gui.h"
#include "vector3.h"		// for projection
#include "text/TextSupport.h"

namespace Gui {

bool Screen::initted = false;
int Screen::width;
int Screen::height;
int Screen::realWidth;
int Screen::realHeight;
float Screen::invRealWidth;
float Screen::invRealHeight;
float Screen::fontScale[2];
std::list<Widget*> Screen::kbshortcut_widgets;
Gui::Fixed *Screen::baseContainer;
Gui::Widget *Screen::focusedWidget;
matrix4x4f Screen::modelMatrix;
matrix4x4f Screen::projMatrix;
GLint Screen::viewport[4];

FontCache Screen::s_fontCache;
std::stack< RefCountedPtr<Text::TextureFont> > Screen::s_fontStack;
RefCountedPtr<Text::TextureFont>Screen::s_defaultFont;

Graphics::Renderer *Screen::s_renderer;
Graphics::RenderState *Screen::alphaBlendState = nullptr;
Graphics::Material *Screen::flatColorMaterial = nullptr;

void Screen::Init(Graphics::Renderer *renderer, int real_width, int real_height, int ui_width, int ui_height)
{
    s_renderer = renderer;

	Screen::width = ui_width;
	Screen::height = ui_height;
	Screen::realWidth = real_width;
	Screen::realHeight = real_height;
	Screen::invRealWidth = 1.0f/real_width;
	Screen::invRealHeight = 1.0f/real_height;
	Screen::initted = true;
	// Why? because although our font textures get bigger with screen
	// resolution, our Gui Ortho projection is still 800x600 so vertex
	// coords must be scaled.
	Screen::fontScale[0] = ui_width / float(real_width);
	Screen::fontScale[1] = ui_height / float(real_height);
    s_defaultFont = s_fontCache.GetTextureFont("GuiFont");
    PushFont(s_defaultFont);
	Screen::baseContainer = new Gui::Fixed();
	Screen::baseContainer->SetSize(float(Screen::width), float(Screen::height));
	Screen::baseContainer->Show();

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA;
	rsd.depthWrite = false;
	alphaBlendState = renderer->CreateRenderState(rsd);

	Graphics::MaterialDescriptor mdesc;
	flatColorMaterial = renderer->CreateMaterial(mdesc);
}

void Screen::Uninit()
{
	Screen::baseContainer->RemoveAllChildren();		// children deleted elsewhere?
	delete Screen::baseContainer;
	delete flatColorMaterial;
}

static sigc::connection _focusedWidgetOnDelete;

void Screen::OnDeleteFocusedWidget()
{
	_focusedWidgetOnDelete.disconnect();
	focusedWidget = 0;
}

void Screen::SetFocused(Widget *w, bool enableKeyRepeat)
{
	ClearFocus();
	_focusedWidgetOnDelete = w->onDelete.connect(sigc::ptr_fun(&Screen::OnDeleteFocusedWidget));
	focusedWidget = w;
}

void Screen::ClearFocus()
{
	if (!focusedWidget) return;
	_focusedWidgetOnDelete.disconnect();
	focusedWidget = 0;
}

bool Screen::Project(const vector3d &in, vector3d &out)
{
	PROFILE_SCOPED()
	// implements gluProject (see the OpenGL documentation or the Mesa implementation of gluProject)
	const float * const M = modelMatrix.Data();
	const float * const P = projMatrix.Data();

	const double vcam[4] = { // camera space
		in.x*M[0] + in.y*M[4] + in.z*M[ 8] + M[12],
		in.x*M[1] + in.y*M[5] + in.z*M[ 9] + M[13],
		in.x*M[2] + in.y*M[6] + in.z*M[10] + M[14],
		in.x*M[3] + in.y*M[7] + in.z*M[11] + M[15]
	};
	const double vclip[4] = { // clip space
		vcam[0]*P[0] + vcam[1]*P[4] + vcam[2]*P[ 8] + vcam[3]*P[12],
		vcam[0]*P[1] + vcam[1]*P[5] + vcam[2]*P[ 9] + vcam[3]*P[13],
		vcam[0]*P[2] + vcam[1]*P[6] + vcam[2]*P[10] + vcam[3]*P[14],
		vcam[0]*P[3] + vcam[1]*P[7] + vcam[2]*P[11] + vcam[3]*P[15]
	};

	if (is_zero_exact(vclip[3])) { return false; }

	const double w = vclip[3];

	const double v[3] = {
		(vclip[0] / w) * 0.5 + 0.5,
		(vclip[1] / w) * 0.5 + 0.5,
		(vclip[2] / w) * 0.5 + 0.5
	};

	out.x = v[0] * viewport[2] + viewport[0];
	out.y = v[1] * viewport[3] + viewport[1];
	out.z = v[2];

	// map to pixels
	out.x = out.x * width * invRealWidth;
	out.y = GetHeight() - out.y * height * invRealHeight;
	return true;
}

void Screen::EnterOrtho()
{
	PROFILE_SCOPED()

	Graphics::Renderer *r = GetRenderer();

	modelMatrix = r->GetCurrentModelView();
	projMatrix = r->GetCurrentProjection();

	r->GetCurrentViewport(&viewport[0]);
	r->SetOrthographicProjection(0, width, height, 0, -1, 1);
	r->SetTransform(matrix4x4f::Identity());
}

void Screen::LeaveOrtho()
{
	PROFILE_SCOPED()

	Graphics::Renderer *r = GetRenderer();

	r->SetProjection(projMatrix);
	r->SetTransform(modelMatrix);
}

void Screen::Draw()
{
	PROFILE_SCOPED()
	assert(Screen::initted);
	EnterOrtho();
	baseContainer->Draw();
	LeaveOrtho();
}

bool Screen::IsBaseWidget(const Widget *w)
{
	PROFILE_SCOPED()
	return w == static_cast<const Widget*>(baseContainer);
}

void Screen::AddBaseWidget(Widget *w, int x, int y)
{
	baseContainer->Add(w, float(x), float(y));
}

void Screen::RemoveBaseWidget(Widget *w)
{
	baseContainer->Remove(w);
}

void Screen::SDLEventCoordToScreenCoord(int sdlev_x, int sdlev_y, float *x, float *y)
{
	*y = sdlev_y*height*invRealHeight;
	*x = sdlev_x*width*invRealWidth;
}

void Screen::OnMouseMotion(SDL_MouseMotionEvent *e)
{
	MouseMotionEvent ev;
	float x, y;
	Screen::SDLEventCoordToScreenCoord(e->x, e->y, &x, &y);
	ev.screenX = ev.x = x;
	ev.screenY = ev.y = y;
	baseContainer->OnMouseMotion(&ev);
	ev.screenX = ev.x = x;
	ev.screenY = ev.y = y;
	RawEvents::onMouseMotion.emit(&ev);
}

void Screen::OnClick(SDL_MouseButtonEvent *e)
{
	MouseButtonEvent ev;
	float x, y;
	Screen::SDLEventCoordToScreenCoord(e->x, e->y, &x, &y);
	ev.button = e->button;
	ev.isdown = (e->type == SDL_MOUSEBUTTONDOWN);
	ev.screenX = ev.x = x;
	ev.screenY = ev.y = y;
	if (ev.isdown) {
		baseContainer->OnMouseDown(&ev);
		RawEvents::onMouseDown.emit(&ev);
	} else {
		baseContainer->OnMouseUp(&ev);
		RawEvents::onMouseUp.emit(&ev);
	}
}

void Screen::OnKeyDown(const SDL_Keysym *sym)
{
	if (focusedWidget) {
		bool accepted = focusedWidget->OnKeyDown(sym);
		// don't check shortcuts if the focused widget accepted the key-press
		if (accepted)
			return;
	}
	for (std::list<Widget*>::iterator i = kbshortcut_widgets.begin(); i != kbshortcut_widgets.end(); ++i) {
		if (!(*i)->IsVisible()) continue;
		if (!(*i)->GetEnabled()) continue;
		(*i)->OnPreShortcut(sym);
	}
}

void Screen::OnKeyUp(const SDL_Keysym *sym)
{
}

void Screen::OnTextInput(const SDL_TextInputEvent *e)
{
	if (!focusedWidget) return;
	Uint32 unicode;
	Text::utf8_decode_char(&unicode, e->text);
	focusedWidget->OnTextInput(unicode);
}

float Screen::GetFontHeight(Text::TextureFont *font)
{
    if (!font) font = GetFont().Get();

	return font->GetHeight() * fontScale[1];
}

float Screen::GetFontDescender(Text::TextureFont *font)
{
    if (!font) font = GetFont().Get();

	return font->GetDescender() * fontScale[1];
}

void Screen::MeasureString(const std::string &s, float &w, float &h, Text::TextureFont *font)
{
	if (!font) font = GetFont().Get();
	assert(font);

	font->MeasureString(s.c_str(), w, h);
	w *= fontScale[0];
	h *= fontScale[1];
}

void Screen::MeasureCharacterPos(const std::string &s, int charIndex, float &x, float &y, Text::TextureFont *font)
{
	assert((charIndex >= 0) && (charIndex <= int(s.size())));

	if (!font) font = GetFont().Get();
	assert(font);

	font->MeasureCharacterPos(s.c_str(), charIndex, x, y);
	x *= fontScale[0];
	y *= fontScale[1];
}

int Screen::PickCharacterInString(const std::string &s, float x, float y, Text::TextureFont *font)
{
	if (!font) font = GetFont().Get();
	assert(font);

	x /= fontScale[0];
	y /= fontScale[1];

	return font->PickCharacter(s.c_str(), x, y);
}

void Screen::RenderString(const std::string &s, float xoff, float yoff, const Color &color, Text::TextureFont *font)
{
	PROFILE_SCOPED()
    if (!font) font = GetFont().Get();

	Graphics::Renderer *r = Gui::Screen::GetRenderer();

	const matrix4x4f &modelMatrix_ = r->GetCurrentModelView();
	Graphics::Renderer::MatrixTicket ticket(r, Graphics::MatrixMode::MODELVIEW);

	const float x = modelMatrix_[12] + xoff;
	const float y = modelMatrix_[13] + yoff;

	r->LoadIdentity();
	r->Translate(floor(x/Screen::fontScale[0])*Screen::fontScale[0], floor(y/Screen::fontScale[1])*Screen::fontScale[1], 0);
	r->Scale(Screen::fontScale[0], Screen::fontScale[1], 1);

	font->RenderString(s.c_str(), 0, 0, color);
}

void Screen::RenderMarkup(const std::string &s, const Color &color, Text::TextureFont *font)
{
	PROFILE_SCOPED()
    if (!font) font = GetFont().Get();

	Graphics::Renderer *r = Gui::Screen::GetRenderer();

	const matrix4x4f &modelMatrix_ = r->GetCurrentModelView();
	Graphics::Renderer::MatrixTicket ticket(r, Graphics::MatrixMode::MODELVIEW);

	const float x = modelMatrix_[12];
	const float y = modelMatrix_[13];

	r->LoadIdentity();
	r->Translate(floor(x/Screen::fontScale[0])*Screen::fontScale[0], floor(y/Screen::fontScale[1])*Screen::fontScale[1], 0);
	r->Scale(Screen::fontScale[0], Screen::fontScale[1], 1);

	font->RenderMarkup(s.c_str(), 0, 0, color);
}

void Screen::AddShortcutWidget(Widget *w)
{
	kbshortcut_widgets.push_back(w);
}

void Screen::RemoveShortcutWidget(Widget *w)
{
	kbshortcut_widgets.remove(w);
}

}
