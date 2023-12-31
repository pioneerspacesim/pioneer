-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Equipment = require 'Equipment'
local Format = require 'Format'
local Game = require 'Game'
local ShipDef = require 'ShipDef'
local ModelSpinner = require 'PiGui.Modules.ModelSpinner'
local EquipMarket = require 'pigui.libs.equipment-market'
local ItemCard = require 'pigui.libs.item-card'
local EquipType = require 'EquipType'
local Vector2 = Vector2
local utils = require 'utils'

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

---@class EquipmentWidget
---@field meta table
local EquipmentWidget = utils.inherits(nil, "EquipmentWidget")

-- Slot information for the empty slot + example slot data layout
local emptySlot = {
	-- type = "Slot", name = "[EMPTY]"
	icon = icons.autopilot_dock, --size = "S1",
	{ icons.hull, ui.Format.Mass(0, 1), le.EQUIPMENT_WEIGHT },
	-- { icons.repairs, "100%", le.EQUIPMENT_INTEGRITY }
	-- { icons.ecm_advanced, "0 KW", "Max Power Draw" },
	-- { icons.temperature, "0 KW", "Operating Heat" },
}

-- Equipment item grouping by underlying slot type
-- TODO: significant refactor to slot system to reduce highly-specialized slots
local sections = {
	{ name = le.PROPULSION, slot = "engine", showCapacity = true },
	{ name = le.WEAPONS, slot = "laser_front", showCapacity = true },
	{ name = le.MISSILES, slot = "missile", showCapacity = true },
	{ name = le.SCOOPS, slot = "scoop", showCapacity = true },
	{ name = le.SENSORS, showCapacity = true, slots = {
		"sensor", "radar", "target_scanner", "hypercloud"
	} },
	{ name = le.SHIELDS, slot = "shield" },
	{ name = le.UTILITY, slots = {
		"cabin", "ecm", "hull_autorepair",
		"energy_booster", "atmo_shield",
		"laser_cooler", "cargo_life_support",
		"autopilot", "trade_computer", "thruster"
	} }
}

--
-- =============================================================================
--  Equipment Item Card
-- =============================================================================
--

---@class UI.EquipCard : UI.ItemCard
local EquipCard = utils.inherits(ItemCard, "UI.EquipCard")

EquipCard.highlightBar = true
EquipCard.detailFields = 4 - 0.3

function EquipCard:drawTooltip(data, isSelected)
	if data.equip then
		local desc = data.equip:GetDescription()
		if desc and #desc > 0 then
			ui.withStyleVars({ WindowPadding = ui.theme.styles.WindowPadding }, function()
				ui.setTooltip(desc)
			end)
		end
	end
end

function EquipCard:drawTitle(data, textWidth, isSelected)
	local pos = ui.getCursorPos()

	-- Draw the slot type
	if data.type then
		ui.text(data.type .. ":")
		ui.sameLine()
	end

	-- Draw the name of what's in the slot
	local fontColor = data.name and colors.white or colors.equipScreenBgText

	local name = data.name or ("[" .. le.EMPTY_SLOT .. "]")
	ui.withStyleColors({ Text = fontColor }, function() ui.text(name) end)

	-- Draw the size of the slot
	if data.size then
		ui.setCursorPos(pos + Vector2(textWidth - ui.calcTextSize(data.size).x --[[ - self.lineSpacing.x ]], 0))
		ui.withStyleColors({ Text = colors.equipScreenBgText }, function()
			ui.text(data.size)
		end)
	end
end

--
-- =============================================================================
--  Equipment Market Widget
-- =============================================================================
--

-- Copied from 05-equipmentMarket.lua
local hasTech = function (station, e)
	local equip_tech_level = e.tech_level or 1 -- default to 1

	if type(equip_tech_level) == "string" then
		if equip_tech_level == "MILITARY" then
			return station.techLevel == 11
		else
			error("Unknown tech level:\t"..equip_tech_level)
		end
	end

	assert(type(equip_tech_level) == "number")
	return station.techLevel >= equip_tech_level
