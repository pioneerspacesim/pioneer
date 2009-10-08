#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "Pi.h"
#include "Player.h"
#include "WorldView.h"
#include "ShipFlavour.h"
#include "ShipCpanel.h"
#include "Mission.h"
#include "CommodityTradeWidget.h"

#define TEXSIZE	128
#define ADD_VIDEO_WIDGET	Add(new DeadVideoLink(295,285), 5, 40)

class DeadVideoLink: public Gui::Widget {
public:
	void PutRandomCrapIntoTexture() {
		int *randcrap = (int*)alloca(TEXSIZE*TEXSIZE);
		for (unsigned int i=0; i<TEXSIZE*TEXSIZE/sizeof(int); i++) randcrap[i] = (Pi::rng.Int32() & 0xfcfcfcfc) >> 2;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, TEXSIZE, TEXSIZE, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, randcrap);
	}
	DeadVideoLink(float w, float h) {
		m_w = w; m_h = h;
		m_created = SDL_GetTicks();
		m_message = new Gui::ToolTip("Video link down");
		glEnable (GL_TEXTURE_2D);
		glGenTextures (1, &m_tex);
		glBindTexture (GL_TEXTURE_2D, m_tex);
		PutRandomCrapIntoTexture();
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glDisable (GL_TEXTURE_2D);
	}
	virtual ~DeadVideoLink() {
		glDeleteTextures(1, &m_tex);
		delete m_message;
	}
	virtual void Draw() {
		float size[2]; GetSize(size);
		if (SDL_GetTicks() - m_created < 1500) {
			m_message->SetText("Connecting...");
			glBegin(GL_QUADS);
				glColor3f(0,0,0);
				glVertex2f(0,0);
				glVertex2f(0,size[1]);
				glVertex2f(size[0],size[1]);
				glVertex2f(size[0],0);
			glEnd();
			DrawMessage();
		} else {
			m_message->SetText("Video link down");

			glEnable (GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, m_tex);
			PutRandomCrapIntoTexture();
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glBegin(GL_QUADS);
				glColor3f(0,0,0);
				glTexCoord2f(0,0);
				glVertex2f(0,0);
				glTexCoord2f(0,1);
				glVertex2f(0,size[1]);
				glTexCoord2f(1,1);
				glVertex2f(size[0],size[1]);
				glTexCoord2f(1,0);
				glVertex2f(size[0],0);
			glEnd();
			glDisable (GL_TEXTURE_2D);
			if (SDL_GetTicks() & 0x400) {
				DrawMessage();
			}
		}
	}
	virtual void GetSizeRequested(float size[2]) {
		size[0] = m_w;
		size[1] = m_h;
	}
private:
	void DrawMessage() {
		float size[2];
		float msgSize[2];
		GetSize(size);
		m_message->GetSize(msgSize);
		glPushMatrix();
		glTranslatef(size[0]*0.5f-msgSize[0]*0.5f, size[1]*0.5f-msgSize[1]*0.5f, 0);
		m_message->Draw();
		glPopMatrix();
	}
	Uint32 m_created;
	GLuint m_tex;
	float m_w, m_h;
	Gui::ToolTip *m_message;
};

class StationSubView: public Gui::Fixed {
public:
	StationSubView(): Gui::Fixed((float)Gui::Screen::GetWidth(), (float)(Gui::Screen::GetHeight()-64)) {
	}
	void GoBack() {
		GetParent()->RemoveChild(this);
		GetParent()->ShowChildren();
		GetParent()->SetTransparency(false);
		static_cast<StationSubView*>(GetParent())->UpdateBaseDisplay();
		delete this;
	}
	void UpdateBaseDisplay() {
		char buf[64];
		m_money->SetText(format_money(Pi::player->GetMoney()));

		const shipstats_t *stats = Pi::player->CalcStats();
		snprintf(buf, sizeof(buf), "%dt", stats->used_capacity - stats->used_cargo);
		m_equipmentMass->SetText(buf);
		
		snprintf(buf, sizeof(buf), "%dt", stats->used_cargo);
		m_cargoSpaceUsed->SetText(buf);
		
		snprintf(buf, sizeof(buf), "%dt", stats->free_capacity);
		m_cargoSpaceFree->SetText(buf);
	}
	void AddBaseDisplay() {
		const float YSEP = floor(Gui::Screen::GetFontHeight() * 1.5f);
		const float ystart = 350.0f;

		Add(new Gui::Label("#007Cash"), 10, ystart);
		Add(new Gui::Label("#007Legal status"), 10, ystart + 2*YSEP);
		Add(new Gui::Label("#007Used"), 140, ystart+4*YSEP);
		Add(new Gui::Label("#007Free"), 220, ystart+4*YSEP);
		Add(new Gui::Label("#007Cargo space"), 10, ystart+5*YSEP);
		Add(new Gui::Label("#007Equipment"), 10, ystart+6*YSEP);

		m_money = new Gui::Label("");
		Add(m_money, 220, ystart);

		m_cargoSpaceUsed = new Gui::Label("");
		Add(m_cargoSpaceUsed, 140, ystart + 5*YSEP);
		
		m_cargoSpaceFree = new Gui::Label("");
		Add(m_cargoSpaceFree, 220, ystart + 5*YSEP);
		
		m_equipmentMass = new Gui::Label("");
		Add(m_equipmentMass, 140, ystart + 6*YSEP);
		
		m_legalstatus = new Gui::Label("Clean");
		Add(m_legalstatus, 220, ystart + 2*YSEP);

		UpdateBaseDisplay();
	}
private:
	Gui::Label *m_legalstatus;
	Gui::Label *m_money;
	Gui::Label *m_cargoSpaceUsed;
	Gui::Label *m_cargoSpaceFree;
	Gui::Label *m_equipmentMass;
};

