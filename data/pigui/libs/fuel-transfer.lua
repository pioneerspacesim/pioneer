

local ui = require 'pigui'
local InfoView = require 'pigui.views.info-view'
local Lang = require 'Lang'
local Game = require 'Game'
local ShipDef = require 'ShipDef'
local StationView = require 'pigui.views.station-view'
local Passengers = require 'Passengers'
local Commodities = require 'Commodities'
local CommodityType = require 'CommodityType'

local l = Lang.GetResource("ui-core")

local colors = ui.theme.colors
local icons = ui.theme.icons
local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local Vector2 = _G.Vector2

local iconSize = ui.rescaleUI(Vector2(28, 28))
local fuel = {}


local function drawCentered(id, fun)
	if not ui.beginTable(id .. "##centered", 3) then return end

	ui.tableSetupColumn("leftPadding")
	ui.tableSetupColumn("content", { "WidthFixed" })
	ui.tableSetupColumn("rightPadding")

	ui.tableNextRow()
	ui.tableSetColumnIndex(1)

	fun()

	ui.endTable()
end

-- wrapper around gaugees, for consistent size, and vertical spacing
local function gauge_bar(x, text, min, max, icon)
	local height = ui.getTextLineHeightWithSpacing()
	local cursorPos = ui.getCursorScreenPos()
	local fudge_factor = 1.0
	local gaugeWidth = ui.getContentRegion().x * fudge_factor
	local gaugePos = Vector2(cursorPos.x, cursorPos.y + height * 0.5)

	ui.gauge(gaugePos, x, '', text, min, max, icon,
		colors.gaugeEquipmentMarket, '', gaugeWidth, height)

	-- ui.addRect(cursorPos, cursorPos + Vector2(gaugeWidth, height), colors.gaugeCargo, 0, 0, 1)
	ui.dummy(Vector2(gaugeWidth, height))
end



-- Gauge bar for internal, interplanetary, fuel tank
local function gauge_fuel()
	local player = Game.player
	local tankSize = ShipDef[player.shipId].fuelTankMass
	local text = string.format(l.FUEL .. ": %dt \t" .. l.DELTA_V .. ": %d km/s",
		tankSize/100 * player.fuel, player:GetRemainingDeltaV()/1000)

	gauge_bar(player.fuel, text, 0, 100, icons.fuel)
end




fuel.pumpDown = function (fuelAmt)
	local player = Game.player
	---@type CargoManager
	local cargoMgr = player:GetComponent('CargoManager')

	if player.fuel == 0 or cargoMgr:GetFreeSpace() == 0 then
		print("No fuel left to pump!")
		return
	end

	-- internal tanks capacity
	local fuelTankMass = ShipDef[player.shipId].fuelTankMass

	-- current fuel in internal tanks, in tonnes
	local availableFuel = math.floor(player.fuel / 100 * fuelTankMass)

	-- Convert fuel to positive number, don't take more fuel than is in the tank
	local drainedFuel = math.min(math.abs(fuelAmt), availableFuel)
	-- Don't pump down more fuel than we can fit in the cargo space available
	drainedFuel = math.min(drainedFuel, cargoMgr:GetFreeSpace())

	-- Military fuel is only used for the hyperdrive
	local ok = cargoMgr:AddCommodity(Commodities.hydrogen, drainedFuel)
	if not ok then
		print("Couldn't pump fuel into cargo hold!")
		return
	end

	-- Set internal thruster fuel tank state
	-- (player.fuel is percentage, between 0-100)
	player:SetFuelPercent(math.clamp(player.fuel - drainedFuel * 100 / fuelTankMass, 0, 100))
end

