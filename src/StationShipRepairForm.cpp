#include "StationShipRepairForm.h"
#include "Pi.h"
#include "Player.h"
#include "ShipCpanel.h"

StationShipRepairForm::StationShipRepairForm(FormController *controller) : FaceForm(controller)
{
	m_station = Pi::player->GetDockedWith();

	SetTitle(stringf(256, "%s ship repairs", m_station->GetLabel().c_str()));

	m_working = new Gui::Label("Your ship is in perfect working condition.");
	Add(m_working, 0, 0);

	m_tableBox = new Gui::HBox();
	m_tableBox->SetSpacing(15.0f);
	Add(m_tableBox, 0, 0);

	Gui::VBox *columnBox = new Gui::VBox();
	m_tableBox->PackEnd(columnBox);

	Gui::HBox *buttonBox = new Gui::HBox();
	buttonBox->SetSpacing(5.0f);
	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationShipRepairForm::RepairHull), 1.0f));
	buttonBox->PackEnd(b);
	buttonBox->PackEnd(new Gui::Label("Repair 1.0% of hull damage"));
	columnBox->PackEnd(buttonBox);

	buttonBox = new Gui::HBox();
	buttonBox->SetSpacing(5.0f);
	b = new Gui::SolidButton();
	b->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationShipRepairForm::RepairHull), 100.0f));
	buttonBox->PackEnd(b);
	m_repairAllDesc = new Gui::Label("");
	buttonBox->PackEnd(m_repairAllDesc);
	columnBox->PackEnd(buttonBox);

	columnBox = new Gui::VBox();
	m_tableBox->PackEnd(columnBox);

	m_repairOneCost = new Gui::Label("");
	columnBox->PackEnd(m_repairOneCost);
	m_repairAllCost = new Gui::Label("");
	columnBox->PackEnd(m_repairAllCost);

	UpdateLabels();
}

void StationShipRepairForm::ShowAll()
{
	FaceForm::ShowAll();
	UpdateLabels();
}

int StationShipRepairForm::GetRepairCost(float percent)
{
	return int(Pi::player->GetFlavour()->price * 0.001 * percent);
}

void StationShipRepairForm::RepairHull(float percent)
{
	int cost = GetRepairCost(percent);
	if (Pi::player->GetMoney() < cost) {
		Pi::cpan->MsgLog()->Message("", "You do not have enough money");
	} else {
		Pi::player->SetMoney(Pi::player->GetMoney() - cost);
		Pi::player->SetPercentHull(Pi::player->GetPercentHull() + percent);
		UpdateLabels();
	}
}

void StationShipRepairForm::UpdateLabels()
{
	float hullPercent = Pi::player->GetPercentHull();

	if (hullPercent >= 100.0f)
	{
		m_working->Show();
		m_tableBox->Hide();
	}

	else {
		m_repairAllDesc->SetText(stringf(128, "Repair all hull damage (%.1f%%)", 100.0f - hullPercent));
		m_repairOneCost->SetText(format_money(GetRepairCost(1.0f)));
		m_repairAllCost->SetText(format_money(GetRepairCost(100.f - hullPercent)));

		m_working->Hide();
		m_tableBox->Show();
	}
}

