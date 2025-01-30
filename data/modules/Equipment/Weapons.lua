-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local EquipTypes = require '.Types'
local Equipment  = require 'Equipment'
local Slot       = require 'HullConfig'.Slot

local EquipType   = EquipTypes.EquipType
local LaserType   = EquipTypes.LaserType
local MissileType = EquipTypes.MissileType

--===============================================
-- Pulse Cannons
--===============================================

Equipment.Register("laser.pulsecannon_1mw", LaserType.New {
	l10n_key="PULSECANNON_1MW",
	price=600, purchasable=true, tech_level=3,
	mass=2, volume=1.5, capabilities = {},
	slot = { type="weapon.energy.pulsecannon", size=1, hardpoint=true },
	laser_stats = {
		lifespan=8, speed=1000, damage=1000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 255, rgba_g = 51, rgba_b = 51, rgba_a = 255
	},
	icon_name="equip_pulsecannon"
})

Equipment.Register("laser.pulsecannon_dual_1mw", LaserType.New {
	l10n_key="PULSECANNON_DUAL_1MW",
	price=1100, purchasable=true, tech_level=3,
	mass=4, volume=2, capabilities = {},
	slot = { type="weapon.energy.pulsecannon", size=2, hardpoint=true },
	laser_stats = {
		lifespan=8, speed=1000, damage=1000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=1, mining=0, rgba_r = 255, rgba_g = 51, rgba_b = 51, rgba_a = 255
	},
	icon_name="equip_pulsecannon"
})

Equipment.Register("laser.pulsecannon_2mw", LaserType.New {
	l10n_key="PULSECANNON_2MW",
	price=1000, purchasable=true, tech_level=5,
	mass=3, volume=2.5, capabilities = {},
	slot = { type="weapon.energy.pulsecannon", size=2, hardpoint=true },
	laser_stats = {
		lifespan=8, speed=1000, damage=2000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 255, rgba_g = 127, rgba_b = 51, rgba_a = 255
	},
	icon_name="equip_pulsecannon"
})

Equipment.Register("laser.pulsecannon_rapid_2mw", LaserType.New {
	l10n_key="PULSECANNON_RAPID_2MW",
	price=1800, purchasable=true, tech_level=5,
	mass=7, volume=7, capabilities={},
	slot = { type="weapon.energy.pulsecannon", size=2, hardpoint=true },
	laser_stats = {
		lifespan=8, speed=1000, damage=2000, rechargeTime=0.13, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 255, rgba_g = 127, rgba_b = 51, rgba_a = 255
	},
	icon_name="equip_pulsecannon_rapid"
})

Equipment.Register("laser.pulsecannon_4mw", LaserType.New {
	l10n_key="PULSECANNON_4MW",
	price=2200, purchasable=true, tech_level=6,
	mass=10, volume=10, capabilities={},
	slot = { type="weapon.energy.pulsecannon", size=3, hardpoint=true },
	laser_stats = {
		lifespan=8, speed=1000, damage=4000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 255, rgba_g = 255, rgba_b = 51, rgba_a = 255
	},
	icon_name="equip_pulsecannon"
})

Equipment.Register("laser.pulsecannon_10mw", LaserType.New {
	l10n_key="PULSECANNON_10MW",
	price=4900, purchasable=true, tech_level=7,
	mass=30, volume=30, capabilities={},
	slot = { type="weapon.energy.pulsecannon", size=4, hardpoint=true },
	laser_stats = {
		lifespan=8, speed=1000, damage=10000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 51, rgba_g = 255, rgba_b = 51, rgba_a = 255
	},
	icon_name="equip_pulsecannon"
})

Equipment.Register("laser.pulsecannon_20mw", LaserType.New {
	l10n_key="PULSECANNON_20MW",
	price=12000, purchasable=true, tech_level="MILITARY",
	mass=65, volume=65, capabilities={},
	slot = { type="weapon.energy.pulsecannon", size=5, hardpoint=true },
	laser_stats = {
		lifespan=8, speed=1000, damage=20000, rechargeTime=0.25, length=30,
		width=5, beam=0, dual=0, mining=0, rgba_r = 0.1, rgba_g = 51, rgba_b = 255, rgba_a = 255
	},
	icon_name="equip_pulsecannon"
})

--===============================================
-- Beam Lasers
--===============================================

