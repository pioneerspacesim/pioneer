// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _EQUIPSET_H
#define _EQUIPSET_H

#include "libs.h"
#include "EquipType.h"
#include "ShipType.h"

class EquipSet {
public:
	EquipSet();

	void InitSlotSizes(const ShipType::Id &t);
	int GetSlotSize(Equip::Slot s) const;
	Equip::Type Get(Equip::Slot s) const;
	Equip::Type Get(Equip::Slot s, int idx) const;
	bool Set(Equip::Slot s, int idx, Equip::Type e);
	int Add(Equip::Type e, int num);
	int Add(Equip::Type e);
	// returns number removed
	int Remove(Equip::Type e, int num);
	int Count(Equip::Slot s, Equip::Type e) const;
	int FreeSpace(Equip::Slot s) const;

	sigc::signal<void,Equip::Type> onChange;

protected:
	int ChangeType(Equip::Type from, Equip::Type to, int num);

	std::vector<Equip::Type> equip[Equip::SLOT_MAX];
};

#endif
