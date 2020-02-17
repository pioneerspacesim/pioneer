-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = import 'Lang'
local Game = import 'Game'
local Format = import 'Format'
local ShipDef = import 'ShipDef'
local Equipment = import 'Equipment'
local StationView = import 'pigui/views/station-view'
local Table = import 'pigui/libs/table.lua'
local PiImage = import 'ui/PiImage'
local ModelSpinner = import 'PiGui.Modules.ModelSpinner'

local ui = import 'pigui/pigui.lua'
local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local l = Lang.GetResource("ui-core")
local colors = ui.theme.colors

local vZero = Vector2(0,0)
local rescaleVector = ui.rescaleUI(Vector2(1, 1), Vector2(1600, 900), true)
local widgetSizes = ui.rescaleUI({
	iconSize = Vector2(64, 64),
	buyButton = Vector2(96, 36),
	confirmButtonSize = Vector2(384, 48),
	itemSpacing = Vector2(18, 4),
}, Vector2(1600, 900))
widgetSizes.iconSpacer = (widgetSizes.buyButton - widgetSizes.iconSize)/2

local shipMarket
local icons = {}
local manufacturerIcons = {}
local selectedItem

local currentIconSize = Vector2(0,0)

local shipSellPriceReduction = 0.5
local equipSellPriceReduction = 0.8

local modelSpinner = ModelSpinner()
local cachedShip = nil
local cachedPattern = nil

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

local function refreshShipMarket()
	widgetSizes.rowVerticalSpacing = Vector2(0, (widgetSizes.iconSize.y + widgetSizes.itemSpacing.y - pionillium.large.size)/2)

	local station = Game.player:GetDockedWith()
	shipMarket.items = station:GetShipsOnSale()
	selectedItem = nil
end

local function manufacturerIcon (manufacturer)
	if(manufacturer ~= "unknown" and manufacturer ~= nil) then
		if manufacturerIcons[manufacturer] == nil then
			manufacturerIcons[manufacturer] = PiImage.New("icons/manufacturer/".. manufacturer ..".png")
		end
		manufacturerIcons[manufacturer]:Draw(widgetSizes.iconSize)
	end
end


local tradeInValue = function(shipDef)
	local value = shipDef.basePrice * shipSellPriceReduction * Game.player.hullPercent/100

	if shipDef.hyperdriveClass > 0 then
		value = value - Equipment.hyperspace["hyperdrive_" .. shipDef.hyperdriveClass].price * equipSellPriceReduction
	end

	for _, t in pairs({Equipment.misc, Equipment.hyperspace, Equipment.laser}) do
		for _, e in pairs(t) do
			local n = Game.player:CountEquip(e)
			value = value + n * e.price * equipSellPriceReduction
		end
	end

	return math.ceil(value)
end

local function buyShip (mkt, sos)
	local player = Game.player
	local station = player:GetDockedWith()
	local def = sos.def

	local cost = def.basePrice - tradeInValue(ShipDef[Game.player.shipId])
	if math.floor(cost) ~= cost then
		error("Ship price non-integer value.")
	end
	if player:GetMoney() < cost then
		mkt.popup.msg = l.YOU_NOT_ENOUGH_MONEY
		mkt.popup:open()
		return
	end

	if player:CrewNumber() > def.maxCrew then
		mkt.popup.msg = l.TOO_SMALL_FOR_CURRENT_CREW
		mkt.popup:open()
		return
	end

	local hdrive = def.hyperdriveClass > 0 and Equipment.hyperspace["hyperdrive_" .. def.hyperdriveClass].capabilities.mass or 0
	if def.equipSlotCapacity.cargo < player.usedCargo or def.capacity < (player.usedCargo + hdrive) then
		mkt.popup.msg = l.TOO_SMALL_TO_TRANSSHIP
		mkt.popup:open()
		return
	end

	local manifest = player:GetEquip("cargo")
	player:AddMoney(-cost)

	station:ReplaceShipOnSale(sos, {
		def     = ShipDef[player.shipId],
		skin    = player:GetSkin(),
		pattern = player.model.pattern,
		label   = player.label,
	})

	player:SetShipType(def.id)
	player:SetSkin(sos.skin)
	if sos.pattern then player.model:SetPattern(sos.pattern) end
	player:SetLabel(sos.label)
	if def.hyperdriveClass > 0 then
		player:AddEquip(Equipment.hyperspace["hyperdrive_" .. def.hyperdriveClass])
	end
	for _, e in pairs(manifest) do
		player:AddEquip(e)
	end
	player:SetFuelPercent(100)
	refreshShipMarket();

	mkt.popup.msg = l.THANKS_AND_REMEMBER_TO_BUY_FUEL
	mkt.popup:open()
end

local yes_no = function (binary)
	if binary == 1 then
		return l.YES
	elseif binary == 0 then
		return l.NO
	else error("argument to yes_no not 0 or 1")
	end
end

