#ifndef _REGISTER_H
#define _REGISTER_H

#include "../Mission.h"

#define MISSION_MAX 1

struct MissionFactoryDef {
	Mission *(*Create)(int);
	int rarity;
};

#include "DonateToCranks.h"

/*
 * Be sure to keep indices since they are used for save()/load()
 */
const MissionFactoryDef missionFactoryFn[MISSION_MAX] = {
	{ &DonateToCranks::Create, 1 },
};

#endif /* _REGISTER_H */
