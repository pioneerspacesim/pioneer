-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local EquipTypes = require '.Types'
local Equipment = require 'Equipment'

local EquipType = EquipTypes.EquipType
local SensorType = EquipTypes.SensorType
local CabinType = EquipTypes.CabinType
local ShieldType = EquipTypes.ShieldType
local ThrusterType = EquipTypes.ThrusterType
local CargoScoopType = EquipTypes.CargoScoopType

--===============================================
-- Computer Modules
--===============================================

Equipment.Register("misc.autopilot", EquipType.New {
	l10n_key="AUTOPILOT",
	price=1400, purchasable=true, tech_level=1,
	slot = { type="computer.autopilot", size=1 },
	mass=0.2, volume=0.5, capabilities = { set_speed=1, autopilot=1 },
	icon_name="equip_autopilot"
})

Equipment.Register("misc.trade_computer", EquipType.New {
	l10n_key="TRADE_COMPUTER",
	price=400, purchasable=true, tech_level=9,
	slot={ type="computer", size=1 },
	mass=0.2, volume=0.5, capabilities={ trade_computer=1 },
	icon_name="equip_trade_computer"
})

--===============================================
-- Sensors
--===============================================

Equipment.Register("sensor.radar", SensorType.New {
	l10n_key="RADAR",
	price=680, purchasable=true, tech_level=3,
	slot = { type="sensor.radar", size=1 },
	mass=1.0, volume=1.0, capabilities = { radar=1 },
	icon_name="equip_radar"
})

--===============================================
-- Shield Generators
--===============================================

Equipment.Register("shield.basic_s1", ShieldType.New {
	l10n_key="SHIELD_GENERATOR",
	price=2500, purchasable=true, tech_level=5,
	slot = { type="shield", size=1 },
	mass=1, volume=2, capabilities = { shield=1 },
	icon_name="equip_shield_generator"
})

Equipment.Register("shield.basic_s2", ShieldType.New {
	l10n_key="SHIELD_GENERATOR",
	price=5500, purchasable=true, tech_level=7,
	slot = { type="shield", size=2 },
	mass=2.8, volume=5, capabilities = { shield=2 },
	icon_name="equip_shield_generator"
})

Equipment.Register("shield.basic_s3", ShieldType.New {
	l10n_key="SHIELD_GENERATOR",
	price=11500, purchasable=true, tech_level=8,
	slot = { type="shield", size=3 },
	mass=7, volume=12.5, capabilities = { shield=3 },
	icon_name="equip_shield_generator"
})

Equipment.Register("shield.basic_s4", ShieldType.New {
	l10n_key="SHIELD_GENERATOR",
	price=23500, purchasable=true, tech_level=9,
	slot = { type="shield", size=4 },
	mass=17.9, volume=32, capabilities = { shield=4 },
	icon_name="equip_shield_generator"
})

Equipment.Register("shield.basic_s5", ShieldType.New {
	l10n_key="SHIELD_GENERATOR",
	price=58500, purchasable=true, tech_level=10,
	slot = { type="shield", size=5 },
	mass=43.7, volume=78, capabilities = { shield=5 },
	icon_name="equip_shield_generator"
})

--===============================================
-- Structure Modifications
--===============================================

Equipment.Register("struct.reinforced_structure_s1", EquipType.New {
	l10n_key="REINFORCED_STRUCTURE",
	price=1200, purchasable=true, tech_level=4,
	slot = { type="structure", size=1 },
	mass=5, volume=2, capabilities = { atmo_shield=6 },
	icon_name="equip_atmo_shield_generator"
})

Equipment.Register("struct.reinforced_structure_s2", EquipType.New {
	l10n_key="REINFORCED_STRUCTURE",
	price=2800, purchasable=true, tech_level=5,
	slot = { type="structure", size=2 },
	mass=10, volume=5, capabilities = { atmo_shield=6 },
	icon_name="equip_atmo_shield_generator"
})

Equipment.Register("struct.reinforced_structure_s3", EquipType.New {
	l10n_key="REINFORCED_STRUCTURE",
	price=7560, purchasable=true, tech_level=6,
	slot = { type="structure", size=3 },
	mass=22, volume=12.5, capabilities = { atmo_shield=6 },
	icon_name="equip_atmo_shield_generator"
})

Equipment.Register("struct.reinforced_structure_s4", EquipType.New {
	l10n_key="REINFORCED_STRUCTURE",
	price=20410, purchasable=true, tech_level=7,
	slot = { type="structure", size=4 },
	mass=46, volume=31, capabilities = { atmo_shield=6 },
	icon_name="equip_atmo_shield_generator"
})

