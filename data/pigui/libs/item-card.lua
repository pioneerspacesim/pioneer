-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
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
ItemCard.lineSpacing = ui.theme.styles.ItemSpacing
ItemCard.rounding = ui.theme.styles.ItemCardRounding

ItemCard.colors = ui.theme.buttonColors.card
ItemCard.iconColor = colors.white

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

	local totalHeight = self:calcHeight()

	-- calculate the background area
	local highlightSize = Vector2(ui.getContentRegion().x, totalHeight)

	ui.dummy(highlightSize)
	local tl, br = ui.getItemRect()

	local isHovered = ui.isItemHovered()
	local isClicked = ui.isItemClicked(0)

	local bgColor = ui.getButtonColor(self.colors, isHovered, isSelected)

	if self.highlightBar then
		-- if we're hovered, we want to draw a little bar to the left of the background
		if isHovered or isSelected then
			ui.addRectFilled(tl - Vector2(self.rounding, 0), tl + Vector2(0, totalHeight), colors.equipScreenHighlight, 2, 5)
		end

		ui.addRectFilled(tl, br, bgColor, self.rounding, (isHovered or isSelected) and 10 or 0) -- 10 == top-right | bottom-right
	else
		-- otherwise just draw a normal rounded rectangle
		ui.addRectFilled(tl, br, bgColor, self.rounding, 0)
	end

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

	if self.highlightBar then
		ui.addCursorPos(Vector2(self.rounding, 0))
	end

	-- initial sizing setup
	local textHeight = pionillium.body.size + pionillium.details.size + lineSpacing.y

	local iconSize = self.iconSize or Vector2(textHeight)
	local iconOffset = (textHeight - iconSize.y) * 0.5

	local textWidth = ui.getContentRegion().x - iconSize.x - lineSpacing.x * 3

	ui.beginGroup()

	local isClicked, isHovered, highlightSize = self:drawBackground(isSelected)

	-- constrain rendering inside the frame padding area
	local tl, br = ui.getItemRect()
	PiGui.PushClipRect(tl + lineSpacing, br - lineSpacing, true)

	local detailTooltip = nil

	ui.setCursorScreenPos(tl + lineSpacing)

	ui.withStyleVars({ ItemSpacing = lineSpacing }, function()
		-- Draw the main icon
		-- The icon is offset vertically to center it in the available space if
		-- smaller than the height of the text
		ui.addIconSimple(tl + lineSpacing + Vector2(0, iconOffset), data.icon, iconSize, data.iconColor or self.iconColor)

		-- Position the cursor for the title and details
		local textLinePos = tl + lineSpacing + Vector2(iconSize.x + lineSpacing.x, 0)
		ui.setCursorScreenPos(textLinePos)

		-- Draw the title line
		self:drawTitle(data, textWidth, isSelected)

		-- Set up the details line
		local pos = textLinePos + Vector2(0, ui.getTextLineHeightWithSpacing())

		-- This block draws several small icons with text next to them
		ui.withFont(pionillium.details, function()
			-- size of the small details icons
			local smIconSize = Vector2(ui.getTextLineHeight())
			local fieldSize = textWidth / self.detailFields

			-- do all of the text first to generate as few draw commands as possible
			for i, v in ipairs(data) do
				local offset = fieldSize * (i - 1) + smIconSize.x + 2
				ui.setCursorScreenPos(pos + Vector2(offset, 1)) -- HACK: force 1-pixel offset here to align baselines
				ui.text(v[2])
				if v[3] and ui.isItemHovered("ForTooltip") then
					detailTooltip = v
				end
			end

			-- Then draw the icons
			for i, v in ipairs(data) do
				local offset = fieldSize * (i - 1)
				ui.setCursorScreenPos(pos + Vector2(offset, 0))
				ui.icon(v[1], smIconSize, colors.white)
			end
		end)

		-- Add a bit of spacing after the slot
		ui.spacing()

		if isHovered and detailTooltip then
			self:drawDetailTooltip(detailTooltip, detailTooltip[3])
		end
	end)

	PiGui.PopClipRect()

	ui.endGroup()

	if ui.isItemHovered("ForTooltip") and not detailTooltip then
		self:drawTooltip(data, isSelected)
	end

	return isClicked, isHovered, highlightSize
end

return ItemCard
