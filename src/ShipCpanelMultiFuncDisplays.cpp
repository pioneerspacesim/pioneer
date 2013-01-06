// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShipCpanelMultiFuncDisplays.h"
#include "galaxy/Sector.h"
#include "Game.h"
#include "HyperspaceCloud.h"
#include "KeyBindings.h"
#include "Lang.h"
#include "libs.h"
#include "Missile.h"
#include "Pi.h"
#include "Player.h"
#include "ShipCpanel.h"
#include "Sound.h"
#include "Space.h"
#include "StringF.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"

using namespace Graphics;

#define SCANNER_RANGE_MAX	100000.0f
#define SCANNER_RANGE_MIN	1000.0f
#define SCANNER_SCALE		0.00001f
#define SCANNER_YSHRINK		0.75f
#define A_BIT				1.1f
#define SCANNER_STEPS		100

enum ScannerBlobWeight { WEIGHT_LIGHT, WEIGHT_HEAVY };

// XXX target colours should be unified throughout the game
static const Color scannerNavTargetColour     = Color( 0,      1.0f,   0      );
static const Color scannerCombatTargetColour  = Color( 1.0f,   0,      0      );
static const Color scannerStationColour       = Color( 1.0f,   1.0f,   1.0f   );
static const Color scannerShipColour          = Color( 0.953f, 0.929f, 0.114f );
static const Color scannerMissileColour       = Color( 0.941f, 0.149f, 0.196f );
static const Color scannerPlayerMissileColour = Color( 0.953f, 0.929f, 0.114f );
static const Color scannerCargoColour         = Color( 0.65f,  0.65f,  0.65f  );
static const Color scannerCloudColour         = Color( 0.5f,   0.5f,   1.0f   );

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
		// cancel time acceleration (but don't unpause)
		if (Pi::game->GetTimeAccel() != Game::TIMEACCEL_PAUSED) {
			Pi::game->RequestTimeAccel(Game::TIMEACCEL_1X);
			Pi::game->SetTimeAccel(Game::TIMEACCEL_1X);
		}
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

ScannerWidget::ScannerWidget(Graphics::Renderer *r) :
	m_renderer(r)
{
	m_mode = SCANNER_MODE_AUTO;
	m_currentRange = m_manualRange = m_targetRange = SCANNER_RANGE_MIN;

	InitObject();
}

ScannerWidget::ScannerWidget(Graphics::Renderer *r, Serializer::Reader &rd) :
	m_renderer(r)
{
	m_mode = ScannerMode(rd.Int32());
	m_currentRange = rd.Float();
	m_manualRange = rd.Float();
	m_targetRange = rd.Float();

	InitObject();
}

void ScannerWidget::InitObject()
{
	m_toggleScanModeConnection = KeyBindings::toggleScanMode.onPress.connect(sigc::mem_fun(this, &ScannerWidget::ToggleMode));
	m_lastRange = SCANNER_RANGE_MAX * 100.0f;		// force regen
	GenerateBaseGeometry();
}

ScannerWidget::~ScannerWidget()
{
	m_toggleScanModeConnection.disconnect();
}

void ScannerWidget::GetSizeRequested(float size[2])
{
	size[0] = 400;
	size[1] = 62;
}

void ScannerWidget::ToggleMode()
{
	if (IsVisible() && Pi::game->GetTimeAccel() != Game::TIMEACCEL_PAUSED) {
		if (m_mode == SCANNER_MODE_AUTO) m_mode = SCANNER_MODE_MANUAL;
		else m_mode = SCANNER_MODE_AUTO;
	}
}

void ScannerWidget::Draw()
{
	if (Pi::player->m_equipment.Get(Equip::SLOT_SCANNER) != Equip::SCANNER) return;

	float size[2];
	GetSize(size);
	m_x = size[0] * 0.5f;
	m_y = size[1] * 0.5f;

	SetScissor(true);

	float rangediff = fabs(m_lastRange-m_currentRange);
	if (rangediff > 200.0 || rangediff / m_currentRange > 0.01) {
		GenerateRingsAndSpokes();
		m_lastRange = m_currentRange;
	}

	// draw objects below player (and below scanner)
	if (!m_contacts.empty()) DrawBlobs(true);

	// disc
	m_renderer->SetBlendMode(BLEND_ALPHA);
	Color green(0.f, 1.f, 0.f, 0.1f);

	// XXX 2d vertices
	VertexArray va(ATTRIB_POSITION | ATTRIB_DIFFUSE, 128); //reserve some space for positions & colors
	va.Add(vector3f(m_x, m_y, 0.f), green);
	for (float a = 0; a < 2 * float(M_PI); a += float(M_PI) * 0.02f) {
		va.Add(vector3f(m_x + m_x * sin(a), m_y + SCANNER_YSHRINK * m_y * cos(a), 0.f), green);
	}
	va.Add(vector3f(m_x, m_y + SCANNER_YSHRINK * m_y, 0.f), green);
	m_renderer->DrawTriangles(&va, Graphics::vtxColorMaterial, TRIANGLE_FAN);

	// circles and spokes
	glPushMatrix();
	glTranslatef(m_x, m_y, 0);
	glScalef(m_x, m_y, 1.0f);
	DrawRingsAndSpokes(false);
	glPopMatrix();

	// objects above
	if (!m_contacts.empty()) DrawBlobs(false);

	m_renderer->SetBlendMode(BLEND_SOLID);

	SetScissor(false);
}

