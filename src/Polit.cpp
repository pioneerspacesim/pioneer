#include "libs.h"
#include "Pi.h"
#include "Polit.h"
#include "StarSystem.h"
#include "Sector.h"
#include "Space.h"
#include "Ship.h"
#include "ShipCpanel.h"
#include "SpaceStation.h"
#include "EquipType.h"
#include "PersistSystemData.h"
#include "Lang.h"
#include "StringF.h"
#include "Game.h"

namespace Polit {

static PersistSystemData<Sint64> s_criminalRecord;
static PersistSystemData<Sint64> s_outstandingFine;
struct crime_t {
	Sint64 record;
	Sint64 fine;
} s_playerPerBlocCrimeRecord[BLOC_MAX];

const char *crimeNames[64] = {
	Lang::TRADING_ILLEGAL_GOODS,
	Lang::UNLAWFUL_WEAPONS_DISCHARGE,
	Lang::PIRACY,
	Lang::MURDER,
};
// in 1/100th credits, as all money is
static const Sint64 crimeBaseFine[64] = {
	50000,
	100000,
	1000000,
	1500000,
};
const char *s_blocDesc[BLOC_MAX] = {
	Lang::INDEPENDENT,
	Lang::EARTH_FEDERATION,
	Lang::INDEPENDENT_CONFEDERATION,
	Lang::EMPIRE
};
const char *s_econDesc[ECON_MAX] = {
	Lang::NO_ESTABLISHED_ORDER,
	Lang::HARD_CAPITALIST,
	Lang::CAPITALIST,
	Lang::MIXED_ECONOMY,
	Lang::PLANNED_ECONOMY
};

struct politDesc_t {
	const char *description;
	int rarity;
	Bloc bloc;
	PolitEcon econ;
	fixed baseLawlessness;
};
const politDesc_t s_govDesc[GOV_MAX] = {
	{ "<invalid turd>", 0, BLOC_NONE, ECON_NONE, fixed(1,1) },
	{ Lang::NO_CENTRAL_GOVERNANCE, 0, BLOC_NONE, ECON_NONE, fixed(1,1) },
	{ Lang::EARTH_FEDERATION_COLONIAL_RULE, 2, BLOC_EARTHFED, ECON_CAPITALIST, fixed(3,10) },
	{ Lang::EARTH_FEDERATION_DEMOCRACY, 3, BLOC_EARTHFED, ECON_CAPITALIST, fixed(15,100) },
	{ Lang::IMPERIAL_RULE, 3, BLOC_EMPIRE, ECON_PLANNED, fixed(15,100) },
	{ Lang::LIBERAL_DEMOCRACY, 2, BLOC_CIS, ECON_CAPITALIST, fixed(25,100) },
	{ Lang::SOCIAL_DEMOCRACY, 2, BLOC_CIS, ECON_MIXED, fixed(20,100) },
	{ Lang::LIBERAL_DEMOCRACY, 2, BLOC_NONE, ECON_CAPITALIST, fixed(25,100) },
	{ Lang::CORPORATE_SYSTEM, 2, BLOC_NONE, ECON_CAPITALIST, fixed(40,100) },
	{ Lang::SOCIAL_DEMOCRACY, 2, BLOC_NONE, ECON_MIXED, fixed(25,100) },
	{ Lang::MILITARY_DICTATORSHIP, 5, BLOC_EARTHFED, ECON_CAPITALIST, fixed(40,100) },
	{ Lang::MILITARY_DICTATORSHIP, 6, BLOC_NONE, ECON_CAPITALIST, fixed(25,100) },
	{ Lang::MILITARY_DICTATORSHIP, 6, BLOC_NONE, ECON_MIXED, fixed(25,100) },
	{ Lang::MILITARY_DICTATORSHIP, 5, BLOC_EMPIRE, ECON_MIXED, fixed(40,100) },
	{ Lang::COMMUNIST, 10, BLOC_NONE, ECON_PLANNED, fixed(25,100) },
	{ Lang::PLUTOCRATIC_DICTATORSHIP, 4, BLOC_NONE, ECON_VERY_CAPITALIST, fixed(45,100) },
	{ Lang::VIOLENT_ANARCHY, 2, BLOC_NONE, ECON_NONE, fixed(90,100) },
};

void Init()
{
	s_criminalRecord.Clear();
	s_outstandingFine.Clear();
	memset(s_playerPerBlocCrimeRecord, 0, sizeof(crime_t)*BLOC_MAX);
}

void Serialize(Serializer::Writer &wr)
{
	s_criminalRecord.Serialize(wr);
	s_outstandingFine.Serialize(wr);
	for (int i=0; i<BLOC_MAX; i++) {
		wr.Int64(s_playerPerBlocCrimeRecord[i].record);
		wr.Int64(s_playerPerBlocCrimeRecord[i].fine);
	}
}

void Unserialize(Serializer::Reader &rd)
{
	Init();
	PersistSystemData<Sint64>::Unserialize(rd, &s_criminalRecord);
	PersistSystemData<Sint64>::Unserialize(rd, &s_outstandingFine);
	for (int i=0; i<BLOC_MAX; i++) {
		s_playerPerBlocCrimeRecord[i].record = rd.Int64();
		s_playerPerBlocCrimeRecord[i].fine = rd.Int64();
	}
}

/* The drawbacks of stuffing stuff into integers */
static int GetCrimeIdxFromEnum(enum Crime crime)
{
	PiVerify(crime);
	for (int i=0; i<64; i++) {
		if (crime & (1<<i)) return i;
	}
	return 0;
}

void NotifyOfCrime(Ship *s, enum Crime crime)
{
	// ignore crimes of NPCs for the time being
	if (!s->IsType(Object::PLAYER)) return;
	// find nearest starport to this evil criminal
	SpaceStation *station = static_cast<SpaceStation*>(Pi::game->GetSpace()->FindNearestTo(s, Object::SPACESTATION));
	if (station) {
		double dist = station->GetPositionRelTo(s).Length();
		// too far away for crime to be noticed :)
		if (dist > 100000.0) return;
		const int crimeIdx = GetCrimeIdxFromEnum(crime);
		Pi::cpan->MsgLog()->ImportantMessage(station->GetLabel(),
				stringf(Lang::X_CANNOT_BE_TOLERATED_HERE, formatarg("crime", crimeNames[crimeIdx])));

		float lawlessness = Pi::game->GetSpace()->GetStarSystem()->GetSysPolit().lawlessness.ToFloat();
		Sint64 oldCrimes, oldFine;
		GetCrime(&oldCrimes, &oldFine);
		Sint64 newFine = std::max(1, 1 + int(crimeBaseFine[crimeIdx] * (1.0-lawlessness)));
		// don't keep compounding fines (maybe should for murder, etc...)
		if ( (!(crime & CRIME_MURDER)) && (newFine < oldFine) ) newFine = 0;
		AddCrime(crime, newFine);
	}
}

void AddCrime(Sint64 crimeBitset, Sint64 addFine)
{
	int politType = Pi::game->GetSpace()->GetStarSystem()->GetSysPolit().govType;

	if (s_govDesc[politType].bloc != BLOC_NONE) {
		const Bloc b = s_govDesc[politType].bloc;
		s_playerPerBlocCrimeRecord[b].record |= crimeBitset;
		s_playerPerBlocCrimeRecord[b].fine += addFine;
	} else {
		SystemPath path = Pi::game->GetSpace()->GetStarSystem()->GetPath();
		Sint64 record = s_criminalRecord.Get(path, 0);
		record |= crimeBitset;
		s_criminalRecord.Set(path, crimeBitset);
		s_outstandingFine.Set(path, s_outstandingFine.Get(path, 0) + addFine);
	}
}

void GetCrime(Sint64 *crimeBitset, Sint64 *fine)
{
	// no crime in hyperspace :)
	if (Pi::game->IsHyperspace()) {
		*crimeBitset = 0;
		*fine = 0;
		return ;
	}

	int politType = Pi::game->GetSpace()->GetStarSystem()->GetSysPolit().govType;

	if (s_govDesc[politType].bloc != BLOC_NONE) {
		const Bloc b = s_govDesc[politType].bloc;
		*crimeBitset = s_playerPerBlocCrimeRecord[b].record;
		*fine = s_playerPerBlocCrimeRecord[b].fine;
	} else {
		SystemPath path = Pi::game->GetSpace()->GetStarSystem()->GetPath();
		*crimeBitset = s_criminalRecord.Get(path, 0);
		*fine = s_outstandingFine.Get(path, 0);
	}
}

const char *GetGovernmentDesc(StarSystem *s)
{
	return s_govDesc[s->GetSysPolit().govType].description;
}
const char *GetEconomicDesc(StarSystem *s)
{
	return s_econDesc [ s_govDesc[s->GetSysPolit().govType].econ ];
}
const char *GetAllegianceDesc(StarSystem *s)
{
	return s_blocDesc [ s_govDesc[s->GetSysPolit().govType].bloc ];
}

#define POLIT_SEED 0x1234abcd

void GetSysPolitStarSystem(const StarSystem *s, const fixed human_infestedness, SysPolit &outSysPolit)
{
	SystemPath path = s->GetPath();
	const unsigned long _init[5] = { path.sectorX, path.sectorY, path.sectorZ, path.systemIndex, POLIT_SEED };
	MTRand rand(_init, 5);

	Sector sec(path.sectorX, path.sectorY, path.sectorZ);

	GovType a = GOV_INVALID;
	
	/* from custom system definition */
	if (sec.m_systems[path.systemIndex].customSys) {
		Polit::GovType t = sec.m_systems[path.systemIndex].customSys->govType;
		a = t;
	}
	if (a == GOV_INVALID) {
		if (path == SystemPath(0,0,0,0)) {
			a = Polit::GOV_EARTHDEMOC;
		} else if (human_infestedness > 0) {
			a = static_cast<GovType>(rand.Int32(GOV_RAND_MIN, GOV_RAND_MAX));
		} else {
			a = GOV_NONE;
		}
	}

	outSysPolit.govType = a;
	outSysPolit.lawlessness = s_govDesc[a].baseLawlessness * rand.Fixed();
}

#define POLIT_SALT 0x8732abdf

bool IsCommodityLegal(const StarSystem *s, Equip::Type t)
{
	SystemPath path = s->GetPath();
	const unsigned long _init[5] = { path.sectorX, path.sectorY, path.sectorZ, path.systemIndex, POLIT_SALT };
	MTRand rand(_init, 5);

	Polit::GovType a = s->GetSysPolit().govType;
	const Bloc b = s_govDesc[a].bloc;

	if (a == GOV_NONE) return true;

	switch (t) {
		case Equip::ANIMAL_MEAT:
		case Equip::LIVE_ANIMALS:
			if ((b == BLOC_EARTHFED) || (b == BLOC_CIS)) return rand.Int32(4)!=0;
			else return true;
		case Equip::LIQUOR:
			if ((b != BLOC_EARTHFED) && (b != BLOC_CIS)) return rand.Int32(8)!=0;
			else return true;
		case Equip::HAND_WEAPONS:
			if (b == BLOC_EARTHFED) return false;
			if (b == BLOC_CIS) return rand.Int32(3)!=0;
			else return rand.Int32(2) == 0;
		case Equip::BATTLE_WEAPONS:
			if ((b != BLOC_EARTHFED) && (b != BLOC_CIS)) return rand.Int32(3)==0;
			return false;
		case Equip::NERVE_GAS:
			if ((b != BLOC_EARTHFED) && (b != BLOC_CIS)) return rand.Int32(10)==0;
			return false;
		case Equip::NARCOTICS:
			if (b == BLOC_EARTHFED) return false;
			if (b == BLOC_CIS) return rand.Int32(7)==0;
			else return rand.Int32(2)==0;
		case Equip::SLAVES:
			if ((b != BLOC_EARTHFED) && (b != BLOC_CIS)) return rand.Int32(16)==0;
			return false;
		default: return true;
	}
}

}

