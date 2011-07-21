#include "StationShipyardForm.h"
#include "Pi.h"
#include "Player.h"
#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "StationShipEquipmentForm.h"
#include "StationShipMarketForm.h"
#include "StationShipRepairForm.h"

StationShipyardForm::StationShipyardForm(FormController *controller) : FaceForm(controller)
{
	SetTitle(stringf(256, "%s shipyard", Pi::player->GetDockedWith()->GetLabel().c_str()));

	Gui::SolidButton *b = new Gui::SolidButton();
	b->SetShortcut(SDLK_1, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationShipyardForm::EquipmentMarket));
	Add(b, 30, 140);
	Gui::Label *l = new Gui::Label("Ship equipment");
	Add(l, 55, 140);

	b = new Gui::SolidButton();
	b->SetShortcut(SDLK_2, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationShipyardForm::Repairs));
	Add(b, 30, 200);
	l = new Gui::Label("Repairs and servicing");
	Add(l, 55, 200);

	b = new Gui::SolidButton();
	b->SetShortcut(SDLK_3, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationShipyardForm::ShipMarket));
	Add(b, 30, 260);
	l = new Gui::Label("New and reconditioned ships");
	Add(l, 55, 260);
}

void StationShipyardForm::EquipmentMarket()
{
    m_formController->ActivateForm(new StationShipEquipmentForm(m_formController));
}

void StationShipyardForm::Repairs()
{
    m_formController->ActivateForm(new StationShipRepairForm(m_formController));
}

void StationShipyardForm::ShipMarket()
{
    m_formController->ActivateForm(new StationShipMarketForm(m_formController));
}