////////////////////////////////////////////////////////////////////

class StationCommoditiesView: public StationSubView {
public:
	StationCommoditiesView();
	virtual void ShowAll();
private:
	void OnClickBuy(int commodity_type) {
		m_station->SellItemTo(Pi::player, (Equip::Type)commodity_type);
		UpdateBaseDisplay();
	}
	void OnClickSell(int commodity_type) {
		Pi::player->SellItemTo(m_station, (Equip::Type)commodity_type);
		UpdateBaseDisplay();
	}
	SpaceStation *m_station;
};

StationCommoditiesView::StationCommoditiesView(): StationSubView()
{
	SetTransparency(false);
}

void StationCommoditiesView::ShowAll()
{
	DeleteAllChildren();

	m_station = Pi::player->GetDockedWith();
	assert(m_station);
	SetTransparency(false);
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "Welcome to %s commodities market", m_station->GetLabel().c_str());
		Add(new Gui::Label(buf), 10, 10);
	}

	Gui::Button *backButton = new Gui::SolidButton();
	backButton->onClick.connect(sigc::mem_fun(this, &StationCommoditiesView::GoBack));
	Add(backButton,680,470);
	Add(new Gui::Label("Go back"), 700, 470);

	CommodityTradeWidget *commodityTradeWidget = new CommodityTradeWidget(m_station);
	commodityTradeWidget->onClickBuy.connect(sigc::mem_fun(this, &StationCommoditiesView::OnClickBuy));
	commodityTradeWidget->onClickSell.connect(sigc::mem_fun(this, &StationCommoditiesView::OnClickSell));
	Add(commodityTradeWidget, 320, 40);

	AddBaseDisplay();
	ADD_VIDEO_WIDGET;

	Gui::Fixed::ShowAll();
}

////////////////////////////////////////////////////////////////////

#define REMOVAL_VALUE_PERCENT 90

class StationLaserPickMount: public StationSubView {
public:
	StationLaserPickMount(Equip::Type t, bool doFit) {
		m_equipType = t;
		m_doFit = doFit;
	}
	virtual void ShowAll();
private:
	void SelectMount(int mount) {
		SpaceStation *station = Pi::player->GetDockedWith();
		Equip::Slot slot = EquipType::types[m_equipType].slot;
		// duplicates some code in StationShipUpgradesView
		if (m_doFit) {
			// fit
			Pi::player->m_equipment.Set(slot, mount, m_equipType);
			Pi::player->CalcStats();
			Pi::player->SetMoney(Pi::player->GetMoney() - station->GetPrice(m_equipType));
			Pi::cpan->SetTemporaryMessage(0, "Fitting "+std::string(EquipType::types[m_equipType].name));
			Gui::Container *p = GetParent();
			GoBack();
			p->ShowAll();
		} else {
			// remove 
			const int value = station->GetPrice(m_equipType) * REMOVAL_VALUE_PERCENT / 100;
			Pi::player->m_equipment.Set(slot, mount, Equip::NONE);
			Pi::player->CalcStats();
			Pi::player->SetMoney(Pi::player->GetMoney() + value);
			station->AddEquipmentStock(m_equipType, 1);
			Pi::cpan->SetTemporaryMessage(0, "Removing "+std::string(EquipType::types[m_equipType].name));
			Gui::Container *p = GetParent();
			GoBack();
			p->ShowAll();
		}
	}
	Equip::Type m_equipType;
	bool m_doFit;
};

void StationLaserPickMount::ShowAll()
{
	DeleteAllChildren();
	SetTransparency(false);
	
	if (m_doFit) Add(new Gui::Label("Fit laser to which gun mount?"), 320, 200);
	else Add(new Gui::Label("Remove laser from which gun mount?"), 320, 200);

	Equip::Slot slot = EquipType::types[m_equipType].slot;

	int xpos = 400;
	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		if (m_doFit && (Pi::player->m_equipment.Get(slot, i) != Equip::NONE)) continue;
		if ((!m_doFit) && (Pi::player->m_equipment.Get(slot, i) != m_equipType)) continue;
		Gui::Button *b = new Gui::SolidButton();
		b->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationLaserPickMount::SelectMount), i));
		Add(b, xpos, 250);
		Add(new Gui::Label(ShipType::gunmountNames[i]), xpos, 270);

		xpos += 50;
	}
	Gui::Fixed::ShowAll();
}

