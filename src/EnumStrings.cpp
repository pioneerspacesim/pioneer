// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "enum_table.h"
#include <string>
#include <map>

namespace EnumStrings {

static std::map< std::string, std::map<int,std::string> > enumStrings;
static std::map< std::string, std::map<std::string,int> > enumValues;

struct EnumTable {
	const char *ns;
	const EnumItem *item;
};
static const EnumTable enumTables[] = {
	{ "BodySuperType",         ENUM_BodySuperType },
	{ "PolitCrime",            ENUM_PolitCrime },
	{ "PolitEcon",             ENUM_PolitEcon },
	{ "PolitGovType",          ENUM_PolitGovType },
	{ "EquipSlot",             ENUM_EquipSlot },
	{ "EquipType",             ENUM_EquipType },
	{ "DualLaserOrientation",  ENUM_DualLaserOrientation },
	{ "ShipTypeTag",           ENUM_ShipTypeTag },
	{ "ShipTypeThruster",      ENUM_ShipTypeThruster },
	{ "ShipJumpStatus",        ENUM_ShipJumpStatus },
	{ "ShipAlertStatus",       ENUM_ShipAlertStatus },
	{ "ShipFuelStatus",        ENUM_ShipFuelStatus },
	{ "ShipFlightState",       ENUM_ShipFlightState },
	{ "ShipAIError",           ENUM_ShipAIError },
	{ "ShipAnimation",         ENUM_ShipAnimation },
	{ "SpaceStationAnimation", ENUM_SpaceStationAnimation },
	{ "FileSystemRoot",        ENUM_FileSystemRoot },
	{ "UIAlignDirection",      ENUM_UIAlignDirection },
	{ "UIMarginDirection",     ENUM_UIMarginDirection },
	{ "UIFont",                ENUM_UIFont },
	{ "UISizeControl",         ENUM_UISizeControl },
	{ "UIEventType",           ENUM_UIEventType },
	{ "UIGradientDirection",   ENUM_UIGradientDirection },
	{ "UIExpandDirection",     ENUM_UIExpandDirection },
	{ "UIKeyboardAction",      ENUM_UIKeyboardAction },
	{ "UIMouseButtonAction",   ENUM_UIMouseButtonAction },
	{ "UIMouseButtonType",     ENUM_UIMouseButtonType },
	{ "UIMouseWheelDirection", ENUM_UIMouseWheelDirection },
	{ "GameUIFaceFlags",       ENUM_GameUIFaceFlags },
	{ 0, 0 }
};

void Init()
{
	for (const EnumTable *table = enumTables; table->ns; table++) {
		std::map<int,std::string> &stringMap = enumStrings[table->ns];
		std::map<std::string,int> &valueMap = enumValues[table->ns];

		for (const EnumItem *item = table->item; item->name; item++) {
			stringMap.insert(std::make_pair(item->value, item->name));
			valueMap.insert(std::make_pair(item->name, item->value));
		}
	}
}

const char *GetString(const char *ns, int value)
{
	std::map< std::string, std::map<int,std::string> >::const_iterator tableIter = enumStrings.find(ns);
	if (tableIter == enumStrings.end())
		return 0;

	const std::map<int,std::string> &table = tableIter->second;
	std::map<int,std::string>::const_iterator e = table.find(value);
	if (e == table.end())
		return 0;

	return e->second.c_str();
}

int GetValue(const char *ns, const char *name)
{
	std::map< std::string, std::map<std::string,int> >::const_iterator tableIter = enumValues.find(ns);
	if (tableIter == enumValues.end())
		return -1;

	const std::map<std::string,int> &table = tableIter->second;
	std::map<std::string,int>::const_iterator e = table.find(name);
	if (e == table.end())
		return -1;

	return e->second;
}

}
