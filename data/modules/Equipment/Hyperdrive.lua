-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Equipment = require 'Equipment'
local Commodities = require 'Commodities'

local HyperdriveType = require 'EquipType'.HyperdriveType

Equipment.Register("hyperspace.hyperdrive_1", HyperdriveType.New {
	l10n_key="DRIVE_CLASS1", fuel=Commodities.hydrogen,
	slot = { type="hyperdrive", size=1 },
	mass=2, volume=2, capabilities={ hyperclass=1 },
	price=700, purchasable=true, tech_level=3,
	icon_name="equip_hyperdrive"
})
Equipment.Register("hyperspace.hyperdrive_2", HyperdriveType.New {
	l10n_key="DRIVE_CLASS2", fuel=Commodities.hydrogen,
	slot = { type="hyperdrive", size=2 },
	mass=6, volume=6, capabilities={ hyperclass=2 },
	price=1300, purchasable=true, tech_level=4,
	icon_name="equip_hyperdrive"
})
Equipment.Register("hyperspace.hyperdrive_3", HyperdriveType.New {
	l10n_key="DRIVE_CLASS3", fuel=Commodities.hydrogen,
	slot = { type="hyperdrive", size=3 },
	mass=11, volume=11, capabilities={ hyperclass=3 },
	price=2500, purchasable=true, tech_level=4,
	icon_name="equip_hyperdrive"
})
Equipment.Register("hyperspace.hyperdrive_4", HyperdriveType.New {
	l10n_key="DRIVE_CLASS4", fuel=Commodities.hydrogen,
	slot = { type="hyperdrive", size=4 },
	mass=25, volume=25, capabilities={ hyperclass=4 },
	price=5000, purchasable=true, tech_level=5,
	icon_name="equip_hyperdrive"
})
Equipment.Register("hyperspace.hyperdrive_5", HyperdriveType.New {
	l10n_key="DRIVE_CLASS5", fuel=Commodities.hydrogen,
	slot = { type="hyperdrive", size=5 },
	mass=60, volume=60, capabilities={ hyperclass=5 },
	price=10000, purchasable=true, tech_level=5,
	icon_name="equip_hyperdrive"
})
Equipment.Register("hyperspace.hyperdrive_6", HyperdriveType.New {
	l10n_key="DRIVE_CLASS6", fuel=Commodities.hydrogen,
	slot = { type="hyperdrive", size=6 },
	mass=130, volume=130, capabilities={ hyperclass=6 },
	price=20000, purchasable=true, tech_level=6,
	icon_name="equip_hyperdrive"
})
Equipment.Register("hyperspace.hyperdrive_7", HyperdriveType.New {
	l10n_key="DRIVE_CLASS7", fuel=Commodities.hydrogen,
	slot = { type="hyperdrive", size=7 },
	mass=245, volume=245, capabilities={ hyperclass=7 },
	price=30000, purchasable=true, tech_level=8,
	icon_name="equip_hyperdrive"
})
Equipment.Register("hyperspace.hyperdrive_8", HyperdriveType.New {
	l10n_key="DRIVE_CLASS8", fuel=Commodities.hydrogen,
	slot = { type="hyperdrive", size=8 },
	mass=360, volume=360, capabilities={ hyperclass=8 },
	price=60000, purchasable=true, tech_level=9,
	icon_name="equip_hyperdrive"
})
Equipment.Register("hyperspace.hyperdrive_9", HyperdriveType.New {
	l10n_key="DRIVE_CLASS9", fuel=Commodities.hydrogen,
	slot = { type="hyperdrive", size=9 },
	mass=540, volume=540, capabilities={ hyperclass=9 },
	price=120000, purchasable=true, tech_level=10,
	icon_name="equip_hyperdrive"
})
Equipment.Register("hyperspace.hyperdrive_mil1", HyperdriveType.New {
	l10n_key="DRIVE_MIL1", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives,
	slot = { type="hyperdrive", size=1 },
	mass=1, volume=1, capabilities={ hyperclass=1 },
	price=23000, purchasable=true, tech_level=10,
	icon_name="equip_hyperdrive_mil"
})
Equipment.Register("hyperspace.hyperdrive_mil2", HyperdriveType.New {
	l10n_key="DRIVE_MIL2", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives,
	slot = { type="hyperdrive", size=2 },
	mass=3, volume=3, capabilities={ hyperclass=2 },
	price=47000, purchasable=true, tech_level="MILITARY",
	icon_name="equip_hyperdrive_mil"
})
Equipment.Register("hyperspace.hyperdrive_mil3", HyperdriveType.New {
	l10n_key="DRIVE_MIL3", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives,
	slot = { type="hyperdrive", size=3 },
	mass=5, volume=5, capabilities={ hyperclass=3 },
	price=85000, purchasable=true, tech_level=11,
	icon_name="equip_hyperdrive_mil"
})
Equipment.Register("hyperspace.hyperdrive_mil4", HyperdriveType.New {
	l10n_key="DRIVE_MIL4", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives,
	slot = { type="hyperdrive", size=4 },
	mass=13, volume=13, capabilities={ hyperclass=4 },
	price=214000, purchasable=true, tech_level=12,
	icon_name="equip_hyperdrive_mil"
})
Equipment.Register("hyperspace.hyperdrive_mil5", HyperdriveType.New {
	l10n_key="DRIVE_MIL5", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives,
	slot = { type="hyperdrive", size=5 },
	mass=29, volume=29, capabilities={ hyperclass=5 },
	price=540000, purchasable=false, tech_level="MILITARY",
	icon_name="equip_hyperdrive_mil"
})
Equipment.Register("hyperspace.hyperdrive_mil6", HyperdriveType.New {
	l10n_key="DRIVE_MIL6", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives,
	slot = { type="hyperdrive", size=6 },
	mass=60, volume=60, capabilities={ hyperclass=6 },
	price=1350000, purchasable=false, tech_level="MILITARY",
	icon_name="equip_hyperdrive_mil"
})
Equipment.Register("hyperspace.hyperdrive_mil7", HyperdriveType.New {
	l10n_key="DRIVE_MIL7", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives,
	slot = { type="hyperdrive", size=7 },
	mass=135, volume=135, capabilities={ hyperclass=7 },
	price=3500000, purchasable=false, tech_level="MILITARY",
	icon_name="equip_hyperdrive_mil"
})
Equipment.Register("hyperspace.hyperdrive_mil8", HyperdriveType.New {
	l10n_key="DRIVE_MIL8", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives,
	slot = { type="hyperdrive", size=8 },
	mass=190, volume=190, capabilities={ hyperclass=8 },
	price=8500000, purchasable=false, tech_level="MILITARY",
	icon_name="equip_hyperdrive_mil"
})
Equipment.Register("hyperspace.hyperdrive_mil9", HyperdriveType.New {
	l10n_key="DRIVE_MIL9", fuel=Commodities.military_fuel, byproduct=Commodities.radioactives,
	slot = { type="hyperdrive", size=9 },
	mass=260, volume=260, capabilities={ hyperclass=9 },
	price=22000000, purchasable=false, tech_level="MILITARY",
	icon_name="equip_hyperdrive_mil"
})
