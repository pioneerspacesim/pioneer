// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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
	m_station(Pi::player->GetDockedWith()),
	m_marketIndex(marketIndex),
	m_sos(m_station->GetShipsOnSale()[marketIndex])
{
	const ShipType &type = ShipType::types[m_sos.id];

	SetTitle(stringf(Lang::SOMEWHERE_SHIP_MARKET, formatarg("station", m_station->GetLabel())));

	SceneGraph::Model *model = Pi::FindModel(type.modelName);
	Add(new ShipSpinnerWidget(model, m_sos.skin, 400, 400), 0, 0);


	Gui::VBox *layoutBox = new Gui::VBox();
	layoutBox->SetSpacing(10.0f);
	Add(layoutBox, 420, 0);

	Gui::HBox *statsBox = new Gui::HBox();
	statsBox->SetSpacing(20.0f);
	layoutBox->PackEnd(statsBox);

	Gui::VBox *labelBox = new Gui::VBox();
	labelBox->PackEnd(new Gui::Label(Lang::SHIP_TYPE));
	labelBox->PackEnd(new Gui::Label(Lang::PRICE));
	labelBox->PackEnd(new Gui::Label(Lang::REGISTRATION_ID));
	labelBox->PackEnd(new Gui::Label(" "));
	labelBox->PackEnd(new Gui::Label(Lang::WEIGHT_EMPTY));
	labelBox->PackEnd(new Gui::Label(Lang::CAPACITY));
	labelBox->PackEnd(new Gui::Label(Lang::FUEL_WEIGHT));
	labelBox->PackEnd(new Gui::Label(Lang::WEIGHT_FULLY_LADEN));
	labelBox->PackEnd(new Gui::Label(" "));
	labelBox->PackEnd(new Gui::Label(Lang::FORWARD_ACCEL_EMPTY));
	labelBox->PackEnd(new Gui::Label(Lang::FORWARD_ACCEL_LADEN));
	labelBox->PackEnd(new Gui::Label(Lang::REVERSE_ACCEL_EMPTY));
	labelBox->PackEnd(new Gui::Label(Lang::REVERSE_ACCEL_LADEN));
	labelBox->PackEnd(new Gui::Label(" "));
	labelBox->PackEnd(new Gui::Label(Lang::HYPERDRIVE_FITTED));
	statsBox->PackEnd(labelBox);

	float forward_accel_empty = type.linThrust[ShipType::THRUSTER_FORWARD] / (-9.81f*1000.0f*(type.hullMass+type.fuelTankMass));
	float forward_accel_laden = type.linThrust[ShipType::THRUSTER_FORWARD] / (-9.81f*1000.0f*(type.hullMass+type.capacity+type.fuelTankMass));
	float reverse_accel_empty = -type.linThrust[ShipType::THRUSTER_REVERSE] / (-9.81f*1000.0f*(type.hullMass+type.fuelTankMass));
	float reverse_accel_laden = -type.linThrust[ShipType::THRUSTER_REVERSE] / (-9.81f*1000.0f*(type.hullMass+type.capacity+type.fuelTankMass));
	
	const int playerShipPrice = Pi::player->GetShipType()->baseprice >> 1;
	Gui::VBox *dataBox = new Gui::VBox();
	dataBox->PackEnd(new Gui::Label(type.name));
	dataBox->PackEnd(new Gui::Label(format_money(type.baseprice - playerShipPrice)));
	dataBox->PackEnd(new Gui::Label(m_sos.regId));
	dataBox->PackEnd(new Gui::Label(" "));
	dataBox->PackEnd(new Gui::Label(stringf(Lang::NUMBER_TONNES, formatarg("mass", type.hullMass))));
	dataBox->PackEnd(new Gui::Label(stringf(Lang::NUMBER_TONNES, formatarg("mass", type.capacity))));
	dataBox->PackEnd(new Gui::Label(stringf(Lang::NUMBER_TONNES, formatarg("mass", type.fuelTankMass))));
	dataBox->PackEnd(new Gui::Label(stringf(Lang::NUMBER_TONNES, formatarg("mass", type.hullMass + type.capacity + type.fuelTankMass))));
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
		float range = Pi::CalcHyperspaceRangeMax(hyperclass, type.hullMass + type.capacity + type.fuelTankMass);

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
	const int playerShipPrice = Pi::player->GetShipType()->baseprice >> 1;
	Sint64 cost = ShipType::types[m_sos.id].baseprice - playerShipPrice;
	if (Pi::player->GetMoney() < cost) {
		Pi::cpan->MsgLog()->Message("", Lang::YOU_NOT_ENOUGH_MONEY);
		return;
	}

	ShipOnSale old(Pi::player->GetShipType()->id, Pi::player->GetLabel(), Pi::player->GetSkin());

	Pi::player->SetMoney(Pi::player->GetMoney() - cost);
	Pi::player->SetFuel(1.0); // new ship should be always fueled 100%
	Pi::player->SetShipType(m_sos.id);
	Pi::player->SetLabel(m_sos.regId);
	Pi::player->SetSkin(m_sos.skin);
	Pi::player->m_equipment.Set(Equip::SLOT_ENGINE, 0, ShipType::types[m_sos.id].hyperdrive);
	Pi::player->UpdateStats();

	m_station->ReplaceShipOnSale(m_marketIndex, old);

	const int dockingPort = m_station->GetMyDockingPort( Pi::player );
	const SpaceStationType::SBayGroup* pBayGroup = m_station->GetStationType()->FindGroupByBay( dockingPort );

	const Aabb &bbox = Pi::player->GetAabb();
	const double bboxRad = bbox.GetRadius();

	if( (pBayGroup->minShipSize > bboxRad) || (bboxRad > pBayGroup->maxShipSize) ) {
		// find another free docking port that we can fit into.
		const int freeDockingPort = m_station->GetFreeDockingPort(Pi::player);
		if(freeDockingPort >= 0) {
			// change docking port we're docked at to the free one we found
			m_station->SwapDockedShipsPort(dockingPort, freeDockingPort);
		}
	}

    Pi::cpan->MsgLog()->Message("", Lang::THANKS_AND_REMEMBER_TO_BUY_FUEL);

    m_formController->CloseForm();
}
