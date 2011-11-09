#include "StationShipViewForm.h"
#include "Pi.h"
#include "Player.h"
#include "ShipSpinnerWidget.h"
#include "ShipCpanel.h"
#include "FormController.h"
#include "Lang.h"
#include "StringF.h"

StationShipViewForm::StationShipViewForm(FormController *controller, int marketIndex) :
	BlankForm(controller),
	m_marketIndex(marketIndex)
{
	m_station = Pi::player->GetDockedWith();

	m_flavour = m_station->GetShipsOnSale()[marketIndex];

	const ShipType &type = ShipType::types[m_flavour.type];

	SetTitle(stringf(Lang::SOMEWHERE_SHIP_MARKET, formatarg("station", m_station->GetLabel())));

	Add(new ShipSpinnerWidget(m_flavour, 400, 400), 0, 0);


	Gui::VBox *layoutBox = new Gui::VBox();
	layoutBox->SetSpacing(10.0f);
	Add(layoutBox, 420, 0);

	Gui::HBox *statsBox = new Gui::HBox();
	statsBox->SetSpacing(20.0f);
	layoutBox->PackEnd(statsBox);

	Gui::VBox *labelBox = new Gui::VBox();
	labelBox->PackEnd(new Gui::Label(Lang::SHIP_TYPE));
	labelBox->PackEnd(new Gui::Label(Lang::PRICE));
	labelBox->PackEnd(new Gui::Label(Lang::PART_EX));
	labelBox->PackEnd(new Gui::Label(Lang::REGISTRATION_ID));
	labelBox->PackEnd(new Gui::Label(" "));
	labelBox->PackEnd(new Gui::Label(Lang::WEIGHT_EMPTY));
	labelBox->PackEnd(new Gui::Label(Lang::WEIGHT_FULLY_LADEN));
	labelBox->PackEnd(new Gui::Label(Lang::CAPACITY));
	labelBox->PackEnd(new Gui::Label(" "));
	labelBox->PackEnd(new Gui::Label(Lang::FORWARD_ACCEL_EMPTY));
	labelBox->PackEnd(new Gui::Label(Lang::FORWARD_ACCEL_LADEN));
	labelBox->PackEnd(new Gui::Label(Lang::REVERSE_ACCEL_EMPTY));
	labelBox->PackEnd(new Gui::Label(Lang::REVERSE_ACCEL_LADEN));
	labelBox->PackEnd(new Gui::Label(" "));
	labelBox->PackEnd(new Gui::Label(Lang::HYPERDRIVE_FITTED));
	statsBox->PackEnd(labelBox);

	float forward_accel_empty = type.linThrust[ShipType::THRUSTER_FORWARD] / (-9.81f*1000.0f*(type.hullMass));
	float forward_accel_laden = type.linThrust[ShipType::THRUSTER_FORWARD] / (-9.81f*1000.0f*(type.hullMass+type.capacity));
	float reverse_accel_empty = -type.linThrust[ShipType::THRUSTER_REVERSE] / (-9.81f*1000.0f*(type.hullMass));
	float reverse_accel_laden = -type.linThrust[ShipType::THRUSTER_REVERSE] / (-9.81f*1000.0f*(type.hullMass+type.capacity));

	Gui::VBox *dataBox = new Gui::VBox();
	dataBox->PackEnd(new Gui::Label(type.name));
	dataBox->PackEnd(new Gui::Label(format_money(m_flavour.price)));
	dataBox->PackEnd(new Gui::Label(format_money(m_flavour.price - Pi::player->GetFlavour()->price)));
	dataBox->PackEnd(new Gui::Label(m_flavour.regid));
	dataBox->PackEnd(new Gui::Label(" "));
	dataBox->PackEnd(new Gui::Label(stringf(Lang::NUMBER_TONNES, formatarg("mass", type.hullMass))));
	dataBox->PackEnd(new Gui::Label(stringf(Lang::NUMBER_TONNES, formatarg("mass", type.hullMass + type.capacity))));
	dataBox->PackEnd(new Gui::Label(stringf( Lang::NUMBER_TONNES, formatarg("mass", type.capacity))));
	dataBox->PackEnd(new Gui::Label(" "));
	dataBox->PackEnd(new Gui::Label(stringf(Lang::NUMBER_G, formatarg("acceleration", forward_accel_empty))));
	dataBox->PackEnd(new Gui::Label(stringf(Lang::NUMBER_G, formatarg("acceleration", forward_accel_laden))));
	dataBox->PackEnd(new Gui::Label(stringf(Lang::NUMBER_G, formatarg("acceleration", reverse_accel_empty))));
	dataBox->PackEnd(new Gui::Label(stringf(Lang::NUMBER_G, formatarg("acceleration", reverse_accel_laden))));
	dataBox->PackEnd(new Gui::Label(" "));
	dataBox->PackEnd(new Gui::Label(Equip::types[type.hyperdrive].name));
	statsBox->PackEnd(dataBox);


	Gui::HBox *row = new Gui::HBox();
	row->SetSpacing(10.0f);

	int row_size = 5, pos = 0;
	for (int drivetype = Equip::DRIVE_CLASS1; drivetype <= Equip::DRIVE_CLASS9; drivetype++) {
		if (type.capacity < Equip::types[drivetype].mass)
			break;

		int hyperclass = Equip::types[drivetype].pval;
		// for the sake of hyperspace range, we count ships mass as 60% of original.
		float range = Pi::CalcHyperspaceRange(hyperclass, type.hullMass + type.capacity);

		Gui::VBox *cell = new Gui::VBox();
		row->PackEnd(cell);

		cell->PackEnd(new Gui::Label(stringf(Lang::CLASS_NUMBER, formatarg("class", hyperclass))));
		if (type.capacity < Equip::types[drivetype].mass)
			cell->PackEnd(new Gui::Label("---"));
		else
			cell->PackEnd(new Gui::Label(stringf(Lang::NUMBER_LY, formatarg("distance", range))));

		if (++pos == row_size) {
			layoutBox->PackEnd(row);

			row = new Gui::HBox();
			row->SetSpacing(15.0f);

			pos = 0;
		}
	}

	if (pos > 0)
		layoutBox->PackEnd(row);


	Gui::HBox *buttonBox = new Gui::HBox();
	buttonBox->SetSpacing(5.0f);

	Gui::SolidButton *b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &StationShipViewForm::BuyShip));
	buttonBox->PackEnd(b);
	buttonBox->PackEnd(new Gui::Label(Lang::BUY_THIS_SHIP));
	Add(buttonBox, 650, 30);
}

void StationShipViewForm::BuyShip()
{
	Sint64 cost = m_flavour.price - Pi::player->GetFlavour()->price;
	if (Pi::player->GetMoney() < cost) {
		Pi::cpan->MsgLog()->Message("", Lang::YOU_NOT_ENOUGH_MONEY);
		return;
	}

	ShipFlavour old = *(Pi::player->GetFlavour());

	Pi::player->SetMoney(Pi::player->GetMoney() - cost);
	Pi::player->ResetFlavour(&m_flavour);
	Pi::player->m_equipment.Set(Equip::SLOT_ENGINE, 0, ShipType::types[m_flavour.type].hyperdrive);
	Pi::player->UpdateMass();

	m_station->ReplaceShipOnSale(m_marketIndex, &old);

    Pi::cpan->MsgLog()->Message("", Lang::THANKS_AND_REMEMBER_TO_BUY_FUEL);

    m_formController->CloseForm();
}
