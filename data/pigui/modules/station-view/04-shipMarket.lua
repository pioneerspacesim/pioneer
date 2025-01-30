-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
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
local EquipSet = require 'EquipSet'
local HullConfig = require 'HullConfig'

local ui = require 'pigui'

local utils = require 'utils'

local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
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
---@type table<table, { price: integer, equip: { [1]: string, [2]: EquipType }[] }
local advertDataCache = {}
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

local function makeAdvertDataCacheEntry(shipOnSale)
	local def = shipOnSale.def ---@type ShipDef
	local config = assert(HullConfig.GetHullConfig(def.id))

	-- TODO: some sort of condition-based discount or an alteration to the
	-- price for purchasing directly from a manufacturer's station?
	local shipPrice = def.basePrice

	local equip = {}

	for _, slot in pairs(config.slots) do

		if slot.default then
			local defaultEquip = Equipment.Get(slot.default)

			-- Have to go through all of this to get an accurate cost for the ship...
			-- TODO: consider adding a "cost function" to EquipType which moves
			-- responsibility for computing accurate cost into the domain of the code
			-- that knows how the equipment will be specialized for the ship?
			if defaultEquip then
				local inst = defaultEquip:Instance()

				if inst.SpecializeForShip then
					inst:SpecializeForShip(config)
				end

				if slot.count then
					inst:SetCount(slot.count)
				end

				table.insert(equip, { slot.id, inst })
				shipPrice = shipPrice + inst.price
			end
		end

	end

	return { price = shipPrice, equip = equip }
end

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
	advertDataCache = utils.map_table(shipMarket.items, function(_, sos)
		return sos, makeAdvertDataCacheEntry(sos)
	end)
end

local function manufacturerIcon (manufacturer)
	if(manufacturer ~= "unknown" and manufacturer ~= nil) then
		if manufacturerIcons[manufacturer] == nil then
			manufacturerIcons[manufacturer] = PiImage.New("icons/manufacturer/".. manufacturer ..".png")
		end
		manufacturerIcons[manufacturer]:Draw(widgetSizes.iconSize)
	end
end

---@param ship Ship
local tradeInValue = function(ship)
	local shipDef = ShipDef[ship.shipId]
	local value = shipDef.basePrice * shipSellPriceReduction * ship.hullPercent/100

	-- We don't need to remove the hyperdrive from the value of the ship since the player is charged for it when buying the ship
	local equipment = ship:GetComponent("EquipSet"):GetInstalledEquipment()
	for _, e in pairs(equipment) do
		value = value + e.price * equipSellPriceReduction
	end

	return math.ceil(value)
end

local function buyShip (mkt, sos)
	local player = Game.player
	local station = assert(player:GetDockedWith())
	local def = sos.def

	local shipData = advertDataCache[sos]

	local cost = shipData.price - tradeInValue(Game.player)
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

	-- Not enough room to put all of the player's current cargo
	if def.cargo < player.usedCargo then
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

	-- TODO: ship ads should support an explicit list of (pre-owned) equipment as well as / instead of factory-default items
	-- At current we just build a list from the HullConfig's default items

	local equipSet = player:GetComponent('EquipSet')

	-- Install pre-built list of default equipment into ship
	for _, pair in ipairs(shipData.equip) do
		local handle = assert(equipSet:GetSlotHandle(pair[1]))

		if equipSet:CanInstallInSlot(handle, pair[2]) then
			equipSet:Install(pair[2], handle)
		else
			logWarning("Default equipment item {} for ship slot {}.{} is not compatible with slot." % { handle.default, player.shipId, handle.id })
		end
	end

	-- FIXME: fallback pass. Hyperdrives should be specified as a default item on the hyperdrive slot
	-- Once all hyperdrives are specified as default, this pass should be removed
	if def.hyperdriveClass > 0 and not player:GetInstalledHyperdrive() then
		local slot = player:GetComponent('EquipSet'):GetAllSlotsOfType('hyperdrive')[1]

		-- Install the best-fitting non-military hyperdrive we can
		local hyperdrive = utils.best_score(Equipment.new, function(_, equip)
			return EquipSet.CompatibleWithSlot(equip, slot) and equip.slot.type:match("%.civilian")
				and equip.capabilities.hyperclass or nil
		end)

		player:GetComponent('EquipSet'):Install(hyperdrive:Instance(), slot)
	end

	player:SetFuelPercent(100)
	refreshShipMarket();

	mkt.popup.msg = l.THANKS_AND_REMEMBER_TO_BUY_FUEL
	mkt.popup:open()