end

local function makeEquipmentMarket()
return EquipMarket.New("EquipmentMarket", l.AVAILABLE_FOR_PURCHASE, {
	itemTypes = { Equipment.misc, Equipment.laser, Equipment.hyperspace },
	columnCount = 5,
	initTable = function(self)
		ui.setColumnWidth(0, self.style.size.x / 2.5)
		ui.setColumnWidth(3, ui.calcTextSize(l.MASS).x + self.style.itemSpacing.x + self.style.itemPadding.x)
		ui.setColumnWidth(4, ui.calcTextSize(l.IN_STOCK).x + self.style.itemSpacing.x + self.style.itemPadding.x)
	end,
	renderHeaderRow = function(self)
		ui.text(l.NAME_OBJECT)
		ui.nextColumn()
		ui.text(l.BUY)
		ui.nextColumn()
		ui.text(l.SELL)
		ui.nextColumn()
		ui.text(l.MASS)
		ui.nextColumn()
		ui.text(l.IN_STOCK)
		ui.nextColumn()
	end,
	renderItem = function(self, item)
		ui.withTooltip(item:GetDescription(), function()
			ui.text(item:GetName())
			ui.nextColumn()
			ui.text(Format.Money(self.funcs.getBuyPrice(self, item)))
			ui.nextColumn()
			ui.text(Format.Money(self.funcs.getSellPrice(self, item)))
			ui.nextColumn()
			ui.text(item.capabilities.mass.."t")
			ui.nextColumn()
			ui.text(self.funcs.getStock(self, item))
			ui.nextColumn()
		end)
	end,
	canDisplayItem = function (s, e)
		local filterSlot = not s.owner.selectedEquip
		if s.owner.selectedEquip then
			for k, v in pairs(s.owner.selectedEquipSlots) do
				filterSlot = filterSlot or utils.contains(e.slots, v)
			end
		end
		return e.purchasable and hasTech(s.owner.station, e) and filterSlot
	end,
	onMouseOverItem = function(s, e)
		local tooltip = e:GetDescription()
		if string.len(tooltip) > 0 then
			ui.withFont(pionillium.body, function() ui.setTooltip(tooltip) end)
		end
	end,
	onClickItem = function(s,e)
		s.funcs.buy(s, e)
		s:refresh()
	end,
	-- If we have an equipment item selected, we're replacing it.
	onClickBuy = function(s,e)
		local selected = s.owner.selectedEquip
		if selected[1] then
			s.owner.ship:RemoveEquip(selected[1], 1, selected.slot)
			s.owner.ship:AddMoney(s.funcs.getSellPrice(s, selected[1]))
			s.owner.ship:GetDockedWith():AddEquipmentStock(selected[1], 1)
		end

		return true
	end,
	-- If the purchase failed, undo the sale of the item previously in the slot
	onBuyFailed = function(s, e, reason)
		local selected = s.owner.selectedEquip
		if selected[1] then
			s.owner.ship:AddEquip(selected[1], 1, selected.slot)
			s.owner.ship:AddMoney(-s.funcs.getSellPrice(s, selected[1]))
			s.owner.ship:GetDockedWith():AddEquipmentStock(e, -1)
		end

		s.defaultFuncs.onBuyFailed(s, e, reason)
	end
})
end

--
-- =============================================================================
--  Ship Equipment Widget Display
-- =============================================================================
--