Equipment.Register("laser.beamlaser_1mw", LaserType.New {
	l10n_key="BEAMLASER_1MW",
	price=2400, purchasable=true, tech_level=4,
	mass=3, volume=3, capabilities={},
	slot = { type="weapon.energy.laser", size=1, hardpoint=true },
	laser_stats = {
		lifespan=8, speed=1000, damage=1500, rechargeTime=0.25, length=10000,
		width=1, beam=1, dual=0, mining=0, rgba_r = 255, rgba_g = 51, rgba_b = 127, rgba_a = 255,
		heatrate=0.02, coolrate=0.01
	},
	icon_name="equip_beamlaser"
})

Equipment.Register("laser.beamlaser_dual_1mw", LaserType.New {
	l10n_key="BEAMLASER_DUAL_1MW",
	price=4800, purchasable=true, tech_level=5,
	mass=6, volume=6, capabilities={},
	slot = { type="weapon.energy.laser", size=2, hardpoint=true },
	laser_stats = {
		lifespan=8, speed=1000, damage=1500, rechargeTime=0.5, length=10000,
		width=1, beam=1, dual=1, mining=0, rgba_r = 255, rgba_g = 51, rgba_b = 127, rgba_a = 255,
		heatrate=0.02, coolrate=0.01
	},
	icon_name="equip_dual_beamlaser"
})

Equipment.Register("laser.beamlaser_2mw", LaserType.New {
	l10n_key="BEAMLASER_RAPID_2MW",
	price=5600, purchasable=true, tech_level=6,
	mass=7, volume=7, capabilities={},
	slot = { type="weapon.energy.laser", size=2, hardpoint=true },
	laser_stats = {
		lifespan=8, speed=1000, damage=3000, rechargeTime=0.13, length=20000,
		width=1, beam=1, dual=0, mining=0, rgba_r = 255, rgba_g = 192, rgba_b = 192, rgba_a = 255,
		heatrate=0.02, coolrate=0.01
	},
	icon_name="equip_beamlaser"
})

--===============================================
-- Plasma Accelerators
--===============================================

Equipment.Register("laser.small_plasma_accelerator", LaserType.New {
	l10n_key="SMALL_PLASMA_ACCEL",
	price=120000, purchasable=true, tech_level=10,
	mass=22, volume=22, capabilities={},
	slot = { type="weapon.energy.plasma_acc", size=5, hardpoint=true },
	laser_stats = {
		lifespan=8, speed=1000, damage=50000, rechargeTime=0.3, length=42,
		width=7, beam=0, dual=0, mining=0, rgba_r = 51, rgba_g = 255, rgba_b = 255, rgba_a = 255
	},
	icon_name="equip_plasma_accelerator"
})

Equipment.Register("laser.large_plasma_accelerator", LaserType.New {
	l10n_key="LARGE_PLASMA_ACCEL",
	price=390000, purchasable=true, tech_level=12,
	mass=50, volume=50, capabilities={},
	slot = { type="weapon.energy.plasma_acc", size=6, hardpoint=true },
	laser_stats = {
		lifespan=8, speed=1000, damage=100000, rechargeTime=0.3, length=42,
		width=7, beam=0, dual=0, mining=0, rgba_r = 127, rgba_g = 255, rgba_b = 255, rgba_a = 255
	},
	icon_name="equip_plasma_accelerator"
})

--===============================================
-- Mining Cannons
--===============================================

Equipment.Register("laser.miningcannon_5mw", LaserType.New {
	l10n_key="MININGCANNON_5MW",
	price=3700, purchasable=true, tech_level=5,
	mass=6, volume=6, capabilities={},
	slot = { type="weapon.mining", size=2, hardpoint=true },
	laser_stats = {
		lifespan=8, speed=1000, damage=5000, rechargeTime=1.5, length=30,
		width=5, beam=0, dual=0, mining=1, rgba_r = 51, rgba_g = 127, rgba_b = 0, rgba_a = 255
	},
	icon_name="equip_mining_laser"
})

Equipment.Register("laser.miningcannon_17mw", LaserType.New {
	l10n_key="MININGCANNON_17MW",
	price=10600, purchasable=true, tech_level=8,
	mass=10, volume=10, capabilities={},
	slot = { type="weapon.mining", size=4, hardpoint=true },
	laser_stats = {
		lifespan=8, speed=1000, damage=17000, rechargeTime=2, length=30,
		width=5, beam=0, dual=0, mining=1, rgba_r = 51, rgba_g = 127, rgba_b = 0, rgba_a = 255
	},
	icon_name="equip_mining_laser"
})

