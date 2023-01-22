-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt


local commodities = require 'Economy'.GetCommodities()
local ui = require 'pigui'

local lc = require 'Lang'.GetResource('core')
local lui = require 'Lang'.GetResource('ui-core')
local lcomm = require 'Lang'.GetResource('commodity')

local icons = ui.theme.icons
local colors = ui.theme.colors
local pionillium = ui.fonts.pionillium

local systemEconView = {}

function systemEconView.buildCommodityList(sys, otherSys)
	local commodityList = {}
	local illegalList = {}

	for name, info in pairs(commodities) do
		local legal = sys:IsCommodityLegal(name)
		local otherLegal = otherSys and otherSys:IsCommodityLegal(name)

		if legal and (not otherSys or otherLegal) then
			table.insert(commodityList, {
				info.l10n_key,
				legal and sys:GetCommodityBasePriceAlterations(name),
				otherSys and otherLegal and otherSys:GetCommodityBasePriceAlterations(name)
			})
		else
			table.insert(illegalList, {
				info.l10n_key,
				legal and sys:GetCommodityBasePriceAlterations(name),
				otherSys and otherLegal and otherSys:GetCommodityBasePriceAlterations(name)
			})
		end
	end

	table.sort(commodityList, function(a, b) return a[1] < b[1] end)
	table.sort(illegalList, function(a, b) return a[1] < b[1] end)
	return commodityList, illegalList
end

-- break-over price percentage for a commodity to be considered a minor/major export
local minor_export = -4
local major_export = -10

-- break-over price percentage for a commodity to be considered a minor/major import
local minor_import = 4
local major_import = 10

local function getExportInfo(price)
	local icon, color, tooltip

	if not price then
		icon = icons.alert_generic
		color = colors.econIllegalCommodity
		tooltip = lui.ILLEGAL_IN_SYSTEM
	elseif price >= minor_import then
		icon = price > major_import and icons.econ_major_import or icons.econ_minor_import
		color = price > major_import and colors.econMajorImport or colors.econMinorImport
		tooltip = price > major_import and lui.MAJOR_IMPORT or lui.MINOR_IMPORT
	elseif price <= minor_export then
		icon = price < major_export and icons.econ_major_export or icons.econ_minor_export
		color = price < major_export and colors.econMajorExport or colors.econMinorExport
		tooltip = price < major_export and lui.MAJOR_EXPORT or lui.MINOR_EXPORT
	else
		color = colors.font
		tooltip = lui.MINIMAL_TRADE
	end

	return icon, color, tooltip
end

local function getProfitabilityInfo(priceA, priceB)
	local icon, color, tooltip
	local priceDiff = (priceA and priceB) and priceB - priceA

	if priceDiff and math.abs(priceDiff) > major_import then
		icon = priceDiff > 0.0 and icons.econ_profit or icons.econ_loss
		color = priceDiff > 0.0 and colors.econProfit or colors.econLoss
		tooltip = priceDiff > 0.0 and lui.PROFITABLE_TRADE or lui.UNPROFITABLE_TRADE
	end

	return icon, color, tooltip
end

local function drawPriceIcon(price, size)
	local icon, color = getExportInfo(price)
	if icon then ui.icon(icon, size, color) else ui.dummy(size) end
end

local function drawCommodityTooltip(info, thisSystem, otherSystem)
	ui.customTooltip(function()
		ui.withFont(pionillium.medlarge, function() ui.text(lcomm[info[1]]) end)
		local color, tooltip = select(2, getProfitabilityInfo(info[2], info[3]))
		if otherSystem and tooltip then
			ui.textColored(color, tooltip)
		end

		ui.spacing()

		ui.text(lui.TRADING_FROM:interp({system = thisSystem.name}))
		color, tooltip = select(2, getExportInfo(info[2]))
		if tooltip then
			ui.sameLine()
			ui.textColored(color, tooltip)
		end

		if otherSystem then
			ui.text(lui.TRADING_TO:interp({system = otherSystem.name}))
			color, tooltip = select(2, getExportInfo(info[3]))
			if tooltip then
				ui.sameLine()
				ui.textColored(color, tooltip)
			end
		end
	end)
end

function systemEconView.drawEconList(commList, illegalList, thisSystem, otherSystem)
	local width = ui.getColumnWidth()
	local iconWidth = ui.getTextLineHeight() + 4
	local iconSize = Vector2(iconWidth, iconWidth)

	local currSysPos = width - (otherSystem and iconWidth * 2 or iconWidth)
	ui.child("CommodityList", Vector2(0, 0), ui.WindowFlags{"NoScrollbar"}, function()
		for _, info in ipairs(commList) do
			ui.group(function()
				ui.text(lcomm[info[1]])
				ui.sameLine(width - iconWidth * 3)

				local icon, color = getProfitabilityInfo(info[2], info[3])
				if otherSystem and icon then ui.icon(icon, iconSize, color) else ui.dummy(iconSize) end

				if otherSystem then
					ui.sameLine(0, 0)
					drawPriceIcon(info[2], iconSize)
					ui.sameLine(0, 0)
					drawPriceIcon(info[3], iconSize)
				else
					ui.sameLine(0, iconWidth)
					drawPriceIcon(info[2], iconSize)
				end
			end)

			if ui.isItemHovered() then
				drawCommodityTooltip(info, thisSystem, otherSystem)
			end
		end

		if not illegalList or #illegalList == 0 then return end

		ui.spacing()
		ui.separator()
		ui.spacing()
		ui.withStyleColors({ Text = colors.econIllegalCommodity }, function()
			ui.text(lc.ILLEGAL_GOODS)
		end)
		ui.spacing()

		for _, info in ipairs(illegalList) do
			ui.group(function()
				ui.text(lcomm[info[1]])
				ui.sameLine(width - iconWidth * 2, 0)

				-- only display illegal icon if the commodity is actually legal in the other system
				if otherSystem and (info[2] or info[3]) then
					drawPriceIcon(info[2], iconSize)
					ui.sameLine(0, 0)
					drawPriceIcon(info[3], iconSize)
				end
			end)

			if ui.isItemHovered() then
				drawCommodityTooltip(info, thisSystem, otherSystem)
			end
		end
	end)
end

function systemEconView.draw(current, other)
	local commList, illegalList = systemEconView.buildCommodityList(current, other)
	systemEconView.drawEconList(commList, illegalList, current, other)
end

return systemEconView
