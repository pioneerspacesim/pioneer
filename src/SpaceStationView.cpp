#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "Pi.h"
#include "Player.h"
#include "utils.h"
#include "Lang.h"

#include "StationServicesForm.h"

SpaceStationView::SpaceStationView(): View()
{
	Gui::Label *l = new Gui::Label(Lang::COMMS_LINK);
	l->Color(1,.7,0);
	m_rightRegion2->Add(l, 10, 0);

	SetTransparency(false);

	m_title = new Gui::Label("");
	Add(m_title, 10, 10);


	m_statusBox = new Gui::Fixed(300, 300);
	Add(m_statusBox, 10, 350);

	const float YSEP = floor(Gui::Screen::GetFontHeight() * 1.5f);

	m_statusBox->Add(new Gui::Label(std::string("#007")+std::string(Lang::CASH)), 0, 0);
	m_statusBox->Add(new Gui::Label(std::string("#007")+std::string(Lang::LEGAL_STATUS)), 0, 2*YSEP);
	m_statusBox->Add(new Gui::Label(std::string("#007")+std::string(Lang::USED)), 130, 4*YSEP);
	m_statusBox->Add(new Gui::Label(std::string("#007")+std::string(Lang::FREE)), 210, 4*YSEP);
	m_statusBox->Add(new Gui::Label(std::string("#007")+std::string(Lang::CARGO_SPACE)), 0, 5*YSEP);
	m_statusBox->Add(new Gui::Label(std::string("#007")+std::string(Lang::SHIP_EQUIPMENT)), 0, 6*YSEP);
	m_statusBox->Add(new Gui::Label(std::string("#007")+std::string(Lang::CABINS)), 0, 7*YSEP);

	m_money = new Gui::Label("");
	m_statusBox->Add(m_money, 210, 0);

	m_cargoSpaceUsed = new Gui::Label("");
	m_statusBox->Add(m_cargoSpaceUsed, 130, 5*YSEP);
	
	m_cargoSpaceFree = new Gui::Label("");
	m_statusBox->Add(m_cargoSpaceFree, 210, 5*YSEP);
	
	m_equipmentMass = new Gui::Label("");
	m_statusBox->Add(m_equipmentMass, 130, 6*YSEP);
	
	m_cabinsUsed = new Gui::Label("");
	m_statusBox->Add(m_cabinsUsed, 130, 7*YSEP);
	
	m_cabinsFree = new Gui::Label("");
	m_statusBox->Add(m_cabinsFree, 210, 7*YSEP);
	
	m_legalstatus = new Gui::Label(Lang::CLEAN);
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
	m_backButtonBox->PackEnd(new Gui::Label(Lang::GO_BACK));


	m_videoLink = 0;
	
	m_undockConnection = Pi::player->onUndock.connect(sigc::mem_fun(m_formStack, &Gui::Stack::Clear));
}

SpaceStationView::~SpaceStationView()
{
	delete m_formController;
	Remove(m_formStack);
	delete m_formStack;
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

	snprintf(buf, sizeof(buf), "%d", Pi::player->m_equipment.Count(Equip::SLOT_CABIN, Equip::PASSENGER_CABIN));
	m_cabinsUsed->SetText(buf);
		
	snprintf(buf, sizeof(buf), "%d", Pi::player->m_equipment.Count(Equip::SLOT_CABIN, Equip::UNOCCUPIED_CABIN));
	m_cabinsFree->SetText(buf);

	if (m_formStack->Size() > 1)
		m_backButtonBox->Show();
	else
		m_backButtonBox->Hide();
	
	if (static_cast<Form*>(m_formStack->Top())->GetType() == Form::BLANK)
		m_statusBox->Hide();
	else
		m_statusBox->Show();
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

			if (!form->GetFaceSeed())
				form->SetFaceSeed(Pi::player->GetDockedWith()->GetSBody()->seed);

			if (!m_videoLink || form->GetFaceFlags() != m_videoLink->GetFlags() ||
				form->GetFaceSeed() != m_videoLink->GetSeed()) {
				if (m_videoLink) {
					Remove(m_videoLink);
					delete m_videoLink;
				}

				m_videoLink = new FaceVideoLink(295, 285, form->GetFaceFlags(), form->GetFaceSeed(),
					form->GetCharacterName(), form->GetCharacterTitle());
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
