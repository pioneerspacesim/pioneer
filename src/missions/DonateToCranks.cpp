#include "../libs.h"
#include "DonateToCranks.h"
#include "../Serializer.h"
#include "../Pi.h"
#include "../Player.h"
#include "../ShipCpanel.h"

#define CAUSE_MAX 3
static const char *religion_blurbs1[CAUSE_MAX] = {
	"DONATE! The Church of The Celestial Flying Spaghetti Monster needs "
		"YOUR money to spread the word of god.",
	"DONATE. The Guardians of the Free Spirit humbly request your charity "
		"to support our monasteries.",
	"FEELING GENEROUS? War Orphan's Support needs your help to keep up "
		"its essential work."

};
static const char *religion_blurbs2[CAUSE_MAX] = {
	"Please select an amount to donate to the Church of the "
			"Celestial Flying Spaghetti Monster.\n",
	"Peace be with you, brother. Please select an amount to donate "
	"to the Guardians of the Free Spirit.\n",
	"Please select an amount to donate to War Orphan's Support, "
	"and end the suffering of children all over the galaxy.\n"
};

void DonateToCranks::Randomize()
{
	m_cause = Pi::rng.Int32(CAUSE_MAX);
}

std::string DonateToCranks::GetBulletinBoardText()
{
	return religion_blurbs1[m_cause];
}

void DonateToCranks::StartChat(GenericChatForm *form)
{
	sigc::slot<void,GenericChatForm*,int> slot = sigc::mem_fun(this, &DonateToCranks::FormResponse);
	form->Message(religion_blurbs2[m_cause]);
	form->AddOption(slot, "$1", 1);
	form->AddOption(slot, "$10", 2);
	form->AddOption(slot, "$100", 3);
	form->AddOption(slot, "$1000", 4);
	form->AddOption(slot, "$10000", 5);
	form->AddOption(slot, "$100000", 6);
	form->AddOption(slot, "Hang up.", 0);
}

void DonateToCranks::FormResponse(GenericChatForm *form, int resp)
{
	if (resp==0) form->Close();
	int amount;
	switch (resp) {
		case 1: amount = 100; break;
		case 2: amount = 1000; break;
		case 3: amount = 10000; break;
		case 4: amount = 100000; break;
		case 5: amount = 1000000; break;
		case 6: amount = 10000000; break;
		default: return;
	}
	if (Pi::player->GetMoney() < amount) {
		Pi::cpan->MsgLog()->Message("", "You do not have enough money.");
	} else {
		Pi::player->SetMoney(Pi::player->GetMoney() - amount);
		if (amount > 1000000)
			Pi::cpan->MsgLog()->Message("", "Wow! That was very generous.");
		else
			Pi::cpan->MsgLog()->Message("", "Thankyou. All donations are welcome.");
		form->UpdateBaseDisplay();
	}
}

void DonateToCranks::_Load()
{
	using namespace Serializer::Read;
	m_cause = rd_int();
}

void DonateToCranks::_Save()
{
	using namespace Serializer::Write;
	wr_int(m_cause);
}

