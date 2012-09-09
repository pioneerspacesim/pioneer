#include "libs.h"
#include "Pi.h"
#include "Polit.h"
#include "galaxy/StarSystem.h"
#include "galaxy/Sector.h"
#include "Factions.h"
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
};
static std::vector<crime_t> s_vplayerPerBlocCrimeRecord;

const char *crimeNames[64] = {
	Lang::TRADING_ILLEGAL_GOODS,
	Lang::UNLAWFUL_WEAPONS_DISCHARGE,
	Lang::PIRACY,
	Lang::MURDER
};
// in 1/100th credits, as all money is
static const Sint64 crimeBaseFine[64] = {
	50000,
	100000,
	1000000,
	1500000,
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
	Uint32 faction;	// default (i.e. Invalid) is UINT_MAX
	PolitEcon econ;
	fixed baseLawlessness;
};
static politDesc_t s_govDesc[GOV_MAX] = {
	{ "<invalid turd>",							0,		UINT_MAX,	ECON_NONE,				fixed(1,1) },
	{ Lang::NO_CENTRAL_GOVERNANCE,				0,		UINT_MAX,	ECON_NONE,				fixed(1,1) },
	{ Lang::EARTH_FEDERATION_COLONIAL_RULE,		2,		UINT_MAX,	ECON_CAPITALIST,		fixed(3,10) },
	{ Lang::EARTH_FEDERATION_DEMOCRACY,			3,		UINT_MAX,	ECON_CAPITALIST,		fixed(15,100) },
	{ Lang::IMPERIAL_RULE,						3,		UINT_MAX,	ECON_PLANNED,			fixed(15,100) },
	{ Lang::LIBERAL_DEMOCRACY,					2,		UINT_MAX,	ECON_CAPITALIST,		fixed(25,100) },
	{ Lang::SOCIAL_DEMOCRACY,					2,		UINT_MAX,	ECON_MIXED,				fixed(20,100) },
	{ Lang::LIBERAL_DEMOCRACY,					2,		UINT_MAX,	ECON_CAPITALIST,		fixed(25,100) },
	{ Lang::CORPORATE_SYSTEM,					2,		UINT_MAX,	ECON_CAPITALIST,		fixed(40,100) },
	{ Lang::SOCIAL_DEMOCRACY,					2,		UINT_MAX,	ECON_MIXED,				fixed(25,100) },
	{ Lang::MILITARY_DICTATORSHIP,				5,		UINT_MAX,	ECON_CAPITALIST,		fixed(40,100) },
	{ Lang::MILITARY_DICTATORSHIP,				6,		UINT_MAX,	ECON_CAPITALIST,		fixed(25,100) },
	{ Lang::MILITARY_DICTATORSHIP,				6,		UINT_MAX,	ECON_MIXED,				fixed(25,100) },
	{ Lang::MILITARY_DICTATORSHIP,				5,		UINT_MAX,	ECON_MIXED,				fixed(40,100) },
	{ Lang::COMMUNIST,							10,		UINT_MAX,	ECON_PLANNED,			fixed(25,100) },
	{ Lang::PLUTOCRATIC_DICTATORSHIP,			4,		UINT_MAX,	ECON_VERY_CAPITALIST,	fixed(45,100) },
	{ Lang::VIOLENT_ANARCHY,					2,		UINT_MAX,	ECON_NONE,				fixed(90,100) },
};

void Init()
{
	s_criminalRecord.Clear();
	s_outstandingFine.Clear();

	// setup the per faction criminal records
	s_vplayerPerBlocCrimeRecord.clear();
	const Uint32 numFactions = Faction::GetNumFactions();
	s_vplayerPerBlocCrimeRecord.reserve( numFactions );
	s_vplayerPerBlocCrimeRecord.resize( numFactions );
	for (Uint32 i=0; i<s_vplayerPerBlocCrimeRecord.size(); i++) {
		s_vplayerPerBlocCrimeRecord[i].record = 0;
		s_vplayerPerBlocCrimeRecord[i].fine = 0;
	}

	// now setup the faction links, hopefully.
	for (Uint32 i=0; i<numFactions; i++) {
		const Faction *ptr = Faction::GetFaction(i);
		if( ptr ) {
			s_govDesc[ ptr->govType ].faction = i;
		}
	}
		
}

void Serialize(Serializer::Writer &wr)
{
	s_criminalRecord.Serialize(wr);
	s_outstandingFine.Serialize(wr);
	wr.Int32(s_vplayerPerBlocCrimeRecord.size());
	for (Uint32 i=0; i<s_vplayerPerBlocCrimeRecord.size(); i++) {
		wr.Int64(s_vplayerPerBlocCrimeRecord[i].record);
		wr.Int64(s_vplayerPerBlocCrimeRecord[i].fine);
	}
}

