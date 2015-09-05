// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Pi.h"
#include "Polit.h"
#include "galaxy/Galaxy.h"
#include "galaxy/StarSystem.h"
#include "galaxy/Sector.h"
#include "galaxy/Economy.h"
#include "Factions.h"
#include "Space.h"
#include "Ship.h"
#include "ShipCpanel.h"
#include "SpaceStation.h"
#include "PersistSystemData.h"
#include "Lang.h"
#include "StringF.h"
#include "Game.h"

namespace Polit {


static PersistSystemData<Sint64> s_criminalRecord;
static PersistSystemData<Sint64> s_outstandingFine;
struct crime_t {
	crime_t() : record(0), fine(0) {}
	Sint64 record;
	Sint64 fine;
};
static std::vector<crime_t> s_playerPerBlocCrimeRecord;

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
	PolitEcon econ;
	fixed baseLawlessness;
};
static politDesc_t s_govDesc[GOV_MAX] = {
	{ "<invalid turd>",							0,		ECON_NONE,				fixed(1,1) },
	{ Lang::NO_CENTRAL_GOVERNANCE,				0,		ECON_NONE,				fixed(1,1) },
	{ Lang::EARTH_FEDERATION_COLONIAL_RULE,		2,		ECON_CAPITALIST,		fixed(3,10) },
	{ Lang::EARTH_FEDERATION_DEMOCRACY,			3,		ECON_CAPITALIST,		fixed(15,100) },
	{ Lang::IMPERIAL_RULE,						3,		ECON_PLANNED,			fixed(15,100) },
	{ Lang::LIBERAL_DEMOCRACY,					2,		ECON_CAPITALIST,		fixed(25,100) },
	{ Lang::SOCIAL_DEMOCRACY,					2,		ECON_MIXED,				fixed(20,100) },
	{ Lang::LIBERAL_DEMOCRACY,					2,		ECON_CAPITALIST,		fixed(25,100) },
	{ Lang::CORPORATE_SYSTEM,					2,		ECON_CAPITALIST,		fixed(40,100) },
	{ Lang::SOCIAL_DEMOCRACY,					2,		ECON_MIXED,				fixed(25,100) },
	{ Lang::MILITARY_DICTATORSHIP,				5,		ECON_CAPITALIST,		fixed(40,100) },
	{ Lang::MILITARY_DICTATORSHIP,				6,		ECON_CAPITALIST,		fixed(25,100) },
	{ Lang::MILITARY_DICTATORSHIP,				6,		ECON_MIXED,				fixed(25,100) },
	{ Lang::MILITARY_DICTATORSHIP,				5,		ECON_MIXED,				fixed(40,100) },
	{ Lang::COMMUNIST,							10,		ECON_PLANNED,			fixed(25,100) },
	{ Lang::PLUTOCRATIC_DICTATORSHIP,			4,		ECON_VERY_CAPITALIST,	fixed(45,100) },
	{ Lang::VIOLENT_ANARCHY,					2,		ECON_NONE,				fixed(90,100) },
};

void Init(RefCountedPtr<Galaxy> galaxy)
{
	s_criminalRecord.Clear();
	s_outstandingFine.Clear();

	// setup the per faction criminal records
	const Uint32 numFactions = galaxy->GetFactions()->GetNumFactions();
	s_playerPerBlocCrimeRecord.clear();
	s_playerPerBlocCrimeRecord.resize( numFactions );
}

void ToJson(Json::Value &jsonObj)
{
	Json::Value politObj(Json::objectValue); // Create JSON object to contain polit data.

	Json::Value criminalRecordObj(Json::objectValue); // Create JSON object to contain criminal record data.
	s_criminalRecord.ToJson(criminalRecordObj);
	politObj["criminal_record"] = criminalRecordObj; // Add criminal record object to polit object.

	Json::Value outstandingFineObj(Json::objectValue); // Create JSON object to contain outstanding fine data.
	s_outstandingFine.ToJson(outstandingFineObj);
	politObj["outstanding_fine"] = outstandingFineObj; // Add outstanding fine object to polit object.

	Json::Value crimeRecordArray(Json::arrayValue); // Create JSON array to contain crime record data.
	for (Uint32 i = 0; i < s_playerPerBlocCrimeRecord.size(); i++)
	{
		Json::Value crimeRecordArrayEl(Json::objectValue); // Create JSON object to contain crime record element.
		crimeRecordArrayEl["record"] = SInt64ToStr(s_playerPerBlocCrimeRecord[i].record);
		crimeRecordArrayEl["fine"] = SInt64ToStr(s_playerPerBlocCrimeRecord[i].fine);
		crimeRecordArray.append(crimeRecordArrayEl); // Append crime record object to array.
	}
	politObj["crime_record"] = crimeRecordArray; // Add crime record array to polit object.

	jsonObj["polit"] = politObj; // Add polit object to supplied object.
}

void FromJson(const Json::Value &jsonObj, RefCountedPtr<Galaxy> galaxy)
{
	Init(galaxy);

	if (!jsonObj.isMember("polit")) throw SavedGameCorruptException();
	Json::Value politObj = jsonObj["polit"];

	if (!politObj.isMember("criminal_record")) throw SavedGameCorruptException();
	if (!politObj.isMember("outstanding_fine")) throw SavedGameCorruptException();
	if (!politObj.isMember("crime_record")) throw SavedGameCorruptException();

	Json::Value criminalRecordObj = politObj["criminal_record"];
	PersistSystemData<Sint64>::FromJson(criminalRecordObj, &s_criminalRecord);

	Json::Value outstandingFineObj = politObj["outstanding_fine"];
	PersistSystemData<Sint64>::FromJson(outstandingFineObj, &s_outstandingFine);

	Json::Value crimeRecordArray = politObj["crime_record"];
	if (!crimeRecordArray.isArray()) throw SavedGameCorruptException();
	assert(s_playerPerBlocCrimeRecord.size() == crimeRecordArray.size());
	for (Uint32 i = 0; i < s_playerPerBlocCrimeRecord.size(); i++)
	{
		Json::Value crimeRecordArrayEl = crimeRecordArray[i];
		if (!crimeRecordArrayEl.isMember("record")) throw SavedGameCorruptException();
		if (!crimeRecordArrayEl.isMember("fine")) throw SavedGameCorruptException();
		s_playerPerBlocCrimeRecord[i].record = StrToSInt64(crimeRecordArrayEl["record"].asString());
		s_playerPerBlocCrimeRecord[i].fine = StrToSInt64(crimeRecordArrayEl["fine"].asString());
	}
}

fixed GetBaseLawlessness(GovType gov) {
	return s_govDesc[gov].baseLawlessness;
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
		Pi::game->log->Add(station->GetLabel(),
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
	const Faction *faction = (Pi::game->GetSpace()->GetStarSystem()->GetFaction());

	if (faction->IsValid()) {
		s_playerPerBlocCrimeRecord[faction->idx].record |= crimeBitset;
		s_playerPerBlocCrimeRecord[faction->idx].fine   += addFine;
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

	const Faction *faction = Pi::game->GetSpace()->GetStarSystem()->GetFaction();

	if (faction->IsValid()) {
		*crimeBitset = s_playerPerBlocCrimeRecord[faction->idx].record;
		*fine        = s_playerPerBlocCrimeRecord[faction->idx].fine;
	} else {
		SystemPath path = Pi::game->GetSpace()->GetStarSystem()->GetPath();
		*crimeBitset = s_criminalRecord.Get(path, 0);
		*fine = s_outstandingFine.Get(path, 0);
	}
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
