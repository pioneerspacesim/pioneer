-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local CommodityType = require 'CommodityType'

-- the custom cargo
local aluminium_tubes = CommodityType.RegisterCommodity("aluminium_tubes", {
	l10n_key = 'ALUMINIUM_TUBES', price=50,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local art_objects = CommodityType.RegisterCommodity("art_objects", {
	l10n_key = 'ART_OBJECTS', price=200,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local clus = CommodityType.RegisterCommodity("clus", {
	l10n_key = 'CLUS', price=20,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local diamonds = CommodityType.RegisterCommodity("diamonds", {
	l10n_key = 'DIAMONDS', price=300,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local digesters = CommodityType.RegisterCommodity("digesters", {
	l10n_key = 'DIGESTERS', price=10,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local electrical_appliances = CommodityType.RegisterCommodity("electrical_appliances", {
	l10n_key = 'ELECTRICAL_APPLIANCES', price=150,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local explosives = CommodityType.RegisterCommodity("explosives", {
	l10n_key = 'EXPLOSIVES', price=50,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local furniture = CommodityType.RegisterCommodity("furniture", {
	l10n_key = 'FURNITURE', price=15,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local greenhouses = CommodityType.RegisterCommodity("greenhouses", {
	l10n_key = 'GREENHOUSES', price=20,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local hazardous_substances = CommodityType.RegisterCommodity("hazardous_substances", {
	l10n_key = 'HAZARDOUS_SUBSTANCES', price=100,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local machine_tools = CommodityType.RegisterCommodity("machine_tools", {
	l10n_key = 'MACHINE_TOOLS', price=10,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local neptunium = CommodityType.RegisterCommodity("neptunium", {
	l10n_key = 'NEPTUNIUM', price=200,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local plutonium = CommodityType.RegisterCommodity("plutonium", {
	l10n_key = 'PLUTONIUM', price=200,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local semi_finished_products = CommodityType.RegisterCommodity("semi_finished_products", {
	l10n_key = 'SEMI_FINISHED_PRODUCTS', price=10,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local spaceship_parts = CommodityType.RegisterCommodity("spaceship_parts", {
	l10n_key = 'SPACESHIP_PARTS', price=250,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local titanium = CommodityType.RegisterCommodity("titanium", {
	l10n_key = 'TITANIUM', price=150,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local tungsten = CommodityType.RegisterCommodity("tungsten", {
	l10n_key = 'TUNGSTEN', price=125,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local uranium = CommodityType.RegisterCommodity("uranium", {
	l10n_key = 'URANIUM', price=175,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local quibbles = CommodityType.RegisterCommodity("quibbles", {
	l10n_key = 'QUIBBLES', price=1,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local wedding_dresses = CommodityType.RegisterCommodity("wedding_dresses", {
	l10n_key = 'WEDDING_DRESSES', price=15,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})
local stem_bolts = CommodityType.RegisterCommodity("stem_bolts", {
	l10n_key = 'STEM_BOLTS', price=143,
	mass=1, purchasable=false,
	l10n_resource="module-cargorun"
})


-- Adapted from the commodity market. They only share
-- the strings and price from the commodities.
local cargo_chemicals = CommodityType.RegisterCommodity("cargo_chemicals", {
	l10n_key = 'CHEMICALS', price=41,			-- 124 (price in commodities)
	mass=1, purchasable=false,
	l10n_resource="commodity"
})
local cargo_computers = CommodityType.RegisterCommodity("cargo_computers", {
	l10n_key = 'COMPUTERS', price=153,			-- 461
	mass=1, purchasable=false,
	l10n_resource="commodity"
})
local cargo_consumer_goods = CommodityType.RegisterCommodity("cargo_consumer_goods", {
	l10n_key = 'CONSUMER_GOODS', price=82,		-- 246
	mass=1, purchasable=false,
	l10n_resource="commodity"
})
local cargo_farm_machinery = CommodityType.RegisterCommodity("cargo_farm_machinery", {
	l10n_key = 'FARM_MACHINERY', price=120,		-- 419
	mass=1, purchasable=false,
	l10n_resource="commodity"
})
local cargo_fertilizer = CommodityType.RegisterCommodity("cargo_fertilizer", {
	l10n_key = 'FERTILIZER', price=8,			-- 25
	mass=1, purchasable=false,
	l10n_resource="commodity"
})
local cargo_fruit_and_veg = CommodityType.RegisterCommodity("cargo_fruit_and_veg", {
	l10n_key = 'FRUIT_AND_VEG', price=35,		-- 105
	mass=1, purchasable=false,
	l10n_resource="commodity"
})
local cargo_grain = CommodityType.RegisterCommodity("cargo_grain", {
	l10n_key = 'GRAIN', price=8,				-- 41
	mass=1, purchasable=false,
	l10n_resource="commodity"
})
local cargo_industrial_machinery = CommodityType.RegisterCommodity("cargo_industrial_machinery", {
	l10n_key = 'INDUSTRIAL_MACHINERY', price=180,-- 358
	mass=1, purchasable=false,
	l10n_resource="commodity"
})
local cargo_liquor = CommodityType.RegisterCommodity("cargo_liquor", {
	l10n_key = 'LIQUOR', price=69,				-- 422
	mass=1, purchasable=false,
	l10n_resource="commodity"
})
local cargo_medicines = CommodityType.RegisterCommodity("cargo_medicines", {
	l10n_key = 'MEDICINES', price=60,			-- 712
	mass=1, purchasable=false,
	l10n_resource="commodity"
})
local cargo_metal_alloys = CommodityType.RegisterCommodity("cargo_metal_alloys", {
	l10n_key = 'METAL_ALLOYS', price=25,		-- 75
	mass=1, purchasable=false,
	l10n_resource="commodity"
})
local cargo_mining_machinery = CommodityType.RegisterCommodity("cargo_mining_machinery", {
	l10n_key = 'MINING_MACHINERY', price=204,	-- 312
	mass=1, purchasable=false,
	l10n_resource="commodity"
})
local cargo_plastics = CommodityType.RegisterCommodity("cargo_plastics", {
	l10n_key = 'PLASTICS', price=12,			-- 36
	mass=1, purchasable=false,
	l10n_resource="commodity"
})
local cargo_precious_metals = CommodityType.RegisterCommodity("cargo_precious_metals", {
	l10n_key = 'PRECIOUS_METALS', price=300,	-- 2180
	mass=1, purchasable=false,
	l10n_resource="commodity"
})
local cargo_textiles = CommodityType.RegisterCommodity("cargo_textiles", {
	l10n_key = 'TEXTILES', price=30,			-- 91
	mass=1, purchasable=false,
	l10n_resource="commodity"
})


local chemical = {
	cargo_chemicals,
	cargo_fertilizer,
	digesters,
	hazardous_substances
}

local mining = {
	cargo_mining_machinery,
	clus,
	explosives
}

local hardware = {
	aluminium_tubes,
	cargo_farm_machinery,
	cargo_industrial_machinery,
	cargo_metal_alloys,
	cargo_mining_machinery,
	cargo_plastics,
	cargo_precious_metals,
	diamonds,
	hazardous_substances,
	machine_tools,
	neptunium,
	plutonium,
	semi_finished_products,
	spaceship_parts,
	stem_bolts,
	titanium,
	tungsten,
	uranium
}

local infrastructure = {
	clus,
	explosives,
	greenhouses
}

local consumer_goods = {
	cargo_chemicals,
	cargo_computers,
	cargo_consumer_goods,
	cargo_fertilizer,
	cargo_fruit_and_veg,
	cargo_grain,
	cargo_liquor,
	cargo_medicines,
	cargo_plastics,
	cargo_textiles,
	electrical_appliances,
	furniture,
	spaceship_parts
}

local expensive = { -- price >= 175
	art_objects,
	cargo_industrial_machinery,
	cargo_mining_machinery,
	cargo_precious_metals,
	diamonds,
	neptunium,
	plutonium,
	spaceship_parts,
	uranium
}

local fluffy = {
	quibbles
}

local wedding = {
	wedding_dresses
}

local art = {
	art_objects
}

local gems = {
	diamonds
}

local radioactive = {
	neptunium,
	plutonium,
	uranium
}

local centrifuges = {
	aluminium_tubes
}

local custom_cargo = {
	{ bkey = "CHEMICAL", goods = chemical, weight = 0 },
	{ bkey = "MINING", goods = mining, weight = 0 },
	{ bkey = "HARDWARE", goods = hardware, weight = 0 },
	{ bkey = "INFRASTRUCTURE", goods = infrastructure, weight = 0 },
	{ bkey = "CONSUMER_GOODS", goods = consumer_goods, weight = 0 },
	{ bkey = "EXPENSIVE", goods = expensive, weight = 0 },
	{ bkey = "FLUFFY", goods = fluffy, weight = 0 },
	{ bkey = "WEDDING" , goods = wedding, weight = 0 },
	{ bkey = "ART", goods = art, weight = 0 },
	{ bkey = "GEMS", goods = gems, weight = 0 },
	{ bkey = "RADIOACTIVE", goods = radioactive, weight = 0 },
	{ bkey = "CENTRIFUGES", goods = centrifuges, weight = 0 }
}

return custom_cargo
