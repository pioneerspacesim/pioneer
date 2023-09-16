-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Game = require 'Game'
local Format = require 'Format'
local ShipDef = require 'ShipDef'
local Equipment = require 'Equipment'
local StationView = require 'pigui.views.station-view'
local Table = require 'pigui.libs.table'
local PiImage = require 'pigui.libs.image'
local ModelSpinner = require 'PiGui.Modules.ModelSpinner'
local CommodityType= require 'CommodityType'

local ui = require 'pigui'
local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local styleColors = ui.theme.styleColors
local l = Lang.GetResource("ui-core")
local Vector2 = _G.Vector2

local vZero = Vector2(0,0)
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

local shipSellPriceReduction = 0.65
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

local function refreshModelSpinner()
	if not selectedItem then return end
	cachedShip = selectedItem.def.modelName
	cachedPattern = selectedItem.pattern
	modelSpinner:setModel(cachedShip, selectedItem.skin, cachedPattern)
end

local function refreshShipMarket()
	widgetSizes.rowVerticalSpacing = Vector2(0, (widgetSizes.iconSize.y + widgetSizes.itemSpacing.y - pionillium.large.size)/2)

	local station = Game.player:GetDockedWith()
	shipMarket.items = station:GetShipsOnSale()
	selectedItem = nil
	shipMarket.selectedItem = nil
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

	player:SetFuelPercent(100)
	refreshShipMarket();

	mkt.popup.msg = l.THANKS_AND_REMEMBER_TO_BUY_FUEL
	mkt.popup:open()
end

local FormatAndCompareShips = {}

local get_color_and_mod_string_for_compare = function (str, a, b)
	compare = a-b
	if compare == 0 then
		return { str, styleColors.white, " " }
	elseif compare < 0 then
		-- note a preceeding 3 per en space as no column padding
		-- this is unicode point 2004
		return { str, styleColors.warning_300, " ↓" }
	else
		return { str,  styleColors.success_300, " ↑" }
	end
end

function FormatAndCompareShips:get_hyperdrive_tuple()
	local hyperdrive_str = self.def.hyperdriveClass > 0 and
		Equipment.hyperspace["hyperdrive_" .. self.def.hyperdriveClass]:GetName() or l.NONE
	
	return get_color_and_mod_string_for_compare( hyperdrive_str, self.def.hyperdriveClass, self.b.def.hyperdriveClass )
end

function FormatAndCompareShips:get_value( key )
	v = self[key]
	if nil == v then
		v = self.def[key]
	end
	return v
end

function FormatAndCompareShips:get_tonnage_tuple( key )
	return get_color_and_mod_string_for_compare( Format.MassTonnes(self:get_value(key)), self:get_value(key), self.b:get_value(key) )
end

function FormatAndCompareShips:get_accel_tuple( thrustKey, massKey, multiplier)
	local accelA = multiplier * self.def.linearThrust[thrustKey] / (-9.81*1000*(self:get_value(massKey)))
	local accelB = multiplier * self.b.def.linearThrust[thrustKey] / (-9.81*1000*(self.b:get_value(massKey)))
	return get_color_and_mod_string_for_compare( Format.AccelG(accelA), accelA, accelB )
end

function FormatAndCompareShips:get_deltav_tuple(massNumeratorKey, massDenominatorKey)
	local deltavA = self.def.effectiveExhaustVelocity * math.log( self:get_value(massNumeratorKey), self.b:get_value(massDenominatorKey) )
	local deltavB = self.b.def.effectiveExhaustVelocity * math.log( self.b:get_value(massNumeratorKey), self.b:get_value(massDenominatorKey) )
	
	return get_color_and_mod_string_for_compare( string.format("%d km/s", deltavA / 1000), deltavA, deltavB )
end

function FormatAndCompareShips:get_unformated_tuple(key)
	return get_color_and_mod_string_for_compare( self:get_value(key), self:get_value(key),  self.b:get_value(key) )
end

function FormatAndCompareShips:get_equip_slot_tuple(key)	
	return get_color_and_mod_string_for_compare( self.def.equipSlotCapacity[key], self.def.equipSlotCapacity[key], self.b.def.equipSlotCapacity[key] )
end