Equipment.Register("struct.reinforced_structure_s5", EquipType.New {
	l10n_key="REINFORCED_STRUCTURE",
	price=55100, purchasable=true, tech_level=8,
	slot = { type="structure", size=5 },
	mass=96.5, volume=77, capabilities = { atmo_shield=6 },
	icon_name="equip_atmo_shield_generator"
})

--===============================================
-- Hull Modifications
--===============================================

Equipment.Register("hull.atmospheric_shielding_s0", EquipType.New {
	l10n_key="ATMOSPHERIC_SHIELDING",
	price=500, purchasable=true, tech_level=4,
	slot = { type="hull.atmo_shield", size=0 },
	mass=0.6, volume=1, capabilities = { atmo_shield=4 },
	icon_name="equip_atmo_shield_generator"
})

Equipment.Register("hull.atmospheric_shielding_s1", EquipType.New {
	l10n_key="ATMOSPHERIC_SHIELDING",
	price=800, purchasable=true, tech_level=4,
	slot = { type="hull.atmo_shield", size=1 },
	mass=1, volume=3, capabilities = { atmo_shield=4 },
	icon_name="equip_atmo_shield_generator"
})

Equipment.Register("hull.atmospheric_shielding_s2", EquipType.New {
	l10n_key="ATMOSPHERIC_SHIELDING",
	price=2100, purchasable=true, tech_level=5,
	slot = { type="hull.atmo_shield", size=2 },
	mass=1.8, volume=5.5, capabilities = { atmo_shield=4 },
	icon_name="equip_atmo_shield_generator"
})

Equipment.Register("hull.atmospheric_shielding_s3", EquipType.New {
	l10n_key="ATMOSPHERIC_SHIELDING",
	price=5800, purchasable=true, tech_level=5,
	slot = { type="hull.atmo_shield", size=3 },
	mass=3.8, volume=11.5, capabilities = { atmo_shield=4 },
	icon_name="equip_atmo_shield_generator"
})

Equipment.Register("hull.atmospheric_shielding_s4", EquipType.New {
	l10n_key="ATMOSPHERIC_SHIELDING",
	price=15740, purchasable=true, tech_level=6,
	slot = { type="hull.atmo_shield", size=4 },
	mass=8, volume=24, capabilities = { atmo_shield=4 },
	icon_name="equip_atmo_shield_generator"
})

Equipment.Register("hull.atmospheric_shielding_s5", EquipType.New {
	l10n_key="ATMOSPHERIC_SHIELDING",
	price=42510, purchasable=true, tech_level=7,
	slot = { type="hull.atmo_shield", size=5 },
	mass=17, volume=43, capabilities = { atmo_shield=4 },
	icon_name="equip_atmo_shield_generator"
})

Equipment.Register("hull.heavy_atmospheric_shielding_s2", EquipType.New {
	l10n_key="ATMOSPHERIC_SHIELDING_HEAVY",
	price=2920, purchasable=true, tech_level=7,
	slot = { type="hull.atmo_shield", size=2 },
	mass=5, volume=6.5, capabilities = { atmo_shield=9 },
	icon_name="equip_atmo_shield_generator"
})

Equipment.Register("hull.heavy_atmospheric_shielding_s3", EquipType.New {
	l10n_key="ATMOSPHERIC_SHIELDING_HEAVY",
	price=7900, purchasable=true, tech_level=7,
	slot = { type="hull.atmo_shield", size=3 },
	mass=10.5, volume=13.5, capabilities = { atmo_shield=9 },
	icon_name="equip_atmo_shield_generator"
})

Equipment.Register("hull.heavy_atmospheric_shielding_s4", EquipType.New {
	l10n_key="ATMOSPHERIC_SHIELDING_HEAVY",
	price=21330, purchasable=true, tech_level=7,
	slot = { type="hull.atmo_shield", size=3 },
	mass=22, volume=28, capabilities = { atmo_shield=9 },
	icon_name="equip_atmo_shield_generator"
})

Equipment.Register("hull.hull_autorepair_s2", EquipType.New {
	l10n_key="HULL_AUTOREPAIR", slots="hull_autorepair",
	price=6250, purchasable=true, tech_level="MILITARY",
	slot = { type="hull.autorepair", size=2 },
	mass=13.5, volume=18, capabilities={ hull_autorepair=1 },
	icon_name="repairs"
})

