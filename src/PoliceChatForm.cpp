#include "PoliceChatForm.h"
#include "Polit.h"
#include "Pi.h"
#include "Player.h"
#include "SpaceStation.h"
#include "ShipCpanel.h"

PoliceChatForm::PoliceChatForm(): GenericChatForm()
{
	AddBaseDisplay();
	AddVideoWidget();
	Sint64 crime, fine;
	Polit::GetCrime(&crime, &fine);

	SetTitle(stringf(256, "%s Police", Pi::player->GetDockedWith()->GetLabel().c_str()).c_str());

	if (fine == 0) {
		Message("We have no business with you at the moment.");
	} else {
		Message(stringf(256, "We do not tolerate crime. You must pay a fine of %s.", format_money(fine).c_str()).c_str());
		AddOption(sigc::mem_fun(this, &PoliceChatForm::Action), "Pay the fine now.", 1);
	}

	AddOption(sigc::mem_fun(this, &PoliceChatForm::Action), "Hang up.", 0);
}

void PoliceChatForm::Action(GenericChatForm *form, int val)
{
	switch (val) {
		case 1:
			{
				Sint64 crime, fine;
				Polit::GetCrime(&crime, &fine);
				if (fine > Pi::player->GetMoney()) {
					Pi::cpan->MsgLog()->Message("", "You do not have enough money.");
				} else {
					Pi::player->SetMoney(Pi::player->GetMoney() - fine);
					Polit::AddCrime(0, -fine);
					Close();
				}
			}
			break;

		default: Close();
			 break;
	}
}
