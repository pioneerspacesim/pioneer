#include "PoliceChatForm.h"

PoliceChatForm::PoliceChatForm(): GenericChatForm()
{
	Message("Who's been a naughty boy then, eh?\nWe will have to take you down to the station forSYNTAX ERROR AT LINE 12");

	AddOption(sigc::mem_fun(this, &PoliceChatForm::HangUp), "Go back", 0);
}

void PoliceChatForm::HangUp(GenericChatForm *form, int val)
{
	Close();
}