void ScannerWidget::Update()
{
	m_contacts.clear();

	if (Pi::player->m_equipment.Get(Equip::SLOT_SCANNER) != Equip::SCANNER) {
		m_mode = SCANNER_MODE_AUTO;
		m_currentRange = m_manualRange = m_targetRange = SCANNER_RANGE_MIN;
		return;
	}

	// range priority is combat target > ship/missile > nav target > other
	enum { RANGE_MAX, RANGE_FAR_OTHER, RANGE_NAV, RANGE_FAR_SHIP, RANGE_COMBAT } range_type = RANGE_MAX;
	float combat_dist = 0, far_ship_dist = 0, nav_dist = 0, far_other_dist = 0;

	// collect the bodies to be displayed, and if AUTO, distances
	Space::BodyNearList nearby;
	Pi::game->GetSpace()->GetBodiesMaybeNear(Pi::player, SCANNER_RANGE_MAX, nearby);
	for (Space::BodyNearIterator i = nearby.begin(); i != nearby.end(); ++i) {
		if ((*i) == Pi::player) continue;

		float dist = float((*i)->GetPositionRelTo(Pi::player).Length());

		Contact c;
		c.type = (*i)->GetType();
		c.pos = (*i)->GetPositionRelTo(Pi::player);
		c.isSpecial = false;

		switch ((*i)->GetType()) {

			case Object::MISSILE:
				// player's own missiles are ignored for range calc but still shown
				if (static_cast<const Missile*>(*i)->GetOwner() == Pi::player) {
					c.isSpecial = true;
					break;
				}

				// else fall through

			case Object::SHIP: {
				const Ship *s = static_cast<const Ship*>(*i);
				if (s->GetFlightState() != Ship::FLYING && s->GetFlightState() != Ship::LANDED)
					continue;

				if ((*i) == Pi::player->GetCombatTarget()) c.isSpecial = true;

				if (m_mode == SCANNER_MODE_AUTO && range_type != RANGE_COMBAT) {
					if (c.isSpecial == true) {
						combat_dist = dist;
						range_type = RANGE_COMBAT;
					}
					else if (dist > far_ship_dist) {
						far_ship_dist = dist;
						range_type = RANGE_FAR_SHIP;
					}
				}
				break;
			}

			case Object::SPACESTATION:
			case Object::CARGOBODY:
			case Object::HYPERSPACECLOUD:

				if ((*i) == Pi::player->GetNavTarget()) c.isSpecial = true;

				if (m_mode == SCANNER_MODE_AUTO && range_type < RANGE_NAV) {
					if (c.isSpecial == true) {
						nav_dist = dist;
						range_type = RANGE_NAV;
					}
					else if (dist > far_other_dist) {
						far_other_dist = dist;
						range_type = RANGE_FAR_OTHER;
					}
				}
				break;

			default:
				continue;
		}

		m_contacts.push_back(c);
	}

	if (KeyBindings::increaseScanRange.IsActive()) {
		if (m_mode == SCANNER_MODE_AUTO) {
			m_manualRange = m_targetRange;
			m_mode = SCANNER_MODE_MANUAL;
		}
		else
			m_manualRange = m_currentRange;
		m_manualRange = Clamp(m_manualRange * 1.05f, SCANNER_RANGE_MIN, SCANNER_RANGE_MAX);
	}
	else if (KeyBindings::decreaseScanRange.IsActive()) {
		if (m_mode == SCANNER_MODE_AUTO) {
			m_manualRange = m_targetRange;
			m_mode = SCANNER_MODE_MANUAL;
		}
		else
			m_manualRange = m_currentRange;
		m_manualRange = Clamp(m_manualRange * 0.95f, SCANNER_RANGE_MIN, SCANNER_RANGE_MAX);
	}

	if (m_mode == SCANNER_MODE_AUTO) {
		switch (range_type) {
			case RANGE_COMBAT:
				m_targetRange = Clamp(combat_dist * A_BIT, SCANNER_RANGE_MIN, SCANNER_RANGE_MAX);
				break;
			case RANGE_FAR_SHIP:
				m_targetRange = Clamp(far_ship_dist * A_BIT, SCANNER_RANGE_MIN, SCANNER_RANGE_MAX);
				break;
			case RANGE_NAV:
				m_targetRange = Clamp(nav_dist * A_BIT, SCANNER_RANGE_MIN, SCANNER_RANGE_MAX);
				break;
			case RANGE_FAR_OTHER:
				m_targetRange = Clamp(far_other_dist * A_BIT, SCANNER_RANGE_MIN, SCANNER_RANGE_MAX);
				break;
			default:
				m_targetRange = SCANNER_RANGE_MAX;
				break;
		}
	}

	else
		m_targetRange = m_manualRange;
}

