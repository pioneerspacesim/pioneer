-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Commodities = require 'Commodities'
local EquipTypes  = require 'EquipType'
local Serializer  = require 'Serializer'

local LaserType       = EquipTypes.LaserType
local EquipType       = EquipTypes.EquipType
local HyperdriveType  = EquipTypes.HyperdriveType
local SensorType      = EquipTypes.SensorType
local BodyScannerType = EquipTypes.BodyScannerType

local laser           = EquipTypes.laser
local hyperspace      = EquipTypes.hyperspace
local misc            = EquipTypes.misc

-- Constants: EquipSlot
--
-- Equipment slots. Every equipment item has a corresponding
-- "slot" that it fits in to. Each slot has an independent capacity.
--
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

misc.missile_unguided = EquipType.New({
	l10n_key="MISSILE_UNGUIDED", slots="missile", price=30,
	missile_type="missile_unguided", tech_level=1,
	capabilities={mass=0.2}, purchasable=true,
	icon_name="equip_missile_unguided"
})
misc.missile_guided = EquipType.New({
	l10n_key="MISSILE_GUIDED", slots="missile", price=50,
	missile_type="missile_guided", tech_level=5,
	capabilities={mass=0.5}, purchasable=true,
	icon_name="equip_missile_guided"
})
misc.missile_smart = EquipType.New({
	l10n_key="MISSILE_SMART", slots="missile", price=95,
	missile_type="missile_smart", tech_level=10,
	capabilities={mass=1}, purchasable=true,
	icon_name="equip_missile_smart"
})
misc.missile_naval = EquipType.New({
	l10n_key="MISSILE_NAVAL", slots="missile", price=160,
	missile_type="missile_naval", tech_level="MILITARY",
	capabilities={mass=1.5}, purchasable=true,
	icon_name="equip_missile_naval"
})
misc.atmospheric_shielding = EquipType.New({
	l10n_key="ATMOSPHERIC_SHIELDING", slots="atmo_shield", price=200,
	capabilities={mass=1, atmo_shield=4},
	purchasable=true, tech_level=3,
	icon_name="equip_atmo_shield_generator"
})
misc.heavy_atmospheric_shielding = EquipType.New({
	l10n_key="ATMOSPHERIC_SHIELDING_HEAVY", slots="atmo_shield", price=900,
	capabilities={mass=2, atmo_shield=9},
	purchasable=true, tech_level=5,
	icon_name="equip_atmo_shield_generator"
})
misc.ecm_basic = EquipType.New({
	l10n_key="ECM_BASIC", slots="ecm", price=6000,
	capabilities={mass=2, ecm_power=2, ecm_recharge=5},
	purchasable=true, tech_level=9, ecm_type = 'ecm',
	hover_message="ECM_HOVER_MESSAGE"
})
misc.ecm_advanced = EquipType.New({
	l10n_key="ECM_ADVANCED", slots="ecm", price=15200,
	capabilities={mass=2, ecm_power=3, ecm_recharge=5},
	purchasable=true, tech_level="MILITARY", ecm_type = 'ecm_advanced',
	hover_message="ECM_HOVER_MESSAGE"
})
misc.radar = EquipType.New({
	l10n_key="RADAR", slots="radar", price=680,
	capabilities={mass=1, radar=1},
	purchasable=true, tech_level=3,
	icon_name="equip_radar"
})
misc.cabin = EquipType.New({
	l10n_key="UNOCCUPIED_CABIN", slots="cabin", price=1350,
	capabilities={mass=1, cabin=1},
	purchasable=true,  tech_level=1,
	icon_name="equip_cabin_empty"
})
misc.cabin_occupied = EquipType.New({
	l10n_key="PASSENGER_CABIN", slots="cabin", price=0,
	capabilities={mass=1}, purchasable=false, tech_level=1,
	icon_name="equip_cabin_occupied"
})
misc.shield_generator = EquipType.New({
	l10n_key="SHIELD_GENERATOR", slots="shield", price=2500,
	capabilities={mass=4, shield=1}, purchasable=true, tech_level=8,
	icon_name="equip_shield_generator"
})
misc.laser_cooling_booster = EquipType.New({
	l10n_key="LASER_COOLING_BOOSTER", slots="laser_cooler", price=380,
	capabilities={mass=1, laser_cooler=2}, purchasable=true, tech_level=8
})
misc.cargo_life_support = EquipType.New({
	l10n_key="CARGO_LIFE_SUPPORT", slots="cargo_life_support", price=700,
	capabilities={mass=1, cargo_life_support=1}, purchasable=true, tech_level=2
})
misc.autopilot = EquipType.New({
	l10n_key="AUTOPILOT", slots="autopilot", price=1400,
	capabilities={mass=1, set_speed=1, autopilot=1}, purchasable=true, tech_level=1,
	icon_name="equip_autopilot"
})
misc.target_scanner = EquipType.New({
	l10n_key="TARGET_SCANNER", slots="target_scanner", price=900,
	capabilities={mass=1, target_scanner_level=1}, purchasable=true, tech_level=9,
	icon_name="equip_scanner"
})
misc.advanced_target_scanner = EquipType.New({
	l10n_key="ADVANCED_TARGET_SCANNER", slots="target_scanner", price=1200,
	capabilities={mass=1, target_scanner_level=2}, purchasable=true, tech_level="MILITARY",
	icon_name="equip_scanner"
})
misc.fuel_scoop = EquipType.New({
	l10n_key="FUEL_SCOOP", slots="scoop", price=3500,
	capabilities={mass=6, fuel_scoop=3}, purchasable=true, tech_level=4,
	icon_name="equip_fuel_scoop"
})
misc.cargo_scoop = EquipType.New({
	l10n_key="CARGO_SCOOP", slots="scoop", price=3900,
	capabilities={mass=7, cargo_scoop=1}, purchasable=true, tech_level=5,
	icon_name="equip_cargo_scoop"
})
misc.multi_scoop = EquipType.New({
	l10n_key="MULTI_SCOOP", slots="scoop", price=12000,
	capabilities={mass=9, cargo_scoop=1, fuel_scoop=2}, purchasable=true, tech_level=9,
	icon_name="equip_multi_scoop"
})
misc.hypercloud_analyzer = EquipType.New({
	l10n_key="HYPERCLOUD_ANALYZER", slots="hypercloud", price=1500,
	capabilities={mass=1, hypercloud_analyzer=1}, purchasable=true, tech_level=10,
	icon_name="equip_scanner"
})
misc.shield_energy_booster = EquipType.New({
	l10n_key="SHIELD_ENERGY_BOOSTER", slots="energy_booster", price=10000,
	capabilities={mass=8, shield_energy_booster=1}, purchasable=true, tech_level=11
})
misc.hull_autorepair = EquipType.New({
	l10n_key="HULL_AUTOREPAIR", slots="hull_autorepair", price=16000,
	capabilities={mass=40, hull_autorepair=1}, purchasable=true, tech_level="MILITARY",
	icon_name="repairs"
})
misc.thrusters_basic = EquipType.New({
	l10n_key="THRUSTERS_BASIC", slots="thruster", price=3000,
	tech_level=5,
	capabilities={mass=0, thruster_power=1}, purchasable=true,
	icon_name="equip_thrusters_basic"
})
misc.thrusters_medium = EquipType.New({
	l10n_key="THRUSTERS_MEDIUM", slots="thruster", price=6500,
	tech_level=8,
	capabilities={mass=0, thruster_power=2}, purchasable=true,
	icon_name="equip_thrusters_medium"
})
misc.thrusters_best = EquipType.New({
	l10n_key="THRUSTERS_BEST", slots="thruster", price=14000,
	tech_level="MILITARY",
	capabilities={mass=0, thruster_power=3}, purchasable=true,
	icon_name="equip_thrusters_best"
})
misc.trade_computer = EquipType.New({
	l10n_key="TRADE_COMPUTER", slots="trade_computer", price=400,
	capabilities={mass=0, trade_computer=1}, purchasable=true, tech_level=9,
	icon_name="equip_trade_computer"
})
misc.planetscanner = BodyScannerType.New({
	l10n_key = 'SURFACE_SCANNER', slots="sensor", price=2950,
	capabilities={mass=1,sensor=1}, purchasable=true, tech_level=5,
	max_range=100000000, target_altitude=0, state="HALTED", progress=0,
	bodyscanner_stats={scan_speed=3, scan_tolerance=0.05},
	stats={ aperture = 50.0, minAltitude = 150, resolution = 768, orbital = false },
	icon_name="equip_planet_scanner"
})
misc.planetscanner_good = BodyScannerType.New({
	l10n_key = 'SURFACE_SCANNER_GOOD', slots="sensor", price=5000,
	capabilities={mass=2,sensor=1}, purchasable=true, tech_level=8,
	max_range=100000000, target_altitude=0, state="HALTED", progress=0,
	bodyscanner_stats={scan_speed=3, scan_tolerance=0.05},
	stats={ aperture = 65.0, minAltitude = 250, resolution = 1092, orbital = false },
	icon_name="equip_planet_scanner"
})
misc.orbitscanner = BodyScannerType.New({
	l10n_key = 'ORBIT_SCANNER', slots="sensor", price=7500,
	capabilities={mass=3,sensor=1}, purchasable=true, tech_level=3,
	max_range=100000000, target_altitude=0, state="HALTED", progress=0,
	bodyscanner_stats={scan_speed=3, scan_tolerance=0.05},
	stats={ aperture = 4.0, minAltitude = 650000, resolution = 6802, orbital = true },
	icon_name="equip_orbit_scanner"
})
misc.orbitscanner_good = BodyScannerType.New({
	l10n_key = 'ORBIT_SCANNER_GOOD', slots="sensor", price=11000,
	capabilities={mass=7,sensor=1}, purchasable=true, tech_level=8,
	max_range=100000000, target_altitude=0, state="HALTED", progress=0,
	bodyscanner_stats={scan_speed=3, scan_tolerance=0.05},
	stats={ aperture = 2.8, minAltitude = 1750000, resolution = 12375, orbital = true },
	icon_name="equip_orbit_scanner"
})

