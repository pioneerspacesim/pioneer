#ifndef _POLICECHATFORM_H
#define _POLICECHATFORM_H

#include "gui/Gui.h"
#include "GenericChatForm.h"

class PoliceChatForm: public GenericChatForm {
public:
	PoliceChatForm();
private:
	void Action(GenericChatForm *form, int val);
};

#endif /* _POLICECHATFORM_H */
