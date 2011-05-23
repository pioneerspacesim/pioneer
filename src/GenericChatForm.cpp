#include "libs.h"
#include "Pi.h"
#include "Player.h"
#include "GenericChatForm.h"
#include "SpaceStation.h"
#include "FaceVideoLink.h"

void GenericChatForm::AddVideoWidget()
{
	if (!m_videoLink) SetFace(0, Pi::player->GetDockedWith()->GetSBody()->seed);
	//Add(new FaceVideoLink(295,285, 0, Pi::player->GetDockedWith()->GetSBody()->seed), 5, 40);
	//Add(new DeadVideoLink(295,285), 5, 40);
	//AddFaceWidget();
}


void GenericChatForm::Close()
{
	//GetParent()->RemoveChild(this);
	//GetParent()->ShowChildren();
	//GetParent()->SetTransparency(false);
	onClose.emit(this);
	//static_cast<GenericChatForm*>(GetParent())->UpdateBaseDisplay();
	//delete this;
}

void GenericChatForm::UpdateBaseDisplay()
{
	if (m_money) {
		char buf[64];
		m_money->SetText(format_money(Pi::player->GetMoney()));

		const shipstats_t *stats = Pi::player->CalcStats();
		snprintf(buf, sizeof(buf), "%dt", stats->used_capacity - stats->used_cargo);
		m_equipmentMass->SetText(buf);
		
		snprintf(buf, sizeof(buf), "%dt", stats->used_cargo);
		m_cargoSpaceUsed->SetText(buf);
		
		snprintf(buf, sizeof(buf), "%dt", stats->free_capacity);
		m_cargoSpaceFree->SetText(buf);
	}
}

void GenericChatForm::AddBaseDisplay()
{
	const float YSEP = floor(Gui::Screen::GetFontHeight() * 1.5f);
	const float ystart = 350.0f;

	Add(new Gui::Label("#007Cash"), 10, ystart);
	Add(new Gui::Label("#007Legal status"), 10, ystart + 2*YSEP);
	Add(new Gui::Label("#007Used"), 140, ystart+4*YSEP);
	Add(new Gui::Label("#007Free"), 220, ystart+4*YSEP);
	Add(new Gui::Label("#007Cargo space"), 10, ystart+5*YSEP);
	Add(new Gui::Label("#007Equipment"), 10, ystart+6*YSEP);

	m_money = new Gui::Label("");
	Add(m_money, 220, ystart);

	m_cargoSpaceUsed = new Gui::Label("");
	Add(m_cargoSpaceUsed, 140, ystart + 5*YSEP);
	
	m_cargoSpaceFree = new Gui::Label("");
	Add(m_cargoSpaceFree, 220, ystart + 5*YSEP);
	
	m_equipmentMass = new Gui::Label("");
	Add(m_equipmentMass, 140, ystart + 6*YSEP);
	
	m_legalstatus = new Gui::Label("Clean");
	Add(m_legalstatus, 220, ystart + 2*YSEP);

	UpdateBaseDisplay();
}

void GenericChatForm::OpenChildChatForm(GenericChatForm *form)
{
//	HideChildren();
//	//SetTransparency(true);
//	
//	Gui::Fixed *f = new Gui::Fixed(470, 400);
//	Add(f, 320, 40);
//	f->SetTransparency(false);
//	f->Add(form, 0, 0);
//	f->ShowAll();
	HideChildren();
	SetTransparency(true);
	Add(form, 0, 0);
	form->ShowAll();
	form->onClose.connect(sigc::mem_fun(this, &GenericChatForm::OnCloseChildChatForm));
}

void GenericChatForm::OnCloseChildChatForm(GenericChatForm *form)
{
	RemoveChild(form);
	SetTransparency(false);
	delete form;
	ShowAll();
}

GenericChatForm::GenericChatForm(): Gui::Fixed(float(Gui::Screen::GetWidth()), float(Gui::Screen::GetHeight()-64))
{
	ReInit();
}

void GenericChatForm::ReInit()
{
	DeleteAllChildren();
	m_legalstatus = 0;
	m_money = 0;
	m_cargoSpaceUsed = 0;
	m_cargoSpaceFree = 0;
	m_equipmentMass = 0;
	m_videoLink = 0;

	m_titleLabel = new Gui::Label("");
	Add(m_titleLabel, 10, 10);

	SetTransparency(false);
	m_chatRegion = new Gui::Fixed(470, 400);
	Add(m_chatRegion, 320, 40);

	m_msgregion = new Gui::VBox();
	m_optregion = new Gui::VBox();
	m_msgregion->SetSizeRequest(470, 150);
       	m_msgregion->SetSpacing(5.0f);
       	m_optregion->SetSpacing(5.0f);
	m_chatRegion->Add(m_msgregion, 0, 0);
	m_chatRegion->Add(m_optregion, 0, 160);
	m_msgregion->Show();
	m_optregion->Show();
	Clear();
}

void GenericChatForm::SetTitle(const char *title)
{
	m_titleLabel->SetText(title);
}

void GenericChatForm::SetFace(Uint32 flags, Uint32 seed)
{
	if (m_videoLink) {
		RemoveChild(m_videoLink);
		delete m_videoLink;
	}
	m_videoLink = new FaceVideoLink(295, 285, flags, seed);
	Add(m_videoLink, 5, 40);
}

void GenericChatForm::Clear()
{
	m_msgregion->DeleteAllChildren();
	m_optregion->DeleteAllChildren();
	hasOpts = false;
}

void GenericChatForm::SetMessage(const char *msg)
{
	m_msgregion->PackEnd(new Gui::Label(msg));
	ShowAll();
}

void GenericChatForm::AddOption(sigc::slot<void,GenericChatForm*,int> slot, const char *text, int val)
{
	if (!hasOpts) {
		hasOpts = true;
		m_optregion->PackStart(new Gui::Label("Suggested responses:"));
	}
	Gui::Box *box = new Gui::HBox();
	Gui::Button *b = new Gui::SolidButton();
//	b->onClick.connect(sigc::bind(sigc::mem_fun(m, &Mission::FormResponse), this, val));
	b->onClick.connect(sigc::bind(slot, this, val));
	box->SetSpacing(5.0f);
	box->PackEnd(b);
	box->PackEnd(new Gui::Label(text));
	m_optregion->PackEnd(box);
	ShowAll();
}

