-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Equipment = require 'Equipment'
local Commodities = require 'Commodities'

local HyperdriveType = require '.Types'.HyperdriveType

--
-- Civilian Drives
--

-- Player-flyable ships
Equipment.Register("hyperspace.hyperdrive_1", HyperdriveType.New {
	l10n_key="DRIVE_CLASS1", fuel=Commodities.hydrogen,
	slot = { type="hyperdrive.civilian", size=1 },
	mass=1.4, volume=2.5, capabilities={ hyperclass=1 },
	fuel_resv_size = 1, factor_eff = 65,
	price=1700, purchasable=true, tech_level=3,
	icon_name="equip_hyperdrive"
})
Equipment.Register("hyperspace.hyperdrive_2", HyperdriveType.New {
	l10n_key="DRIVE_CLASS2", fuel=Commodities.hydrogen,
	slot = { type="hyperdrive.civilian", size=2 },
	mass=4, volume=6, capabilities={ hyperclass=2 },
	fuel_resv_size = 4, factor_eff = 60,
	price=2300, purchasable=true, tech_level=4,
	icon_name="equip_hyperdrive"
})
Equipment.Register("hyperspace.hyperdrive_3", HyperdriveType.New {
	l10n_key="DRIVE_CLASS3", fuel=Commodities.hydrogen,
	slot = { type="hyperdrive.civilian", size=3 },
	mass=9.5, volume=15, capabilities={ hyperclass=3 },
	fuel_resv_size = 10, factor_eff = 55,
	price=7500, purchasable=true, tech_level=4,
	icon_name="equip_hyperdrive"
})
Equipment.Register("hyperspace.hyperdrive_4", HyperdriveType.New {
	l10n_key="DRIVE_CLASS4", fuel=Commodities.hydrogen,
	slot = { type="hyperdrive.civilian", size=4 },
	mass=25, volume=40, capabilities={ hyperclass=4 },
	fuel_resv_size = 30, factor_eff = 42,
	price=21000, purchasable=true, tech_level=6,
	icon_name="equip_hyperdrive"
})
Equipment.Register("hyperspace.hyperdrive_5", HyperdriveType.New {
	l10n_key="DRIVE_CLASS5", fuel=Commodities.hydrogen,
	slot = { type="hyperdrive.civilian", size=5 },
	mass=76, volume=120, capabilities={ hyperclass=5 },
	fuel_resv_size = 75, factor_eff = 35,
	price=68000, purchasable=true, tech_level=7,
	icon_name="equip_hyperdrive"
})

-- Small bulk-ship jumpdrive
Equipment.Register("hyperspace.hyperdrive_6", HyperdriveType.New {
	l10n_key="DRIVE_CLASS6", fuel=Commodities.hydrogen,
	slot = { type="hyperdrive.civilian", size=6 },
	mass=152, volume=340, capabilities={ hyperclass=6 },
	fuel_resv_size = 120, factor_eff = 30,
	price=129000, purchasable=true, tech_level=7,
	icon_name="equip_hyperdrive"
})
-- Large bulk-ship jumpdrive
Equipment.Register("hyperspace.hyperdrive_7", HyperdriveType.New {
	l10n_key="DRIVE_CLASS7", fuel=Commodities.hydrogen,
	slot = { type="hyperdrive.civilian", size=7 },
	mass=540, volume=960, capabilities={ hyperclass=7 },
	fuel_resv_size = 350, factor_eff = 25,
	price=341000, purchasable=true, tech_level=9,
	icon_name="equip_hyperdrive"
})

--
-- Military Drives (not yet ported)
--

Equipment.Register("hyperspace.hyperdrive_mil1", HyperdriveType.New {
	l10n_key="DRIVE_MIL1", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives,
	slot = { type="hyperdrive.military", size=1 },
	mass=2, volume=2.5, capabilities={ hyperclass=1 },
	fuel_resv_size = 1, factor_eff = 75,
	price=23000, purchasable=true, tech_level=10,
	icon_name="equip_hyperdrive_mil"
})
Equipment.Register("hyperspace.hyperdrive_mil2", HyperdriveType.New {
	l10n_key="DRIVE_MIL2", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives,
	slot = { type="hyperdrive.military", size=2 },
	mass=5.5, volume=6, capabilities={ hyperclass=2 },
	fuel_resv_size = 4, factor_eff = 68,
	price=47000, purchasable=true, tech_level="MILITARY",
	icon_name="equip_hyperdrive_mil"
})
Equipment.Register("hyperspace.hyperdrive_mil3", HyperdriveType.New {
	l10n_key="DRIVE_MIL3", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives,
	slot = { type="hyperdrive.military", size=3 },
	mass=12.5, volume=15, capabilities={ hyperclass=3 },
	fuel_resv_size = 10, factor_eff = 60,
	price=85000, purchasable=true, tech_level=11,
	icon_name="equip_hyperdrive_mil"
})
Equipment.Register("hyperspace.hyperdrive_mil4", HyperdriveType.New {
	l10n_key="DRIVE_MIL4", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives,
	slot = { type="hyperdrive.military", size=4 },
	mass=32, volume=40, capabilities={ hyperclass=4 },
	fuel_resv_size = 30, factor_eff = 48,
	price=214000, purchasable=true, tech_level=12,
	icon_name="equip_hyperdrive_mil"
})
Equipment.Register("hyperspace.hyperdrive_mil5", HyperdriveType.New {
	l10n_key="DRIVE_MIL5", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives,
	slot = { type="hyperdrive.military", size=5 },
	mass=105, volume=120, capabilities={ hyperclass=5 },
	fuel_resv_size = 75, factor_eff = 42,
	price=540000, purchasable=false, tech_level="MILITARY",
	icon_name="equip_hyperdrive_mil"
})
