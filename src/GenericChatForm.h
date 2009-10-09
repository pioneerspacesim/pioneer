#ifndef _GENERICCHATFORM_H
#define _GENERICCHATFORM_H

#include <string>
#include "Gui.h"

class GenericChatForm: public Gui::Fixed {
public:
	GenericChatForm();
	virtual ~GenericChatForm() {}
	void Close() { onFormClose.emit(); }
	void Clear();
	void AddOption(sigc::slot<void,GenericChatForm*,int> slot, const char *text, int val);
	void Message(const char*);
	sigc::signal<void> onFormClose;
	sigc::signal<void> onSomethingChanged;

	Gui::VBox *m_msgregion;
	Gui::VBox *m_optregion;
private:
	bool hasOpts;
};	

#endif /* _GENERICCHATFORM_H */
