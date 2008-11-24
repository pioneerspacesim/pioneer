#include "libs.h"
#include "Gui.h"

#define BUTTON_SIZE	16

namespace Gui {
ToggleButton::ToggleButton()
{
	m_pressed = false;
	SetSize(BUTTON_SIZE, BUTTON_SIZE);
}
bool ToggleButton::OnMouseDown(MouseButtonEvent *e)
{
	if (e->button == 1) {
		onPress.emit();
		m_pressed = !m_pressed;
		if (m_pressed) {
			onChange.emit(this, true);
		} else {
			onChange.emit(this, false);
		}
	}
	return false;
}

void ToggleButton::OnActivate()
{
	m_pressed = !m_pressed;
	if (m_pressed) {
		onChange.emit(this, true);
	} else {
		onChange.emit(this, false);
	}
}

void ToggleButton::GetSizeRequested(float size[2])
{
	size[0] = BUTTON_SIZE;
	size[1] = BUTTON_SIZE;
}

void ToggleButton::Draw()
{
	if (m_pressed) {
		glBegin(GL_QUADS);
			glColor3f(.6,.6,.6);
			glVertex2f(0,15);
			glVertex2f(15,15);
			glVertex2f(15,0);
			glVertex2f(0,0);
			
			glColor3fv(Color::bgShadow);
			glVertex2f(0,13);
			glVertex2f(13,13);
			glVertex2f(13,0);
			glVertex2f(0,0);
			
			glColor3fv(Color::bg);
			glVertex2f(2,13);
			glVertex2f(13,13);
			glVertex2f(13,2);
			glVertex2f(2,2);
		glEnd();
	} else {
		glBegin(GL_QUADS);
			glColor3f(.6,.6,.6);
			glVertex2f(0,15);
			glVertex2f(15,15);
			glVertex2f(15,0);
			glVertex2f(0,0);
			
			glColor3fv(Color::bgShadow);
			glVertex2f(2,15);
			glVertex2f(15,15);
			glVertex2f(15,2);
			glVertex2f(2,2);
			
			glColor3fv(Color::bg);
			glVertex2f(2,13);
			glVertex2f(13,13);
			glVertex2f(13,2);
			glVertex2f(2,2);
		glEnd();
	}
}

}