end

local FormatAndCompareShips = utils.class("ui.ShipMarket.FormatAndCompareShips")

function FormatAndCompareShips:beginTable()
	if not ui.beginTable("specs", 7 ) then
		return false
	end

	ui.tableSetupColumn("name1")
	ui.tableSetupColumn("body1")
	ui.tableSetupColumn("indicator1", {"WidthFixed"})
	ui.tableSetupColumn("gap", {"WidthFixed"})
	ui.tableSetupColumn("name2")
	ui.tableSetupColumn("body2")
	ui.tableSetupColumn("indicator2", {"WidthFixed"})

	return true
end

---@param desc       string        Description of the item being drawn
---@param a	         number        The value for the new ship
---@param b          number        The value for the old shio
---@param fmt_a      function|nil  A function that takes a and formats it to a string, if required
---@param fmt_b      function|nil  A function that takes b and formats it to a string, if nil then fmt_a is used
function FormatAndCompareShips:compare_and_draw_column(desc, a, b, fmt_a, fmt_b)
	local compare = a-b

	if self.column == 0 then
		ui.tableNextRow()
	end

	fmt_b = fmt_b and fmt_b or fmt_a

	ui.tableSetColumnIndex(0 + self.column)
	ui.textColored(ui.theme.colors.fontDim, desc)

	ui.tableSetColumnIndex(1 + self.column)

	local new_str = fmt_a and fmt_a( a ) or a

	if compare < 0 then
		local old_str = fmt_b and fmt_b( b ) or b
		ui.withTooltip( l.CURRENT_SHIP .. ": " .. old_str, function ()
			ui.textAlignedColored(new_str, 1.0, ui.theme.colors.compareWorse)
			ui.tableSetColumnIndex(2 + self.column)
			ui.icon( ui.theme.icons.shipmarket_compare_worse, Vector2(ui.getTextLineHeight()), ui.theme.colors.compareWorse)
		end )
	elseif compare > 0 then
		local old_str = fmt_b and fmt_b( b ) or b
		ui.withTooltip( l.CURRENT_SHIP .. ": " .. old_str, function ()
			ui.textAlignedColored(new_str, 1.0,  ui.theme.colors.compareBetter)
			ui.tableSetColumnIndex(2 + self.column)
			ui.icon( ui.theme.icons.shipmarket_compare_better, Vector2(ui.getTextLineHeight()), ui.theme.colors.compareBetter)
		end )
	else
		ui.textAligned(new_str, 1.0)
		ui.tableSetColumnIndex(2 + self.column)
		ui.dummy( Vector2(ui.getTextLineHeight()) )
	end

	if self.column == 0 then
		self.column = 4
	else
		self.column = 0
	end
end

function FormatAndCompareShips:draw_hyperdrive_cell(desc)

	local function fmt( v )
		return v > 0 and v < 8 and
			Equipment.new["hyperspace.hyperdrive_" .. v]:GetName() or l.NONE
	end

	self:compare_and_draw_column( desc, self.def.hyperdriveClass, self.b.def.hyperdriveClass, fmt )
end

function FormatAndCompareShips:get_value(key)
	local v = self[key]
	if nil == v then
		v = self.def[key]
	end
	return v
end

function FormatAndCompareShips:draw_tonnage_cell(desc, key)
	self:compare_and_draw_column( desc, self:get_value(key), self.b:get_value(key), Format.MassTonnes )