////////////////////////////////////////////////////////////////////

class StationShipUpgradesView: public StationSubView {
public:
	StationShipUpgradesView();
	virtual void ShowAll();
private:
	void RemoveItem(Equip::Type t) {
		SpaceStation *station = Pi::player->GetDockedWith();
		Equip::Slot s = EquipType::types[t].slot;
		int value = station->GetPrice(t) * REMOVAL_VALUE_PERCENT / 100;
		int num = Pi::player->m_equipment.Count(s, t);
		
		if (num) {
			if ((s == Equip::SLOT_LASER) && (num > 1)) {
				/* you have a choice of mount points for lasers */
				HideChildren();
				SetTransparency(true);
				StationSubView *v = new StationLaserPickMount(t, false);
				Add(v, 0, 0);
				v->ShowAll();
			} else {
				Pi::player->m_equipment.Remove(s, t, 1);
				Pi::player->CalcStats();
				Pi::player->SetMoney(Pi::player->GetMoney() + value);
				station->AddEquipmentStock(t, 1);
				Pi::cpan->SetTemporaryMessage(0, "Removing "+std::string(EquipType::types[t].name));
				UpdateBaseDisplay();
				ShowAll();
			}
		}
	}
	void FitItem(Equip::Type t) {
		SpaceStation *station = Pi::player->GetDockedWith();
		Equip::Slot s = EquipType::types[t].slot;
		const shipstats_t *stats = Pi::player->CalcStats();
		int freespace = Pi::player->m_equipment.FreeSpace(s);
		
		if (Pi::player->GetMoney() < station->GetPrice(t)) {
			Pi::cpan->SetTemporaryMessage(0, "You do not have enough money");
		} else if (stats->free_capacity < EquipType::types[t].mass) {
			Pi::cpan->SetTemporaryMessage(0, "There is no space on your ship");
		} else if (freespace) {
			if ((freespace > 1) && (s == Equip::SLOT_LASER)) {
				/* you have a choice of mount points for lasers */
				HideChildren();
				SetTransparency(true);
				StationSubView *v = new StationLaserPickMount(t, true);
				Add(v, 0, 0);
				v->ShowAll();
			} else {
				Pi::player->m_equipment.Add(s, t);
				Pi::player->CalcStats();
				Pi::player->SetMoney(Pi::player->GetMoney() - station->GetPrice(t));
				Pi::cpan->SetTemporaryMessage(0, "Fitting "+std::string(EquipType::types[t].name));
				UpdateBaseDisplay();
				ShowAll();
			}
		} else {
			Pi::cpan->SetTemporaryMessage(0, "There is no space on your ship");
		}
	}
};

StationShipUpgradesView::StationShipUpgradesView(): StationSubView()
{
	SetTransparency(false);
}

void StationShipUpgradesView::ShowAll()
{
	DeleteAllChildren();

	SpaceStation *station = Pi::player->GetDockedWith();
	assert(station);
	SetTransparency(false);
	Add(new Gui::Label(station->GetLabel() + " Shipyard"), 10, 10);
	
	Gui::Button *backButton = new Gui::SolidButton();
	backButton->onClick.connect(sigc::mem_fun(this, &StationShipUpgradesView::GoBack));
	Add(backButton,680,470);
	Add(new Gui::Label("Go back"), 700, 470);

	Gui::Fixed *fbox = new Gui::Fixed(470, 400);
	Add(fbox, 320, 40);

	Gui::VScrollBar *scroll = new Gui::VScrollBar();
	Gui::VScrollPortal *portal = new Gui::VScrollPortal(450,400);
	scroll->SetAdjustment(&portal->vscrollAdjust);
	//int GetStock(Equip::Type t) const { return m_equipmentStock[t]; }

	int NUM_ITEMS = 0;
	const float YSEP = floor(Gui::Screen::GetFontHeight() * 1.5f);
	for (int i=1; i<Equip::TYPE_MAX; i++) {
		if ((!(EquipType::types[i].slot == Equip::SLOT_CARGO)) &&
		    station->GetStock(static_cast<Equip::Type>(i))) NUM_ITEMS++;
	}

	Gui::Fixed *innerbox = new Gui::Fixed(450, NUM_ITEMS*YSEP);
	for (int i=1, num=0; i<Equip::TYPE_MAX; i++) {
		Equip::Type type = static_cast<Equip::Type>(i);
		if (EquipType::types[i].slot == Equip::SLOT_CARGO) continue;
		int stock = station->GetStock(type);
		if (!stock) continue;
		Gui::Label *l = new Gui::Label(EquipType::types[i].name);
		innerbox->Add(l,0,num*YSEP);
		
		innerbox->Add(new Gui::Label(format_money(station->GetPrice(type))), 200, num*YSEP);

		innerbox->Add(new Gui::Label(format_money(REMOVAL_VALUE_PERCENT * station->GetPrice(type) / 100)),
				275, num*YSEP);
		
		innerbox->Add(new Gui::Label(stringf(64, "%dt", EquipType::types[i].mass)), 360, num*YSEP);
		
		Gui::Button *b;
		b = new Gui::SolidButton();
		b->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationShipUpgradesView::FitItem), type));
		innerbox->Add(b, 400, num*YSEP);
		// only have remove button if we have this item installed
		if (Pi::player->m_equipment.Count(EquipType::types[i].slot, type )) {
			b = new Gui::SolidButton();
			innerbox->Add(b, 420, num*YSEP);
			b->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationShipUpgradesView::RemoveItem), type));
		}
		num++;
	}
	innerbox->ShowAll();

	const float *col = Gui::Color::tableHeading;
	fbox->Add((new Gui::Label("Item"))->Color(col), 0, 0);
	fbox->Add((new Gui::Label("$ to fit"))->Color(col), 200, 0);
	fbox->Add((new Gui::Label("$ for removal"))->Color(col), 275, 0);
	fbox->Add((new Gui::Label("Wt"))->Color(col), 360, 0);
	fbox->Add((new Gui::Label("Fit"))->Color(col), 400, 0);
	fbox->Add((new Gui::Label("Remove"))->Color(col), 420, 0);
	fbox->Add(portal, 0, YSEP);
	fbox->Add(scroll, 455, YSEP);
	portal->Add(innerbox);
	portal->ShowAll();
	fbox->ShowAll();
	AddBaseDisplay();
	ADD_VIDEO_WIDGET;

	Gui::Fixed::ShowAll();
}

