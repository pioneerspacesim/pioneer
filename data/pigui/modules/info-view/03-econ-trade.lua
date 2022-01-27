-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local InfoView = require 'pigui.views.info-view'
local Lang = require 'Lang'
local Game = require "Game"
local Equipment = require "Equipment"
local ShipDef = require "ShipDef"
local StationView = require 'pigui.views.station-view'
local Format = require "Format"
local l = Lang.GetResource("ui-core")

local colors = ui.theme.colors
local icons = ui.theme.icons
local pionillium = ui.fonts.pionillium
local Vector2 = _G.Vector2

local iconSize = ui.rescaleUI(Vector2(28, 28))
local buttonSpaceSize = iconSize
-- FIXME: need to manually set itemSpacing to be able to properly size columns
-- Need a style-var query system for best effect
local itemSpacing = ui.rescaleUI(Vector2(6, 12), Vector2(1600, 900))

local shipDef
local hyperdrive
local hyperdrive_fuel

local jettison = function (item)
	local enabled = Game.player.flightState == "FLYING"
	local tooltip = l.JETTISON .. "##".. item:GetName()

	local button = ui.iconButton(icons.cargo_crate_illegal, buttonSpaceSize, tooltip)

	if button and enabled then
		Game.player:Jettison(item)
	end

	return button
end

local cachedCargoList = nil
local maxCargoWidth = 0
local function rebuildCargoList()
	local count = {}
	local maxCargoCount = 0
	for _, et in pairs(Game.player:GetEquip("cargo")) do
		if not count[et] then count[et] = 0 end
		count[et] = count[et]+1
		maxCargoCount = math.max(count[et], maxCargoCount)
	end

	cachedCargoList = count
	maxCargoWidth = ui.calcTextSize(maxCargoCount.."t").x + itemSpacing.x * 2
	return count
end

local function cargolist ()
	local count = cachedCargoList or rebuildCargoList()

	ui.columns(3, "cargoTable")

	ui.setColumnWidth(0, maxCargoWidth)
	for et, nb in pairs(count) do
		-- count
		local text = nb .. "t"
		ui.text(text)
		ui.nextColumn()

		-- name
		ui.text(et:GetName())
		ui.nextColumn()

		-- jettison button
		jettison(et)
		ui.nextColumn()
	end
	ui.columns(1, "")
end


local pumpDown = function (fuel)
	local player = Game.player
	if player.fuel == 0 or player:GetEquipFree("cargo") == 0 then
		print("No fuel left to pump!")
		return
	end

	-- internal tanks capacity
	local fuelTankMass = ShipDef[player.shipId].fuelTankMass

	-- current fuel in internal tanks, in tonnes
	local availableFuel = math.floor(player.fuel / 100 * fuelTankMass)

	fuel = fuel * -1

	-- Don't go above 100%
	if fuel > availableFuel then fuel = availableFuel end

	-- Military fuel is only used for the hyperdrive
	local drainedFuel = player:AddEquip(Equipment.cargo.hydrogen, fuel)

	-- Set internal thruster fuel tank state
	-- (player.fuel is percentage, between 0-100)
	player:SetFuelPercent(math.clamp(player.fuel - drainedFuel * 100 / fuelTankMass, 0, 100))
end

-- wrapper around gaugees, for consistent size, and vertical spacing
local function gauge_bar(x, text, min, max, icon)
	local height = ui.getTextLineHeight()
	local gaugePos = ui.getCursorScreenPos()
	local fudge_factor = 1.0
	local gaugeWidth = ui.getContentRegion().x * fudge_factor
	gaugePos.y = gaugePos.y + height

	ui.gauge(gaugePos, x, '', text, min, max, icon,
		colors.gaugeEquipmentMarket, '', gaugeWidth, height)

	ui.text("")
	ui.text("")
end

-- Gauge bar for internal, interplanetary, fuel tank
local function gauge_fuel()
	local player = Game.player
	local text = string.format(l.FUEL .. ": %dt \t" .. l.DELTA_V .. ": %d km/s",
		shipDef.fuelTankMass/100 * player.fuel, player:GetRemainingDeltaV()/1000)

	gauge_bar(player.fuel, text, 0, 100, icons.fuel)
end

