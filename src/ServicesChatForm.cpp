#include "ServicesChatForm.h"
#include "Pi.h"
#include "Player.h"
#include "SpaceStation.h"

ServicesChatForm::ServicesChatForm() : FaceChatForm()
{
	SetTitle(stringf(256, "%s services", Pi::player->GetDockedWith()->GetLabel().c_str()));

	SetBgColor(0.19, 0.39, 0.19, 1.0);
	SetTransparency(false);
	Add(new Gui::Label("Quit yo jibber-jabber"), 20, 20);
}