////////////////////////////////////////////////////////////////////

class StationViewShipView: public StationSubView {
public:
	StationViewShipView(int flavour_idx): StationSubView() {
		SpaceStation *station = Pi::player->GetDockedWith();
		m_flavourIdx = flavour_idx;
		m_flavour = station->GetShipsOnSale()[flavour_idx];
		m_sbreModel = sbreLookupModelByName(ShipType::types[m_flavour.type].sbreModelName);
		m_ondraw3dcon = Pi::spaceStationView->onDraw3D.connect(
				sigc::mem_fun(this, &StationViewShipView::Draw3D));
	}
	virtual ~StationViewShipView() {
		m_ondraw3dcon.disconnect();
	}
	virtual void ShowAll();
	void Draw3D();
private:
	void BuyShip() {
		// nasty. signal station->onShipsForSaleChanged will fire
		// and 'this' will be erased when StationShipyardView is
		// updated with new contents, so m_flavour must be kept on stack
		// alloc to avoid trouble
		ShipFlavour f = m_flavour;
		ShipFlavour _old = *Pi::player->GetFlavour();
		int cost = f.price - Pi::player->GetFlavour()->price;
		if (Pi::player->GetMoney() < cost) {
			Pi::cpan->SetTemporaryMessage(0, "You do not have enough money");
		} else {
			Pi::player->SetMoney(Pi::player->GetMoney() - cost);
			Pi::player->ChangeFlavour(&f);
			
			SpaceStation *station = Pi::player->GetDockedWith();
			station->ReplaceShipOnSale(m_flavourIdx, &_old);
		}
	}
	ShipFlavour m_flavour;
	int m_sbreModel;
	int m_flavourIdx;
	sigc::connection m_ondraw3dcon;
};

