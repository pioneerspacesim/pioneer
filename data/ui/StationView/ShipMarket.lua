-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
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

local shipTable =
	ui:Table()
		:SetRowSpacing(5)
		:SetColumnSpacing(10)
		:SetHeadingRow({"Ship","Price","Capacity"})
		:SetHeadingFont("LARGE")
		:SetMouseEnabled(true)

local shipInfo =
	ui:Expand("VERTICAL")

local function icon (manufacturer)
	return manufacturer ~= ""
		and ui:Image("icons/manufacturer/"..manufacturer..".png", { "PRESERVE_ASPECT" })
		or ui:Margin(32)
end

local function tradeInValue (def)
	return def.basePrice * 0.5
end

local function buyShip (num)
	local player = Game.player
	local station = player:GetDockedWith()
	local sos = station:GetShipsOnSale()[num]
	local def = sos.def

	local cost = def.basePrice - tradeInValue(ShipDef[Game.player.shipId])
	if player:GetMoney() < cost then
		Comms.Message(l.YOU_NOT_ENOUGH_MONEY)
		return
	end
	player:AddMoney(-cost)

	station:ReplaceShipOnSale(num, {
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

shipTable.onRowClicked:Connect(function (row)
	local station = Game.player:GetDockedWith()
	local sos = station:GetShipsOnSale()[row+1]
	local def = sos.def

	local forwardAccelEmpty =  def.linearThrust.FORWARD / (-9.81*1000*(def.hullMass+def.fuelTankMass))
	local forwardAccelFull  =  def.linearThrust.FORWARD / (-9.81*1000*(def.hullMass+def.capacity+def.fuelTankMass))
	local reverseAccelEmpty = -def.linearThrust.REVERSE / (-9.81*1000*(def.hullMass+def.fuelTankMass))
	local reverseAccelFull  = -def.linearThrust.REVERSE / (-9.81*1000*(def.hullMass+def.capacity+def.fuelTankMass))

	local buyButton = ui:Button("Buy Ship"):SetFont("HEADING_LARGE")
	buyButton.onClick:Connect(function () buyShip(row+1) end)

	shipInfo:SetInnerWidget(
		ui:VBox():PackEnd({
			ui:HBox():PackEnd({
				ui:Align("LEFT", ui:Label(def.name):SetFont("HEADING_LARGE")),
				ui:Expand("HORIZONTAL", ui:Align("RIGHT", icon(def.manufacturer))),
			}),
			ui:Grid(2,1):SetRow(0, {
				"Price: "..Format.Money(def.basePrice),
				"After trade-in: "..Format.Money(def.basePrice - tradeInValue(ShipDef[Game.player.shipId])),
			}),
			ModelSpinner.New(ui, def.modelName, sos.skin),
			ui:Label("Hyperdrive fitted: "..lcore[def.defaultHyperdrive]):SetFont("SMALL"),
			ui:Margin(10, "VERTICAL",
				ui:Grid(2,1)
					:SetFont("SMALL")
					:SetRow(0, {
						ui:Table()
							:SetColumnSpacing(5)
							:AddRow({"Forward accel (empty)", Format.AccelG(forwardAccelEmpty)})
							:AddRow({"Forward accel (full)",  Format.AccelG(forwardAccelFull)})
							:AddRow({"Reverse accel (empty)", Format.AccelG(reverseAccelEmpty)})
							:AddRow({"Reverse accel (full)",  Format.AccelG(reverseAccelFull)}),
						ui:Table()
							:SetColumnSpacing(5)
							:AddRow({"Weight empty",        Format.MassTonnes(def.hullMass)})
							:AddRow({"Capacity",            Format.MassTonnes(def.capacity)})
							:AddRow({"Fuel weight",         Format.MassTonnes(def.fuelTankMass)})
							:AddRow({"Weight fully loaded", Format.MassTonnes(def.hullMass+def.capacity+def.fuelTankMass)})
					})
			),
			ui:Align("MIDDLE", buyButton),
		})
	)
end)

local function updateStation (station, shipsOnSale)
	if station ~= Game.player:GetDockedWith() then return end

	shipTable:ClearRows()

	for i = 1,#shipsOnSale do
		local def = shipsOnSale[i].def
		shipTable:AddRow({def.name, Format.Money(def.basePrice), def.capacity.."t"})
	end
end

Event.Register("onShipMarketUpdate", updateStation)

local shipMarket = function (args)
	local station = Game.player:GetDockedWith()
	updateStation(station, station:GetShipsOnSale())

	return
		ui:Grid(2,1)
			:SetColumn(0, {shipTable})
			:SetColumn(1, {shipInfo})
end

return shipMarket
