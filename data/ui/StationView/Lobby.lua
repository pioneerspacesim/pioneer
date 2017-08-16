-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local Rand = import("Rand")
local Character = import("Character")
local Lang = import("Lang")
local Comms = import("Comms")
local Format = import("Format")
local Equipment = import("Equipment")
local ShipDef = import("ShipDef")

local InfoFace = import("ui/InfoFace")
local MessageBox = import("ui/MessageBox")
local InfoGauge = import("ui/InfoGauge")
local SmallLabeledButton = import("ui/SmallLabeledButton")

local l = Lang.GetResource("ui-core")

local ui = Engine.ui

local lobby = function (tab)
	local station = Game.player:GetDockedWith()
	local shipDef = ShipDef[Game.player.shipId]

	local rand = Rand.New(station.seed)
	local face = InfoFace.New(Character.New({ title = l.STATION_MANAGER }, rand))

	local hyperdrive = table.unpack(Game.player:GetEquip("engine")) or nil
	local hyperdrive_fuel = hyperdrive and hyperdrive.fuel or Equipment.cargo.hydrogen

	local launchButton = ui:Button(l.REQUEST_LAUNCH):SetFont("HEADING_LARGE")
	launchButton.onClick:Connect(function ()
		local crimes, fine = Game.player:GetCrimeOutstanding()

		if not Game.player:HasCorrectCrew() then
			MessageBox.Message(l.LAUNCH_PERMISSION_DENIED_CREW)
			Comms.ImportantMessage(l.LAUNCH_PERMISSION_DENIED_CREW, station.label)
		elseif fine > 0 then
			MessageBox.Message(l.LAUNCH_PERMISSION_DENIED_FINED)
			Comms.ImportantMessage(l.LAUNCH_PERMISSION_DENIED_FINED, station.label)
		elseif not Game.player:Undock() then
			MessageBox.Message(l.LAUNCH_PERMISSION_DENIED_BUSY)
			Comms.ImportantMessage(l.LAUNCH_PERMISSION_DENIED_BUSY, station.label)
		else
			Game.SwitchView()
		end
	end)

	local connections = {}

	local deltaV = ui:Label("")
	local updateDeltaV = function ()
		if Game.player:GetDockedWith() ~= station then
			connections.updateDeltaVFuel:Disconnect()
			connections.updateDeltaVCargo:Disconnect()
			return
		end
		local dv = Game.player:GetRemainingDeltaV()
		deltaV:SetText(string.format("%d km/s", dv/1000))
	end
	updateDeltaV()
	connections.updateDeltaVFuel = Game.player:Connect("fuel", updateDeltaV)
	connections.updateDeltaVCargo = Game.player:Connect("usedCargo", updateDeltaV)

	local fuelGauge = InfoGauge.New({
		label		= ui:NumberLabel("PERCENT"),
		warningLevel	= 0.1,
		criticalLevel	= 0.05,
		levelAscending	= false,
	})

	local fuelGaugeTons = ui:Label("")
	local updateFuelGaugeTons = function ()
		if Game.player:GetDockedWith() ~= station then
			connections.updateFuelGaugeTons:Disconnect()
			return
		end
		fuelGaugeTons:SetText(string.format("%.2f", shipDef.fuelTankMass * Game.player.fuel/100) .. "t")
	end
	updateFuelGaugeTons()
	connections.updateFuelGaugeTons = Game.player:Connect("fuel", updateFuelGaugeTons)

	local cargoGauge = ui:Gauge()
	cargoGauge:SetUpperValue(Game.player.totalCargo - Game.player.usedCargo + Game.player:CountEquip(hyperdrive_fuel))
	local cargoGaugeTons = ui:Label("")
	local function updateCargo ()
		if Game.player:GetDockedWith() ~= station then
			connections.updateCargo:Disconnect()
			return
		end
		local amount = Game.player:CountEquip(hyperdrive_fuel)
		cargoGauge:SetValue(amount)
		cargoGaugeTons:SetText(amount .. "t")
	end
	updateCargo()
	connections.updateCargo = Game.player:Connect("usedCargo", updateCargo)

	local hyperspaceRange = ui:Label("")
	local updateHyperspaceRange = function ()
		if Game.player:GetDockedWith() ~= station then
			connections.updateHyperspaceRangeFuel:Disconnect()
			connections.updateHyperspaceRangeCargo:Disconnect()
			return
		end
		hyperspaceRange:SetText(string.format("%.1f ", Game.player.hyperspaceRange) .. l.LY)
	end
	updateHyperspaceRange()
	connections.updateHyperspaceRangeFuel = Game.player:Connect("fuel", updateHyperspaceRange)
	connections.updateHyperspaceRangeCargo = Game.player:Connect("usedCargo", updateHyperspaceRange)

	local totalFuel = ui:Label("")
	local updateTotalFuel = function ()
		if Game.player:GetDockedWith() ~= station then
			connections.updateTotalFuel:Disconnect()
			return
		end
		totalFuel:SetText(Format.Money(station:GetEquipmentPrice(Equipment.cargo.hydrogen) * shipDef.fuelTankMass/100 * Game.player.fuel))
	end
	updateTotalFuel()
	connections.updateTotalFuel = Game.player:Connect("fuel", updateTotalFuel)

	local totalHyperdriveFuel = ui:Label("")
	local updateTotalHyperdriveFuel = function ()
		if Game.player:GetDockedWith() ~= station then
			connections.updateTotalHyperdriveFuel:Disconnect()
			return
		end
		totalHyperdriveFuel:SetText(Format.Money(station:GetEquipmentPrice(hyperdrive_fuel) * Game.player:CountEquip(hyperdrive_fuel)))
	end
	updateTotalHyperdriveFuel()
	connections.updateTotalHyperdriveFuel = Game.player:Connect("usedCargo", updateTotalHyperdriveFuel)

	fuelGauge.label:Bind("valuePercent", Game.player, "fuel")
	fuelGauge.gauge:Bind("valuePercent", Game.player, "fuel")

	local refuelInternalTank = function (delta)
		local stock = station:GetEquipmentStock(Equipment.cargo.hydrogen)
		local price = station:GetEquipmentPrice(Equipment.cargo.hydrogen)
		local fuel = Game.player.fuel + delta
		local mass = shipDef.fuelTankMass/100 * delta
		local total = price * mass

		if delta > 0 and stock == 0 then MessageBox.Message(l.ITEM_IS_OUT_OF_STOCK) return end

		if total > Game.player:GetMoney() then
			total = Game.player:GetMoney()
			mass = total / price
			fuel = Game.player.fuel + mass * 100 / shipDef.fuelTankMass
		end

		if stock < mass then
			mass = stock
			total = price * mass
			fuel = Game.player.fuel + mass * 100 / shipDef.fuelTankMass
		end

		Game.player:AddMoney(-total)
		station:AddEquipmentStock(Equipment.cargo.hydrogen, -math.ceil(mass))
		Game.player:SetFuelPercent(fuel)
	end

	local refuelHyperdrive = function (mass)
		local stock = station:GetEquipmentStock(hyperdrive_fuel)
		local price = station:GetEquipmentPrice(hyperdrive_fuel)
		local total = price * mass

		if mass > 0 and stock == 0 then MessageBox.Message(l.ITEM_IS_OUT_OF_STOCK) return end

		if total > Game.player:GetMoney() then
			mass = math.floor(Game.player:GetMoney() / price)
			total = price * mass
		end

		if stock < mass then
			mass = stock
			total = price * mass
		end

		Game.player:AddMoney(-total)
		if mass < 0 then
			Game.player:RemoveEquip(hyperdrive_fuel, math.abs(mass), "cargo")
		else
			Game.player:AddEquip(hyperdrive_fuel, mass, "cargo")
		end
		station:AddEquipmentStock(hyperdrive_fuel, -mass)
	end

	local refuelFullButton = ui:Button(l.REFUEL_FULL):SetFont("XSMALL")
	refuelFullButton.onClick:Connect(function () refuelInternalTank(100 - Game.player.fuel) end)

	local add10Button = ui:Button("+10%"):SetFont("XSMALL")
	add10Button.onClick:Connect(function () refuelInternalTank(math.clamp(10, 0, 100 - Game.player.fuel)) end)

	local sub10Button = ui:Button("-10%"):SetFont("XSMALL")
	sub10Button.onClick:Connect(function () refuelInternalTank(math.clamp(-10, -Game.player.fuel, 0)) end)

	local addOneButton = ui:Button("+1t"):SetFont("XSMALL")
	addOneButton.onClick:Connect(function () refuelHyperdrive(math.clamp(1, 0, Game.player.totalCargo - Game.player.usedCargo)) end)

	local addTenButton = ui:Button("+10t"):SetFont("XSMALL")
	addTenButton.onClick:Connect(function () refuelHyperdrive(math.clamp(10, 0, Game.player.totalCargo - Game.player.usedCargo)) end)

	local subOneButton = ui:Button("-1t"):SetFont("XSMALL")
	subOneButton.onClick:Connect(function () refuelHyperdrive(math.clamp(-1, -Game.player:CountEquip(hyperdrive_fuel), 0)) end)

	local subTenButton = ui:Button("-10t"):SetFont("XSMALL")
	subTenButton.onClick:Connect(function () refuelHyperdrive(math.clamp(-10, -Game.player:CountEquip(hyperdrive_fuel), 0)) end)

	local tech_certified

	if station.techLevel == 11 then
		tech_certified = l.TECH_CERTIFIED_MILITARY
	else
		tech_certified = string.interp(l.TECH_CERTIFIED, { tech_level = station.techLevel})
	end

	local orbit_period = station.path:GetSystemBody().orbitPeriod

	local station_orbit_info = ""
	if station.type == "STARPORT_ORBITAL" then
		station_orbit_info =
			string.interp(l.STATION_ORBIT, { orbit_period = string.format("%.2f", orbit_period),
											 parent_body = station.path:GetSystemBody().parent.name})
	end

	return
		ui:Grid({60,1,39},1)
			:SetColumn(0, {
				ui:VBox(10):PackEnd({
					ui:Label(station.label):SetFont("HEADING_LARGE"),
					ui:Align("LEFT", tech_certified),
					ui:Align("LEFT", station_orbit_info),
					ui:Expand(),
					ui:VBox(10):PackEnd({
						ui:HBox(10):PackEnd({
							ui:Image("icons/goods/Hydrogen.png", { "PRESERVE_ASPECT" }),
							ui:VBox():PackEnd({
								ui:Label(Format.Money(station:GetEquipmentPrice(Equipment.cargo.hydrogen)) .. "/t"):SetFont("XSMALL"),
								totalFuel:SetFont("XSMALL"),
							}),
							refuelFullButton,
							sub10Button,
							add10Button,
							ui:VBox():PackEnd({
								fuelGauge,
								ui:HBox(10):PackEnd({
									ui:Label(l.DELTA_V .. ":"):SetFont("SMALL"),
									deltaV:SetFont("SMALL"),
									ui:Expand("HORIZONTAL", ui:Align("RIGHT", fuelGaugeTons:SetFont("SMALL"))),
								}),
							}),
						}),
						ui:HBox(10):PackEnd({
							ui:Image("icons/goods/" .. hyperdrive_fuel.icon_name .. ".png", { "PRESERVE_ASPECT" }),
							ui:VBox():PackEnd({
								ui:Label(Format.Money(station:GetEquipmentPrice(hyperdrive_fuel)) .. "/t"):SetFont("XSMALL"),
								totalHyperdriveFuel:SetFont("XSMALL"),
							}),
							subTenButton,
							subOneButton,
							addOneButton,
							addTenButton,
							ui:VBox():PackEnd({
								ui:HBox(10):PackEnd({
									cargoGauge,
									cargoGaugeTons,
								}),
								ui:HBox(10):PackEnd({
									ui:Label(l.HYPERSPACE_RANGE .. ":"):SetFont("SMALL"),
									hyperspaceRange:SetFont("SMALL"),
								}),
							}),
						}),
					}),
					ui:Align("MIDDLE", launchButton),
				})
			})
			:SetColumn(2, {
				face.widget
			})
end

return lobby
