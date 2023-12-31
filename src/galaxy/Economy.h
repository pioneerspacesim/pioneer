// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GALAXY_ECONOMY_H
#define GALAXY_ECONOMY_H

#include "JsonFwd.h"
#include "fixed.h"

#include <array>
#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace GalacticEconomy {
	// Loads JSON files containing information about commodities.
	void Init();

	// Call at the start of loading a game, before any other code makes references to commodity Ids.
	// Loads and restores the commodity ID mappings in the saved game
	// Commodity/EconomyIds returned by economy functions are only guaranteed to be correct with
	// respect to the saved state between a FromJson() and ToJson() call.
	void LoadFromJson(const Json &obj);

	// Call when saving a game, serializes out name -> CommodityId mappings
	void SaveToJson(Json &obj);

	using CommodityId = uint32_t;
	static const CommodityId InvalidCommodityId = 0;
	using EconomyId = uint32_t;
	static const EconomyId InvalidEconomyId = 0;

	// Information about a particular commodity type
	struct CommodityInfo {
		using Input = std::pair<CommodityId, fixed>;
		static const int MAX_ECON_INPUTS = 2;

		CommodityId id;
		EconomyId producer;

		const char *name;
		const char *l10n_key;

		// production requirement. eg metal alloys input would be metal ore
		// (used in trade balance calculations)
		std::array<Input, MAX_ECON_INPUTS> inputs;
		float price;

		// The chance this commodity is legal in a random system, expressed as chance[0] / chance[1]
		fixed default_legality;
	};

	// Information about a particular economy type.
	struct EconomyInfo {
		EconomyId id;
		const char *name;

		struct TranslationKeys {
			const char *small;
			const char *medium;
			const char *large;
			const char *huge;
		} l10n_key;

		struct CommodityAffinity {
			fixed agricultural;
			fixed industrial;
			fixed metallicity;
		} affinity;

		struct GenerationAffinity {
			fixed agricultural;
			fixed industrial;
			fixed metallicity;
			fixed population;
			fixed random;
		} generation;
	};

	struct ConsumableInfo {
		std::array<uint32_t, 2> random_consumption;
		fixed locally_produced_min;
	};

	// A list of all defined commodities
	const std::vector<CommodityInfo> &Commodities();

	// A list of all defined economies.
	const std::vector<EconomyInfo> &Economies();

	// Commodities consumed by populated stations / worlds
	const std::map<CommodityId, ConsumableInfo> &Consumables();

	// Returns a reference to a null CommodityInfo structure if passed InvalidCommodityId
	const CommodityInfo &GetCommodityById(CommodityId Id);

	// Returns a reference to a null EconomyInfo structure if passed InvalidEconomyId
	const EconomyInfo &GetEconomyById(EconomyId Id);

	// Returns InvalidCommodityId if there is no commodity by that name
	CommodityId GetCommodityByName(const std::string &name);

	// Returns InvalidEconomyId if there is no economy by that name
	EconomyId GetEconomyByName(const std::string &name);

} // namespace GalacticEconomy

#endif