function EquipmentWidget.New(id)
	---@class EquipmentWidget
	local self = setmetatable({}, EquipmentWidget.meta)

	---@type Ship
	self.ship = nil
	---@type SpaceStation?
	self.station = nil
	self.showShipNameEdit = false
	self.showEmptySlots = true

	self.selectedEquip = nil
	self.selectedEquipSlots = nil
	self.modelSpinner = ModelSpinner()

	self.showHoveredEquipLocation = false
	self.lastHoveredEquipLine = Vector2(0, 0)
	self.lastHoveredEquipTag = nil

	self.equipmentMarket = makeEquipmentMarket()
	self.equipmentMarket.owner = self
	self.tabs = { equipmentInfoTab }
	self.activeTab = 1

	self.id = id or "EquipmentWidget"
	return self
end

--[[
self.selectedEquip format:
{
	[1] = equipment table or nil
	[2] = equipment index in slot or nil
	slot = name of slot currently selected
	mass = mass of current equipment
	name = name of current equipment
}
--]]

function EquipmentWidget:onEquipmentClicked(equipDetail, slots)
	if self.station then
		self.selectedEquip = equipDetail
		self.selectedEquipSlots = slots
		self.equipmentMarket:refresh()
		self.equipmentMarket.scrollReset = true
	end
end

function EquipmentWidget:onEmptySlotClicked(slots)
	if self.station then
		self.selectedEquip = { nil, nil }
		self.selectedEquipSlots = slots
		self.equipmentMarket:refresh()
		self.equipmentMarket.scrollReset = true
	end
end

equipmentInfoTab = {
	name = l.EQUIPMENT,
	---@param self EquipmentWidget
	draw = function(self)
		ui.withFont(pionillium.body, function()
			for i, v in ipairs(sections) do
				self:drawEquipSection(v)
			end
		end)
	end
}

--
-- =============================================================================
--  Equipment Item Drawing Functions
-- =============================================================================
--

-- Generate all UI data appropriate to the passed equipment item
local function makeEquipmentData(equip)
	local out = {
		icon = equip.icon_name and icons[equip.icon_name] or icons.systems_management,
		name = equip:GetName(),
		equip = equip
	}

	if equip:Class() == EquipType.LaserType then
		table.insert(out, {
			icons.comms, -- PLACEHOLDER
			string.format("%d RPM", 60 / equip.laser_stats.rechargeTime),
			le.SHOTS_PER_MINUTE
		})
		table.insert(out, {
			icons.ecm_advanced,
			string.format("%.1f KW", equip.laser_stats.damage),
			le.DAMAGE_PER_SHOT
		})
	elseif equip:Class() == EquipType.BodyScannerType then
		table.insert(out, {
			icons.scanner,
			string.format("%s px", ui.Format.Number(equip.stats.resolution, 0)),
			le.SENSOR_RESOLUTION
		})

		table.insert(out, {
			icons.altitude,
			ui.Format.Distance(equip.stats.minAltitude),
			le.SENSOR_MIN_ALTITUDE
		})
	-- elseif equip:Class() == EquipType.HyperdriveType then
	-- elseif equip:Class() == EquipType.SensorType then
	-- elseif utils.contains(equip.slots, "missile") then
	-- elseif utils.contains(equip.slots, "shield") then
	-- elseif utils.contains(equip.slots, "cabin") then
	-- elseif utils.contains(equip.slots, "radar") then
	-- elseif utils.contains(equip.slots, "autopilot") then
	end -- TODO: more data for different equip types

	local equipHealth = 1
	local equipMass = equip.capabilities.mass * 1000
	table.insert(out, { icons.hull, ui.Format.Mass(equipMass, 1), le.EQUIPMENT_WEIGHT })
	table.insert(out, { icons.repairs, string.format("%d%%", equipHealth * 100), le.EQUIPMENT_INTEGRITY })

	return out
end

