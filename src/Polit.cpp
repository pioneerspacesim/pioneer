#include "libs.h"
#include "Pi.h"
#include "Polit.h"
#include "StarSystem.h"
#include "Sector.h"
#include "custom_starsystems.h"
#include "EquipType.h"
#include "PersistSystemData.h"

namespace Polit {

static PersistSystemData<Sint64> s_criminalRecord;
static PersistSystemData<Sint64> s_outstandingFine;

struct crime_t {
	Sint64 record;
	Sint64 fine;
} s_playerPerPolitCrimeRecord[POL_MAX];

struct politDesc_t {
	const char *description;
	/* Is this type a union (earth fed, etc), or merely a category
	 * with independent instances (ie each system has own justice system
	 * and therefore criminal record database) */
	bool politUnified;
};

const politDesc_t s_politTypes[POL_MAX] = {
	{ "<invalid turd>", 0 },
	{ "No central governance.", 0 },
	{ "Member of the Earth Federation.", 1 },
	{ "Member of the Confederation of Independent Systems.", 1 }
};

void Init()
{
	s_criminalRecord.Clear();
	s_outstandingFine.Clear();
	memset(s_playerPerPolitCrimeRecord, 0, sizeof(crime_t)*POL_MAX);
}

void Serialize()
{
	using namespace Serializer::Write;
	s_criminalRecord.Serialize();
	s_outstandingFine.Serialize();
	for (int i=0; i<POL_MAX; i++) {
		wr_int64(s_playerPerPolitCrimeRecord[i].record);
		wr_int64(s_playerPerPolitCrimeRecord[i].fine);
	}
}

void Unserialize()
{
	using namespace Serializer::Read;
	Init();
	if (Serializer::Read::IsOlderThan(5)) {

	} else {
		PersistSystemData<Sint64>::Unserialize(&s_criminalRecord);
		PersistSystemData<Sint64>::Unserialize(&s_outstandingFine);
		for (int i=0; i<POL_MAX; i++) {
			s_playerPerPolitCrimeRecord[i].record = rd_int64();
			s_playerPerPolitCrimeRecord[i].fine = rd_int64();
		}
	}
}

void AddCrime(Sint64 crimeBitset, Sint64 addFine)
{
	int politType = Pi::currentSystem->GetPoliticalType();

	if (s_politTypes[politType].politUnified) {
		s_playerPerPolitCrimeRecord[politType].record |= crimeBitset;
		s_playerPerPolitCrimeRecord[politType].fine += addFine;
	} else {
		SysLoc loc = Pi::currentSystem->GetLocation();
		Sint64 record = s_criminalRecord.Get(loc, 0);
		record |= crimeBitset;
		s_criminalRecord.Set(loc, crimeBitset);
		s_outstandingFine.Set(loc, s_outstandingFine.Get(loc, 0) + addFine);
	}
}

void GetCrime(Sint64 *crimeBitset, Sint64 *fine)
{
	int politType = Pi::currentSystem->GetPoliticalType();

	if (s_politTypes[politType].politUnified) {
		*crimeBitset = s_playerPerPolitCrimeRecord[politType].record;
		*fine = s_playerPerPolitCrimeRecord[politType].fine;
	} else {
		SysLoc loc = Pi::currentSystem->GetLocation();
		*crimeBitset = s_criminalRecord.Get(loc, 0);
		*fine = s_outstandingFine.Get(loc, 0);
	}
}

const char *GetDesc(StarSystem *s)
{
	return s_politTypes[s->GetPoliticalType()].description;
}

Polit::Alignment GetAlignmentForStarSystem(StarSystem *s, fixed human_infestedness)
{
	int sx, sy, sys_idx;
	s->GetPos(&sx, &sy, &sys_idx);

	Sector sec(sx, sy);
	
	/* from custom system definition */
	if (sec.m_systems[sys_idx].customSys) {
		Polit::Alignment t = sec.m_systems[sys_idx].customSys->polit;
		if (t != POL_INVALID) return t;
	}

	const unsigned long _init[3] = { sx, sy, sys_idx };
	MTRand rand(_init, 3);

	if ((sx == 0) && (sy == 0) && (sys_idx == 0)) {
		return Polit::POL_EARTH;
	} else if (human_infestedness > 0) {
		return static_cast<Alignment>(rand.Int32(POL_EARTH, POL_MAX-1));
	} else {
		return POL_NONE;
	}
}

bool IsCommodityLegal(StarSystem *s, Equip::Type t)
{
	int sx, sy, sys_idx;
	s->GetPos(&sx, &sy, &sys_idx);
	const unsigned long _init[3] = { sx, sy, sys_idx };
	MTRand rand(_init, 3);

	Polit::Alignment a = s->GetPoliticalType();

	if (a == POL_NONE) return true;

	switch (t) {
		case Equip::ANIMAL_MEAT:
			if ((a == POL_EARTH) || (a == POL_CONFED)) return rand.Int32(4)!=0;
			else return true;
		case Equip::LIQUOR:
			if ((a != POL_EARTH) && (a != POL_CONFED)) return rand.Int32(8)!=0;
			else return true;
		case Equip::HAND_WEAPONS:
			if (a == POL_EARTH) return false;
			if (a == POL_CONFED) return rand.Int32(3)!=0;
			else return rand.Int32(2) == 0;
		case Equip::BATTLE_WEAPONS:
			if ((a != POL_EARTH) && (a != POL_CONFED)) return rand.Int32(3)==0;
			return false;
		case Equip::NERVE_GAS:
			if ((a != POL_EARTH) && (a != POL_CONFED)) return rand.Int32(10)==0;
			return false;
		case Equip::NARCOTICS:
			if (a == POL_EARTH) return false;
			if (a == POL_CONFED) return rand.Int32(7)==0;
			else return rand.Int32(2)==0;
		default: return true;
	}
}

}

