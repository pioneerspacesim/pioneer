-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Economy = require 'Economy'
local EquipType = require 'EquipType'

local CommodityType = require 'CommodityType'

local commodities = Economy.GetCommodities()
local economies = Economy.GetEconomies()

local cargo = EquipType.cargo

local CARGOLANGRESOURCE = "commodity"

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

-- TODO: don't create a separate EquipType instance for each commoditity,
-- instead store cargo separately from ship equipment
for name, commodity in pairs(commodities) do
	local econ = commodity.producer > 0 and economies[commodity.producer] or {}
	cargo[commodity.name] = EquipType.EquipType.New({
		name=commodity.name, slots="cargo",
		l10n_key=commodity.l10n_key, l10n_resource=CARGOLANGRESOURCE,
		price=commodity.price, capabilities={mass=1}, economy_type=econ.name,
		purchasable=true, icon_name=icon_names[commodity.name] or ""
	})

	CommodityType.RegisterCommodity(name, {
		l10n_key = commodity.l10n_key,
		l10n_resource = nil,
		price = commodity.price,
		mass = 1,
		economy_type = econ.name,
		purchasable = true,
		icon_name = icon_names[name] or ""
	})
end

return Commodities
