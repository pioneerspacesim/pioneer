local ui = require 'pigui'
local Lang = require 'Lang'
local Game = require 'Game'
local ShipDef = require 'ShipDef'

local Commodities = require 'Commodities'
local CommodityType = require 'CommodityType'

local Comms = require 'Comms'
--local HyperDrive = require 'Hyperdrive'
local HyperdriveType = require 'EquipType'.HyperdriveType
local l = Lang.GetResource("ui-core")
local le = Lang.GetResource("equipment-core")
local lmf = Lang.GetResource("module-fuel")

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

-- todo keep this consistant with 03-econ-trade
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
function fuel.gauge_fuel()
    local player = Game.player
    local tankSize = ShipDef[player.shipId].fuelTankMass
    local text = string.format(l.FUEL .. ": %dt \t" .. l.DELTA_V .. ": %d km/s",
        tankSize / 100 * player.fuel, player:GetRemainingDeltaV() / 1000)

    gauge_bar(player.fuel, text, 0, 100, icons.fuel)
end

--transfer fuel from cargo to main tank
function fuel.pumpDown(fuelAmt)
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

-- a utility function for collecting fuel related values in one place
---@param hyperdrive Equipment.HyperdriveType
function fuel.getFuelStats(hyperdrive)
    local fuel_stats = {}

    -- main thruster fuel
    fuel_stats.main = {
        size = ShipDef[Game.player.shipId].fuelTankMass,                           -- Maximum tank size
        left = Game.player.fuelMassLeft,                                           -- Fuel left in the tank
        free = ShipDef[Game.player.shipId].fuelTankMass - Game.player.fuelMassLeft -- Free space in the tank
    }

    -- cargo fuel
    local cargoMgr = Game.player:GetComponent('CargoManager')
    fuel_stats.cargo = {
        -- todo maybe ad size here for consistancy
        size = cargoMgr:GetTotalSpace(),
        left = cargoMgr:CountCommodity(hyperdrive.fuel),       -- Fuel left in cargo
        free = cargoMgr:GetFreeSpace(),                        -- Free cargo space
        fuelType = hyperdrive.fuel,
        leftH = cargoMgr:CountCommodity(Commodities.hydrogen), -- for mil drive fuel computer to know how much main thruster fuel reserver
    }

    -- hyperdrive fuel
    fuel_stats.drive = {
        size = hyperdrive:GetMaxFuel(),                        -- Maximum fuel size for the drive
        left = hyperdrive.storedFuel,                          -- Fuel left in the drive
        free = hyperdrive:GetMaxFuel() - hyperdrive.storedFuel -- Free space in the drive tank
    }
    return fuel_stats
end

-- allow positive and negitive transfers clamp beteween 2 tanks of object shape {left, free} where left is remaining fuel and free is free space.
--- @return number amount of fuel actualy transfered
local function clampTransfer(amt, from, to)
    -- Ensure we're not transferring more than the hyperdrive holds or has
    local delta = math.clamp(amt, -to.left, to.free)

    -- Ensure we're not transferring more than the cargo bay holds
    delta = math.clamp(delta, -from.free, from.left)

    -- Return the clamped transfer amount
    return delta
end

---@param hyperdrive Equipment.HyperdriveType
function fuel.transfer_hyperfuel_hydrogen(hyperdrive, amt)
    -- for normal drive transfer from main tank
    local fuelStats = fuel.getFuelStats(hyperdrive)
    local delta = clampTransfer(amt, fuelStats.main, fuelStats.drive)
    local fuelTankSize = fuelStats.main.size
    local fuelMassLeft = fuelStats.main.left

    -- move the fuel
    if math.abs(delta) > 0.0001 then
        Game.player:SetFuelPercent((fuelMassLeft - delta) * 100 / fuelTankSize)
        hyperdrive:SetFuel(Game.player, hyperdrive.storedFuel + delta)
    end
end

---@param hyperdrive Equipment.HyperdriveType
function fuel.transfer_hyperfuel_mil(hyperdrive, amt)
    --for mil drive transfer from cargo
    local cargoMgr = Game.player:GetComponent('CargoManager')

    local fuelStats = fuel.getFuelStats(hyperdrive)

    -- Ensure we're not transferring more than the hyperdrive or cargo bay holds
    local delta = clampTransfer(amt, fuelStats.cargo, fuelStats.drive)

    print("Fuel Transfer: " .. delta)
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

        print("Drive Fuel: " .. drive.storedFuel)
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

local cargoReserve = 2
local mainReserve = 4
local comsVerbosity = 1
local function joinWithSpaces(...)
    return table.concat({ ... }, " ")
