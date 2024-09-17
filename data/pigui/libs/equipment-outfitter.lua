-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game      = require 'Game'
local Economy   = require 'Economy'
local Equipment = require 'Equipment'
local EquipSet  = require 'EquipSet'
local Lang      = require 'Lang'
local utils     = require 'utils'

local ui = require 'pigui'

local pionillium = ui.fonts.pionillium

local colors = ui.theme.colors
local icons = ui.theme.icons
local styles = ui.theme.styles

local Module = require 'pigui.libs.module'
local EquipCard = require 'pigui.libs.equip-card'

local l = Lang.GetResource("ui-core")

local compare = function(a, b, invert)
	local n = invert and b - a or a - b
	if n > 0 then
		return colors.compareBetter
	elseif n == 0 then
		return colors.font
	else
		return colors.compareWorse
	end
end


local pigui = require 'Engine'.pigui

local framePadding = ui.rescaleUI(Vector2(10, 12))
local function getCustomButtonHeight()
	return framePadding.y * 2.0 + pionillium.heading.size * 1.5
end

local customButton = function(label, icon, infoText, variant)
	local lineHeight = pionillium.heading.size * 1.5
	local height = lineHeight + framePadding.y * 2.0
	local icon_size = Vector2(lineHeight)
	local size = Vector2(ui.getContentRegion().x, height)
	local rounding = styles.ItemCardRounding

	local iconOffset = framePadding + Vector2(rounding, 0)
	local textOffset = iconOffset   + Vector2(icon_size.x + framePadding.x, pionillium.heading.size * 0.25)
	local fontCol = ui.theme.colors.font

	local startPos = ui.getCursorScreenPos()

	variant = variant or ui.theme.buttonColors.dark
	local clicked = false

	ui.withButtonColors(variant, function()
		pigui.PushStyleVar("FramePadding", framePadding)
		pigui.PushStyleVar("FrameRounding", rounding)
		clicked = pigui.Button("##" .. label, size)
		pigui.PopStyleVar(2)
	end)

	if ui.isItemHovered() then
		local tl, br = ui.getItemRect()
		ui.addRectFilled(tl, tl + Vector2(rounding, size.y), fontCol, 4, 0x5)
	end

	ui.withFont(pionillium.heading, function()
		ui.addIconSimple(startPos + iconOffset, icon, icon_size, fontCol)
		ui.addText(startPos + textOffset, fontCol, label)

		if infoText then
			local calcSize = ui.calcTextSize(infoText)
			local infoTextOffset = Vector2(size.x - (framePadding.x + calcSize.x), textOffset.y)
			ui.addText(startPos + infoTextOffset, fontCol, infoText)
		else
			local endOffset = size.x - framePadding.x
			local width = ui.getTextLineHeight() / 1.6
			ui.addRectFilled(
				startPos + Vector2(endOffset - width, framePadding.y),
				startPos + Vector2(endOffset, height - framePadding.y),
				variant.normal, 4, 0)
		end
	end)

	return clicked
end

--=============================================================================

local EquipCardAvailable = EquipCard.New()
EquipCardAvailable.tooltipStats = false

---@class UI.EquipmentOutfitter.EquipData : UI.EquipCard.Data
---@field canInstall boolean
---@field available boolean
---@field price number

---@class UI.EquipmentOutfitter.EquipCard : UI.EquipCard
local EquipCardUnavailable = EquipCard.New()

EquipCardUnavailable.tooltipStats = false
EquipCardUnavailable.backgroundColor = ui.theme.styleColors.gray_800
EquipCardUnavailable.hoveredColor = ui.theme.styleColors.gray_700
EquipCardUnavailable.selectedColor = ui.theme.styleColors.gray_600
EquipCardUnavailable.textColor = ui.theme.styleColors.danger_300

---@param data UI.EquipmentOutfitter.EquipData
function EquipCardUnavailable:tooltipContents(data, isSelected)
	EquipCard.tooltipContents(self, data, isSelected)

	ui.spacing()

	ui.withStyleColors({ Text = self.textColor }, function()

		if not data.canInstall then
			ui.textWrapped(l.NOT_SUPPORTED_ON_THIS_SHIP % { equipment = data.name })
		else
			ui.textWrapped(l.YOU_NOT_ENOUGH_MONEY)
		end
	end)
end

--=============================================================================