---@param hyperdrive Equipment.HyperdriveType
function fuel.transfer_hyperfuel_mil(hyperdrive, amt)
	local cargoMgr = Game.player:GetComponent('CargoManager')
	local availableFuel = cargoMgr:CountCommodity(hyperdrive.fuel)
	local availableCargo = cargoMgr:GetFreeSpace()

	-- Ensure we're not transferring more than the hyperdrive holds
	local delta = math.clamp(amt, -hyperdrive.storedFuel, hyperdrive:GetMaxFuel() - hyperdrive.storedFuel)
	-- Ensure we're not transferring more than the cargo bay holds
	delta = math.clamp(delta, -availableCargo, availableFuel)

	-- TODO(CargoManager): liquid tanks / partially-filled cargo containers
	-- Until then, we round down to a whole integer in both directions since we're dealing with integer cargo masses
	delta = math.modf(delta)

	if delta > 0 then
		cargoMgr:RemoveCommodity(hyperdrive.fuel, delta)
		hyperdrive:SetFuel(Game.player, hyperdrive.storedFuel + delta)
	elseif delta < 0 then
		cargoMgr:AddCommodity(hyperdrive.fuel, math.abs(delta))
		hyperdrive:SetFuel(Game.player, hyperdrive.storedFuel + delta)
	end
end

function fuel.fuelTransferButton(drive, amt)
	local icon = amt < 0 and icons.chevron_down or icons.chevron_up

	if ui.button(ui.get_icon_glyph(icon) .. tostring(math.abs(amt))) then

		if drive.fuel == Commodities.hydrogen then
			fuel.transfer_hyperfuel_hydrogen(drive, amt)
		elseif drive.fuel == Commodities.military_fuel then
			fuel.transfer_hyperfuel_mil(drive, amt)
		end

	end
end

function fuel.drawCentered(id, fun)
	if not ui.beginTable(id .. "##centered", 3) then return end

	ui.tableSetupColumn("leftPadding")
	ui.tableSetupColumn("content", { "WidthFixed" })
	ui.tableSetupColumn("rightPadding")

	ui.tableNextRow()
	ui.tableSetColumnIndex(1)

	fun()

	ui.endTable()
end

-- a function to dransfer fuel from the ship tank to the hyperdrive
function fuel.drawFuelTransfer(drive)
	-- Don't allow transferring fuel while the hyperdrive is doing its thing
	if Game.player:IsHyperspaceActive() then return end

	drawCentered("Hyperdrive", function()
		ui.horizontalGroup(function()
			fuel.fuelTransferButton(drive, 10)
			fuel.fuelTransferButton(drive, 1)

			ui.text(l.TRANSFER_FUEL)

			fuel.fuelTransferButton(drive, -1)
			fuel.fuelTransferButton(drive, -10)
		end)
	end)
end

--ui for above
function fuel.drawPumpDialog()
	local width1 = ui.calcTextSize(l.PUMP_DOWN)
	local width2 = ui.calcTextSize(l.REFUEL)

	local width
	if width2.x > width1.x then width = width2.x else width = width1.x end
	width = width * 1.2

	-- to-do: maybe show disabled version of button if fuel==100 or hydrogen==0
	-- (if so, what about military fuel?)
	-- at the moment: no visual cue for this.
	ui.alignTextToLineHeight(ui.getButtonHeight())
	ui.text(l.REFUEL)
	ui.sameLine(width)
	local options = {1, 10, 100}
	for _, k in ipairs(options) do
		if ui.button(tostring(k)  .. "##fuel", Vector2(100, 0)) then
			-- Refuel k tonnes from cargo hold
			Game.player:Refuel(Commodities.hydrogen, k)
		end
		ui.sameLine()
	end
	ui.text("")

	-- To-do: Maybe also show disabled version of button if currentfuel == 0 or player:GetEquipFree("cargo") == 0
	-- at the moment: no visual cue for this.
	ui.alignTextToLineHeight(ui.getButtonHeight())
	ui.text(l.PUMP_DOWN)
	ui.sameLine(width)
	for _, v in ipairs(options) do
		local fuelVal = -1*v
		if ui.button(fuelVal .. "##pump", Vector2(100, 0)) then
			fuel.pumpDown(fuel)
		end
		ui.sameLine()
	end
	ui.text("")
end




return fuel
