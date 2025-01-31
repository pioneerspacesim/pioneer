-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local EquipTypes = require '.Types'
local Equipment = require 'Equipment'

local EquipType = EquipTypes.EquipType
local BodyScannerType = EquipTypes.BodyScannerType

--===============================================
-- ECM
--===============================================

Equipment.Register("misc.ecm_basic", EquipType.New {
	l10n_key="ECM_BASIC",
	price=6000, purchasable=true, tech_level=9,
	slot = { type="utility.ecm", size=1, hardpoint=true },
	mass=2, volume=3, capabilities={ ecm_power=2, ecm_recharge=5 },
	ecm_type = 'ecm',
	hover_message="ECM_HOVER_MESSAGE"
})

Equipment.Register("misc.ecm_advanced", EquipType.New {
	l10n_key="ECM_ADVANCED",
	price=15200, purchasable=true, tech_level="MILITARY",
	slot = { type="utility.ecm", size=2, hardpoint=true },
	mass=2, volume=5, capabilities={ ecm_power=3, ecm_recharge=5 },
	ecm_type = 'ecm_advanced',
	hover_message="ECM_HOVER_MESSAGE"
})

--===============================================
-- Scanners
--===============================================

Equipment.Register("misc.target_scanner", EquipType.New {
	l10n_key="TARGET_SCANNER",
	price=900, purchasable=true, tech_level=9,
	slot = { type="utility.scanner.combat_scanner", size=1, hardpoint=true },
	mass=0.5, volume=0, capabilities={ target_scanner_level=1 },
	icon_name="equip_scanner"
})

Equipment.Register("misc.advanced_target_scanner", EquipType.New {
	l10n_key="ADVANCED_TARGET_SCANNER",
	price=1200, purchasable=true, tech_level="MILITARY",
	slot = { type="utility.scanner.combat_scanner", size=2, hardpoint=true },
	mass=1.0, volume=0, capabilities={ target_scanner_level=2 },
	icon_name="equip_scanner"
})

Equipment.Register("misc.hypercloud_analyzer", EquipType.New {
	l10n_key="HYPERCLOUD_ANALYZER",
	price=1500, purchasable=true, tech_level=10,
	slot = { type="utility.scanner.hypercloud", size=1, hardpoint=true },
	mass=0.5, volume=0, capabilities={ hypercloud_analyzer=1 },
	icon_name="equip_scanner"
})

Equipment.Register("misc.planetscanner", BodyScannerType.New {
	l10n_key = 'SURFACE_SCANNER',
	price=2950, purchasable=true, tech_level=5,
	slot = { type="utility.scanner.planet", size=1, hardpoint=true },
	mass=1, volume=1, capabilities={ sensor=1 },
	stats={ aperture = 50.0, minAltitude = 150, resolution = 768, orbital = false },
	icon_name="equip_planet_scanner"
})

Equipment.Register("misc.planetscanner_good", BodyScannerType.New {
	l10n_key = 'SURFACE_SCANNER_GOOD',
	price=5000, purchasable=true, tech_level=8,
	slot = { type="utility.scanner.planet", size=2, hardpoint=true },
	mass=2, volume = 2, capabilities={ sensor=1 },
	stats={ aperture = 65.0, minAltitude = 250, resolution = 1092, orbital = false },
	icon_name="equip_planet_scanner"
})

Equipment.Register("misc.orbitscanner", BodyScannerType.New {
	l10n_key = 'ORBIT_SCANNER',
	price=7500, purchasable=true, tech_level=3,
	slot = { type="utility.scanner.planet", size=1, hardpoint=true },
	mass=3, volume=2, capabilities={ sensor=1 },
	stats={ aperture = 4.0, minAltitude = 650000, resolution = 6802, orbital = true },
	icon_name="equip_orbit_scanner"
})

Equipment.Register("misc.orbitscanner_good", BodyScannerType.New {
	l10n_key = 'ORBIT_SCANNER_GOOD',
	price=11000, purchasable=true, tech_level=8,
	slot = { type="utility.scanner.planet", size=2, hardpoint=true },
	mass=7, volume=4, capabilities={ sensor=1 },
	stats={ aperture = 2.8, minAltitude = 1750000, resolution = 12375, orbital = true },
	icon_name="equip_orbit_scanner"
})