local tradeMenu = function()
	if(selectedItem) then
		ui.withStyleVars({ WindowPadding = shipMarket.style.windowPadding, ItemSpacing = shipMarket.style.itemSpacing}, function()
			ui.child("TradeMenu", Vector2(0,0), {"AlwaysUseWindowPadding"}, function()
				local colHeadingWidth = ui.getContentRegion().x - widgetSizes.buyButton.x
				local def = selectedItem.def

				ui.columns(2, "shipMarketInfo")
				ui.setColumnWidth(0, colHeadingWidth)

				ui.withFont(orbiteer.xlarge.name, orbiteer.xlarge.size, function()
					ui.text(selectedItem.def.name)
				end)
				ui.withFont(orbiteer.medlarge.name, orbiteer.medlarge.size, function()
					ui.text(shipClassString[selectedItem.def.shipClass])
				end)

				ui.text(l.PRICE..": "..Format.Money(selectedItem.def.basePrice, false))
				ui.sameLine()
				ui.text(l.AFTER_TRADE_IN..": "..Format.Money(selectedItem.def.basePrice - tradeInValue(ShipDef[Game.player.shipId]), false))

				ui.nextColumn()
				ui.dummy(widgetSizes.iconSpacer)
				ui.sameLine()
				manufacturerIcon(selectedItem.def.manufacturer)
				local shipBought = false
				ui.withFont(pionillium.medlarge.name, orbiteer.medlarge.size, function()
					shipBought = ui.coloredSelectedButton(l.BUY_SHIP, widgetSizes.buyButton, false, colors.buttonBlue, nil, true)
				end)
				ui.columns(1, "")

				if shipBought then
					buyShip(shipMarket, selectedItem)
					return
				end

				local spinnerWidth = ui.getContentRegion().x
				modelSpinner:setSize(Vector2(spinnerWidth, spinnerWidth / 2.5))
				if selectedItem.def.modelName ~= cachedShip then
					cachedShip = selectedItem.def.modelName
					cachedPattern = selectedItem.pattern
					modelSpinner:setModel(cachedShip, selectedItem.skin, cachedPattern)
				end

				modelSpinner:draw()

				local hyperdrive_str = selectedItem.def.hyperdriveClass > 0 and
						Equipment.hyperspace["hyperdrive_" .. selectedItem.def.hyperdriveClass]:GetName() or l.NONE

				local forwardAccelEmpty =  def.linearThrust.FORWARD / (-9.81*1000*(def.hullMass+def.fuelTankMass))
				local forwardAccelFull  =  def.linearThrust.FORWARD / (-9.81*1000*(def.hullMass+def.capacity+def.fuelTankMass))
				local reverseAccelEmpty = -def.linearThrust.REVERSE / (-9.81*1000*(def.hullMass+def.fuelTankMass))
				local reverseAccelFull  = -def.linearThrust.REVERSE / (-9.81*1000*(def.hullMass+def.capacity+def.fuelTankMass))
				local deltav = def.effectiveExhaustVelocity * math.log((def.hullMass + def.fuelTankMass) / def.hullMass)
				local deltav_f = def.effectiveExhaustVelocity * math.log((def.hullMass + def.fuelTankMass + def.capacity) / (def.hullMass + def.capacity))
				local deltav_m = def.effectiveExhaustVelocity * math.log((def.hullMass + def.fuelTankMass + def.capacity) / def.hullMass)

				ui.withFont(pionillium.medlarge.name, pionillium.medlarge.size, function()
					ui.child("ShipSpecs", ui.getContentRegion() - widgetSizes.itemSpacing, {"AlwaysUseWindowPadding"}, function()
						local colSpecNameWidth = ui.getContentRegion().x / 3.5
						local colSpecValWidth = (ui.getContentRegion().x - colSpecNameWidth*2) / 2

						ui.columns(4, "ShipSpecsTable")
						ui.setColumnWidth(0, colSpecNameWidth)
						ui.setColumnWidth(1, colSpecValWidth)
						ui.setColumnWidth(2, colSpecNameWidth)
						ui.setColumnWidth(3, colSpecValWidth)

						ui.text(l.HYPERDRIVE_FITTED)
						ui.nextColumn()
						ui.text(hyperdrive_str)
						ui.nextColumn()
						ui.nextColumn()
						ui.nextColumn()

						ui.text(l.WEIGHT_FULLY_LOADED)
						ui.nextColumn()
						ui.text(Format.MassTonnes(def.hullMass+def.capacity+def.fuelTankMass))
						ui.nextColumn()
						ui.text(l.WEIGHT_EMPTY)
						ui.nextColumn()
						ui.text(Format.MassTonnes(def.hullMass))
						ui.nextColumn()

						ui.text(l.FORWARD_ACCEL_FULL)
						ui.nextColumn()
						ui.text(Format.AccelG(forwardAccelFull))
						ui.nextColumn()
						ui.text(l.CAPACITY)
						ui.nextColumn()
						ui.text(Format.MassTonnes(def.capacity))
						ui.nextColumn()

						ui.text(l.FORWARD_ACCEL_EMPTY)
						ui.nextColumn()
						ui.text(Format.AccelG(forwardAccelEmpty))
						ui.nextColumn()
						ui.text(l.FUEL_WEIGHT)
						ui.nextColumn()
						ui.text(Format.MassTonnes(def.fuelTankMass))
						ui.nextColumn()

						ui.text(l.REVERSE_ACCEL_EMPTY)
						ui.nextColumn()
						ui.text(Format.AccelG(reverseAccelEmpty))
						ui.nextColumn()
						ui.text(l.MINIMUM_CREW)
						ui.nextColumn()
						ui.text(def.minCrew)
						ui.nextColumn()

						ui.text(l.REVERSE_ACCEL_FULL)
						ui.nextColumn()
						ui.text(Format.AccelG(reverseAccelFull))
						ui.nextColumn()
						ui.text(l.MAXIMUM_CREW)
						ui.nextColumn()
						ui.text(def.maxCrew)
						ui.nextColumn()

						ui.text(l.DELTA_V_EMPTY)
						ui.nextColumn()
						ui.text(string.format("%d km/s", deltav / 1000))
						ui.nextColumn()
						ui.text(l.MISSILE_MOUNTS)
						ui.nextColumn()
						ui.text(def.equipSlotCapacity["missile"])
						ui.nextColumn()

						ui.text(l.DELTA_V_FULL)
						ui.nextColumn()
						ui.text(string.format("%d km/s", deltav_f / 1000))
						ui.nextColumn()
						ui.text(l.ATMOSPHERIC_SHIELDING)
						ui.nextColumn()
						ui.text(yes_no(def.equipSlotCapacity["atmo_shield"]))
						ui.nextColumn()

						ui.text(l.DELTA_V_MAX)
						ui.nextColumn()
						ui.text(string.format("%d km/s", deltav_m / 1000))
						ui.nextColumn()
						ui.text(l.SCOOP_MOUNTS)
						ui.nextColumn()
						ui.text(def.equipSlotCapacity["scoop"])
						ui.nextColumn()

						ui.columns(1, "")
					end)

				end)
			end)
		end)
	end