hyperspace.hyperdrive_1 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS1", fuel=Commodities.hydrogen, slots="engine",
	price=700, capabilities={mass=2, hyperclass=1}, purchasable=true, tech_level=3,
	icon_name="equip_hyperdrive"
})
hyperspace.hyperdrive_2 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS2", fuel=Commodities.hydrogen, slots="engine",
	price=1300, capabilities={mass=6, hyperclass=2}, purchasable=true, tech_level=4,
	icon_name="equip_hyperdrive"
})
hyperspace.hyperdrive_3 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS3", fuel=Commodities.hydrogen, slots="engine",
	price=2500, capabilities={mass=11, hyperclass=3}, purchasable=true, tech_level=4,
	icon_name="equip_hyperdrive"
})
hyperspace.hyperdrive_4 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS4", fuel=Commodities.hydrogen, slots="engine",
	price=5000, capabilities={mass=25, hyperclass=4}, purchasable=true, tech_level=5,
	icon_name="equip_hyperdrive"
})
hyperspace.hyperdrive_5 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS5", fuel=Commodities.hydrogen, slots="engine",
	price=10000, capabilities={mass=60, hyperclass=5}, purchasable=true, tech_level=5,
	icon_name="equip_hyperdrive"
})
hyperspace.hyperdrive_6 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS6", fuel=Commodities.hydrogen, slots="engine",
	price=20000, capabilities={mass=130, hyperclass=6}, purchasable=true, tech_level=6,
	icon_name="equip_hyperdrive"
})
hyperspace.hyperdrive_7 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS7", fuel=Commodities.hydrogen, slots="engine",
	price=30000, capabilities={mass=245, hyperclass=7}, purchasable=true, tech_level=8,
	icon_name="equip_hyperdrive"
})
hyperspace.hyperdrive_8 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS8", fuel=Commodities.hydrogen, slots="engine",
	price=60000, capabilities={mass=360, hyperclass=8}, purchasable=true, tech_level=9,
	icon_name="equip_hyperdrive"
})
hyperspace.hyperdrive_9 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS9", fuel=Commodities.hydrogen, slots="engine",
	price=120000, capabilities={mass=540, hyperclass=9}, purchasable=true, tech_level=10,
	icon_name="equip_hyperdrive"
})
hyperspace.hyperdrive_mil1 = HyperdriveType.New({
	l10n_key="DRIVE_MIL1", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives, slots="engine",
	price=23000, capabilities={mass=1, hyperclass=1}, purchasable=true, tech_level=10,
	icon_name="equip_hyperdrive_mil"
})
hyperspace.hyperdrive_mil2 = HyperdriveType.New({
	l10n_key="DRIVE_MIL2", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives, slots="engine",
	price=47000, capabilities={mass=3, hyperclass=2}, purchasable=true, tech_level="MILITARY",
	icon_name="equip_hyperdrive_mil"
})
hyperspace.hyperdrive_mil3 = HyperdriveType.New({
	l10n_key="DRIVE_MIL3", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives, slots="engine",
	price=85000, capabilities={mass=5, hyperclass=3}, purchasable=true, tech_level=11,
	icon_name="equip_hyperdrive_mil"
})
hyperspace.hyperdrive_mil4 = HyperdriveType.New({
	l10n_key="DRIVE_MIL4", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives, slots="engine",
	price=214000, capabilities={mass=13, hyperclass=4}, purchasable=true, tech_level=12,
	icon_name="equip_hyperdrive_mil"
})
hyperspace.hyperdrive_mil5 = HyperdriveType.New({
	l10n_key="DRIVE_MIL5", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives, slots="engine",
	price=540000, capabilities={mass=29, hyperclass=5}, purchasable=false, tech_level="MILITARY",
	icon_name="equip_hyperdrive_mil"
})
hyperspace.hyperdrive_mil6 = HyperdriveType.New({
	l10n_key="DRIVE_MIL6", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives, slots="engine",
	price=1350000, capabilities={mass=60, hyperclass=6}, purchasable=false, tech_level="MILITARY",
	icon_name="equip_hyperdrive_mil"
})
hyperspace.hyperdrive_mil7 = HyperdriveType.New({
	l10n_key="DRIVE_MIL7", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives, slots="engine",
	price=3500000, capabilities={mass=135, hyperclass=7}, purchasable=false, tech_level="MILITARY",
	icon_name="equip_hyperdrive_mil"
})
hyperspace.hyperdrive_mil8 = HyperdriveType.New({
	l10n_key="DRIVE_MIL8", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives, slots="engine",
	price=8500000, capabilities={mass=190, hyperclass=8}, purchasable=false, tech_level="MILITARY",
	icon_name="equip_hyperdrive_mil"
})
hyperspace.hyperdrive_mil9 = HyperdriveType.New({
	l10n_key="DRIVE_MIL9", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives, slots="engine",
	price=22000000, capabilities={mass=260, hyperclass=9}, purchasable=false, tech_level="MILITARY",
	icon_name="equip_hyperdrive_mil"
})

