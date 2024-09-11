
local debugView = require 'pigui.views.debug'
local ShipDef = require 'ShipDef'
local ShipBuilder = require 'modules.MissionUtils.ShipBuilder'
local ShipConfig = require 'ShipConfig'

local ui = require 'pigui'

debugView.registerTab("ShipBuilder", {
	selectedHull = nil,
	draw = function(self)
		if not ui.beginTabItem("ShipBuilder") then
			return
		end

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

		ui.endTabItem()
	end
})
