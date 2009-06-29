#ifndef _MISSION_H
#define _MISSION_H

#include <string>
#include "Gui.h"

class Mission;

class MissionChatForm: public Gui::VBox {
public:
	MissionChatForm() { SetSpacing(5.0f); }
	virtual ~MissionChatForm() {}
	void Close() { onFormClose.emit(); }
	void AddOption(Mission *, const char *text, int val);
	void Message(const char*);
	sigc::signal<void> onFormClose;
	sigc::signal<void> onSomethingChanged;
private:
};	

class Mission {
public:
	Mission(int type): type(type) {}
	virtual ~Mission() {}
	virtual void Randomize() = 0;
	virtual std::string GetBulletinBoardText() = 0;
	virtual void StartChat(MissionChatForm *) = 0;
	virtual void FormResponse(MissionChatForm*, int) = 0;

	static Mission *GenerateRandom();
	static Mission *Load();
	void Save();

	const int type;
protected:
	virtual void _Save() = 0;
	virtual void _Load() = 0;
};

#endif /* _MISSION */
