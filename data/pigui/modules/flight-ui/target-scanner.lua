-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local Game = require 'Game'
local Vector2 = _G.Vector2

-- cache ui
local pionillium = ui.fonts.pionillium
local colors = ui.theme.colors
local icons = ui.theme.icons

local lc = require 'Lang'.GetResource("core")
local lui = require 'Lang'.GetResource("ui-core")

-- cache player each frame
local player = nil

local gameView = require 'pigui.views.game'

local shipInfoLowerBound
local function displayTargetScannerFor(target, offset)
	local hull = target:GetHullPercent()
	local shield = target:GetShieldsPercent()
	local class = target:GetShipType()
	local label = target.label
	local engine = target:GetEquip('engine', 1)
	local stats = target:GetStats()
	local mass = stats.staticMass
	local cargo = stats.usedCargo
	if engine then
		engine = engine:GetName()
	else
		engine = 'No Hyperdrive'
	end
	local uiPos = Vector2(ui.screenWidth - 30, 1 * ui.gauge_height)
	if shield then
		ui.gauge(uiPos - Vector2(ui.gauge_width, 0), shield, nil, nil, 0, 100, icons.shield, colors.gaugeShield, lui.HUD_SHIELD_STRENGTH)
	end
	uiPos.y = uiPos.y + ui.gauge_height * 1.5
	ui.gauge(uiPos - Vector2(ui.gauge_width, 0), hull, nil, nil, 0, 100, icons.hull, colors.gaugeHull, lui.HUD_HULL_STRENGTH)
	uiPos.y = uiPos.y + ui.gauge_height * 0.8
	local r = ui.addStyledText(uiPos, ui.anchor.right, ui.anchor.top, label, colors.frame, pionillium.medium, nil, colors.lightBlackBackground)
	uiPos.y = uiPos.y + r.y + offset
	r = ui.addStyledText(uiPos, ui.anchor.right, ui.anchor.top, class, colors.frame, pionillium.medium, nil, colors.lightBlackBackground)
	uiPos.y = uiPos.y + r.y + offset
	r = ui.addStyledText(uiPos, ui.anchor.right, ui.anchor.top, engine, colors.frame, pionillium.medium, nil, colors.lightBlackBackground)
	uiPos.y = uiPos.y + r.y + offset
	r = ui.addFancyText(uiPos, ui.anchor.right, ui.anchor.top, {
		{ text=lui.HUD_MASS .. ' ', color=colors.reticuleCircleDark, font=pionillium.medium },
		{ text=mass, color=colors.reticuleCircle, font=pionillium.medium, },
		{ text=lc.UNIT_TONNES, color=colors.reticuleCircleDark, font=pionillium.medium }
	}, colors.lightBlackBackground)
	uiPos.y = uiPos.y + r.y + offset
	r = ui.addFancyText(uiPos, ui.anchor.right, ui.anchor.top, {
		{ text=lui.HUD_CARGO_MASS .. ' ', color=colors.reticuleCircleDark, font=pionillium.medium, },
		{ text=cargo, color=colors.reticuleCircle, font=pionillium.medium },
		{ text=lc.UNIT_TONNES, color=colors.reticuleCircleDark, font=pionillium.medium }
	}, colors.lightBlackBackground)
	shipInfoLowerBound = uiPos + Vector2(0, r.y + 15)
end

local function displayTargetScanner()
	local offset = 7
	shipInfoLowerBound = Vector2(ui.screenWidth - 30, 1 * ui.gauge_height)
	if player:GetEquipCountOccupied('target_scanner') > 0 or player:GetEquipCountOccupied('advanced_target_scanner') > 0 then
           -- what is the difference between target_scanner and advanced_target_scanner?
		local target = player:GetNavTarget()
		if target and target:IsShip() then
			displayTargetScannerFor(target, offset)
		else
			target = player:GetCombatTarget()
			if target and target:IsShip() then
				displayTargetScannerFor(target, offset)
			end
		end
	end
	if player:GetEquipCountOccupied('hypercloud') > 0 then
		local target = player:GetNavTarget()
		if target and target:IsHyperspaceCloud() then
			local arrival = target:IsArrival()
			local ship = target:GetShip()
			if ship then
				local stats = ship:GetStats()
				local mass = stats.staticMass
				local path,destName = ship:GetHyperspaceDestination()
				local date = target:GetDueDate()
				local dueDate = ui.Format.Datetime(date)
				local uiPos = shipInfoLowerBound + Vector2(0, 15)
				local name = (arrival and lc.HYPERSPACE_ARRIVAL_CLOUD or lc.HYPERSPACE_DEPARTURE_CLOUD)
				local r = ui.addStyledText(uiPos, ui.anchor.right, ui.anchor.top, name , colors.frame, pionillium.medium, nil, colors.lightBlackBackground)
				uiPos = uiPos + Vector2(0, r.y + offset)
				r = ui.addFancyText(uiPos, ui.anchor.right, ui.anchor.top, {
					{ text=lui.HUD_MASS .. ' ', color=colors.reticuleCircleDark, font=pionillium.medium },
					{ text=mass, color=colors.reticuleCircle, font=pionillium.medium },
					{ text=lc.UNIT_TONNES, color=colors.reticuleCircleDark, font=pionillium.medium }
				}, colors.lightBlackBackground)
				uiPos.y = uiPos.y + r.y + offset
				r = ui.addStyledText(uiPos, ui.anchor.right, ui.anchor.top, destName, colors.frame, pionillium.medium, nil, colors.lightBlackBackground)
				uiPos.y = uiPos.y + r.y + offset
				ui.addStyledText(uiPos, ui.anchor.right, ui.anchor.top, dueDate, colors.frame, pionillium.medium, nil, colors.lightBlackBackground)
			else
				local uiPos = Vector2(ui.screenWidth - 30, 1 * ui.gauge_height)
				ui.addStyledText(uiPos, ui.anchor.right, ui.anchor.top, lc.HYPERSPACE_ARRIVAL_CLOUD_REMNANT , colors.frame, pionillium.medium, nil, colors.lightBlackBackground)
			end
		end
	end
end

gameView.registerModule('target-scanner', {
    showInHyperspace = false,
    draw = function(self, dT)
        player = gameView.player
        displayTargetScanner()
    end
})
