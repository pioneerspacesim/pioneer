#ifndef _CHATFORM_H
#define _CHATFORM_H

#include "Form.h"

class ChatForm : public FaceForm {
public:
	ChatForm();

	void SetMessage(std::string msg);
	void AddOption(std::string text, int val);

	void Clear();

	virtual void OnOptionClicked(int option) = 0;

private:
	void OnOptionClickedTrampoline(int option) {
		this->OnOptionClicked(option);
	}

	Gui::Label *m_message;
	Gui::VBox *m_options;

	bool m_hasOptions;
};

#endif