void StationViewShipView::Draw3D()
{
	/* XXX duplicated code in InfoView.cpp */
	ObjParams params = {
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f },

		{	// pColor[3]
		{ { 1.0f, 0.0f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
		{ { 0.8f, 0.6f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
		{ { 0.5f, 0.5f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },

		// pText[3][256]	
		{ "IR-L33T", "ME TOO" },
	};

	m_flavour.ApplyTo(&params);

	float guiscale[2];
	Gui::Screen::GetCoords2Pixels(guiscale);
	static float rot1, rot2;
	rot1 += .5*Pi::GetFrameTime();
	rot2 += Pi::GetFrameTime();
	glClearColor(0.25,.37,.63,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	const float bx = 5;
	const float by = 40;
	Gui::Screen::EnterOrtho();
	glColor3f(0,0,0);
	glBegin(GL_QUADS); {
		glVertex2f(bx,by);
		glVertex2f(bx,by+400);
		glVertex2f(bx+400,by+400);
		glVertex2f(bx+400,by);
	} glEnd();
	Gui::Screen::LeaveOrtho();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-.5, .5, -.5, .5, 1.0f, 10000.0f);
	glDepthRange (0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	float lightCol[] = { 1,1,1 };
	float lightDir[] = { 1,1,0 };

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	sbreSetDirLight (lightCol, lightDir);
	glViewport(bx/guiscale[0], (Gui::Screen::GetHeight() - by - 400)/guiscale[1],
			400/guiscale[0], 400/guiscale[1]);
	
	matrix4x4d rot = matrix4x4d::RotateXMatrix(rot1);
	rot.RotateY(rot2);

	vector3d p(0, 0, -2 * sbreGetModelRadius(m_sbreModel));
	sbreSetDepthRange (Pi::GetScrWidth()*0.5f, 0.0f, 1.0f);
	sbreRenderModel(&p.x, &rot[0], m_sbreModel, &params);
	glPopAttrib();
}

void StationViewShipView::ShowAll()
{
	const ShipType &t = ShipType::types[m_flavour.type];
	DeleteAllChildren();

	SetTransparency(true);
	SpaceStation *station = Pi::player->GetDockedWith();
	Add(new Gui::Label(station->GetLabel() + " Shipyard"), 10, 10);
	
	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &StationViewShipView::GoBack));
	Add(b,680,470);
	Add(new Gui::Label("Go back"), 700, 470);
	
	b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &StationViewShipView::BuyShip));
	Add(b,480,470);
	Add(new Gui::Label("Buy this ship"), 500, 470);


	const float YSEP = floor(Gui::Screen::GetFontHeight() * 1.5f);
	float y = 40;
	Add(new Gui::Label("Ship type"), 450, y);
	Add(new Gui::Label(t.name), 600, y);
	y+=YSEP;
	Add(new Gui::Label("Price"), 450, y);
	Add(new Gui::Label(format_money(m_flavour.price)), 600, y);
	y+=YSEP;
	Add(new Gui::Label("Registration id"), 450, y);
	Add(new Gui::Label(m_flavour.regid), 600, y);
	y+=YSEP;
	y+=YSEP;
	Add(new Gui::Label("Weight empty"), 450, y);
	Add(new Gui::Label(stringf(64, "%d t", t.hullMass)), 600, y);
	y+=YSEP;
	Add(new Gui::Label("Weight fully loaded"), 450, y);
	Add(new Gui::Label(stringf(64, "%d t", t.hullMass + t.capacity)), 600, y);
	y+=YSEP;
	Add(new Gui::Label("Capacity"), 450, y);
	Add(new Gui::Label(stringf(64, "%d t", t.capacity)), 600, y);
	y+=YSEP;
	y+=YSEP;
	// forward accel
	float accel = t.linThrust[ShipType::THRUSTER_REAR] / (-9.81*1000.0*(t.hullMass));
	Add(new Gui::Label("Forward accel (empty)"), 450, y);
	Add(new Gui::Label(stringf(64, "%.1f G", accel)), 600, y);
	y+=YSEP;
	accel = t.linThrust[ShipType::THRUSTER_REAR] / (-9.81*1000.0*(t.hullMass + t.capacity));
	Add(new Gui::Label("Forward accel (laden)"), 450, y);
	Add(new Gui::Label(stringf(64, "%.1f G", accel)), 600, y);
	y+=YSEP;
	// rev accel
	accel = t.linThrust[ShipType::THRUSTER_FRONT] / (9.81*1000.0*(t.hullMass));
	Add(new Gui::Label("Reverse accel (empty)"), 450, y);
	Add(new Gui::Label(stringf(64, "%.1f G", accel)), 600, y);
	y+=YSEP;
	accel = t.linThrust[ShipType::THRUSTER_FRONT] / (9.81*1000.0*(t.hullMass + t.capacity));
	Add(new Gui::Label("Reverse accel (laden)"), 450, y);
	Add(new Gui::Label(stringf(64, "%.1f G", accel)), 600, y);
	y+=YSEP;

	//AddBaseDisplay();
	//ADD_VIDEO_WIDGET;

	Gui::Fixed::ShowAll();
}


////////////////////////////////////////////////////////////////////

class StationShipRepairsView: public StationSubView {
public:
	StationShipRepairsView();
	virtual ~StationShipRepairsView() {
	}
	virtual void ShowAll();
private:
	int GetCostOfFixingHull(float percent) {
		return (int)(Pi::player->GetFlavour()->price * 0.001 * percent);
	}

	void RepairHull(float percent) {
		int cost = GetCostOfFixingHull(percent);
		if (Pi::player->GetMoney() < cost) {
			Pi::cpan->SetTemporaryMessage(0, "You do not have enough money");
		} else {
			Pi::player->SetMoney(Pi::player->GetMoney() - cost);
			Pi::player->SetPercentHull(Pi::player->GetPercentHull() + percent);
			ShowAll();
		}
	}
//	sigc::connection m_onShipsForSaleChangedConnection;
};

StationShipRepairsView::StationShipRepairsView(): StationSubView()
{
}

void StationShipRepairsView::ShowAll()
{
	DeleteAllChildren();

	SpaceStation *station = Pi::player->GetDockedWith();
	assert(station);
	SetTransparency(false);
	
	Add(new Gui::Label(station->GetLabel() + " Shipyard"), 10, 10);
	
	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &StationShipRepairsView::GoBack));
	Add(b,680,470);
	Add(new Gui::Label("Go back"), 700, 470);

	Gui::Fixed *fbox = new Gui::Fixed(470, 400);
	Add(fbox, 320, 40);

	const float YSEP = floor(Gui::Screen::GetFontHeight() * 1.5f);
	float ypos = YSEP;

	float hullPercent = Pi::player->GetPercentHull();
	if (hullPercent >= 100.0f) {
		fbox->Add(new Gui::Label("Your ship is in perfect working condition."), 0, ypos);
	} else {
		int costAll = GetCostOfFixingHull(100.0f - hullPercent);
		int cost1 = GetCostOfFixingHull(1.0f);
		if (cost1 < costAll) {
			fbox->Add(new Gui::Label("Repair 1.0% of hull damage"), 0, ypos);
			fbox->Add(new Gui::Label(format_money(cost1)), 350, ypos);
			
			Gui::SolidButton *b = new Gui::SolidButton();
			b->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationShipRepairsView::RepairHull), 1.0f));
			fbox->Add(b, 430, ypos);
			ypos += YSEP;
		}
		fbox->Add(new Gui::Label(stringf(128, "Repair all hull damage (%.1f%%)", 100.0f-hullPercent)), 0, ypos);
		fbox->Add(new Gui::Label(format_money(costAll)), 350, ypos);
		
		Gui::SolidButton *b = new Gui::SolidButton();
		b->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationShipRepairsView::RepairHull), 100.0f-hullPercent));
		fbox->Add(b, 430, ypos);
	}

	const float *col = Gui::Color::tableHeading;
	fbox->Add((new Gui::Label("Item"))->Color(col), 0, 0);
	fbox->Add((new Gui::Label("Price"))->Color(col), 350, 0);
	fbox->Add((new Gui::Label("Repair"))->Color(col), 430, 0);
	fbox->ShowAll();
	AddBaseDisplay();
	ADD_VIDEO_WIDGET;

	Gui::Fixed::ShowAll();
}


