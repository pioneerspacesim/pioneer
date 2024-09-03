-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local utils= require 'utils'
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
function gauges.displayGauges(min, max)
	if gauges.dirty then
		table.sort(gauges, function(a,b) return a.priority < b.priority end)
		gauges.dirty = false
	end

	local drawGauges = utils.map_array(gauges, function(g)
		local value = g.value()
		if value then return { value, g } end
	end)

	local spacing = ui.gauge_height * 1.4
	local height = spacing * #drawGauges

	local pos, size = ui.rectcut(min, max, height, ui.sides.bottom)

	ui.setNextWindowPos(pos, "Always")
	ui.setNextWindowSize(size, "Always")
	ui.setNextWindowPadding(Vector2(0, 0))

	ui.window("PlayerGauges", gaugeWindowFlags, function()
		local uiPos = ui.getCursorScreenPos() + Vector2(0, ui.gauge_height * 0.5)
		for i, t in ipairs(drawGauges) do
			local value, g = t[1], t[2]
			ui.gauge(uiPos, value, g.unit, g.format, g.min, g.max, g.icon, g.color, g.tooltip)
			uiPos = uiPos + Vector2(0, spacing)
		end
	end)
end

gauges.registerGauge(0, {
	value = function ()
		local t = Game.player:GetGunTemperature(0) * 100
		if t and t > 0 then return t
		else return nil end
	end,
	icon = icons.forward, color = colors.gaugeWeapon,
	tooltip = lui.HUD_FORWARD_GUN_TEMPERATURE
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
			local pressure, density = frame:GetAtmosphericState(Game.player)
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

gameView.registerHudModule("gauges", {
	side = "left",
	showInHyperspace = false,
	debugReload = function() package.reimport() end,
	draw = function(_, min, max)
		colors = ui.theme.colors
		icons = ui.theme.icons
		gauges.displayGauges(min, max)
	end
})

return gauges
