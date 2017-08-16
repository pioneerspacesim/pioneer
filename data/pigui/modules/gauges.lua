local Engine = import('Engine')
local Game = import('Game')
local ui = import('pigui/pigui.lua')
local Vector = import('Vector')
local Color = import('Color')
local Lang = import("Lang")
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");
local utils = import("utils")
local Event = import("Event")

local pionillium = ui.fonts.pionillium
local pionicons = ui.fonts.pionicons
local colors = ui.theme.colors
local icons = ui.theme.icons


function addGauge(priority, valueFun, unit, format, min, max, icon, color, tooltip)
	ui.registerGauge(function()
			local v = valueFun()
			return { value = v, unit = unit, format = format, min = min, max = max, icon = icon, color = color, tooltip = tooltip }
									 end
		, priority)
end

addGauge(0, function ()
					 local t = Game.player:GetGunTemperature(0) * 100
					 if t and t > 0 then
						 return t
					 else
						 return nil
					 end
						end, nil, nil, 0, 100, icons.forward, colors.gaugeWeapon, lui.HUD_FORWARD_GUN_TEMPERATURE)
addGauge(1, function ()
					 local t = Game.player:GetGunTemperature(1) * 100
					 if t and t > 0 then
						 return t
					 else
						 return nil
					 end
						end, nil, nil, 0, 100, icons.backward, colors.gaugeWeapon, lui.HUD_BACKWARD_GUN_TEMPERATURE)
addGauge(2, function ()
					 local frame = Game.player.frameBody
					 if frame then
						 local pressure, density = frame:GetAtmosphericState()
						 return pressure
					 else
						 return nil
					 end
						end,
				 'atm', '%.2f', 0, 15, icons.pressure, colors.gaugePressure, lui.HUD_ATMOSPHERIC_PRESSURE)
addGauge(3, function ()
					 local t = Game.player:GetHullTemperature() * 100
					 if t and t > 0 then
						 return t
					 else
						 return nil
					 end
						end, nil, nil, 0, 100, icons.temperature, colors.gaugeTemperature, lui.HUD_HULL_TEMPERATURE)
addGauge(4, function () return Game.player:GetShieldsPercent() end, nil, nil, 0, 100, icons.shield, colors.gaugeShield, lui.HUD_SHIELD_STRENGTH)
addGauge(5, function () return Game.player:GetHullPercent() end, nil, nil, 0, 100, icons.hull, colors.gaugeHull, lui.HUD_HULL_STRENGTH)


return {}
