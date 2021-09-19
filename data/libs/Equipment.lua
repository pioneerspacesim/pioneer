-- Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local utils = require 'utils'
local Serializer = require 'Serializer'
local Lang = require 'Lang'
local ShipDef = require 'ShipDef'
local Timer = require 'Timer'
local Comms = require 'Comms'

local Game = package.core['Game']
local Space = package.core['Space']


local EquipTypes = require 'EquipType'

local LaserType = EquipTypes.LaserType
local EquipType = EquipTypes.EquipType
local HyperdriveType = EquipTypes.HyperdriveType
local SensorType = EquipTypes.SensorType
local BodyScannerType = EquipTypes.BodyScannerType

local cargo = require 'Commodities'
local laser = EquipTypes.laser
local hyperspace = EquipTypes.hyperspace
local misc = EquipTypes.misc

-- Constants: EquipSlot
--
-- Equipment slots. Every equipment or cargo type has a corresponding
-- "slot" that it fits in to. Each slot has an independent capacity.
--
-- cargo - any cargo (commodity) item
-- engine - hyperdrives and military drives
-- laser_front - front attachment point for lasers and plasma accelerators
-- laser_rear - rear attachment point for lasers and plasma accelerators
-- missile - missile
-- ecm - ecm system
-- radar - radar
-- target_scanner - target scanner
-- hypercloud - hyperspace cloud analyser
-- hull_autorepair - hull auto-repair system
-- energy_booster - shield energy booster unit
-- atmo_shield - atmospheric shielding
-- cabin - cabin
-- shield - shield
-- scoop - scoop used for scooping things (cargo, fuel/hydrogen)
-- laser_cooler - laser cooling booster
-- cargo_life_support - cargo bay life support
-- autopilot - autopilot
-- trade_computer - commodity trade analyzer computer module

-- why is this text string when all others are numbers?
local miltech = "MILITARY"

--[[
		Uniform equipment entries:
		New(
			l10n_key, slots, price,
			capabilities={},
			standard attributes (purchasable, tech_level, infovis),
			non-standard attributes
		)

		infovis=1 : the item will be listed under "Equipment" on the info-view ship screen
--]]

