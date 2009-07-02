#ifndef _MISSION_DELIVERPACKAGE_H
#define _MISSION_DELIVERPACKAGE_H

#include "../Mission.h"
#include "../Gui.h"
#include "../StarSystem.h"

class DeliverPackage: public Mission {
public:
	DeliverPackage(int type): Mission(type) {
		m_flavour = 0;
	}
	virtual void Randomize();
	virtual std::string GetBulletinBoardText();
	virtual std::string GetMissionText();
	virtual std::string GetClientName() { return m_personName; }
	virtual void StartChat(MissionChatForm *);
	virtual void FormResponse(MissionChatForm*, int);
	virtual void AttachToPlayer();

	static Mission* Create(int type) {
		return new DeliverPackage(type);
	}
protected:
	virtual void _Load();
	virtual void _Save();
private:
	void TestCompleted();
	void PutOptions(MissionChatForm *);
	bool m_personGender;
	std::string m_personName;
	int m_flavour;
	int m_basePay;
	double m_deadline;
	SBodyPath m_dest;

	sigc::connection m_onDockConn;
};


#endif /* _MISSION_DELIVERPACKAGE_H */
