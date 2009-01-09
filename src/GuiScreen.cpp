#include "Gui.h"
#include "glfreetype.h"

namespace Gui {

TextureFontFace *Screen::font;
bool Screen::initted = false;
int Screen::width;
int Screen::height;
int Screen::realWidth;
int Screen::realHeight;
float Screen::invRealWidth;
float Screen::invRealHeight;
float Screen::fontScale[2];
std::list<Widget*> Screen::kbshortcut_widgets;
std::vector<Screen::LabelPos> Screen::labelPositions;
Gui::Fixed *Screen::baseContainer;

void Screen::Init(int real_width, int real_height, int ui_width, int ui_height)
{
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
	Screen::fontScale[0] = ui_width / (float)real_width;
	Screen::fontScale[1] = ui_height / (float)real_height;
	Screen::font = new TextureFontFace("guifont.ttf", 12/fontScale[0], 12/fontScale[1]);
	Screen::baseContainer = new Gui::Fixed();
	Screen::baseContainer->SetSize(Screen::width, Screen::height);
	Screen::baseContainer->SetPosition(0,0);
	Screen::baseContainer->Show();
}

GLint Screen::Project(GLdouble objX, GLdouble objY, GLdouble objZ, const GLdouble *model, const GLdouble *proj, const GLint *view, GLdouble* winX, GLdouble *winY, GLdouble *winZ)
{
	GLint o = gluProject(objX, objY, objZ, model, proj, view, winX, winY, winZ);
	*winX = (*winX) * width * invRealWidth;
	*winY = GetHeight() - (*winY) * height * invRealHeight;
	return o;
}

void Screen::EnterOrtho()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
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
	labelPositions.clear();
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
	baseContainer->Add(w, x, y);
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
	OnClickTestLabels(ev);
	if (ev.isdown) {
		baseContainer->OnMouseDown(&ev);
		RawEvents::onMouseDown.emit(&ev);
	} else {
		baseContainer->OnMouseUp(&ev);
		RawEvents::onMouseUp.emit(&ev);
	}
}

void Screen::OnClickTestLabels(const Gui::MouseButtonEvent &ev)
{
	// hm. it is a pity the UI is fixed size for these purposes...
	for (std::vector<LabelPos>::iterator l = labelPositions.begin(); l != labelPositions.end(); ++l) {
		float dx = abs((*l).x - ev.x);
		float dy = abs((*l).y - ev.y);

		if ((dx < 5) && (dy < 5)) {
			(*l).onClick.emit(&ev);
		}
	}
}

void Screen::OnKeyDown(const SDL_keysym *sym)
{
	for (std::list<Widget*>::iterator i = kbshortcut_widgets.begin(); i != kbshortcut_widgets.end(); ++i) {
		if (!(*i)->IsVisible()) continue;
		if (!(*i)->GetEnabled()) continue;
		(*i)->OnPreShortcut(sym);
	}
}

float Screen::GetFontHeight()
{
	return font->GetHeight() * fontScale[1];
}

void Screen::MeasureString(const std::string &s, float &w, float &h)
{
	font->MeasureString(s.c_str(), w, h);
	w *= fontScale[0];
	h *= fontScale[1];
}

void Screen::MeasureLayout(const std::string &s, const float width, float outSize[2])
{
	font->MeasureLayout(s.c_str(), width / fontScale[0], outSize);
	outSize[0] *= fontScale[0];
	outSize[1] *= fontScale[1];
}

void Screen::LayoutString(const std::string &s, const float width)
{
	glPushMatrix();
	glScalef(Screen::fontScale[0], Screen::fontScale[1], 1);
	font->LayoutString(s.c_str(), width / fontScale[0]);
	glPopMatrix();
}

void Screen::RenderString(const std::string &s)
{
	glPushMatrix();
	glScalef(Screen::fontScale[0], Screen::fontScale[1], 1);
	font->RenderString(s.c_str());
	glPopMatrix();
}

void Screen::RenderMarkup(const std::string &s)
{
	glPushMatrix();
	glScalef(Screen::fontScale[0], Screen::fontScale[1], 1);
	font->RenderMarkup(s.c_str());
	glPopMatrix();
}

bool Screen::CanPutLabel(float x, float y)
{
	for (std::vector<LabelPos>::iterator i = labelPositions.begin(); i != labelPositions.end(); ++i) {
		if ((fabs(x-(*i).x) < 5) &&
		    (fabs(y-(*i).y) < 5)) return false;
	}
	return true;
}


void Screen::RenderLabel(const std::string &s, float x, float y)
{
	if (CanPutLabel(x, y)) {
		labelPositions.push_back(LabelPos(x,y));
		glPushMatrix();
		glTranslatef(x, y, 0);
		glScalef(Screen::fontScale[0], Screen::fontScale[1], 1);
		glTranslatef(0.5*font->GetWidth(), -0.5*font->GetHeight(), 0);
		font->RenderString(s.c_str());
		glPopMatrix();
	}
}

void Screen::PutClickableLabel(const std::string &s, float x, float y, sigc::slot<void, const Gui::MouseButtonEvent*> slot)
{
	if (CanPutLabel(x, y)) {
		LabelPos p = LabelPos(x,y);
		p.onClick.connect(slot);
		labelPositions.push_back(p);
		glPushMatrix();
		glTranslatef(x, y, 0);
		glScalef(Screen::fontScale[0], Screen::fontScale[1], 1);
		glTranslatef(0.5*font->GetWidth(), -0.5*font->GetHeight(), 0);
		font->RenderString(s.c_str());
		glPopMatrix();
	}
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
