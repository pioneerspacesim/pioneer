local Equipment = require 'Equipment'

local ExplorerData
ExplorerData = {

	memberCost = 2,
	deviceCost = 14,

	explorerRank = 0,
	-- explorerInvite: 0, no invite, 1, active invite, 2, invite accepted (member), 3, invite rejected
	explorerInvite = 0,
	systemsExplored = 0,
	jumpsMade = 0,
	lightyearsTraveled = 0,
	-- bountylist: make a list of systems explored while a member of the explorers guild
	-- 			   upon docking and connecting to explorers guild, list is used to calculate rewards
	bountylist = {},
	-- hyperSource: save the system path we are jumping from to accumulate lightyearstraveled
	hyperSource = nil,

	device = Equipment.EquipType.New({
		l10n_key="EXPLORER_DEVICE", l10n_resource="module-explorerclub", slots="explorer_device", price=0,
		capabilities={mass=0, explorer_device=1},
		purchasable=false, tech_level=5, infovis=1
	})

}

return ExplorerData