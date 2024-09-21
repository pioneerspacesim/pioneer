-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local HullConfig = require 'HullConfig'
local ShipDef = require 'ShipDef'
local ShipBuilder = require 'modules.MissionUtils.ShipBuilder'
local utils       = require 'utils'

local debugView = require 'pigui.views.debug'

local Rules = ShipBuilder.OutfitRules

local ui = require 'pigui'

local debugRule = ShipBuilder.Template:clone {
	label = "TEST SHIP",
	-- shipId = 'coronatrix',
	hyperclass = 1,
	role = "mercenary",
	rules = {
		{
			slot = "weapon",
			filter = "weapon.energy",
			pick = "random",
			limit = 1,
			maxThreatFactor = 0.6
		},
		{
			slot = "missile_rack",
			maxSize = 2
		},
		{
			slot = "missile",
			maxSize = 2,
			minSize = 2,
			limit = 4
		},
		{
			slot = "missile",
			maxSize = 1,
			minSize = 1,
		},
		{
			slot = "shield",
			maxSize = 1,
			limit = 2,
			balance = true
		},
		Rules.DefaultHyperdrive,
		-- Rules.DefaultLaserCooling,
		{
			slot = nil,
			equip = "misc.laser_cooling_booster",
			limit = 1,
			minThreat = 15.0
		},
		Rules.DefaultAtmoShield,
	}
}

debugView.registerTab("ShipBuilder", {
	label = "Ship Builder",
	icon = ui.theme.icons.ship,

	selectedHull = nil,
	spawnThreat = 20.0,

	show = function() return require 'Game'.player end,

	draw = function(self)
		self.spawnThreat = ui.sliderFloat("Threat", self.spawnThreat, 1.0, 200.0)

		if ui.button("Spawn Test Ship") then
			ShipBuilder.MakeShipNear(require 'Game'.player, debugRule, self.spawnThreat, 15.00, 20.00)
		end

		local target = require 'Game'.player:GetCombatTarget()

		if target then
			ui.text("Equipment:")
			ui.spacing()

			local equipSet = target:GetComponent('EquipSet')

			for id, equip in pairs(equipSet:GetInstalledEquipment()) do
				ui.text(id .. ": " .. equip:GetName())
			end

			ui.spacing()
		end

		ui.child("hullList", function()
			if self.selectedHull then
				if ui.button("<") then
					self.selectedHull = nil
					return
				end

				ui.sameLine()
				ui.text(self.selectedHull)

				ui.spacing()

				if ui.collapsingHeader("Hull Threat") then
					local threat = ShipBuilder.GetHullThreat(self.selectedHull)

					for k, v in pairs(threat) do
						ui.text(tostring(k) .. ": " .. tostring(v))
					end
				end

				if ui.collapsingHeader("Slots") then
					local config = HullConfig.GetHullConfigs()[self.selectedHull]

					for k, v in pairs(config.slots) do
						ui.text(k)

						local data = "  "
						for k2, v2 in pairs(v) do
							data = data .. tostring(k2) .. " = " .. tostring(v2) .. ", "
						end

						ui.text(data)
					end
				end
			else
				local hulls = utils.build_array(pairs(ShipDef))

				table.sort(hulls, function(a, b)
					return ShipBuilder.GetHullThreat(a.id).total < ShipBuilder.GetHullThreat(b.id).total
				end)

				for _, def in ipairs(hulls) do
					if def.tag == "SHIP" then
						if ui.selectable(def.id) then
							self.selectedHull = def.id
						end

						local threat = ShipBuilder.GetHullThreat(def.id)
						ui.sameLine()
						ui.text("threat: " .. tostring(threat.total))
					end
				end
			end
		end)
	end
})
