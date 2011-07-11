#include "StationShipViewForm.h"
#include "Pi.h"
#include "Player.h"

StationShipViewForm::StationShipViewForm(FormController *controller, ShipFlavour flavour) :
	BlankForm(controller),
	m_flavour(flavour)
{
	m_station = Pi::player->GetDockedWith();

	SetTitle(stringf(256, "%s ship market", m_station->GetLabel().c_str()));

	const ShipType &type = ShipType::types[m_flavour.type];

	Gui::VBox *labelBox = new Gui::VBox();
	labelBox->PackEnd(new Gui::Label("Ship type"));
	labelBox->PackEnd(new Gui::Label("Price"));
	labelBox->PackEnd(new Gui::Label("Registration id"));
	labelBox->PackEnd(new Gui::Label(" "));
	labelBox->PackEnd(new Gui::Label("Weight empty"));
	labelBox->PackEnd(new Gui::Label("Weight fully loaded"));
	labelBox->PackEnd(new Gui::Label("Capacity"));
	labelBox->PackEnd(new Gui::Label(" "));
	labelBox->PackEnd(new Gui::Label("Forward accel (empty)"));
	labelBox->PackEnd(new Gui::Label("Forward accel (laden)"));
	labelBox->PackEnd(new Gui::Label("Reverse accel (empty)"));
	labelBox->PackEnd(new Gui::Label("Reverse accel (laden)"));
	labelBox->PackEnd(new Gui::Label(" "));
	labelBox->PackEnd(new Gui::Label("Hyperdrive fitted"));
	Add(labelBox, 420, 0);

	float forward_accel_empty = type.linThrust[ShipType::THRUSTER_FORWARD] / (-9.81f*1000.0f*(type.hullMass));
	float forward_accel_laden = type.linThrust[ShipType::THRUSTER_FORWARD] / (-9.81f*1000.0f*(type.hullMass+type.capacity));
	float reverse_accel_empty = -type.linThrust[ShipType::THRUSTER_REVERSE] / (-9.81f*1000.0f*(type.hullMass));
	float reverse_accel_laden = -type.linThrust[ShipType::THRUSTER_REVERSE] / (-9.81f*1000.0f*(type.hullMass+type.capacity));

	Gui::VBox *dataBox = new Gui::VBox();
	dataBox->PackEnd(new Gui::Label(type.name));
	dataBox->PackEnd(new Gui::Label(format_money(m_flavour.price)));
	dataBox->PackEnd(new Gui::Label(m_flavour.regid));
	dataBox->PackEnd(new Gui::Label(" "));
	dataBox->PackEnd(new Gui::Label(stringf(64, "%d t", type.hullMass)));
	dataBox->PackEnd(new Gui::Label(stringf(64, "%d t", type.hullMass + type.capacity)));
	dataBox->PackEnd(new Gui::Label(stringf(64, "%d t", type.capacity)));
	dataBox->PackEnd(new Gui::Label(" "));
	dataBox->PackEnd(new Gui::Label(stringf(64, "%.1f G", forward_accel_empty)));
	dataBox->PackEnd(new Gui::Label(stringf(64, "%.1f G", forward_accel_laden)));
	dataBox->PackEnd(new Gui::Label(stringf(64, "%.1f G", reverse_accel_empty)));
	dataBox->PackEnd(new Gui::Label(stringf(64, "%.1f G", reverse_accel_laden)));
	dataBox->PackEnd(new Gui::Label(" "));
	dataBox->PackEnd(new Gui::Label(EquipType::types[type.hyperdrive].name));
	Add(dataBox, 600, 0);
}
