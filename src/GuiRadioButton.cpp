#include "libs.h"
#include "Gui.h"

#define BUTTON_SIZE	16

namespace Gui {
RadioButton::RadioButton(Gui::RadioGroup *g)
{
	m_pressed = false;
	SetSize(BUTTON_SIZE, BUTTON_SIZE);
	if (g) g->Add(this);
}
RadioButton::~RadioButton()
{
	
}
bool RadioButton::OnMouseDown(MouseButtonEvent *e)
{
	onPress.emit();
	OnActivate();
	return false;
}
void RadioButton::OnActivate()
{
	if (!m_pressed) onSelect.emit(this);
	m_pressed = true;
}
void RadioButton::GetSizeRequested(float &w, float &h)
{
	w = BUTTON_SIZE;
	h = BUTTON_SIZE;
}

void RadioButton::Draw()
{
	if (m_pressed) {
		glBegin(GL_QUADS);
			glColor3fv(Color::bgShadow);
			glVertex2f(0,0);
			glVertex2f(15,0);
			glVertex2f(15,15);
			glVertex2f(0,15);
			
			glColor3f(.6f,.6f,.6f);
			glVertex2f(2,0);
			glVertex2f(15,0);
			glVertex2f(15,13);
			glVertex2f(2,13);
			
			glColor3fv(Color::bg);
			glVertex2f(2,2);
			glVertex2f(13,2);
			glVertex2f(13,13);
			glVertex2f(2,13);
		glEnd();
	} else {
		glBegin(GL_QUADS);
			glColor3f(.6f,.6f,.6f);
			glVertex2f(0,0);
			glVertex2f(15,0);
			glVertex2f(15,15);
			glVertex2f(0,15);
			
			glColor3fv(Color::bgShadow);
			glVertex2f(2,0);
			glVertex2f(15,0);
			glVertex2f(15,13);
			glVertex2f(2,13);
			
			glColor3fv(Color::bg);
			glVertex2f(2,2);
			glVertex2f(13,2);
			glVertex2f(13,13);
			glVertex2f(2,13);
		glEnd();
	}

}
}
