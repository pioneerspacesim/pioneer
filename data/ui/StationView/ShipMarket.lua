-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local SpaceStation = import("SpaceStation")
local Game = import("Game")
local Event = import("Event")
local Format = import("Format")
local Lang = import("Lang")
local Comms = import("Comms")
local ShipDef = import("ShipDef")

local ModelSpinner = import("UI.Game.ModelSpinner")
local ModelSkin = import("SceneGraph.ModelSkin")

local ui = Engine.ui

local l = Lang.GetResource("ui-core")

-- XXX equipment strings are in core. this sucks
local lcore = Lang.GetResource("core")

local shipClassString = {
	light_cargo_shuttle        = l.LIGHT_CARGO_SHUTTLE,
	light_courier              = l.LIGHT_COURIER,
	light_fighter              = l.LIGHT_FIGHTER,
	light_freighter            = l.LIGHT_FREIGHTER,
	light_passenger_shuttle    = l.LIGHT_PASSENGER_SHUTTLE,
	light_passenger_transport  = l.LIGHT_PASSENGER_TRANSPORT,
	medium_cargo_shuttle       = l.MEDIUM_CARGO_SHUTTLE,
	medium_courier             = l.MEDIUM_COURIER,
	medium_fighter             = l.MEDIUM_FIGHTER,
	medium_freighter           = l.MEDIUM_FREIGHTER,
	medium_passenger_shuttle   = l.MEDIUM_PASSENGER_SHUTTLE,
	medium_passenger_transport = l.MEDIUM_PASSENGER_TRANSPORT,
	heavy_cargo_shuttle        = l.HEAVY_CARGO_SHUTTLE,
	heavy_courier              = l.HEAVY_COURIER,
	heavy_fighter              = l.HEAVY_FIGHTER,
	heavy_freighter            = l.HEAVY_FREIGHTER,
	heavy_passenger_shuttle    = l.HEAVY_PASSENGER_SHUTTLE,
	heavy_passenger_transport  = l.HEAVY_PASSENGER_TRANSPORT,

	unknown                    = "",
}

local shipTable =
	ui:Table()
		:SetRowSpacing(5)
		:SetColumnSpacing(10)
		:SetHeadingRow({'', l.SHIP, l.PRICE, l.CAPACITY})
		:SetHeadingFont("LARGE")
		:SetRowAlignment("CENTER")
		:SetMouseEnabled(true)

local shipInfo =
	ui:Expand("VERTICAL")

local function shipClassIcon (shipClass)
	return shipClass ~= "unknown"
		and ui:Image("icons/shipclass/"..shipClass..".png", { "PRESERVE_ASPECT" })
		or ui:Margin(32)
end

local function manufacturerIcon (manufacturer)
	return manufacturer ~= "unknown"
		and ui:Image("icons/manufacturer/"..manufacturer..".png", { "PRESERVE_ASPECT" })
		or ui:Margin(32)
end

local function tradeInValue (def)
	return def.basePrice * 0.5
end

local function buyShip (sos)
	local player = Game.player
	local station = player:GetDockedWith()
	local def = sos.def

	local cost = def.basePrice - tradeInValue(ShipDef[Game.player.shipId])
	if player:GetMoney() < cost then
		Comms.Message(l.YOU_NOT_ENOUGH_MONEY)
		return
	end
	player:AddMoney(-cost)

	station:ReplaceShipOnSale(sos, {
		def   = ShipDef[player.shipId],
		skin  = player:GetSkin(),
		label = player.label,
	})

	player:SetShipType(def.id)
	player:SetSkin(sos.skin)
	player:SetLabel(sos.label)
	player:AddEquip(def.defaultHyperdrive)
	player:SetFuelPercent(100)

	shipInfo:SetInnerWidget(
		ui:MultiLineText(l.THANKS_AND_REMEMBER_TO_BUY_FUEL)
	)

end

local currentShipOnSale