Equipment.Register("hull.hull_autorepair_s3", EquipType.New {
	l10n_key="HULL_AUTOREPAIR", slots="hull_autorepair",
	price=16000, purchasable=true, tech_level="MILITARY",
	slot = { type="hull.autorepair", size=3 },
	mass=30, volume=40, capabilities={ hull_autorepair=1 },
	icon_name="repairs"
})

Equipment.Register("hull.hull_autorepair_s4", EquipType.New {
	l10n_key="HULL_AUTOREPAIR", slots="hull_autorepair",
	price=40960, purchasable=true, tech_level="MILITARY",
	slot = { type="hull.autorepair", size=4 },
	mass=64.5, volume=85, capabilities={ hull_autorepair=1 },
	icon_name="repairs"
})

Equipment.Register("hull.hull_autorepair_s5", EquipType.New {
	l10n_key="HULL_AUTOREPAIR", slots="hull_autorepair",
	price=16000, purchasable=true, tech_level="MILITARY",
	slot = { type="hull.autorepair", size=5 },
	mass=138, volume=185, capabilities={ hull_autorepair=1 },
	icon_name="repairs"
})

--===============================================
-- Thruster Mods
--===============================================

-- S1 thrusters
Equipment.Register("thruster.default_s1", ThrusterType.New {
	l10n_key="THRUSTERS_DEFAULT", slots="thruster",
	price=120, purchasable=true, tech_level=2,
	slot = { type="thruster", size=1 },
	mass=0, volume=0, capabilities={ thruster_power=0 },
	icon_name="equip_thrusters_basic"
})

Equipment.Register("thruster.improved_s1", ThrusterType.New {
	l10n_key="THRUSTERS_IMPROVED", slots="thruster",
	price=250, purchasable=true, tech_level=5,
	slot = { type="thruster", size=1 },
	mass=0.1, volume=0.05, capabilities={ thruster_power=1 },
	icon_name="equip_thrusters_basic"
})

Equipment.Register("thruster.optimised_s1", ThrusterType.New {
	l10n_key="THRUSTERS_OPTIMISED", slots="thruster",
	price=560, purchasable=true, tech_level=8,
	slot = { type="thruster", size=1 },
	mass=0.05, volume=0.03, capabilities={ thruster_power=2 },
	icon_name="equip_thrusters_medium"
})

Equipment.Register("thruster.naval_s1", ThrusterType.New {
	l10n_key="THRUSTERS_NAVAL", slots="thruster",
	price=1400, purchasable=true, tech_level="MILITARY",
	slot = { type="thruster", size=1 },
	mass=0.1, volume=0.1, capabilities={ thruster_power=3 },
	icon_name="equip_thrusters_best"
})

-- S2 thrusters
Equipment.Register("thruster.default_s2", ThrusterType.New {
	l10n_key="THRUSTERS_DEFAULT", slots="thruster",
	price=220, purchasable=true, tech_level=2,
	slot = { type="thruster", size=2 },
	mass=0, volume=0, capabilities={ thruster_power=0 },
	icon_name="equip_thrusters_basic"
})

Equipment.Register("thruster.improved_s2", ThrusterType.New {
	l10n_key="THRUSTERS_IMPROVED", slots="thruster",
	price=460, purchasable=true, tech_level=5,
	slot = { type="thruster", size=2 },
	mass=0.24, volume=0.12, capabilities={ thruster_power=1 },
	icon_name="equip_thrusters_basic"
})

Equipment.Register("thruster.optimised_s2", ThrusterType.New {
	l10n_key="THRUSTERS_OPTIMISED", slots="thruster",
	price=1025, purchasable=true, tech_level=8,
	slot = { type="thruster", size=2 },
	mass=0.12, volume=0.1, capabilities={ thruster_power=2 },
	icon_name="equip_thrusters_medium"
})

Equipment.Register("thruster.naval_s2", ThrusterType.New {
	l10n_key="THRUSTERS_NAVAL", slots="thruster",
	price=2565, purchasable=true, tech_level="MILITARY",
	slot = { type="thruster", size=2 },
	mass=0.24, volume=0.24, capabilities={ thruster_power=3 },
	icon_name="equip_thrusters_best"
})

-- S3 thrusters
Equipment.Register("thruster.default_s3", ThrusterType.New {
	l10n_key="THRUSTERS_DEFAULT", slots="thruster",
	price=420, purchasable=true, tech_level=2,
	slot = { type="thruster", size=3 },
	mass=0, volume=0, capabilities={ thruster_power=0 },
	icon_name="equip_thrusters_basic"
})

