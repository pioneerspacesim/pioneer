#ifndef _EQUIPSET_H
#define _EQUIPSET_H

#include "libs.h"
#include "EquipType.h"
#include "ShipType.h"
#include <vector>

class EquipSet {
public:
	EquipSet() {}

	void InitSlotSizes(const ShipType::Type t) {
		const ShipType &st = ShipType::types[t];
		for (int i=0; i<Equip::SLOT_MAX; i++) {
			// vector swap idiom (de-allocates unneeded space)
			std::vector<Equip::Type>(st.equipSlotCapacity[i], Equip::NONE).swap(equip[i]);
		}
		onChange.emit(Equip::NONE);
	}
	int GetSlotSize(Equip::Slot s) const {
		return equip[s].size();
	}
	Equip::Type Get(Equip::Slot s) const {
		return Get(s, 0);
	}
	Equip::Type Get(Equip::Slot s, int idx) const {
		assert(idx >= 0);
		if (signed(equip[s].size()) <= idx) return Equip::NONE;
		else return equip[s][idx];
	}
	bool Set(Equip::Slot s, int idx, Equip::Type e) {
		assert(idx >= 0);
		assert(e < Equip::TYPE_MAX);
		if (signed(equip[s].size()) <= idx) return false;
		equip[s][idx] = e;
		onChange.emit(e);
		return true;
	}
	int Add(Equip::Type e, int num) {
		return ChangeType(Equip::NONE, e, num);
	}
	int Add(Equip::Type e) {
		return Add(e, 1);
	}
	// returns number removed
	int Remove(Equip::Type e, int num) {
		return ChangeType(e, Equip::NONE, num);
	}
	int Count(Equip::Slot s, Equip::Type e) const {
		assert(e < Equip::TYPE_MAX);
		int num = 0;
		for (unsigned int i=0; i<equip[s].size(); i++) {
			if (equip[s][i] == e) num++;
		}
		return num;
	}
	int FreeSpace(Equip::Slot s) const {
		return Count(s, Equip::NONE);
	}

	sigc::signal<void,Equip::Type> onChange;
protected:
	int ChangeType(Equip::Type from, Equip::Type to, int num) {
		assert(num >= 0);
		assert((from < Equip::TYPE_MAX) && (to < Equip::TYPE_MAX));

		if (from == to) return 0;

		assert((from == Equip::NONE) || (to == Equip::NONE) || (Equip::types[from].slot == Equip::types[to].slot));
		const Equip::Type e = (from == Equip::NONE) ? to : from;
		const Equip::Slot s = Equip::types[e].slot;
		int numDone = 0;
		for (unsigned int i=0; (numDone < num) && (i < equip[s].size()); i++) {
			if (equip[s][i] == from) {
				equip[s][i] = to;
				numDone++;
			}
		}
		if (numDone) {
			if (from != Equip::NONE) onChange.emit(from);
			if (to != Equip::NONE) onChange.emit(to);
		}
		return numDone;
	}

	std::vector<Equip::Type> equip[Equip::SLOT_MAX];
};

#endif