void Unserialize(Serializer::Reader &rd)
{
	Init();
	PersistSystemData<Sint64>::Unserialize(rd, &s_criminalRecord);
	PersistSystemData<Sint64>::Unserialize(rd, &s_outstandingFine);
	const Uint32 blocSize = rd.Int32();
	assert(s_vplayerPerBlocCrimeRecord.size()==blocSize);
	for (Uint32 i=0; i<blocSize; i++) {
		s_vplayerPerBlocCrimeRecord[i].record = rd.Int64();
		s_vplayerPerBlocCrimeRecord[i].fine = rd.Int64();
	}
}

/* The drawbacks of stuffing stuff into integers */
static int GetCrimeIdxFromEnum(enum Crime crime)
{
	assert(crime);
	for (int i = 0; i < 64; ++i) {
		if (crime & 1) return i;
		crime = Crime(crime >> 1); // cast needed because this gets promoted to 'int'
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

	if (s_govDesc[politType].faction != UINT_MAX) {
		const Uint32 b = s_govDesc[politType].faction;
		s_vplayerPerBlocCrimeRecord[b].record |= crimeBitset;
		s_vplayerPerBlocCrimeRecord[b].fine += addFine;
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

	if (s_govDesc[politType].faction != UINT_MAX) {
		const Uint32 b = s_govDesc[politType].faction;
		*crimeBitset = s_vplayerPerBlocCrimeRecord[b].record;
		*fine = s_vplayerPerBlocCrimeRecord[b].fine;
	} else {
		SystemPath path = Pi::game->GetSpace()->GetStarSystem()->GetPath();
		*crimeBitset = s_criminalRecord.Get(path, 0);
		*fine = s_outstandingFine.Get(path, 0);
	}
}

#define POLIT_SEED 0x1234abcd

void GetSysPolitStarSystem(const StarSystem *s, const fixed human_infestedness, SysPolit &outSysPolit)
{
	SystemPath path = s->GetPath();
	const unsigned long _init[5] = { Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), path.systemIndex, POLIT_SEED };
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
			const Faction *pFaction = Faction::GetFaction( s->GetFactionIndex() );
			if( pFaction && GOV_INVALID != pFaction->govType ) {
				// found valid faction
				a = pFaction->govType;
			} else {
				// found an invalid faction, meaning index 0 and thus independent, pick something at random
				a = static_cast<GovType>(rand.Int32(GOV_RAND_MIN, GOV_RAND_MAX));
			}
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
	const unsigned long _init[5] = { Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), path.systemIndex, POLIT_SALT };
	MTRand rand(_init, 5);

	Polit::GovType a = s->GetSysPolit().govType;
	if (a == GOV_NONE) return true;

	const Uint32 b = s_govDesc[a].faction;
	const Faction *ptr = Faction::GetFaction( b );

	if( b != UINT_MAX && ptr ) {
		Faction::EquipProbMap::const_iterator iter = ptr->equip_legality.find(t);
		if( iter != ptr->equip_legality.end() ) {
			Faction::ProbEqualityPair per = (*iter).second;
			if( 0==per.first ) {
				// the probabilty is 0 so we know that this isn't a legal item
				return false;
			} else {
				// choose between equality tests
				if( per.second ) {
					return rand.Int32(per.first)==0;
				} else {
					return rand.Int32(per.first)!=0;
				}
			}
		}
	}
	else
	{
		// this is a non-faction system - do some hardcoded test
		switch (t) {
			case Equip::HAND_WEAPONS:
				return rand.Int32(2) == 0;
			case Equip::BATTLE_WEAPONS:
				return rand.Int32(3)==0;
			case Equip::NERVE_GAS:
				return rand.Int32(10)==0;
			case Equip::NARCOTICS:
				return rand.Int32(2)==0;
			case Equip::SLAVES:
				return rand.Int32(16)==0;
			default: return true;
		}
	}
	return true;
}

}

const char *SysPolit::GetGovernmentDesc() const
{
	return Polit::s_govDesc[govType].description;
}

const char *SysPolit::GetEconomicDesc() const
{
	return Polit::s_econDesc[ Polit::s_govDesc[govType].econ ];
}

const char *SysPolit::GetAllegianceDesc() const
{
	if( Polit::s_govDesc[govType].faction != UINT_MAX ) {
		const Faction *ptr = Faction::GetFaction( Polit::s_govDesc[govType].faction );
		return ptr ? ptr->name.c_str() : Lang::NO_CENTRAL_GOVERNANCE;
	}
	return Polit::s_govDesc[govType].description;
}