////////////////////////////////////////////////////////////////////

class StationBuyShipsView: public StationSubView {
public:
	StationBuyShipsView();
	virtual ~StationBuyShipsView() {
		m_onShipsForSaleChangedConnection.disconnect();
	}
	virtual void ShowAll();
private:
	void ViewShip(int idx) {
		HideChildren();
		SetTransparency(true);
		StationSubView *v = new StationViewShipView(idx);
		Add(v, 0, 0);
		v->ShowAll();
	}
	sigc::connection m_onShipsForSaleChangedConnection;
};

StationBuyShipsView::StationBuyShipsView(): StationSubView()
{
	SpaceStation *station = Pi::player->GetDockedWith();
	m_onShipsForSaleChangedConnection = station->onShipsForSaleChanged.connect(
			sigc::mem_fun(this, &StationBuyShipsView::ShowAll));
	SetTransparency(false);
}

void StationBuyShipsView::ShowAll()
{
	DeleteAllChildren();

	SpaceStation *station = Pi::player->GetDockedWith();
	assert(station);
	SetTransparency(false);
	
	Add(new Gui::Label(station->GetLabel() + " Shipyard"), 10, 10);
	
	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &StationBuyShipsView::GoBack));
	Add(b,680,470);
	Add(new Gui::Label("Go back"), 700, 470);

	Gui::Fixed *fbox = new Gui::Fixed(470, 400);
	Add(fbox, 320, 40);

	Gui::VScrollBar *scroll = new Gui::VScrollBar();
	Gui::VScrollPortal *portal = new Gui::VScrollPortal(450,400);
	scroll->SetAdjustment(&portal->vscrollAdjust);

	std::vector<ShipFlavour> &ships = station->GetShipsOnSale();
	int NUM_ITEMS = ships.size();
	const float YSEP = floor(Gui::Screen::GetFontHeight() * 1.5f);

	int num = 0;
	Gui::Fixed *innerbox = new Gui::Fixed(450, NUM_ITEMS*YSEP);
	for (std::vector<ShipFlavour>::iterator i = ships.begin(); i!=ships.end(); ++i) {
		Gui::Label *l = new Gui::Label(ShipType::types[(*i).type].name);
		innerbox->Add(l,0,num*YSEP);
		innerbox->Add(new Gui::Label(format_money((*i).price)), 200, num*YSEP);
		innerbox->Add(new Gui::Label(format_money((*i).price - Pi::player->GetFlavour()->price) ), 275, num*YSEP);
		innerbox->Add(new Gui::Label(stringf(16, "%dt", ShipType::types[(*i).type].capacity)), 370, num*YSEP);
		
		Gui::SolidButton *b = new Gui::SolidButton();
		b->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationBuyShipsView::ViewShip), num));
		innerbox->Add(b, 430, num*YSEP);
		num++;
	}
	innerbox->ShowAll();

	const float *col = Gui::Color::tableHeading;
	fbox->Add((new Gui::Label("Ship"))->Color(col), 0, 0);
	fbox->Add((new Gui::Label("Price"))->Color(col), 200, 0);
	fbox->Add((new Gui::Label("Part exchange"))->Color(col), 275, 0);
	fbox->Add((new Gui::Label("Capacity"))->Color(col), 370, 0);
	fbox->Add((new Gui::Label("View"))->Color(col), 430, 0);
	fbox->Add(portal, 0, YSEP);
	fbox->Add(scroll, 455, YSEP);
	portal->Add(innerbox);
	portal->ShowAll();
	fbox->ShowAll();
	AddBaseDisplay();
	ADD_VIDEO_WIDGET;

	Gui::Fixed::ShowAll();
}