misc.missile_unguided = EquipType.New({
	l10n_key="MISSILE_UNGUIDED", slots="missile", price=30,
	capabilities={mass=1, missile=1},
	purchasable=true, tech_level=1,
	missile_type="missile_unguided", icon_name="missile_unguided"
})
misc.missile_guided = EquipType.New({
	l10n_key="MISSILE_GUIDED", slots="missile", price=50,
	capabilities={mass=1},
	purchasable=true, tech_level=5,
	icon_name="missile_guided", missile_type="missile_guided"
})
misc.missile_smart = EquipType.New({
	l10n_key="MISSILE_SMART", slots="missile", price=95,
	capabilities={mass=1},
	purchasable=true, tech_level=10,
	icon_name="missile_smart", missile_type="missile_smart"
})
misc.missile_naval = EquipType.New({
	l10n_key="MISSILE_NAVAL", slots="missile", price=160,
	capabilities={mass=1},
	purchasable=true, tech_level=miltech,
	icon_name="missile_naval", missile_type="missile_naval"
})
misc.atmospheric_shielding = EquipType.New({
	l10n_key="ATMOSPHERIC_SHIELDING", slots="atmo_shield", price=200,
	capabilities={mass=1, atmo_shield=9},
	purchasable=true, tech_level=3, infovis=1
})
misc.heavy_atmospheric_shielding = EquipType.New({
	l10n_key="ATMOSPHERIC_SHIELDING_HEAVY", slots="atmo_shield", price=900,
	capabilities={mass=2, atmo_shield=19},
	purchasable=true, tech_level=5, infovis=1
})
misc.ecm_basic = EquipType.New({
	l10n_key="ECM_BASIC", slots="ecm", price=6000,
	capabilities={mass=2, ecm_power=2, ecm_recharge=5},
	purchasable=true, tech_level=9, infovis=1,
	ecm_type = 'ecm'
})
misc.ecm_advanced = EquipType.New({
	l10n_key="ECM_ADVANCED", slots="ecm", price=15200,
	capabilities={mass=2, ecm_power=3, ecm_recharge=5},
	purchasable=true, tech_level=miltech, infovis=1,
	ecm_type = 'ecm_advanced'
})
misc.radar = EquipType.New({
	l10n_key="RADAR", slots="radar", price=680,
	capabilities={mass=1, radar=1},
	purchasable=true, tech_level=3, infovis=1
})
misc.cabin = EquipType.New({
	l10n_key="UNOCCUPIED_CABIN", slots="cabin", price=1350,
	capabilities={mass=1, cabin=1},
	purchasable=true, tech_level=1, infovis=1,
	plural_l10n_key = "N_UNOCCUPIED_PASSENGER_CABINS"
})
misc.cabin_occupied = EquipType.New({
	l10n_key="PASSENGER_CABIN", slots="cabin", price=0,
	capabilities={mass=1},
	purchasable=false, tech_level=1, infovis=1,
	plural_l10n_key = "N_OCCUPIED_PASSENGER_CABINS"
})
misc.shield_generator = EquipType.New({
	l10n_key="SHIELD_GENERATOR", slots="shield", price=2500,
	capabilities={mass=4, shield=1},
	purchasable=true, tech_level=8, infovis=1,
	plural_l10n_key="N_SHIELD_GENERATORS"
})
misc.laser_cooling_booster = EquipType.New({
	l10n_key="LASER_COOLING_BOOSTER", slots="laser_cooler", price=380,
	capabilities={mass=1, laser_cooler=2},
	purchasable=true, tech_level=8, infovis=1
})
misc.cargo_life_support = EquipType.New({
	l10n_key="CARGO_LIFE_SUPPORT", slots="cargo_life_support", price=700,
	capabilities={mass=1, cargo_life_support=1},
	purchasable=true, tech_level=2, infovis=1
})
misc.autopilot = EquipType.New({
	l10n_key="AUTOPILOT", slots="autopilot", price=1400,
	capabilities={mass=1, set_speed=1, autopilot=1},
	purchasable=true, tech_level=1, infovis=1
})
misc.target_scanner = EquipType.New({
	l10n_key="TARGET_SCANNER", slots="target_scanner", price=900,
	capabilities={mass=1, target_scanner_level=1},
	purchasable=true, tech_level=9, infovis=1
})
misc.advanced_target_scanner = EquipType.New({
	l10n_key="ADVANCED_TARGET_SCANNER", slots="target_scanner", price=1200,
	capabilities={mass=1, target_scanner_level=2},
	purchasable=true, tech_level=miltech, infovis=1
})
misc.fuel_scoop = EquipType.New({
	l10n_key="FUEL_SCOOP", slots="scoop", price=3500,
	capabilities={mass=6, fuel_scoop=3},
	purchasable=true, tech_level=4, infovis=1
})
misc.cargo_scoop = EquipType.New({
	l10n_key="CARGO_SCOOP", slots="scoop", price=3900,
	capabilities={mass=7, cargo_scoop=1},
	purchasable=true, tech_level=5, infovis=1
})
misc.multi_scoop = EquipType.New({
	l10n_key="MULTI_SCOOP", slots="scoop", price=12000,
	capabilities={mass=9, cargo_scoop=1, fuel_scoop=2},
	purchasable=true, tech_level=9, infovis=1
})
misc.hypercloud_analyzer = EquipType.New({
	l10n_key="HYPERCLOUD_ANALYZER", slots="hypercloud", price=1500,
	capabilities={mass=1, hypercloud_analyzer=1},
	purchasable=true, tech_level=10, infovis=1
})
misc.shield_energy_booster = EquipType.New({
	l10n_key="SHIELD_ENERGY_BOOSTER", slots="energy_booster", price=10000,
	capabilities={mass=8, shield_energy_booster=1},
	purchasable=true, tech_level=11, infovis=1
})
misc.hull_autorepair = EquipType.New({
	l10n_key="HULL_AUTOREPAIR", slots="hull_autorepair", price=16000,
	capabilities={mass=40, hull_autorepair=1},
	purchasable=true, tech_level=miltech, infovis=1
})
misc.thrusters_basic = EquipType.New({
	l10n_key="THRUSTERS_BASIC", slots="thruster", price=3000,
	capabilities={mass=0, thruster_power=1},
	purchasable=true, tech_level=5, infovis=1,
	icon_name="thrusters_basic"
})
misc.thrusters_medium = EquipType.New({
	l10n_key="THRUSTERS_MEDIUM", slots="thruster", price=6500,
	capabilities={mass=0, thruster_power=2},
	purchasable=true, tech_level=8, infovis=1,
	icon_name="thrusters_medium"
})
misc.thrusters_best = EquipType.New({
	l10n_key="THRUSTERS_BEST", slots="thruster", price=14000,
	capabilities={mass=0, thruster_power=3},
	purchasable=true, tech_level=miltech, infovis=1,
	icon_name="thrusters_best"
})
misc.trade_computer = EquipType.New({
	l10n_key="TRADE_COMPUTER", slots="trade_computer", price=400,
	capabilities={mass=0, trade_computer=1},
	purchasable=true, tech_level=9, infovis=1
})
misc.planetscanner = BodyScannerType.New({
	l10n_key = 'PLANETSCANNER', slots="sensor", price=15000,
	capabilities={mass=1, sensor=1},
	purchasable=false, tech_level=1, infovis=1,
	icon_on_name="body_scanner_on", icon_off_name="body_scanner_off",
	max_range=100000000, target_altitude=0, state="HALTED", progress=0,
	bodyscanner_stats={scan_speed=3, scan_tolerance=0.05}
})

