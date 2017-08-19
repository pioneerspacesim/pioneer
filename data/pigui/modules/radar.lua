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

local player = nil
local pionillium = ui.fonts.pionillium
local colors = ui.theme.colors
local icons = ui.theme.icons

local MAX_RADAR_SIZE = 1000000000
local MIN_RADAR_SIZE = 1000

local shouldDisplay2DRadar = false
local current_radar_size = 10000

local function getColorFor(item)
	local body = item.body
	if body == player:GetNavTarget() then
		return colors.radarNavTarget
	elseif body == player:GetCombatTarget() then
		return colors.radarCombatTarget
	elseif body:IsShip() then
		return colors.radarShip
	elseif body:IsHyperspaceCloud() then
		return colors.radarCloud
	elseif body:IsMissile() then
		return colors.radarMissile
	elseif body:IsStation() then
		return colors.radarStation
	elseif body:IsCargoContainer() then
		return colors.radarCargo
	end
	return colors.grey
end

-- display the 2D radar
local function display2DRadar(cntr, size)
	local targets = ui.getTargetsNearby(current_radar_size)
	local halfsize = size * 0.5
	local thirdsize = size * 0.3
	local twothirdsize = size * 0.7

	local function line(x,y)
		-- ui.addLine(cntr + Vector(x, y) * halfsize, cntr + Vector(x,y) * size, colors.reticuleCircle, ui.reticuleCircleThickness)
		ui.addLine(cntr + Vector(x, y) * thirdsize, cntr + Vector(x,y) * twothirdsize, colors.reticuleCircle, ui.reticuleCircleThickness)
	end
	ui.addCircleFilled(cntr, size, colors.lightBlueBackground, ui.circleSegments(size), 1)
	ui.addCircle(cntr, size, colors.reticuleCircle, ui.circleSegments(size), ui.reticuleCircleThickness)
	--	ui.addCircle(cntr, halfsize, colors.reticuleCircle, ui.circleSegments(halfsize), ui.reticuleCircleThickness)
	ui.addCircle(cntr, thirdsize, colors.reticuleCircle, ui.circleSegments(thirdsize), ui.reticuleCircleThickness)
	ui.addCircle(cntr, twothirdsize, colors.reticuleCircle, ui.circleSegments(twothirdsize), ui.reticuleCircleThickness)
	local l = ui.oneOverSqrtTwo
	line(-l, l)
	line(-l, -l)
	line(l, -l)
	line(l, l)
	local combatTarget = player:GetCombatTarget()
	local navTarget = player:GetNavTarget()
	local tooltip = {}
	for k,v in pairs(targets) do
		if v.distance < current_radar_size then
			local halfRadarSize = current_radar_size / 2
			local alpha = 255
			if v.distance > halfRadarSize then
				alpha = 255 * (1 - (v.distance - halfRadarSize) / halfRadarSize)
			end
			local position = cntr + v.aep * size * 2
			if v.body == navTarget then
				local color = Color(colors.navTarget.r, colors.navTarget.g, colors.navTarget.b, alpha)
				ui.addIcon(position, icons.square, color, 12, ui.anchor.center, ui.anchor.center)
			elseif v.body == combatTarget then
				local color = Color(colors.combatTarget.r, colors.combatTarget.g, colors.combatTarget.b, alpha)
				ui.addIcon(position, icons.square, color, 12, ui.anchor.center, ui.anchor.center)
			else
				local color = getColorFor(v)
				ui.addCircleFilled(position, 2, color, 4, 1)
			end
			local mouse_position = ui.getMousePos()
			if (mouse_position - position):magnitude() < 4 then
				table.insert(tooltip, v.label)
			end
		end
	end
	if #tooltip > 0 then
		ui.setTooltip(table.concat(tooltip, "\n"))
	end
	local d, d_u = ui.Format.Distance(current_radar_size)
	local distance = d .. ' ' .. d_u
	local textcenter = cntr + Vector((halfsize + twothirdsize) * 0.5, size)
	local textsize = ui.addStyledText(textcenter, ui.anchor.left, ui.anchor.bottom, distance, colors.frame, pionillium.small, lui.HUD_RADAR_DISTANCE, colors.lightBlackBackground)
end

local click_on_radar = false
-- display either the 3D or the 2D radar, show a popup on right click to select
local function displayRadar()
	player = Game.player
	local radar = player:GetEquip("radar")
	-- only display if there actually *is* a radar installed
	if #radar > 0 then

		local size = ui.reticuleCircleRadius * 0.66
		local cntr = Vector(ui.screenWidth / 2, ui.screenHeight - size - 15)

		local mp = ui.getMousePos()
		if (Vector(mp.x,mp.y) - cntr):magnitude() > size then
			click_on_radar = false
		end
		if (Vector(mp.x,mp.y) - cntr):magnitude() < size then
			if ui.isMouseClicked(1) then
				click_on_radar = true
			end
			if click_on_radar and ui.isMouseReleased(1) then
				ui.openPopup("radarselector")
			end
			local wheel = ui.getMouseWheel()
			if wheel > 0 then
				current_radar_size = math.max(current_radar_size / 10, MIN_RADAR_SIZE)
			elseif wheel < 0 then
				current_radar_size = math.min(current_radar_size * 10, MAX_RADAR_SIZE)
			end
		end
		ui.popup("radarselector", function()
							 if ui.selectable(lui.HUD_2D_RADAR, shouldDisplay2DRadar, {}) then
								 Event.Queue('changeMFD', 'radar')
							 end
							 if ui.selectable(lui.HUD_3D_RADAR, not shouldDisplay2DRadar, {}) then
								 Event.Queue('changeMFD', 'scanner')
							 end
		end)
		if shouldDisplay2DRadar then
			display2DRadar(cntr, size)
		end
	end
end

Event.Register('changeMFD', function(selected)
								 Event.Queue('onChangeMFD', selected)
end)

Event.Register('onChangeMFD', function(selected)
								 if selected == "radar" then
									 shouldDisplay2DRadar = true;
									 Game.SetRadarVisible(false)
								 elseif selected == "scanner" then
									 Game.SetRadarVisible(true)
									 shouldDisplay2DRadar = false;
								 end
end)


ui.registerModule("game", displayRadar)

return {}