--===============================================
-- Missiles
--===============================================

Equipment.Register("missile.unguided_s1", MissileType.New {
	l10n_key="MISSILE_UNGUIDED",
	price=30, purchasable=true, tech_level=1,
	missile_stats = { shipType="missile_unguided", guided=false, warheadSize=200.0, fuzeRadius=100.0, effectiveRadius=1000.0, chargeEffectiveness=1.0, ecmResist=5.0 },
	volume=0, mass=0.045,
	slot = { type="missile", size=1, hardpoint=true },
	icon_name="equip_missile_unguided"
})
-- Approximately equivalent in size to an R60M / AA-8 'Aphid'
Equipment.Register("missile.guided_s1", MissileType.New {
	l10n_key="MISSILE_GUIDED",
	price=45, purchasable=true, tech_level=5,
	missile_stats = { shipType="missile_guided", guided=true, warheadSize=125.0, fuzeRadius=30.0, effectiveRadius=800.0, chargeEffectiveness=3.0, ecmResist=1.0 },
	volume=0, mass=0.065,
	slot = { type="missile", size=1, hardpoint=true },
	icon_name="equip_missile_guided"
})
-- Approximately equivalent in size to an R73 / AA-11 'Archer'
Equipment.Register("missile.guided_s2", MissileType.New {
	l10n_key="MISSILE_GUIDED",
	price=60, purchasable=true, tech_level=5,
	missile_stats = { shipType="missile_guided", guided=true, warheadSize=200.0, fuzeRadius=40.0, effectiveRadius=1500.0, chargeEffectiveness=3.5, ecmResist=1.0 },
	volume=0, mass=0.145,
	slot = { type="missile", size=2, hardpoint=true },
	icon_name="equip_missile_guided"
})
-- Approximately equivalent in size to an R77 / AA-12 'Adder'
Equipment.Register("missile.smart_s3", MissileType.New {
	l10n_key="MISSILE_SMART",
	price=95, purchasable=true, tech_level=9,
	missile_stats = { shipType="missile_smart", guided=true, warheadSize=320.0, fuzeRadius=35.0, effectiveRadius=2000.0, chargeEffectiveness=4.0, ecmResist=2.0 },
	volume=0, mass=0.5,
	slot = { type="missile", size=3, hardpoint=true },
	icon_name="equip_missile_smart"
})
-- TBD
Equipment.Register("missile.naval_s4", MissileType.New {
	l10n_key="MISSILE_NAVAL",
	price=160, purchasable=true, tech_level="MILITARY",
	missile_stats = { shipType="missile_naval", guided=true, warheadSize=580.0, fuzeRadius=40.0, effectiveRadius=2000.0, chargeEffectiveness=4.5, ecmResist=3.0 },
	volume=0, mass=1,
	slot = { type="missile", size=4, hardpoint=true },
	icon_name="equip_missile_naval"
})

--===============================================
-- Missile Pylons
--===============================================

-- "LH-420 Hydri Missile Rack"
Equipment.Register("missile_rack.524", EquipType.New {
	l10n_key="MISSILE_RACK_524",
	price=2250, purchasable=true, tech_level="MILITARY",
	volume=0.0, mass=0.8,
	slot = { type = "pylon.rack", size=5, hardpoint=true },
	provides_slots = {
		Slot:clone { id = "1", type = "missile", size = 4, hardpoint = true },
		Slot:clone { id = "2", type = "missile", size = 4, hardpoint = true },
	},
	icon_name="equip_missile_unguided"
})

-- "LH-340 Hydri Missile Rack"
Equipment.Register("missile_rack.543", EquipType.New {
	l10n_key="MISSILE_RACK_543",
	price=1800, purchasable=true, tech_level=9,
	volume=0.0, mass=1.2,
	slot = { type = "pylon.rack", size=5, hardpoint=true },
	provides_slots = {
		Slot:clone { id = "1", type = "missile", size = 3, hardpoint = true },
		Slot:clone { id = "2", type = "missile", size = 3, hardpoint = true },
		Slot:clone { id = "3", type = "missile", size = 3, hardpoint = true },
		Slot:clone { id = "4", type = "missile", size = 3, hardpoint = true },
	},
	icon_name="equip_missile_unguided"
})