-- Draw all visuals related to an individual equipment item.
-- This includes the background, highlight, icon, [slot type],
-- name, [size], and up to 4 sub-stats directly on the icon card
--
-- DrawEquipmentItem* functions take a "data" argument table with the form:
--   name  - translated name to display on the slot
--   equip - equipment object being drawn (or none for an empty slot)
--   type* - translated "slot type" name to display
--   size* - (short) string to be displayed in the "equipment size" field
--   [...] - up to 4 { icon, value, tooltip } data items for the stats line
function EquipmentWidget:drawEquipmentItem(data, isSelected)
	ui.addCursorPos(Vector2(lineSpacing.x * 2, 0))

	local pos = ui.getCursorScreenPos()
	local isClicked, isHovered, size = EquipCard:draw(data, isSelected)

	if isHovered and data.tagName then
		self.showHoveredEquipLocation = true
		self.lastHoveredEquipLine(pos.x + size.x, pos.y + size.y * 0.5)
	end

	return isClicked, isHovered
end

-- Override this to draw any detailed tooltips
function EquipmentWidget:drawEquipmentItemTooltip(data, isSelected)
	if data.equip then
		local desc = data.equip:GetDescription()
		if desc and #desc > 0 then ui.setTooltip(desc) end
	end
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
	ui.setCursorPos(textStart + Vector2(0, textOffsetY or 0))
	ui.text(text)
	local wp = ui.getWindowPos()
	if tooltip and ui.isMouseHoveringRect(wp + iconPos, wp + cellEnd + Vector2(0, iconSize.y)) then
		ui.setTooltip(tooltip)
	end
end

