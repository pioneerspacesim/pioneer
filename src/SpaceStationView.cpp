#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "Pi.h"
#include "Player.h"
#include "WorldView.h"
#include "ShipFlavour.h"
#include "ShipCpanel.h"
#include "CommodityTradeWidget.h"
#include "GenericChatForm.h"
#include "LuaChatForm.h"
#include "PoliceChatForm.h"
#include "LmrModel.h"
#include "utils.h"

#if 0
////////////////////////////////////////////////////////////////////

#define REMOVAL_VALUE_PERCENT 90

class StationLaserPickMount: public GenericChatForm {
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
			Pi::player->UpdateMass();
			Pi::player->SetMoney(Pi::player->GetMoney() - station->GetPrice(m_equipType));
			Pi::cpan->MsgLog()->Message("", "Fitting "+std::string(EquipType::types[m_equipType].name));
			Gui::Container *p = GetParent();
			Close();
			p->ShowAll();
		} else {
			// remove 
			const Sint64 value = station->GetPrice(m_equipType) * REMOVAL_VALUE_PERCENT / 100;
			Pi::player->m_equipment.Set(slot, mount, Equip::NONE);
			Pi::player->UpdateMass();
			Pi::player->SetMoney(Pi::player->GetMoney() + value);
			station->AddEquipmentStock(m_equipType, 1);
			Pi::cpan->MsgLog()->Message("", "Removing "+std::string(EquipType::types[m_equipType].name));
			Gui::Container *p = GetParent();
			Close();
			p->ShowAll();
		}
	}
	Equip::Type m_equipType;
	bool m_doFit;
};

void StationLaserPickMount::ShowAll()
{
	ReInit();
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
		Add(b, float(xpos), 250);
		Add(new Gui::Label(ShipType::gunmountNames[i]), float(xpos), 270);

		xpos += 50;
	}
	Gui::Fixed::ShowAll();
}

////////////////////////////////////////////////////////////////////

