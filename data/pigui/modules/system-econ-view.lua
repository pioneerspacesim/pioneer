-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game    = require 'Game'
local Economy = require 'Economy'
local ui      = require 'pigui'
local utils   = require 'utils'

local lc = require 'Lang'.GetResource('core')
local lui = require 'Lang'.GetResource('ui-core')
local lcomm = require 'Lang'.GetResource('commodity')

local Commodities = Economy.GetCommodities()

local icons = ui.theme.icons
local colors = ui.theme.colors
local pionillium = ui.fonts.pionillium

---@class UI.SystemEconView
---@field New fun(): self
local SystemEconView = utils.class('UI.SystemEconView')

local CompareMode = {
	BySystem = 1,
	ByStation = 2
}

local priceModTab = {
	major_import = { icons.econ_major_import, colors.econMajorImport, lui.MAJOR_IMPORT },
	minor_import = { icons.econ_minor_import, colors.econMinorImport, lui.MINOR_IMPORT },
	major_export = { icons.econ_major_export, colors.econMajorExport, lui.MAJOR_EXPORT },
	minor_export = { icons.econ_minor_export, colors.econMinorExport, lui.MINOR_EXPORT },

	illegal = { icons.alert_generic, colors.econIllegalCommodity, lui.ILLEGAL_IN_SYSTEM },
	minimal_trade = { nil, colors.font, lui.MINIMAL_TRADE },
}

local profitTab = {
	profit = { icons.econ_profit, colors.econProfit, lui.PROFITABLE_TRADE },
	loss = { icons.econ_loss, colors.econLoss, lui.UNPROFITABLE_TRADE },
}

-- break-over price percentage for a commodity to be considered a minor/major export
local minor_export = -4
local major_export = -10

-- break-over price percentage for a commodity to be considered a minor/major import
local minor_import = 4
local major_import = 10

-- price percentage difference necessary to make a trade viable
local viable_trade = 4 + Economy.TotalTradeFees

function SystemEconView.ClassifyPrice(pricemod)
	if not pricemod then
		return priceModTab.illegal
	elseif pricemod >= major_import then
		return priceModTab.major_import
	elseif pricemod >= minor_import then
		return priceModTab.minor_import
	elseif pricemod <= major_export then
		return priceModTab.major_export
	elseif pricemod <= minor_export then
		return priceModTab.minor_export
	end
end

function SystemEconView.GetPricemod(item, price)
	return (price / item.price - 1.0) * 100
end

function SystemEconView:Constructor()
	self.compareMode = CompareMode.BySystem
	self.selectedCommodity = nil
	self.savedMarket = nil
end

---@param sys StarSystem
---@param otherSys StarSystem?
function SystemEconView.buildCommodityList(sys, otherSys)
	local commodityList = {}
	local illegalList = {}

	for name, info in pairs(Commodities) do
		local legal = sys:IsCommodityLegal(name)
		local otherLegal = otherSys and otherSys:IsCommodityLegal(name)

		local tab = {
			info.l10n_key,
			legal and sys:GetCommodityBasePriceAlterations(name),
			otherSys and otherLegal and otherSys:GetCommodityBasePriceAlterations(name)
		}

		if legal and (not otherSys or otherLegal) then
			table.insert(commodityList, tab)
		else
			table.insert(illegalList, tab)
		end
	end

	table.sort(commodityList, function(a, b) return a[1] < b[1] end)
	table.sort(illegalList, function(a, b) return a[1] < b[1] end)
	return commodityList, illegalList
end

---@param system StarSystem
---@param station SpaceStation
---@param otherStation SpaceStation?
function SystemEconView.buildStationCommodityList(system, station, otherStation)
	local commodityList = {}
	local illegalList = {}

	for name, item in pairs(Commodities) do
		local legal = system:IsCommodityLegal(name)
		local systemPrice = system:GetCommodityBasePriceAlterations(name)
		local price = station:GetCommodityPrice(item)
		local otherPrice = otherStation and otherStation:GetCommodityPrice(item)

		local tab = {
			item.l10n_key,
			legal and SystemEconView.GetPricemod(item, price) - systemPrice,
			legal and otherPrice and SystemEconView.GetPricemod(item, otherPrice) - systemPrice
		}

		table.insert(legal and commodityList or illegalList, tab)
	end

	table.sort(commodityList, function(a, b) return a[1] < b[1] end)
	table.sort(illegalList, function(a, b) return a[1] < b[1] end)
	return commodityList, illegalList
end