end

function FormatAndCompareShips:draw_accel_cell(desc, thrustKey, massKey )
	local accelA = self.def.linearThrust[thrustKey] / (9.8106*1000*(self:get_value(massKey)))
	local accelB = self.b.def.linearThrust[thrustKey] / (9.8106*1000*(self.b:get_value(massKey)))
	self:compare_and_draw_column( desc, accelA, accelB, Format.AccelG )
end

function FormatAndCompareShips:draw_deltav_cell(desc, massNumeratorKey, massDenominatorKey)
	local deltavA = self.def.effectiveExhaustVelocity * math.log( self:get_value(massNumeratorKey) / self:get_value(massDenominatorKey) )
	local deltavB = self.b.def.effectiveExhaustVelocity * math.log( self.b:get_value(massNumeratorKey) / self.b:get_value(massDenominatorKey) )

	local function fmt( v )
		return string.format("%d km/s", v / 1000)
	end

	self:compare_and_draw_column( desc, deltavA, deltavB, fmt )
end

function FormatAndCompareShips:draw_unformated_cell(desc, key)
	self:compare_and_draw_column( desc, self:get_value(key),  self.b:get_value(key) )
end

local function getNumSlotsCompatibleWithType(def, type)
	local config = HullConfig.GetHullConfig(def.id)
	local count = 0

	for _, slot in pairs(config.slots) do
		if EquipSet.SlotTypeMatches(type, slot.type) then
			count = count + (slot.count or 1)
		end
	end

	return count
end

local function getBestSlotSizeOfType(def, type)
	local config = HullConfig.GetHullConfig(def.id)
	local slot, size = utils.best_score(config.slots, function(_, slot)
		return EquipSet.SlotTypeMatches(type, slot.type) and slot.size or nil
	end)

	return slot and size or 0
end

function FormatAndCompareShips:draw_equip_slot_cell(desc, key)
	self:compare_and_draw_column( desc, getNumSlotsCompatibleWithType(self.def, key), getNumSlotsCompatibleWithType(self.b.def, key) )
end

function FormatAndCompareShips:draw_yes_no_equip_slot_cell(desc, key)

	local function fmt( v ) return v==1 and l.YES or l.NO end

	self:compare_and_draw_column( desc, getNumSlotsCompatibleWithType(self.def, key), getNumSlotsCompatibleWithType(self.b.def, key), fmt )
end

function FormatAndCompareShips:draw_atmos_pressure_limit_cell(desc)

	local a_shield = getBestSlotSizeOfType(self.def, "hull.atmo_shield")
	local b_shield = getBestSlotSizeOfType(self.b.def, "hull.atmo_shield")

	local function fmt( def, has_shield )
		local atmoSlot
		if has_shield > 1 then
			atmoSlot = string.format("%d(+%d/+%d) atm", def.atmosphericPressureLimit,
			def.atmosphericPressureLimit * (Equipment.new["hull.atmospheric_shielding_s2"].capabilities.atmo_shield - 1),
			def.atmosphericPressureLimit * (Equipment.new["hull.heavy_atmospheric_shielding_s2"].capabilities.atmo_shield - 1) )
		elseif has_shield > 0 then
			atmoSlot = string.format("%d(+%d) atm", def.atmosphericPressureLimit,
			def.atmosphericPressureLimit * (Equipment.new["hull.atmospheric_shielding_s2"].capabilities.atmo_shield - 1) )
		else
			atmoSlot = string.format("%d atm", def.atmosphericPressureLimit)
		end
		return atmoSlot
	end

	local function fmt_a( v )
		return fmt( self.def, a_shield )
	end

	local function fmt_b( v )
		return fmt( self.b.def, b_shield )
	end

	-- multiply the values by 1000 and then add on if there is capacity for atmo_shielding so that the compare takes that into account
	-- however, note the formatting ignores the passed in value and therefore displays correctly.
	self:compare_and_draw_column( desc, self.def.atmosphericPressureLimit*1000+a_shield, self.b.def.atmosphericPressureLimit*1000+b_shield, fmt_a, fmt_b )
