-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Vector2 = _G.Vector2

local ui = require 'pigui'
local lui = require 'Lang'.GetResource("ui-core")

local icons = ui.theme.icons
local colors = ui.theme.colors

local gameView = require 'pigui.views.game'

local gauges = {
	gaugeCount = 0
}

function gauges.registerGauge(priority, data)
	data.min = data.min or 0
	data.max = data.max or 100
	data.priority = priority

	table.insert(gauges, data)
	gauges.gaugeCount = gauges.gaugeCount + 1
	gauges.dirty = true
end

local gaugeWindowFlags = ui.WindowFlags {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"}
function gauges.displayGauges()
	if gauges.dirty then
		table.sort(gauges, function(a,b) return a.priority < b.priority end)
		gauges.dirty = false
	end

	local gauge_stretch = 1.4
	local current_view = Game.CurrentView()
	local c = gauges.gaugeCount + 0.1
	if current_view == "world" then
		ui.setNextWindowSize(Vector2(ui.gauge_width, ui.gauge_height * c * gauge_stretch), "Always")
		local tws = ui.timeWindowSize
		if not tws then
			tws = Vector2(0, 100)
		end
		tws = tws + Vector2(0, 30) -- extra offset
		ui.setNextWindowPos(Vector2(5, ui.screenHeight - tws.y - ui.gauge_height * c * gauge_stretch), "Always")
		ui.window("PlayerGauges", gaugeWindowFlags, function()
			local uiPos = ui.getWindowPos() + Vector2(0, ui.gauge_height)
			for k,g in ipairs(gauges) do
				local value = g.value()
				if value then
					ui.gauge(uiPos, value, g.unit, g.format, g.min, g.max, g.icon, g.color, g.tooltip)
					uiPos = uiPos + Vector2(0, ui.gauge_height * gauge_stretch)
				end
			end
		end)
	end
end

gauges.registerGauge(0, {
	value = function ()
		local t = Game.player:GetGunTemperature(0) * 100
		if t and t > 0 then return t
		else return nil end
	end,
	icon = icons.forward, color = colors.gaugeWeapon,
	tooltop = lui.HUD_FORWARD_GUN_TEMPERATURE
})

gauges.registerGauge(1, {
	value = function ()
		local t = Game.player:GetGunTemperature(1) * 100
		if t and t > 0 then return t
		else return nil end
	end,
	icon = icons.backward, color = colors.gaugeWeapon,
	tooltip = lui.HUD_BACKWARD_GUN_TEMPERATURE
})

gauges.registerGauge(2, {
	value = function ()
		local frame = Game.player.frameBody
		if frame then
			local pressure, density = frame:GetAtmosphericState()
			return pressure
		else return nil end
	end,
	unit = 'atm', format = '%.2f', min = 0, max = 15,
	icon = icons.pressure, color = colors.gaugePressure,
	tooltip = lui.HUD_ATMOSPHERIC_PRESSURE
})

gauges.registerGauge(3, {
	value = function ()
		local t = Game.player:GetHullTemperature() * 100
		if t and t > 0 then return t
		else return nil end
	end,
	icon = icons.temperature, color = colors.gaugeTemperature,
	tooltip = lui.HUD_HULL_TEMPERATURE
})

gauges.registerGauge(4, {
	value = function () return Game.player:GetShieldsPercent() end,
	icon = icons.shield, color = colors.gaugeShield, tooltip = lui.HUD_SHIELD_STRENGTH
})

gauges.registerGauge(5, {
	value = function () return Game.player:GetHullPercent() end,
	icon = icons.hull, color = colors.gaugeHull, tooltip = lui.HUD_HULL_STRENGTH
})

gameView.registerModule("gauges", {
	showInHyperspace = false,
	draw = function()
		colors = ui.theme.colors
		icons = ui.theme.icons
		gauges.displayGauges()
	end
})

return gauges
