#include "StationShipEquipmentForm.h"
#include "Pi.h"
#include "Player.h"
#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "StationShipViewForm.h"
#include "ShipCpanel.h"
#include "Lang.h"
#include "StringF.h"

#define REMOVAL_VALUE_PERCENT 90


class PickLaserMountForm : public FaceForm {
public:
	PickLaserMountForm(FormController *controller, StationShipEquipmentForm *equipForm, Equip::Type equipType, bool doFit);

private:
	void PickMount(int i);

	StationShipEquipmentForm *m_equipForm;
	Equip::Type m_equipType;
	bool m_doFit;
};


StationShipEquipmentForm::StationShipEquipmentForm(FormController *controller) : FaceForm(controller)
{
	m_station = Pi::player->GetDockedWith();

	SetTitle(stringf(Lang::SOMEWHERE_SHIP_EQUIPMENT, formatarg("station", m_station->GetLabel())));

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
		if (!m_station->GetStock(type) &&
			!(Pi::player->m_equipment.Count(Equip::types[i].slot, type) &&
			Equip::types[i].techLevel <= Pi::currentSystem->m_techlevel))
			continue;
		Gui::Label *l = new Gui::Label(Equip::types[i].name);
		if (Equip::types[i].description) {
			l->SetToolTip(Equip::types[i].description);
		}
		innerbox->Add(l,0,num*YSEP);
		
		innerbox->Add(new Gui::Label(format_money(m_station->GetPrice(type))), 200, num*YSEP);

		innerbox->Add(new Gui::Label(format_money(REMOVAL_VALUE_PERCENT * m_station->GetPrice(type) / 100)),
				275, num*YSEP);
		
		innerbox->Add(new Gui::Label(stringf(Lang::NUMBER_TONNES, formatarg("mass", Equip::types[i].mass))), 360, num*YSEP);

		ButtonPair pair;
		pair.type = type;
		
		pair.add = new Gui::SolidButton();
		pair.add->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationShipEquipmentForm::FitItem), type));
		innerbox->Add(pair.add, 400, num*YSEP);

		pair.remove = new Gui::SolidButton();
		pair.remove->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationShipEquipmentForm::RemoveItem), type));
		innerbox->Add(pair.remove, 420, num*YSEP);

		m_buttons.push_back(pair);

		num++;
	}

	portal->Add(innerbox);

	Gui::Fixed *heading = new Gui::Fixed(470, Gui::Screen::GetFontHeight());
	const float *col = Gui::Theme::Colors::tableHeading;
	heading->Add((new Gui::Label(Lang::ITEM))->Color(col), 0, 0);
	heading->Add((new Gui::Label(Lang::PRICE_TO_FIT))->Color(col), 200, 0);
	heading->Add((new Gui::Label(Lang::PRICE_TO_REMOVE))->Color(col), 275, 0);
	heading->Add((new Gui::Label(Lang::WT))->Color(col), 360, 0);
	heading->Add((new Gui::Label(Lang::FIT))->Color(col), 400, 0);
	heading->Add((new Gui::Label(Lang::REMOVE))->Color(col), 420, 0);
	outerbox->PackEnd(heading);

	Gui::HBox *body = new Gui::HBox();
	body->PackEnd(portal);
	body->PackEnd(scroll);
	outerbox->PackEnd(body);

	outerbox->SetSpacing(YSEP-Gui::Screen::GetFontHeight());
	Add(outerbox, 0, 0);

}

void StationShipEquipmentForm::ShowAll()
{
	FaceForm::ShowAll();
	RecalcButtonVisibility();
}

void StationShipEquipmentForm::RecalcButtonVisibility()
{
	for (std::list<ButtonPair>::iterator i = m_buttons.begin(); i != m_buttons.end(); i++) {
		Equip::Slot slot = Equip::types[(*i).type].slot;

		if (Pi::player->m_equipment.FreeSpace(slot) && m_station->GetStock((*i).type))
			(*i).add->Show();
		else
			(*i).add->Hide();

		if (Pi::player->m_equipment.Count(slot, (*i).type))
			(*i).remove->Show();
		else
			(*i).remove->Hide();
	}
}

