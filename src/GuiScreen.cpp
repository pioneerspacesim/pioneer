#include "Gui.h"
#include "glfreetype.h"

namespace Gui {

FontFace *Screen::font;
bool Screen::initted = false;
int Screen::width;
int Screen::height;
int Screen::realWidth;
int Screen::realHeight;
float Screen::invRealWidth;
float Screen::invRealHeight;
std::list<Widget*> Screen::widgets;
std::list<Widget*> Screen::kbshortcut_widgets;
float Screen::font_xsize;
float Screen::font_ysize;
std::vector<Screen::LabelPos> Screen::labelPositions;

void Screen::Init(int real_width, int real_height, int ui_width, int ui_height)
{
	Screen::width = ui_width;
	Screen::height = ui_height;
	Screen::realWidth = real_width;
	Screen::realHeight = real_height;
	Screen::invRealWidth = 1.0f/real_width;
	Screen::invRealHeight = 1.0f/real_height;
	Screen::initted = true;
	Screen::font = new FontFace("font.ttf");
	Screen::font_xsize = 16*0.8;
	Screen::font_ysize = 16;
}

GLint Screen::Project(GLdouble objX, GLdouble objY, GLdouble objZ, const GLdouble *model, const GLdouble *proj, const GLint *view, GLdouble* winX, GLdouble *winY, GLdouble *winZ)
{
	GLint o = gluProject(objX, objY, objZ, model, proj, view, winX, winY, winZ);
	*winX = (*winX) * width * invRealWidth;
	*winY = (*winY) * height * invRealHeight;
	return o;
}

void Screen::EnterOrtho()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//glOrtho(0, 320, 0, 200, -1, 1);
	glOrtho(0, width, 0, height, -1, 1);
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
	
	for (std::list<Widget*>::iterator i = Screen::widgets.begin(); i != Screen::widgets.end(); ++i) {
		if (!(*i)->IsVisible()) continue;
		glPushMatrix();
		float pos[2];
		(*i)->GetPosition(pos);
		glTranslatef(pos[0], pos[1], 0);
		(*i)->Draw();
		glPopMatrix();
	}

	LeaveOrtho();
}

void Screen::AddBaseWidget(Widget *w)
{
	Screen::widgets.push_back(w);
}

void Screen::RemoveBaseWidget(Widget *w)
{
	Screen::widgets.remove(w);
}

void Screen::OnClick(SDL_MouseButtonEvent *e)
{
	MouseButtonEvent ev;
	float x = e->x;
	float y = e->y;
	y = height-(y*height*invRealHeight);
	x = x*width*invRealWidth;
	ev.button = e->button;
	ev.isdown = (e->type == SDL_MOUSEBUTTONDOWN);
	ev.x = x;
	ev.y = y;
	OnClickTestLabels(ev);
	for (std::list<Widget*>::iterator i = Screen::widgets.begin(); i != Screen::widgets.end(); ++i) {
		float size[2],pos[2];
		if (!(*i)->IsVisible()) continue;
		int evmask = (*i)->GetEventMask();
		if (ev.isdown) {
			if (!(evmask & Widget::EVENT_MOUSEDOWN)) continue;
		} else {
			if (!(evmask & Widget::EVENT_MOUSEUP)) continue;
		}
		(*i)->GetPosition(pos);
		(*i)->GetSize(size);

		if ((x >= pos[0]) && (x < pos[0]+size[0]) &&
		    (y >= pos[1]) && (y < pos[1]+size[1])) {

			ev.x = x-pos[0];
			ev.y = y-pos[1];

			if (ev.isdown) {
				(*i)->OnMouseDown(&ev);
			} else {
				(*i)->OnMouseUp(&ev);
			}
		}
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
		(*i)->OnPreShortcut(sym);
	}
}

void Screen::RenderString(const std::string &s)
{
	glPushMatrix();
	glScalef(Screen::font_xsize, Screen::font_ysize, 1);
	font->RenderString(s.c_str());
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
		glScalef(Screen::font_xsize, Screen::font_ysize, 1);
		glTranslatef(0.5*font->GetWidth(), -0.4*font->GetHeight(), 0);
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
		glScalef(Screen::font_xsize, Screen::font_ysize, 1);
		glTranslatef(0.5*font->GetWidth(), -0.4*font->GetHeight(), 0);
		font->RenderString(s.c_str());
		glPopMatrix();
	}
}

void Screen::AddShortcutWidget(Widget *w)
{
	kbshortcut_widgets.push_back(w);
}

}
