#ifndef _REGISTER_H
#define _REGISTER_H

#include "../Mission.h"

#define MISSION_MAX 3

struct MissionFactoryDef {
	Mission *(*Create)(int);
	int rarity;
};

#include "DonateToCranks.h"
#include "DeliverPackage.h"
#include "GoodsTrader.h"

/*
 * Be sure to keep indices since they are used for save()/load()
 */
const MissionFactoryDef missionFactoryFn[MISSION_MAX] = {
	{ &DonateToCranks::Create, 1 },
	{ &DeliverPackage::Create, 1 },
	{ &GoodsTrader::Create, 1 },
};

#endif /* _REGISTER_H */
