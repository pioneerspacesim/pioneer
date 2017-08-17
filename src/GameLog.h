#ifndef _GAMELOG_H
#define _GAMELOG_H

#include "libs.h"

/*
 * For storing all in-game log messages
 * and drawing the last X messages as overlay (all views)
 * atm it holds only 6 messages, but do extend
 */


class GameLog {
public:
	enum Priority { PRIORITY_NORMAL = 0,
									PRIORITY_IMPORTANT = 1,
									PRIORITY_ALERT = 2 };

	void Add(const std::string&);
	void Add(const std::string &from, const std::string &msg, Priority priority);

};

#endif