local function getProfitabilityInfo(priceA, priceB)
	local priceDiff = (priceA and priceB) and priceA - priceB

	if priceDiff and math.abs(priceDiff) > viable_trade then
		return priceDiff > 0 and profitTab.profit or profitTab.loss
	end
end

local function drawIcon(cls, size)
	if cls then
		ui.icon(cls[1], size, cls[2])
	else
		ui.dummy(size)
	end
end

local function drawExportTooltip(price)
	local cls = SystemEconView.ClassifyPrice(price) or priceModTab.minimal_trade
	ui.sameLine()
	ui.textColored(cls[2], cls[3])
end

local function drawCommodityTooltip(info, thisSystem, otherSystem)
	ui.customTooltip(function()
		ui.withFont(pionillium.heading, function() ui.text(lcomm[info[1]]) end)

		local profit = getProfitabilityInfo(info[2], info[3])
		if otherSystem and profit then
			ui.textColored(profit[2], profit[3])
		end

		ui.spacing()

		if otherSystem then
			ui.text(lui.TRADING_FROM:interp({system = otherSystem.name}))
			drawExportTooltip(info[3])

			ui.text(lui.TRADING_TO:interp({system = thisSystem.name}))
			drawExportTooltip(info[2])
		else
			ui.text(lui.TRADING_AT:interp({system = thisSystem.name}))
			drawExportTooltip(info[2])
		end
	end)
end

---@param thisSystem table
---@param otherSystem table?
function SystemEconView:drawCommodityList(commList, illegalList, thisSystem, otherSystem)
	local width = ui.getColumnWidth()
	local iconWidth = ui.getTextLineHeight() + 4
	local iconSize = Vector2(iconWidth, iconWidth)

	ui.child("CommodityList", Vector2(0, 0), ui.WindowFlags{"NoScrollbar"}, function()
		for _, info in ipairs(commList) do
			ui.group(function()
				ui.text(lcomm[info[1]])
				ui.sameLine(width - iconWidth * 3)

				drawIcon(otherSystem and getProfitabilityInfo(info[2], info[3]), iconSize)

				if otherSystem then
					ui.sameLine(0, 0)
					drawIcon(SystemEconView.ClassifyPrice(info[3]), iconSize)
					ui.sameLine(0, 0)
				else
					ui.sameLine(0, iconWidth)
				end

				drawIcon(SystemEconView.ClassifyPrice(info[2]), iconSize)
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
					drawIcon(SystemEconView.ClassifyPrice(info[2]), iconSize)
					ui.sameLine(0, 0)
					drawIcon(SystemEconView.ClassifyPrice(info[3]), iconSize)
				end
			end)

			if ui.isItemHovered() then
				drawCommodityTooltip(info, thisSystem, otherSystem)
			end
		end
	end)
end

---@param selected StarSystem
---@param current StarSystem
function SystemEconView:drawSystemComparison(selected, current)
	local showComparison = current ~= selected and current and selected.population > 0
		and (Game.player["trade_computer_cap"] or 0) > 0

	local otherSys = showComparison and current or nil

	ui.withFont(pionillium.body, function()
		ui.text(lui.COMMODITY_TRADE_ANALYSIS)
		ui.spacing()

		ui.withFont(pionillium.heading, function()
			if showComparison then
				ui.text(current.name)
				ui.sameLine(ui.getContentRegion().x - ui.calcTextSize(selected.name).x)
			end

			ui.text(selected.name)
		end)

		ui.spacing()
		ui.separator()
		ui.spacing()

		if selected.population <= 0 then
			ui.icon(icons.alert_generic, Vector2(ui.getTextLineHeight()), ui.theme.colors.font)
			ui.sameLine()
			ui.text(lc.NO_REGISTERED_INHABITANTS)
		else
			local commList, illegalList = SystemEconView.buildCommodityList(selected, otherSys)
			self:drawCommodityList(commList, illegalList, selected, otherSys)
		end
	end)
end

---@param selected SystemBody
---@param current SystemBody?
function SystemEconView:drawStationComparison(selected, current)
	local showComparison = current ~= selected and current
		and (Game.player["trade_computer_cap"] or 0) > 0

	if not showComparison then current = nil end

	local system = selected.system
	local station = selected.body --[[@as SpaceStation]]
	local otherStation = current and current.body --[[@as SpaceStation?]]

	ui.withFont(pionillium.body, function()
		ui.text(lui.COMMODITY_TRADE_ANALYSIS)
		ui.spacing()

		ui.withFont(pionillium.heading, function()
			if current then
				ui.text(current.name)
				ui.sameLine(ui.getContentRegion().x - ui.calcTextSize(selected.name).x)
			end

			ui.text(selected.name)
		end)

		ui.spacing()
		ui.separator()
		ui.spacing()

		if not station or (showComparison and not otherStation) then
			ui.icon(icons.alert_generic, Vector2(ui.getTextLineHeight()), ui.theme.colors.font)
			ui.sameLine()
			ui.text(lui.NO_AVAILABLE_DATA)
		else
			local commList, illegalList = SystemEconView.buildStationCommodityList(system, station, otherStation)
			self:drawCommodityList(commList, illegalList, selected, current)
		end
	end)