void ScannerWidget::DrawBlobs(bool below)
{
	for (std::list<Contact>::iterator i = m_contacts.begin(); i != m_contacts.end(); ++i) {
		ScannerBlobWeight weight = WEIGHT_LIGHT;

		const Color *color = 0;

		switch (i->type) {
			case Object::SHIP:
				if (i->isSpecial)
					color = &scannerCombatTargetColour;
				else
					color = &scannerShipColour;
				weight = WEIGHT_HEAVY;
				break;

			case Object::MISSILE:
				if (i->isSpecial)
					color = &scannerPlayerMissileColour;
				else
					color = &scannerMissileColour;
				break;

			case Object::SPACESTATION:
				if (i->isSpecial)
					color = &scannerNavTargetColour;
				else
					color = &scannerStationColour;
				weight = WEIGHT_HEAVY;
				break;

			case Object::CARGOBODY:
				if (i->isSpecial)
					color = &scannerNavTargetColour;
				else
					color = &scannerCargoColour;
				break;

			case Object::HYPERSPACECLOUD:
				if (i->isSpecial)
					color = &scannerNavTargetColour;
				else
					color = &scannerCloudColour;
				break;

			default:
				continue;
		}

		float pointSize = 1.f;
		if (weight == WEIGHT_LIGHT) {
			glLineWidth(1);
			pointSize = 3.f;
		}
		else {
			glLineWidth(2);
			pointSize = 4.f;
		}

		vector3d pos = i->pos * Pi::player->GetOrient();
		if ((pos.y > 0) && (below)) continue;
		if ((pos.y < 0) && (!below)) continue;

		float x = m_x + m_x * float(pos.x) * m_scale;
		float y_base = m_y + m_y * SCANNER_YSHRINK * float(pos.z) * m_scale;
		float y_blob = y_base - m_y * SCANNER_YSHRINK * float(pos.y) * m_scale;

		glColor3f(color->r, color->g, color->b);
		glBegin(GL_LINES);
		glVertex2f(x, y_base);
		glVertex2f(x, y_blob);
		glEnd();

		vector2f blob(x, y_blob);
		m_renderer->DrawPoints2D(1, &blob, color, pointSize);
	}
}

void ScannerWidget::GenerateBaseGeometry()
{
	static const float circle = float(2 * M_PI);
	static const float step = circle / SCANNER_STEPS;

	// circle (to be scaled and offset)
	m_circle.clear();
	m_circle.push_back(vector2f(0.0f, SCANNER_YSHRINK));
	float a = step;
	for (int i=1; i < SCANNER_STEPS; i++, a += step) {
		vector2f v = vector2f(sin(a), SCANNER_YSHRINK * cos(a));
		m_circle.push_back(v); m_circle.push_back(v);
	}
	m_circle.push_back(vector2f(0.0f, SCANNER_YSHRINK));

	// spokes
	m_spokes.clear();
	for (float ang = 0; ang < circle; ang += float(M_PI * 0.25)) {
		m_spokes.push_back(vector2f(0.1f * sin(ang), 0.1f * SCANNER_YSHRINK * cos(ang)));
		m_spokes.push_back(vector2f(sin(ang), SCANNER_YSHRINK * cos(ang)));
	}
}

