-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local ShipDef = require 'ShipDef'
local ModelSpinner = require 'PiGui.Modules.ModelSpinner'
local EquipCard = require 'pigui.libs.equip-card'
local EquipSet  = require 'EquipSet'
local pigui = require 'Engine'.pigui
local Vector2 = Vector2

local utils = require 'utils'

local Module = require 'pigui.libs.module'
local Outfitter = require 'pigui.libs.equipment-outfitter'

local Lang = require 'Lang'
local l = Lang.GetResource("ui-core")
local le = Lang.GetResource("equipment-core")

local ui = require 'pigui'
local colors = ui.theme.colors
local icons = ui.theme.icons
local pionillium = ui.fonts.pionillium

local lineSpacing = ui.rescaleUI(Vector2(8, 6))
local iconSize = Vector2(pionillium.body.size)

local equipmentInfoTab

---@class UI.EquipmentWidget : UI.Module
---@field New fun(id): self
local EquipmentWidget = utils.class("UI.EquipmentWidget", Module)

-- Equipment item grouping by underlying slot kinds
EquipmentWidget.Sections = {
	{ name = le.PROPULSION, types = { "engine", "thruster", "hyperdrive" } },
	{ name = le.WEAPONS, type = "weapon" },
	{ name = le.MISSILES, types = { "pylon", "missile_bay", "missile" } },
	{ name = le.SHIELDS, type = "shield" },
	{ name = le.SENSORS, type = "sensor", },
	{ name = le.COMPUTER_MODULES, type = "computer", },
	{ name = le.CABINS, types = { "cabin" } },
	{ name = le.HULL_MOUNTS, types = { "hull", "utility", "fuel_scoop", "structure" } },
}

--
-- =============================================================================
--  Equipment Market Widget
-- =============================================================================
--

---@param equip EquipType
function EquipmentWidget:onBuyItem(equip)
	local player = Game.player

	if self.selectedEquip then
		self:onSellItem(self.selectedEquip)
	end

	if equip:isProto() then
		equip = equip:Instance()
	end

	player:AddMoney(-self.market:getBuyPrice(equip))
	self.station:AddEquipmentStock(equip:GetPrototype(), -1)

	assert(self.ship:GetComponent("EquipSet"):Install(equip, self.selectedSlot))

	self.selectedEquip = self.selectedSlot and equip
	self:buildSections()
end

---@param equip EquipType
function EquipmentWidget:onSellItem(equip)
	assert(self.selectedEquip)
	assert(self.selectedEquip == equip)

	local player = Game.player

	player:AddMoney(self.market:getSellPrice(equip))
	self.station:AddEquipmentStock(equip:GetPrototype(), 1)

	assert(self.ship:GetComponent("EquipSet"):Remove(equip))

	self.selectedEquip = nil
	self:buildSections()
end

--
-- =============================================================================
--  Ship Equipment Widget Display
-- =============================================================================
--

function EquipmentWidget:Constructor(id)
	Module.Constructor(self)

	self.market = Outfitter.New()

	self.market:hookMessage("onBuyItem", function(_, item)
		self:onBuyItem(item)

		if self.selectedSlot then
			local nextSlot = self.adjacentSlots[self.selectedSlot]
			if nextSlot and not nextSlot.equip then
				self:onSelectSlot(nextSlot)
			end
		end

		self.market.replaceEquip = self.selectedEquip
		self.market:refresh()
	end)

	self.market:hookMessage("onSellItem", function(_, item)
		self:onSellItem(item)

		self.market.replaceEquip = self.selectedEquip
		self.market:refresh()
	end)

	self.market:hookMessage("onClose", function(_, item)
		self:clearSelection()

		self.market.replaceEquip = nil
		self.market.filterSlot = nil
	end)

	self.market:hookMessage("drawShipSpinner", function(_, size)
		self.modelSpinner:setSize(size)
		self.modelSpinner:draw()
	end)

	---@type Ship
	self.ship = nil
	---@type SpaceStation?
	self.station = nil
	self.showShipNameEdit = false
	self.showEmptySlots = true

	---@type EquipType?
	self.selectedEquip = nil
	---@type HullConfig.Slot?
	self.selectedSlot = nil
	self.selectionActive = false

	self.modelSpinner = ModelSpinner()

	self.showHoveredEquipLocation = false
	self.lastHoveredEquipLine = Vector2(0, 0)
	self.lastHoveredEquipTag = nil

	self.tabs = { equipmentInfoTab }
	self.activeTab = 1

	self.id = id or "EquipmentWidget"

	self.sectionList = {}
	---@type table<HullConfig.Slot, UI.EquipCard.Data>
	self.adjacentSlots = {}
end