---@class UI.EquipmentOutfitter : UI.Module
---@field New fun(): self
local Outfitter = utils.class("UI.EquipmentOutfitter", Module)

function Outfitter:Constructor()
	Module.Constructor(self)

	self.ship = nil ---@type Ship
	self.station = nil ---@type SpaceStation
	self.filterSlot = nil ---@type ShipDef.Slot?
	self.replaceEquip = nil ---@type EquipType?
	self.canSellEquip = false

	self.equipmentList = {} ---@type UI.EquipmentOutfitter.EquipData[]
	self.selectedEquip = nil ---@type UI.EquipmentOutfitter.EquipData?
	self.currentEquip = nil ---@type UI.EquipCard.Data?

	self.compare_stats = nil ---@type { label: string, a: EquipType.UI.Stats?, b: EquipType.UI.Stats? }[]?
end

--==================
-- Helper functions
--==================

function Outfitter:stationHasTech(level)
	level = level == "MILITARY" and 11 or level
	return self.station.techLevel >= level
end

-- Override to support e.g. custom equipment shops
---@return EquipType[]
function Outfitter:getAvailableEquipment()
	local shipConfig = self.ship:GetComponent('EquipSet').config
	local slotCount = self.filterSlot and self.filterSlot.count

	return utils.map_table(Equipment.new, function(id, equip)
		if self:getStock(equip) <= 0 then
			-- FIXME: restore when equipment stocking converted to new system
			-- return id, nil
		end

		if not equip.purchasable or not self:stationHasTech(equip.tech_level) then
			return id, nil
		end

		if not EquipSet.CompatibleWithSlot(equip, self.filterSlot) then
			return id, nil
		end

		-- Instance the equipment item if we need to modify it for the ship it's installed in
		if slotCount or equip.SpecializeForShip then
			equip = equip:Instance()
		end

		-- Some equipment items might change their details based on the ship they're installed in
		if equip.SpecializeForShip then
			equip:SpecializeForShip(shipConfig)
		end

		-- Some slots collapse multiple equipment items into a single logical item
		-- Those slots are treated as all-or-nothing for less confusion
		if slotCount then
			equip:SetCount(slotCount)
		end

		return id, equip
	end)
end

---@param e EquipType
function Outfitter:getStock(e)
	e = e.__proto or e
	return self.station:GetEquipmentStock(e)
end

-- Cost of the equipment item if buying
function Outfitter:getBuyPrice(e)
	-- If the item instance has a specific price, use that instead of the station price
	-- TODO: the station should instead have a price modifier that adjusts the price of an equipment item
	return rawget(e, "price") or self.station:GetEquipmentPrice(e:GetPrototype())
end

-- Money gained from equipment item if selling
function Outfitter:getSellPrice(e)
	-- If the item instance has a specific price, use that instead of the station price
	-- TODO: the station should instead have a price modifier that adjusts the price of an equipment item
	return (rawget(e, "price") or self.station:GetEquipmentPrice(e:GetPrototype())) * Economy.BaseResellPriceModifier
end

-- Purchase price of an item less the sale cost of the old item
function Outfitter:getInstallPrice(e)
	return self:getBuyPrice(e) - (self.replaceEquip and self:getSellPrice(self.replaceEquip) or 0)
end

function Outfitter.sortEquip(e1, e2)
	if e1.slot then
		return e1.slot.size < e2.slot.size
			or (e1.slot.size == e2.slot.size and e1:GetName() < e2:GetName())
	else
		return e1:GetName() < e2:GetName()
	end
end

