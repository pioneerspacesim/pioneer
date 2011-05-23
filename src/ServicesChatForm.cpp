#include "ServicesChatForm.h"

ServicesChatForm::ServicesChatForm() : FaceChatForm()
{
	SetBgColor(0.19, 0.39, 0.19, 1.0);
	SetTransparency(false);
	Add(new Gui::Label("Quit yo jibber-jabber"), 20, 20);
}
