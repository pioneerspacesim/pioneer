#include "libs.h"
#include "ShipCpanelMultiFuncDisplays.h"
#include "Space.h"
#include "Pi.h"
#include "Player.h"

#define SCANNER_SCALE	0.01f
#define SCANNER_YSHRINK 0.75f

MsgLogWidget::MsgLogWidget()
{
	msgAge = 0;
	msgLabel = new Gui::Label("");
	curMsgType = NONE;
	Add(msgLabel, 0, 4);
}

void MsgLogWidget::Update()
{
	if (curMsgType != NONE) {
		// has it expired?
		bool expired = (Pi::GetGameTime() - msgAge > 5.0);

		if (expired || ((curMsgType == NOT_IMPORTANT) && !m_msgQueue.empty())) {
			ShowNext();
		}
	} else {
		ShowNext();
	}
}

void MsgLogWidget::ShowNext()
{
	if (m_msgQueue.empty()) {
		// current message expired and queue empty
		msgLabel->SetText("");
		msgAge = 0;
		onUngrabFocus.emit();
	} else {
		// current message expired and more in queue
		Pi::BoinkNoise();
		message_t msg("","",NONE);
		// use MUST_SEE messages first
		for (std::list<message_t>::iterator i = m_msgQueue.begin();
				i != m_msgQueue.end(); ++i) {
			if ((*i).type == MUST_SEE) {
				msg = *i;
				m_msgQueue.erase(i);
				break;
			}
		}
		if (msg.type == NONE) {
			msg = m_msgQueue.front();
			m_msgQueue.pop_front();
		}

		if (msg.sender == "") {
			msgLabel->SetText("#0f0"+msg.message);
		} else {
			msgLabel->SetText(stringf(1024, "#ca0Message from %s:\n#0f0%s", msg.sender.c_str(), msg.message.c_str()));
		}
		msgAge = (float)Pi::GetGameTime();
		curMsgType = msg.type;
		onGrabFocus.emit();
	}
}

void MsgLogWidget::GetSizeRequested(float size[2])
{
	size[0] = 400;
	size[1] = 64;
}

/////////////////////////////////

void ScannerWidget::GetSizeRequested(float size[2])
{
	size[0] = 400;
	size[1] = 62;
}

void ScannerWidget::Draw()
{
	float size[2];
	GetSize(size);
	const float mx = size[0]*0.5f;
	const float my = size[1]*0.5f;
	float c2p[2];
	Widget::SetClipping(size[0], size[1]);
	Gui::Screen::GetCoords2Pixels(c2p);
	
	// draw objects below player (and below scanner)
	DrawBlobs(true);
	/* disc */
	glEnable(GL_BLEND);
	glColor4f(0,1,0,0.1);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(mx, my);
	for (float a=0; a<2*M_PI; a+=M_PI*0.02) {
		glVertex2f(mx + mx*sin(a), my + SCANNER_YSHRINK*my*cos(a));
	}
	glVertex2f(mx, my + SCANNER_YSHRINK*my);
	glEnd();
	glDisable(GL_BLEND);
	
	glLineWidth(1);
	glColor3f(0,.4,0);
	DrawDistanceRings();
	glPushMatrix();
	glEnable(GL_BLEND);
	glColor4f(0,.4,0,0.25);
	glTranslatef(0.5f*c2p[0],0.5f*c2p[1],0);
	DrawDistanceRings();
	glTranslatef(0,-c2p[1],0);
	DrawDistanceRings();
	glTranslatef(-c2p[0],0,0);
	DrawDistanceRings();
	glTranslatef(0,c2p[1],0);
	DrawDistanceRings();
	glPopMatrix();
	glDisable(GL_BLEND);
	DrawBlobs(false);
	Widget::EndClipping();
}