laser.pulsecannon_1mw = LaserType.New({
	l10n_key="PULSECANNON_1MW", price=600, capabilities={mass=1},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=1000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 255, rgba_g = 51, rgba_b = 51, rgba_a = 255
	}, purchasable=true, tech_level=3,
	icon_name="equip_pulsecannon"
})
laser.pulsecannon_dual_1mw = LaserType.New({
	l10n_key="PULSECANNON_DUAL_1MW", price=1100, capabilities={mass=4},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=1000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=1, mining=0, rgba_r = 255, rgba_g = 51, rgba_b = 51, rgba_a = 255
	}, purchasable=true, tech_level=4,
	icon_name="equip_dual_pulsecannon"
})
laser.pulsecannon_2mw = LaserType.New({
	l10n_key="PULSECANNON_2MW", price=1000, capabilities={mass=3},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=2000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 255, rgba_g = 127, rgba_b = 51, rgba_a = 255
	}, purchasable=true, tech_level=5,
	icon_name="equip_pulsecannon"
})
laser.pulsecannon_rapid_2mw = LaserType.New({
	l10n_key="PULSECANNON_RAPID_2MW", price=1800, capabilities={mass=7},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=2000, rechargeTime=0.13, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 255, rgba_g = 127, rgba_b = 51, rgba_a = 255
	}, purchasable=true, tech_level=5,
	icon_name="equip_pulsecannon_rapid"
})
laser.beamlaser_1mw = LaserType.New({
	l10n_key="BEAMLASER_1MW", price=2400, capabilities={mass=3},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=1500, rechargeTime=0.25, length=10000,
		width=1, beam=1, dual=0, mining=0, rgba_r = 255, rgba_g = 51, rgba_b = 127, rgba_a = 255,
		heatrate=0.02, coolrate=0.01
	}, purchasable=true, tech_level=4,
	icon_name="equip_beamlaser"
})
laser.beamlaser_dual_1mw = LaserType.New({
	l10n_key="BEAMLASER_DUAL_1MW", price=4800, capabilities={mass=6},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=1500, rechargeTime=0.5, length=10000,
		width=1, beam=1, dual=1, mining=0, rgba_r = 255, rgba_g = 51, rgba_b = 127, rgba_a = 255,
		heatrate=0.02, coolrate=0.01
	}, purchasable=true, tech_level=5,
	icon_name="equip_dual_beamlaser"
})
laser.beamlaser_2mw = LaserType.New({
	l10n_key="BEAMLASER_RAPID_2MW", price=5600, capabilities={mass=7},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=3000, rechargeTime=0.13, length=20000,
		width=1, beam=1, dual=0, mining=0, rgba_r = 255, rgba_g = 192, rgba_b = 192, rgba_a = 255,
		heatrate=0.02, coolrate=0.01
	}, purchasable=true, tech_level=6,
	icon_name="equip_beamlaser"
})
laser.pulsecannon_4mw = LaserType.New({
	l10n_key="PULSECANNON_4MW", price=2200, capabilities={mass=10},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=4000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 255, rgba_g = 255, rgba_b = 51, rgba_a = 255
	}, purchasable=true, tech_level=6,
	icon_name="equip_pulsecannon"
})
laser.pulsecannon_10mw = LaserType.New({
	l10n_key="PULSECANNON_10MW", price=4900, capabilities={mass=30},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=10000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 51, rgba_g = 255, rgba_b = 51, rgba_a = 255
	}, purchasable=true, tech_level=7,
	icon_name="equip_pulsecannon"
})
laser.pulsecannon_20mw = LaserType.New({
	l10n_key="PULSECANNON_20MW", price=12000, capabilities={mass=65},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=20000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 0.1, rgba_g = 51, rgba_b = 255, rgba_a = 255
	}, purchasable=true, tech_level="MILITARY",
	icon_name="equip_pulsecannon"
})
laser.miningcannon_5mw = LaserType.New({
	l10n_key="MININGCANNON_5MW", price=3700, capabilities={mass=6},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=5000, rechargeTime=1.5, length=30,
		width=5, beam=0, dual=0, mining=1, rgba_r = 51, rgba_g = 127, rgba_b = 0, rgba_a = 255
	}, purchasable=true, tech_level=5,
	icon_name="equip_mining_laser"
})
laser.miningcannon_17mw = LaserType.New({
	l10n_key="MININGCANNON_17MW", price=10600, capabilities={mass=10},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=17000, rechargeTime=2, length=30,
		width=5, beam=0, dual=0, mining=1, rgba_r = 51, rgba_g = 127, rgba_b = 0, rgba_a = 255
	}, purchasable=true, tech_level=8,
	icon_name="equip_mining_laser"
})
laser.small_plasma_accelerator = LaserType.New({
	l10n_key="SMALL_PLASMA_ACCEL", price=120000, capabilities={mass=22},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=50000, rechargeTime=0.3, length=42,
		width=7, beam=0, dual=0, mining=0, rgba_r = 51, rgba_g = 255, rgba_b = 255, rgba_a = 255
	}, purchasable=true, tech_level=10,
	icon_name="equip_plasma_accelerator"
})
laser.large_plasma_accelerator = LaserType.New({
	l10n_key="LARGE_PLASMA_ACCEL", price=390000, capabilities={mass=50},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=100000, rechargeTime=0.3, length=42,
		width=7, beam=0, dual=0, mining=0, rgba_r = 127, rgba_g = 255, rgba_b = 255, rgba_a = 255
	}, purchasable=true, tech_level=12,
	icon_name="equip_plasma_accelerator"
})

local serialize = function()
	local ret = {}
	for _,k in ipairs{"laser", "hyperspace", "misc"} do
		local tmp = {}
		for kk, vv in pairs(EquipTypes[k]) do
			tmp[kk] = vv
		end
		ret[k] = tmp
	end
	return ret
end

local unserialize = function (data)
	for _,k in ipairs{"laser", "hyperspace", "misc"} do
		local tmp = EquipTypes[k]
		for kk, vv in pairs(data[k]) do
			tmp[kk] = vv
		end
	end
end

Serializer:Register("Equipment", serialize, unserialize)

return EquipTypes
