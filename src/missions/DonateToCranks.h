#ifndef _MISSION_DONATETOCRANKS_H
#define _MISSION_DONATETOCRANKS_H

#include "../Mission.h"
#include "../Gui.h"

class DonateToCranks: public Mission {
public:
	DonateToCranks(int type): Mission(type) {
		m_cause = 0;
	}
	virtual void Randomize();
	virtual std::string GetBulletinBoardText();
	virtual void StartChat(GenericChatForm *);
	virtual void FormResponse(GenericChatForm*, int);

	static Mission* Create(int type) {
		return new DonateToCranks(type);
	}
protected:
	virtual void _Load();
	virtual void _Save();
	int m_cause;
};


#endif /* _MISSION_DONATETOCRANKS_H */
