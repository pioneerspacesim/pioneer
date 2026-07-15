-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Lang = require 'Lang'
local Format = require 'Format'
local Gravity = require 'pigui.libs.gravity'
local TwrGauge = require 'pigui.libs.twr-gauge'
local lui = Lang.GetResource("ui-core")
local ui = require 'pigui'
local Vector2 = _G.Vector2

local font = ui.fonts.pionillium.medium
local windowFlags = ui.WindowFlags {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoScrollbar"}

-- Extra horizontal gap between label and value columns (pixels).
local LABEL_VALUE_GAP = 48
local EARTH_G = 9.80665
local TWR_BAR_PADDING = Vector2(8, 2)

local function shouldShowDisplay()
	if ui.optionsWindow.isOpen then return false end
	if not Game.player then return false end
	local view = Game.CurrentView()
	if not view then return false end
	-- These sreens already have something in the bottom right, so we don't show the widget on them
	if view == "SectorView" then return false end
	if view == "WorldView" then return false end
	-- We want to show on the system overview (Atlas) but not on the system map (Orrery)
	if view == "SystemView" and Game.systemView:GetDisplayMode() == "Orrery" then return false end
	return true
end

local function getTwrTargetBody(player)
	if Game.CurrentView() == "SystemView" and Game.systemView:GetDisplayMode() == "Atlas" then
		local obj = Game.systemView:GetSelectedObject()
		if obj and obj.ref and obj.ref.path then
			return obj.ref.physicsBody or obj.ref
		end
	end
	return player:GetNavTarget()
end

local function drawTwrBar(twr)
	local pos = ui.getCursorScreenPos() - Vector2(0, TWR_BAR_PADDING.y + 1)
	local size = Vector2(ui.getContentRegion().x, ui.getTextLineHeight() + (TWR_BAR_PADDING.y * 2))
	TwrGauge.DrawBar(pos, size, twr)
end

local function drawRow(row)
	if row.bar then
		drawTwrBar(row.twr)
	end

	local labelWidth = ui.calcTextSize(row.label).x
	ui.addCursorPos(Vector2(TWR_BAR_PADDING.x, 0))
	ui.text(row.label)
	if row.tooltip and ui.isItemHovered() then
		ui.setTooltip(row.tooltip)
	end
	local winPad = ui.getWindowPadding().x
	local width = ui.getContentRegion().x - TWR_BAR_PADDING.x
	local textWidth = ui.calcTextSize(row.value).x
	ui.sameLine(math.max(width - textWidth, labelWidth + LABEL_VALUE_GAP) + winPad, 0)

	if row.bar then
		ui.withStyleColors({ Text = TwrGauge.GetTextColor(row.twr) }, function()
			ui.text(row.value)
		end)
	else
		ui.text(row.value)
	end
	if row.tooltip and ui.isItemHovered() then
		ui.setTooltip(row.tooltip)
	end
end

local function displayMassTwrDisplay()
	if not shouldShowDisplay() then return end

	local player = Game.player
	if not player then return end

	local totalMass = player.staticMass + player.fuelMassLeft
	local upAccel = player:GetAcceleration("up")
	local showCurrentTwr = player:IsDocked() or player:IsLanded()
	local targetBody = getTwrTargetBody(player)

	local localGravity = Gravity.GetGravityAtBody(player)
	local twrCurrent = localGravity and (upAccel / localGravity) or math.huge

	local rows = {
		{ label = lui.TOTAL_MASS, value = Format.MassTonnes(totalMass) },
		{ label = lui.LOCAL_GRAVITY, value = ui.Format.Gravity((localGravity or 0) / EARTH_G) },
	}
	if showCurrentTwr then
		table.insert(rows, {
			label = lui.TWR_CURRENT,
			value = ui.Format.TWR(twrCurrent, 2),
			twr = twrCurrent,
			tooltip = lui.TWR_CURRENT_TOOLTIP,
			bar = true,
		})
	end
	if targetBody then
		local targetGravity = Gravity.GetSurfaceGravity(targetBody)
		local twrTarget = targetGravity and (upAccel / targetGravity) or math.huge
		table.insert(rows, {
			label = lui.TWR_TARGET,
			value = ui.Format.TWR(twrTarget, 2),
			twr = twrTarget,
			tooltip = lui.TWR_TARGET_TOOLTIP,
			bar = true,
		})
	end

	local labelWidth, valueWidth = 0, 0
	ui.withFont(font.name, font.size, function()
		for _, row in ipairs(rows) do
			labelWidth = math.max(labelWidth, ui.calcTextSize(row.label).x)
			valueWidth = math.max(valueWidth, ui.calcTextSize(row.value).x)
		end
	end)

	local padding = ui.getWindowPadding()
	local width = math.max(labelWidth + valueWidth + LABEL_VALUE_GAP + (padding.x * 2) + (TWR_BAR_PADDING.x * 2), 1)
	local itemSpacing = ui.getItemSpacing().y
	local contentHeight = (#rows * font.size) + (math.max(#rows - 1, 0) * itemSpacing)
	local height = math.max(contentHeight + (padding.y * 2) + (TWR_BAR_PADDING.y - 1), 1)

	ui.setNextWindowSize(Vector2(width, height), "Always")
	ui.setNextWindowPos(Vector2(ui.screenWidth - width, ui.screenHeight - height), "Always")

	ui.window("MassTwr", windowFlags, function()
		ui.withFont(font.name, font.size, function()
			for _, row in ipairs(rows) do
				drawRow(row)
			end
		end)
	end)
end

ui.registerModule("game", { id = "mass-twr-display", draw = displayMassTwrDisplay })

return {}