function FormatAndCompareShips:get_yes_no_equip_slot_tuple(key)
	binary = self.def.equipSlotCapacity[key]
	yes_no = "unknown"
	if binary == 1 then
		yes_no = l.YES
	elseif binary == 0 then
		yes_no = l.NO
	else
		error("argument to yes_no not 0 or 1")
	end

	return get_color_and_mod_string_for_compare( yes_no, self.def.equipSlotCapacity[key], self.b.def.equipSlotCapacity[key] )
end

function FormatAndCompareShips:get_atmos_pressure_limit_tuple()

	local atmoSlot
	if self.def.equipSlotCapacity.atmo_shield > 0 then
		atmoSlot = string.format("%d(+%d/+%d) atm", self.def.atmosphericPressureLimit,
		self.def.atmosphericPressureLimit * (Equipment.misc.atmospheric_shielding.capabilities.atmo_shield - 1),
		self.def.atmosphericPressureLimit * (Equipment.misc.heavy_atmospheric_shielding.capabilities.atmo_shield - 1) )
	else
		atmoSlot = string.format("%d atm", self.def.atmosphericPressureLimit)
	end
	return get_color_and_mod_string_for_compare( atmoSlot, self.def.atmosphericPressureLimit, self.b.def.atmosphericPressureLimit )
end

function FormatAndCompareShips:new(def,b)
	o = {}
	setmetatable( o, self )
	self.__index = self
	o.emptyMass = def.hullMass + def.fuelTankMass
	o.fullMass = def.hullMass + def.capacity + def.fuelTankMass
	o.massAtCapacity = def.hullMass + def.capacity
	o.cargoCapacity = def.equipSlotCapacity["cargo"]
	o.def = def
	o.b = b
	return o
end


