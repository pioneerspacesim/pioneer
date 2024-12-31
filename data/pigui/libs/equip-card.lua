-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ItemCard = require 'pigui.libs.item-card'
local Vector2 = Vector2
local utils = require 'utils'

local Lang = require 'Lang'
local le = Lang.GetResource("equipment-core")

local ui = require 'pigui'

local icons = ui.theme.icons
local colors = ui.theme.colors
local styles = ui.theme.styles
local pionillium = ui.fonts.pionillium

--
-- =============================================================================
--  Equipment Item Card
-- =============================================================================
--

-- Class: UI.EquipCard
--
-- Draw all visuals related to an individual equipment item.
-- This includes the background, highlight, icon, [slot type],
-- name, [size], and up to 4 sub-stats directly on the icon card
--
-- DrawEquipmentItem* functions take a "data" argument table with the form:
--   icon  - icon index to display for the slot
--   name  - translated name to display on the slot
--   equip - equipment object being drawn (or none for an empty slot)
--   type* - translated "slot type" name to display
--   size* - string of the form "S1" to display in the slot name for sized slots
--   stats - list of detailed item stats to display in the item tooltip,
--           in the format { label, icon, value, color }
--   [...] - up to 4 { icon, value, tooltip } data items for the stats line

---@class UI.EquipCard : UI.ItemCard
---@field New fun(): self
local EquipCard = utils.inherits(ItemCard, "UI.EquipCard")

---@class UI.EquipCard.Data
---@field icon ui.Icon
---@field name string?
---@field count integer?
---@field type string?
---@field size string?
---@field equip EquipType?
---@field slot HullConfig.Slot?
---@field present integer?
---@field total integer?
---@field stats EquipType.UI.Stats[] | nil

EquipCard.tooltipStats = true
EquipCard.highlightBar = true
EquipCard.detailFields = 4
EquipCard.tooltipWrapEm = 18

EquipCard.emptyIcon = icons.autopilot_dock

local tooltipStyle = {
	WindowPadding = ui.theme.styles.WindowPadding,
	WindowRounding = EquipCard.rounding
}

---@param data UI.EquipCard.Data
function EquipCard:tooltipContents(data, isSelected)
	if not data.equip then
		return
	end

	ui.withFont(pionillium.body, function()
		ui.text(data.equip:GetName())

		local desc = data.equip:GetDescription()

		if desc and desc ~= "" then
			ui.withFont(pionillium.details, function()
				ui.spacing()
				ui.textWrapped(desc)
			end)
		end
	end)

	if self.tooltipStats and data.stats then
		ui.spacing()
		ui.separator()
		ui.spacing()

		ui.withFont(pionillium.details, function()
			self.drawEquipStats(data)
		end)
	end
end

---@param data UI.EquipCard.Data
function EquipCard:drawTooltip(data, isSelected)
	if not (self.tooltipStats and data.stats or data.equip) then
		return
	end

	ui.withStyleVars(tooltipStyle, function()
		ui.customTooltip(function()
			local wrapWidth = ui.getTextLineHeight() * self.tooltipWrapEm

			ui.pushTextWrapPos(wrapWidth)

			self:tooltipContents(data, isSelected)

			ui.popTextWrapPos()
		end)
	end)
end

---@param data UI.EquipCard.Data
function EquipCard.drawEquipStats(data)
	if ui.beginTable("EquipStats", 2, { "NoSavedSettings" }) then

		ui.tableSetupColumn("##name", { "WidthStretch" })
		ui.tableSetupColumn("##value", { "WidthFixed" })

		ui.pushTextWrapPos(-1)
		for i, tooltip in ipairs(data.stats) do
			ui.tableNextRow()

			ui.tableSetColumnIndex(0)
			ui.text(tooltip[1] .. ":")

			ui.tableSetColumnIndex(1)
			ui.icon(tooltip[2], Vector2(ui.getTextLineHeight(), ui.getTextLineHeight()), ui.theme.colors.font)
			ui.sameLine(0, styles.ItemInnerSpacing.x)

			local value, format = tooltip[3], tooltip[4]
			ui.text(format(value))
		end
		ui.popTextWrapPos()

		ui.endTable()
	end
end

local slotColors = { Text = colors.equipScreenBgText }

---@param data UI.EquipCard.Data
function EquipCard:drawTitle(data, textWidth, isSelected)
	local pos = ui.getCursorPos()

	-- Draw the name of what's in the slot
	if data.name then
		ui.text(data.name)
	else
		ui.withStyleColors(slotColors, function()
			ui.text("[" .. le.EMPTY_SLOT .. "]")
		end)
	end

	-- If there's a slot count, draw it
	if data.count then
		ui.sameLine()
		ui.withStyleColors(slotColors, function()
			ui.text("x" .. data.count)
		end)
	elseif data.present then
		ui.sameLine()
		ui.withStyleColors(slotColors, function()
			ui.text(data.present .. "/" .. data.total)
		end)
	end

	-- Draw the size and/or type of the slot
	local slotType = data.type and data.size and "{type} | {size}" % data
		or data.type or data.size

	if slotType then
		ui.setCursorPos(pos + Vector2(textWidth - ui.calcTextSize(slotType).x, 0))
		ui.withStyleColors(slotColors, function()
			ui.text(slotType)
		end)
	end
end

---@param equip EquipType?
---@param compare EquipType?
function EquipCard.getDataForEquip(equip, compare)
	---@type UI.EquipCard.Data
	local out = {
		icon = equip and icons[equip.icon_name] or EquipCard.emptyIcon,
		iconColor = equip and colors.white or colors.fontDim
	}

	if equip then
		out.name = equip:GetName()
		out.equip = equip
		out.size = equip.slot and ("S" .. equip.slot.size) or nil
		out.count = equip.count

		out.stats = equip:GetDetailedStats()

		for i, v in ipairs(equip:GetItemCardStats()) do
			out[i] = v
		end
	end

	return out
end

return EquipCard