Equipment.Register("thruster.improved_s3", ThrusterType.New {
	l10n_key="THRUSTERS_IMPROVED", slots="thruster",
	price=880, purchasable=true, tech_level=5,
	slot = { type="thruster", size=3 },
	mass=0.46, volume=0.23, capabilities={ thruster_power=1 },
	icon_name="equip_thrusters_basic"
})

Equipment.Register("thruster.optimised_s3", ThrusterType.New {
	l10n_key="THRUSTERS_OPTIMISED", slots="thruster",
	price=1950, purchasable=true, tech_level=8,
	slot = { type="thruster", size=3 },
	mass=0.23, volume=0.2, capabilities={ thruster_power=2 },
	icon_name="equip_thrusters_medium"
})

Equipment.Register("thruster.naval_s3", ThrusterType.New {
	l10n_key="THRUSTERS_NAVAL", slots="thruster",
	price=4970, purchasable=true, tech_level="MILITARY",
	slot = { type="thruster", size=3 },
	mass=0.46, volume=0.46, capabilities={ thruster_power=3 },
	icon_name="equip_thrusters_best"
})

-- S4 Thrusters
Equipment.Register("thruster.default_s4", ThrusterType.New {
	l10n_key="THRUSTERS_DEFAULT", slots="thruster",
	price=880, purchasable=true, tech_level=2,
	slot = { type="thruster", size=4 },
	mass=0, volume=0, capabilities={ thruster_power=0 },
	icon_name="equip_thrusters_basic"
})

Equipment.Register("thruster.improved_s4", ThrusterType.New {
	l10n_key="THRUSTERS_IMPROVED", slots="thruster",
	price=1850, purchasable=true, tech_level=5,
	slot = { type="thruster", size=4 },
	mass=0.96, volume=0.48, capabilities={ thruster_power=1 },
	icon_name="equip_thrusters_basic"
})

Equipment.Register("thruster.optimised_s4", ThrusterType.New {
	l10n_key="THRUSTERS_OPTIMISED", slots="thruster",
	price=4096, purchasable=true, tech_level=8,
	slot = { type="thruster", size=4 },
	mass=0.48, volume=0.42, capabilities={ thruster_power=2 },
	icon_name="equip_thrusters_medium"
})

Equipment.Register("thruster.naval_s4", ThrusterType.New {
	l10n_key="THRUSTERS_NAVAL", slots="thruster",
	price=10240, purchasable=true, tech_level="MILITARY",
	slot = { type="thruster", size=4 },
	mass=0.96, volume=0.96, capabilities={ thruster_power=3 },
	icon_name="equip_thrusters_best"
})

-- S5 thrusters
Equipment.Register("thruster.default_s5", ThrusterType.New {
	l10n_key="THRUSTERS_DEFAULT", slots="thruster",
	price=1950, purchasable=true, tech_level=2,
	slot = { type="thruster", size=5 },
	mass=0, volume=0, capabilities={ thruster_power=0 },
	icon_name="equip_thrusters_basic"
})

Equipment.Register("thruster.improved_s5", ThrusterType.New {
	l10n_key="THRUSTERS_IMPROVED", slots="thruster",
	price=4090, purchasable=true, tech_level=5,
	slot = { type="thruster", size=5 },
	mass=2.1, volume=1.05, capabilities={ thruster_power=1 },
	icon_name="equip_thrusters_basic"
})

Equipment.Register("thruster.optimised_s5", ThrusterType.New {
	l10n_key="THRUSTERS_OPTIMISED", slots="thruster",
	price=9050, purchasable=true, tech_level=8,
	slot = { type="thruster", size=5 },
	mass=1.05, volume=0.9, capabilities={ thruster_power=2 },
	icon_name="equip_thrusters_medium"
})

Equipment.Register("thruster.naval_s5", ThrusterType.New {
	l10n_key="THRUSTERS_NAVAL", slots="thruster",
	price=22620, purchasable=true, tech_level="MILITARY",
	slot = { type="thruster", size=5 },
	mass=2.1, volume=2.1, capabilities={ thruster_power=3 },
	icon_name="equip_thrusters_best"
})

--===============================================
-- Scoops
--===============================================

Equipment.Register("misc.fuel_scoop_s1", EquipType.New {
	l10n_key="FUEL_SCOOP",
	price=3500, purchasable=true, tech_level=4,
	slot = { type="fuel_scoop", size=1, hardpoint=true },
	mass=6, volume=4, capabilities={ fuel_scoop=2 },
	icon_name="equip_fuel_scoop"
})