end

local commodityOptions = {}
for k, v in pairs(Commodities) do
	table.insert(commodityOptions, k)
end
table.sort(commodityOptions)

function SystemEconView:drawPriceList(key, prices)
	local iconSize = Vector2(ui.getTextLineHeight() + 4)
	local out = nil

	if not ui.beginTable("prices", 3) then return end

	ui.tableSetupColumn("Name", { "WidthStretch" })
	ui.tableSetupColumn("Indicators")
	ui.tableSetupColumn("Amount")

	for i, info in ipairs(prices) do
		ui.tableNextRow()

		ui.tableSetColumnIndex(0)
		ui.alignTextToLineHeight(iconSize.y)
		if ui.selectable(info[1], false, { "SpanAllColumns" }) then out = i end

		ui.tableSetColumnIndex(1)

		local price = ui.Format.Money(info[2])
		local profit = (info[2] - self.savedMarket[key] > 0) and profitTab.profit or profitTab.loss
		drawIcon(profit, iconSize)

		ui.tableSetColumnIndex(2)
		ui.textColored(ui.theme.styleColors.gray_200, price)
	end

	ui.endTable()

	return out
end

function SystemEconView:drawSystemFinder()
	local selectedIndex = self.selectedCommodity or 1
	local key = commodityOptions[selectedIndex]
	local comm = Commodities[key]
	local commName = lcomm[comm.l10n_key]
	local station = Game.player:GetDockedWith() ---@type SpaceStation

	ui.withFont(pionillium.body, function()

		if self.compareMode == CompareMode.BySystem then
			if ui.button("[Nearby Systems]") then self.compareMode = CompareMode.ByStation end
		elseif self.compareMode == CompareMode.ByStation then
			if ui.button("[This System]") then self.compareMode = CompareMode.BySystem end
		end

		if station then
			ui.sameLine()

			if ui.button("Load Market Data") then
				self:loadMarketData(Economy.GetStationMarket(station.path).commodities)
			end
		end

		ui.nextItemWidth(-1)
		local changed, index = ui.combo("##commodityCombo", selectedIndex - 1, commodityOptions)
		ui.spacing()
		if changed then self.selectedCommodity = index + 1 end


		if not self.savedMarket then

		else
			ui.text(lui.COMMODITY_TRADE_ANALYSIS)
			ui.spacing()

			ui.withFont(pionillium.heading, function()
				local price = ui.Format.Money(self.savedMarket[key])
				ui.textColored(ui.theme.styleColors.gray_200, commName)
				ui.sameLine(ui.getContentRegion().x - ui.calcTextSize(price).x)
				ui.text(price)
			end)

			ui.spacing()
			ui.separator()
			ui.spacing()

			if self.compareMode == CompareMode.BySystem then

				local nearbySystems = Game.system:GetNearbySystems(20, function(s) return s.numberOfStations > 0 end)

				local entries = utils.map_array(nearbySystems, function(system)
					local price = Economy.GetMarketPrice(comm.price, system:GetCommodityBasePriceAlterations(key))
					return { system.name, price, system.path }
				end)

				table.sort(entries, function(a, b) return a[2] > b[2] end)

				local idx = self:drawPriceList(key, entries)
				if idx then
					Game.sectorView:GotoSystemPath(entries[idx][3])
				end

			elseif self.compareMode == CompareMode.ByStation then

				local stations = utils.map_array(Game.system:GetStationPaths(), function(path)
					local sBody = path:GetSystemBody()
					local station = sBody.physicsBody --[[@as SpaceStation]]
					local price = station:GetCommodityPrice(Commodities[key])

					return { sBody.name, price, path }
				end)

				table.sort(stations, function(a, b) return a[2] > b[2] end)

				local idx = self:drawPriceList(key, stations)
				if idx then
					Game.player:SetNavTarget(stations[idx][3])
				end

			end

		end

	end)
end

function SystemEconView:loadMarketData(market)
	self.savedMarket = {}

	for key, data in pairs(market) do
		local comm = Commodities[key]
		self.savedMarket[key] = Economy.GetMarketPrice(comm.price, data[3])
	end
end

return SystemEconView