shipTable.onRowClicked:Connect(function (row)
	local station = Game.player:GetDockedWith()
	currentShipOnSale = station:GetShipsOnSale()[row+1]
	local def = currentShipOnSale.def

	local forwardAccelEmpty =  def.linearThrust.FORWARD / (-9.81*1000*(def.hullMass+def.fuelTankMass))
	local forwardAccelFull  =  def.linearThrust.FORWARD / (-9.81*1000*(def.hullMass+def.capacity+def.fuelTankMass))
	local reverseAccelEmpty = -def.linearThrust.REVERSE / (-9.81*1000*(def.hullMass+def.fuelTankMass))
	local reverseAccelFull  = -def.linearThrust.REVERSE / (-9.81*1000*(def.hullMass+def.capacity+def.fuelTankMass))
	local deltav = def.effectiveExhaustVelocity * math.log((def.hullMass + def.fuelTankMass) / def.hullMass)
	local deltav_f = def.effectiveExhaustVelocity * math.log((def.hullMass + def.fuelTankMass + def.capacity) / (def.hullMass + def.capacity))
	local deltav_m = def.effectiveExhaustVelocity * math.log((def.hullMass + def.fuelTankMass + def.capacity) / def.hullMass)

	local buyButton = ui:Button(l.BUY_SHIP):SetFont("HEADING_LARGE")
	buyButton.onClick:Connect(function () buyShip(currentShipOnSale) end)

	shipInfo:SetInnerWidget(
		ui:VBox():PackEnd({
			ui:HBox():PackEnd({
				ui:Align("LEFT",
					ui:VBox():PackEnd({
						ui:Label(def.name):SetFont("HEADING_LARGE"),
						ui:Label(shipClassString[def.shipClass]):SetFont("HEADING_SMALL"),
					})
				),
				ui:Expand("HORIZONTAL", ui:Align("RIGHT", manufacturerIcon(def.manufacturer))),
			}),
			ui:Grid(2,1):SetRow(0, {
				l.PRICE..": "..Format.Money(def.basePrice),
                l.AFTER_TRADE_IN..": "..Format.Money(def.basePrice - tradeInValue(ShipDef[Game.player.shipId])),
			}),
			ModelSpinner.New(ui, def.modelName, currentShipOnSale.skin),
			ui:Label(l.HYPERDRIVE_FITTED.." "..lcore[def.defaultHyperdrive]):SetFont("SMALL"),
			ui:Margin(10, "VERTICAL",
				ui:Grid(2,1)
					:SetFont("SMALL")
					:SetRow(0, {
						ui:Table()
							:SetColumnSpacing(5)
							:AddRow({l.FORWARD_ACCEL_EMPTY, Format.AccelG(forwardAccelEmpty)})
							:AddRow({l.FORWARD_ACCEL_FULL,  Format.AccelG(forwardAccelFull)})
							:AddRow({l.REVERSE_ACCEL_EMPTY, Format.AccelG(reverseAccelEmpty)})
							:AddRow({l.REVERSE_ACCEL_FULL,  Format.AccelG(reverseAccelFull)})
							:AddRow({l.DELTA_V_EMPTY, string.format("%d km/s", deltav / 1000)})
							:AddRow({l.DELTA_V_FULL, string.format("%d km/s", deltav_f / 1000)}),							
						ui:Table()
							:SetColumnSpacing(5)
							:AddRow({l.WEIGHT_EMPTY,        Format.MassTonnes(def.hullMass)})
							:AddRow({l.CAPACITY,            Format.MassTonnes(def.capacity)})
							:AddRow({l.WEIGHT_FULLY_LOADED, Format.MassTonnes(def.hullMass+def.capacity+def.fuelTankMass)})
							:AddRow({l.FUEL_WEIGHT,         Format.MassTonnes(def.fuelTankMass)})
							:AddRow({l.DELTA_V_MAX, string.format("%d km/s", deltav_m / 1000)})
					})
			),
			ui:Align("MIDDLE", buyButton),
		})
	)
end)

local function updateStation (station, shipsOnSale)
	if station ~= Game.player:GetDockedWith() then return end

	shipTable:ClearRows()

	local seen = false

	for i = 1,#shipsOnSale do
		local sos = shipsOnSale[i]
		if sos == currentShipOnSale then
			seen = true
		end
		local def = sos.def
		shipTable:AddRow({shipClassIcon(def.shipClass), def.name, Format.Money(def.basePrice), def.capacity.."t"})
	end

	if currentShipOnSale then
		if not seen then
			currentShipOnSale = nil
			shipInfo:SetInnerWidget(
				ui:MultiLineText(l.SHIP_VIEWING_WAS_SOLD)
			)
		end
	else
		shipInfo:RemoveInnerWidget()
	end
end

Event.Register("onShipMarketUpdate", updateStation)

local shipMarket = function (args)
	local station = Game.player:GetDockedWith()
	currentShipOnSale = nil
	updateStation(station, station:GetShipsOnSale())

	return
		ui:Grid({48,4,48},1)
			:SetColumn(0, {shipTable})
			:SetColumn(2, {shipInfo})
end

return shipMarket