class StationShipUpgradesView: public GenericChatForm {
public:
	StationShipUpgradesView();
	virtual void ShowAll();
private:
	void RemoveItem(Equip::Type t) {
		SpaceStation *station = Pi::player->GetDockedWith();
		Equip::Slot s = EquipType::types[t].slot;
		Sint64 value = station->GetPrice(t) * REMOVAL_VALUE_PERCENT / 100;
		int num = Pi::player->m_equipment.Count(s, t);
		
		if (num) {
			if ((s == Equip::SLOT_LASER) && (num > 1)) {
				/* you have a choice of mount points for lasers */
				OpenChildChatForm(new StationLaserPickMount(t, false));
			} else {
				Pi::player->m_equipment.Remove(t, 1);
				Pi::player->UpdateMass();
				Pi::player->SetMoney(Pi::player->GetMoney() + value);
				station->AddEquipmentStock(t, 1);
				Pi::cpan->MsgLog()->Message("", "Removing "+std::string(EquipType::types[t].name));
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
			Pi::cpan->MsgLog()->Message("", "You do not have enough money");
		} else if (stats->free_capacity < EquipType::types[t].mass) {
			Pi::cpan->MsgLog()->Message("", "There is no space on your ship");
		} else if (freespace) {
			if ((freespace > 1) && (s == Equip::SLOT_LASER)) {
				/* you have a choice of mount points for lasers */
				OpenChildChatForm(new StationLaserPickMount(t, true));
			} else {
				Pi::player->m_equipment.Add(t);
				Pi::player->UpdateMass();
				Pi::player->SetMoney(Pi::player->GetMoney() - station->GetPrice(t));
				Pi::cpan->MsgLog()->Message("", "Fitting "+std::string(EquipType::types[t].name));
				UpdateBaseDisplay();
				ShowAll();
			}
		} else {
			Pi::cpan->MsgLog()->Message("", "There is no space on your ship");
		}
	}
};

StationShipUpgradesView::StationShipUpgradesView(): GenericChatForm()
{
	SetTransparency(false);
}

void StationShipUpgradesView::ShowAll()
{
	ReInit();

	SpaceStation *station = Pi::player->GetDockedWith();
	assert(station);
	SetTransparency(false);
	SetTitle(stringf(256, "%s Shipyard", station->GetLabel().c_str()).c_str());
	
	Gui::Button *backButton = new Gui::SolidButton();
	backButton->onClick.connect(sigc::mem_fun(this, &StationShipUpgradesView::Close));
	Add(backButton,680,470);
	Add(new Gui::Label("Go back"), 700, 470);

	Gui::Fixed *fbox = new Gui::Fixed(470, 400);
	Add(fbox, 320, 40);

	Gui::VScrollBar *scroll = new Gui::VScrollBar();
	Gui::VScrollPortal *portal = new Gui::VScrollPortal(450);
	scroll->SetAdjustment(&portal->vscrollAdjust);
	//int GetStock(Equip::Type t) const { return m_equipmentStock[t]; }

	int NUM_ITEMS = 0;
	const float YSEP = floor(Gui::Screen::GetFontHeight() * 1.5f);
	for (int i=Equip::FIRST_SHIPEQUIP; i<=Equip::LAST_SHIPEQUIP; i++) {
		if (station->GetStock(static_cast<Equip::Type>(i))) NUM_ITEMS++;
	}

	Gui::Fixed *innerbox = new Gui::Fixed(450, NUM_ITEMS*YSEP);
	for (int i=Equip::FIRST_SHIPEQUIP, num=0; i<=Equip::LAST_SHIPEQUIP; i++) {
		Equip::Type type = static_cast<Equip::Type>(i);
		int stock = station->GetStock(type);
		if (!stock) continue;
		Gui::Label *l = new Gui::Label(EquipType::types[i].name);
		if (EquipType::types[i].description) {
			l->SetToolTip(EquipType::types[i].description);
		}
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

	const float *col = Gui::Theme::Colors::tableHeading;
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
	AddVideoWidget();

	Gui::Fixed::ShowAll();
}


////////////////////////////////////////////////////////////////////

class StationShipRepairsView: public GenericChatForm {
public:
	StationShipRepairsView();
	virtual ~StationShipRepairsView() {
	}
	virtual void ShowAll();
private:
	int GetCostOfFixingHull(float percent) {
		return int(Pi::player->GetFlavour()->price * 0.001 * percent);
	}

	void RepairHull(float percent) {
		int cost = GetCostOfFixingHull(percent);
		if (Pi::player->GetMoney() < cost) {
			Pi::cpan->MsgLog()->Message("", "You do not have enough money");
		} else {
			Pi::player->SetMoney(Pi::player->GetMoney() - cost);
			Pi::player->SetPercentHull(Pi::player->GetPercentHull() + percent);
			ShowAll();
		}
	}
//	sigc::connection m_onShipsForSaleChangedConnection;
};

StationShipRepairsView::StationShipRepairsView(): GenericChatForm()
{
}

void StationShipRepairsView::ShowAll()
{
	ReInit();

	SpaceStation *station = Pi::player->GetDockedWith();
	assert(station);
	SetTransparency(false);
	
	SetTitle(stringf(256, "%s Shipyard", station->GetLabel().c_str()).c_str());
	
	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &StationShipRepairsView::Close));
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
			
			Gui::SolidButton *sb = new Gui::SolidButton();
			sb->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationShipRepairsView::RepairHull), 1.0f));
			fbox->Add(sb, 430, ypos);
			ypos += YSEP;
		}
		fbox->Add(new Gui::Label(stringf(128, "Repair all hull damage (%.1f%%)", 100.0f-hullPercent)), 0, ypos);
		fbox->Add(new Gui::Label(format_money(costAll)), 350, ypos);
		
		Gui::SolidButton *sb = new Gui::SolidButton();
		sb->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationShipRepairsView::RepairHull), 100.0f-hullPercent));
		fbox->Add(sb, 430, ypos);
	}

	const float *col = Gui::Theme::Colors::tableHeading;
	fbox->Add((new Gui::Label("Item"))->Color(col), 0, 0);
	fbox->Add((new Gui::Label("Price"))->Color(col), 350, 0);
	fbox->Add((new Gui::Label("Repair"))->Color(col), 430, 0);
	fbox->ShowAll();
	AddBaseDisplay();
	AddVideoWidget();

	Gui::Fixed::ShowAll();
}
#endif

#include "StationServicesForm.h"

