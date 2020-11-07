
local commodities = require 'Economy'.GetCommodities()
local ui = require 'pigui'

local lc = require 'Lang'.GetResource('core')
local lui = require 'Lang'.GetResource('ui-core')
local lcomm = require 'Lang'.GetResource('commodity')

local icons = ui.theme.icons
local colors = ui.theme.colors

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

local function drawPriceIcon(price, size, tradeComp)
	local icon, color, tooltip = nil

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
	end

	if icon then ui.icon(icon, size, color, tooltip) else ui.dummy(size) end
end

function systemEconView.drawEconList(commList, illegalList, showOther)
	local width = ui.getColumnWidth()
	local iconWidth = ui.getTextLineHeight() + 4
	local iconSize = Vector2(iconWidth, iconWidth)

	local currSysPos = width - (showOther and iconWidth * 2 or iconWidth)
	ui.child("CommodityList", Vector2(0, 0), ui.WindowFlags{"NoScrollbar"}, function()
		for _, info in ipairs(commList) do
			ui.text(rawget(lcomm, info[1]) or ('[NO_JSON]'..info[1]))
			if showOther and math.abs(info[3] - info[2]) > major_import then
				ui.sameLine(width - iconWidth * 3)
				local profit = info[3] - info[2] > 0.0
				ui.icon(profit and icons.econ_profit or icons.econ_loss, iconSize, profit and colors.econProfit or colors.econLoss)
			end
			ui.sameLine(currSysPos)

			drawPriceIcon(info[2], iconSize)
			if showOther then
				ui.sameLine(width - iconWidth)
				drawPriceIcon(info[3], iconSize)
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
			ui.text(rawget(lcomm, info[1]) or ('[NO_JSON]'..info[1]))

			-- only display illegal icon if the commodity is actually legal in the other system
			if showOther and info[2] or info[3] then
				ui.sameLine(currSysPos)
				drawPriceIcon(info[2], iconSize)
				ui.sameLine(width - iconWidth)
				drawPriceIcon(info[3], iconSize)
			end
		end
	end)
end

function systemEconView.draw(current, other)
	local commList, illegalList = systemEconView.buildCommodityList(current, other)
	systemEconView.drawEconList(commList, illegalList, other)
end

return systemEconView
