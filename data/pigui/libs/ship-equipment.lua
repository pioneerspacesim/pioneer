-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local ShipDef = require 'ShipDef'
local ModelSpinner = require 'PiGui.Modules.ModelSpinner'
local EquipCard = require 'pigui.libs.equip-card'
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

-- Equipment item grouping by underlying slot type
-- TODO: significant refactor to slot system to reduce highly-specialized slots
local sections = {
	{ name = le.PROPULSION, types = { "engine", "thruster", "hyperdrive" }, showCapacity = true },
	{ name = le.WEAPONS, type = "weapon", showCapacity = true },
	{ name = le.MISSILES, types = { "pylon", "missile" }, showCapacity = true },
	{ name = le.SHIELDS, type = "shield", showCapacity = true },
	{ name = le.SENSORS, type = "sensor", showCapacity = true, },
	{ name = le.COMPUTER_MODULES, type = "computer", showCapacity = true, },
	{ name = le.HULL_MOUNTS, types = { "hull", "utility" }, showCapacity = true },
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

	if equip.SpecializeForShip then
		equip = equip:SpecializeForShip(self.ship)
	else
		equip = equip:Instance()
	end

	player:AddMoney(-self.market:getBuyPrice(equip))
	self.station:AddEquipmentStock(equip:GetPrototype(), -1)

	assert(self.ship:GetComponent("EquipSet"):Install(equip, self.selectedSlot))

	self.selectedEquip = equip
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

		self.market.replaceEquip = self.selectedEquip
		self.market:refresh()
	end)

	self.market:hookMessage("onSellItem", function(_, item)
		self:onSellItem(item)

		self.market.replaceEquip = self.selectedEquip
		self.market:refresh()
	end)

	self.market:hookMessage("onClose", function(_, item)
		print("onClose")
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
	---@type ShipDef.Slot?
	self.selectedSlot = nil
	self.selectionActive = false

	self.modelSpinner = ModelSpinner()

	self.showHoveredEquipLocation = false
	self.lastHoveredEquipLine = Vector2(0, 0)
	self.lastHoveredEquipTag = nil

	self.tabs = { equipmentInfoTab }
	self.activeTab = 1

	self.id = id or "EquipmentWidget"
end

function EquipmentWidget:clearSelection()
	self.selectedEquip = nil
	self.selectedSlot = nil
	self.selectionActive = false
end

function EquipmentWidget:onSelectEquip(equipDetail)
	if self.selectionActive and not self.selectedSlot and self.selectedEquip == equipDetail then
		self:clearSelection()
		return
	end

	if self.station then
		self.selectedEquip = equipDetail
		self.selectedSlot = nil
		self.selectionActive = true

		self.market.filterSlot = nil
		self.market.replaceEquip = self.selectedEquip
		self.market:refresh()
	end
end

function EquipmentWidget:onSelectSlot(slot)
	if self.selectionActive and self.selectedSlot == slot then
		self:clearSelection()
		return
	end

	if self.station then
		self.selectedSlot = slot
		self.selectedEquip = self.ship:GetComponent("EquipSet"):GetItemInSlot(slot)
		self.selectionActive = true

		self.market.filterSlot = self.selectedSlot
		self.market.replaceEquip = self.selectedEquip
		self.market:refresh()
	end
end

function EquipmentWidget:onSetShipName(newName)
	self.ship:SetShipName(newName)
end

equipmentInfoTab = {
	name = l.EQUIPMENT,
	---@param self UI.EquipmentWidget
	draw = function(self)
		ui.withFont(pionillium.body, function()
			local equipSet = self.ship:GetComponent("EquipSet")

			for i, section in ipairs(sections) do
				local slots = {}

				for _, type in ipairs(section.types or { section.type }) do
					table.append(slots, equipSet:GetAllSlotsOfType(type, section.hardpoint))
				end

				self:drawSlotSection(section.name, slots)
			end

			self:drawEquipSection(le.MISC, equipSet:GetInstalledNonSlot())
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
	-- Apply tree indent from left
	ui.addCursorPos(Vector2(lineSpacing.x, 0))

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

-- Return the translated name of a slot, falling back to a generic name for the
-- slot type if none is specified.
---@param slot ShipDef.Slot
function EquipmentWidget:getSlotName(slot)
	if slot.i18n_key then
		return Lang.GetResource(slot.i18n_res)[slot.i18n_key]
	end

	local base_type = slot.type:match("(%w+)%.?")
	local i18n_key = (slot.hardpoint and "HARDPOINT_" or "SLOT_") .. base_type:upper()
	return le[i18n_key]
end

-- Show an inline detail on a section header line with optional tooltip
---@param cellEnd Vector2
local function drawHeaderDetail(cellEnd, text, icon, tooltip, textOffsetY)
	local textStart = cellEnd - Vector2(ui.calcTextSize(text).x + lineSpacing.x, 0)
	local iconPos = textStart - Vector2(iconSize.x + lineSpacing.x / 2, 0)

	ui.setCursorPos(iconPos)
	ui.icon(icon, iconSize, colors.white)

	ui.setCursorPos(textStart + Vector2(0, textOffsetY or 0))
	ui.text(text)

	local wp = ui.getWindowPos()
	if tooltip and ui.isMouseHoveringRect(wp + iconPos, wp + cellEnd + Vector2(0, iconSize.y)) then
		ui.setTooltip(tooltip)
	end
end

-- Draw an equipment section header
function EquipmentWidget:drawSectionHeader(name, numItems, totalWeight, maxSlots)
	-- This function makes heavy use of draw cursor manipulation to achieve
	-- complicated layout goals
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

	return sectionOpen
end

-- Calculate information about an equipment category for displaying ship internal equipment
---@param slots ShipDef.Slot[]
function EquipmentWidget:calcEquipSectionInfo(slots)
	local EquipSet = self.ship:GetComponent("EquipSet")

	local names = {}
	local occupied = 0
	local totalWeight = 0

	for _, slot in ipairs(slots) do
		names[slot] = self:getSlotName(slot)

		local item = EquipSet:GetItemInSlot(slot)
		if item then
			occupied = occupied + 1
			totalWeight = totalWeight + item.mass
		end
	end

	return names, occupied, totalWeight
end

-- Draw an equipment section and all contained equipment items
---@param equipment EquipType[]
function EquipmentWidget:drawEquipSection(name, equipment)

	-- local slots = data.slots or { data.slot }
	-- local equipment, maxSlots, weight = self:calcEquipSectionInfo(slots)
	local weight = 0
	for _, equip in ipairs(equipment) do
		weight = weight + equip.mass
	end

	local sectionOpen = self:drawSectionHeader(name, #equipment, weight)

	if sectionOpen then
		-- heaviest items to the top, then stably sort based on name
		table.sort(equipment, function(a, b)
			local mass = a.mass - b.mass
			return mass > 0 or (mass == 0 and a:GetName() < b:GetName())
		end)

		-- Draw each equipment item in this section
		for i, equip in ipairs(equipment) do
			local equipData = EquipCard.getDataForEquip(equip)
			local isSelected = self.selectedEquip == equip

			if self:drawEquipmentItem(equipData, isSelected) then
				self:message("onSelectEquip", equip)
			end
		end

		-- If we have more slots available in this section, show an empty slot
		if self.showEmptySlots then
			local equipData = EquipCard.getDataForEquip(nil)
			local isSelected = self.selectionActive and not self.selectedSlot and not self.selectedEquip

			if self:drawEquipmentItem(equipData, isSelected) then
				self:message("onSelectEquip", nil)
			end
		end

		ui.treePop()
	end

end

-- Draw a list of equipment slots and contained items
---@param slots ShipDef.Slot[]
function EquipmentWidget:drawSlotSection(name, slots)
	local names, numFull, totalMass = self:calcEquipSectionInfo(slots)

	local sectionOpen = self:drawSectionHeader(name, numFull, totalMass, #slots)
	if not sectionOpen then
		return
	end

	-- Sort by slot name first, then internal ID for tiebreaker
	table.sort(slots, function(a, b)
		return names[a] < names[b] or (names[a] == names[b] and a.id < b.id)
	end)

	local equipSet = self.ship:GetComponent("EquipSet")

	for _, slot in ipairs(slots) do
		local isSelected = self.selectedSlot == slot
		local equip = equipSet:GetItemInSlot(slot)

		if equip or self.showEmptySlots then

			local slotData = EquipCard.getDataForEquip(equip)

			slotData.size = slotData.size or ("S" .. slot.size)
			slotData.type = names[slot]
			slotData.count = slot.count

			if self:drawEquipmentItem(slotData, isSelected) then
				self:message("onSelectSlot", slot)
			end

		end
	end

	ui.treePop()
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

	self.market:refresh()
end

function EquipmentWidget:debugReload()
	package.reimport('pigui.libs.item-card')
	package.reimport('pigui.libs.equip-card')
	package.reimport('pigui.libs.equipment-outfitter')
	package.reimport('modules.Equipment.Stats')
	package.reimport()
end

return EquipmentWidget