SpaceStationView::SpaceStationView(): View()
{
	Gui::Label *l = new Gui::Label("Comms Link");
	l->Color(1,.7,0);
	m_rightRegion2->Add(l, 10, 0);

	SetTransparency(false);

	m_title = new Gui::Label("");
	Add(m_title, 10, 10);


	m_statusBox = new Gui::Fixed(300, 300);
	Add(m_statusBox, 10, 350);

	const float YSEP = floor(Gui::Screen::GetFontHeight() * 1.5f);

	m_statusBox->Add(new Gui::Label("#007Cash"), 0, 0);
	m_statusBox->Add(new Gui::Label("#007Legal status"), 0, 2*YSEP);
	m_statusBox->Add(new Gui::Label("#007Used"), 130, 4*YSEP);
	m_statusBox->Add(new Gui::Label("#007Free"), 210, 4*YSEP);
	m_statusBox->Add(new Gui::Label("#007Cargo space"), 0, 5*YSEP);
	m_statusBox->Add(new Gui::Label("#007Equipment"), 0, 6*YSEP);

	m_money = new Gui::Label("");
	m_statusBox->Add(m_money, 210, 0);

	m_cargoSpaceUsed = new Gui::Label("");
	m_statusBox->Add(m_cargoSpaceUsed, 130, 5*YSEP);
	
	m_cargoSpaceFree = new Gui::Label("");
	m_statusBox->Add(m_cargoSpaceFree, 210, 5*YSEP);
	
	m_equipmentMass = new Gui::Label("");
	m_statusBox->Add(m_equipmentMass, 130, 6*YSEP);
	
	m_legalstatus = new Gui::Label("Clean");
	m_statusBox->Add(m_legalstatus, 210, 2*YSEP);


	m_formStack = new Gui::Stack();

	m_formController = new FormController(m_formStack);
	m_formController->onRefresh.connect(sigc::mem_fun(this, &SpaceStationView::RefreshForForm));


	m_backButtonBox = new Gui::HBox();
	m_backButtonBox->SetSpacing(4.0f);
	Add(m_backButtonBox, 680, 470);

	Gui::SolidButton *b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(m_formController, &FormController::CloseForm));
	m_backButtonBox->PackEnd(b);
	m_backButtonBox->PackEnd(new Gui::Label("Go back"));


	m_videoLink = 0;
	
	m_undockConnection = Pi::player->onUndock.connect(sigc::mem_fun(m_formStack, &Gui::Stack::Clear));
}

SpaceStationView::~SpaceStationView()
{
	delete m_formController;
	m_undockConnection.disconnect();
}

void SpaceStationView::Update()
{
	char buf[64];
	m_money->SetText(format_money(Pi::player->GetMoney()));

	const shipstats_t *stats = Pi::player->CalcStats();
	snprintf(buf, sizeof(buf), "%dt", stats->used_capacity - stats->used_cargo);
	m_equipmentMass->SetText(buf);
	
	snprintf(buf, sizeof(buf), "%dt", stats->used_cargo);
	m_cargoSpaceUsed->SetText(buf);
		
	snprintf(buf, sizeof(buf), "%dt", stats->free_capacity);
	m_cargoSpaceFree->SetText(buf);

	if (m_formStack->Size() > 1)
		m_backButtonBox->Show();
	else
		m_backButtonBox->Hide();
	
	if (static_cast<Form*>(m_formStack->Top())->GetType() == Form::BLANK)
		m_statusBox->Hide();
	else
		m_statusBox->Show();
}

void SpaceStationView::Draw3D()
{
	onDraw3D.emit();
}

void SpaceStationView::OnSwitchTo()
{
	if (m_videoLink) {
		Remove(m_videoLink);
		delete m_videoLink;
		m_videoLink = 0;
	}
	m_formController->JumpToForm(new StationServicesForm(m_formController));
}

void SpaceStationView::RefreshForForm(Form *f)
{
	m_title->SetText(f->GetTitle());

	switch (f->GetType()) {

		case Form::FACE: {
			FaceForm *form = static_cast<FaceForm*>(f);

			if (form->GetFaceSeed() == -1UL)
				form->SetFaceSeed(Pi::player->GetDockedWith()->GetSBody()->seed);

			if (!m_videoLink || form->GetFaceFlags() != m_videoLink->GetFlags() || form->GetFaceSeed() != m_videoLink->GetSeed()) {
				if (m_videoLink) {
					Remove(m_videoLink);
					delete m_videoLink;
				}

				m_videoLink = new FaceVideoLink(295, 285, form->GetFaceFlags(), form->GetFaceSeed());
				Add(m_videoLink, 10, 40);
			}
			m_videoLink->ShowAll();

			Remove(m_formStack);
			Add(m_formStack, 320, 40);

			break;
		}

		case Form::BLANK:
		default: {
			if (m_videoLink)
				m_videoLink->Hide();

			Remove(m_formStack);
			Add(m_formStack, 10, 40);

			break;
		}

	}

	m_formStack->ShowAll();
}
