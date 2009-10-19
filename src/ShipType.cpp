#include "ShipType.h"
#include "Serializer.h"

const char *ShipType::gunmountNames[GUNMOUNT_MAX] = {
	"Front", "Rear" };

const ShipType ShipType::types[] = {
	{
		"Swordfish Starfighter",
		"66",
		{ 2e6,-2e6,1e6,-1e6,-1e6,1e6 },
		1e7,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,0,0), vector3f(0,0,1) }
		},
		{ 20, 1, 1, 0 },
		20, 5, 4000000,
		Equip::DRIVE_CLASS1,
	}, {
		// besides running a wicked corporatist regime in the
		// sirius system, Sirius corporation make a range of
		// lovely starships
		"Sirius Interdictor", "61",
		{ 4e6,-4e6,1e6,-1e6,-1e6,1e6 },
		4e7,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,-0.5,0), vector3f(0,0,1) }
		},
		{ 90, 1, 2, 0 },
		90, 20, 16000000,
		Equip::DRIVE_CLASS3,
	}, {
		// john - you should pick names yourself or this happens
		"Ladybird Starfighter",
		"62",
		{ 2e6,-2e6,1e6,-1e6,-1e6,1e6 },
		1e7,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,0,0), vector3f(0,0,1) }
		},
		{ 60, 1, 1, 0 },
		60, 15, 8700000,
		Equip::DRIVE_CLASS2,
	}, {
		"Taipan",
		"taipan",
		{ 4e6,-4e6,1e6,-1e6,-1e6,1e6 },
		1e7,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,0,0), vector3f(0,0,1) }
		},
		{ 240, 1, 1, 0 },
		240, 55, 56000000,
		Equip::DRIVE_CLASS4,
	}, {
		"Walrus",
		"60",
		{ 12e6,-12e6,4e6,-4e6,-4e6,4e6 },
		1e7,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,0,0), vector3f(0,0,1) }
		},
		{ 320, 1, 1, 0 },
		320, 80, 35000000,
		Equip::DRIVE_CLASS5,
	}, {
		"Flowerfairy Heavy Trader",
		"63",
		{ 1e6,-1e6,1e6,-1e6,-1e6,1e6 },
		1e7,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,0,0), vector3f(0,0,1) }
		},
		{ 500, 1, 2, 0 },
		500, 125, 55000000,
		Equip::DRIVE_CLASS6,
	}
};

void EquipSet::Save()
{
	using namespace Serializer::Write;
	for (int i=0; i<Equip::SLOT_MAX; i++) {
		for (unsigned int j=0; j<equip[i].size(); j++) {
			wr_int(static_cast<int>(equip[i][j]));
		}
	}
}

/*
 * Should have initialised with EquipSet(ShipType::Type) first
 */
void EquipSet::Load()
{
	using namespace Serializer::Read;
	for (int i=0; i<Equip::SLOT_MAX; i++) {
		for (unsigned int j=0; j<equip[i].size(); j++) {
			equip[i][j] = static_cast<Equip::Type>(rd_int());
		}
	}
}