end

shipMarket = Table.New("shipMarketWidget", false, {
	columnCount = 4,
	initTable = function(self)
		local iconColumnWidth = widgetSizes.iconSize.x + widgetSizes.itemSpacing.x
		local columnWidth = (self.style.size.x - iconColumnWidth) / (self.columnCount-1)
		ui.setColumnWidth(0, widgetSizes.iconSize.x + widgetSizes.itemSpacing.x)
		ui.setColumnWidth(1, columnWidth)
		ui.setColumnWidth(2, columnWidth)
		ui.setColumnWidth(3, columnWidth)
	end,
	renderHeaderRow = function(s)
		ui.withFont(orbiteer.xlarge.name, orbiteer.xlarge.size, function()
			ui.text('')
			ui.nextColumn()
			ui.text(l.SHIP)
			ui.nextColumn()
			ui.text(l.PRICE)
			ui.nextColumn()
			ui.text(l.CAPACITY)
			ui.nextColumn()
		end)
	end,
	renderItem = function(s, item)
		if(icons[item.def.shipClass] == nil) then
			icons[item.def.shipClass] = PiImage.New("icons/shipclass/".. item.def.shipClass ..".png")
			currentIconSize = icons[item.def.shipClass].texture.size
		end
		if not selectedItem then selectedItem = item end
		icons[item.def.shipClass]:Draw(widgetSizes.iconSize)
		ui.nextColumn()
		ui.withStyleVars({ItemSpacing = vZero}, function()
			ui.dummy(widgetSizes.rowVerticalSpacing)
			ui.text(item.def.name)
			ui.nextColumn()
			ui.dummy(widgetSizes.rowVerticalSpacing)
			ui.text(Format.Money(item.def.basePrice,false))
			ui.nextColumn()
			ui.dummy(widgetSizes.rowVerticalSpacing)
			ui.text(item.def.capacity.."t")
			ui.nextColumn()
		end)
	end,
	onClickItem = function(s,e)
		selectedItem = e
	end,
	sortingFunction = function(s1,s2) return s1.def.name < s2.def.name end
})

local function renderShipMarket()
	ui.withFont(pionillium.large.name, pionillium.large.size, function()
		ui.child("shipMarketContainer", Vector2(0, ui.getContentRegion().y - StationView.style.height), {}, function()
			shipMarket:render()
			ui.sameLine()
			tradeMenu()
		end)

		StationView:shipSummary()
	end)
end

StationView:registerView({
	id = "shipMarketView",
	name = l.SHIP_MARKET,
	icon = ui.theme.icons.ship,
	showView = true,
	draw = renderShipMarket,
	refresh = function()
		refreshShipMarket()
		shipMarket.scrollReset = true
	end,
})
