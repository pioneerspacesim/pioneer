#include "StationServicesForm.h"
#include "Pi.h"
#include "Player.h"
#include "SpaceStation.h"
#include "WorldView.h"
#include "SpaceStationView.h"
#include "StationShipyardForm.h"
#include "StationCommodityMarketForm.h"
#include "StationBulletinBoardForm.h"
#include "StationPoliceForm.h"
#include "PiLang.h"

StationServicesForm::StationServicesForm(FormController *controller) : FaceForm(controller)
{
	SetTitle(stringf(256, "%s services", Pi::player->GetDockedWith()->GetLabel().c_str()));

	Gui::Label *l = new Gui::Label(PiLang::SPACESTATION_LONG_WELCOME_MESSAGE);
	Add(l,0,0);

	Gui::SolidButton *b = new Gui::SolidButton();
	b->SetShortcut(SDLK_1, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationServicesForm::RequestLaunch));
	Add(b, 30, 140);
	l = new Gui::Label(PiLang::REQUEST_LAUNCH);
	Add(l, 55, 140);

	b = new Gui::SolidButton();
	b->SetShortcut(SDLK_2, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationServicesForm::Shipyard));
	Add(b, 30, 200);
	l = new Gui::Label(PiLang::SHIPYARD);
	Add(l, 55, 200);

	b = new Gui::SolidButton();
	b->SetShortcut(SDLK_3, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationServicesForm::CommodityMarket));
	Add(b, 30, 260);
	l = new Gui::Label("Commodity market");
	Add(l, 55, 260);

	b = new Gui::SolidButton();
	b->SetShortcut(SDLK_4, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationServicesForm::BulletinBoard));
	Add(b, 30, 320);
	l = new Gui::Label("Bulletin board");
	Add(l, 55, 320);

	b = new Gui::SolidButton();
	b->SetShortcut(SDLK_5, KMOD_NONE);
	b->onClick.connect(sigc::mem_fun(this, &StationServicesForm::Police));
	Add(b, 30, 380);
	l = new Gui::Label(PiLang::CONTACT_LOCAL_POLICE);
	Add(l, 55, 380);
}

void StationServicesForm::RequestLaunch()
{
	Pi::worldView->OnClickBlastoff();
	Pi::SetView(Pi::worldView);
}

void StationServicesForm::Shipyard()
{
	m_formController->ActivateForm(new StationShipyardForm(m_formController));
}

void StationServicesForm::CommodityMarket()
{
	m_formController->ActivateForm(new StationCommodityMarketForm(m_formController));
}

void StationServicesForm::BulletinBoard()
{
	m_formController->ActivateForm(new StationBulletinBoardForm(m_formController));
}

void StationServicesForm::Police()
{
	m_formController->ActivateForm(new StationPoliceForm(m_formController));
}
