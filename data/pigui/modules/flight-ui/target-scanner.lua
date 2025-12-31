-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local Vector2 = _G.Vector2

-- cache ui
local font_heading = ui.fonts.pionillium.heading
local font_content = ui.fonts.pionillium.body
local colors = ui.theme.colors
local icons = ui.theme.icons

local lc = require 'Lang'.GetResource("core")
local lui = require 'Lang'.GetResource("ui-core")
local gameView = require 'pigui.views.game'

-- cache player each frame
local player = nil ---@type Player

local scannerWindowFlags = ui.WindowFlags { "NoTitleBar", "NoResize",
	"NoFocusOnAppearing", "NoBringToFrontOnFocus" }

-- Format the mass in tonnes
-- The ui.Format() function reformats the units whereas we always want to
-- display using tonnes.
local function formatMass(massInTonnes)
	return string.format("%d %s", massInTonnes or 0, lc.UNIT_TONNES)
end

-- draw a 2-column table of name/value data with optional headings
local function drawTable(data, heading1, heading2)
	-- data is a table containing:
	-- - name of item
	-- - value of item
	if ui.beginTable("Data", 2) then
		ui.tableSetupColumn(heading1 or "", { "WidthStretch" })
		ui.tableSetupColumn(heading2 or "", { "WidthFixed" } )
		if heading1 or heading2 then
			ui.withFont(font_heading, function()
				ui.tableHeadersRow()
			end)
		end
		for _, item in pairs(data) do
			ui.tableNextRow()
			ui.tableNextColumn()
			ui.text(item.name)
			ui.tableNextColumn()
			ui.text(item.value)
		end
		ui.endTable()
	end
end

---@param target Ship
local function displayTargetScannerFor(target, maxWidth)
	local hull = target:GetHullPercent()
	local shield = target:GetShieldsPercent() or 0
	local engine = target:GetInstalledHyperdrive()

	local data = {
		{ name = lui.HYPERDRIVE, value = engine and engine:GetName() or lui.NO_DRIVE },
		{ name = lui.HUD_MASS, value = formatMass(target.staticMass) },
		{ name = lui.HUD_CARGO_MASS, value = formatMass(target.usedCargo) },
	}

	ui.withFont(font_content, function()
		drawTable(data, target.label, target:GetShipType())
	end)

	local spacing = ui.gauge_height * 1.4
	local yOff = Vector2(0, ui.gauge_height * 0.5)
	local uiPos = ui.getCursorScreenPos()

	if shield then
		ui.gauge(uiPos + yOff, shield, nil, nil, 0, 100, icons.shield,
			colors.gaugeShield, lui.HUD_SHIELD_STRENGTH, maxWidth)
		yOff.y = yOff.y + spacing
	end
	ui.gauge(uiPos + yOff, hull, nil, nil, 0, 100, icons.hull,
		colors.gaugeHull, lui.HUD_HULL_STRENGTH, maxWidth)
	yOff.y = yOff.y + spacing
end

local function displayTargetScanner(min, max)
	local scanner_level = (player["target_scanner_level_cap"] or 0)
	if scanner_level == 0 then return end

	-- what is the difference between target_scanner and advanced_target_scanner?
	local target = player:GetNavTarget()
	if not target or not target:IsShip() then
		target = player:GetCombatTarget()
	end
	if not target or not target:IsShip() then
		return
	end

	local textHeight = ui.getTextLineHeightWithSpacing()
	local headingHeight = textHeight
	ui.withFont(font_heading, function()
		headingHeight = ui.getTextLineHeightWithSpacing()
	end)
	-- TODO : it would be nice if we didn't have to pre-calculate this and could
	-- instead somehow get it from the displayTargetScannerFor() function
	local height = (ui.gauge_height * 1.4) * 2 + textHeight * 3 + headingHeight + ui.gauge_height * 0.5

	local pos, size = ui.rectcut(min, max, height, ui.sides.bottom)

	ui.setNextWindowPos(pos, "Always")
	ui.setNextWindowSize(size, "Always")
	ui.setNextWindowPadding(Vector2(0, 0))
	ui.window("TargetScanner", scannerWindowFlags, function()
		displayTargetScannerFor(target, size.x)
	end)
end

local function displayCloudScanner(min, max)
	local hypercloud_level = (player["hypercloud_analyzer_cap"] or 0)
	local target = player:GetNavTarget()

	if hypercloud_level == 0 or not target or not target:IsHyperspaceCloud() then
		return
	end

	local arrival = target:IsArrival()
	local ship = target:GetShip()

	local height = ui.getTextLineHeightWithSpacing(font_heading)
		+ ui.getTextLineHeightWithSpacing() * 4
		+ ui.getItemSpacing().y

	local pos, size = ui.rectcut(min, max, height, ui.sides.bottom)

	ui.setNextWindowPos(pos, "Always")
	ui.setNextWindowSize(size, "Always")
	ui.setNextWindowPadding(Vector2(0, 0))
	ui.window("CloudScanner", scannerWindowFlags, function()
		ui.addRectFilled(pos, pos + Vector2(ui.getContentRegion().x,
		                 ui.getFrameHeight()), colors.TableHeaderBg, 0,
		                 ui.RoundCornersNone)
		ui.withFont(font_heading, function()
			ui.text(target:GetLabel())
		end)
		if ship then
			-- NOTE: ships in arrival clouds have their destination set to the
			-- source system. This should probably get refactored some time in
			-- the future. While clever reuse of existing state, it makes for
			-- challenging maintenance and understanding of the code.
			local _,systemName = ship:GetHyperspaceDestination()
			local systemLabel = arrival and lui.HUD_HYPERSPACE_ORIGIN or lui.HUD_HYPERSPACE_DESTINATION

			local data = {
				{ name = lui.SHIP_TYPE, value = ship:GetShipType()},
				{ name = lui.HUD_MASS, value = formatMass(ship.staticMass) },
				{ name = systemLabel, value = systemName },
				{ name = lui.HUD_ARRIVAL_DATE, value = ui.Format.Datetime(target:GetDueDate()) }
			}
			ui.withFont(font_content, function()
				drawTable(data)
			end)
		end
	end) -- window function
end

local function onDebugReload()
	package.reimport()
end

gameView.registerHudModule('target-scanner', {
	side = "right",
	showInHyperspace = false,
	debugReload = onDebugReload,
	draw = function(_, min, max)
		colors = ui.theme.colors
		icons = ui.theme.icons
		player = gameView.player
		-- restrict this HUD module to the width of the time window
		if ui.timeWindowSize then
			min.x = math.max(min.x, max.x - ui.timeWindowSize.x)
		end
		displayTargetScanner(min, max)
	end
})

gameView.registerHudModule('hyperspacecloud-scanner', {
	side = "right",
	showInHyperspace = false,
	debugReload = onDebugReload,
	draw = function(_, min, max)
		colors = ui.theme.colors
		icons = ui.theme.icons
		player = gameView.player
		-- restrict this HUD module to the width of the time window
		if ui.timeWindowSize then
			min.x = math.max(min.x, max.x - ui.timeWindowSize.x)
		end
		displayCloudScanner(min, max)
	end
})
