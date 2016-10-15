-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
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

	local rand = Rand.New(station.seed)
	local face = InfoFace.New(Character.New({ title = l.STATION_MANAGER }, rand))

	local launchButton = ui:Button(ui:Expand("HORIZONTAL", ui:Align("MIDDLE", l.REQUEST_LAUNCH))):SetFont("HEADING_LARGE")
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

	local fuelGauge = InfoGauge.New({
		label		= ui:NumberLabel("PERCENT"),
		warningLevel	= 0.1,
		criticalLevel	= 0.05,
		levelAscending	= false,
	})
	fuelGauge.label:Bind("valuePercent", Game.player, "fuel")
	fuelGauge.gauge:Bind("valuePercent", Game.player, "fuel")

	local fuelSlider = ui:HSlider():SetRange(0, 100)
	local fuelSliderLabel = ui:Label(string.format("%.2f", Game.player.fuel) .. "%")
	local totalLabel = ui:Label("0")
	fuelSlider:SetValue(Game.player.fuel)
	fuelSlider.onValueChanged:Connect(function (fuel)
		local total = Game.player:GetDockedWith():GetEquipmentPrice(Equipment.cargo.hydrogen) * ShipDef[Game.player.shipId].fuelTankMass/100 * (fuel - Game.player.fuel)
		fuelSliderLabel:SetText(string.format("%.2f", fuel) .. "%")
		totalLabel:SetText(string.format("%.2f", total))
	end)

	local applyButton = SmallLabeledButton.New("Apply")
	applyButton.button.onClick:Connect(function ()
		local stock = Game.player:GetDockedWith():GetEquipmentStock(Equipment.cargo.hydrogen)
		local price = Game.player:GetDockedWith():GetEquipmentPrice(Equipment.cargo.hydrogen)
		local fuel = fuelSlider:GetValue()
		local mass = ShipDef[Game.player.shipId].fuelTankMass/100 * (fuel - Game.player.fuel)
		local total = price * mass

		if total > Game.player:GetMoney() then
			total = Game.player:GetMoney()
			mass = total / price
			fuel = Game.player.fuel + mass * 100 / ShipDef[Game.player.shipId].fuelTankMass
			fuelSlider:SetValue(fuel)
		end

		if stock < mass then
			mass = stock
			total = price * mass
			fuel = Game.player.fuel + mass * 100 / ShipDef[Game.player.shipId].fuelTankMass
			fuelSlider:SetValue(fuel)
		end

		Game.player:AddMoney(-total)
		Game.player:GetDockedWith():AddEquipmentStock(Equipment.cargo.hydrogen, -math.ceil(mass))
		Game.player:SetFuelPercent(fuel)
	end)

	local hangupButton = SmallLabeledButton.New(l.HANG_UP)
	hangupButton.button.onClick:Connect(function () ui:DropLayer() end)

	local refuelButton = ui:Button(ui:Expand("HORIZONTAL", ui:Align("MIDDLE", l.REFUEL))):SetFont("HEADING_LARGE")
	refuelButton.onClick:Connect(function ()
		if Game.player:GetDockedWith():GetEquipmentStock(Equipment.cargo.hydrogen) ~= 0 then
			ui:NewLayer(
				ui:ColorBackground(0,0,0,0.5,
					ui:Grid({25,50,25},{20,50,30}):SetCell(1,1,
						ui:Background(
							ui:VBox(10):PackEnd({
								ui:MultiLineText("Hello Commander,\nwe have xt hydrogen in our stock for a price of x/t.\nHow can I serve you?"),
								ui:Margin(10),
								fuelGauge,
								ui:HBox(10):PackEnd({ fuelSlider, fuelSliderLabel }),
								ui:HBox(10):PackEnd({ ui:Label("Total:"), totalLabel }),
								applyButton,
								ui:Margin(10),
								hangupButton,
							})
						)
					)
				)
			)
		else
			MessageBox.Message(l.ITEM_IS_OUT_OF_STOCK)
		end
	end)

	local tech_certified

	if station.techLevel == 11 then
		tech_certified = l.TECH_CERTIFIED_MILITARY
	else
		tech_certified = string.interp(l.TECH_CERTIFIED, { tech_level = station.techLevel})
	end

	return
		ui:Grid({48,4,48},1)
			:SetColumn(0, {
				ui:VBox(10):PackEnd({
					ui:Label(station.label):SetFont("HEADING_LARGE"),
					ui:Align("LEFT", tech_certified),
					ui:Expand(),
					ui:Grid({15,70,15},1)
						:SetColumn(1, {
							ui:VBox(10):PackEnd({
								refuelButton,
								launchButton,
							})
						})
				})
			})
			:SetColumn(2, {
				face.widget
			})
end

return lobby