function EquipmentWidget:clearSelection()
	self.selectedEquip = nil
	self.selectedSlot = nil
	self.selectionActive = false
end

---@param slotData UI.EquipCard.Data
---@param children table?
function EquipmentWidget:onSelectSlot(slotData, children)
	if not slotData then
		self:clearSelection()
		return
	end

	local isSelected = self.selectedEquip == slotData.equip
		and self.selectedSlot == slotData.slot

	if self.selectionActive and isSelected then
		self:clearSelection()
		return
	end

	if self.station then
		self.selectedSlot = slotData.slot
		self.selectedEquip = slotData.equip
		self.selectionActive = true

		local hasChildren = children and children.count > 0

		self.market.filterSlot = self.selectedSlot
		self.market.replaceEquip = self.selectedEquip
		self.market.canReplaceEquip = not hasChildren
		self.market.canSellEquip = not (self.selectedSlot and self.selectedSlot.required or hasChildren)
		self.market:refresh()
	end
end

function EquipmentWidget:onSetShipName(newName)
	self.ship:SetShipName(newName)
end

-- Return the translated name of a slot, falling back to a generic name for the
-- slot type if none is specified.
---@param slot HullConfig.Slot
function EquipmentWidget:getSlotName(slot)
	if slot.i18n_key then
		return Lang.GetResource(slot.i18n_res)[slot.i18n_key]
	end

	local base_type = slot.type:match("([%w_-]+)%.?")
	local i18n_key = (slot.hardpoint and "HARDPOINT_" or "SLOT_") .. base_type:upper()
	return le[i18n_key]
end

function EquipmentWidget:buildSlotSubgroup(equipSet, equip, card)
	local slots = utils.build_array(pairs(equip.provides_slots))
	local subGroup = self:buildSlotGroup(equipSet, equip:GetName(), slots)

	-- Subgroups use the table identity of the parent equipment item as a
	-- stable identifier
	subGroup.id = tostring(equip)

	card.present = subGroup.count
	card.total = subGroup.countMax

	return subGroup
end

