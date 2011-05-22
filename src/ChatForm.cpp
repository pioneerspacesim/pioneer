#include "ChatForm.h"

ChatForm::ChatForm() : Gui::Fixed(470, 400)
{
	SetBgColor(0.19, 0.39, 0.19, 1.0);
	SetTransparency(false);
	Add(new Gui::Label("Quit yo jibber-jabber"), 20, 20);
}