Equipment.Register("misc.fuel_scoop_s2", EquipType.New {
	l10n_key="FUEL_SCOOP",
	price=6500, purchasable=true, tech_level=5,
	slot = { type="fuel_scoop", size=2, hardpoint=true },
	mass=8, volume=7, capabilities={ fuel_scoop=3 },
	icon_name="equip_fuel_scoop"
})

Equipment.Register("misc.fuel_scoop_s3", EquipType.New {
	l10n_key="FUEL_SCOOP",
	price=9500, purchasable=true, tech_level=7,
	slot = { type="fuel_scoop", size=3, hardpoint=true },
	mass=14, volume=10, capabilities={ fuel_scoop=5 },
	icon_name="equip_fuel_scoop"
})

Equipment.Register("misc.fuel_scoop_s4", EquipType.New {
	l10n_key="FUEL_SCOOP",
	price=14500, purchasable=true, tech_level=9,
	slot = { type="fuel_scoop", size=4, hardpoint=true },
	mass=22, volume=16, capabilities={ fuel_scoop=7 },
	icon_name="equip_fuel_scoop"
})

Equipment.Register("misc.fuel_scoop_s5", EquipType.New {
	l10n_key="FUEL_SCOOP",
	price=21500, purchasable=true, tech_level=12,
	slot = { type="fuel_scoop", size=5, hardpoint=true },
	mass=30, volume=22, capabilities={ fuel_scoop=9 },
	icon_name="equip_fuel_scoop"
})

Equipment.Register("misc.cargo_scoop", CargoScoopType.New {
	l10n_key="CARGO_SCOOP",
	price=2900, purchasable=true, tech_level=5,
	mass=2, volume=4, capabilities={ cargo_scoop=1 },
	icon_name="equip_cargo_scoop"
})

--===============================================
-- Passenger Cabins
--===============================================

Equipment.Register("misc.cabin_s1", CabinType.New {
	l10n_key="UNOCCUPIED_CABIN",
	price=1350, purchasable=true, tech_level=1,
	slot = { type="cabin.passenger.basic", size=1 },
	mass=1, volume=0, capabilities={ cabin=1 },
	icon_name="equip_cabin_empty"
})

Equipment.Register("misc.cabin_s2", CabinType.New {
	l10n_key="UNOCCUPIED_CABIN",
	price=3550, purchasable=true, tech_level=1,
	slot = { type="cabin.passenger.basic", size=2 },
	mass=4, volume=0, capabilities={ cabin=3 },
	icon_name="equip_cabin_empty"
})

Equipment.Register("misc.cabin_s3", CabinType.New {
	l10n_key="UNOCCUPIED_CABIN",
	price=6550, purchasable=true, tech_level=1,
	slot = { type="cabin.passenger.basic", size=3 },
	mass=8, volume=0, capabilities={ cabin=8 },
	icon_name="equip_cabin_empty"
})

Equipment.Register("misc.cabin_s4", CabinType.New {
	l10n_key="UNOCCUPIED_CABIN",
	price=13550, purchasable=true, tech_level=1,
	slot = { type="cabin.passenger.basic", size=4 },
	mass=16, volume=0, capabilities={ cabin=22 },
	icon_name="equip_cabin_empty"
})

Equipment.Register("misc.cabin_s5", CabinType.New {
	l10n_key="UNOCCUPIED_CABIN",
	price=35150, purchasable=true, tech_level=1,
	slot = { type="cabin.passenger.basic", size=5 },
	mass=36, volume=0, capabilities={ cabin=60 },
	icon_name="equip_cabin_empty"
})

--===============================================
-- Slot-less equipment
--===============================================

Equipment.Register("misc.laser_cooling_booster", EquipType.New {
	l10n_key="LASER_COOLING_BOOSTER",
	price=380, purchasable=true, tech_level=8,
	mass=1, volume=2, capabilities={ laser_cooler=2 },
})

Equipment.Register("misc.shield_energy_booster", EquipType.New {
	l10n_key="SHIELD_ENERGY_BOOSTER",
	price=10000, purchasable=true, tech_level=11,
	mass=5, volume=8, capabilities={ shield_energy_booster=1 },
})

Equipment.Register("misc.cargo_life_support", EquipType.New {
	l10n_key="CARGO_LIFE_SUPPORT",
	price=700, purchasable=true, tech_level=2,
	mass=1, volume=2, capabilities={ cargo_life_support=1 },
})