end

function FormatAndCompareShips:Constructor(def, b)
	self.column = 0
	self.emptyMass = def.hullMass + def.fuelTankMass
	self.fullMass = def.hullMass + def.equipCapacity + def.fuelTankMass
	self.massAtCapacity = def.hullMass + def.equipCapacity
	self.cargoCapacity = def.cargo
	self.def = def
	self.b = b
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

				local cost = advertDataCache[selectedItem].price

				ui.withFont(pionillium.heading, function()
					ui.text(l.PRICE..": "..Format.Money(cost, false))
					ui.sameLine()
					ui.text(l.AFTER_TRADE_IN..": "..Format.Money(cost - tradeInValue(Game.player), false))
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

				local currentShip = FormatAndCompareShips.New( ShipDef[Game.player.shipId], nil )
				local shipFormatAndCompare = FormatAndCompareShips.New( def, currentShip )

				ui.child("ShipSpecs", Vector2(0, 0), function()
					ui.withStyleVars({ CellPadding = Vector2(3,4) }, function()

						if not shipFormatAndCompare:beginTable() then return end

						shipFormatAndCompare:draw_hyperdrive_cell( l.HYPERDRIVE_FITTED )
						shipFormatAndCompare:draw_tonnage_cell( l.CARGO_SPACE, "cargoCapacity" )
						shipFormatAndCompare:draw_accel_cell( l.FORWARD_ACCEL_FULL, "FORWARD", "fullMass" )
						shipFormatAndCompare:draw_tonnage_cell( l.WEIGHT_FULLY_LOADED, "fullMass" )
						shipFormatAndCompare:draw_accel_cell( l.FORWARD_ACCEL_EMPTY, "FORWARD", "emptyMass" )
						shipFormatAndCompare:draw_tonnage_cell( l.WEIGHT_EMPTY, "hullMass" )
						shipFormatAndCompare:draw_accel_cell( l.REVERSE_ACCEL_EMPTY, "REVERSE", "emptyMass" )
						shipFormatAndCompare:draw_tonnage_cell( l.EQUIPMENT_CAPACITY, "equipCapacity" )
						shipFormatAndCompare:draw_accel_cell( l.REVERSE_ACCEL_FULL, "REVERSE", "fullMass" )
						shipFormatAndCompare:draw_tonnage_cell( l.FUEL_WEIGHT, "fuelTankMass" )
						shipFormatAndCompare:draw_deltav_cell( l.DELTA_V_EMPTY, "emptyMass", "hullMass")
						shipFormatAndCompare:draw_unformated_cell( l.MINIMUM_CREW, "minCrew" )
						shipFormatAndCompare:draw_deltav_cell( l.DELTA_V_FULL, "fullMass", "massAtCapacity")
						shipFormatAndCompare:draw_unformated_cell( l.MAXIMUM_CREW, "maxCrew" )
						shipFormatAndCompare:draw_deltav_cell( l.DELTA_V_MAX, "fullMass", "hullMass")
						shipFormatAndCompare:draw_equip_slot_cell( l.MISSILE_MOUNTS, "missile" )
						shipFormatAndCompare:draw_yes_no_equip_slot_cell( l.ATMOSPHERIC_SHIELDING, "hull.atmo_shield" )
						shipFormatAndCompare:draw_atmos_pressure_limit_cell( l.ATMO_PRESS_LIMIT )
						shipFormatAndCompare:draw_equip_slot_cell( l.SCOOP_MOUNTS, "scoop" )
						shipFormatAndCompare:draw_equip_slot_cell( l.PASSENGER_CABIN_CAPACITY, "cabin" )

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
			ui.text(Format.Money(advertDataCache[item].price, false))
			ui.nextColumn()
			ui.dummy(widgetSizes.rowVerticalSpacing)
			ui.text(item.def.equipCapacity.."t")
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