////////////////////////////////////////////////////////////////////

class StationShipyardView: public StationSubView {
public:
	StationShipyardView();
	virtual void ShowAll();
private:
	void GotoUpgradesView() {
		HideChildren();
		SetTransparency(true);
		StationSubView *v = new StationShipUpgradesView();
		Add(v, 0, 0);
		v->ShowAll();
	}
	void GotoBuyShipsView() {
		HideChildren();
		SetTransparency(true);
		StationSubView *v = new StationBuyShipsView();
		Add(v, 0, 0);
		v->ShowAll();
	}
	void GotoShipRepairsView() {
		HideChildren();
		SetTransparency(true);
		StationSubView *v = new StationShipRepairsView();
		Add(v, 0, 0);
		v->ShowAll();
	}
};

StationShipyardView::StationShipyardView(): StationSubView()
{
	SetTransparency(false);
}

void StationShipyardView::ShowAll()
{
	DeleteAllChildren();

	SpaceStation *station = Pi::player->GetDockedWith();
	assert(station);
	SetTransparency(false);
	Add(new Gui::Label(station->GetLabel() + " Shipyard"), 10, 10);
	
	Gui::Button *backButton = new Gui::SolidButton();
	backButton->onClick.connect(sigc::mem_fun(this, &StationShipyardView::GoBack));
	Add(backButton,680,470);
	Add(new Gui::Label("Go back"), 700, 470);

	Gui::SolidButton *b = new Gui::SolidButton();
	b->SetShortcut(SDLK_1, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationShipyardView::GotoUpgradesView));
	Add(b, 340, 240);
	Gui::Label *l = new Gui::Label("Ship equipment");
	Add(l, 365, 240);
	
	b = new Gui::SolidButton();
	b->SetShortcut(SDLK_2, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationShipyardView::GotoShipRepairsView));
	Add(b, 340, 300);
	l = new Gui::Label("Repairs and servicing");
	Add(l, 365, 300);
	
	b = new Gui::SolidButton();
	b->SetShortcut(SDLK_3, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationShipyardView::GotoBuyShipsView));
	Add(b, 340, 360);
	l = new Gui::Label("New and reconditioned ships");
	Add(l, 365, 360);
	
	AddBaseDisplay();
	ADD_VIDEO_WIDGET;

	Gui::Fixed::ShowAll();
}

////////////////////////////////////////////////////////////////////

class StationBBView: public StationSubView {
public:
	StationBBView();
	virtual ~StationBBView() {
		m_onBBChangedConnection.disconnect();
	}
	virtual void ShowAll();
private:
	void OpenMission(int idx);
	sigc::connection m_onBBChangedConnection;
};

StationBBView::StationBBView(): StationSubView()
{
	SpaceStation *station = Pi::player->GetDockedWith();
	m_onBBChangedConnection = station->onBulletinBoardChanged.connect(
			sigc::mem_fun(this, &StationBBView::ShowAll));
	SetTransparency(false);
}

void StationBBView::OpenMission(int midx)
{
	DeleteAllChildren();
	SpaceStation *station = Pi::player->GetDockedWith();
	Add(new Gui::Label(station->GetLabel() + " Bulletin Board"), 10, 10);
	
	Gui::Fixed *f = new Gui::Fixed(470, 400);
	Add(f, 320, 40);
	Mission *m = station->GetBBMissions()[midx];
	MissionChatForm *chatform = new MissionChatForm();
	chatform->onFormClose.connect(sigc::mem_fun(this, &StationBBView::ShowAll));
	chatform->onSomethingChanged.connect(sigc::mem_fun(this, &StationBBView::UpdateBaseDisplay));
	m->StartChat(chatform);
	f->Add(chatform, 0, 0);
	f->ShowAll();
	
	AddBaseDisplay();
	ADD_VIDEO_WIDGET;
	Gui::Fixed::ShowAll();
}