-- "Cnida-104 Missile Rail"
Equipment.Register("missile_rack.414", EquipType.New {
	l10n_key="MISSILE_RAIL_S4",
	price=1125, purchasable=true, tech_level="MILITARY",
	volume=0.0, mass=0.5,
	slot = { type = "pylon.rack", size=4, hardpoint=true },
	provides_slots = {
		Slot:clone { id = "1", type = "missile", size = 4, hardpoint = true },
	},
	icon_name="equip_missile_unguided"
})

-- "LH-320 Hydri Missile Rack"
Equipment.Register("missile_rack.423", EquipType.New {
	l10n_key="MISSILE_RACK_423",
	price=900, purchasable=true, tech_level=9,
	volume=0.0, mass=0.6,
	slot = { type = "pylon.rack", size=4, hardpoint=true },
	provides_slots = {
		Slot:clone { id = "1", type = "missile", size = 3, hardpoint = true },
		Slot:clone { id = "2", type = "missile", size = 3, hardpoint = true },
	},
	icon_name="equip_missile_unguided"
})

-- "LH-240 Hydri Missile Rack"
Equipment.Register("missile_rack.442", EquipType.New {
	l10n_key="MISSILE_RACK_442",
	price=1500, purchasable=true, tech_level=5,
	volume=0.0, mass=0.8,
	slot = { type = "pylon.rack", size=4, hardpoint=true },
	provides_slots = {
		Slot:clone { id = "1", type = "missile", size = 2, hardpoint = true },
		Slot:clone { id = "2", type = "missile", size = 2, hardpoint = true },
		Slot:clone { id = "3", type = "missile", size = 2, hardpoint = true },
		Slot:clone { id = "4", type = "missile", size = 2, hardpoint = true },
	},
	icon_name="equip_missile_unguided"
})

-- "Cnida-103 Missile Rail"
Equipment.Register("missile_rack.313", EquipType.New {
	l10n_key="MISSILE_RAIL_S3",
	price=450, purchasable=true, tech_level=9,
	volume=0.0, mass=0.2,
	slot = { type = "pylon.rack", size=3, hardpoint=true },
	provides_slots = {
		Slot:clone { id = "1", type = "missile", size = 3, hardpoint = true },
	},
	icon_name="equip_missile_unguided"
})

-- "LH-220 Hydri Missile Rack"
Equipment.Register("missile_rack.322", EquipType.New {
	l10n_key="MISSILE_RACK_322",
	price=750, purchasable=true, tech_level=5,
	volume=0.0, mass=0.4,
	slot = { type = "pylon.rack", size=3, hardpoint=true },
	provides_slots = {
		Slot:clone { id = "1", type = "missile", size = 2, hardpoint = true },
		Slot:clone { id = "2", type = "missile", size = 2, hardpoint = true },
	},
	icon_name="equip_missile_unguided"
})

-- "LH-140 Hydri Missile Rack"
Equipment.Register("missile_rack.341", EquipType.New {
	l10n_key="MISSILE_RACK_341",
	price=600, purchasable=true, tech_level=1,
	volume=0.0, mass=0.5,
	slot = { type = "pylon.rack", size=3, hardpoint=true },
	provides_slots = {
		Slot:clone { id = "1", type = "missile", size = 1, hardpoint = true },
		Slot:clone { id = "2", type = "missile", size = 1, hardpoint = true },
		Slot:clone { id = "3", type = "missile", size = 1, hardpoint = true },
		Slot:clone { id = "4", type = "missile", size = 1, hardpoint = true },
	},
	icon_name="equip_missile_unguided"
})

-- "Cnida-102 Missile Rail"
Equipment.Register("missile_rack.212", EquipType.New {
	l10n_key="MISSILE_RAIL_S2",
	price=375, purchasable=true, tech_level=5,
	volume=0.0, mass=0.1,
	slot = { type = "pylon.rack", size=2, hardpoint=true },
	provides_slots = {
		Slot:clone { id = "1", type = "missile", size = 2, hardpoint = true },
	},
	icon_name="equip_missile_unguided"
})

-- "LH-120 Hydri Missile Rack"
Equipment.Register("missile_rack.221", EquipType.New {
	l10n_key="MISSILE_RACK_221",
	price=300, purchasable=true, tech_level=1,
	volume=0.0, mass=0.2,
	slot = { type = "pylon.rack", size=2, hardpoint=true },
	provides_slots = {
		Slot:clone { id = "1", type = "missile", size = 1, hardpoint = true },
		Slot:clone { id = "2", type = "missile", size = 1, hardpoint = true },
	},
	icon_name="equip_missile_unguided"
})

