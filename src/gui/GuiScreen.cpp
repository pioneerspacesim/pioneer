#include "Gui.h"
#include "vector3.h"		// for projection

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
GLdouble Screen::modelMatrix[16];
GLdouble Screen::projMatrix[16];
GLint Screen::viewport[4];

FontCache Screen::s_fontCache;
std::stack< RefCountedPtr<Text::TextureFont> > Screen::s_fontStack;
RefCountedPtr<Text::TextureFont>Screen::s_defaultFont;

Graphics::Renderer *Screen::s_renderer;

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
}

void Screen::Uninit()
{
	Screen::baseContainer->RemoveAllChildren();		// children deleted elsewhere?
	delete Screen::baseContainer;
}

static sigc::connection _focusedWidgetOnDelete;

void Screen::OnDeleteFocusedWidget()
{
	_focusedWidgetOnDelete.disconnect();
	focusedWidget = 0;
	SDL_EnableKeyRepeat(0, 0); // disable key repeat
}

void Screen::SetFocused(Widget *w, bool enableKeyRepeat)
{
	ClearFocus();
	if (enableKeyRepeat)
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	_focusedWidgetOnDelete = w->onDelete.connect(sigc::ptr_fun(&Screen::OnDeleteFocusedWidget));
	focusedWidget = w;
}

void Screen::ClearFocus()
{
	if (!focusedWidget) return;
	_focusedWidgetOnDelete.disconnect();
	focusedWidget = 0;
	SDL_EnableKeyRepeat(0, 0); // disable key repeat
}

void Screen::ShowBadError(const char *msg)
{
	fprintf(stderr, "%s", msg);
	baseContainer->HideChildren();
	
	Gui::Fixed *f = new Gui::Fixed(6*GetWidth()/8.0f, 6*GetHeight()/8.0f);
	Gui::Screen::AddBaseWidget(f, GetWidth()/8, GetHeight()/8);
	f->SetTransparency(false);
	f->SetBgColor(0.4f,0,0,1.0f);
	f->Add(new Gui::Label(msg), 10, 10);

	Gui::Button *okButton = new Gui::LabelButton(new Gui::Label("Ok"));
	okButton->SetShortcut(SDLK_RETURN, KMOD_NONE);
	f->Add(okButton, 10.0f, 6*GetHeight()/8.0f - 32);
	f->ShowAll();
	f->Show();

	do {
		Gui::MainLoopIteration();
		SDL_Delay(10);
	} while (!okButton->IsPressed());

	Gui::Screen::RemoveBaseWidget(f);
	delete f;
	baseContainer->ShowAll();
}

bool Screen::Project(const vector3d &in, vector3d &out)
{
	GLint o = gluProject(in.x, in.y, in.z, modelMatrix, projMatrix, viewport, &out.x, &out.y, &out.z);
	out.x = out.x * width * invRealWidth;
	out.y = GetHeight() - out.y * height * invRealHeight;
	return (o == GL_TRUE);
}

void Screen::EnterOrtho()
{
	glGetDoublev (GL_MODELVIEW_MATRIX, modelMatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projMatrix);
	glGetIntegerv (GL_VIEWPORT, viewport);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, width, height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void Screen::LeaveOrtho()
{	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
}

void Screen::Draw()
{
	assert(Screen::initted);
	EnterOrtho();
	baseContainer->Draw();
	LeaveOrtho();
}

bool Screen::IsBaseWidget(const Widget *w)
{
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

void Screen::OnKeyDown(const SDL_keysym *sym)
{
	if (focusedWidget) {
		bool accepted = focusedWidget->OnKeyPress(sym);
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

void Screen::OnKeyUp(const SDL_keysym *sym)
{
}

float Screen::GetFontHeight(Text::TextureFont *font)
{
    if (!font) font = GetFont().Get();

	return font->GetHeight() * fontScale[1];
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
    if (!font) font = GetFont().Get();

	GLdouble modelMatrix_[16];
	glPushMatrix();
	glGetDoublev (GL_MODELVIEW_MATRIX, modelMatrix_);
	float x = modelMatrix_[12] + xoff;
	float y = modelMatrix_[13] + yoff;
	glLoadIdentity();
	glTranslatef(floor(x/Screen::fontScale[0])*Screen::fontScale[0],
			floor(y/Screen::fontScale[1])*Screen::fontScale[1], 0);
	glScalef(Screen::fontScale[0], Screen::fontScale[1], 1);
	font->RenderString(s.c_str(), 0, 0, color);
	glPopMatrix();
}

void Screen::RenderMarkup(const std::string &s, const Color &color, Text::TextureFont *font)
{
    if (!font) font = GetFont().Get();

	GLdouble modelMatrix_[16];
	glPushMatrix();
	glGetDoublev (GL_MODELVIEW_MATRIX, modelMatrix_);
	float x = modelMatrix_[12];
	float y = modelMatrix_[13];
	glLoadIdentity();
	glTranslatef(floor(x/Screen::fontScale[0])*Screen::fontScale[0],
			floor(y/Screen::fontScale[1])*Screen::fontScale[1], 0);
	glScalef(Screen::fontScale[0], Screen::fontScale[1], 1);
	font->RenderMarkup(s.c_str(), 0, 0, color);
	glPopMatrix();
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