void StationShipEquipmentForm::FitItem(Equip::Type t)
{
	Equip::Slot slot = Equip::types[t].slot;

	const shipstats_t *stats = Pi::player->CalcStats();
	int freespace = Pi::player->m_equipment.FreeSpace(slot);
	
	if (Pi::player->GetMoney() < m_station->GetPrice(t)) {
		Pi::cpan->MsgLog()->Message("", Lang::YOU_NOT_ENOUGH_MONEY);
		return;
	}

	if (!freespace || stats->free_capacity < Equip::types[t].mass) {
		Pi::cpan->MsgLog()->Message("", Lang::NO_SPACE_ON_SHIP);
		return;
	}

	if (freespace > 1 && slot == Equip::SLOT_LASER) {
		/* you have a choice of mount points for lasers */
		m_formController->ActivateForm(new PickLaserMountForm(m_formController, this, t, true));
		return;
	}

	FitItemForce(t);
}
	
void StationShipEquipmentForm::RemoveItem(Equip::Type t) {
	Equip::Slot slot = Equip::types[t].slot;

	int num = Pi::player->m_equipment.Count(slot, t);
	if (!num)
		return;

	if (num > 1 && slot == Equip::SLOT_LASER) {
		/* you have a choice of mount points for lasers */
		m_formController->ActivateForm(new PickLaserMountForm(m_formController, this, t, false));
		return;
	}

	RemoveItemForce(t);
}

void StationShipEquipmentForm::FitItemForce(Equip::Type t, int pos) {
	if (pos < 0)
		Pi::player->m_equipment.Add(t);
	else
		Pi::player->m_equipment.Set(Equip::types[t].slot, pos, t);

	Pi::player->UpdateMass();
	Pi::player->SetMoney(Pi::player->GetMoney() - m_station->GetPrice(t));
	Pi::cpan->MsgLog()->Message("", Lang::FITTING+std::string(Equip::types[t].name));

	RecalcButtonVisibility();
}

void StationShipEquipmentForm::RemoveItemForce(Equip::Type t, int pos) {
	if (pos < 0)
		Pi::player->m_equipment.Remove(t, 1);
	else
		Pi::player->m_equipment.Set(Equip::types[t].slot, pos, Equip::NONE);

	Pi::player->UpdateMass();
	Pi::player->SetMoney(Pi::player->GetMoney() + m_station->GetPrice(t) * REMOVAL_VALUE_PERCENT / 100);
	m_station->AddEquipmentStock(t, 1);
	Pi::cpan->MsgLog()->Message("", Lang::REMOVING+std::string(Equip::types[t].name));

	RecalcButtonVisibility();
}



PickLaserMountForm::PickLaserMountForm(FormController *controller, StationShipEquipmentForm *equipForm, Equip::Type equipType, bool doFit) :
	FaceForm(controller),
	m_equipForm(equipForm),
	m_equipType(equipType),
	m_doFit(doFit)
{
	Gui::VBox *layoutBox = new Gui::VBox();
	layoutBox->SetSpacing(10.0f);

	if (m_doFit)
		layoutBox->PackEnd(new Gui::Label(Lang::FIT_TO_WHICH_MOUNT));
	else
		layoutBox->PackEnd(new Gui::Label(Lang::REMOVE_FROM_WHICH_MOUNT));

	Equip::Slot slot = Equip::types[m_equipType].slot;

	for (int i=0; i<ShipType::GUNMOUNT_MAX; i++) {
		if (m_doFit && (Pi::player->m_equipment.Get(slot, i) != Equip::NONE)) continue;
		if ((!m_doFit) && (Pi::player->m_equipment.Get(slot, i) != m_equipType)) continue;

		Gui::HBox *buttonBox = new Gui::HBox();
		buttonBox->SetSpacing(5.0f);

		Gui::Button *b = new Gui::SolidButton();
		b->onClick.connect(sigc::bind(sigc::mem_fun(this, &PickLaserMountForm::PickMount), i));
		buttonBox->PackEnd(b);
		buttonBox->PackEnd(new Gui::Label(ShipType::gunmountNames[i]));

		layoutBox->PackEnd(buttonBox);
	}

	Add(layoutBox, 0, 100);
}

void PickLaserMountForm::PickMount(int i)
{
	if (m_doFit)
		m_equipForm->FitItemForce(m_equipType, i);
	else
		m_equipForm->RemoveItemForce(m_equipType, i);

	m_formController->CloseForm();
}