-- "Cnida-101 Missile Rail"
Equipment.Register("missile_rack.111", EquipType.New {
	l10n_key="MISSILE_RAIL_S1",
	price=150, purchasable=true, tech_level=1,
	volume=0.0, mass=0.1,
	slot = { type = "pylon.rack", size=1, hardpoint=true },
	provides_slots = {
		Slot:clone { id = "1", type = "missile", size = 1, hardpoint = true },
	},
	icon_name="equip_missile_unguided"
})

--===============================================
-- Internal Missile Bays
--===============================================

-- For the Coronatrix, Malabar, Sinonatrix (and Police), and Vatakara
Equipment.Register("missile_bay.opli_internal_s2", EquipType.New {
	l10n_key="OPLI_INTERNAL_MISSILE_RACK_S2",
	price=150, purchasable=true, tech_level=1,
	volume=5.0, mass=0.5,
	slot = { type = "missile_bay.opli_internal", size=2, hardpoint=true },
	provides_slots = {
		Slot:clone { id = "1", type = "missile", size = 2, hardpoint = true },
		Slot:clone { id = "2", type = "missile", size = 2, hardpoint = true },
		Slot:clone { id = "3", type = "missile", size = 2, hardpoint = true },
		Slot:clone { id = "4", type = "missile", size = 2, hardpoint = true },
		Slot:clone { id = "5", type = "missile", size = 2, hardpoint = true },
	},
	icon_name="equip_missile_unguided"
})

-- For the Natrix, and Nerodia
Equipment.Register("missile_bay.opli_internal_s3", EquipType.New {
	l10n_key="OPLI_INTERNAL_MISSILE_RACK_S3",
	price=450, purchasable=true, tech_level=5,
	volume=8.5, mass=0.9,
	slot = { type = "missile_bay.opli_internal", size=3, hardpoint=true },
	provides_slots = {
		Slot:clone { id = "1", type = "missile", size = 3, hardpoint = true },
		Slot:clone { id = "2", type = "missile", size = 3, hardpoint = true },
		Slot:clone { id = "3", type = "missile", size = 3, hardpoint = true },
	},
	icon_name="equip_missile_unguided"
})

-- For the Molar Ramsayi
-- NOTE: This is a mixed-size bay containing both S3 and S2 missiles.
Equipment.Register("missile_bay.opli_internal_s4", EquipType.New {
	l10n_key="OPLI_INTERNAL_MISSILE_RACK_S4",
	price=950, purchasable=true, tech_level=7,
	volume=11.0, mass=1.1,
	slot = { type = "missile_bay.opli_internal", size=4, hardpoint=true },
	provides_slots = {
		Slot:clone { id = "1", type = "missile", size = 3, hardpoint = true },
		Slot:clone { id = "2", type = "missile", size = 3, hardpoint = true },
		Slot:clone { id = "3", type = "missile", size = 3, hardpoint = true },
		Slot:clone { id = "4", type = "missile", size = 3, hardpoint = true },
		Slot:clone { id = "5", type = "missile", size = 3, hardpoint = true },
		Slot:clone { id = "6", type = "missile", size = 2, hardpoint = true },
		Slot:clone { id = "7", type = "missile", size = 2, hardpoint = true },
		Slot:clone { id = "8", type = "missile", size = 2, hardpoint = true },
	},
	icon_name="equip_missile_unguided"
})

Equipment.Register("missile_bay.bowfin_internal", EquipType.New {
	l10n_key="OKB_KALURI_BOWFIN_MISSILE_RACK",
	price=150, purchasable=true, tech_level=1,
	volume=0.0, mass=0.2,
	slot = { type = "missile_bay.bowfin_internal", size=2, hardpoint=true },
	provides_slots = {
		Slot:clone { id = "1", type = "missile", size = 1, hardpoint = true },
		Slot:clone { id = "2", type = "missile", size = 1, hardpoint = true },
		Slot:clone { id = "3", type = "missile", size = 1, hardpoint = true },
		Slot:clone { id = "4", type = "missile", size = 1, hardpoint = true },
		Slot:clone { id = "5", type = "missile", size = 1, hardpoint = true },
	},
	icon_name="equip_missile_unguided"
})
