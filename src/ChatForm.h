#ifndef _CHATFORM_H
#define _CHATFORM_H

#include "Form.h"

class ChatForm : public FaceForm {
public:
	ChatForm(FormController *controller);

	virtual void ShowAll();

	void SetMessage(std::string msg);
	void AddOption(std::string text, int val);
	void Clear();
	void Close();

	virtual void OnOptionClicked(int option) = 0;

private:
	void OnOptionClickedTrampoline(int option);

	Gui::Label *m_message;
	Gui::VBox *m_options;

	bool m_hasOptions;
	bool m_doSetup;
	bool m_close;
};

#endif
