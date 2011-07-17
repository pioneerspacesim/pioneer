#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "Pi.h"
#include "Player.h"
#include "WorldView.h"
#include "ShipFlavour.h"
#include "ShipCpanel.h"
#include "CommodityTradeWidget.h"
#include "GenericChatForm.h"
#include "LuaChatForm.h"
#include "PoliceChatForm.h"
#include "LmrModel.h"
#include "utils.h"

#if 0
////////////////////////////////////////////////////////////////////

class StationShipRepairsView: public GenericChatForm {
public:
	StationShipRepairsView();
	virtual ~StationShipRepairsView() {
	}
	virtual void ShowAll();
private:
	int GetCostOfFixingHull(float percent) {
		return int(Pi::player->GetFlavour()->price * 0.001 * percent);
	}

	void RepairHull(float percent) {
		int cost = GetCostOfFixingHull(percent);
		if (Pi::player->GetMoney() < cost) {
			Pi::cpan->MsgLog()->Message("", "You do not have enough money");
		} else {
			Pi::player->SetMoney(Pi::player->GetMoney() - cost);
			Pi::player->SetPercentHull(Pi::player->GetPercentHull() + percent);
			ShowAll();
		}
	}
//	sigc::connection m_onShipsForSaleChangedConnection;
};

StationShipRepairsView::StationShipRepairsView(): GenericChatForm()
{
}

void StationShipRepairsView::ShowAll()
{
	ReInit();

	SpaceStation *station = Pi::player->GetDockedWith();
	assert(station);
	SetTransparency(false);
	
	SetTitle(stringf(256, "%s Shipyard", station->GetLabel().c_str()).c_str());
	
	Gui::Button *b = new Gui::SolidButton();
	b->onClick.connect(sigc::mem_fun(this, &StationShipRepairsView::Close));
	Add(b,680,470);
	Add(new Gui::Label("Go back"), 700, 470);

	Gui::Fixed *fbox = new Gui::Fixed(470, 400);
	Add(fbox, 320, 40);

	const float YSEP = floor(Gui::Screen::GetFontHeight() * 1.5f);
	float ypos = YSEP;

	float hullPercent = Pi::player->GetPercentHull();
	if (hullPercent >= 100.0f) {
		fbox->Add(new Gui::Label("Your ship is in perfect working condition."), 0, ypos);
	} else {
		int costAll = GetCostOfFixingHull(100.0f - hullPercent);
		int cost1 = GetCostOfFixingHull(1.0f);
		if (cost1 < costAll) {
			fbox->Add(new Gui::Label("Repair 1.0% of hull damage"), 0, ypos);
			fbox->Add(new Gui::Label(format_money(cost1)), 350, ypos);
			
			Gui::SolidButton *sb = new Gui::SolidButton();
			sb->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationShipRepairsView::RepairHull), 1.0f));
			fbox->Add(sb, 430, ypos);
			ypos += YSEP;
		}
		fbox->Add(new Gui::Label(stringf(128, "Repair all hull damage (%.1f%%)", 100.0f-hullPercent)), 0, ypos);
		fbox->Add(new Gui::Label(format_money(costAll)), 350, ypos);
		
		Gui::SolidButton *sb = new Gui::SolidButton();
		sb->onClick.connect(sigc::bind(sigc::mem_fun(this, &StationShipRepairsView::RepairHull), 100.0f-hullPercent));
		fbox->Add(sb, 430, ypos);
	}

	const float *col = Gui::Theme::Colors::tableHeading;
	fbox->Add((new Gui::Label("Item"))->Color(col), 0, 0);
	fbox->Add((new Gui::Label("Price"))->Color(col), 350, 0);
	fbox->Add((new Gui::Label("Repair"))->Color(col), 430, 0);
	fbox->ShowAll();
	AddBaseDisplay();
	AddVideoWidget();

	Gui::Fixed::ShowAll();
}
#endif

#include "StationServicesForm.h"

SpaceStationView::SpaceStationView(): View()
{
	Gui::Label *l = new Gui::Label("Comms Link");
	l->Color(1,.7,0);
	m_rightRegion2->Add(l, 10, 0);

	SetTransparency(false);

	m_title = new Gui::Label("");
	Add(m_title, 10, 10);


	m_statusBox = new Gui::Fixed(300, 300);
	Add(m_statusBox, 10, 350);

	const float YSEP = floor(Gui::Screen::GetFontHeight() * 1.5f);

	m_statusBox->Add(new Gui::Label("#007Cash"), 0, 0);
	m_statusBox->Add(new Gui::Label("#007Legal status"), 0, 2*YSEP);
	m_statusBox->Add(new Gui::Label("#007Used"), 130, 4*YSEP);
	m_statusBox->Add(new Gui::Label("#007Free"), 210, 4*YSEP);
	m_statusBox->Add(new Gui::Label("#007Cargo space"), 0, 5*YSEP);
	m_statusBox->Add(new Gui::Label("#007Equipment"), 0, 6*YSEP);

	m_money = new Gui::Label("");
	m_statusBox->Add(m_money, 210, 0);

	m_cargoSpaceUsed = new Gui::Label("");
	m_statusBox->Add(m_cargoSpaceUsed, 130, 5*YSEP);
	
	m_cargoSpaceFree = new Gui::Label("");
	m_statusBox->Add(m_cargoSpaceFree, 210, 5*YSEP);
	
	m_equipmentMass = new Gui::Label("");
	m_statusBox->Add(m_equipmentMass, 130, 6*YSEP);
	
	m_legalstatus = new Gui::Label("Clean");
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
	m_backButtonBox->PackEnd(new Gui::Label("Go back"));


	m_videoLink = 0;
	
	m_undockConnection = Pi::player->onUndock.connect(sigc::mem_fun(m_formStack, &Gui::Stack::Clear));
}

SpaceStationView::~SpaceStationView()
{
	delete m_formController;
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

	if (m_formStack->Size() > 1)
		m_backButtonBox->Show();
	else
		m_backButtonBox->Hide();
	
	if (static_cast<Form*>(m_formStack->Top())->GetType() == Form::BLANK)
		m_statusBox->Hide();
	else
		m_statusBox->Show();
}

void SpaceStationView::Draw3D()
{
	onDraw3D.emit();
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

			if (form->GetFaceSeed() == -1UL)
				form->SetFaceSeed(Pi::player->GetDockedWith()->GetSBody()->seed);

			if (!m_videoLink || form->GetFaceFlags() != m_videoLink->GetFlags() || form->GetFaceSeed() != m_videoLink->GetSeed()) {
				if (m_videoLink) {
					Remove(m_videoLink);
					delete m_videoLink;
				}

				m_videoLink = new FaceVideoLink(295, 285, form->GetFaceFlags(), form->GetFaceSeed());
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