void ScannerWidget::GenerateRingsAndSpokes()
{
	int csize = m_circle.size();
	int ssize = m_spokes.size();
	m_vts.clear();

	// inner circle
	for (int i=0; i<csize; i++) m_vts.push_back(m_circle[i] * 0.1f);

	// dynamic circles
	for (int p = 0; p < 7; ++p) {
		float sz = (pow(2.0f, p) * 1000.0f) / m_currentRange;
		if (sz <= 0.1f) continue;
		if (sz >= 1.0f) break;
		for (int i=0; i<csize; i++) m_vts.push_back(m_circle[i] * sz);
	}

	// spokes
	for (int i=0; i<ssize; i++) m_vts.push_back(m_spokes[i]);

	// outer ring
	m_edgeVts.clear();
	m_edgeCols.clear();
	int dimstart = 2 * int(SCANNER_STEPS * m_currentRange / SCANNER_RANGE_MAX);
	float a = 2.0f * M_PI * m_currentRange / SCANNER_RANGE_MAX;
	vector3f vn(sin(a), SCANNER_YSHRINK * cos(a), 0.0f);

	// bright part
	Color col = Color(0.7f, 0.7f, 0.f, 0.5f);
	if (m_mode == SCANNER_MODE_AUTO) {
		// green like the scanner to indicate that the scanner is controlling the range
		col = Color(0.f, 0.7f, 0.f, 0.5f);
	}
	for (int i=0; i<=dimstart; i++) {
		if (i == csize) return;			// whole circle bright case
		m_edgeVts.push_back(vector3f(m_circle[i].x, m_circle[i].y, 0.0f));
		m_edgeCols.push_back(col);
	}
	m_edgeVts.push_back(vn); m_edgeCols.push_back(col);

	// dim part
	col = Color(0.2f, 0.3f, 0.2f, 0.5f);
	m_edgeVts.push_back(vn); m_edgeCols.push_back(col);
	for (int i=dimstart+1; i<csize; i++) {
		m_edgeVts.push_back(vector3f(m_circle[i].x, m_circle[i].y, 0.0f));
		m_edgeCols.push_back(col);
	}
}

void ScannerWidget::DrawRingsAndSpokes(bool blend)
{
	Color col(0.f, 0.4f, 0.f, 0.5f);
	m_renderer->DrawLines2D(m_vts.size(), &m_vts[0], col);
	m_renderer->DrawLines(m_edgeVts.size(), &m_edgeVts[0], &m_edgeCols[0]);
}


void ScannerWidget::TimeStepUpdate(float step)
{
	if (m_targetRange < m_currentRange)
		m_currentRange = Clamp(m_currentRange - (m_currentRange*step), m_targetRange, SCANNER_RANGE_MAX);
	else if (m_targetRange > m_currentRange)
		m_currentRange = Clamp(m_currentRange + (m_currentRange*step), SCANNER_RANGE_MIN, m_targetRange);

	m_scale = SCANNER_SCALE * (SCANNER_RANGE_MAX / m_currentRange);
}

void ScannerWidget::Save(Serializer::Writer &wr)
{
	wr.Int32(Sint32(m_mode));
	wr.Float(m_currentRange);
	wr.Float(m_manualRange);
	wr.Float(m_targetRange);
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

			Gui::ImageButton *b;
			switch (t) {
				case Equip::MISSILE_UNGUIDED:
					b = new Gui::ImageButton("icons/missile_unguided.png");
					break;
				case Equip::MISSILE_GUIDED:
					b = new Gui::ImageButton("icons/missile_guided.png");
					break;
				case Equip::MISSILE_SMART:
					b = new Gui::ImageButton("icons/missile_smart.png");
					break;
				default:
				case Equip::MISSILE_NAVAL:
					b = new Gui::ImageButton("icons/missile_naval.png");
					break;
			}
			Add(b, spacing * i, 40);
			b->onClick.connect(sigc::bind(sigc::mem_fun(this, &UseEquipWidget::FireMissile), i));
			b->SetToolTip(Equip::types[t].name);
			b->SetRenderDimensions(16, 16);
		}
	}

	{
		const Equip::Type t = Pi::player->m_equipment.Get(Equip::SLOT_ECM);
		if (t != Equip::NONE) {
			Gui::ImageButton *b = 0;
			if (t == Equip::ECM_BASIC) b = new Gui::ImageButton("icons/ecm_basic.png");
			else if (t == Equip::ECM_ADVANCED) b = new Gui::ImageButton("icons/ecm_advanced.png");
			assert(b);

			b->onClick.connect(sigc::mem_fun(Pi::player, &Ship::UseECM));
			b->SetRenderDimensions(32, 32);

			Add(b, 32, 0);
		}
	}

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
	m_buttons[0]->SetRenderDimensions(34, 17);

	m_buttons[1] = new Gui::ImageRadioButton(m_rg, "icons/multifunc_equip.png", "icons/multifunc_equip_on.png");
	m_buttons[1]->onSelect.connect(sigc::bind(sigc::mem_fun(this, &MultiFuncSelectorWidget::OnClickButton), MFUNC_EQUIPMENT));
	m_buttons[1]->SetShortcut(SDLK_F10, KMOD_NONE);
	m_buttons[1]->SetRenderDimensions(34, 17);

	m_buttons[2] = new Gui::ImageRadioButton(m_rg, "icons/multifunc_msglog.png", "icons/multifunc_msglog_on.png");
	m_buttons[2]->onSelect.connect(sigc::bind(sigc::mem_fun(this, &MultiFuncSelectorWidget::OnClickButton), MFUNC_MSGLOG));
	m_buttons[2]->SetShortcut(SDLK_F11, KMOD_NONE);
	m_buttons[2]->SetRenderDimensions(34, 17);

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
