
local debugView = require 'pigui.views.debug'
local ShipDef = require 'ShipDef'
local ShipBuilder = require 'modules.MissionUtils.ShipBuilder'
local ShipConfig = require 'ShipConfig'

local Rules = ShipBuilder.OutfitRules

local ui = require 'pigui'

local debugRule = ShipBuilder.Template:clone {
	label = "TEST SHIP",
	shipId = 'coronatrix',
	rules = {
		{
			slot = "weapon",
			filter = "weapon.energy",
			limit = 1
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
			limit = 2
		},
		Rules.DefaultHyperdrive,
		Rules.DefaultLaserCooling,
		Rules.DefaultAtmoShield,
	}
}

debugView.registerTab("ShipBuilder", {
	label = "Ship Builder",
	icon = ui.theme.icons.ship,

	selectedHull = nil,
	spawnThreat = 20.0,

	show = function() return not require 'Game':InHyperspace() end,

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

					ui.endTabItem()
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
					local config = ShipConfig[self.selectedHull]

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
				for id, def in pairs(ShipDef) do
					if ui.selectable(id) then
						self.selectedHull = id
					end

					local threat = ShipBuilder.GetHullThreat(id)
					ui.sameLine()
					ui.text("threat: " .. tostring(threat.total))
				end
			end
		end)
	end
})