hyperspace.hyperdrive_1 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS1", fuel=cargo.hydrogen, slots="engine", price=700,
	capabilities={mass=4, hyperclass=1},
	purchasable=true, tech_level=3
})
hyperspace.hyperdrive_2 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS2", fuel=cargo.hydrogen, slots="engine", price=1300,
	capabilities={mass=10, hyperclass=2},
	purchasable=true, tech_level=4
})
hyperspace.hyperdrive_3 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS3", fuel=cargo.hydrogen, slots="engine", price=2500,
	capabilities={mass=20, hyperclass=3},
	purchasable=true, tech_level=4
})
hyperspace.hyperdrive_4 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS4", fuel=cargo.hydrogen, slots="engine", price=5000,
	capabilities={mass=40, hyperclass=4},
	purchasable=true, tech_level=5
})
hyperspace.hyperdrive_5 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS5", fuel=cargo.hydrogen, slots="engine", price=10000,
	capabilities={mass=120, hyperclass=5},
	purchasable=true, tech_level=5
})
hyperspace.hyperdrive_6 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS6", fuel=cargo.hydrogen, slots="engine", price=20000,
	capabilities={mass=225, hyperclass=6},
	purchasable=true, tech_level=6
})
hyperspace.hyperdrive_7 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS7", fuel=cargo.hydrogen, slots="engine", price=30000,
	capabilities={mass=400, hyperclass=7},
	purchasable=true, tech_level=8
})
hyperspace.hyperdrive_8 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS8", fuel=cargo.hydrogen, slots="engine", price=60000,
	capabilities={mass=580, hyperclass=8},
	purchasable=true, tech_level=9
})
hyperspace.hyperdrive_9 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS9", fuel=cargo.hydrogen, slots="engine", price=120000,
	capabilities={mass=740, hyperclass=9},
	purchasable=true, tech_level=10
})
hyperspace.hyperdrive_mil1 = HyperdriveType.New({
	l10n_key="DRIVE_MIL1", fuel=cargo.military_fuel, slots="engine", price=23000,
	capabilities={mass=3, hyperclass=1},
	purchasable=true, tech_level=10,
	byproduct=cargo.radioactives
})
hyperspace.hyperdrive_mil2 = HyperdriveType.New({
	l10n_key="DRIVE_MIL2", fuel=cargo.military_fuel, slots="engine", price=47000,
	capabilities={mass=8, hyperclass=2},
	purchasable=true, tech_level=miltech,
	byproduct=cargo.radioactives
})
hyperspace.hyperdrive_mil3 = HyperdriveType.New({
	l10n_key="DRIVE_MIL3", fuel=cargo.military_fuel, slots="engine", price=85000,
	capabilities={mass=16, hyperclass=3},
	purchasable=true, tech_level=11,
	byproduct=cargo.radioactives
})
hyperspace.hyperdrive_mil4 = HyperdriveType.New({
	l10n_key="DRIVE_MIL4", fuel=cargo.military_fuel, slots="engine", price=214000,
	capabilities={mass=30, hyperclass=4},
	purchasable=true, tech_level=12,
	byproduct=cargo.radioactives
})
hyperspace.hyperdrive_mil5 = HyperdriveType.New({
	l10n_key="DRIVE_MIL5", fuel=cargo.military_fuel,  slots="engine", price=540000,
	capabilities={mass=53, hyperclass=5},
	purchasable=false, tech_level=miltech,
	byproduct=cargo.radioactives
})
hyperspace.hyperdrive_mil6 = HyperdriveType.New({
	l10n_key="DRIVE_MIL6", fuel=cargo.military_fuel,  slots="engine", price=1350000,
	capabilities={mass=78, hyperclass=6},
	purchasable=false, tech_level=miltech,
	byproduct=cargo.radioactives
})
hyperspace.hyperdrive_mil7 = HyperdriveType.New({
	l10n_key="DRIVE_MIL7", fuel=cargo.military_fuel, slots="engine", price=3500000,
	capabilities={mass=128, hyperclass=7},
	purchasable=false, tech_level=miltech,
	byproduct=cargo.radioactives
})
hyperspace.hyperdrive_mil8 = HyperdriveType.New({
	l10n_key="DRIVE_MIL8", fuel=cargo.military_fuel, slots="engine", price=8500000,
	capabilities={mass=196, hyperclass=8},
	purchasable=false, tech_level=miltech,
	byproduct=cargo.radioactives
})
hyperspace.hyperdrive_mil9 = HyperdriveType.New({
	l10n_key="DRIVE_MIL9", fuel=cargo.military_fuel, slots="engine", price=22000000,
	capabilities={mass=285, hyperclass=9},
	purchasable=false, tech_level=miltech,
	byproduct=cargo.radioactives
})

