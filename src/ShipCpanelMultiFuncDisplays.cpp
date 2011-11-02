#include "libs.h"
#include "ShipCpanelMultiFuncDisplays.h"
#include "ShipCpanel.h"
#include "Space.h"
#include "Pi.h"
#include "Player.h"
#include "Missile.h"
#include "HyperspaceCloud.h"
#include "Sector.h"
#include "Sound.h"
#include "Lang.h"
#include "StringF.h"
#include "KeyBindings.h"

#define SCANNER_RANGE_MAX	100000.0f
#define SCANNER_RANGE_MIN	3000.0f
#define SCANNER_SCALE		0.00001f
#define SCANNER_YSHRINK		0.75f
#define SCANNER_MODE_AUTO	0
#define SCANNER_MODE_MAN	1
#define A_BIT				1.1f

MsgLogWidget::MsgLogWidget()
{
	m_msgAge = 0;
	m_msgLabel = new Gui::Label("");
	m_curMsgType = NONE;
	Add(m_msgLabel, 0, 4);
}

void MsgLogWidget::Update()
{
	if (m_curMsgType != NONE) {
		// has it expired?
		bool expired = (SDL_GetTicks() - m_msgAge > 5000);

		if (expired || ((m_curMsgType == NOT_IMPORTANT) && !m_msgQueue.empty())) {
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
		m_msgLabel->SetText("");
		m_msgAge = 0;
		onUngrabFocus.emit();
	} else {
		// current message expired and more in queue
		Pi::BoinkNoise();
		Pi::SetTimeAccel(1);
		Pi::RequestTimeAccel(1);
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
			m_msgLabel->SetText("#0f0" + msg.message);
		} else {
			m_msgLabel->SetText(
				std::string("#ca0") + stringf(Lang::MESSAGE_FROM_X, formatarg("sender", msg.sender)) +
				std::string("\n#0f0") + msg.message);
		}
		m_msgAge = SDL_GetTicks();
		m_curMsgType = msg.type;
		onGrabFocus.emit();
	}
}

void MsgLogWidget::GetSizeRequested(float size[2])
{
	size[0] = 400;
	size[1] = 64;
}

/////////////////////////////////

ScannerWidget::ScannerWidget()
{
	m_mode = SCANNER_MODE_AUTO;

	KeyBindings::toggleScanMode.onPress.connect(sigc::mem_fun(this, &ScannerWidget::NextMode));
}

void ScannerWidget::GetSizeRequested(float size[2])
{
	size[0] = 400;
	size[1] = 62;
}

void ScannerWidget::NextMode()
{
	if (IsVisible()) {
		if (m_mode == SCANNER_MODE_AUTO) m_mode = SCANNER_MODE_MAN;
		else m_mode = SCANNER_MODE_AUTO;
	}
}

