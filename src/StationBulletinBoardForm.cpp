#include "StationBulletinBoardForm.h"
#include "SpaceStation.h"
#include "Pi.h"
#include "Player.h"
#include "StationAdvertForm.h"
#include "FormController.h"
#include "Lang.h"

StationBulletinBoardForm::StationBulletinBoardForm(FormController *controller) : FaceForm(controller)
{
	m_station = Pi::player->GetDockedWith();

	SetTitle(m_station->GetLabel() + " " + Lang::BULLETIN_BOARD);

	Gui::VScrollPortal *scrollbox = new Gui::VScrollPortal(460);
	Gui::VScrollBar *scrollbar = new Gui::VScrollBar();
	scrollbar->SetAdjustment(&scrollbox->vscrollAdjust);

	m_advertbox = new Gui::Fixed();
	UpdateAdverts();
	m_advertbox->ShowAll();

	scrollbox->Add(m_advertbox);
	scrollbox->ShowAll();

	Add(scrollbox, 0, 0);
	Add(scrollbar, 460, 10);

	ShowAll();

	m_bbChangedConnection = m_station->onBulletinBoardChanged.connect(sigc::mem_fun(this, &StationBulletinBoardForm::OnBulletinBoardChanged));
	m_bbAdvertDeletedConnection = m_station->onBulletinBoardAdvertDeleted.connect(sigc::mem_fun(this, &StationBulletinBoardForm::OnBulletinBoardAdvertDeleted));
}

StationBulletinBoardForm::~StationBulletinBoardForm()
{
	m_bbChangedConnection.disconnect();
	m_bbAdvertDeletedConnection.disconnect();
}

void StationBulletinBoardForm::OnBulletinBoardChanged()
{
	UpdateAdverts();
	m_advertbox->ShowAll();
}

void StationBulletinBoardForm::OnBulletinBoardAdvertDeleted(const BBAdvert &ad)
{
	UpdateAdverts();
	m_advertbox->ShowAll();
}

void StationBulletinBoardForm::UpdateAdverts()
{
	m_advertbox->DeleteAllChildren();

	const float YSEP = floor(Gui::Screen::GetFontHeight() * 5);

	const std::list<const BBAdvert*> adverts = m_station->GetBBAdverts();

	m_advertbox->SetSizeRequest(450, adverts.size() * YSEP);

	float y = 0;
	for (std::list<const BBAdvert*>::const_iterator i = adverts.begin(); i != adverts.end(); i++) {
		Gui::SolidButton *b = new Gui::SolidButton();
		b->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationBulletinBoardForm::ActivateAdvertForm), *(*i)));
		m_advertbox->Add(b, 0, y);

		Gui::Label *l = new Gui::Label((*i)->description);
		m_advertbox->Add(l, 20, y);

		y += YSEP;
	}
}

void StationBulletinBoardForm::ActivateAdvertForm(const BBAdvert &ad)
{
	m_formController->ActivateForm(ad.builder(m_formController, m_station, ad));
}