-- Draw an equipment section header
function EquipmentWidget:drawSectionHeader(data, numItems, maxSlots, totalWeight)

	-- This function makes heavy use of draw cursor maniupulation to achieve
	-- complicated layout goals
	---@type boolean, Vector2, Vector2
	local sectionOpen, contentsPos, cursorPos
	local cellWidth = ui.getContentRegion().x / 5
	local textOffsetY = (pionillium.heading.size - pionillium.body.size) / 2

	ui.withFont(pionillium.heading, function()
		ui.withStyleVars({FramePadding = lineSpacing}, function()
			sectionOpen = ui.treeNode(data.name, { "FramePadding", (self.showEmptySlots or numItems > 0) and "DefaultOpen" or nil })
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
	if data.showCapacity then
		local capacityStr = maxSlots > 0 and string.format("%d/%d", numItems, maxSlots) or tostring(numItems)
		cellEnd = cellEnd - Vector2(cellWidth, 0)
		drawHeaderDetail(cellEnd, capacityStr, icons.antinormal, le.TOTAL_MODULE_CAPACITY, textOffsetY)
	end

	ui.setCursorPos(contentsPos)

	return sectionOpen
end

-- Calculate information about an equipment category for displaying ship internal equipment
function EquipmentWidget:calcEquipSectionInfo(slots)
	local equipment = {}
	local maxSlots = 0
	local totalWeight = 0

	-- Gather all equipment items in the specified slot(s) for this section
	-- TODO: this can be refactored once the equipment system has been overhauled

	for _, name in ipairs(slots) do
		local slot = self.ship:GetEquip(name)
		maxSlots = maxSlots + self.ship:GetEquipSlotCapacity(name)

		for i, equip in pairs(slot) do
			table.insert(equipment, {
				equip, i,
				slot = name,
				mass = equip.capabilities.mass,
				name = equip:GetName()
			})
			totalWeight = totalWeight + (equip.capabilities.mass or 0)
		end
	end

	return equipment, maxSlots, totalWeight
end

-- Draw an equipment section and all contained equipment items
function EquipmentWidget:drawEquipSection(data)

	local slots = data.slots or { data.slot }
	local equipment, maxSlots, weight = self:calcEquipSectionInfo(slots)

	local sectionOpen = self:drawSectionHeader(data, #equipment, maxSlots, weight)

	if sectionOpen then
		-- heaviest items to the top, then stably sort based on name
		table.sort(equipment, function(a, b)
			local mass = (a.mass or 0) - (b.mass or 0)
			return mass > 0 or (mass == 0 and a.name < b.name)
		end)

		-- Draw each equipment item in this section
		for i, v in ipairs(equipment) do
			local equipData = makeEquipmentData(v[1])
			local isSelected = self.selectedEquip and (self.selectedEquip[1] == v[1] and self.selectedEquip[2] == v[2])

			if self:drawEquipmentItem(equipData, isSelected) then
				self:onEquipmentClicked(v, slots)
			end
		end

		-- If we have more slots available in this section, show an empty slot
		if maxSlots > 0 and self.showEmptySlots and #equipment < maxSlots then
			local isSelected = self.selectedEquip and (not self.selectedEquip[1] and self.selectedEquipSlots[1] == slots[1])

			if self:drawEquipmentItem(emptySlot, isSelected) then
				self:onEmptySlotClicked(slots)
			end
		end

		ui.treePop()
	end

end

--
-- =============================================================================
--  Ship Spinner / Station Market Display
-- =============================================================================
--

function EquipmentWidget:drawShipSpinner()
	if not self.modelSpinner then self:refresh() end

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

				if (apply) then self.ship:SetShipName(entry) end
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
		if self.showHoveredEquipLocation then
			local tagPos = startPos + self.modelSpinner:getTagPos(self.lastHoveredEquipTag)
			local lineTurnPos = lineStartPos + Vector2(40, 0)
			local dir = (tagPos - lineTurnPos):normalized()
			ui.addLine(lineStartPos, lineTurnPos, colors.white, 2)
			ui.addLine(lineTurnPos, tagPos - dir * 4, colors.white, 2)
			ui.addCircle(tagPos, 4, colors.white, 16, 2)
		end
	end)
end

function EquipmentWidget:drawMarketButtons()
	if ui.button(l.GO_BACK, Vector2(0, 0)) then
		self.selectedEquip = nil
		return
	end
	ui.sameLine()

	if self.selectedEquip[1] then
		local price = self.equipmentMarket.funcs.getSellPrice(self.equipmentMarket, self.selectedEquip[1])

		if ui.button(l.SELL_EQUIPPED, Vector2(0, 0)) then
			self.ship:RemoveEquip(self.selectedEquip[1], 1, self.selectedEquip.slot)
			self.ship:AddMoney(price)
			self.ship:GetDockedWith():AddEquipmentStock(self.selectedEquip[1], 1)
			self.selectedEquip = { nil, nil }
			return
		end
		ui.sameLine()
		ui.text(l.PRICE .. ": " .. Format.Money(price))
	end
end

function EquipmentWidget:draw()
	-- reset hovered equipment state
	self.showHoveredEquipLocation = false

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
			if self.tabs[self.activeTab] == equipmentInfoTab and self.station and self.selectedEquip then

				local _pos = ui.getCursorPos()
				local marketSize = ui.getContentRegion() - Vector2(0, ui.getButtonHeight(pionillium.heading))

				if self.selectedEquip then
					self.equipmentMarket.title = self.selectedEquip[1] and l.REPLACE_EQUIPMENT_WITH or l.AVAILABLE_FOR_PURCHASE
					self.equipmentMarket.style.size = marketSize
					self.equipmentMarket:render()
				end

				ui.withFont(pionillium.heading, function()
					ui.setCursorPos(_pos + Vector2(0, marketSize.y))
					self:drawMarketButtons()
					ui.sameLine()
				end)

			else
				self:drawShipSpinner()
			end
		end)
	end)
end

function EquipmentWidget:refresh()
	self.selectedEquip = nil
	self.selectedEquipSlots = nil

	local shipDef = ShipDef[self.ship.shipId]
	self.modelSpinner:setModel(shipDef.modelName, self.ship:GetSkin(), self.ship.model.pattern)
	self.modelSpinner.spinning = false
end

function EquipmentWidget:debugReload()
	package.reimport('pigui.libs.item-card')
	package.reimport()
end

return EquipmentWidget
