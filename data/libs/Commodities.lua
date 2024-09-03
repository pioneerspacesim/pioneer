-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local CommodityType = require 'CommodityType'
local Economy       = require 'Economy'

--
-- Interface: Commodities
--
-- Commodities is a module that wraps the <CommodityType> registry for easy access.
--
-- It automatically registers all commodities defined in the economy JSON files,
-- and any commodity in the game can be retrieved by indexing the table with the
-- name of the commodity in questino.
--

local Commodities = CommodityType.registry

-- TODO: normalize icon name conventions; ideally we don't need this table at all and can simply
-- use the commodity name for the icon
local icon_names = {
	hydrogen = "Hydrogen",
	liquid_oxygen = "Liquid_Oxygen",
	water = "Water",
	carbon_ore = "Carbon_ore",
	metal_ore = "Metal_ore",
	metal_alloys = "Metal_alloys",
	precious_metals = "Precious_metals",
	plastics = "Plastics",
	fruit_and_veg = "Fruit_and_Veg",
	animal_meat = "Animal_Meat",
	live_animals = "Live_Animals",
	liquor = "Liquor",
	grain = "Grain",
	slaves = "Slaves",
	textiles = "Textiles",
	fertilizer = "Fertilizer",
	medicines = "Medicines",
	consumer_goods = "Consumer_goods",
	computers = "Computers",
	rubbish = "Rubbish",
	radioactives = "Radioactive_waste",
	narcotics = "Narcotics",
	nerve_gas = "Nerve_Gas",
	military_fuel = "Military_fuel",
	robots = "Robots",
	hand_weapons = "Hand_weapons",
	air_processors = "Air_processors",
	farm_machinery = "Farm_machinery",
	mining_machinery = "Mining_machinery",
	battle_weapons = "Battle_weapons",
	industrial_machinery = "Industrial_machinery"
}

local economies = Economy.GetEconomies()

for name, commodity in pairs(Economy.GetCommodities()) do
	local econ = commodity.producer > 0 and economies[commodity.producer] or {}

	local required_life_support_level = nil
	-- TODO: define this in commodity JSON files rather than explicit callouts
	if name == "live_animals" or name == "slaves" then
		required_life_support_level = 1
	end

	CommodityType.RegisterCommodity(name, {
		l10n_key = commodity.l10n_key,
		l10n_resource = nil,
		price = commodity.price,
		mass = 1,
		life_support = required_life_support_level,
		economy_type = econ.name,
		purchasable = true,
		icon_name = icon_names[name] or ""
	})
end

return Commodities
