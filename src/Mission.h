#ifndef _MISSION_H
#define _MISSION_H

#include <string>
#include "Gui.h"
#include "GenericChatForm.h"

struct CouldNotMakeMissionException {};

struct SBodyPath;

class Mission {
public:
	Mission() { m_agreedPayoff = 0; m_status = ACTIVE; }
	enum MissionState { ACTIVE, COMPLETED, FAILED };
	const std::string &GetMissionText() const { return m_missionText; }
	const std::string &GetClientName() const { return m_clientName; }
	MissionState GetStatus() const { return m_status; }
	int GetPayoff() const { return m_agreedPayoff; }

	enum MissionState m_status;
	std::string m_missionText;
	std::string m_clientName;
	int m_agreedPayoff;
protected:
	// various useful utility things
	// In form "the Arcturus system"
	static std::string NaturalSystemName(const SBodyPath &);
	// In form "Snaar trading post in the Arcturus system"
	static std::string NaturalSpaceStationName(const SBodyPath &);
};

#endif /* _MISSION */