local tradeMenu = function()
	if(selectedItem) then
		ui.withStyleVars({ ItemSpacing = shipMarket.style.itemSpacing}, function()
			ui.child("TradeMenu", Vector2(0,0), function()
				local colHeadingWidth = ui.getContentRegion().x - widgetSizes.buyButton.x
				local def = selectedItem.def

				ui.columns(2, "shipMarketInfo")
				ui.setColumnWidth(0, colHeadingWidth)

				ui.withFont(orbiteer.title, function()
					ui.text(selectedItem.def.name)
				end)
				ui.withFont(orbiteer.body, function()
					ui.text(shipClassString[selectedItem.def.shipClass])
				end)

				ui.withFont(pionillium.heading, function()
					ui.text(l.PRICE..": "..Format.Money(selectedItem.def.basePrice, false))
					ui.sameLine()
					ui.text(l.AFTER_TRADE_IN..": "..Format.Money(selectedItem.def.basePrice - tradeInValue(ShipDef[Game.player.shipId]), false))
				end)

				ui.nextColumn()
				ui.dummy(widgetSizes.iconSpacer)
				ui.sameLine()
				manufacturerIcon(selectedItem.def.manufacturer)
				local shipBought = false
				ui.withFont(pionillium.body, function()
					shipBought = ui.button(l.BUY_SHIP, widgetSizes.buyButton)
				end)
				ui.columns(1, "")

				if shipBought then
					buyShip(shipMarket, selectedItem)
					return
				end

				local spinnerWidth = ui.getContentRegion().x
				modelSpinner:setSize(Vector2(spinnerWidth, spinnerWidth / 2.5))
				modelSpinner:draw()

				local shipFormatAndCompare = FormatAndCompareShips:new( def, FormatAndCompareShips:new( ShipDef[Game.player.shipId], nil ) )

				local shipInfoTable = {
					{
						l.HYPERDRIVE_FITTED, shipFormatAndCompare:get_hyperdrive_tuple(),
						l.CARGO_SPACE, shipFormatAndCompare:get_tonnage_tuple( "cargoCapacity" )
					}, {
						l.FORWARD_ACCEL_FULL, shipFormatAndCompare:get_accel_tuple( "FORWARD", "fullMass", 1 ),
						l.WEIGHT_FULLY_LOADED, shipFormatAndCompare:get_tonnage_tuple( "fullMass" )
					}, {
						l.FORWARD_ACCEL_EMPTY, shipFormatAndCompare:get_accel_tuple( "FORWARD", "emptyMass", 1 ),
						l.WEIGHT_EMPTY, shipFormatAndCompare:get_tonnage_tuple( "hullMass" )
					}, {
						l.REVERSE_ACCEL_EMPTY, shipFormatAndCompare:get_accel_tuple( "REVERSE", "emptyMass", -1 ),
						l.CAPACITY, shipFormatAndCompare:get_tonnage_tuple( "capacity" )
					}, {
						l.REVERSE_ACCEL_FULL, shipFormatAndCompare:get_accel_tuple( "REVERSE", "fullMass", -1 ),
						l.FUEL_WEIGHT, shipFormatAndCompare:get_tonnage_tuple( "fuelTankMass" )
					}, {
						l.DELTA_V_EMPTY, shipFormatAndCompare:get_deltav_tuple( "emptyMass", "hullMass"),
						l.MINIMUM_CREW, shipFormatAndCompare:get_unformated_tuple( "minCrew" )
					}, {
						l.DELTA_V_FULL, shipFormatAndCompare:get_deltav_tuple( "fullMass", "massAtCapacity"),
						l.MAXIMUM_CREW, shipFormatAndCompare:get_unformated_tuple( "maxCrew" )
					}, {
						l.DELTA_V_MAX, shipFormatAndCompare:get_deltav_tuple( "fullMass", "hullMass"),
						l.MISSILE_MOUNTS, shipFormatAndCompare:get_equip_slot_tuple( "missile" )
					}, {
						l.ATMOSPHERIC_SHIELDING, shipFormatAndCompare:get_yes_no_equip_slot_tuple( "atmo_shield" ),
						l.ATMO_PRESS_LIMIT, shipFormatAndCompare:get_atmos_pressure_limit_tuple()
					}, {
						l.SCOOP_MOUNTS, shipFormatAndCompare:get_equip_slot_tuple( "scoop" ),
						l.PASSENGER_CABIN_CAPACITY, shipFormatAndCompare:get_equip_slot_tuple( "cabin" )
					},
				}

				ui.child("ShipSpecs", Vector2(0, 0), function()
					ui.withStyleVars({ CellPadding = Vector2(8, 4) }, function()

						if not ui.beginTable("specs", 6, {"NoPadInnerX"} ) then return end

						ui.tableSetupColumn("name1")
						ui.tableSetupColumn("body1")
						ui.tableSetupColumn("indicator1", {"WidthFixed"})
						ui.tableSetupColumn("name2")
						ui.tableSetupColumn("body2")
						ui.tableSetupColumn("indicator2", {"WidthFixed"})

						for _, item in ipairs(shipInfoTable) do
							ui.tableNextRow()

							ui.tableSetColumnIndex(0)
							ui.textColored(styleColors.gray_300, item[1])

							ui.tableSetColumnIndex(1)
							ui.textAlignedColored(item[2][1], 1.0, item[2][2])

							ui.tableSetColumnIndex(2)
							-- this is an em space (unicode point 2003)
							-- as we don't have and column padding
							ui.textColored(item[2][2],item[2][3] .. " ")

							ui.tableSetColumnIndex(3)
							ui.textColored(styleColors.gray_300, item[3])

							ui.tableSetColumnIndex(4)
							ui.textAlignedColored(item[4][1], 1.0, item[4][2])

							ui.tableSetColumnIndex(5)
							ui.textColored(item[2][2],item[4][3])
						end

						ui.endTable()

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
	renderHeaderRow = function(_)
		ui.withFont(orbiteer.heading, function()
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
	renderItem = function(_, item)
		if(icons[item.def.shipClass] == nil) then
			icons[item.def.shipClass] = PiImage.New("icons/shipclass/".. item.def.shipClass ..".png")
		end
		if not selectedItem then
			selectedItem = item
			shipMarket.selectedItem = item
			refreshModelSpinner()
		end
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
	onClickItem = function(_,e)
		selectedItem = e
		shipMarket.selectedItem = e
		refreshModelSpinner()
	end,
	sortingFunction = function(s1,s2) return s1.def.name < s2.def.name end
})

StationView:registerView({
	id = "shipMarketView",
	name = l.SHIP_MARKET,
	icon = ui.theme.icons.ship,
	showView = true,
	draw = function()
		ui.withFont(pionillium.heading, function()
			shipMarket:render()
		end)

		ui.sameLine(0, widgetSizes.itemSpacing.x)
		tradeMenu()
	end,
	refresh = function()
		refreshShipMarket()
		shipMarket.scrollReset = true
	end,
	debugReload = function()
		package.reimport()
	end
})
