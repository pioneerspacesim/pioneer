#include "StationShipEquipmentForm.h"
#include "Pi.h"
#include "Player.h"
#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "StationShipViewForm.h"

#define REMOVAL_VALUE_PERCENT 90

StationShipEquipmentForm::StationShipEquipmentForm(FormController *controller) : FaceForm(controller)
{
	m_station = Pi::player->GetDockedWith();

	SetTitle(stringf(256, "%s ship equipment", m_station->GetLabel().c_str()));

	Gui::VScrollBar *scroll = new Gui::VScrollBar();
	Gui::VScrollPortal *portal = new Gui::VScrollPortal(450);
	scroll->SetAdjustment(&portal->vscrollAdjust);

	int NUM_ITEMS = 0;
	const float YSEP = floor(Gui::Screen::GetFontHeight() * 1.5f);
	for (int i=Equip::FIRST_SHIPEQUIP; i<=Equip::LAST_SHIPEQUIP; i++) {
		if (m_station->GetStock(static_cast<Equip::Type>(i)))
			NUM_ITEMS++;
	}

	Gui::VBox *outerbox = new Gui::VBox();

	Gui::Fixed *innerbox = new Gui::Fixed(450, NUM_ITEMS*YSEP);
	innerbox->SetTransparency(true);

	for (int i=Equip::FIRST_SHIPEQUIP, num=0; i<=Equip::LAST_SHIPEQUIP; i++) {
		Equip::Type type = static_cast<Equip::Type>(i);
		int stock = m_station->GetStock(type);
		if (!stock) continue;
		Gui::Label *l = new Gui::Label(EquipType::types[i].name);
		if (EquipType::types[i].description) {
			l->SetToolTip(EquipType::types[i].description);
		}
		innerbox->Add(l,0,num*YSEP);
		
		innerbox->Add(new Gui::Label(format_money(m_station->GetPrice(type))), 200, num*YSEP);

		innerbox->Add(new Gui::Label(format_money(REMOVAL_VALUE_PERCENT * m_station->GetPrice(type) / 100)),
				275, num*YSEP);
		
		innerbox->Add(new Gui::Label(stringf(64, "%dt", EquipType::types[i].mass)), 360, num*YSEP);
		
		Gui::Button *b;
		b = new Gui::SolidButton();
		//b->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationShipUpgradesView::FitItem), type));
		innerbox->Add(b, 400, num*YSEP);
		// only have remove button if we have this item installed
		if (Pi::player->m_equipment.Count(EquipType::types[i].slot, type )) {
			b = new Gui::SolidButton();
			innerbox->Add(b, 420, num*YSEP);
			//b->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationShipUpgradesView::RemoveItem), type));
		}
		num++;
	}
	innerbox->ShowAll();

	portal->Add(innerbox);
	portal->ShowAll();

	Gui::Fixed *heading = new Gui::Fixed(470, Gui::Screen::GetFontHeight());
	const float *col = Gui::Theme::Colors::tableHeading;
	heading->Add((new Gui::Label("Item"))->Color(col), 0, 0);
	heading->Add((new Gui::Label("$ to fit"))->Color(col), 200, 0);
	heading->Add((new Gui::Label("$ for removal"))->Color(col), 275, 0);
	heading->Add((new Gui::Label("Wt"))->Color(col), 360, 0);
	heading->Add((new Gui::Label("Fit"))->Color(col), 400, 0);
	heading->Add((new Gui::Label("Remove"))->Color(col), 420, 0);
	outerbox->PackEnd(heading);

	Gui::HBox *body = new Gui::HBox();
	body->PackEnd(portal);
	body->PackEnd(scroll);
	outerbox->PackEnd(body);

	outerbox->SetSpacing(YSEP-Gui::Screen::GetFontHeight());
	outerbox->ShowAll();

	Add(outerbox, 0, 0);
	ShowAll();
}
