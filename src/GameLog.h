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
	void Add(const std::string&);
	void Add(const std::string &from, const std::string &msg);

};

#endif