void ScannerWidget::Draw()
{
	if (Pi::player->m_equipment.Get(Equip::SLOT_SCANNER) != Equip::SCANNER) return;

	UpdateContactsAndScale();

	float size[2];
	GetSize(size);
	m_x = size[0] * 0.5f;
	m_y = size[1] * 0.5f;
	Widget::SetClipping(size[0], size[1]);
	float c2p[2];
	Gui::Screen::GetCoords2Pixels(c2p);
	
	// draw objects below player (and below scanner)
	if (!m_contacts.empty()) DrawBlobs(true);

	// disc
	glEnable(GL_BLEND);
	glColor4f(0, 1.0f, 0, 0.1f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(m_x, m_y);
	for (float a = 0; a < 2 * M_PI; a += M_PI * 0.02) {
		glVertex2f(m_x + m_x * sin(a), m_y + SCANNER_YSHRINK * m_y * cos(a));
	}
	glVertex2f(m_x, m_y + SCANNER_YSHRINK * m_y);
	glEnd();
	glDisable(GL_BLEND);

	// circles and spokes
	glLineWidth(1);
	DrawRingsAndSpokes(false);
	// draw blended in slightly different places to anti-alias
	glPushMatrix();
	glEnable(GL_BLEND);
	glTranslatef(0.5f * c2p[0], 0.5f * c2p[1], 0);
	DrawRingsAndSpokes(true);
	glTranslatef(0, -c2p[1], 0);
	DrawRingsAndSpokes(true);
	glTranslatef(-c2p[0], 0, 0);
	DrawRingsAndSpokes(true);
	glTranslatef(0, c2p[1], 0);
	DrawRingsAndSpokes(true);
	glPopMatrix();
	glDisable(GL_BLEND);

	// objects above
	if (!m_contacts.empty()) DrawBlobs(false);

	Widget::EndClipping();
	glLineWidth(1.0f);
	glPointSize(1.0f);

	m_contacts.clear();
}

void ScannerWidget::UpdateContactsAndScale()
{
	// collect the bodies to be displayed, and if AUTO, distances
	float combat_dist = 0, far_ship_dist = 0, nav_dist = 0, far_other_dist = 0;
	for (Space::bodiesIter_t i = Space::bodies.begin(); i != Space::bodies.end(); ++i) {
		if ((*i) == Pi::player) continue;

		float dist = float((*i)->GetPositionRelTo(Pi::player).Length());
		if (dist > SCANNER_RANGE_MAX) continue;

		switch ((*i)->GetType()) {
			case Object::MISSILE:
				// XXX should ignore player's own missiles for range calc
			case Object::SHIP:
				if (m_mode == SCANNER_MODE_AUTO && !combat_dist) {
					if (dist > far_ship_dist) far_ship_dist = dist;
					if ((*i) == Pi::player->GetCombatTarget()) combat_dist = dist;
				}
				break;
			case Object::CARGOBODY:
			case Object::HYPERSPACECLOUD:
				// XXX could maybe add orbital stations
				if (m_mode == SCANNER_MODE_AUTO && !nav_dist && !combat_dist) {
					if (dist > far_other_dist) far_other_dist = dist;
					if ((*i) == Pi::player->GetNavTarget()) nav_dist = dist;
				}
				break;
			default: continue;
		}

		m_contacts.push_back(*i);
	}

	// range priority is combat target > ship/missile > nav target > other
	if (m_mode == SCANNER_MODE_AUTO) {
		if (combat_dist) m_range =
			Clamp(combat_dist * A_BIT, SCANNER_RANGE_MIN, SCANNER_RANGE_MAX);
		else if (far_ship_dist) m_range =
			Clamp(far_ship_dist * A_BIT, SCANNER_RANGE_MIN, SCANNER_RANGE_MAX);
		else if (nav_dist) m_range =
			Clamp(nav_dist * A_BIT, SCANNER_RANGE_MIN, SCANNER_RANGE_MAX);
		else if (far_other_dist) m_range =
			Clamp(far_other_dist * A_BIT, SCANNER_RANGE_MIN, SCANNER_RANGE_MAX);
		else m_range = SCANNER_RANGE_MAX;

		m_scale = SCANNER_SCALE * (SCANNER_RANGE_MAX / m_range);
	} else {
		if (KeyBindings::increaseScanRange.IsActive() && m_range < SCANNER_RANGE_MAX) {
			m_range = Clamp(m_range * 1.05f, SCANNER_RANGE_MIN, SCANNER_RANGE_MAX);
			m_scale = SCANNER_SCALE * (SCANNER_RANGE_MAX / m_range);
		} else if (KeyBindings::decreaseScanRange.IsActive() && m_range > SCANNER_RANGE_MIN) {
			m_range = Clamp(m_range * 0.95f, SCANNER_RANGE_MIN, SCANNER_RANGE_MAX);
			m_scale = SCANNER_SCALE * (SCANNER_RANGE_MAX / m_range);
		}
	}
}

void ScannerWidget::DrawBlobs(bool below)
{
	for (std::list<Body*>::iterator i = m_contacts.begin(); i != m_contacts.end(); ++i) {
		switch ((*i)->GetType()) {
			case Object::SHIP:
				if ((*i) == Pi::player->GetCombatTarget()) glColor3f(0.941f, 0.149f, 0.196f);
				else glColor3f(0.953f, 0.929f, 0.114f);
				glLineWidth(2);
				glPointSize(4);
				break;
			case Object::MISSILE:
				if (dynamic_cast<Missile*>(*i)->GetOwner() == Pi::player) glColor3f(0.953f, 0.929f, 0.114f);
				else glColor3f(0.941f, 0.149f, 0.196f);
				glLineWidth(1);
				glPointSize(3);
				break;
			case Object::CARGOBODY:
				if ((*i) == Pi::player->GetNavTarget()) glColor3f(0, 1.0f, 0);
				else glColor3f(0.65f, 0.65f, 0.65f);
				glLineWidth(1);
				glPointSize(3);
				break;
			case Object::HYPERSPACECLOUD:
				glColor3f(0.5f, 0.5f, 1.0f);
				glLineWidth(1);
				glPointSize(3);
				break;
			default: continue;
		}

		vector3d pos = (*i)->GetPositionRelTo(Pi::player);
		matrix4x4d rot;
		Pi::player->GetRotMatrix(rot);
		pos = rot.InverseOf() * pos;

		if ((pos.y > 0) && (below)) continue;
		if ((pos.y < 0) && (!below)) continue;

		float x = m_x + m_x * float(pos.x) * m_scale;
		float y_base = m_y + m_y * SCANNER_YSHRINK * float(pos.z) * m_scale;
		float y_blob = y_base - m_y * SCANNER_YSHRINK * float(pos.y) * m_scale;

		glBegin(GL_LINES);
		glVertex2f(x, y_base);
		glVertex2f(x, y_blob);
		glEnd();

		glBegin(GL_POINTS);
		glVertex2f(x, y_blob);
		glEnd();
	}
}

void ScannerWidget::DrawRingsAndSpokes(bool blend)
{
	static const float circle = float(2 * M_PI);
	static const float step = float(M_PI * 0.02); // 1/100th or 3.6 degrees

	/* soicles */
	if (blend) glColor4f(0, 0.4f, 0, 0.25f);
	else glColor3f(0, 0.4f, 0);
	/* inner soicle */
	glBegin(GL_LINE_LOOP);
	for (float a = 0; a < circle; a += step) {
		glVertex2f(m_x + 0.1f * m_x * sin(a), m_y + SCANNER_YSHRINK * 0.1f * m_y * cos(a));
	}
	glEnd();
	/* dynamic soicles */
	for (int p = 0; p < 7; ++p) {
		float sz = (pow(2.0f, p) * 1000.0f) / m_range;
		if (sz <= 0.1f) continue;
		if (sz >= 1.0f) break;
		glBegin(GL_LINE_LOOP);
		for (float a = 0; a < circle; a += step) {
			glVertex2f(m_x + sz * m_x * sin(a), m_y + SCANNER_YSHRINK * sz * m_y * cos(a));
		}
		glEnd();
	}
	/* schpokes */
	glBegin(GL_LINES);
	for (float a = 0; a < circle; a += float(M_PI * 0.25)) {
		glVertex2f(m_x + m_x * 0.1f * sin(a), m_y + 0.1f * SCANNER_YSHRINK * m_y * cos(a));
		glVertex2f(m_x + m_x * sin(a), m_y + SCANNER_YSHRINK * m_y * cos(a));
	}
	glEnd();

	/* outer range soicle */
	float range_percent = m_range / SCANNER_RANGE_MAX;

	if (m_mode == SCANNER_MODE_AUTO) {
		/* green like the scanner to indicate that the scanner is controlling the range */
		if (blend) glColor4f(0, 0.7f, 0, 0.25f);
		else glColor3f(0, 0.7f, 0);
	} else {
		if (blend) glColor4f(0.6f, 0.6f, 0, 0.25f);
		else glColor3f(0.6f, 0.6f, 0);
	}

	glBegin(GL_LINE_LOOP);
	for (float a = 0; a < range_percent * circle; a += step) {
		glVertex2f(m_x - m_x * sin(a), m_y + SCANNER_YSHRINK * m_y * cos(a));
	}
	if (range_percent < 1.0f) {
		/* this vertex is so the part that indicates range ends at
		 * the exact point rather than a multiple of 3.6 degrees */
		glVertex2f(m_x - m_x * sin(range_percent * circle),
			m_y + SCANNER_YSHRINK * m_y * cos(range_percent * circle));
		if (blend) glColor4f(0.2f, 0.3f, 0.2f, 0.25f);
		else glColor3f(0.2f, 0.3f, 0.2f);
		/* we start the second color at the same point so that it changes
		 * immediately rather than blending over 3.6 degrees */
		for (float a = range_percent * circle; a < circle; a += step) {
			glVertex2f(m_x - m_x * sin(a), m_y + SCANNER_YSHRINK * m_y * cos(a));
		}
		/* this vertex ensures that the end of the second color
		 * doesn't blend with the beginning of the first color */
		glVertex2f(m_x, m_y + SCANNER_YSHRINK * m_y);
	}
	glEnd();
}

/////////////////////////////////

UseEquipWidget::UseEquipWidget(): Gui::Fixed(400,100)
{
	m_onPlayerEquipChangedCon = Pi::onPlayerChangeEquipment.connect(sigc::mem_fun(this, &UseEquipWidget::UpdateEquip));
	UpdateEquip();
}

UseEquipWidget::~UseEquipWidget()
{
	m_onPlayerEquipChangedCon.disconnect();
}

void UseEquipWidget::GetSizeRequested(float size[2])
{
	size[0] = 400;
	size[1] = 62;
}

void UseEquipWidget::FireMissile(int idx)
{
	if (!Pi::player->GetCombatTarget()) {
		Pi::cpan->MsgLog()->Message("", Lang::SELECT_A_TARGET);
		return;
	}

	Pi::player->FireMissile(idx, static_cast<Ship*>(Pi::player->GetCombatTarget()));
}

void UseEquipWidget::UpdateEquip()
{
	DeleteAllChildren();
	int numSlots = Pi::player->m_equipment.GetSlotSize(Equip::SLOT_MISSILE);

	if (numSlots) {
		float spacing = 380.0f / numSlots;

		for (int i = 0; i < numSlots; ++i) {
			const Equip::Type t = Pi::player->m_equipment.Get(Equip::SLOT_MISSILE, i);
			if (t == Equip::NONE) continue;

			Gui::Button *b;
			switch (t) {
				case Equip::MISSILE_UNGUIDED:
					b = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/missile_unguided.png");
					break;
				case Equip::MISSILE_GUIDED:
					b = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/missile_guided.png");
					break;
				case Equip::MISSILE_SMART:
					b = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/missile_smart.png");
					break;
				default:
				case Equip::MISSILE_NAVAL:
					b = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/missile_naval.png");
					break;
			}
			Add(b, spacing * i, 40);
			b->onClick.connect(sigc::bind(sigc::mem_fun(this, &UseEquipWidget::FireMissile), i));
			b->SetToolTip(Equip::types[t].name);
		}
	}

	{
		const Equip::Type t = Pi::player->m_equipment.Get(Equip::SLOT_ECM);
		if (t != Equip::NONE) {
			Gui::Button *b = 0;
			if (t == Equip::ECM_BASIC) b = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/ecm_basic.png");
			else if (t == Equip::ECM_ADVANCED) b = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/ecm_advanced.png");
			assert(b);

			b->onClick.connect(sigc::mem_fun(Pi::player, &Ship::UseECM));

			Add(b, 32, 0);
		}
	}
		
}

///////////////////////////////////////////////

MultiFuncSelectorWidget::MultiFuncSelectorWidget(): Gui::Fixed(144, 17)
{
	m_active = 0;
	m_rg = new Gui::RadioGroup();
	
	m_buttons[0] = new Gui::ImageRadioButton(m_rg, PIONEER_DATA_DIR "/icons/multifunc_scanner.png", PIONEER_DATA_DIR "/icons/multifunc_scanner_on.png");
	m_buttons[0]->onSelect.connect(sigc::bind(sigc::mem_fun(this, &MultiFuncSelectorWidget::OnClickButton), MFUNC_SCANNER));
	m_buttons[0]->SetShortcut(SDLK_F9, KMOD_NONE);
	m_buttons[0]->SetSelected(true);

	m_buttons[1] = new Gui::ImageRadioButton(m_rg, PIONEER_DATA_DIR "/icons/multifunc_equip.png", PIONEER_DATA_DIR "/icons/multifunc_equip_on.png");
	m_buttons[1]->onSelect.connect(sigc::bind(sigc::mem_fun(this, &MultiFuncSelectorWidget::OnClickButton), MFUNC_EQUIPMENT));
	m_buttons[1]->SetShortcut(SDLK_F10, KMOD_NONE);

	m_buttons[2] = new Gui::ImageRadioButton(m_rg, PIONEER_DATA_DIR "/icons/multifunc_msglog.png", PIONEER_DATA_DIR "/icons/multifunc_msglog_on.png");
	m_buttons[2]->onSelect.connect(sigc::bind(sigc::mem_fun(this, &MultiFuncSelectorWidget::OnClickButton), MFUNC_MSGLOG));
	m_buttons[2]->SetShortcut(SDLK_F11, KMOD_NONE);

	UpdateButtons();

	ShowAll();
}

MultiFuncSelectorWidget::~MultiFuncSelectorWidget()
{
	delete m_rg;
}

void MultiFuncSelectorWidget::OnClickButton(multifuncfunc_t f)
{
	m_active = int(f);
	UpdateButtons();
	onSelect.emit(f);
}

void MultiFuncSelectorWidget::UpdateButtons()
{
	RemoveAllChildren();

	for (int i = 0; i < MFUNC_MAX; ++i) {
		Add(m_buttons[i], 36.0f + 36.0f * float(i), 0.0);
	}
}