function EquipmentWidget:buildSlotGroup(equipSet, name, slots)
	local items = {}
	local children = {}
	local occupied = 0
	local totalWeight = 0

	-- Sort the table of slots lexicographically
	local names = utils.map_table(slots, function(_, slot) return slot, self:getSlotName(slot) end)
	table.sort(slots, function(a, b) return names[a] < names[b] or (names[a] == names[b] and a.id < b.id) end)

	for i, slot in ipairs(slots) do
		local equip = equipSet:GetItemInSlot(slot)

		-- Build item cards for all slots
		local slotData = EquipCard.getDataForEquip(equip)

		slotData.type = names[slot]
		slotData.size = slotData.size or ("S" .. slot.size)
		slotData.count = slotData.count or slot.count
		slotData.slot = slot

		if equip then
			occupied = occupied + 1
			totalWeight = totalWeight + equip.mass

			if equip.provides_slots then
				children[i] = self:buildSlotSubgroup(equipSet, equip, slotData)
				totalWeight = totalWeight + children[i].weight
			end
		end

		local prevCard = items[#items]
		if prevCard and prevCard.slot then
			self.adjacentSlots[prevCard.slot] = slotData
		end

		table.insert(items, slotData)
	end

	return {
		name = name,
		items = items,
		children = children,
		count = occupied,
		countMax = #slots,
		weight = totalWeight
	}
end

function EquipmentWidget:buildSections()
	self.sectionList = {}
	self.adjacentSlots = {}

	local equipSet = self.ship:GetComponent("EquipSet")
	local config = equipSet.config

	for i, section in ipairs(EquipmentWidget.Sections) do
		local slots = {}

		for _, type in ipairs(section.types or { section.type }) do
			for id, slot in pairs(config.slots) do
				local matches = EquipSet.SlotTypeMatches(slot.type, type) and (section.hardpoint == nil or section.hardpoint == slot.hardpoint)

				if matches then
					table.insert(slots, slot)
				end
			end
		end

		table.insert(self.sectionList, self:buildSlotGroup(equipSet, section.name, slots))
	end

	local nonSlot = equipSet:GetInstalledNonSlot()

	-- Sort non-slot equipment by total mass
	table.sort(nonSlot, function(a, b)
		return a.mass > b.mass or (a.mass == b.mass and a:GetName() < b:GetName())
	end)

	local equipCards = {}
	local children = {}
	local equipWeight = 0.0

	for i, equip in ipairs(nonSlot) do
		local card = EquipCard.getDataForEquip(equip)

		equipWeight = equipWeight + equip.mass

		if equip.provides_slots then
			children[i] = self:buildSlotSubgroup(equipSet, equip, card)
			equipWeight = equipWeight + children[i].weight
		end

		table.insert(equipCards, card)
	end

	table.insert(self.sectionList, {
		name = le.MISC_EQUIPMENT,
		items = equipCards,
		children = children,
		count = #nonSlot,
		weight = equipWeight,
		isMiscEquip = true
	})
end

equipmentInfoTab = {
	name = l.EQUIPMENT,
	---@param self UI.EquipmentWidget
	draw = function(self)
		ui.withFont(pionillium.body, function()
			for _, section in ipairs(self.sectionList) do
				self:drawSlotGroup(section)
			end

		end)
	end
}

--
-- =============================================================================
--  Equipment Item Drawing Functions
-- =============================================================================
--

-- Wrapper for EquipCard which handles updating the "last hovered" information
function EquipmentWidget:drawEquipmentItem(data, isSelected)
	local pos = ui.getCursorScreenPos()
	local isClicked, isHovered, size = EquipCard:draw(data, isSelected)

	if isHovered and data.tagName then
		self.showHoveredEquipLocation = true
		self.lastHoveredEquipLine(pos.x + size.x, pos.y + size.y * 0.5)
	end

	return isClicked, isHovered
end

--
-- =============================================================================
--  Equipment Section Drawing Functions
-- =============================================================================
--

-- Show an inline detail on a section header line with optional tooltip
---@param cellEnd Vector2
local function drawHeaderDetail(cellEnd, text, icon, tooltip, textOffsetY)
	local textStart = cellEnd - Vector2(ui.calcTextSize(text).x + lineSpacing.x, 0)
	local iconPos = textStart - Vector2(iconSize.x + lineSpacing.x / 2, 0)

	ui.setCursorPos(iconPos)
	ui.icon(icon, iconSize, colors.white)
	local tl = ui.getItemRect()

	ui.setCursorPos(textStart + Vector2(0, textOffsetY or 0))
	ui.text(text)
	local br = select(2, ui.getItemRect())

	if tooltip and ui.isMouseHoveringRect(tl, br) then
		ui.setTooltip(tooltip)
	end
end

-- Draw an equipment section header
function EquipmentWidget:drawSectionHeader(section, fun)
	-- This function makes heavy use of draw cursor manipulation to achieve
	-- complicated layout goals

	local name = section.name
	local totalWeight = section.weight
	local numItems = section.count
	local maxSlots = section.countMax

	---@type boolean, Vector2, Vector2
	local sectionOpen, contentsPos, cursorPos

	local cellWidth = ui.getContentRegion().x / 5
	local textOffsetY = (pionillium.heading.size - pionillium.body.size) / 2

	ui.withFont(pionillium.heading, function()
		ui.withStyleVars({FramePadding = lineSpacing}, function()

			local nodeFlags = { "FramePadding", (self.showEmptySlots or numItems > 0) and "DefaultOpen" or nil }
			sectionOpen = ui.treeNode(name, nodeFlags)
			contentsPos = ui.getCursorPos()
			ui.sameLine(0, 0)
			cursorPos = ui.getCursorPos() + Vector2(0, lineSpacing.y)

		end)
	end)

	-- Show the total weight of all items in this section
	local weightStr = ui.Format.Mass(totalWeight * 1000, 1)
	local cellEnd = cursorPos + Vector2(ui.getContentRegion().x - lineSpacing.x, 0)
	drawHeaderDetail(cellEnd, weightStr, icons.hull, le.TOTAL_MODULE_WEIGHT, textOffsetY)

	-- For sections with definite slot counts, show the number of used and total slots
	if maxSlots then
		local capacityStr = maxSlots > 0 and string.format("%d/%d", numItems, maxSlots) or tostring(numItems)
		cellEnd = cellEnd - Vector2(cellWidth, 0)
		drawHeaderDetail(cellEnd, capacityStr, icons.antinormal, le.TOTAL_MODULE_CAPACITY, textOffsetY)
	end

	ui.setCursorPos(contentsPos)

	if sectionOpen then
		fun()
		ui.treePop()
	end

	return sectionOpen
end

function EquipmentWidget:drawOpenHeader(id, defaultOpen, fun)
	local isOpen = pigui.GetBoolState(id, defaultOpen)

	if isOpen then
		ui.treePush(id)
		fun()
		ui.treePop()
	end

	local iconSize = Vector2(ui.getTextLineHeight())

	local clicked = ui.invisibleButton(id, Vector2(ui.getContentRegion().x, ui.getTextLineHeight()))
	local tl, br = ui.getItemRect()

	local color = ui.getButtonColor(ui.theme.buttonColors.transparent, ui.isItemHovered(), ui.isItemActive())
	ui.addRectFilled(tl, br, color, ui.theme.styles.ItemCardRounding, 0xF)

	ui.addIconSimple((tl + br - iconSize) * 0.5,
		isOpen and icons.chevron_up or icons.chevron_down,
		iconSize, colors.fontDim)

	ui.setItemTooltip(isOpen and l.COLLAPSE or l.EXPAND)

	if clicked then
		pigui.SetBoolState(id, not isOpen)
	end
end

function EquipmentWidget:drawSlotGroup(list)
	local drawList = function()
		for i, card in ipairs(list.items) do

			local equip = card.equip
			local isSelected = self.selectionActive
				and (self.selectedSlot == card.slot and self.selectedEquip == equip)

			if equip or self.showEmptySlots then
				if self:drawEquipmentItem(card, isSelected) then
					self:message("onSelectSlot", card, list.children[i])
				end

				local childSlots = list.children[i]
				if childSlots then
					self:drawSlotGroup(childSlots)
				end
			end

		end

		if self.showEmptySlots and list.isMiscEquip then
			local card = EquipCard.getDataForEquip(nil)
			local isSelected = self.selectionActive and not self.selectedSlot and not self.selectedEquip

			if self:drawEquipmentItem(card, isSelected) then
				self:message("onSelectSlot", card)
			end
		end
	end

	if list.id then
		self:drawOpenHeader(list.id, self.showEmptySlots or list.count > 0, drawList)
	else
		self:drawSectionHeader(list, drawList)
	end
end

--
-- =============================================================================
--  Ship Spinner / Station Market Display
-- =============================================================================
--

function EquipmentWidget:drawShipSpinner()
	local shipDef = ShipDef[self.ship.shipId]

	ui.group(function ()

		ui.withFont(ui.fonts.orbiteer.large, function()

			if self.showShipNameEdit then

				ui.alignTextToFramePadding()
				ui.text(shipDef.name)
				ui.sameLine()

				ui.pushItemWidth(-1.0)
				local entry, apply = ui.inputText("##ShipNameEntry", self.ship.shipName, ui.InputTextFlags {"EnterReturnsTrue"})
				ui.popItemWidth()

				if (apply) then
					self:message("onSetShipName", entry)
				end

			else
				ui.text(shipDef.name)
			end

		end)

		local startPos = ui.getCursorScreenPos()

		self.modelSpinner:setSize(ui.getContentRegion())
		self.modelSpinner:draw()

		-- WIP "physicalized component" display - draw a line between the equipment item
		-- and the location in the ship where it is mounted
		local lineStartPos = self.lastHoveredEquipLine

		-- TODO: disabled until all ships have tag markup for internal components
		if false and self.showHoveredEquipLocation then
			local tagPos = startPos + self.modelSpinner:getTagPos(self.lastHoveredEquipTag)
			local lineTurnPos = lineStartPos + Vector2(40, 0)
			local dir = (tagPos - lineTurnPos):normalized()
			ui.addLine(lineStartPos, lineTurnPos, colors.white, 2)
			ui.addLine(lineTurnPos, tagPos - dir * 4, colors.white, 2)
			ui.addCircle(tagPos, 4, colors.white, 16, 2)
		end

	end)
end

function EquipmentWidget:render()
	ui.withFont(pionillium.body, function()
		ui.child("ShipInfo", Vector2(ui.getContentRegion().x * 1 / 3, 0), { "NoSavedSettings" }, function()
			if #self.tabs > 1 then
				self.activeTab = ui.tabBarFont("##tabs", self.tabs, pionillium.heading, self)
			else
				self.tabs[1].draw(self)
			end
		end)

		ui.sameLine(0, lineSpacing.x * 2)

		ui.child("##container", function()
			if self.tabs[self.activeTab] == equipmentInfoTab and self.station and self.selectionActive then

				if self.selectionActive then
					self.market:render()
				end

			else
				self:drawShipSpinner()
			end
		end)
	end)
end

function EquipmentWidget:draw()
	-- reset hovered equipment state
	self.showHoveredEquipLocation = false

	self:update()
	self.market:update()

	self:render()
end

function EquipmentWidget:refresh()
	self.selectedEquip = nil
	self.selectedSlot = nil
	self.selectionActive = false

	local shipDef = ShipDef[self.ship.shipId]
	self.modelSpinner:setModel(shipDef.modelName, self.ship:GetSkin(), self.ship.model.pattern)
	self.modelSpinner.spinning = false

	self.market.ship = self.ship
	self.market.station = self.station

	self.market.filterSlot = nil
	self.market.replaceEquip = nil

	self:buildSections()
end

function EquipmentWidget:debugReload()
	package.reimport('pigui.libs.item-card')
	package.reimport('pigui.libs.equip-card')
	package.reimport('pigui.libs.equipment-outfitter')
	package.reimport('modules.Equipment.Stats')
	package.reimport()
end

return EquipmentWidget
