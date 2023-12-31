-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local PiGui = require 'Engine'.pigui
local Vector2 = Vector2
local utils = require 'utils'

local ui = require 'pigui'
local colors = ui.theme.colors
local pionillium = ui.fonts.pionillium

--
-- Class: UI.ItemCard
--
-- This class implements an "item card" display with a line of detailed stats
-- below it.
--

---@class UI.ItemCard
local ItemCard = utils.inherits(nil, "UI.ItemCard")

ItemCard.highlightBar = false
ItemCard.detailFields = 2

ItemCard.iconSize = nil ---@type Vector2?
ItemCard.lineSpacing  = ui.theme.styles.ItemSpacing
ItemCard.rounding = 4

ItemCard.backgroundColor = colors.tableBackground
ItemCard.selectedColor   = colors.tableSelection
ItemCard.hoveredColor    = colors.tableHighlight

function ItemCard:drawTitle(data, regionWidth, isSelected)
	-- override to draw your specific item card type!
end

function ItemCard:drawTooltip(data, isSelected)
	-- override to draw your specific item card type!
end

function ItemCard:drawDetailTooltip(detail, tooltip)
	-- can be overridden to draw extra info!

	ui.withFont(pionillium.body, function()
		ui.withStyleVars({ WindowPadding = ui.theme.styles.WindowPadding }, function()
			ui.setTooltip(tooltip)
		end)
	end)
end

function ItemCard:calcHeight()
	local lineSpacing = self.lineSpacing

	local textHeight = pionillium.body.size + pionillium.details.size + lineSpacing.y
	local iconHeight = self.iconSize and self.iconSize.y or 0
	local totalHeight = math.max(iconHeight, textHeight) + lineSpacing.y * 2

	return totalHeight
end

-- Draw an empty item card background
function ItemCard:drawBackground(isSelected)

	local lineSpacing = self.lineSpacing
	local totalHeight = self:calcHeight()

	-- calculate the background area
	local highlightBegin = ui.getCursorScreenPos()
	local highlightSize = Vector2(ui.getContentRegion().x, totalHeight)
	local highlightEnd = highlightBegin + highlightSize

	ui.dummy(highlightSize)

	local isHovered = ui.isMouseHoveringRect(highlightBegin, highlightEnd + Vector2(0, lineSpacing.y)) and ui.isWindowHovered()
	local bgColor = (isSelected and self.selectedColor) or (isHovered and self.hoveredColor) or self.backgroundColor

	if self.highlightBar then
		-- if we're hovered, we want to draw a little bar to the left of the background
		if isHovered or isSelected then
			ui.addRectFilled(highlightBegin - Vector2(self.rounding, 0), highlightBegin + Vector2(0, totalHeight), colors.equipScreenHighlight, 2, 5)
		end

		ui.addRectFilled(highlightBegin, highlightEnd, bgColor, self.rounding, (isHovered or isSelected) and 10 or 0) -- 10 == top-right | bottom-right
	else
		-- otherwise just draw a normal rounded rectangle
		ui.addRectFilled(highlightBegin, highlightEnd, bgColor, self.rounding, 0)
	end

	local isClicked = isHovered and ui.isMouseClicked(0)

	return isClicked, isHovered, highlightSize

end

-- Draw an "item card" visual with a primary icon, customizable title line, and
-- a row of detailed stats.
--
-- draw takes a "data" argument table with a minimum set of required fields:
--   icon  - icon index to draw on the item
--   [...] - up to 4 { icon, value, tooltip } data items for the stats line
function ItemCard:draw(data, isSelected)
	local lineSpacing = self.lineSpacing

	-- initial sizing setup
	local textHeight = pionillium.body.size + pionillium.details.size + lineSpacing.y

	local iconSize = self.iconSize or Vector2(textHeight)
	local iconOffset = (textHeight - iconSize.y) * 0.5

	local textWidth = ui.getContentRegion().x - iconSize.x - lineSpacing.x * 3

	local totalHeight = math.max(iconSize.y, textHeight) + lineSpacing.y * 2

	-- calculate the background area
	local highlightBegin = ui.getCursorScreenPos()
	local highlightSize = Vector2(ui.getContentRegion().x, totalHeight)
	local highlightEnd = highlightBegin + highlightSize

	ui.beginGroup()
	ui.dummy(highlightSize)

	local isHovered = ui.isItemHovered()
	local isClicked = ui.isItemClicked(0)

	local bgColor = (isSelected and self.selectedColor) or (isHovered and self.hoveredColor) or self.backgroundColor

	if self.highlightBar then
		-- if we're hovered, we want to draw a little bar to the left of the background
		if isHovered or isSelected then
			ui.addRectFilled(highlightBegin - Vector2(self.rounding, 0), highlightBegin + Vector2(0, totalHeight), colors.equipScreenHighlight, 2, 5)
		end

		ui.addRectFilled(highlightBegin, highlightEnd, bgColor, self.rounding, (isHovered or isSelected) and 10 or 0) -- 10 == top-right | bottom-right
	else
		-- otherwise just draw a normal rounded rectangle
		ui.addRectFilled(highlightBegin, highlightEnd, bgColor, self.rounding, 0)
	end

	local detailTooltip = nil

	PiGui.PushClipRect(highlightBegin + lineSpacing, highlightEnd - lineSpacing, true)
	ui.setCursorScreenPos(highlightBegin)

	ui.withStyleVars({ ItemSpacing = lineSpacing }, function()
		-- Set up padding for the top and left sides
		ui.addCursorPos(lineSpacing + Vector2(0, iconOffset))

		-- Draw the main icon and add some spacing next to it
		ui.icon(data.icon, iconSize, colors.white)
		-- ui.addCursorPos(Vector2(iconHeight + lineSpacing.x, 0))
		ui.sameLine()
		ui.addCursorPos(Vector2(0, -iconOffset))

		-- Draw the title line
		local pos = ui.getCursorPos()
		self:drawTitle(data, textWidth, isSelected)

		-- Set up the details line
		pos = pos + Vector2(0, ui.getTextLineHeightWithSpacing())
		ui.setCursorPos(pos)

		-- This block draws several small icons with text next to them
		ui.withFont(pionillium.details, function()
			-- size of the small details icons
			local smIconSize = Vector2(ui.getTextLineHeight())
			local fieldSize = textWidth / self.detailFields

			-- do all of the text first to generate as few draw commands as possible
			for i, v in ipairs(data) do
				local offset = fieldSize * (i - 1) + smIconSize.x + 2
				ui.setCursorPos(pos + Vector2(offset, 1)) -- HACK: force 1-pixel offset here to align baselines
				ui.text(v[2])
				if v[3] and ui.isItemHovered() then
					detailTooltip = v
				end
			end

			-- Then draw the icons
			for i, v in ipairs(data) do
				local offset = fieldSize * (i - 1)
				ui.setCursorPos(pos + Vector2(offset, 0))
				ui.icon(v[1], smIconSize, colors.white)
			end

			-- ensure we consume the appropriate amount of space if we don't have any details
			if #data == 0 then
				ui.newLine()
			end
		end)

		-- Add a bit of spacing after the slot
		ui.spacing()

		if isHovered and not detailTooltip then
			self:drawTooltip(data, isSelected)
		elseif detailTooltip then
			self:drawDetailTooltip(detailTooltip, detailTooltip[3])
		end
	end)

	PiGui.PopClipRect()

	ui.endGroup()

	return isClicked, isHovered, highlightSize
end

return ItemCard