laser.pulsecannon_1mw = LaserType.New({
	l10n_key="PULSECANNON_1MW", slots = {"laser_front", "laser_rear"}, price=600,
	capabilities={mass=1},
	purchasable=true, tech_level=3,
	laser_stats = {	lifespan=8, speed=1000, damage=1000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 255, rgba_g = 51, rgba_b = 51, rgba_a = 255 }
})
laser.pulsecannon_dual_1mw = LaserType.New({
	l10n_key="PULSECANNON_DUAL_1MW", slots = {"laser_front", "laser_rear"}, price=1100,
	capabilities={mass=4},
	purchasable=true, tech_level=4,
	laser_stats = { lifespan=8, speed=1000, damage=1000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=1, mining=0, rgba_r = 255, rgba_g = 51, rgba_b = 51, rgba_a = 255	}
})
laser.pulsecannon_2mw = LaserType.New({
	l10n_key="PULSECANNON_2MW", slots = {"laser_front", "laser_rear"}, price=1000,
	capabilities={mass=3},
	purchasable=true, tech_level=5,
	laser_stats = {	lifespan=8, speed=1000, damage=2000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 255, rgba_g = 127, rgba_b = 51, rgba_a = 255 }
})
laser.pulsecannon_rapid_2mw = LaserType.New({
	l10n_key="PULSECANNON_RAPID_2MW", slots = {"laser_front", "laser_rear"}, price=1800,
	capabilities={mass=7},
	purchasable=true, tech_level=5,
	laser_stats = { lifespan=8, speed=1000, damage=2000, rechargeTime=0.13, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 255, rgba_g = 127, rgba_b = 51, rgba_a = 255 }
})
laser.beamlaser_1mw = LaserType.New({
	l10n_key="BEAMLASER_1MW", slots = {"laser_front", "laser_rear"}, price=2400,
	capabilities={mass=3},
	purchasable=true, tech_level=4,
	laser_stats = {	lifespan=8, speed=1000, damage=1500, rechargeTime=0.25, length=10000,
		width=1, beam=1, dual=0, mining=0, rgba_r = 255, rgba_g = 51, rgba_b = 127, rgba_a = 255,
		heatrate=0.02, coolrate=0.01 }
})
laser.beamlaser_dual_1mw = LaserType.New({
	l10n_key="BEAMLASER_DUAL_1MW", slots = {"laser_front", "laser_rear"}, price=4800,
	capabilities={mass=6},
	purchasable=true, tech_level=5,
	laser_stats = {	lifespan=8, speed=1000, damage=1500, rechargeTime=0.5, length=10000,
		width=1, beam=1, dual=1, mining=0, rgba_r = 255, rgba_g = 51, rgba_b = 127, rgba_a = 255,
		heatrate=0.02, coolrate=0.01 }
})
laser.beamlaser_2mw = LaserType.New({
	l10n_key="BEAMLASER_RAPID_2MW", slots = {"laser_front", "laser_rear"}, price=5600,
	capabilities={mass=7},
	purchasable=true, tech_level=6,
	laser_stats = {	lifespan=8, speed=1000, damage=3000, rechargeTime=0.13, length=20000,
		width=1, beam=1, dual=0, mining=0, rgba_r = 255, rgba_g = 192, rgba_b = 192, rgba_a = 255,
		heatrate=0.02, coolrate=0.01 }
})
laser.pulsecannon_4mw = LaserType.New({
	l10n_key="PULSECANNON_4MW", slots = {"laser_front", "laser_rear"}, price=2200,
	capabilities={mass=10},
	purchasable=true, tech_level=6,
	laser_stats = {	lifespan=8, speed=1000, damage=4000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 255, rgba_g = 255, rgba_b = 51, rgba_a = 255 }
})
laser.pulsecannon_10mw = LaserType.New({
	l10n_key="PULSECANNON_10MW", slots = {"laser_front", "laser_rear"}, price=4900,
	capabilities={mass=30},
	purchasable=true, tech_level=7,
	laser_stats = {	lifespan=8, speed=1000, damage=10000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 51, rgba_g = 255, rgba_b = 51, rgba_a = 255	}
})
laser.pulsecannon_20mw = LaserType.New({
	l10n_key="PULSECANNON_20MW", slots = {"laser_front", "laser_rear"}, price=12000,
	capabilities={mass=65},
	purchasable=true, tech_level=miltech,
	laser_stats = { lifespan=8, speed=1000, damage=20000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 0.1, rgba_g = 51, rgba_b = 255, rgba_a = 255 }
})
laser.miningcannon_5mw = LaserType.New({
	l10n_key="MININGCANNON_5MW", slots = {"laser_front", "laser_rear"}, price=3700,
	capabilities={mass=6},
	purchasable=true, tech_level=5,
	laser_stats = {	lifespan=8, speed=1000, damage=5000, rechargeTime=1.5, length=30,
		width=5, beam=0, dual=0, mining=1, rgba_r = 51, rgba_g = 127, rgba_b = 0, rgba_a = 255 }
})
laser.miningcannon_17mw = LaserType.New({
	l10n_key="MININGCANNON_17MW", slots = {"laser_front", "laser_rear"}, price=10600,
	capabilities={mass=10},
	purchasable=true, tech_level=8,
	laser_stats = {	lifespan=8, speed=1000, damage=17000, rechargeTime=2, length=30,
		width=5, beam=0, dual=0, mining=1, rgba_r = 51, rgba_g = 127, rgba_b = 0, rgba_a = 255 }
})
laser.small_plasma_accelerator = LaserType.New({
	l10n_key="SMALL_PLASMA_ACCEL", slots = {"laser_front", "laser_rear"}, price=120000,
	capabilities={mass=22},
	purchasable=true, tech_level=10,
	laser_stats = { lifespan=8, speed=1000, damage=50000, rechargeTime=0.3, length=42,
		width=7, beam=0, dual=0, mining=0, rgba_r = 51, rgba_g = 255, rgba_b = 255, rgba_a = 255 }
})
laser.large_plasma_accelerator = LaserType.New({
	l10n_key="LARGE_PLASMA_ACCEL", slots = {"laser_front", "laser_rear"}, price=390000,
	capabilities={mass=50},
	purchasable=true, tech_level=12,
	laser_stats = { lifespan=8, speed=1000, damage=100000, rechargeTime=0.3, length=42,
		width=7, beam=0, dual=0, mining=0, rgba_r = 127, rgba_g = 255, rgba_b = 255, rgba_a = 255 }
})

local serialize = function()
	local ret = {}
	for _,k in ipairs{"cargo","laser", "hyperspace", "misc"} do
		local tmp = {}
		for kk, vv in pairs(EquipTypes[k]) do
			tmp[kk] = vv
		end
		ret[k] = tmp
	end
	return ret
end

local unserialize = function (data)
	for _,k in ipairs{"cargo","laser", "hyperspace", "misc"} do
		local tmp = EquipTypes[k]
		for kk, vv in pairs(data[k]) do
			tmp[kk] = vv
		end
	end
end

Serializer:Register("Equipment", serialize, unserialize)

return EquipTypes