void ScannerWidget::DrawBlobs(bool below)
{
	float size[2];
	GetSize(size);
	float mx = size[0]*0.5f;
	float my = size[1]*0.5f;
	glLineWidth(2);
	glPointSize(4);
	for (Space::bodiesIter_t i = Space::bodies.begin(); i != Space::bodies.end(); ++i) {
		if ((*i) == Pi::player) continue;
		if ((!(*i)->IsType(Object::SHIP)) &&
		    (!(*i)->IsType(Object::CARGOBODY))) continue;
		switch ((*i)->GetType()) {
			case Object::SHIP: glColor3f(1,0,0); break;
			case Object::CARGOBODY:
				glColor3f(.5,.5,1); break;
			default: continue;
		}
		if ((*i)->GetFrame() == Pi::player->GetFrame()) {
			vector3d pos = (*i)->GetPosition() - Pi::player->GetPosition();
			matrix4x4d rot;
			Pi::player->GetRotMatrix(rot);
			pos = rot.InverseOf() * pos;

			if ((pos.y>0)&&(below)) continue;
			if ((pos.y<0)&&(!below)) continue;

			glBegin(GL_LINES);
			glVertex2f(mx + (float)pos.x*SCANNER_SCALE, my + SCANNER_YSHRINK*(float)pos.z*SCANNER_SCALE);
			glVertex2f(mx + (float)pos.x*SCANNER_SCALE, my + SCANNER_YSHRINK*(float)pos.z*SCANNER_SCALE - SCANNER_YSHRINK*(float)pos.y*SCANNER_SCALE);
			glEnd();
			
			glBegin(GL_POINTS);
			glVertex2f(mx + (float)pos.x*SCANNER_SCALE, my + SCANNER_YSHRINK*(float)pos.z*SCANNER_SCALE - SCANNER_YSHRINK*(float)pos.y*SCANNER_SCALE);
			glEnd();
		}
	}
}
void ScannerWidget::DrawDistanceRings()
{
	float size[2];
	GetSize(size);
	float mx = size[0]*0.5f;
	float my = size[1]*0.5f;

	/* soicles */
	for (float sz=1.0f; sz>0.1f; sz-=0.33f) {
		glBegin(GL_LINE_LOOP);
		for (float a=0; a<2*M_PI; a+=(float)(M_PI*0.02)) {
			glVertex2f(mx + sz*mx*sin(a), my + SCANNER_YSHRINK*sz*my*cos(a));
		}
		glEnd();
	}
	/* schpokes */
	glBegin(GL_LINES);
	for (float a=0; a<2*M_PI; a+=(float)(M_PI*0.25)) {
		glVertex2f(mx, my);
		glVertex2f(mx + mx*sin(a), my + SCANNER_YSHRINK*my*cos(a));
	}
	glEnd();

}

///////////////////////////////////////////////

MultiFuncSelectorWidget::MultiFuncSelectorWidget(): Gui::Fixed(144, 17)
{
	m_active = 0;
	m_rg = new Gui::RadioGroup();
	
	m_buttons[0] = new Gui::ImageRadioButton(m_rg, "icons/multifunc_scanner.png", "icons/multifunc_scanner_on.png");
	m_buttons[0]->onSelect.connect(sigc::bind(sigc::mem_fun(this, &MultiFuncSelectorWidget::OnClickButton), MFUNC_SCANNER));
	m_buttons[0]->SetShortcut(SDLK_F9, KMOD_NONE);
	m_buttons[0]->SetSelected(true);

	m_buttons[1] = new Gui::ImageRadioButton(m_rg, "icons/multifunc_autopilot.png", "icons/multifunc_autopilot_on.png");
	m_buttons[1]->onSelect.connect(sigc::bind(sigc::mem_fun(this, &MultiFuncSelectorWidget::OnClickButton), MFUNC_AUTOPILOT));
	m_buttons[1]->SetShortcut(SDLK_F10, KMOD_NONE);

	m_buttons[2] = new Gui::ImageRadioButton(m_rg, "icons/multifunc_equip.png", "icons/multifunc_equip_on.png");
	m_buttons[2]->onSelect.connect(sigc::bind(sigc::mem_fun(this, &MultiFuncSelectorWidget::OnClickButton), MFUNC_EQUIPMENT));
	m_buttons[2]->SetShortcut(SDLK_F11, KMOD_NONE);

	m_buttons[3] = new Gui::ImageRadioButton(m_rg, "icons/multifunc_msglog.png", "icons/multifunc_msglog_on.png");
	m_buttons[3]->onSelect.connect(sigc::bind(sigc::mem_fun(this, &MultiFuncSelectorWidget::OnClickButton), MFUNC_MSGLOG));
	m_buttons[3]->SetShortcut(SDLK_F12, KMOD_NONE);

	UpdateButtons();

	ShowAll();
}
void MultiFuncSelectorWidget::OnClickButton(multifuncfunc_t f)
{
	m_active = (int)f;
	UpdateButtons();
	onSelect.emit(f);
}
void MultiFuncSelectorWidget::UpdateButtons()
{
	RemoveAllChildren();

	for (int i=0; i<MFUNC_MAX; i++) {
		// disable equip one
		if (i == MFUNC_AUTOPILOT) continue;
		Add(m_buttons[i], i*36.0, 0.0);
	}
}

