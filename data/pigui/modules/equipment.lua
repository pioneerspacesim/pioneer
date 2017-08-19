local Engine = import('Engine')
local Game = import('Game')
local ui = import('pigui/pigui.lua')
local Vector = import('Vector')
local Color = import('Color')
local Lang = import("Lang")
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");
local lec = Lang.GetResource("equipment-core");
local utils = import("utils")
local Event = import("Event")

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons

local mainIconSize = Vector(32,32) * (ui.screenHeight / 1200)
local mainWideIconSize = Vector(64,32) * (ui.screenHeight / 1200)
local mainForegroundColor = colors.reticuleCircle
local mainBackgroundColor = colors.lightBlueBackground
local mainHoverColor = colors.lightBlueBackground:tint(0.2)
local mainPressedColor = colors.lightBlueBackground:tint(0.5)
local mainButtonPadding = 3
local function iconButton(leftupper, icon, is_wide, icon_size, text, is_disabled, bg_color, fg_color, hover_color, pressed_color, tooltip)
	local textsize = ui.calcTextSize(text)
	local x = icon_size.x + textsize.x + mainButtonPadding * 2
	local y = math.max(icon_size.y, textsize.y) -- + mainButtonPadding * 2
	local clicked = false
	if is_disabled then
		bg_color = colors.grey
	end
	if ui.isMouseHoveringRect(leftupper, leftupper + Vector(x, y)) and not ui.isMouseHoveringAnyWindow() then
		if not is_disabled then
			if ui.isMouseDown(0) then
				bg_color = pressed_color
			else
				bg_color = hover_color
			end
			if ui.isMouseReleased(0) then
				clicked = true
			end
		end
		ui.setTooltip(tooltip)
	end
	ui.addRectFilled(leftupper, leftupper + Vector(x, y), bg_color, 0.1, 15)
	if is_wide then
		ui.addWideIcon(leftupper + Vector(mainButtonPadding, mainButtonPadding), icon, fg_color, icon_size, ui.anchor.left, ui.anchor.top)
	else
		ui.addIcon(leftupper + Vector(mainButtonPadding, mainButtonPadding), icon, fg_color, icon_size, ui.anchor.left, ui.anchor.top)
	end
	ui.addText(leftupper + Vector(icon_size.x + mainButtonPadding, (icon_size.y - textsize.y)/2), fg_color, text) --  + mainButtonPadding
	return Vector(x, y), clicked
end

local function displayECM(uiPos)
	player = Game.player
	local current_view = Game.CurrentView()
	if current_view == "world" then
		local ecms = player:GetEquip('ecm')
		for i,ecm in ipairs(ecms) do
			local size, clicked = iconButton(uiPos, icons[ecm.ecm_type], false, mainIconSize, "ECM", not player:IsECMReady(), mainBackgroundColor, mainForegroundColor, mainHoverColor, mainPressedColor, 'ECM')
			uiPos = uiPos + Vector(0, size.y + 10)
			if clicked then
				player:UseECM()
			end
		end
	end
	return uiPos
end

local function displayBodyScanner(uiPos)
	player = Game.player
	local current_view = Game.CurrentView()
	if current_view == "world" then
		local sensors = player:GetEquip('sensor')
		for i,sensor in ipairs(sensors) do
			local size,clicked = iconButton(uiPos, icons.scanner, false, mainIconSize, sensor:GetName(), false, mainBackgroundColor, mainForegroundColor, mainHoverColor, mainPressedColor, 'Body scanner')
			uiPos = uiPos + Vector(0, size.y + 10)
			if clicked then
				player:StartSensor(i)
				-- or StopSensor(i)
			end
		end
	end
	return uiPos
end

local function getMissileIcon(missile)
	if icons[missile.missile_type] then
		return icons[missile.missile_type]
	else
		print("no icon for missile " .. missile.missile_type)
		return icons.bullseye
	end
end

local function fireMissile(index)
	if not player:GetCombatTarget() then
		Game.AddCommsLogLine(lc.SELECT_A_TARGET, "", 1)
	else
		player:FireMissileAt(index, player:GetCombatTarget())
	end
end

local function displayMissiles(uiPos)
	player = Game.player
	local current_view = Game.CurrentView()
	if current_view == "world" then
		local missiles = player:GetEquip('missile')
		local count = {}
		local types = {}
		local index = {}
		for i,missile in ipairs(missiles) do
			count[missile.missile_type] = (count[missile.missile_type] or 0) + 1
			types[missile.missile_type] = missile
			index[missile.missile_type] = i
		end
		for t,missile in pairs(types) do
			local c = count[t]
			local size,clicked = iconButton(uiPos, getMissileIcon(missile), true, mainWideIconSize, c, c == 0, mainBackgroundColor, mainForegroundColor, mainHoverColor, mainPressedColor, lec[missile.l10n_key])
			uiPos = uiPos + Vector(0, size.y + 10)
			if clicked then
				print("firing missile " .. t .. ", " .. index[t])
				fireMissile(index[t])
			end
		end
	end
	return uiPos
end

local function displayEquipment()
	local uiPos = Vector(15, ui.screenHeight / 3 + 10)
	uiPos = displayMissiles(uiPos)
	uiPos = displayECM(uiPos + Vector(0, 10))
	uiPos = displayBodyScanner(uiPos + Vector(0, 10))
end

ui.registerModule("game", displayEquipment)

return {}