function Outfitter:buildEquipmentList()
	local equipment = self:getAvailableEquipment()

	---@type EquipType[]
	local equipList = {}

	for _, v in pairs(equipment) do
		table.insert(equipList, v)
	end

	table.sort(equipList, self.sortEquip)

	local currentProto = self.replaceEquip and self.replaceEquip:GetPrototype()
	local equipSet = self.ship:GetComponent("EquipSet")
	local money = Game.player:GetMoney()

	self.currentEquip = self.replaceEquip and EquipCard.getDataForEquip(self.replaceEquip) or nil

	self.equipmentList = utils.map_array(equipList, function(equip)
		local data = EquipCard.getDataForEquip(equip, self.replaceEquip)
		---@cast data UI.EquipmentOutfitter.EquipData

		data.price = self:getBuyPrice(equip)

		if self.filterSlot then
			data.canInstall = equipSet:CanInstallInSlot(self.filterSlot, equip)
		else
			data.canInstall = equipSet:CanInstallLoose(equip)
		end

		data.available = data.canInstall and money >= self:getInstallPrice(equip)

		-- Replace condition widget with price instead
		-- trim leading '$' character since we're drawing it with an icon instead
		data[#data] = { icons.money, ui.Format.Money(data.price):sub(2, -1) }

		if equip:GetPrototype() == currentProto then
			data.type = l.INSTALLED
		end

		return data
	end)
end

local emptyStats = {}

function Outfitter:buildCompareStats()
	local a = self.selectedEquip and self.selectedEquip.stats or emptyStats
	local b = self.currentEquip and self.currentEquip.stats or emptyStats

	local out = {}

	if a and b then
		local s1 = 1
		local s2 = 1

		-- A bit messy, but essentially inserts the shared prefix of the array
		-- followed by the leftover values from A and then the leftover values
		-- from B.
		while s1 <= #a or s2 <= #b do
			local stat_a = a[s1]
			local stat_b = b[s2]

			if s1 == s2 and stat_a and stat_b and stat_a[1] == stat_b[1] then
				table.insert(out, { label = stat_a[1], a = stat_a, b = stat_b })
				s1 = s1 + 1
				s2 = s2 + 1
			elseif stat_a then
				table.insert(out, { label = stat_a[1], a = stat_a, b = nil })
				s1 = s1 + 1
			elseif stat_b then
				table.insert(out, { label = stat_b[1], a = nil, b = stat_b})
				s2 = s2 + 1
			end
		end
	elseif a then
		out = utils.map_array(a, function(v) return { label = v[1], a = v, b = nil } end)
	elseif b then
		out = utils.map_array(b, function(v) return { label = v[1], a = nil, b = v } end)
	end

	self.compare_stats = out
end

--==================
-- Message handlers
--==================

---@param item UI.EquipmentOutfitter.EquipData
function Outfitter:onSelectItem(item)
	self.selectedEquip = item
	self:buildCompareStats()
end

---@param item EquipType
function Outfitter:onBuyItem(item)
	self.selectedEquip = nil
end

---@param item EquipType
function Outfitter:onSellItem(item)
	self.selectedEquip = nil
end

function Outfitter:onClose()
	return
end

function Outfitter:refresh()
	self.selectedEquip = nil

	if not self.ship or not self.station then
		self.equipmentList = {}
		return
	end

	self:buildEquipmentList()
	self:buildCompareStats()
end

--==================
-- Render functions
--==================

function Outfitter:drawEquipmentItem(data, isSelected)
	if data.available then
		return EquipCardAvailable:draw(data, isSelected)
	else
		return EquipCardUnavailable:draw(data, isSelected)
	end
end

function Outfitter:drawBuyButton(data)
	local icon = icons.autopilot_dock
	local price_text = ui.Format.Money(self:getInstallPrice(data.equip))

	local variant = data.available and ui.theme.buttonColors.dark or ui.theme.buttonColors.disabled
	if customButton(l.BUY_EQUIP % data, icon, price_text, variant) and data.available then
		self:message("onBuyItem", data.equip)
	end
end

function Outfitter:drawSellButton(data)
	local icon = icons.autopilot_undock_illegal
	local price_text = ui.Format.Money(self:getSellPrice(data.equip))

	local variant = self.canSellEquip and ui.theme.buttonColors.dark or ui.theme.buttonColors.disabled
	if customButton(l.SELL_EQUIP % data, icon, price_text, variant) and self.canSellEquip then
		self:message("onSellItem", data.equip)
	end
end

---@param label string
---@param stat_a EquipType.UI.Stats?
---@param stat_b EquipType.UI.Stats?
function Outfitter:renderCompareRow(label, stat_a, stat_b)
	ui.tableNextColumn()
	ui.text(label)

	local icon_size = Vector2(ui.getTextLineHeight())
	local color = stat_a and stat_b
		and compare(stat_a[3], stat_b[3], stat_a[5])
		or colors.font

	ui.tableNextColumn()
	if stat_a then
		ui.icon(stat_a[2], icon_size, colors.font)
		ui.sameLine()

		local val, format = stat_a[3], stat_a[4]
		ui.textColored(color, format(val))
	end

	ui.tableNextColumn()
	if stat_b then
		ui.icon(stat_b[2], icon_size, colors.font)
		ui.sameLine()

		local val, format = stat_b[3], stat_b[4]
		ui.text(format(val))
	end
end

function Outfitter:renderCompareStats()
	if self.selectedEquip then
		ui.group(function()
			ui.text(l.SELECTED .. ":")

			ui.withFont(pionillium.heading, function()
				ui.text(self.selectedEquip.name)
			end)
		end)
	end

	if self.currentEquip then
		ui.sameLine()

		ui.group(function()
			ui.textAligned(l.EQUIPPED .. ":", 1.0)

			ui.withFont(pionillium.heading, function()
				ui.textAligned(self.currentEquip.name, 1.0)
			end)
		end)
	end

	ui.separator()
	ui.spacing()

	if self.selectedEquip then
		ui.textWrapped(self.selectedEquip.equip:GetDescription())
	elseif self.currentEquip then
		ui.textWrapped(self.currentEquip.equip:GetDescription())
	end

	ui.spacing()

	if self.compare_stats then

		ui.beginTable("##CompareEquipStats", 3)

		ui.tableSetupColumn("##name", { "WidthStretch" })
		ui.tableSetupColumn("##selected", { "WidthFixed" })
		ui.tableSetupColumn("##current", { "WidthFixed" })

		for _, row in ipairs(self.compare_stats) do
			ui.tableNextRow()
			self:renderCompareRow(row.label, row.a, row.b)
		end

		ui.endTable()

	end

end

function Outfitter:render()

	local spacing = ui.getItemSpacing()
	local panelWidth = (ui.getContentRegion().x - spacing.x * 4.0) / 2

	ui.child("##ListPane", Vector2(panelWidth, 0), function()

		ui.withStyleVars({ FrameRounding = 4 }, function()
			if ui.button("<") then
				self:message("onClose")
			end
		end)

		ui.sameLine()

		ui.withFont(pionillium.heading, function()
			ui.text(self.replaceEquip and l.REPLACE_EQUIPMENT_WITH or l.AVAILABLE_FOR_PURCHASE)
		end)

		ui.spacing()

		local buttonLineHeight = 0.0
		local spacing = ui.getItemSpacing()

		if self.selectedEquip then
			buttonLineHeight = buttonLineHeight + getCustomButtonHeight() + spacing.y
		end

		if self.currentEquip then
			buttonLineHeight = buttonLineHeight + getCustomButtonHeight() + spacing.y
		end

		if buttonLineHeight > 0.0 then
			buttonLineHeight = buttonLineHeight + spacing.y * 2.0
		end

		ui.child("##EquipmentList", Vector2(0, -buttonLineHeight), function()
			for _, data in ipairs(self.equipmentList) do
				local clicked = self:drawEquipmentItem(data, data == self.selectedEquip)

				if clicked then
					self:message("onSelectItem", data)
				end

				local doubleClicked = clicked and ui.isMouseDoubleClicked(0)

				if doubleClicked then
					self:message("onBuyItem", data.equip)
				end
			end
		end)

		if self.selectedEquip or self.currentEquip then
			ui.separator()
			ui.spacing()
		end

		if self.selectedEquip then
			self:drawBuyButton(self.selectedEquip)
		end

		if self.currentEquip then
			self:drawSellButton(self.currentEquip)
		end

	end)

	ui.sameLine(0, spacing.x * 4)

	local linePos = ui.getCursorScreenPos() - Vector2(spacing.x * 2, 0)
	ui.addLine(linePos, linePos + Vector2(0, ui.getContentRegion().y), ui.theme.styleColors.gray_500, 2.0)

	ui.group(function()
		local scannerSize = Vector2(panelWidth, (ui.getContentRegion().y - ui.getItemSpacing().y) / 2.0)

		ui.child("##EquipmentDetails", Vector2(panelWidth, scannerSize.y), function()

			if self.selectedEquip or self.currentEquip then

				ui.withFont(pionillium.body, function()
					self:renderCompareStats()
				end)

			end

		end)

		self:drawShipSpinner(scannerSize)
	end)

end

function Outfitter:drawShipSpinner(size)
	-- Override this in an owning widget to draw a ship spinner
end

return Outfitter
