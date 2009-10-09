#ifndef _MISSION_H
#define _MISSION_H

#include <string>
#include "Gui.h"
#include "GenericChatForm.h"

struct CouldNotMakeMissionException {};

struct SBodyPath;

class Mission {
public:
	Mission(int type): type(type) { m_agreedPayoff = 0; m_status = ACTIVE; }
	virtual ~Mission() {}
	virtual void Randomize() = 0;
	virtual std::string GetBulletinBoardText() = 0;
	virtual std::string GetMissionText() { return "<no mission description>"; }
	virtual std::string GetClientName() { return "---"; }
	virtual void StartChat(GenericChatForm *) = 0;
	virtual void FormResponse(GenericChatForm*, int) = 0;
	virtual void AttachToPlayer() {}
	enum MissionState { ACTIVE, COMPLETED, FAILED };
	MissionState GetStatus() const { return m_status; }

	static Mission *GenerateRandom();
	static Mission *Load();
	void Save();
	int GetPayoff() const { return m_agreedPayoff; }

	const int type;
protected:
	void GiveToPlayer();
	virtual void _Save() = 0;
	virtual void _Load() = 0;
	
	enum MissionState m_status;
	int m_agreedPayoff;
	// various useful utility things
	// In form "the Arcturus system"
	static std::string NaturalSystemName(const SBodyPath &);
	// In form "Snaar trading post in the Arcturus system"
	static std::string NaturalSpaceStationName(const SBodyPath &);
};

#endif /* _MISSION */
