// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "galaxy/Economy.h"
#include "profiler/Profiler.h"

#include "FileSystem.h"
#include "Json.h"
#include "JsonUtils.h"
#include "Lang.h"
#include "utils.h"

#include <set>
#include <stdexcept>

namespace GalacticEconomy {

	std::vector<CommodityInfo> m_commodities;
	std::vector<EconomyInfo> m_economies;

	/* clang-format off */
	// A dummy CommodityInfo structure for use in debugging bad EconomyIds and other foolishness.
	CommodityInfo null_commodity = {
		InvalidCommodityId,
		InvalidEconomyId,
		"NULL_COMMODITY",
		"NULL_COMMODITY",
		{},
		0.0
	};

	const char *null_economy_name = "NULL_ECONOMY";

	// A dummy EconomyInfo structure for use in debugging bad EconomyIds and other foolishness.
	EconomyInfo null_economy = {
		InvalidEconomyId,
		null_economy_name,
		{
			null_economy_name,
			null_economy_name,
			null_economy_name,
			null_economy_name
		},
		{},
		{}
	};
	/* clang-format on */

	std::map<CommodityId, ConsumableInfo> m_consumables;

	std::map<CommodityId, IllegalInfo> m_illegalCommodities;

	// map commodity names to commodity ids for loading/saving
	// references to set/map contents are never invalidated except when pointing to deleted elements
	std::map<std::string, CommodityId> m_commodityNameLookup;

	// map economy names to economy ids for loading/saving
	std::map<std::string, EconomyId> m_economyNameLookup;

	// store the actual string data pointed to by economy / commodity names and lang keys
	std::set<std::string> m_string_data;

	const char *get_lang_key(const Json &j)
	{
		std::string key = j;
		return m_string_data.emplace(key).first->c_str();
	}

	void from_json(const Json &j, CommodityInfo &out)
	{
		// id and name are set by LoadEconomyData

		out.l10n_key = get_lang_key(j["l10n_key"]);

		// this depends on m_commodityNameLookup being fully populated by looping over all commodity names
		// and pre-assigning CommodityIds for them.
		const Json &inputs = j["inputs"];
		if (inputs.is_array() && inputs.size() > 0) {
			for (size_t idx = 0; idx < inputs.size() && idx < CommodityInfo::MAX_ECON_INPUTS; idx++) {
				const Json &input = inputs[idx];

				std::string name;
				fixed amount = fixed(1, 1);

				if (input.is_array()) {
					name = input[0];
					amount = input[1];
				} else {
					name = input;
				}

				CommodityId inputCommodity = GetCommodityByName(name);
				if (inputCommodity == InvalidCommodityId)
					Output("warning: commodity %s has invalid input commodity %d (%s)\n", out.name, int(idx), name.c_str());

				out.inputs[idx] = {
					inputCommodity, amount
				};
			}
		}

		// Commodities must be loaded after economy data
		std::string economy_key = j["producer"];
		out.producer = GetEconomyByName(economy_key);

		if (out.producer == InvalidEconomyId)
			Output("warning: commodity %s has invalid producing economy %s\n", out.name, economy_key.c_str());

		out.price = j["price"];
	}

	void from_json(const Json &j, EconomyInfo &out)
	{
		// id and name are set by LoadEconomyData

		const Json &desc = j["description"];
		out.l10n_key.small = get_lang_key(desc["small"]);
		out.l10n_key.medium = get_lang_key(desc["medium"]);
		out.l10n_key.large = get_lang_key(desc["large"]);
		out.l10n_key.huge = get_lang_key(desc["huge"]);

		const Json &affinity = j["affinity"];
		out.affinity.agricultural = affinity["agricultural"];
		out.affinity.industrial = affinity["industrial"];
		out.affinity.metallicity = affinity["metallicity"];

		const Json &generation = j["generation"];
		out.generation.agricultural = generation["agricultural"];
		out.generation.industrial = generation["industrial"];
		out.generation.metallicity = generation["metallicity"];
		out.generation.population = generation["population"];
		out.generation.random = generation["random"];
	}

	void from_json(const Json &j, ConsumableInfo &out)
	{
		const Json &random = j["random"];
		out.random_consumption[0] = random[0];
		out.random_consumption[1] = random[1];
		out.locally_produced_min = j.value("locally_produced_min", fixed(1, 1));
	}

	void LoadCommodityData()
	{
		PROFILE_SCOPED()
		m_commodities.clear();
		m_commodityNameLookup.clear();

		std::vector<FileSystem::FileInfo> files;
		FileSystem::gameDataFiles.ReadDirectory("economy/commodities", files);

		std::vector<Json> commodity_datas;
		for (const auto &file : files) {
			if (!file.IsFile()) continue;

			commodity_datas.push_back(JsonUtils::LoadJsonDataFile(file.GetPath()));
		}

		// Build the commodity name -> CommodityId mapping
		try {
			CommodityId idx = 1; // first valid index is 1.
			for (const Json &data : commodity_datas) {
				const Json &commodities = data["commodities"];
				for (const auto &it : commodities.items()) {
					// Don't load the full commodity info here, just set up the name and index correctly.
					CommodityInfo value = {};
					value.name = m_string_data.emplace(it.key()).first->c_str();
					value.id = idx++;
					m_commodities.push_back(value);
					m_commodityNameLookup.emplace(it.key(), m_commodities.size());
				}
			}
		} catch (Json::type_error &e) {
			throw std::runtime_error(std::string("error building commodity index") + e.what());
		}

		// Load commodity data now that all commodities have Ids assigned.
		try {
			for (const Json &data : commodity_datas) {
				const Json &commodities = data["commodities"];
				for (const auto &t : commodities.items()) {
					CommodityId id = m_commodityNameLookup.at(t.key());
					CommodityInfo &value = m_commodities[id - 1];
					from_json(t.value(), value);

					Output("INFO: commodity %s (%d) loaded (producer: %d, price: %.1f, inputs: %d:%.1f, %d:%.1f)\n",
						value.name, value.id, value.producer, value.price,
						value.inputs[0].first, value.inputs[0].second.ToDouble(),
						value.inputs[1].first, value.inputs[1].second.ToDouble());
				}
			}

		} catch (Json::type_error &e) {
			throw std::runtime_error(std::string("error building commodity data") + e.what());
		}
	}