void StationBBView::ShowAll()
{
	DeleteAllChildren();

	SpaceStation *station = Pi::player->GetDockedWith();
	assert(station);
	SetTransparency(false);
	
	Add(new Gui::Label(station->GetLabel() + " Bulletin Board"), 10, 10);
	
	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &StationBBView::GoBack));
	Add(b,680,470);
	Add(new Gui::Label("Go back"), 700, 470);

	Gui::Fixed *fbox = new Gui::Fixed(470, 400);
	Add(fbox, 320, 40);

	Gui::VScrollBar *scroll = new Gui::VScrollBar();
	Gui::VScrollPortal *portal = new Gui::VScrollPortal(450,400);
	scroll->SetAdjustment(&portal->vscrollAdjust);

	const std::vector<Mission*> &missions = station->GetBBMissions();
	int NUM_ITEMS = missions.size();
	const float YSEP = floor(Gui::Screen::GetFontHeight() * 5);

	int num = 0;
	Gui::Fixed *innerbox = new Gui::Fixed(450, NUM_ITEMS*YSEP);
	for (std::vector<Mission*>::const_iterator i = missions.begin(); i!=missions.end(); ++i) {
		Gui::SolidButton *b = new Gui::SolidButton();
		b->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationBBView::OpenMission), num));
		innerbox->Add(b, 10, num*YSEP);
		
		Gui::Label *l = new Gui::Label((*i)->GetBulletinBoardText());
		innerbox->Add(l,40,num*YSEP);
		
		num++;
	}
	innerbox->ShowAll();

	fbox->Add(portal, 0, 10);
	fbox->Add(scroll, 455, 10);
	portal->Add(innerbox);
	portal->ShowAll();
	fbox->ShowAll();
	AddBaseDisplay();
	ADD_VIDEO_WIDGET;

	Gui::Fixed::ShowAll();
}

/////////////////////////////////////////////////////////////////////

class StationRootView: public StationSubView {
public:
	StationRootView() {}
	virtual ~StationRootView() {}
	virtual void ShowAll();
private:
	void GotoShipyard();
	void GotoCommodities();
	void GotoBB();
	void OnClickRequestLaunch();
};

void StationRootView::ShowAll()
{

	SetTransparency(false);
	DeleteAllChildren();
	SpaceStation *station = Pi::player->GetDockedWith();
	assert(station);
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "Welcome to %s", station->GetLabel().c_str());
		Add(new Gui::Label(buf), 10, 10);
	}
	Gui::Label *l = new Gui::Label("Hello friend! Thankyou for docking with this space station!\n"
	"We regret to inform you that due to a spacetime fissure you have "
	"ended up in the terrible mirror universe of boringness, and that there "
	"is nothing to do. We hope to have things back to normal within a "
	"few weeks, and in the mean time would like to offer our apologies for "
	"any loss of earnings, immersion or lunch.  "
	"Currently the usual space station services are not available, but we "
	"can offer you this promotional message from one of the station's sponsors:\n"
	"                       DIET STEAKETTE: IT'S BAD");

	Gui::Fixed *fbox = new Gui::Fixed(450, 400);
	fbox->Add(l, 0, 0);
	Add(fbox, 320, 40);
	fbox->ShowAll();

	Gui::SolidButton *b = new Gui::SolidButton();
	b->SetShortcut(SDLK_1, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationRootView::OnClickRequestLaunch));
	Add(b, 340, 240);
	l = new Gui::Label("Request Launch");
	Add(l, 365, 240);

	b = new Gui::SolidButton();
	b->SetShortcut(SDLK_2, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationRootView::GotoShipyard));
	Add(b, 340, 300);
	l = new Gui::Label("Shipyard");
	Add(l, 365, 300);

	b = new Gui::SolidButton();
	b->SetShortcut(SDLK_3, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationRootView::GotoCommodities));
	Add(b, 340, 360);
	l = new Gui::Label("Commodity market");
	Add(l, 365, 360);

	b = new Gui::SolidButton();
	b->SetShortcut(SDLK_4, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationRootView::GotoBB));
	Add(b, 340, 420);
	l = new Gui::Label("Bulletin board");
	Add(l, 365, 420);

	AddBaseDisplay();
	ADD_VIDEO_WIDGET;

	Gui::Fixed::ShowAll();
}

void StationRootView::OnClickRequestLaunch()
{
	Pi::player->SetDockedWith(0,0);
	Pi::SetView(Pi::worldView);
}

void StationRootView::GotoCommodities()
{
	HideChildren();
	SetTransparency(true);
	StationSubView *v = new StationCommoditiesView();
	Add(v, 0, 0);
	v->ShowAll();
}

void StationRootView::GotoShipyard()
{
	HideChildren();
	SetTransparency(true);
	StationSubView *v = new StationShipyardView();
	Add(v, 0, 0);
	v->ShowAll();
}

void StationRootView::GotoBB()
{
	HideChildren();
	SetTransparency(true);
	StationSubView *v = new StationBBView();
	Add(v, 0, 0);
	v->ShowAll();
}

/////////////////////////////////////////////////////////////////////

SpaceStationView::SpaceStationView(): View()
{
	Gui::Label *l = new Gui::Label("Comms Link");
	l->Color(1,.7,0);
	m_rightRegion2->Add(l, 10, 0);
}

void SpaceStationView::OnSwitchTo()
{
	DeleteAllChildren();
	SetTransparency(true);
	StationSubView *v = new StationRootView();
	Add(v, 0, 0);
	v->ShowAll();
}

void SpaceStationView::Draw3D()
{
	onDraw3D.emit();
}

void SpaceStationView::Update()
{
}