end
-- a function to dransfer fuel from the ship tank to the hyperdrive
function fuel.drawFuelTransfer(drive)
    -- Don't allow transferring fuel while the hyperdrive is doing its thing
    if Game.player:IsHyperspaceActive() then return end

    -- otherwise continue for section
    drawCentered("Hyperdrive", function()
        ui.horizontalGroup(function()
            fuel.fuelTransferButton(drive, 10)
            fuel.fuelTransferButton(drive, 1)

            ui.text(l.TRANSFER_FUEL)

            fuel.fuelTransferButton(drive, -1)
            fuel.fuelTransferButton(drive, -10)
        end)
    end)

    if fuel.hasFuelComputerCapability() then --todo maybe make it installed by default
        --lmf = {
        --    FUEL = "FUEL",
        --    EMERGENCY_RESERVE = "EMERGENCY_RESERVE",
        --    MAIN_TANK = "MAIN_TANK",
        --    CARGO_BAY = "CARGO_BAY",
        --    DRIVE_TANK = "DRIVE_TANK",
        --    OPERATIONAL_SETTINGS = "OPERATIONAL_SETTINGS",
        --    FUEL_COMPUTER = "FUEL_COMPUTER"
        --}

        local fuelStats = fuel.getFuelStats(drive)
        ui.text(joinWithSpaces(le.FUEL_COMPUTER, lmf.OPERATIONAL_SETTINGS))
        cargoReserve = ui.sliderInt(joinWithSpaces(lmf.CARGO_BAY, lmf.EMERGENCY_RESERVE), cargoReserve, 0,
            fuelStats.main.size) --todo set to cargo size
        mainReserve = ui.sliderInt(joinWithSpaces(lmf.MAIN_TANK, lmf.EMERGENCY_RESERVE), mainReserve, 0,
            fuelStats.main.size)

        print("Coms Verbosity:", comsVerbosity)
        --if ui.checkbox("Enable Coms Messages", comsVerbosity==1) then
        --    comsVerbosity = 1 else comsVerbosity = 0
        --end
        --else
        --ui.text("You have NO fuel computer installed!")
    end
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
    local options = { 1, 10, 100 }
    for _, k in ipairs(options) do
        if ui.button(tostring(k) .. "##fuel", Vector2(100, 0)) then
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
        local fuelVal = -1 * v
        if ui.button(fuelVal .. "##pump", Vector2(100, 0)) then
            fuel.pumpDown(fuelVal)
        end
        ui.sameLine()
    end
    ui.text("")
end

function fuel.hasFuelComputerCapability()
    return (Game.player["fuel_computer_cap"] or 0) > 0
end

--this code is used to sirialise and deserilize the fuel computer settings to save game.
function fuel.setComputerReserve(mainR, cargoR)
    ;
    mainReserve = mainR or 4
    cargoReserve = cargoR or 2
end

function fuel.getComputerReserve()
    ;
    return {
        mainReserve = mainReserve,
        cargoReserve = cargoReserve
    }
end

function fuel.GetUnreservedRange(fuelStats)
    local player = Game.player

    local cargoExesse = math.min(0, fuelStats.cargo.left - cargoReserve);
    local mainExesse = math.min(fuelStats.main.left - mainReserve)
    local extraMass = cargoExesse + mainExesse
    return player:GetInstalledHyperdrive():GetCustomRange(player, extraMass)
end

function fuel.GetReservedDeltaV(shipStats)
    local player = Game.player

    -- todo i dont know how to get the delta v for a custom fuel level.
end

function fuel.computerTransfer()
    local player = Game.player
    local drive = player:GetInstalledHyperdrive()

    if not drive then
        return 0, 0
    end
    local range = drive:GetRange(player);
    local rangeMax = drive:GetMaximumRange(player);

    --drive.getMaxRange(startMass, endMass)
    --if we have no fuel for out next jump
    --check if there is fuel in our tank drive above threshhold (70%)
    -- if there is not check if there is fuel in out cargo above 2t;

    local fuelStats = fuel.getFuelStats(drive)
    local cargoExesse = fuelStats.cargo.left - cargoReserve;
    local mainExesse = fuelStats.main.left - mainReserve
    -- if thes is negitive we have no reserves. maybe the computer should maintin them

    local customRange = fuel.GetUnreservedRange(fuelStats)
    --print("E: ".. E .. " Extra Mass: " ..extraMass.." Total Unreserverd Range: "..customRange);

    --joinWithSpaces(lmf.INITIATING_TRANSFER, lmf.JUMP_RANGE, "%2fly;", lmf.MAX_RANGE,"%2fly;", lmf.UNRESERVED_RANGE,"%2fly;")
    local text = string.format(
        joinWithSpaces(lmf.INITIATING_TRANSFER,
            lmf.JUMP_RANGE..":", "%.2fly;",
            lmf.MAX_RANGE..":", "%.2fly;",
            lmf.UNRESERVED_RANGE..":","%.2fly;"),
        range, rangeMax, customRange)
    Comms.Message(text, le.FUEL_COMPUTER)

    if (fuelStats.cargo.fuelType ~= Commodities.hydrogen) then
        -- mil drive then send cargo exess to drive
        if cargoExesse > 0 then
            fuel.transfer_hyperfuel_mil(drive, cargoExesse);
        else
            Comms.ImportantMessage(lmf.NO_MIL_DRIVE_FUEL, le.FUEL_COMPUTER)
        end

        -- if we have a milatary drive then we can also top up the main tank with the H left.
        local mainCargoExess = math.min(0, fuelStats.cargo.leftH - mainReserve);
        if mainCargoExess > 0 then
            print("FuelComputer: Cargo has exess H for main tank fuel: " .. mainCargoExess)
            --fuel.pumpDown(-cargoExesse);
            Game.player:Refuel(Commodities.hydrogen, mainCargoExess)
            Comms.Message(string.format(lmf.TOPPING_UP, mainCargoExess), le.FUEL_COMPUTER)
        end
    else
        --send cargo exess to thrusters and thruster exess to drive
        if cargoExesse > 0 then
            print("FuelComputer: Cargo has exess fuel: " .. cargoExesse)
            --fuel.pumpDown(-cargoExesse);
            Game.player:Refuel(Commodities.hydrogen, cargoExesse)
            Comms.Message(string.format(lmf.TOPPING_UP, cargoExesse), le.FUEL_COMPUTER)
        end

        if (mainExesse > 0) then
            fuel.transfer_hyperfuel_hydrogen(drive, mainExesse);
        else
            Comms.ImportantMessage(lmf.NO_DRIVE_FUEL, le.FUEL_COMPUTER)
        end
    end
end

return fuel