	void LoadConsumableData()
	{
		PROFILE_SCOPED()
		Json file = JsonUtils::LoadJsonDataFile("economy/consumables.json");
		const Json &consumables = file["consumables"];

		if (!consumables.is_object()) {
			Output("warning: could not load consumable commodities list from file economy/consumables.json\n");
			return;
		}

		for (const auto &pair : consumables.items()) {
			CommodityId id = GetCommodityByName(pair.key());
			if (id == InvalidCommodityId) {
				Output("warning: [consumables] commodity %s does not exist\n", pair.key().c_str());
				continue;
			}

			ConsumableInfo value;
			from_json(pair.value(), value);
			m_consumables.emplace(id, value);

			Output("INFO: loaded consumable data for commodity %s (%d) -> (%d-%d, %.2f)\n",
				pair.key().c_str(), id,
				value.random_consumption[0], value.random_consumption[1],
				value.locally_produced_min.ToDouble());
		}
	}

	void LoadIllegalCommodityData()
	{
		PROFILE_SCOPED()
		Json file = JsonUtils::LoadJsonDataFile("economy/illegal.json");
		const Json &commodities = file["commodities"];

		if (!commodities.is_object()) {
			Output("warning: could not load illegal commodities list from file economy/illegal.json\n");
			return;
		}

		for (const auto &pair : commodities.items()) {
			CommodityId id = GetCommodityByName(pair.key());
			if (id == InvalidCommodityId) {
				Output("warning: [illegals] commodity %s does not exist\n", pair.key().c_str());
				continue;
			}

			IllegalInfo value;
			value.id = id;
			const Json &chance = pair.value()["chance"];
			if (!chance.is_array())
				value.chance = { 1, 2 };
			else
				value.chance = { chance[0].get<uint32_t>(), chance[1].get<uint32_t>() };

			m_illegalCommodities.emplace(id, value);
			Output("INFO: loaded illegal data for commodity %s (%d) (%d in %d)\n",
				pair.key().c_str(), id, value.chance[0], value.chance[1]);
		}
	}

	void LoadEconomyData()
	{
		PROFILE_SCOPED()
		const Json economy = JsonUtils::LoadJsonDataFile("economy/economies.json");
		try {
			for (const auto &el : economy.items()) {
				std::string key = el.key();
				EconomyInfo val = el.value();

				if (m_economyNameLookup.count(key)) {
					char err[256] = {};
					snprintf(err, 256, "double-definition of economy %s in economy/economies.json\n", key.c_str());
					throw std::runtime_error(err);
				}

				m_string_data.emplace(key);
				EconomyId id = m_economies.size() + 1;
				m_economyNameLookup[key] = id;

				val.id = id;
				val.name = m_string_data.emplace(key).first->c_str();
				m_economies.push_back(val);
				Output("INFO: economy %s -> id %d (generation: %.2f %.2f %.2f %.2f %.2f)\n",
					val.name, val.id, val.generation.agricultural.ToDouble(), val.generation.industrial.ToDouble(),
					val.generation.metallicity.ToDouble(), val.generation.population.ToDouble(), val.generation.population.ToDouble());
			}

		} catch (Json::type_error &e) {
			throw std::runtime_error(std::string("Error loading economy/economies.json: ") + e.what());
		}
	}

	void Init()
	{
		PROFILE_SCOPED()

		LoadEconomyData();
		LoadCommodityData();
		LoadConsumableData();
		LoadIllegalCommodityData();

		printf("\n");
	}

	void FromJson(const Json &obj)
	{
	}

	void ToJson(Json &obj)
	{
		Json economy{};

		Json commodityArray = Json::object();
		for (const auto &t : m_commodities) {
			commodityArray.emplace(t.name, t.id);
		}
		economy["commodity"] = commodityArray;

		Json economyArray = Json::object();
		for (const auto &t : m_economies) {
			economyArray.emplace(t.name, t.id);
		}
		economy["economy"] = economyArray;

		obj["economyData"] = economy;
	}

	const std::vector<CommodityInfo> &Commodities()
	{
		return m_commodities;
	}

	const std::vector<EconomyInfo> &Economies()
	{
		return m_economies;
	}

	const std::map<CommodityId, ConsumableInfo> &Consumables()
	{
		return m_consumables;
	}

	const std::map<CommodityId, IllegalInfo> &IllegalCommodities()
	{
		return m_illegalCommodities;
	}

	const CommodityInfo &GetCommodityById(CommodityId Id)
	{
		return Id && Id <= m_commodities.size() ?
			m_commodities[Id - 1] :
			null_commodity;
	}

	const EconomyInfo &GetEconomyById(EconomyId Id)
	{
		return Id && Id <= m_commodities.size() ?
			m_economies[Id - 1] :
			null_economy;
	}

	CommodityId GetCommodityByName(const std::string &name)
	{
		return m_commodityNameLookup.count(name) ? m_commodityNameLookup.at(name) : 0;
	}

	EconomyId GetEconomyByName(const std::string &name)
	{
		return m_economyNameLookup.count(name) ? m_economyNameLookup.at(name) : 0;
	}

} // namespace GalacticEconomy
