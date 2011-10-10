#ifndef _EQUIPTYPE_H
#define _EQUIPTYPE_H
#include "Color.h"

#define EQUIP_INPUTS	2

struct EquipType;
struct LaserType;

namespace Equip {
	enum Slot {
#define Slot_ITEM(x) SLOT_##x,
#include "EquipTypeEnums.h"
		SLOT_MAX
	};
	enum Type {
#define CommodityType_ITEM(x) x,
#define CommodityType_ITEM_X(x,y) // these come later
#include "EquipTypeEnums.h"

#define EquipType_ITEM(x) x,
#define EquipType_ITEM_X(x,y) // these come later
#include "EquipTypeEnums.h"

		TYPE_MAX,

#define CommodityType_ITEM(x)
#define CommodityType_ITEM_X(x,y) x = y,
#define EquipType_ITEM(x)
#define EquipType_ITEM_X(x,y) x = y,
#include "EquipTypeEnums.h"
	};

	const int LASER_MINING = 0x1;
	const int LASER_DUAL = 0x2;

	extern const LaserType lasers[];
	extern const EquipType types[];
}

struct EquipType {
	const char *name;
	const char *description;
	Equip::Slot slot;
	int tableIndex;			// Index into subtype-specific table
	// for commodities: production requirement. eg metal alloys input would be metal ore
	// for hyperdrives: inputs[0] = fuel commodity
	Equip::Type inputs[EQUIP_INPUTS];
	int basePrice;
	int mass;
	int pval; // hello angband. used for general 'power' attribute...
	int econType;
	int techLevel; /* 0-5 */
	float rechargeTime;			// to be eliminated maybe
};

struct LaserType {
	float lifespan;
	float speed;
	float damage;
	float rechargeTime;
	float psize;
	int flags;
	Color color;
};

#endif /* _EQUIPTYPE_H */