-- Gauge bar for hyperdrive fuel / range
local function gauge_hyperdrive()
	local player = Game.player
	local text = string.format(l.FUEL .. ": %dt \t" .. l.HYPERSPACE_RANGE .. ": %d " .. l.LY,
		player:CountEquip(hyperdrive_fuel), player:GetHyperspaceRange())

	gauge_bar(player:CountEquip(hyperdrive_fuel), text, 0,
		player.totalCargo - player.usedCargo + player:CountEquip(hyperdrive_fuel),
		icons.hyperspace)
end

-- Gauge bar for used/free cargo space
local function gauge_cargo()
	local player = Game.player
	gauge_bar(player.usedCargo, string.format('%%it %s / %it %s', l.USED, player.totalCargo - player.usedCargo, l.FREE),
		0, player.totalCargo, icons.market)
end

-- Gauge bar for used/free cabins
local function gauge_cabins()
	local player = Game.player
	local cabins_total = player:GetEquipCountOccupied("cabin")
	local cabins_free = player.cabin_cap or 0
	local cabins_used = cabins_total - cabins_free
	gauge_bar(cabins_used, string.format('%%i %s / %i %s', l.USED, cabins_free, l.FREE),
		0, cabins_total, icons.personal)
end

local function drawEconTrade()
	local player = Game.player

	if ui.collapsingHeader(l.FUEL, {"DefaultOpen"}) then
		gauge_fuel()

		local width1 = ui.calcTextSize(l.PUMP_DOWN)
		local width2 = ui.calcTextSize(l.REFUEL)

		local width
		if width2.x > width1.x then width = width2.x else width = width1.x end
		width = width * 1.2

		-- to-do: maybe show disabled version of button if fuel==100 or hydrogen==0
		-- (if so, what about military fuel?)
		-- at the moment: no visual cue for this.
		ui.text(l.REFUEL)
		ui.sameLine(width)
		local options = {1, 10, 100}
		for _, k in ipairs(options) do
			if ui.button(tostring(k)  .. "##fuel", Vector2(100, 0)) then
				-- Refuel k tonnes from cargo hold
				Game.player:Refuel(k)
			end
			ui.sameLine()
		end
		ui.text("")

		-- To-do: Maybe also show disabled version of button if currentfuel == 0 or player:GetEquipFree("cargo") == 0
		-- at the moment: no visual cue for this.
		ui.text(l.PUMP_DOWN)
		ui.sameLine(width)
		for _, v in ipairs(options) do
			local fuel = -1*v
			if ui.button(fuel .. "##pump", Vector2(100, 0)) then
				pumpDown(fuel)
			end
			ui.sameLine()
		end
		ui.text("")
	end

	gauge_hyperdrive()

	if ui.collapsingHeader(l.CABINS, {"DefaultOpen"}) then

		gauge_cabins()
	end

	if ui.collapsingHeader(l.FINANCE, {"DefaultOpen"}) then
		local cash = player:GetMoney()
		ui.text(l.CASH)
		ui.sameLine()
		ui.text(Format.Money(cash))
	end

end

InfoView:registerView({
	id = "econTrade",
	name = l.ECONOMY_TRADE,
	icon = ui.theme.icons.cargo_manifest,
	showView = true,
	draw = function()
		ui.withStyleVars({ItemSpacing = itemSpacing}, function()
			ui.withFont(pionillium.medlarge, function()
				local sizex = (ui.getColumnWidth() - itemSpacing.x) / 2
				local sizey = ui.getContentRegion().y - StationView.style.height
				ui.child("leftpanel", Vector2(sizex, sizey), function()
					drawEconTrade()
				end)

				ui.sameLine()

				ui.child("rightpanel", Vector2(sizex, sizey), function()
					if ui.collapsingHeader(l.CARGO, {"DefaultOpen"}) then
						gauge_cargo()
						cargolist()
					end
				end)
			end)
		end)
	end,
	refresh = function()
		Game.player.equipSet:AddListener(function (slot)
			if slot == "cargo" then cachedCargoList = nil end
		end)
		cachedCargoList = nil
		shipDef = ShipDef[Game.player.shipId]
		hyperdrive = table.unpack(Game.player:GetEquip("engine")) or nil
		hyperdrive_fuel = hyperdrive and hyperdrive.fuel or Equipment.cargo.hydrogen
	end,
	debugReload = function()
		package.reimport()
	end
})
